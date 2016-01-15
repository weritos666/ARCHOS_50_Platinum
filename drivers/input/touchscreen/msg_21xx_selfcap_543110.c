
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/gpio.h>

#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <mach/gpio.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <asm/unistd.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/earlysuspend.h>
//#include <mach/ty_gpio.h> //gaohw
#include <linux/input.h>


//-------------------------------------------------------------------
//-------------------------------------------------------------------
#define TYQ_TP_ANDROID4_0_SUPPORT

#if defined(CONFIG_TOUCHSCREEN_FT5X06) && defined(CONFIG_TY_TOUCHSCREEN_MSTAR) 
extern unsigned char ft_probe_flag;
#endif
#define TYN_VIRTAUL_KEY_FRAMEWORK     //add by gaohw
#define u8         unsigned char
#define u32        unsigned int
#define s32        signed int

#define REPORT_PACKET_LENGTH    (8)
#define TY_TP_INT 40
#define TY_TP_RST 26

#define MSG21XX_INT_GPIO       (TY_TP_INT)         //gaohw
#define MSG21XX_RESET_GPIO     (TY_TP_RST)    //gaohw

#define MS_TS_MSG21XX_X_MAX   (480)
#define MS_TS_MSG21XX_Y_MAX   (800)

/*#define	TOUCH_KEY_HOME	      (102)
#define	TOUCH_KEY_MENU        (139)
#define	TOUCH_KEY_BACK        (158)
*/



//Tianyu ADD
#define	TYN_TOUCH_FMUPGRADE 0x6501
#define	TYN_TOUCH_FWVER     (TYN_TOUCH_FMUPGRADE +1 )
#define   TYN_TOUCH_VENDOR   (TYN_TOUCH_FMUPGRADE +2 )
#define	NV_TOUCH_FT
#define TYN_TOUCH_FOCALTECH    1

struct ty_touch_fmupgrade_S
{
  unsigned int	touchType;
  unsigned long ulCRC;
  unsigned long ulReserved;
  unsigned long bufLength;
  void* 		bufAddr;
};
static struct touch_ctrl_dev
{
  dev_t devno;
  struct cdev cdev;
  struct class *class;
}ty_touch_ctrl;




static int msg21xx_irq = 0;
static struct i2c_client     *msg21xx_i2c_client;
static struct work_struct    msg21xx_wq;



#ifdef CONFIG_HAS_EARLYSUSPEND                      //add by gaohw
static struct early_suspend  early_suspend;
#endif
static struct input_dev     *input = NULL;
/*********start::add by gaohw***********/
struct mstar_tp_irq_status {
	volatile unsigned long irqen;
};
static struct mstar_tp_irq_status mstar_tp_status;
/**********end:: add by gaohw********/
struct msg21xx_ts_data
{
	uint16_t            addr;
	struct i2c_client  *client;
	struct input_dev   *input_dev;
	int                 use_irq;
	struct work_struct  work;
	int (*power)(int on);
	int (*get_int_status)(void);
	void (*reset_ic)(void);
};


#define MAX_TOUCH_FINGER 2
typedef struct
{
    u16 X;
    u16 Y;
} TouchPoint_t;

typedef struct
{
    u8 nTouchKeyMode;
    u8 nTouchKeyCode;
    u8 nFingerNum;
    TouchPoint_t Point[MAX_TOUCH_FINGER];
} TouchScreenInfo_t;

//---------------------------------------------------------------------------
#define	__FIRMWARE_UPDATE__
#ifdef __FIRMWARE_UPDATE__
/*#if defined(TYQ_TBW5913B_SUPPORT)
#define AUTO_UPDATE
#endif
*/
#ifdef AUTO_UPDATE
static struct work_struct    msg21xx_wq_gao;
#endif

#define FW_ADDR_MSG21XX   (0xC4>>1)
#define FW_ADDR_MSG21XX_TP   (0x4C>>1)
#define FW_UPDATE_ADDR_MSG21XX   (0x92>>1)
#define TP_DEBUG	printk
static  char *fw_version;

static unsigned short major_version;
static unsigned short minor_version;
#ifdef AUTO_UPDATE
static unsigned short fm_major_version;
static unsigned short fm_minor_version;
#endif
static u8 temp[94][1024];
static int FwDataCnt;
struct class *firmware_class;
struct device *firmware_cmd_dev;

static int touch_ctrl_init(void *touch);

static void HalTscrCReadI2CSeq(u8 addr, u8* read_data, u8 size)
{
   //according to your platform.
   	int rc;

	struct i2c_msg msgs[] =
    {
		{
			.addr = addr,
			.flags = I2C_M_RD,
			.len = size,
			.buf = read_data,
		},
	};

	rc = i2c_transfer(msg21xx_i2c_client->adapter, msgs, 1);
	if( rc < 0 )
    {
		printk("HalTscrCReadI2CSeq error %d\n", rc);
	}
}

static void HalTscrCDevWriteI2CSeq(u8 addr, u8* data, u16 size)
{
    //according to your platform.
   	int rc;

	struct i2c_msg msgs[] =
    {
		{
			.addr = addr,
			.flags = 0,
			.len = size,
			.buf = data,
		},
	};
	rc = i2c_transfer(msg21xx_i2c_client->adapter, msgs, 1);
	if( rc < 0 )
    {
		printk("HalTscrCDevWriteI2CSeq error %d,addr = %d\n", rc,addr);
	}
}

static void dbbusDWIICEnterSerialDebugMode(void)
{
    u8 data[5];

    // Enter the Serial Debug Mode
    data[0] = 0x53;
    data[1] = 0x45;
    data[2] = 0x52;
    data[3] = 0x44;
    data[4] = 0x42;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 5);
}

static void dbbusDWIICStopMCU(void)
{
    u8 data[1];

    // Stop the MCU
    data[0] = 0x37;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICIICUseBus(void)
{
    u8 data[1];

    // IIC Use Bus
    data[0] = 0x35;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICIICReshape(void)
{
    u8 data[1];

    // IIC Re-shape
    data[0] = 0x71;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICIICNotUseBus(void)
{
    u8 data[1];

    // IIC Not Use Bus
    data[0] = 0x34;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICNotStopMCU(void)
{
    u8 data[1];

    // Not Stop the MCU
    data[0] = 0x36;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);
}

static void dbbusDWIICExitSerialDebugMode(void)
{
    u8 data[1];

    // Exit the Serial Debug Mode
    data[0] = 0x45;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, data, 1);

    // Delay some interval to guard the next transaction
    //udelay ( 200 );        // delay about 0.2ms
}

static void drvISP_EntryIspMode(void)
{
    u8 bWriteData[5] =
    {
        0x4D, 0x53, 0x54, 0x41, 0x52
    };

    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 5);
}

static u8 drvISP_Read(u8 n, u8* pDataToRead)    //First it needs send 0x11 to notify we want to get flash data back.
{
    u8 Read_cmd = 0x11;
    unsigned char dbbus_rx_data[2] = {0};
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &Read_cmd, 1);
    udelay(100);        // delay about 100us*****
    if (n == 1)
    {
        HalTscrCReadI2CSeq(FW_UPDATE_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
        *pDataToRead = dbbus_rx_data[0];
    }
    else
    {
        HalTscrCReadI2CSeq(FW_UPDATE_ADDR_MSG21XX, pDataToRead, n);
    }

    return 0;
}

static void drvISP_WriteEnable(void)
{
    u8 bWriteData[2] =
    {
        0x10, 0x06
    };
    u8 bWriteData1 = 0x12;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
}


static void drvISP_ExitIspMode(void)
{
    u8 bWriteData = 0x24;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData, 1);
}

static u8 drvISP_ReadStatus(void)
{
    u8 bReadData = 0;
    u8 bWriteData[2] =
    {
        0x10, 0x05
    };
    u8 bWriteData1 = 0x12;

    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
    udelay(100);        // delay about 100us*****
    drvISP_Read(1, &bReadData);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
    return bReadData;
}


static void drvISP_ChipErase(void)
{
    u8 bWriteData[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    u8 bWriteData1 = 0x12;
		u32 timeOutCount=0;
    drvISP_WriteEnable();

    //Enable write status register
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x50;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);

    //Write Status
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x01;
    bWriteData[2] = 0x00;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 3);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);

    //Write disable
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x04;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
		udelay(100);        // delay about 100us*****
    timeOutCount=0;
		while ( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
		{
		   timeOutCount++;
	     if ( timeOutCount >= 100000 ) break; /* around 1 sec timeout */
	  }
    drvISP_WriteEnable();

    bWriteData[0] = 0x10;
    bWriteData[1] = 0xC7;

  	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
		udelay(100);        // delay about 100us*****
		timeOutCount=0;
		while ( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
		{
		   timeOutCount++;
	     if ( timeOutCount >= 500000 ) break; /* around 5 sec timeout */
	  }
}

static void drvISP_Program(u16 k, u8* pDataToWrite)
{
    u16 i = 0;
    u16 j = 0;
    //u16 n = 0;
    u8 TX_data[133];
    u8 bWriteData1 = 0x12;
    u32 addr = k * 1024;
		u32 timeOutCount=0;
    for (j = 0; j < 8; j++)   //128*8 cycle
    {
        TX_data[0] = 0x10;
        TX_data[1] = 0x02;// Page Program CMD
        TX_data[2] = (addr + 128 * j) >> 16;
        TX_data[3] = (addr + 128 * j) >> 8;
        TX_data[4] = (addr + 128 * j);
        for (i = 0; i < 128; i++)
        {
            TX_data[5 + i] = pDataToWrite[j * 128 + i];
        }
        udelay(100);        // delay about 100us*****
       
        timeOutCount=0;
				while ( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
				{
		   			timeOutCount++;
	     			if ( timeOutCount >= 100000 ) break; /* around 1 sec timeout */
	  		}
        
        
        
        drvISP_WriteEnable();
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, TX_data, 133);   //write 133 byte per cycle
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);
    }
}


static void drvISP_Verify(u16 k, u8* pDataToVerify)
{
    u16 i = 0, j = 0;
    u8 bWriteData[5] =
    {
        0x10, 0x03, 0, 0, 0
    };
    u8 RX_data[256];
    u8 bWriteData1 = 0x12;
    u32 addr = k * 1024;
    u8 index=0;
    u32 timeOutCount;
    for (j = 0; j < 8; j++)   //128*8 cycle
    {
        bWriteData[2] = (u8)((addr + j * 128) >> 16);
        bWriteData[3] = (u8)((addr + j * 128) >> 8);
        bWriteData[4] = (u8)(addr + j * 128);
        udelay(100);        // delay about 100us*****
        
        
        timeOutCount=0;
				while ( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
				{
		   		timeOutCount++;
	     		if ( timeOutCount >= 100000 ) break; /* around 1 sec timeout */
	  		}
        
        
        
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 5);    //write read flash addr
        udelay(100);        // delay about 100us*****
        drvISP_Read(128, RX_data);
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);    //cmd end
        for (i = 0; i < 128; i++)   //log out if verify error
        {
        if((RX_data[i]!=0)&&index<10)
		{
        //TP_DEBUG("j=%d,RX_data[%d]=0x%x\n",j,i,RX_data[i]);
        index++;
		}
            if (RX_data[i] != pDataToVerify[128 * j + i])
            {
                TP_DEBUG("k=%d,j=%d,i=%d===============Update Firmware Error================",k,j,i);
            }
        }
    }
}

static ssize_t firmware_update_show(struct device *dev,
                                    struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", fw_version);
}

static void _HalTscrHWReset(void)
{
	// gpio_request(MSG21XX_RESET_GPIO, "reset");           //add by gaohw
	  gpio_direction_output(MSG21XX_RESET_GPIO, 1);
	  gpio_set_value(MSG21XX_RESET_GPIO, 1);
	  gpio_set_value(MSG21XX_RESET_GPIO, 0);
    mdelay(10);  /* Note that the RST must be in LOW 10ms at least */
	  gpio_set_value(MSG21XX_RESET_GPIO, 1);
    /* Enable the interrupt service thread/routine for INT after 50ms */
    mdelay(50);
//	gpio_free(MSG21XX_RESET_GPIO);
}
static ssize_t firmware_update_store(struct device *dev,
                                     struct device_attribute *attr, const char *buf, size_t size)
{
    u8 i;
    u8 dbbus_tx_data[4];
    unsigned char dbbus_rx_data[2] = {0};


		_HalTscrHWReset();
		//1.Erase TP Flash first
	  dbbusDWIICEnterSerialDebugMode();
		dbbusDWIICStopMCU();
		dbbusDWIICIICUseBus();
		dbbusDWIICIICReshape();
		mdelay(300);
		
				
		// Disable the Watchdog
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x3C;
		dbbus_tx_data[2] = 0x60;
		dbbus_tx_data[3] = 0x55;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x3C;
		dbbus_tx_data[2] = 0x61;
		dbbus_tx_data[3] = 0xAA;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	  //Stop MCU
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x0F;
		dbbus_tx_data[2] = 0xE6;
		dbbus_tx_data[3] = 0x01;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	  //set FRO to 50M
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x11;
    dbbus_tx_data[2] = 0xE2;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
    dbbus_rx_data[0] = 0;
    dbbus_rx_data[1] = 0;
    HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
    TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
    dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;  //Clear Bit 3
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);



		//set MCU clock,SPI clock =FRO
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x22;
		dbbus_tx_data[2] = 0x00;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
		
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x23;
		dbbus_tx_data[2] = 0x00;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

		
	  // Enable slave's ISP ECO mode
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x08;
		dbbus_tx_data[2] = 0x0c;
		dbbus_tx_data[3] = 0x08;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	  //Enable SPI Pad
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x02;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
		HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
		TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
		dbbus_tx_data[3] = (dbbus_rx_data[0] | 0x20);  //Set Bit 5
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	  //WP overwrite
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x0E;
		dbbus_tx_data[3] = 0x02;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	  //set pin high
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x10;
		dbbus_tx_data[3] = 0x08;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

		dbbusDWIICIICNotUseBus();
		dbbusDWIICNotStopMCU();
		dbbusDWIICExitSerialDebugMode();
		
		
		
    drvISP_EntryIspMode();
    drvISP_ChipErase();
    _HalTscrHWReset();
    mdelay(300);
    
    //2.Program and Verify
    dbbusDWIICEnterSerialDebugMode();
		dbbusDWIICStopMCU();
		dbbusDWIICIICUseBus();
		dbbusDWIICIICReshape();


		
		// Disable the Watchdog
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x3C;
		dbbus_tx_data[2] = 0x60;
		dbbus_tx_data[3] = 0x55;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x3C;
		dbbus_tx_data[2] = 0x61;
		dbbus_tx_data[3] = 0xAA;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	  //Stop MCU
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x0F;
		dbbus_tx_data[2] = 0xE6;
		dbbus_tx_data[3] = 0x01;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	  //set FRO to 50M
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x11;
    dbbus_tx_data[2] = 0xE2;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
    dbbus_rx_data[0] = 0;
    dbbus_rx_data[1] = 0;
    HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
    TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
    dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;  //Clear Bit 3
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);



		//set MCU clock,SPI clock =FRO
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x22;
		dbbus_tx_data[2] = 0x00;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
		
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x23;
		dbbus_tx_data[2] = 0x00;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	  // Enable slave's ISP ECO mode
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x08;
		dbbus_tx_data[2] = 0x0c;
		dbbus_tx_data[3] = 0x08;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
		
	  //Enable SPI Pad
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x02;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
		HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
		TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
		dbbus_tx_data[3] = (dbbus_rx_data[0] | 0x20);  //Set Bit 5
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	  //WP overwrite
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x0E;
		dbbus_tx_data[3] = 0x02;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	  //set pin high
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x10;
		dbbus_tx_data[3] = 0x08;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

		dbbusDWIICIICNotUseBus();
		dbbusDWIICNotStopMCU();
		dbbusDWIICExitSerialDebugMode();

    ///////////////////////////////////////
    // Start to load firmware
    ///////////////////////////////////////
    drvISP_EntryIspMode();

    for (i = 0; i < 94; i++)   // total  94 KB : 1 byte per R/W
    {
        drvISP_Program(i, temp[i]);    // program to slave's flash
        drvISP_Verify ( i, temp[i] ); //verify data
    }
		TP_DEBUG("update OK\n");
    drvISP_ExitIspMode();
    FwDataCnt = 0;
    return size;
}

static DEVICE_ATTR(update, 0744, firmware_update_show, firmware_update_store);


/*test=================*/
static ssize_t firmware_clear_show(struct device *dev,
                                    struct device_attribute *attr, char *buf)
{
    u16 k=0,i = 0, j = 0;
    u8 bWriteData[5] =
    {
        0x10, 0x03, 0, 0, 0
    };
    u8 RX_data[256];
    u8 bWriteData1 = 0x12;
	u32 addr = 0;
		u32 timeOutCount=0;
	for (k = 0; k < 94; i++)   // total  94 KB : 1 byte per R/W
   {
      addr = k * 1024;
    for (j = 0; j < 8; j++)   //128*8 cycle
    {
        bWriteData[2] = (u8)((addr + j * 128) >> 16);
        bWriteData[3] = (u8)((addr + j * 128) >> 8);
        bWriteData[4] = (u8)(addr + j * 128);
        udelay(100);        // delay about 100us*****

        timeOutCount=0;
				while ( ( drvISP_ReadStatus() & 0x01 ) == 0x01 )
				{
		   		timeOutCount++;
	     		if ( timeOutCount >= 100000 ) break; /* around 1 sec timeout */
	  		}
        
        
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, bWriteData, 5);    //write read flash addr
        udelay(100);        // delay about 100us*****
        drvISP_Read(128, RX_data);
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG21XX, &bWriteData1, 1);    //cmd end
        for (i = 0; i < 128; i++)   //log out if verify error
        {
            if (RX_data[i] != 0xFF)
            {
                TP_DEBUG("k=%d,j=%d,i=%d===============erase not clean================",k,j,i);
            }
        }
    }
	}
   TP_DEBUG("read finish\n");
    return sprintf(buf, "%s\n", fw_version);
}

static ssize_t firmware_clear_store(struct device *dev,
                                     struct device_attribute *attr, const char *buf, size_t size)
{

		u8 dbbus_tx_data[4];
		unsigned char dbbus_rx_data[2] = {0};

		_HalTscrHWReset();
		dbbusDWIICEnterSerialDebugMode();
		dbbusDWIICStopMCU();
		dbbusDWIICIICUseBus();
		dbbusDWIICIICReshape();


		
		// Disable the Watchdog
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x3C;
		dbbus_tx_data[2] = 0x60;
		dbbus_tx_data[3] = 0x55;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x3C;
		dbbus_tx_data[2] = 0x61;
		dbbus_tx_data[3] = 0xAA;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

	  //Stop MCU
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x0F;
		dbbus_tx_data[2] = 0xE6;
		dbbus_tx_data[3] = 0x01;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	  //set FRO to 50M
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x11;
    dbbus_tx_data[2] = 0xE2;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
    dbbus_rx_data[0] = 0;
    dbbus_rx_data[1] = 0;
    HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
    TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
    dbbus_tx_data[3] = dbbus_rx_data[0] & 0xF7;  //Clear Bit 3
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);



		//set MCU clock,SPI clock =FRO
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x22;
		dbbus_tx_data[2] = 0x00;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
		
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x23;
		dbbus_tx_data[2] = 0x00;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

		
	  // Enable slave's ISP ECO mode
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x08;
		dbbus_tx_data[2] = 0x0c;
		dbbus_tx_data[3] = 0x08;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);
	  //Enable SPI Pad
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x02;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 3);
		HalTscrCReadI2CSeq(FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2);
		TP_DEBUG("dbbus_rx_data[0]=0x%x", dbbus_rx_data[0]);
		dbbus_tx_data[3] = (dbbus_rx_data[0] | 0x20);  //Set Bit 5
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	  //WP overwrite
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x0E;
		dbbus_tx_data[3] = 0x02;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);


	  //set pin high
		dbbus_tx_data[0] = 0x10;
		dbbus_tx_data[1] = 0x1E;
		dbbus_tx_data[2] = 0x10;
		dbbus_tx_data[3] = 0x08;
		HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX, dbbus_tx_data, 4);

		dbbusDWIICIICNotUseBus();
		dbbusDWIICNotStopMCU();
		dbbusDWIICExitSerialDebugMode();


    ///////////////////////////////////////
    // Start to load firmware
    ///////////////////////////////////////
    drvISP_EntryIspMode();
	  TP_DEBUG("chip erase+\n");
    drvISP_ChipErase();
	  TP_DEBUG("chip erase-\n");
    drvISP_ExitIspMode();
    return size;
}

static DEVICE_ATTR(clear, 0744, firmware_clear_show, firmware_clear_store);

/*test=================*/
/*Add by Tracy.Lin for update touch panel firmware and get fw version*/

static ssize_t firmware_version_show(struct device *dev,
                                     struct device_attribute *attr, char *buf)
{
    TP_DEBUG("*** firmware_version_show fw_version = %s***\n", fw_version);
    return sprintf(buf, "%s\n", fw_version);
}

static ssize_t firmware_version_store(struct device *dev,
                                      struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned char dbbus_tx_data[3];
    unsigned char dbbus_rx_data[4] ;
    //unsigned short major=0, minor=0;

    fw_version = kzalloc(sizeof(char), GFP_KERNEL);
		//SM-BUS GET FW VERSION
    dbbus_tx_data[0] = 0x53;
    dbbus_tx_data[1] = 0x00;
    dbbus_tx_data[2] = 0x74;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX_TP, &dbbus_tx_data[0], 3);
    HalTscrCReadI2CSeq(FW_ADDR_MSG21XX_TP, &dbbus_rx_data[0], 4);

	major_version = (dbbus_rx_data[1]<<8)+dbbus_rx_data[0];
    minor_version= (dbbus_rx_data[3]<<8)+dbbus_rx_data[2];

    TP_DEBUG("***major = %d ***\n", major_version);
    TP_DEBUG("***minor = %d ***\n", minor_version);
    sprintf(fw_version,"%03d%03d", major_version, minor_version);
	
  /*  major = (dbbus_rx_data[1]<<8)+dbbus_rx_data[0];
    minor = (dbbus_rx_data[3]<<8)+dbbus_rx_data[2];

    TP_DEBUG("***major = %d ***\n", major);
    TP_DEBUG("***minor = %d ***\n", minor);
    sprintf(fw_version,"%03d%03d", major, minor);
    */
    TP_DEBUG("***fw_version = %s ***\n", fw_version);


    return size;
}
static DEVICE_ATTR(version, 0744, firmware_version_show, firmware_version_store);

static int fts_GetFWVer(unsigned long * pu32)
{
	unsigned char dbbus_tx_data[3];
    unsigned char dbbus_rx_data[4] ;
   // unsigned short major=0, minor=0;
		//SM-BUS GET FW VERSION
    dbbus_tx_data[0] = 0x53;
    dbbus_tx_data[1] = 0x00;
    dbbus_tx_data[2] = 0x74;
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG21XX_TP, &dbbus_tx_data[0], 3);
    HalTscrCReadI2CSeq(FW_ADDR_MSG21XX_TP, &dbbus_rx_data[0], 4);

    major_version= (dbbus_rx_data[1]<<8)+dbbus_rx_data[0];
    minor_version= (dbbus_rx_data[3]<<8)+dbbus_rx_data[2];

   *pu32 = (major_version<<16) | minor_version;
   //*pu32 = 0;
    return 0;
}


static ssize_t firmware_data_show(struct device *dev,
                                  struct device_attribute *attr, char *buf)
{
    return FwDataCnt;
}

static ssize_t firmware_data_store(struct device *dev,
                                   struct device_attribute *attr, const char *buf, size_t size)
{

    int i;
	TP_DEBUG("***FwDataCnt = %d ***\n", FwDataCnt);
    for (i = 0; i < 1024; i++)
    {
        memcpy(temp[FwDataCnt], buf, 1024);
    }
    FwDataCnt++;
    return size;
}
static DEVICE_ATTR(data, 0744, firmware_data_show, firmware_data_store);
#endif


///////////////////////////////////////////////////////////////////////

/*
sysfs file format:
key tyep:key value:key center x:key center y:key width:key height

#define TOUCH_Y_MAX 836
#define SCREEN_Y_MAX 800

*/


static ssize_t ty_touch_virtual_keys_show(struct kobject *kobj,
										  struct kobj_attribute *attr, char *buf)
{
		#if defined(TYQ_TBW5913_SUPPORT) || defined (TYQ_TBE5916_SUPPORT)
		return sprintf(buf, 
			#ifdef TYQ_TP_ANDROID4_0_SUPPORT
			__stringify(EV_KEY) ":" __stringify(KEY_HOMEPAGE) ":48:530:40:40"
			#else				   
				   __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":48:530:40:40"
			#endif
			   ":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":120:530:40:40"
			   ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":190:530:40:40"
			   ":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":290:530:40:40"
			   "\n");	
	   #else
		return sprintf(buf,	
			#ifdef TYQ_TP_ANDROID4_0_SUPPORT
			__stringify(EV_KEY) ":" __stringify(KEY_HOMEPAGE) ":50:863:100:80"
			#else
		       __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":50:863:100:80"
		       #endif
		       ":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":175:863:100:80"
		       ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":300:863:100:80"
		       ":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":420:863:80:80"				   
		       "\n");
		#endif	

}

static struct kobj_attribute ty_touch_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.ms-msg21xx",
		.mode = S_IRUGO,
	},
	.show = &ty_touch_virtual_keys_show,
};


//**********************************************************************//
static struct attribute *ty_touch_properties_attrs[] = {
	&ty_touch_virtual_keys_attr.attr,
	NULL
};

static struct attribute_group ty_touch_properties_attr_group = {
	.attrs = ty_touch_properties_attrs,
};

//**********************************************************************//

static int fts_creat_virtual_key_sysfs(void)
{
	struct kobject *properties_kobj;
	int ret = 0;

	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj)
		ret = sysfs_create_group(properties_kobj,
								 &ty_touch_properties_attr_group);
	if (!properties_kobj || ret)
		printk("failed to create board_properties\n");

	return ret;
}

////////////////////////////////////////////////////////////////////////////




//---------------------------------------------------------------------------
static void msg21xx_suspend(struct early_suspend *h)
{
	printk("Enter %s \n",__func__);
	if((msg21xx_irq)&&(test_and_clear_bit(0,&mstar_tp_status.irqen)))                //gaohw
		disable_irq_nosync(msg21xx_irq);
		
	cancel_work_sync(&msg21xx_wq);
	//gpio_request(MSG21XX_RESET_GPIO, "reset");   //add by gaohw
	gpio_direction_output(MSG21XX_RESET_GPIO, 0);   //add by gaohw
	gpio_set_value(MSG21XX_RESET_GPIO, 0);
	//gpio_free(MSG21XX_RESET_GPIO);         //add by gaohw
	input_mt_sync(input);
	input_sync(input);
	
	printk("TP exit %s \n",__func__);
}

static void msg21xx_resume(struct early_suspend *h)
{

	printk("Enter %s \n",__func__);
	
	/*input_report_abs(input, ABS_MT_TOUCH_MAJOR, 1);
	input_report_abs(input, ABS_MT_POSITION_X, 3000);
	input_report_abs(input, ABS_MT_POSITION_Y, 3000);
	input_mt_sync(input);
	input_sync(input);
	*/
	#if 1//defined(TYQ_TBW5913B_SUPPORT) || defined(TYQ_TBW5913C_SUPPORT) 
	//gpio_request(MSG21XX_RESET_GPIO, "reset");   //add by gaohw
	gpio_direction_output(MSG21XX_RESET_GPIO, 1);   //add by gaohw
	/*TY:niuli insert 2 msleep(5) to add delay time when system resume, 
	so lcd will not white background when resume on 2012*/
	msleep(5);
	gpio_set_value(MSG21XX_RESET_GPIO, 1);
	msleep(5);
	//gpio_free(MSG21XX_RESET_GPIO);         //add by gaohw
	#else
	gpio_direction_input(MSG21XX_RESET_GPIO);
	/*TY:niuli insert 2 msleep(5) to add delay time when system resume, 
	so lcd will not white background when resume on 2012*/
	msleep(10);
	#endif
	if((msg21xx_irq)&&(!test_and_set_bit(0,&mstar_tp_status.irqen)))          //add by  gaohw
		enable_irq(msg21xx_irq);
}
//#if defined(CONFIG_TOUCHSCREEN_FT5X06) && defined(CONFIG_TY_TOUCHSCREEN_MSTAR) 

//#else
static void msg21xx_chip_init(void)
{
	#if 1 //defined(TYQ_TBW5913B_SUPPORT) || defined(TYQ_TBW5913C_SUPPORT)
    /* After the LCD is on, power on the TP controller */
	//gpio_request(MSG21XX_RESET_GPIO, "reset");   //add by gaohw
	gpio_direction_output(MSG21XX_RESET_GPIO, 1);
	gpio_set_value(MSG21XX_RESET_GPIO, 1);
	gpio_set_value(MSG21XX_RESET_GPIO, 0);
    mdelay(200);  /* Note that the RST must be in LOW 10ms at least */
	gpio_set_value(MSG21XX_RESET_GPIO, 1);
    /* Enable the interrupt service thread/routine for INT after 50ms */
    mdelay(50);
	//gpio_free(MSG21XX_RESET_GPIO);         //add by gaohw
	#else
	/* After the LCD is on, power on the TP controller */
	//gpio_request(MSG21XX_RESET_GPIO, "reset");   //add by gaohw
	//gpio_direction_output(MSG21XX_RESET_GPIO, 1);
	gpio_direction_output(MSG21XX_RESET_GPIO, 0);
	//gpio_set_value(MSG21XX_RESET_GPIO, 1);
	gpio_set_value(MSG21XX_RESET_GPIO, 0);
    mdelay(200);  /* Note that the RST must be in LOW 10ms at least */
	//gpio_set_value(MSG21XX_RESET_GPIO, 1);
	gpio_direction_input(MSG21XX_RESET_GPIO);
    /* Enable the interrupt service thread/routine for INT after 50ms */
    mdelay(50);
	//gpio_free(MSG21XX_RESET_GPIO);         //add by gaohw
	#endif
}
//#endif

static u8 Calculate_8BitsChecksum( u8 *msg, s32 s32Length )
{
    s32 s32Checksum = 0;
    s32 i;

    for ( i = 0 ; i < s32Length; i++ )
    {
        s32Checksum += msg[i];
    }

    return (u8)( ( -s32Checksum ) & 0xFF );
}


#ifdef AUTO_UPDATE
static void msg21xx_data_disposal_gao(struct work_struct *work)
{
	int i =0;
	TP_DEBUG("current fw_version is %s,wrong version !! \n",fw_version);
	
	for(i=0;i<94;i++)
 	{
 		firmware_data_store(firmware_cmd_dev,0,MSG_2133_BIN+(i*1024),(size_t)i);		
 	}
 	 firmware_update_store(firmware_cmd_dev,0,0,(size_t)i);

}
#endif


static void msg21xx_data_disposal(struct work_struct *work)
{
    u8 val[8] = {0};
    u8 Checksum = 0;
	u8 i;
    u32 delta_x = 0, delta_y = 0;
    u32 u32X = 0;
    u32 u32Y = 0;
	u8 touchkeycode = 0;
	TouchScreenInfo_t *touchData=NULL;
	static u32 preKeyStatus=0;
	//static u32 preFingerNum=0;

/*#define SWAP_X_Y   (1)
#ifdef SWAP_X_Y
    int tempx;
    int tempy;
#endif
*/
	touchData = kzalloc(sizeof(TouchScreenInfo_t), GFP_KERNEL);

	/* tianyu quhl add 2012-10-19 */
	if(touchData==NULL)
	{
		printk("=====%s:request memory failed!!!!=====\n",__func__);
		return;
	}
    memset(touchData, 0, sizeof(TouchScreenInfo_t));

    //_TRACE((TSCR_LEVEL_C_TYP, "DrvTscrCGetData in"));
    i2c_master_recv(msg21xx_i2c_client,&val[0],REPORT_PACKET_LENGTH);
    Checksum = Calculate_8BitsChecksum(&val[0], 7); //calculate checksum

	
    //printk("primary:: val[0]=%d,val[1]=%d,val[2]=%d,val[3]=%d,val[4]=%d,val[5]=%d,val[6]=%d,val[7]=%d\n",val[0],val[1],val[2],val[3],val[4],val[5],val[6],val[7]);
    if ((Checksum == val[7]) && (val[0] == 0x52))   //check the checksum  of packet
    {
        u32X = (((val[1] & 0xF0) << 4) | val[2]);         //parse the packet to coordinates
        u32Y = (((val[1] & 0x0F) << 8) | val[3]);

        delta_x = (((val[4] & 0xF0) << 4) | val[5]);
        delta_y = (((val[4] & 0x0F) << 8) | val[6]);
  //  printk("primary:: u32X=%d,u32Y=%d\n",u32X,u32Y);
/*    #ifdef SWAP_X_Y
		tempy = u32X;
		tempx = u32Y;
        u32X = tempx;
        u32Y = tempy;

		tempy = delta_x;
		tempx = delta_y;
        delta_x = tempx;
        delta_y = tempy;
	#endif
*/
        //DBG("[HAL] u32X = %x, u32Y = %x", u32X, u32Y);
        //DBG("[HAL] delta_x = %x, delta_y = %x", delta_x, delta_y);

        if ((val[1] == 0xFF) && (val[2] == 0xFF) && (val[3] == 0xFF) && (val[4] == 0xFF) && (val[6] == 0xFF))
        {
            touchData->Point[0].X = 0; // final X coordinate
            touchData->Point[0].Y = 0; // final Y coordinate

          /*  if (val[5] != 0x0)
            {
                touchData->nTouchKeyMode = 1; //TouchKeyMode
				touchData->nTouchKeyCode = val[5]; //TouchKeyCode
                touchData->nFingerNum = 1;
            }
            else
            {
                touchData->nFingerNum = 0; //touch end
                touchData->nTouchKeyCode = 0; //TouchKeyMode
                touchData->nTouchKeyMode = 0; //TouchKeyMode
            }*/

	    if((val[5]==0x0)||(val[5]==0xFF))
            {
                touchData->nFingerNum = 0; //touch end
                touchData->nTouchKeyCode = 0; //TouchKeyMode
                touchData->nTouchKeyMode = 0; //TouchKeyMode
            }
            else
            {
                touchData->nTouchKeyMode = 1; //TouchKeyMode
                touchData->nTouchKeyCode = val[5]; //TouchKeyCode
                touchData->nFingerNum = 1;
		 // printk("&&&&&&&&useful key code report touch key code = %d\n",touchData->nTouchKeyCode);		
            }

		  
        }
		else
		{
		    touchData->nTouchKeyMode = 0; //Touch on screen...

			if ((delta_x == 0) && (delta_y == 0))
			{
				touchData->nFingerNum = 1; //one touch
				touchData->Point[0].X = (u32X * MS_TS_MSG21XX_X_MAX) / 2048;
				touchData->Point[0].Y = (u32Y * MS_TS_MSG21XX_Y_MAX ) / 2048;//1781;

				/* Calibrate if the touch panel was reversed in Y */
	#if 0
				touchData->Point[0].Y = (MS_TS_MSG21XX_Y_MAX - touchData->Point[0].Y);
	#endif
			}
			else
			{
				u32 x2, y2;

				touchData->nFingerNum = 2; //two touch

				/* Finger 1 */
				touchData->Point[0].X = (u32X * MS_TS_MSG21XX_X_MAX) / 2048;
				touchData->Point[0].Y = (u32Y * MS_TS_MSG21XX_Y_MAX ) / 2048;//1781;

				/* Finger 2 */
				if (delta_x > 2048)     //transform the unsigh value to sign value
				{
					delta_x -= 4096;
				}
				if (delta_y > 2048)
				{
					delta_y -= 4096;
				}

				x2 = (u32)(u32X + delta_x);
				y2 = (u32)(u32Y + delta_y);

				touchData->Point[1].X = (x2 * MS_TS_MSG21XX_X_MAX) / 2048;
				touchData->Point[1].Y = (y2 * MS_TS_MSG21XX_Y_MAX ) /2048;// 1781;

				/* Calibrate if the touch panel was reversed in Y */
	#if 0
				touchData->Point[1].Y = (MS_TS_MSG21XX_Y_MAX - y2);
	#endif
			}
		}

		//report...
		if(touchData->nTouchKeyMode)
		{
			
			if (touchData->nTouchKeyCode == 1)
				#ifdef TYQ_TP_ANDROID4_0_SUPPORT
				touchkeycode = KEY_HOMEPAGE;
				#else
				touchkeycode = KEY_HOME;
				#endif
			if (touchData->nTouchKeyCode == 2)
				touchkeycode = KEY_MENU;
			if (touchData->nTouchKeyCode == 4)
				touchkeycode = KEY_BACK;
			if (touchData->nTouchKeyCode == 8)
				touchkeycode = KEY_SEARCH;
			
			if(preKeyStatus!=touchkeycode)
			{
				preKeyStatus=touchkeycode;
				//input_report_key(input, touchkeycode, 1);
				//printk("&&&&&&&&useful key code report touch key code = %d\n",touchkeycode);
					#ifdef TYQ_TP_ANDROID4_0_SUPPORT
					if(touchkeycode == KEY_HOMEPAGE)
					#else
					if(touchkeycode == KEY_HOME)
					#endif
					{
						//input_report_abs(input, ABS_MT_TOUCH_MAJOR, 1);
						input_report_abs(input, ABS_MT_POSITION_X, 50);
						input_report_abs(input, ABS_MT_POSITION_Y, 863);
						input_mt_sync(input);
				
					}

					if(touchkeycode == KEY_MENU)
					{
					//	input_report_abs(input, ABS_MT_TOUCH_MAJOR, 1);
						input_report_abs(input, ABS_MT_POSITION_X, 175);
						input_report_abs(input, ABS_MT_POSITION_Y, 863);
						input_mt_sync(input);
				
					}

					if(touchkeycode == KEY_BACK)
					{
					//	input_report_abs(input, ABS_MT_TOUCH_MAJOR, 1);
						input_report_abs(input, ABS_MT_POSITION_X, 300);
						input_report_abs(input, ABS_MT_POSITION_Y, 863);
						input_mt_sync(input);
				
					}

					if(touchkeycode == KEY_SEARCH)
					{
					//	input_report_abs(input, ABS_MT_TOUCH_MAJOR, 1);
						input_report_abs(input, ABS_MT_POSITION_X, 420);
						input_report_abs(input, ABS_MT_POSITION_Y, 863);
						input_mt_sync(input);
				
					}
					
					
			}
			
			input_sync(input);
		}
        else
        {
		    preKeyStatus=0; //clear key status..

            if((touchData->nFingerNum) == 0)   //touch end
            {
				//preFingerNum=0;
			/*	input_report_key(input, KEY_MENU, 0);
				input_report_key(input, KEY_HOME, 0);
				input_report_key(input, KEY_BACK, 0);
				input_report_key(input, KEY_SEARCH, 0);
			*/
			//	input_report_abs(input, ABS_MT_TOUCH_MAJOR, 0);
				input_mt_sync(input);
				input_sync(input);
            }
            else //touch on screen
            {
			    /*
				if(preFingerNum!=touchData->nFingerNum)   //for one touch <--> two touch issue
				{
					printk("langwenlong number has changed\n");
					preFingerNum=touchData->nFingerNum;
					input_report_abs(input, ABS_MT_TOUCH_MAJOR, 0);
				    input_mt_sync(input);
				    input_sync(input);
				}*/

				for(i = 0;i < (touchData->nFingerNum);i++)
				{
				//	input_report_abs(input, ABS_MT_TOUCH_MAJOR, 1);
					input_report_abs(input, ABS_MT_POSITION_X, touchData->Point[i].X);
					input_report_abs(input, ABS_MT_POSITION_Y, touchData->Point[i].Y);
					input_mt_sync(input);
				//	printk("X=%d,Y=%d\n",touchData->Point[i].X,touchData->Point[i].Y);
				}

				input_sync(input);
			}
		}
    }
    else
    {
        //DBG("Packet error 0x%x, 0x%x, 0x%x", val[0], val[1], val[2]);
        //DBG("             0x%x, 0x%x, 0x%x", val[3], val[4], val[5]);
        //DBG("             0x%x, 0x%x, 0x%x", val[6], val[7], Checksum);
		printk(KERN_ERR "err status in tp\n");
    }
	if((msg21xx_irq)&&(!test_and_set_bit(0,&mstar_tp_status.irqen)))          //add by  gaohw
		enable_irq(msg21xx_irq);
   // enable_irq(msg21xx_irq);

	//tianyu quhl add 2012-10-19
	if(touchData)
	{
		kfree(touchData);
	}
}



static int msg21xx_ts_open(struct input_dev *dev)
{
	return 0;
}

static void msg21xx_ts_close(struct input_dev *dev)
{
	printk("msg21xx_ts_close\n");
}


static int msg21xx_init_input(void)
{
	int err;

	input = input_allocate_device();
	input->name = msg21xx_i2c_client->name;
	input->phys = "I2C";
	input->id.bustype = BUS_I2C;
	input->dev.parent = &msg21xx_i2c_client->dev;
	input->open = msg21xx_ts_open;
	input->close = msg21xx_ts_close;

	set_bit(EV_ABS, input->evbit);
	set_bit(EV_SYN, input->evbit);
	
	#ifdef TYQ_TP_ANDROID4_0_SUPPORT
	set_bit(INPUT_PROP_DIRECT,input->propbit);
	#else
	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_TOUCH, input->keybit);
	#endif
	set_bit(KEY_BACK, input->keybit);
	set_bit(KEY_MENU, input->keybit);
	#ifdef TYQ_TP_ANDROID4_0_SUPPORT
		set_bit(KEY_HOMEPAGE, input->keybit);
	#else
		set_bit(KEY_HOME, input->keybit);
	#endif
	set_bit(KEY_SEARCH, input->keybit);
	
//	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, 1, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_X, 0, MS_TS_MSG21XX_X_MAX, 0, 0);
	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, MS_TS_MSG21XX_Y_MAX, 0, 0);

	err = input_register_device(input);
	if (err)
		goto fail_alloc_input;

fail_alloc_input:
	return 0;
}
static irqreturn_t msg21xx_interrupt(int irq, void *dev_id)
{
	if((msg21xx_irq)&&(test_and_clear_bit(0,&mstar_tp_status.irqen)))          //gaohw
		disable_irq_nosync(msg21xx_irq);
	schedule_work(&msg21xx_wq);
	return IRQ_HANDLED;
}


static int __devinit msg21xx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int  err = 0;
	#ifdef AUTO_UPDATE
	int i = 0;
	#endif
	#if defined(CONFIG_TOUCHSCREEN_FT5X06) && defined(CONFIG_TY_TOUCHSCREEN_MSTAR) 
	u8 val[9] = {0};
	if(ft_probe_flag ==1)
		return -1;
	#endif
	
	msg21xx_i2c_client = client;
	gpio_request(MSG21XX_RESET_GPIO, "reset");   //add by gaohw
	gpio_request(MSG21XX_INT_GPIO, "interrupt");
//	gpio_request(MSG21XX_RESET_GPIO, "reset");
	gpio_direction_input(MSG21XX_INT_GPIO);
	//gpio_free(MSG21XX_INT_GPIO);         //add by gaohw
//	gpio_set_value(MSG21XX_INT_GPIO, 1);
//#if defined(CONFIG_TOUCHSCREEN_FT5X06) && defined(CONFIG_TY_TOUCHSCREEN_MSTAR) 
 // tp have been reseted  in board-7x27a.c 
//#else
	msg21xx_chip_init();
//#endif
	#if defined(CONFIG_TOUCHSCREEN_FT5X06) && defined(CONFIG_TY_TOUCHSCREEN_MSTAR) 
	//msleep(200);
	//printk("********************msg21xx_probe********************************\n");
	err = i2c_master_recv(msg21xx_i2c_client,&val[0],REPORT_PACKET_LENGTH);
	if(err < 0)
	{
		printk("Mstar tp detected failure!\n");
		goto exit0;
	}
	#endif
	INIT_WORK(&msg21xx_wq, msg21xx_data_disposal);
	
	#ifdef AUTO_UPDATE
	INIT_WORK(&msg21xx_wq_gao, msg21xx_data_disposal_gao);
	#endif
	
	msg21xx_init_input();
	msg21xx_irq = client->irq;   //gaohw
	set_bit(0,&mstar_tp_status.irqen); //add by gaohw
	err = request_irq(msg21xx_irq, msg21xx_interrupt,
							IRQF_TRIGGER_RISING, "msg21xx", NULL);          //gaohw no modify
	if (err != 0) {
		printk("%s: cannot register irq\n", __func__);
		goto exit;
	}
	#ifdef CONFIG_HAS_EARLYSUSPEND
	early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN +1;              //gao +1?
	early_suspend.suspend = msg21xx_suspend;
	early_suspend.resume = msg21xx_resume;
	register_early_suspend(&early_suspend);
	#endif

#ifdef TYN_VIRTAUL_KEY_FRAMEWORK
	fts_creat_virtual_key_sysfs();               //add by gaohw
#endif
	
	/*frameware upgrade*/
#ifdef __FIRMWARE_UPDATE__
	touch_ctrl_init(0);
	firmware_class = class_create(THIS_MODULE, "ms-touchscreen-msg20xx");
    if (IS_ERR(firmware_class))
        pr_err("Failed to create class(firmware)!\n");
    firmware_cmd_dev = device_create(firmware_class,
                                     NULL, 0, NULL, "device");
    if (IS_ERR(firmware_cmd_dev))
        pr_err("Failed to create device(firmware_cmd_dev)!\n");

    // version
    if (device_create_file(firmware_cmd_dev, &dev_attr_version) < 0)
        pr_err("Failed to create device file(%s)!\n", dev_attr_version.attr.name);
    // update
    if (device_create_file(firmware_cmd_dev, &dev_attr_update) < 0)
        pr_err("Failed to create device file(%s)!\n", dev_attr_update.attr.name);
    // data
    if (device_create_file(firmware_cmd_dev, &dev_attr_data) < 0)
        pr_err("Failed to create device file(%s)!\n", dev_attr_data.attr.name);
	// clear
    if (device_create_file(firmware_cmd_dev, &dev_attr_clear) < 0)
        pr_err("Failed to create device file(%s)!\n", dev_attr_clear.attr.name);

	dev_set_drvdata(firmware_cmd_dev, NULL);
#endif

	#ifdef AUTO_UPDATE
  #ifndef __MSG_2133_BIN__
  	#error "please include firmware array"
  #else
  fm_major_version = MSG_2133_BIN[0x3077] + (MSG_2133_BIN[0x3076] << 8);
  fm_minor_version = MSG_2133_BIN[0x3075] + (MSG_2133_BIN[0x3074] << 8);
  mdelay(100);
 firmware_version_store(firmware_cmd_dev,0,0,(size_t)i);
 printk("fw_version =%s  major_version =%d  minor_version =%d fm_major_version =%d fm_minor_version=%d\n", fw_version,major_version,minor_version,fm_major_version,fm_minor_version);
  if((major_version == fm_major_version) &&(minor_version < fm_minor_version)) 
// if(fw_version[2]!=0x31 && fw_version[4]!=0x31 && fw_version[5]!=0x31)
 {
 	schedule_work(&msg21xx_wq_gao);
 	/*TP_DEBUG("current fw_version is %s,wrong version !! \n",fw_version);
	for(i=0;i<94;i++)
 	{
 		firmware_data_store(firmware_cmd_dev,0,MSG_2133_BIN+(i*1024),(size_t)i);		
 	}
 	 firmware_update_store(firmware_cmd_dev,0,0,(size_t)i);
 	*/
 }
 else
 {
	TP_DEBUG("firmware version check okay \n"); 		
 }
 
 //mdelay(10000);
 	#endif
#endif

	return 0;

#if defined(CONFIG_TOUCHSCREEN_FT5X06) && defined(CONFIG_TY_TOUCHSCREEN_MSTAR) 
exit0:
	 gpio_free(MSG21XX_RESET_GPIO);
	 gpio_free(MSG21XX_INT_GPIO);
#endif	 

exit:
	return err;
}


/*******************************start irmware updae control*******************************/

static int touch_ctrl_open(struct inode *inode, struct file *filp) 
{ 
  printk("%s\n", __func__); 
  return 0;   
} 
 
static ssize_t touch_ctrl_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) 
{		  
  printk("%s, calibration %s %d\n", __func__, buf, count);	
  
  //NvOdmTouchSetAutoCalibration(touch_context->hTouchDevice, FTS_TRUE);
  return 0;
} 
static ssize_t touch_ctrl_read (struct file *file, char *buf, size_t count, loff_t *offset)
{
  printk("touch_ctrl_read calibration\n"); 
  return 0; 
}

static long touch_ctrl_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
	struct ty_touch_fmupgrade_S  bufarg;
	int iret;
	unsigned char *pBuf;
	u8 bRet = 0;
	unsigned long  ulVer;
	//unsigned long i;
	int mVer;

	if (0 == arg)
	{
		printk("%s:arg null pointer.\n",__func__);
		return -EFAULT;
	}
	
	if (copy_from_user(&bufarg, (void *) arg, sizeof(bufarg))) 
	{
		printk("%s:null pointer.\n",__func__);
		return -EFAULT;
	}

	printk("%s:cmd=0x%x.\n",__func__,cmd);
	
	switch (cmd) 
	{
		case TYN_TOUCH_FWVER:
			printk("%s:getver.\n",__func__);
			if (NULL == bufarg.bufAddr)
			{
				printk("%s:bufAddr null pointer.\n",__func__);
				return -EFAULT;
			}
			printk("%s:getver 1.\n",__func__);
			
			if ((0>= bufarg.bufLength )||( sizeof(unsigned long) < bufarg.bufLength ))
			{
				printk("%s:bufLength null pointer.\n",__func__);
				return -EFAULT;
			}			
			//printk("%s:lenth=%x,addr=0x%x\n",__func__,(unsigned int)bufarg.bufLength,(bufarg.bufAddr));
			printk("%s:getver 2\n",__func__);
			
			pBuf = kzalloc(bufarg.bufLength, GFP_KERNEL);
			if (0 == pBuf)
			{
				printk("%s:alloc buffer failed.\n",__func__);
				return 0;
			}

			if (copy_from_user(pBuf, (void *)bufarg.bufAddr, bufarg.bufLength)) 
			{
				printk("%s:get buffer error.\n",__func__);
				return -EFAULT;
			}
			printk("%s:getver 3.\n",__func__);

			
			ulVer = 0;
			
			//get firmware version
	#if defined(NV_TOUCH_FT)
			mdelay(200);
			bRet = fts_GetFWVer(&ulVer);
			if(0 > bRet) 
			{
				*(unsigned long*)bufarg.bufAddr = 0;
				printk("%s:FT_GetFWVer failed. \n",__func__);
				kfree(pBuf);
				return -EFAULT;
			}
	#endif

			*(( unsigned long *)bufarg.bufAddr) = ulVer;
			
			//printk("%s:ver=0x%x \n",__func__,*(unsigned long*)bufarg.bufAddr);
			if( pBuf ) kfree(pBuf);
			
			break;
			
		case TYN_TOUCH_FMUPGRADE:
			if (0 == bufarg.bufAddr)
			{
				printk("%s:null pointer.\n",__func__);
				return 0;
			}
			
			pBuf =&temp[0][0]; //kzalloc(bufarg.bufLength, GFP_KERNEL);
			if (0 == pBuf)
			{
				printk("%s:alloc buffer failed.\n",__func__);
				return 0;
			}
			
			if (copy_from_user(pBuf, (void *) bufarg.bufAddr, bufarg.bufLength)) 
			{
				printk("%s:get buffer error.\n",__func__);
				return -EAGAIN;
			}
			mdelay(200);
			bRet = fts_GetFWVer(&ulVer);
			if(0 > bRet) 
			{
				printk("%s:FT_GetFWVer failed. \n",__func__);
				return -EFAULT;
			}

			if((ulVer >> 16) != (pBuf[0x3077] + (pBuf[0x3076] << 8)))
			{
				printk("%s:major_version  do not match. \n",__func__);
				return -EFAULT;
			}
			
			//get crc and check crc 	  
			//
			
			//printk("%s:type=0x%x,len=0x%x \n",__func__,bufarg.touchType,bufarg.bufLength);
			//printk("ver=0x%x,0x%x,0x%x \n",pBuf[bufarg.bufLength-3],pBuf[bufarg.bufLength-2],pBuf[bufarg.bufLength-1]);
			
     #if defined(NV_TOUCH_FT)
			if( TYN_TOUCH_FOCALTECH == bufarg.touchType)
			{
				iret = firmware_update_store(0,0,0,0);//fts_ctpm_fw_upgrade_with_i_file(pBuf,bufarg.bufLength);
				if(iret)
				{
					printk("update focaltech firmware failed.\n");
				}
				//kfree(pBuf);
				return iret;
			}
    #endif
		//	if( pBuf ) kfree(pBuf);
			break;
		case TYN_TOUCH_VENDOR:
			ulVer = 0;
			mdelay(200);
			bRet = fts_GetFWVer(&ulVer);
			if(0 > bRet) 
			{
				printk("%s:FT_GetFWVer failed. \n",__func__);
				return -EFAULT;
			}

			mVer=ulVer>>16;
			printk("====get touch vendor :%d,%ld=========\n",mVer,ulVer);

			if((ulVer >> 16) == 6)
			{
				#if defined(TYQ_TBE5916_SUPPORT)||defined(TYQ_TBW5913_SUPPORT)
				if(copy_to_user(bufarg.bufAddr,"BYD_tp(ic:msg2133)",strlen("BYD_tp(ic:msg2133)")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
				#else
				if(copy_to_user(bufarg.bufAddr,"truly_tp(ic:msg2133)",strlen("truly_tp(ic:msg2133)")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
				#endif
			}
			else if((ulVer >> 16) == 1)
			{
				#if defined(TYQ_TBE5916_SUPPORT)||defined(TYQ_TBW5913_SUPPORT)
				if(copy_to_user(bufarg.bufAddr,"truly_tp(ic:msg2133)",strlen("truly_tp(ic:msg2133)")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
				#else
				if(copy_to_user(bufarg.bufAddr,"unknown(ic:msg2133)",strlen("unknown(ic:msg2133)")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
				#endif
			}
			else if((ulVer >> 16) == 7 )
			{	
				#if defined(TYQ_TBE5916_SUPPORT)||defined(TYQ_TBW5913_SUPPORT)
				if(copy_to_user(bufarg.bufAddr,"truly_tp(ic:msg2133)",strlen("truly_tp(ic:msg2133)")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
				#else
				if(copy_to_user(bufarg.bufAddr,"unknown(ic:msg2133)",strlen("unknown(ic:msg2133)")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
				#endif
			}
			else if((ulVer >> 16) == 4 )
			{	
				if(copy_to_user(bufarg.bufAddr,"truly_tp(ic:msg2133)",strlen("truly_tp(ic:msg2133)")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			else if((ulVer >> 16) == 5)
			{
				if(copy_to_user(bufarg.bufAddr,"lcetron_tp(ic:msg2133)",strlen("lcetron_tp(ic:msg2133)")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			else
			{
				if(copy_to_user(bufarg.bufAddr,"unknown(ic:msg2133)",strlen("unknown(ic:msg2133)")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			break;
		default:
			printk("%s:Invalid command.\n",__func__);
			
	}
	
	return 0;
}

static int touch_ctrl_close(struct inode *inode, struct file *filp) 
{ 
  printk("%s\n", __func__); 
  return 0; 
} 

struct file_operations ty_touch_ctrl_fops_msg = 
{ 
	.write = touch_ctrl_write,
	.read = touch_ctrl_read, 
	.open = touch_ctrl_open, 
	.release = touch_ctrl_close, 
	.unlocked_ioctl	 = touch_ctrl_ioctl,
}; 

static int touch_ctrl_init(void *touch) 
{ 
  int ret = 0; 
  struct device *fts_chr_device;

  ret = alloc_chrdev_region(&ty_touch_ctrl.devno, 0, 1, "touch_ctrl"); 
  if(ret)
  { 
	printk("%s, can't alloc chrdev\n", __func__); 
  } 
  printk("%s, register chrdev(%d, %d)\n", __func__, MAJOR(ty_touch_ctrl.devno), MINOR(ty_touch_ctrl.devno)); 
		 
  cdev_init(&ty_touch_ctrl.cdev, &ty_touch_ctrl_fops_msg);
 
  ty_touch_ctrl.cdev.owner = THIS_MODULE; 
  ty_touch_ctrl.cdev.ops = &ty_touch_ctrl_fops_msg;
  
  ret = cdev_add(&ty_touch_ctrl.cdev, ty_touch_ctrl.devno, 1); 
  if(ret < 0)
  { 
	printk("%s, add char devive error, ret %d\n", __func__, ret); 
  } 

  ty_touch_ctrl.class = class_create(THIS_MODULE, "fts_touch_ctrl"); 
  if(IS_ERR(ty_touch_ctrl.class)){ 
	printk("%s, creating class error\n", __func__); 
  } 

  fts_chr_device=device_create(ty_touch_ctrl.class, NULL, ty_touch_ctrl.devno, NULL, "fts_fw_entry");
  if(NULL == fts_chr_device){
	printk("%s, create device, error\n", __func__);
	class_destroy(ty_touch_ctrl.class);
  }

  return (int)touch; 
} 


/*******************************end irmware updae*******************************/


static int __devexit msg21xx_remove(struct i2c_client *client)
{
	return 0;
}



static const struct i2c_device_id msg21xx_id[] = {
	{ "ms-msg21xx", 0x26 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, msg21xx_id);


static struct i2c_driver msg21xx_driver = {
	.driver = {
		   .name = "ms-msg21xx",
		   .owner = THIS_MODULE,
	},
	.probe = msg21xx_probe,
	.remove = __devexit_p(msg21xx_remove),
	.id_table = msg21xx_id,
};





static int __init msg21xx_init(void)
{
	int err;
	err = i2c_add_driver(&msg21xx_driver);
	if (err) {
		printk(KERN_WARNING "msg21xx  driver failed "
		       "(errno = %d)\n", err);
	} else {
		printk( "Successfully added driver %s\n",
		          msg21xx_driver.driver.name);
	}
	return err;
}

static void __exit msg21xx_cleanup(void)
{
	i2c_del_driver(&msg21xx_driver);
}


module_init(msg21xx_init);
module_exit(msg21xx_cleanup);

MODULE_AUTHOR("Mstar semiconductor");
MODULE_DESCRIPTION("Driver for msg21xx Touchscreen Controller");
MODULE_LICENSE("GPL");

