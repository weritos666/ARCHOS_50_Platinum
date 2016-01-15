/*tydrv qupw add the file  for getting  batt id by bq2202a*/
/*notice:  only modify BATT_ID_GPIO pin number according your project*/
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
/*tydrv qupw add for battwiki misc control */
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>


#define  BATT_ID_GPIO   11    //use gpio11 connect batt_id pin

#define ROM_DATA_LEN  	8	/*7 BYTE IDENTIFICATION DATA & 1 BYTE CRC(ROM DATA)*/
#define PAGE_DATA_LEN 	32	/* 32BYTE PAGE DATA(EPROM)*/
#define PAGE_USEFUL_DATA 	11	/*useful data is data[1] to data[10],total bytes is 10:TBW5931BYD,and \0; data[0] is CRC data */

#define READ_ROM_CMD 	    0x33/*Read ROM Cmd = 0x33 */
#define READ_PAGE_CMD 	0xC3/*Read PAGE Cmd = 0xC3 */
#define READ_FILED_CMD 	0xF0/*Read PAGE Cmd = 0xF0 */

#define PAGE_LENTH        33     // 32 byte pagedata and 1byte crc

#define PAGE0_MSB_ADDR 	0x00     /*PAGE0 ADDR START = 0x0000*/
#define PAGE0_LSB_ADDR 	0x00


#define BQ2202A_GPIO_CONFIG_OUTPUT   GPIO_CFG(BATT_ID_GPIO,0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,GPIO_CFG_2MA)

#define BQ2202A_GPIO_CONFIG_INPUT   GPIO_CFG(BATT_ID_GPIO,0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,GPIO_CFG_2MA)

static u8 batt_legal =0;
static u8 gpio_request_ok =0;

#define MANUFACT_LENTH  10

static char g_batt_str[MANUFACT_LENTH]="INVALID";

struct batt_id_info{
	   char manufacture[MANUFACT_LENTH];
	   char batt_ids[PAGE_USEFUL_DATA+1];
};

static struct batt_id_info batt_id_groups[]=
{
  	{"BYD","TBW5931BYD"},
  	{"INVALID","NONE"}
};
bool hdq_init(uint32_t gpio)
{

	int Sdq_reset_Value =0x01;	

	if (gpio_request_ok==0)
	   return false;
	
	gpio_direction_output(BATT_ID_GPIO,0x00);
	
	udelay(500);  //more than 480 us 

	//gpio_tlmm_config(BQ2202A_GPIO_CONFIG_INPUT,GPIO_CFG_ENABLE);
	gpio_direction_input(BATT_ID_GPIO);  //actrule =20us
	udelay(80);
	
	Sdq_reset_Value = gpio_get_value(BATT_ID_GPIO);
	
	udelay(500);  //ensure more than 480us
	
	if(Sdq_reset_Value == 0x01){
		return false;
	}
	else 
		return true;	
}

void hdq_write_bit(uint32_t gpio,bool BitData)
{
	//gpio_tlmm_config(BQ2202A_GPIO_CONFIG_OUTPUT,GPIO_CFG_ENABLE);
	
	gpio_direction_output(BATT_ID_GPIO,0x00);
	//gpio_set_value(BATT_ID_GPIO, 0x00);
	 udelay(8); // max is 15 us

	// send bit 
	if (BitData == 0x01)
		gpio_direction_input(BATT_ID_GPIO);
	
	    udelay(80);   //max is 120 - 15 us

	//end send bit
	if (BitData == 0x00)
		gpio_direction_input(BATT_ID_GPIO);

	  udelay(8);   
	   

}

/*read bit reality use 35 us , means  5 +24 <35us then add Trec= over 5us  datesheet aqure   17 -60 us, */
bool hdq_read_bit(uint32_t gpio)
{
	int Sdq_read_Value;
	
	gpio_direction_output(BATT_ID_GPIO,0x00);	
	 udelay(5);  //less than 13us
	
	gpio_direction_input(BATT_ID_GPIO);
	
	 udelay(24);  //   18 -24  is stable
	
    Sdq_read_Value = gpio_get_value(BATT_ID_GPIO);
	   
     udelay(40);    // 
	   
	return (Sdq_read_Value&0x01);
}

void hdq_write_byte(uint32_t gpio,u8 data)
{
	u8  Count = 0;
	
	for(Count = 0 ; Count <= 7 ; Count++)
		hdq_write_bit(gpio,(data >> Count) & 0x01);	
}

u8 hdq_read_byte(uint32_t gpio){
	u8 Count = 0;
	u8 Read_Data = 0;
	
	for(Count = 0 ; Count <= 7 ; Count++)
		Read_Data |= hdq_read_bit(gpio) << Count;	
	
	return Read_Data;
}

u8 hdq_calc_crc(u8 data_byte, u8 crc){
	u32 bit_mask = 0;
	u8 carry_check = 0;
	u8 temp_data = 0;
	
	temp_data = data_byte;
	for ( bit_mask = 0; bit_mask <= 7; bit_mask ++)
	{
		data_byte = data_byte ^ crc;
		crc = crc >> 1;

		temp_data = temp_data >> 1;
		carry_check = data_byte & 0x01;
		if (carry_check)
		{
			crc = crc ^ 0x8C;
		}
		data_byte = temp_data;
	}
	return crc;
}

u8 hdq_check_sum_crc(u8 *RomData ,u32 Datalen)

{
	u8 crc_init = 0;
	u8 crc = 0;
	u32 i = 0;
	
	for(i = 0 ;i < Datalen ; i++){
	crc = hdq_calc_crc(RomData[i],crc_init);
	crc_init = crc;
	}
	
	//printk("bq2022 hdq_check_sum_crc =%x \n",crc);
	return crc ;
}
		
u8 Bq2022a_Identification_Check(void)
{

	u32 ByteCount;
	u8  ReadRomData[ROM_DATA_LEN] = {0};
	u8  ReadPageData[PAGE_USEFUL_DATA+1] = {0};  // conclude crc data
	u8  reset_try,id_try=0;
	u8  id_cnt;
	//u8  ReadPageCRCData[33] = {0};

readidretry:
for (reset_try=3;reset_try >0;reset_try--)
   	{
   	
	if(hdq_init(BATT_ID_GPIO)){ /*init Bq2022A ,reset & respone*/
		
		printk("bq2022a reset  OK!\r\n");
		break;
	}
	
   	}
   
    if (reset_try==0)
    {
    	printk("bq2022a reset  fail!\r\n");
		
         return 0;
		 
    }

	
	/*first send read rom cmd 0x33, read family code and id number +crc*/
	hdq_write_byte(BATT_ID_GPIO,READ_ROM_CMD);	

	/*read Rom data*/		
	for(ByteCount = 0 ; ByteCount < ROM_DATA_LEN ; ByteCount++)
		ReadRomData[ByteCount] = hdq_read_byte(BATT_ID_GPIO);
	
	//for(ByteCount = 0 ; ByteCount < ROM_DATA_LEN ; ByteCount++)
	   printk("bq2022 ReadRomData=%x, \n",ReadRomData[0]);  // id = 09h
	
	/*crc is right? */
	if( hdq_check_sum_crc(ReadRomData,ROM_DATA_LEN-1)!= ReadRomData[ROM_DATA_LEN-1]){
		printk("bq2022 check crc ReadRomData=%x, is failed \n",ReadRomData[ROM_DATA_LEN-1]);
		
		//return 0;
	}

	/*second send page 0 addr to read battery id chars  See figure 7*/
	hdq_write_byte(BATT_ID_GPIO,READ_PAGE_CMD);	/*read eprom data (page0)*/
	hdq_write_byte(BATT_ID_GPIO,PAGE0_MSB_ADDR);
	hdq_write_byte(BATT_ID_GPIO,PAGE0_LSB_ADDR);


#if 1
/*only read page useful data to save time  first data is crc data,*/
	for(ByteCount = 0 ; ByteCount < PAGE_USEFUL_DATA ; ByteCount++)/*useful data is data[0] to data[10],total bytes is 11:TBW5931BYD*/
			ReadPageData[ByteCount] = hdq_read_byte(BATT_ID_GPIO);

     // the end is 0;
	    ReadPageData[ByteCount]= 0;
#else
   /*read all page date then get useful date*/
	for(ByteCount = 0 ; ByteCount < PAGE_LENTH ; ByteCount++)/*useful data is data[1] to data[10],total bytes is 10:TBW5932BAK*/
			ReadPageCRCData[ByteCount] = hdq_read_byte(BATT_ID_GPIO);

     strncpy(ReadPageData,ReadPageCRCData,PAGE_USEFUL_DATA);
	 ReadPageData[PAGE_USEFUL_DATA]=0;
	 
#endif
	printk("bq2022 ReadPageData battery package0 id = %s \n\r",(char *)&ReadPageData[1]);

	for (id_cnt=0;id_cnt < sizeof(batt_id_groups)/sizeof(batt_id_groups[0]);id_cnt++)
		{
           if (strcmp(batt_id_groups[id_cnt].manufacture,"INVALID")==0)
           	{
           	   id_try ++;
			   
			   if (id_try >3) // three times
			      strcpy(g_batt_str,"INVALID");
			   else
			   	  goto readidretry;
			   
		   	   break;
           	}
		   else
		   	{
              if (strcmp(batt_id_groups[id_cnt].batt_ids, (char *)&ReadPageData[1])==0)
			  	 {
			  	   strcpy(g_batt_str,batt_id_groups[id_cnt].manufacture);
			       
				   printk("bq2022 :This batterry is legal ok,manufacture is :%s \n",g_batt_str);
				   return 1;  // succsess
			       
              	 }
		    }
	    }

   
	return 0;   // fail
	
}

/* Send to msm_battery.c power supply batterystatus ,1: legal batt  0: unlegal batt*/
u8 bq2202a_report_batt_status(void)
{
	
    return batt_legal;
}

EXPORT_SYMBOL(bq2202a_report_batt_status);
/*add it to a misc device */

#define BATID_IOR 0X3F
#define IOCTL_GET_BATT_ID _IOR(BATID_IOR, 0x02, char[MANUFACT_LENTH])

static int bq2202a_open(struct inode *inode, struct file *file)
{
	
	return nonseekable_open(inode, file);
}
static long
bq2202a_ioctl(struct file *file, unsigned int cmd,
		   unsigned long arg)
{
	void __user *argp = (void __user *)arg;
    char str_buf[MANUFACT_LENTH];
	
	switch (cmd) {
		
		case IOCTL_GET_BATT_ID:
			
			  strcpy(str_buf, g_batt_str);
			
			if(copy_to_user(argp, str_buf, strlen(str_buf))) {
				printk("bq2202a read batt id ioctl failed \n");
				return -EFAULT;
			}
		    return 0;
		default:		
		    return 0;
		}
}



static const struct file_operations bq2202a_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = bq2202a_ioctl,
	.open = bq2202a_open,
};

static struct miscdevice bq2202a_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bq2202a_batt_id",
	.fops = &bq2202a_fops,
};



static int __init bq2202a_dev_init(void)
{
	int err;
    int rc;
	
	err = misc_register(&bq2202a_device);
	if (err)
		{
		   printk("bq2202a register error !\n");
		   return err;
		}
	
	rc = gpio_request(BATT_ID_GPIO, "gpio_batt");
	
	if (rc <0) {
		printk("bq2202a failed to request gpio_batt_en =%d\n",rc);
		return rc;
	}
	
	gpio_request_ok = 1;
	
	gpio_tlmm_config(BQ2202A_GPIO_CONFIG_OUTPUT,GPIO_CFG_ENABLE);

	batt_legal = Bq2022a_Identification_Check();

	return 0;
}

static void  __exit bq2202a_dev_exit(void)
{
	misc_deregister(&bq2202a_device);
}

module_init(bq2202a_dev_init);
module_exit(bq2202a_dev_exit);

