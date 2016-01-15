/* Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */


#include <linux/delay.h>
#include <linux/module.h>
//#include <mach/gpio.h>
#include <mach/pmic.h>
#include "msm_fb.h"
/*tydrv qupw add for gp_clk gpio27  as  pwm output  start*/
#include <linux/clk.h>
/*tydrv qupw add for gp_clk gpio27  as  pwm output  end*/

#include <linux/hrtimer.h>

/*tydrv niuli add for keypad light misc control on 20111117*/
#include <linux/miscdevice.h>
#include <linux/gpio.h>

static int spi_cs;
static int spi_clk;
static int spi_sdo;
static int spi_sdi;
static int lcd_reset;

/*ty:thomasyang add config gpio status for read lcd ic begin 20120606*/
#ifdef TYQ_LCDC_COMP_SUPPORT
static unsigned int lcddrvic_id;
#else
static unsigned int lcddrvic_id;
#endif
/*ty:thomasyang add config gpio status for read lcd ic end 20120606*/
/*ty:niuli add config gpio status for lcd_id pin begin 20120625*/
#ifdef TYQ_LCDC_COMP_SUPPORT
#define TYQ_LCDC_NO_CURRENT	1
#else
#define TYQ_LCDC_NO_CURRENT	0
#endif
/*ty:niuli add config gpio status for lcd_id pin end 20120625*/

/*pengwei added for press powerkey to light on screen abnormal*/
#ifdef TYQ_BACKLIGHT_POWERKEY_ON_FLAG
int ty_bl_flag;
int ty_bl_level;
#endif


#define CS(a)  gpio_set_value(spi_cs, a)
#define SCLK(a)  gpio_set_value(spi_clk, a)
#define SDI(a)  gpio_set_value(spi_sdo, a)
#define SDO      gpio_get_value(spi_sdi)

/*pengwei added for tow kinds of timing*/
#define LCD_SPI_TIMING_SUPPORT
/*pengwei added for tow kinds of timing*/

/*tydrv qupw add for gp_clk gpio27  as  pwm output  start*/

#define DRV_BACKLIGHT_CTL_RT9293B_SUPPORT 1
/*tydrv qupw add for gp_clk gpio27  as  pwm output  end*/

/*tydrv niuli add for sleep in&out on 20111115  begin*/
#define DRV_WAKE_IN_AND_OUT 1
/*tydrv niuli add for sleep in&out on 20111115  end*/

/*tydrv niuli add for sleep in&out on 20111116  begin*/
#define DRV_BACKLIGHT_CTL_RT9293B_LEVEL 1
/*tydrv niuli add for sleep in&out on 20111116  end*/

#define DRV_KPDLIGHT_CTL_PMU_SUPPORT  1
/*tydrv niuli add for keypad light misc control on 20111117  begin*/
#define DRV_KPDLIGHT_MISC_IOCTL 1
/*tydrv niuli add ffor keypad light misc control on 20111117  end*/

#define DRV_SPI_COMMAND_FAST  1

#define OTM8018B_CHIP_ID	0x40
#define NT35510_CHIP_ID		0xB0
#define HX8369B_CHIP_ID     0x00

struct truly_state_type{
	boolean disp_initialized;
	boolean display_on;
	boolean disp_powered_up;
};

static struct truly_state_type truly_state = { 0 };
static struct msm_panel_common_pdata *lcdc_truly_pdata;

/*tydrv qupw add for gp_clk gpio27  as  pwm output  start*/

#if DRV_BACKLIGHT_CTL_RT9293B_SUPPORT

struct ty_bl_cntx{
    unsigned int   bl_pre_level;
    unsigned int   bl_level;
    unsigned int   bl_duty_num;
    /*concurrency*/
};


static struct ty_bl_cntx blpwm_cntx;

static struct clk *gp_clkto_pwm; /* drives the gp_clk pwm  interface */


/*pwm =37.5hz  duty_num  N=256,  if duty set to 50% duty =N/2=128; BUT PARA= 2XD,SO duty=50%,duty_num=256*/
#if 0
static void rt9293b_bl_init(void)
{

  gpio_set_value(109,1);  // enable RT9293

}
#endif
/*niuli add DRV_BACKLIGHT_CTL_RT9293B_LEVEL for BACKLIGHT level begin on 20111116*/
#ifdef DRV_BACKLIGHT_CTL_RT9293B_LEVEL
void backlight_onoff(int level)
{
	if (IS_ERR(gp_clkto_pwm)) {
		printk(KERN_ERR "Couldnt find gp_clk\n");
		return;
	}

	if(level==0)
	{
	   gpio_set_value(109,0);	
	}
	else
	{
		if(truly_state.display_on)
		{
		/*niuli add for level brigntenss max=256*/
			if(level<33)
				blpwm_cntx.bl_duty_num = blpwm_cntx.bl_level*1;/*TYDRV qupw for level enchange*/
			else {
				blpwm_cntx.bl_duty_num = blpwm_cntx.bl_level*2 -32;/*TYDRV niuli for 90% brightness*/
			}
//	blpwm_cntx.bl_duty_num = 255- blpwm_cntx.bl_level*4+2;/*TYDRV qupw for level enchange*/
			/*thomasyang add for level brigntenss is not enough*/
			blpwm_cntx.bl_duty_num += 20;
			clk_set_rate(gp_clkto_pwm, blpwm_cntx.bl_duty_num);
			if(blpwm_cntx.bl_pre_level == 0)
			{
				gpio_set_value(109,1);	
			}
		}
	}
}
static void RT9293B_Level_Setting(int level)
{
    // int ret = 0;
	 
	// printk("  truly set level=%d\n",level);
    blpwm_cntx.bl_level = level;
    if(blpwm_cntx.bl_level>128)
   	return;

    if(blpwm_cntx.bl_level == blpwm_cntx.bl_pre_level)
     return;

	backlight_onoff(level);

	printk(" truly set level=%d\n",level);


	
    blpwm_cntx.bl_pre_level = blpwm_cntx.bl_level;


}
#else
static void RT9293B_Level_Setting(int level)
{
     int ret;

	 
	// printk(" truly set level=%d\n",level);
    blpwm_cntx.bl_level = level;

    if(blpwm_cntx.bl_level == blpwm_cntx.bl_pre_level)
     return;

	
	if (IS_ERR(gp_clkto_pwm)) {
		printk(KERN_ERR "Couldnt find gp_clk\n");
		return;
	}

	if (blpwm_cntx.bl_level==0)
		{
		 gpio_set_value(109,0);	// disable RT9293
		
		}	
	else
		{
		 if ( blpwm_cntx.bl_pre_level==0)
		     gpio_set_value(109,1);	//enable RT9293
	    }
	blpwm_cntx.bl_duty_num = 256- blpwm_cntx.bl_level;/*TYDRV qupw for level enchange*/

	ret = clk_set_rate(gp_clkto_pwm,blpwm_cntx.bl_duty_num);
	
	
	if (ret) {
		printk("+++RT9293B_Level_Setting:fail\n");
		return;
	}
      

    blpwm_cntx.bl_pre_level = blpwm_cntx.bl_level;


}
#endif
/*niuli add DRV_BACKLIGHT_CTL_RT9293B_LEVEL for BACKLIGHT level end on 20111116*/
#endif 

/*tydrv qupw add for gp_clk gpio27  as  pwm output  end*/


#ifdef DRV_KPDLIGHT_CTL_PMU_SUPPORT

#define KPDBL_CTRL_OFF_STATE  0
#define KPDBL_CTRL_ON_STATE   1
#define MAX_KEYPAD_BL_LEVEL  16

static unsigned int kpd_bl_cur_state = KPDBL_CTRL_OFF_STATE;
static unsigned int kpd_enable_count = 0;

static void kpd_bl_ctrl(unsigned int onoff)
{
    int ret,value;

    //printk("kpd_bl_ctrl:onoff[%d],kpd_bl_cur_state[%d]\n",onoff,kpd_bl_cur_state);
    if(onoff)
        kpd_enable_count++;

    if((kpd_bl_cur_state == KPDBL_CTRL_OFF_STATE)&&(onoff == 0))
        return;
    else if((kpd_bl_cur_state == KPDBL_CTRL_ON_STATE)&&(onoff != 0)){
        /*kernel start: That firstly set backlight failed because the rpc server don't ready.
          but during kernel start,Will set twice backlight,so let the second setting valid*/
        if(kpd_enable_count > 2)
           return;
    }

    if(onoff){
        value = 127;
        kpd_bl_cur_state = KPDBL_CTRL_ON_STATE;
    }
    else{
        value = 0;
        kpd_bl_cur_state = KPDBL_CTRL_OFF_STATE;
    }

/*tyqupw modify for tbw5912 key led use mpp8, notice: bp also modify  be use  pm_mpp_config_i_sink*/
if (value)
	ret=pmic_secure_mpp_config_i_sink(PM_MPP_8,PM_MPP__I_SINK__LEVEL_5mA,PM_MPP__I_SINK__SWITCH_ENA);
else
	ret=pmic_secure_mpp_config_i_sink(PM_MPP_8,PM_MPP__I_SINK__LEVEL_5mA,PM_MPP__I_SINK__SWITCH_DIS);

    //ret = pmic_set_led_intensity(LED_KEYPAD, value / MAX_KEYPAD_BL_LEVEL);
   if (ret)
	   printk("kpd_bl_ctrl led fail ret=%d\n",ret);

    if(kpd_enable_count > 2)
        kpd_enable_count = 0x0ff;

   return;

}

#endif

/*tydrv niuli add for keypad light misc control on 20111117*/
#if DRV_KPDLIGHT_MISC_IOCTL
static struct platform_device *truly_misc_data;

#define TRULYIO 0x1f
#define TRULY_IOCTL_KPD_SET _IOW(TRULYIO, 0x01, int)
#define TRULY_IOCTL_GET_LCD_ID _IOR(TRULYIO, 0x02, char[11])

 static int truly_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}
static long
truly_ioctl(struct file *file, unsigned int cmd,
		   unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int ret ;
	unsigned int kpd_state=0;
	char buf[20];
	char *str1 = "truly_lcd OTM8018B";
	char *str2 ="tianma_lcd_hx8369";
	switch (cmd) {
		case TRULY_IOCTL_KPD_SET://set keppad light 	
			if (copy_from_user(&kpd_state, argp, sizeof(kpd_state))){
				ret = -EFAULT;
				break;
			}
			printk(KERN_INFO "truly_ioctl:--->>>>TRULY_IOCTL_KPD_SET  kpd_state=%d\r\n",kpd_state);	
			if(kpd_state){
				kpd_bl_ctrl(1);
			}
			else{
				kpd_bl_ctrl(0);
			}
				
			break;
		case TRULY_IOCTL_GET_LCD_ID:// get lcdc information
			printk("truly_ioctl:--->>>>get lcdc information lcddrvic_id=%d\n", lcddrvic_id);
			if(lcddrvic_id == OTM8018B_CHIP_ID) {
				strcpy(buf, str1);
			}
			else
				{
				strcpy(buf,str2);
				}
			if(copy_to_user(argp, buf, strlen(buf))) {
				printk("ty_lcd:TRULY_IOCTL_GET_LCD_ID read failed \n");
				return -EFAULT;
			}
			break;
		default:
			break;	
		}
	return ret;
}

static struct file_operations truly_fops = {
	.owner = THIS_MODULE,
	.open = truly_open,
	.unlocked_ioctl = truly_ioctl,
};

static struct miscdevice truly_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "truly_misc_device",
	.fops = &truly_fops,
};
#endif


void WMLCDCOM_HX( unsigned short command)
{
    unsigned short i; 
    CS(0);
    udelay(10);
    SCLK(1);
    SDI(1);
    udelay(100);
    ///Write Command
    SCLK(0);
    SDI(0);
    udelay(100);
    SCLK(1);
    udelay(100);
        /////// 
    for(i=0;i<8;i++)
    {
        SCLK(0);   
        if(command&0x0080)
            SDI(1);    
        else
            SDI(0);  
        udelay(100);
        SCLK(1);
        udelay(100); 
        //SCLK(0);
        command<<=1;
    }
    CS(1);
}

void WMLCDDATA_HX(unsigned short data)   
{
    unsigned short i; 
    CS(0);
    udelay(10);
    SCLK(1);
    SDI(1);
    udelay(100);
    //Write Data
    SCLK(0);
    SDI(1);
    udelay(100);
    SCLK(1);
    udelay(100);
    //////
    for(i=0;i<8;i++)
    {
        SCLK(0);   
        if(data&0x0080)
            SDI(1);    
        else
            SDI(0);  
        udelay(100);
        SCLK(1);
        udelay(100); 
        //SCLK(0);
        data<<=1;
    }
    CS(1);
}



#ifdef DRV_SPI_COMMAND_FAST
void WMLCDCOM_OTM( unsigned short command)
{

//**************
//the 1st byte
//**************
 unsigned short i,MB=0x20;
	// MB=MB|data;
    CS(1);
    SCLK(0);
    SDI(0);
    CS(0);
    
    for(i=0;i<8;i++)
    {
    SCLK(0);	  
    if(MB&0x80)
    SDI(1);    
    else
    SDI(0);
    
    SCLK(1);
    //SCLK(0);
    MB<<=1;
    }
    
    MB=command;
    
    for(i=0;i<8;i++)
    {
    SCLK(0);	  
    if(MB&0x8000)
    SDI(1);    
    else
    SDI(0);
    
    SCLK(1);
    MB<<=1;
    }
	
    CS(1);
    SCLK(0);
    //**************
    //the 2nd byte
    //**************
    CS(0);
    MB=0x00;
    
    for(i=0;i<8;i++)
    {
    SCLK(0);	  
    if(MB&0x80)
    SDI(1);    
    else
    SDI(0);
    
    SCLK(1);
    MB<<=1;
    }
    
    MB=command<<8;
    
    for(i=0;i<8;i++)
    {
    SCLK(0);	  
    if(MB&0x8000)
    SDI(1);    
    else
    SDI(0);
    
    SCLK(1);
    MB<<=1;
    }
	
    CS(1);
    SCLK(0);

}

void WMLCDDATA_OTM( unsigned short data)
{

	unsigned short i,MB=0x40;
//	MB=MB|data;
	
      CS(1);
      //Delayus(10);
      SCLK(0);
      SDI(0);
      CS(0);
      
      
      for(i=0;i<8;i++)
      {
      SCLK(0);	  
      if(MB&0x80)
      SDI(1);    
      else
      SDI(0);
      
      SCLK(1);
      MB<<=1;
      }
      
      MB=data;
      for(i=0;i<8;i++)
      {
      SCLK(0);	  
      if(MB&0x80)
      SDI(1);    
      else
      SDI(0);
      
      SCLK(1);
      MB<<=1;
      }
      CS(1);
      SCLK(0);
      
}


unsigned short RMLCDCOM(unsigned short command)
{
	unsigned short i, MB, data;

	//**************
	//the 1st byte
	//**************
	MB = 0x20;

	CS(1);

	SCLK(0);
	SDI(0);
	CS(0);

	for (i=0; i<8; i++) {
		SCLK(0);

		if (MB&0x80) {
			SDI(1);
		} else {
			SDI(0);
		}

		SCLK(1);
		MB<<=1;
	}
	
	MB=command;
	
	for(i=0;i<8;i++) {
		SCLK(0);	  
	
		if(MB&0x8000) {
			SDI(1);
		} else {
			SDI(0);
		}

		SCLK(1);
		MB<<=1;
	}
	
	CS(1);
	SCLK(0);

	//**************
	//the 2nd byte
	//**************
	CS(0);
	MB=0x00;

	for (i=0; i<8; i++) {
		SCLK(0);

		if (MB&0x80) {
			SDI(1);
		} else {
			SDI(0);
		}

		SCLK(1);
		MB<<=1;
	}

	MB = command<<8;

	for(i=0;i<8;i++) {
		SCLK(0);	  
	
		if(MB&0x8000) {
			SDI(1);
		} else {
			SDI(0);
		}

		SCLK(1);
		MB<<=1;
	}

	CS(1);
	SCLK(0);

	//*******************
	//read register value
	//*******************
	CS(0);
	MB = 0xc0;
	for (i=0; i<8; i++) {
		SCLK(0);

		if (MB&0x80) {
			SDI(1);
		} else {
			SDI(0);
		}

		SCLK(1);
		MB<<=1;
	}

	MB = 0x00;
	for (i=8; i>0; i--) {
		SCLK(0);
		udelay(100);
		SCLK(1);
		if (SDO) {
			data = 1;
		} else {
			data = 0;
		}
		MB = MB | (data << (i-1));
	}

	return MB;	
}

#else
void WMLCDCOM_OTM( unsigned short command)
{

//**************
//the 1st byte
//**************
 unsigned short i,MB=0x20;
	// MB=MB|data;
	
    CS(1);
    udelay(10);
    SCLK(0);
    SDI(0);
    udelay(100);
    CS(0);
    udelay(100);
    
    for(i=0;i<8;i++)
    {
    SCLK(0);	  
    if(MB&0x80)
    SDI(1);    
    else
    SDI(0);
    
    udelay(100);
    SCLK(1);
    udelay(100);
    //SCLK(0);
    MB<<=1;
    }
    
    MB=command;
    
    for(i=0;i<8;i++)
    {
    SCLK(0);	  
    if(MB&0x8000)
    SDI(1);    
    else
    SDI(0);
    
    udelay(100);
    SCLK(1);
    udelay(100);		
    MB<<=1;
    }
	
    CS(1);
    SCLK(0);
    udelay(100);
    //**************
    //the 2nd byte
    //**************
    CS(0);
    udelay(100);
    MB=0x00;
    
    for(i=0;i<8;i++)
    {
    SCLK(0);	  
    if(MB&0x80)
    SDI(1);    
    else
    SDI(0);
    
    udelay(100);
    SCLK(1);
    udelay(100);
    MB<<=1;
    }
    
    MB=command<<8;
    
    for(i=0;i<8;i++)
    {
    SCLK(0);	  
    if(MB&0x8000)
    SDI(1);    
    else
    SDI(0);
    
    udelay(100);
    SCLK(1);
    udelay(100);
    MB<<=1;
    }
	
    CS(1);
    SCLK(0);
    udelay(100);
	
}


void WMLCDDATA_OTM( unsigned short data)
{


	unsigned short i,MB=0x40;
//	MB=MB|data;
	
      CS(1);
      //Delayus(10);
      SCLK(0);
      SDI(0);
      udelay(100);
      CS(0);
      udelay(100);
      
      
      for(i=0;i<8;i++)
      {
      SCLK(0);	  
      if(MB&0x80)
      SDI(1);    
      else
      SDI(0);
      
      udelay(100);
      SCLK(1);
      udelay(100);	
      MB<<=1;
      }
      
      MB=data;
      for(i=0;i<8;i++)
      {
      SCLK(0);	  
      if(MB&0x80)
      SDI(1);    
      else
      SDI(0);
      
      udelay(100);
      SCLK(1);
      udelay(100);	
      MB<<=1;
      }
      CS(1);
      SCLK(0);
      udelay(100);
      
}
#endif

/*pengwei added to separation tow write Timing*/
#ifdef LCD_SPI_TIMING_SUPPORT
void WMLCDCOM(unsigned short command)
{
#ifdef TYQ_LCDC_COMP_SUPPORT
	if(lcddrvic_id == OTM8018B_CHIP_ID)
		WMLCDCOM_OTM(command);
	else		
		WMLCDCOM_HX(command);
#else
		WMLCDCOM_OTM(command);
#endif
}


void WMLCDDATA(unsigned short command)
{
#ifdef TYQ_LCDC_COMP_SUPPORT
	if(lcddrvic_id == OTM8018B_CHIP_ID)
		WMLCDDATA_OTM(command);
	else		
		WMLCDDATA_HX(command);
#else
	WMLCDDATA_OTM(command);
#endif
}
#endif
/*pengwei added to separation tow write Timing*/

static void spi_init(void)
{

    /* Setting the Default GPIO's */
    spi_clk = *(lcdc_truly_pdata->gpio_num);   //97
    spi_cs   = *(lcdc_truly_pdata->gpio_num + 1);  //94
    spi_sdo  = *(lcdc_truly_pdata->gpio_num + 2);  //98
    spi_sdi  = 96; // hardware offer number
    lcd_reset = *(lcdc_truly_pdata->gpio_num + 3); //88

    /* Set the output so that we dont disturb the slave device */
    gpio_set_value(spi_sdo, 0);
    gpio_set_value(spi_clk, 1);

    /* Set the Chip Select De-asserted */
    gpio_set_value(spi_cs, 1);
	#if 0
	if(!truly_state.disp_initialized)
	{
	    gpio_set_value(lcd_reset, 1);
	    mdelay(20); /*tydrv  qupw delay 10MS for lcd reset*/
	    gpio_set_value(lcd_reset, 0);
	    mdelay(20); /*tydrv  qupw delay 10MS for lcd reset*/
	    gpio_set_value(lcd_reset, 1);
	    mdelay(2);
	}
	#endif

}

static void truly_disp_powerup(void)
{
	if (!truly_state.disp_powered_up && !truly_state.display_on) {
		/* Reset the hardware first */
		/* Include DAC power up implementation here */
	      truly_state.disp_powered_up = TRUE;
	}
}
#if defined(TYQ_LCDC_COMP_SUPPORT)
static void truly_hx8369b_disp_on(void)
{
	uint32	data;
printk("%s \n",__func__);
	printk("pengwei hx8369b_disp_on");
		if (!truly_state.disp_initialized) {
			printk("pengwei hx8369b_disp_on_init________#########");
		   // mdelay(10); 
	//NT35510+3.8AUO FOT TFT1N6287CODE 101015
		 CS(1);
		 SCLK(1);
		 SDI(1);
		 gpio_set_value(lcd_reset, 1);
		 mdelay(10);	
		 gpio_set_value(lcd_reset, 0);
		 mdelay(10);	
		 gpio_set_value(lcd_reset, 1);
		 mdelay(20);
	/*truly TFT1P6394(OTM8018B+3.97)CODE-120606*/

        WMLCDCOM(0xB9); 	// Set EXTC
		WMLCDDATA(0xFF);
		WMLCDDATA(0x83);
		WMLCDDATA(0x69);
		mdelay(5);
		

                WMLCDCOM(0x3A); 
		WMLCDDATA(0x70); 

                WMLCDCOM(0xB5);		//
		WMLCDDATA(0x0B);		//	VREF[4:0]  4.8V
		WMLCDDATA(0x0B);		//	NVREF[4:0] 4.8V
		WMLCDDATA(0x24);		//	FB_OFF, VR_TRIM[5:0]
		
                WMLCDCOM(0xCB);		//
		WMLCDDATA(0x6D);		//	VREF[4:0]  4.8V
		
                WMLCDCOM(0xC6);		//
		WMLCDDATA(0x41);		//	VREF[4:0]  4.8V
		WMLCDDATA(0xFF);		//	NVREF[4:0] 4.8V
		WMLCDDATA(0x7D);		//	FB_OFF, VR_TRIM[5:0]
		WMLCDDATA(0xFF);		//	
		
	 	WMLCDCOM(0xB1);		//
		WMLCDDATA(0x13);		//	BT[4:0]
		WMLCDDATA(0x83);		//	GAS_EN, AP[2:0]
		WMLCDDATA(0x77);		//	FS1[3:0], FS0[3:0]
		WMLCDDATA(0x00);		//	DT[1:0], DC_DIV[2:0]
		WMLCDDATA(0x90);		//	FB_ON, BTP[4:0]	 VSP=5.2V
		WMLCDDATA(0x10);		//	BTN[4:0]	VSN=5.2V
		WMLCDDATA(0x1F);		//	VRHP[5:0]	VSPR=4.6V
		WMLCDDATA(0x1F);		//	VRHN[5:0]	VSNR= -4.6V
		WMLCDDATA(0x0C);
		WMLCDDATA(0x0A);
			
	       
	 	WMLCDCOM(0xB3);		//SET RGB
		WMLCDDATA(0x83);		//SDO_EN, EPF[1:0], RES_SEL[4:0]=0x03 480x800
		WMLCDDATA(0x0E);		//[3:0]= DPL, HSPL, VSPL, EPL	 	HSPL, VSPL=0: low active, EPL=0 DE high enable
		WMLCDDATA(0x31);		//[7:0]= RCM, 0, HPL[5:0]
		WMLCDDATA(0x03);		//[7:0]= 00, VPL[5:0]
				
	 	WMLCDCOM(0xB4);		
		WMLCDDATA(0x02);				    
			
	 	WMLCDCOM(0xB6);		// SETVCOM
		WMLCDDATA(0xB0);		
		WMLCDDATA(0xB0);				 
        		
	 	WMLCDCOM(0xCC);		//
		WMLCDDATA(0x00);		//	REV_PANEL				
  		
WMLCDCOM(0xD5);		//GIP
WMLCDDATA(0x00); //1   PANSEL
WMLCDDATA(0x00); //2   SHR_0[11:8]
WMLCDDATA(0x04); //3   SHR_0[7:0]
WMLCDDATA(0x03); //4   SHR_1[11:8]
WMLCDDATA(0x31); //5   SHR_1[7:0]
WMLCDDATA(0x03); //6   SHR_2[11:8]
WMLCDDATA(0x35); //7   SHR_2[7:0]
WMLCDDATA(0x12); //8   SHR0_1[3:0], SHR0_2[3:0]
WMLCDDATA(0x01); //9   SHR0_3[3:0], SHR1_1[3:0]
WMLCDDATA(0x20); //10  SHR1_2[3:0], SHR1_3[3:0]
WMLCDDATA(0x12); //11  SHR2_1[3:0], SHR2_2[3:0]
WMLCDDATA(0x00); //12  SHR2_3[3:0]
WMLCDDATA(0x21); //13  SPON[7:0]
WMLCDDATA(0x60); //14  SPOFF[7:0]
WMLCDDATA(0x1B); //15  SHP[3:0], SCP[3:0]
WMLCDDATA(0x00); //16  GTO[5:0]
WMLCDDATA(0x00); //17  GNO[7:0]
WMLCDDATA(0x08); //18  CHR[7:0]
WMLCDDATA(0x20); //19  CON[7:0]
WMLCDDATA(0x3E); //20  COFF[7:0]
WMLCDDATA(0x13); //21  CHP[3:0], CCP[3:0]
WMLCDDATA(0x00); //22  EQ_DELAY[7:0]
WMLCDDATA(0x00); //23  EQ_DELAY_HSYNC[7:0]
WMLCDDATA(0x00); //24  CGTS[18:16]
WMLCDDATA(0xC3); //25  CGTS[15:8]
WMLCDDATA(0x00); //26  CGTS[7:0]
WMLCDDATA(0x00); //27  CGTS_INV[18:16]
WMLCDDATA(0x00); //28  CGTS_INV[15:8]
WMLCDDATA(0x00); //29  CGTS_INV[7:0]
WMLCDDATA(0x03); //30  CGTS_ZERO_1ST_FRAME[18:16]
WMLCDDATA(0x00); //31  CGTS_ZERO_1ST_FRAME[15:8]
WMLCDDATA(0x00); //32  CGTS_ZERO_1ST_FRAME[7:0]
WMLCDDATA(0x03); //33
WMLCDDATA(0x00); //34
WMLCDDATA(0x00); //35
WMLCDDATA(0x04); //36  USER_GIP_GATE[7:0]
WMLCDDATA(0x00); //37  COS1_L_GS[3:0], COS2_L_GS[3:0]
WMLCDDATA(0x00); //38  COS3_L_GS[3:0], COS4_L_GS[3:0]
WMLCDDATA(0x89); //39  COS5_L_GS[3:0], COS6_L_GS[3:0]
WMLCDDATA(0x00); //40  COS7_L_GS[3:0], COS8_L_GS[3:0]
WMLCDDATA(0xAA); //41  COS9_L_GS[3:0], COS10_L_GS[3:0]
WMLCDDATA(0x11); //42  COS11_L_GS[3:0], COS12_L_GS[3:0]
WMLCDDATA(0x33); //43  COS13_L_GS[3:0], COS14_L_GS[3:0]
WMLCDDATA(0x11); //44  COS15_L_GS[3:0], COS16_L_GS[3:0]
WMLCDDATA(0x00); //45  COS17_L_GS[3:0], COS18_L_GS[3:0]
WMLCDDATA(0x00); //46  COS1_L[3:0], COS2_L[3:0]
WMLCDDATA(0x00); //47  COS3_L[3:0], COS4_L[3:0]
WMLCDDATA(0x98); //48  COS5_L[3:0], COS6_L[3:0]
WMLCDDATA(0x00); //49  COS7_L[3:0], COS8_L[3:0]
WMLCDDATA(0x00); //50  COS9_L[3:0], COS10_L[3:0]
WMLCDDATA(0x22); //51  COS11_L[3:0], COS12_L[3:0]
WMLCDDATA(0x00); //52  COS13_L[3:0], COS14_L[3:0]
WMLCDDATA(0x44); //53  COS15_L[3:0], COS16_L[3:0]
WMLCDDATA(0x00); //54  COS17_L[3:0], COS18_L[3:0]
WMLCDDATA(0x00); //55  COS1_R_GS[3:0], COS2_R_GS[3:0]    
WMLCDDATA(0x00); //56  COS3_R_GS[3:0], COS4_R_GS[3:0]    
WMLCDDATA(0x89); //57  COS5_R_GS[3:0], COS6_R_GS[3:0]    
WMLCDDATA(0x00); //58  COS7_R_GS[3:0], COS8_R_GS[3:0]    
WMLCDDATA(0x99); //59  COS9_R_GS[3:0], COS10_R_GS[3:0]   
WMLCDDATA(0x00); //60  COS11_R_GS[3:0], COS12_R_GS[3:0]  
WMLCDDATA(0x22); //61  COS13_R_GS[3:0], COS14_R_GS[3:0]  
WMLCDDATA(0x00); //62  COS15_R_GS[3:0], COS16_R_GS[3:0]  
WMLCDDATA(0x00); //63  COS17_R_GS[3:0], COS18_R_GS[3:0]  
WMLCDDATA(0x00); //64  COS1_R[3:0], COS2_R[3:0]    
WMLCDDATA(0x00); //65  COS3_R[3:0], COS4_R[3:0]    
WMLCDDATA(0x98); //66  COS5_R[3:0], COS6_R[3:0]    
WMLCDDATA(0x00); //67  COS7_R[3:0], COS8_R[3:0]    
WMLCDDATA(0x11); //68  COS9_R[3:0], COS10_R[3:0]   
WMLCDDATA(0x33); //69  COS11_R[3:0], COS12_R[3:0]  
WMLCDDATA(0x11); //70  COS13_R[3:0], COS14_R[3:0]  
WMLCDDATA(0x55); //71  COS15_R[3:0], COS16_R[3:0]  
WMLCDDATA(0x00); //72  COS17_R[3:0], COS18_R[3:0]  
WMLCDDATA(0x00); //73  TCON_OPT[7:0]
WMLCDDATA(0x00); //74  GIP_OPT[22:16]
WMLCDDATA(0x00); //75  GIP_OPT[15:8]
WMLCDDATA(0x01); //76  GIP_OPT[7:0]
WMLCDDATA(0x00); //77
WMLCDDATA(0x00); //78
WMLCDDATA(0x00); //79
WMLCDDATA(0x0F); //80  HZ_L[17:16], HZ_UL[17:16]
WMLCDDATA(0x00); //81  HZ_L[15:8]
WMLCDDATA(0xCF); //82  HZ_L[7:0]
WMLCDDATA(0xFF); //83  HZ_UL[15:8]
WMLCDDATA(0xFF); //84  HZ_UL[7:0]
WMLCDDATA(0x0F); //85  HZ_R[17:16], HZ_UR[17:16]
WMLCDDATA(0x00); //86  HZ_R[15:8]               
WMLCDDATA(0xCF); //87  HZ_R[7:0]                
WMLCDDATA(0xFF); //88  HZ_UR[15:8]              
WMLCDDATA(0xFF); //89  HZ_UR[7:0]               
WMLCDDATA(0x00); //90  EQ_ENB_B,EQ_ENB_F, HSYNC_TO_CL1_CNT9[8]
WMLCDDATA(0x2A); //91  HSYNC_TO_CL1_CNT9[7:0]
WMLCDDATA(0x5A); //92  COFF_TCON[7:0
	            
 		
	 	WMLCDCOM(0xE3);		//Set EQ
		WMLCDDATA(0x07);		
		WMLCDDATA(0x0B);		
		WMLCDDATA(0x07);		
		WMLCDDATA(0x0B);      
        		
	        WMLCDCOM(0xC0);		
		WMLCDDATA(0x85);		
		WMLCDDATA(0x50);		
		WMLCDDATA(0x00);		
		WMLCDDATA(0x7F);
		WMLCDDATA(0x1C);
		WMLCDDATA(0x09); 
      		
	 	WMLCDCOM(0xE0);		
		WMLCDDATA(0x00);
		WMLCDDATA(0x04);
		WMLCDDATA(0x04);
		WMLCDDATA(0x0D);
		WMLCDDATA(0x10);
		WMLCDDATA(0x2B);
		WMLCDDATA(0x1C);
		WMLCDDATA(0x2E);
		WMLCDDATA(0x01);
		WMLCDDATA(0x0B);
		WMLCDDATA(0x11);
		WMLCDDATA(0x15);
		WMLCDDATA(0x18);
		WMLCDDATA(0x16);
		WMLCDDATA(0x16);
		WMLCDDATA(0x10);
		WMLCDDATA(0x13);
		WMLCDDATA(0x00);
		WMLCDDATA(0x04);
		WMLCDDATA(0x04);
		WMLCDDATA(0x0D);
		WMLCDDATA(0x10);
		WMLCDDATA(0x2B);
		WMLCDDATA(0x1C);
		WMLCDDATA(0x2E);
		WMLCDDATA(0x01);
		WMLCDDATA(0x0B);
		WMLCDDATA(0x11);
		WMLCDDATA(0x15);
		WMLCDDATA(0x18);
		WMLCDDATA(0x16);
		WMLCDDATA(0x16);
		WMLCDDATA(0x10);
		WMLCDDATA(0x13);
		WMLCDDATA(0x01);		
	
                WMLCDCOM(0x11);		
		mdelay(150);
					WMLCDCOM(0x29); 
					 
					mdelay(20); 
			
		      CS(0);
	  truly_state.disp_initialized = TRUE;
	  }

	  data = 0;
	  printk(KERN_INFO "pengwei  truly_disp_on: id=%x\n", data);
}
//static void truly_disp_on(void)
static void truly_lcdc_otm_disp_on(void)
{
	uint32	data;
	printk("%s initialized :%d  \n",__func__,truly_state.disp_initialized);
	if (!truly_state.disp_initialized) {
	   // mdelay(10); 
//NT35510+3.8AUO FOT TFT1N6287CODE 101015
     CS(1);
     SCLK(1);
     SDI(1);
     gpio_set_value(lcd_reset, 1);
     mdelay(10);	
     gpio_set_value(lcd_reset, 0);
     mdelay(10);	
     gpio_set_value(lcd_reset, 1);
     mdelay(20);
/*truly TFT1P6394(OTM8018B+3.97)CODE-120606*/
WMLCDCOM(0xff00);  // 
WMLCDDATA(0x80);
WMLCDCOM(0xff01);  // enable EXTC
WMLCDDATA(0x09);
WMLCDCOM(0xff02);  // 
WMLCDDATA(0x01);

WMLCDCOM(0xff80);  // enable Orise mode
WMLCDDATA(0x80);
WMLCDCOM(0xff81); // 
WMLCDDATA(0x09);

WMLCDCOM(0xff03);  // enable SPI+I2C cmd2 read
WMLCDDATA(0x01);

WMLCDCOM(0xb282); //niuli_truly
WMLCDDATA(0xe0);

//gamma DC																	 
WMLCDCOM(0xc0b4); //1+2dot inversion 
WMLCDDATA(0x10);
WMLCDCOM(0xC489); //reg off
WMLCDDATA(0x08);
WMLCDCOM(0xC0a3); //pre-charge //V02
WMLCDDATA(0x00);

WMLCDCOM(0xC582); //REG-pump23 
WMLCDDATA(0xA3);
WMLCDCOM(0xC590);  //Pump setting (3x=D6)-->(2x=96)//v02 01/11
	WMLCDDATA(0x96);
WMLCDCOM(0xC591);  //Pump setting(VGH/VGL)
WMLCDDATA(0x87);
	
WMLCDCOM(0xD800); //GVDD=4.5V 
	WMLCDDATA(0x72);
WMLCDCOM(0xD801);  //NGVDD=4.5V
WMLCDDATA(0x72);
//VCOMDC
WMLCDCOM(0xd900);  // VCOMDC=
	WMLCDDATA(0x61);
								
								
WMLCDCOM(0xE100); WMLCDDATA(0x09);
WMLCDCOM(0xE101); WMLCDDATA(0x0a);
WMLCDCOM(0xE102); WMLCDDATA(0x0e);
WMLCDCOM(0xE103); WMLCDDATA(0x0d);
WMLCDCOM(0xE104); WMLCDDATA(0x07);
WMLCDCOM(0xE105); WMLCDDATA(0x18);
WMLCDCOM(0xE106); WMLCDDATA(0x0d);
WMLCDCOM(0xE107); WMLCDDATA(0x0d);
WMLCDCOM(0xE108); WMLCDDATA(0x01);
WMLCDCOM(0xE109); WMLCDDATA(0x04);
WMLCDCOM(0xE10A); WMLCDDATA(0x05);
WMLCDCOM(0xE10B); WMLCDDATA(0x06);
WMLCDCOM(0xE10C); WMLCDDATA(0x0e);
WMLCDCOM(0xE10D); WMLCDDATA(0x25);
WMLCDCOM(0xE10E); WMLCDDATA(0x22);
WMLCDCOM(0xE10F); WMLCDDATA(0x05);
					  
// Negative 		  
WMLCDCOM(0xE200); WMLCDDATA(0x09);
WMLCDCOM(0xE201); WMLCDDATA(0x0a);
WMLCDCOM(0xE202); WMLCDDATA(0x0e);
WMLCDCOM(0xE203); WMLCDDATA(0x0d);
WMLCDCOM(0xE204); WMLCDDATA(0x07);
WMLCDCOM(0xE205); WMLCDDATA(0x18);
WMLCDCOM(0xE206); WMLCDDATA(0x0d);
WMLCDCOM(0xE207); WMLCDDATA(0x0d);
WMLCDCOM(0xE208); WMLCDDATA(0x01);
WMLCDCOM(0xE209); WMLCDDATA(0x04);
WMLCDCOM(0xE20A); WMLCDDATA(0x05);
WMLCDCOM(0xE20B); WMLCDDATA(0x06);
WMLCDCOM(0xE20C); WMLCDDATA(0x0e);
WMLCDCOM(0xE20D); WMLCDDATA(0x25);
WMLCDCOM(0xE20E); WMLCDDATA(0x22);
WMLCDCOM(0xE20F); WMLCDDATA(0x05);						

// RGB I/F setting VSYNC for OTM8018 0x0e
	WMLCDCOM(0xC181); //external Vsync(08)  /Vsync,Hsync(0c) /Vsync,Hsync,DE(0e) //V02(0e)  / all  included clk(0f)
	WMLCDDATA(0x77);//66

	WMLCDCOM(0xC489); //pre-charge Enable
	WMLCDDATA(0x08);	//04/30 08->00	P-CH ON

	WMLCDCOM(0xC0a2); //pre-charge //PULL-LOW 04/30
	WMLCDDATA(0x04);

	WMLCDCOM(0xC0a3); //pre-charge //P-CH	04/30
	WMLCDDATA(0x00);

	WMLCDCOM(0xC0a4); //pre-charge //P-CH	04/30
	WMLCDDATA(0x02);

	WMLCDCOM(0xC480); //Proch Source Output level
	WMLCDDATA(0x30);

	WMLCDCOM(0xC1A1);
	WMLCDDATA(0x0e);

	WMLCDCOM(0xC1A6);
	WMLCDDATA(0x01);

	WMLCDCOM(0xC5c0); //external Vsync(08)  /Vsync,Hsync(0c) /Vsync,Hsync,DE(0e) //V02(0e)  / all  included clk(0f)
	WMLCDDATA(0x00);

	WMLCDCOM(0xB08B); //
	WMLCDDATA(0x40);

	WMLCDCOM(0xF5B2); //VRGH Disable
	WMLCDDATA(0x15);
	WMLCDCOM(0xF5B3); //
	WMLCDDATA(0x00);
	WMLCDCOM(0xF5B4); //
	WMLCDDATA(0x15);
	WMLCDCOM(0xF5B5); //
	WMLCDDATA(0x00);
WMLCDCOM(0xC481); //source bias //V02
WMLCDDATA(0x83);

	WMLCDCOM(0xC593); //VRGH minimum,
	WMLCDDATA(0x03);
WMLCDCOM(0xC592); //Pump45
WMLCDDATA(0x01);//(01)

WMLCDCOM(0xC5B1);  //DC voltage setting ;[0]GVDD output, default: 0xa8
WMLCDDATA(0xA9);
	
	//--------------------------------------------------------------------------------
	//		initial setting 2 < tcon_goa_wave >  480x800
	//--------------------------------------------------------------------------------
	// C09x : mck_shift1/mck_shift2/mck_shift3
	WMLCDCOM(0xC090);	// c091[7:0] : mck_shift1_hi[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC091);	// c092[7:0] : mck_shift1_lo[7:0]
	WMLCDDATA(0x44);		//44->56
	WMLCDCOM(0xC092);	// c093[7:0] : mck_shift2_hi[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC093);	// c094[7:0] : mck_shift2_lo[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC094);	// c095[7:0] : mck_shift3_hi[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC095);	// c096[7:0] : mck_shift3_lo[7:0]
	WMLCDDATA(0x03);	                                                                                                                      

	// C1Ax : hs_shift/vs_shift
	WMLCDCOM(0xC1a6);	// c1a6[7:0] : hs_shift[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC1a7);	// c1a7[7:0] : vs_shift_hi[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC1a8);	// c1a8[7:0] : vs_shift_lo[7:0]
	WMLCDDATA(0x00);
	//----------------------------------------------------
// CE8x : vst1, vst2, vst3, vst4
WMLCDCOM(0xCE80); // ce81[7:0] : vst1_shift[7:0]
	WMLCDDATA(0x87);
WMLCDCOM(0xCE81); // ce82[7:0] : 0000,	vst1_width[3:0]
WMLCDDATA(0x03);
WMLCDCOM(0xCE82); // ce83[7:0] : vst1_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCE83); // ce84[7:0] : vst2_shift[7:0]
	WMLCDDATA(0x85);
WMLCDCOM(0xCE84); // ce85[7:0] : 0000,	vst2_width[3:0]
WMLCDDATA(0x03);
WMLCDCOM(0xCE85); // ce86[7:0] : vst2_tchop[7:0]
WMLCDDATA(0x00);								
WMLCDCOM(0xCE86); // ce87[7:0] : vst3_shift[7:0]											  
	WMLCDDATA(0x86);																														  
WMLCDCOM(0xCE87); // ce88[7:0] : 0000,	vst3_width[3:0] 																			  
WMLCDDATA(0x03);															  
WMLCDCOM(0xCE88); // ce89[7:0] : vst3_tchop[7:0]								  
WMLCDDATA(0x00);																	  
WMLCDCOM(0xCE89); // ce8a[7:0] : vst4_shift[7:0]									  
	WMLCDDATA(0x84);
WMLCDCOM(0xCE8a); // ce8b[7:0] : 0000,	vst4_width[3:0]
WMLCDDATA(0x03);
WMLCDCOM(0xCE8b); // ce8c[7:0] : vst4_tchop[7:0]
WMLCDDATA(0x00);

//CEAx : clka1, clka2
WMLCDCOM(0xCEa0); // cea1[7:0] : clka1_width[3:0], clka1_shift[11:8]
WMLCDDATA(0x38);
WMLCDCOM(0xCEa1); // cea2[7:0] : clka1_shift[7:0]
	WMLCDDATA(0x03);
WMLCDCOM(0xCEa2); // cea3[7:0] : clka1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEa3); // cea4[7:0] : clka1_switch[7:0]
	WMLCDDATA(0x20);
WMLCDCOM(0xCEa4); // cea5[7:0] : clka1_extend[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEa5); // cea6[7:0] : clka1_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEa6); // cea7[7:0] : clka1_tglue[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEa7); // cea8[7:0] : clka2_width[3:0], clka2_shift[11:8]
WMLCDDATA(0x38);
WMLCDCOM(0xCEa8); // cea9[7:0] : clka2_shift[7:0]
	WMLCDDATA(0x02);
WMLCDCOM(0xCEa9); // ceaa[7:0] : clka2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEaa); // ceab[7:0] : clka2_switch[7:0]
	WMLCDDATA(0x21);
WMLCDCOM(0xCEab); // ceac[7:0] : clka2_extend
WMLCDDATA(0x00);
WMLCDCOM(0xCEac); // cead[7:0] : clka2_tchop
WMLCDDATA(0x00);
WMLCDCOM(0xCEad); // ceae[7:0] : clka2_tglue
WMLCDDATA(0x00);

//CEBx : clka3, clka4
WMLCDCOM(0xCEb0); // ceb1[7:0] : clka3_width[3:0], clka3_shift[11:8]
WMLCDDATA(0x38);
WMLCDCOM(0xCEb1); // ceb2[7:0] : clka3_shift[7:0]
	WMLCDDATA(0x01);
WMLCDCOM(0xCEb2); // ceb3[7:0] : clka3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEb3); // ceb4[7:0] : clka3_switch[7:0]
	WMLCDDATA(0x22);
WMLCDCOM(0xCEb4); // ceb5[7:0] : clka3_extend[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEb5); // ceb6[7:0] : clka3_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEb6); // ceb7[7:0] : clka3_tglue[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEb7); // ceb8[7:0] : clka4_width[3:0], clka2_shift[11:8]
	WMLCDDATA(0x38);
WMLCDCOM(0xCEb8); // ceb9[7:0] : clka4_shift[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEb9); // ceba[7:0] : clka4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEba); // cebb[7:0] : clka4_switch[7:0]
	WMLCDDATA(0x23);
WMLCDCOM(0xCEbb); // cebc[7:0] : clka4_extend
WMLCDDATA(0x00);
WMLCDCOM(0xCEbc); // cebd[7:0] : clka4_tchop
WMLCDDATA(0x00);
WMLCDCOM(0xCEbd); // cebe[7:0] : clka4_tglue
WMLCDDATA(0x00);

//CECx : clkb1, clkb2
WMLCDCOM(0xCEc0); // cec1[7:0] : clkb1_width[3:0], clkb1_shift[11:8]
WMLCDDATA(0x30);
WMLCDCOM(0xCEc1); // cec2[7:0] : clkb1_shift[7:0]
	WMLCDDATA(0x00);
WMLCDCOM(0xCEc2); // cec3[7:0] : clkb1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEc3); // cec4[7:0] : clkb1_switch[7:0]
	WMLCDDATA(0x24);
WMLCDCOM(0xCEc4); // cec5[7:0] : clkb1_extend[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEc5); // cec6[7:0] : clkb1_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEc6); // cec7[7:0] : clkb1_tglue[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEc7); // cec8[7:0] : clkb2_width[3:0], clkb2_shift[11:8]
WMLCDDATA(0x30);
WMLCDCOM(0xCEc8); // cec9[7:0] : clkb2_shift[7:0]
	WMLCDDATA(0x01);
WMLCDCOM(0xCEc9); // ceca[7:0] : clkb2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEca); // cecb[7:0] : clkb2_switch[7:0]
	WMLCDDATA(0x25);
WMLCDCOM(0xCEcb); // cecc[7:0] : clkb2_extend
WMLCDDATA(0x00);
WMLCDCOM(0xCEcc); // cecd[7:0] : clkb2_tchop
WMLCDDATA(0x00);
WMLCDCOM(0xCEcd); // cece[7:0] : clkb2_tglue
WMLCDDATA(0x00);

//CEDx : clkb3, clkb4
WMLCDCOM(0xCEd0); // ced1[7:0] : clkb3_width[3:0], clkb3_shift[11:8]
WMLCDDATA(0x30);
WMLCDCOM(0xCEd1); // ced2[7:0] : clkb3_shift[7:0]
	WMLCDDATA(0x02);
WMLCDCOM(0xCEd2); // ced3[7:0] : clkb3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEd3); // ced4[7:0] : clkb3_switch[7:0]
	WMLCDDATA(0x26);
WMLCDCOM(0xCEd4); // ced5[7:0] : clkb3_extend[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEd5); // ced6[7:0] : clkb3_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEd6); // ced7[7:0] : clkb3_tglue[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEd7); // ced8[7:0] : clkb4_width[3:0], clkb4_shift[11:8]
WMLCDDATA(0x30);
WMLCDCOM(0xCEd8); // ced9[7:0] : clkb4_shift[7:0]
	WMLCDDATA(0x03);
WMLCDCOM(0xCEd9); // ceda[7:0] : clkb4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEda); // cedb[7:0] : clkb4_switch[7:0]
	WMLCDDATA(0x27);
WMLCDCOM(0xCEdb); // cedc[7:0] : clkb4_extend
WMLCDDATA(0x00);
WMLCDCOM(0xCEdc); // cedd[7:0] : clkb4_tchop
WMLCDDATA(0x00);
WMLCDCOM(0xCEdd); // cede[7:0] : clkb4_tglue
WMLCDDATA(0x00);

	//CFCx :

	WMLCDCOM(0xCFc7); // cfc8[7:0] : reg_goa_gnd_opt, reg_goa_dpgm_tail_set, reg_goa_f_gating_en, reg_goa_f_odd_gating, toggle_mod1, 2, 3, 4
	WMLCDDATA(0x00);
	WMLCDCOM(0xCFc9); // cfca[7:0] : reg_goa_gnd_period[7:0]
	WMLCDDATA(0x00);

//CFDx :
WMLCDCOM(0xCFd0); // cfd1[7:0] : 0000000, reg_goa_frame_odd_high
WMLCDDATA(0x00);	// Parameter 1

	//--------------------------------------------------------------------------------
//		initial setting 3 < Panel setting >
//--------------------------------------------------------------------------------
// cbcx
WMLCDCOM(0xCBc0); //cbc1[7:0] : enmode H-byte of sig1  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBc1); //cbc2[7:0] : enmode H-byte of sig2  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBc2); //cbc3[7:0] : enmode H-byte of sig3  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBc3); //cbc4[7:0] : enmode H-byte of sig4  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBc4); //cbc5[7:0] : enmode H-byte of sig5  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc5); //cbc6[7:0] : enmode H-byte of sig6  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc6); //cbc7[7:0] : enmode H-byte of sig7  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc7); //cbc8[7:0] : enmode H-byte of sig8  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc8); //cbc9[7:0] : enmode H-byte of sig9  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc9); //cbca[7:0] : enmode H-byte of sig10 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBca); //cbcb[7:0] : enmode H-byte of sig11 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBcb); //cbcc[7:0] : enmode H-byte of sig12 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBcc); //cbcd[7:0] : enmode H-byte of sig13 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBcd); //cbce[7:0] : enmode H-byte of sig14 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBce); //cbcf[7:0] : enmode H-byte of sig15 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);

// cbdx
WMLCDCOM(0xCBd0); //cbd1[7:0] : enmode H-byte of sig16 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd1); //cbd2[7:0] : enmode H-byte of sig17 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd2); //cbd3[7:0] : enmode H-byte of sig18 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd3); //cbd4[7:0] : enmode H-byte of sig19 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd4); //cbd5[7:0] : enmode H-byte of sig20 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd5); //cbd6[7:0] : enmode H-byte of sig21 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd6); //cbd7[7:0] : enmode H-byte of sig22 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd7); //cbd8[7:0] : enmode H-byte of sig23 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd8); //cbd9[7:0] : enmode H-byte of sig24 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd9); //cbda[7:0] : enmode H-byte of sig25 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBda); //cbdb[7:0] : enmode H-byte of sig26 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBdb); //cbdc[7:0] : enmode H-byte of sig27 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBdc); //cbdd[7:0] : enmode H-byte of sig28 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBdd); //cbde[7:0] : enmode H-byte of sig29 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBde); //cbdf[7:0] : enmode H-byte of sig30 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);

// cbex
WMLCDCOM(0xCBe0); //cbe1[7:0] : enmode H-byte of sig31 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe1); //cbe2[7:0] : enmode H-byte of sig32 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe2); //cbe3[7:0] : enmode H-byte of sig33 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe3); //cbe4[7:0] : enmode H-byte of sig34 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe4); //cbe5[7:0] : enmode H-byte of sig35 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe5); //cbe6[7:0] : enmode H-byte of sig36 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe6); //cbe7[7:0] : enmode H-byte of sig37 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe7); //cbe8[7:0] : enmode H-byte of sig38 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe8); //cbe9[7:0] : enmode H-byte of sig39 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe9); //cbea[7:0] : enmode H-byte of sig40 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);

// cc8x
WMLCDCOM(0xCC80); //cc81[7:0] : reg setting for signal01 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC81); //cc82[7:0] : reg setting for signal02 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC82); //cc83[7:0] : reg setting for signal03 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC83); //cc84[7:0] : reg setting for signal04 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC84); //cc85[7:0] : reg setting for signal05 selection with u2d mode
WMLCDDATA(0x0C);
WMLCDCOM(0xCC85); //cc86[7:0] : reg setting for signal06 selection with u2d mode
WMLCDDATA(0x0A);
WMLCDCOM(0xCC86); //cc87[7:0] : reg setting for signal07 selection with u2d mode
WMLCDDATA(0x10);
WMLCDCOM(0xCC87); //cc88[7:0] : reg setting for signal08 selection with u2d mode
WMLCDDATA(0x0E);
WMLCDCOM(0xCC88); //cc89[7:0] : reg setting for signal09 selection with u2d mode
WMLCDDATA(0x03);
WMLCDCOM(0xCC89); //cc8a[7:0] : reg setting for signal10 selection with u2d mode
WMLCDDATA(0x04);

// cc9x
WMLCDCOM(0xCC90); //cc91[7:0] : reg setting for signal11 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC91); //cc92[7:0] : reg setting for signal12 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC92); //cc93[7:0] : reg setting for signal13 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC93); //cc94[7:0] : reg setting for signal14 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC94); //cc95[7:0] : reg setting for signal15 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC95); //cc96[7:0] : reg setting for signal16 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC96); //cc97[7:0] : reg setting for signal17 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC97); //cc98[7:0] : reg setting for signal18 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC98); //cc99[7:0] : reg setting for signal19 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC99); //cc9a[7:0] : reg setting for signal20 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9a); //cc9b[7:0] : reg setting for signal21 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9b); //cc9c[7:0] : reg setting for signal22 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9c); //cc9d[7:0] : reg setting for signal23 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9d); //cc9e[7:0] : reg setting for signal24 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9e); //cc9f[7:0] : reg setting for signal25 selection with u2d mode
WMLCDDATA(0x0B);

// ccax
WMLCDCOM(0xCCa0); //cca1[7:0] : reg setting for signal26 selection with u2d mode
WMLCDDATA(0x09);
WMLCDCOM(0xCCa1); //cca2[7:0] : reg setting for signal27 selection with u2d mode
WMLCDDATA(0x0F);
WMLCDCOM(0xCCa2); //cca3[7:0] : reg setting for signal28 selection with u2d mode
WMLCDDATA(0x0D);
WMLCDCOM(0xCCa3); //cca4[7:0] : reg setting for signal29 selection with u2d mode
WMLCDDATA(0x01);
WMLCDCOM(0xCCa4); //cca5[7:0] : reg setting for signal20 selection with u2d mode
WMLCDDATA(0x02);
WMLCDCOM(0xCCa5); //cca6[7:0] : reg setting for signal31 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCa6); //cca7[7:0] : reg setting for signal32 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCa7); //cca8[7:0] : reg setting for signal33 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCa8); //cca9[7:0] : reg setting for signal34 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCa9); //ccaa[7:0] : reg setting for signal35 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCaa); //ccab[7:0] : reg setting for signal36 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCab); //ccac[7:0] : reg setting for signal37 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCac); //ccad[7:0] : reg setting for signal38 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCad); //ccae[7:0] : reg setting for signal39 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCae); //ccaf[7:0] : reg setting for signal40 selection with u2d mode
WMLCDDATA(0x00);

// ccbx
WMLCDCOM(0xCCb0); //ccb1[7:0] : reg setting for signal01 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCb1); //ccb2[7:0] : reg setting for signal02 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCb2); //ccb3[7:0] : reg setting for signal03 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCb3); //ccb4[7:0] : reg setting for signal04 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCb4); //ccb5[7:0] : reg setting for signal05 selection with d2u mode
WMLCDDATA(0x0D);
WMLCDCOM(0xCCb5); //ccb6[7:0] : reg setting for signal06 selection with d2u mode
WMLCDDATA(0x0F);
WMLCDCOM(0xCCb6); //ccb7[7:0] : reg setting for signal07 selection with d2u mode
WMLCDDATA(0x09);
WMLCDCOM(0xCCb7); //ccb8[7:0] : reg setting for signal08 selection with d2u mode
WMLCDDATA(0x0B);
WMLCDCOM(0xCCb8); //ccb9[7:0] : reg setting for signal09 selection with d2u mode
WMLCDDATA(0x02);
WMLCDCOM(0xCCb9); //ccba[7:0] : reg setting for signal10 selection with d2u mode
WMLCDDATA(0x01);

// cccx
WMLCDCOM(0xCCc0); //ccc1[7:0] : reg setting for signal11 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc1); //ccc2[7:0] : reg setting for signal12 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc2); //ccc3[7:0] : reg setting for signal13 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc3); //ccc4[7:0] : reg setting for signal14 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc4); //ccc5[7:0] : reg setting for signal15 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc5); //ccc6[7:0] : reg setting for signal16 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc6); //ccc7[7:0] : reg setting for signal17 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc7); //ccc8[7:0] : reg setting for signal18 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc8); //ccc9[7:0] : reg setting for signal19 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc9); //ccca[7:0] : reg setting for signal20 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCca); //cccb[7:0] : reg setting for signal21 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCcb); //cccc[7:0] : reg setting for signal22 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCcc); //cccd[7:0] : reg setting for signal23 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCcd); //ccce[7:0] : reg setting for signal24 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCce); //cccf[7:0] : reg setting for signal25 selection with d2u mode
WMLCDDATA(0x0E);

// ccdx
WMLCDCOM(0xCCd0); //ccd1[7:0] : reg setting for signal26 selection with d2u mode
WMLCDDATA(0x10);
WMLCDCOM(0xCCd1); //ccd2[7:0] : reg setting for signal27 selection with d2u mode
WMLCDDATA(0x0A);
WMLCDCOM(0xCCd2); //ccd3[7:0] : reg setting for signal28 selection with d2u mode
WMLCDDATA(0x0C);
WMLCDCOM(0xCCd3); //ccd4[7:0] : reg setting for signal29 selection with d2u mode
WMLCDDATA(0x04);
WMLCDCOM(0xCCd4); //ccd5[7:0] : reg setting for signal30 selection with d2u mode
WMLCDDATA(0x03);
WMLCDCOM(0xCCd5); //ccd6[7:0] : reg setting for signal31 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCd6); //ccd7[7:0] : reg setting for signal32 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCd7); //ccd8[7:0] : reg setting for signal33 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCd8); //ccd9[7:0] : reg setting for signal34 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCd9); //ccda[7:0] : reg setting for signal35 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCda); //ccdb[7:0] : reg setting for signal36 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCdb); //ccdc[7:0] : reg setting for signal37 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCdc); //ccdd[7:0] : reg setting for signal38 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCdd); //ccde[7:0] : reg setting for signal39 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCde); //ccdf[7:0] : reg setting for signal40 selection with d2u mode
WMLCDDATA(0x00);

WMLCDCOM(0x2000);
//WMLCDCOM(0x2100);

WMLCDCOM(0x3A00);  //  RGB 18bits D[17:0]
WMLCDDATA(0x77);			

WMLCDCOM(0x1100);	
mdelay(120);
	
WMLCDCOM(0x2900);	
mdelay(50);
		CS(0);
		truly_state.disp_initialized = TRUE;
	}
	
	data = 0;
	printk(KERN_INFO "qupw truly_disp_on: id=%x\n", data);
}
#endif


/*pengwei added for NEW lcd notify*/
#ifdef LCD_SPI_TIMING_SUPPORT
void truly_disp_on(void)
{
#ifdef TYQ_LCDC_COMP_SUPPORT
if(lcddrvic_id == OTM8018B_CHIP_ID)
	truly_lcdc_otm_disp_on();
else
	truly_hx8369b_disp_on();
#else

	uint32	data;
	if (!truly_state.disp_initialized) {
	   // mdelay(10); 
//NT35510+3.8AUO FOT TFT1N6287CODE 101015
     CS(1);
     SCLK(1);
     SDI(1);
     gpio_set_value(lcd_reset, 1);
     mdelay(10);	
     gpio_set_value(lcd_reset, 0);
     mdelay(10);	
     gpio_set_value(lcd_reset, 1);
     mdelay(20);
/*truly TFT1P6394(OTM8018B+3.97)CODE-120606*/
WMLCDCOM(0xff00);  // 
WMLCDDATA(0x80);
WMLCDCOM(0xff01);  // enable EXTC
WMLCDDATA(0x09);
WMLCDCOM(0xff02);  // 
WMLCDDATA(0x01);

WMLCDCOM(0xff80);  // enable Orise mode
WMLCDDATA(0x80);
WMLCDCOM(0xff81); // 
WMLCDDATA(0x09);

WMLCDCOM(0xff03);  // enable SPI+I2C cmd2 read
WMLCDDATA(0x01);

WMLCDCOM(0xb282); //niuli_truly
WMLCDDATA(0xe0);

//gamma DC																	 
WMLCDCOM(0xc0b4); //1+2dot inversion 
WMLCDDATA(0x10);
WMLCDCOM(0xC489); //reg off
WMLCDDATA(0x08);
WMLCDCOM(0xC0a3); //pre-charge //V02
WMLCDDATA(0x00);

WMLCDCOM(0xC582); //REG-pump23 
WMLCDDATA(0xA3);
WMLCDCOM(0xC590);  //Pump setting (3x=D6)-->(2x=96)//v02 01/11
	WMLCDDATA(0x96);
WMLCDCOM(0xC591);  //Pump setting(VGH/VGL)
WMLCDDATA(0x87);
	
WMLCDCOM(0xD800); //GVDD=4.5V 
	WMLCDDATA(0x72);
WMLCDCOM(0xD801);  //NGVDD=4.5V
WMLCDDATA(0x72);
//VCOMDC
WMLCDCOM(0xd900);  // VCOMDC=
	WMLCDDATA(0x61);
								
								
WMLCDCOM(0xE100); WMLCDDATA(0x09);
WMLCDCOM(0xE101); WMLCDDATA(0x0a);
WMLCDCOM(0xE102); WMLCDDATA(0x0e);
WMLCDCOM(0xE103); WMLCDDATA(0x0d);
WMLCDCOM(0xE104); WMLCDDATA(0x07);
WMLCDCOM(0xE105); WMLCDDATA(0x18);
WMLCDCOM(0xE106); WMLCDDATA(0x0d);
WMLCDCOM(0xE107); WMLCDDATA(0x0d);
WMLCDCOM(0xE108); WMLCDDATA(0x01);
WMLCDCOM(0xE109); WMLCDDATA(0x04);
WMLCDCOM(0xE10A); WMLCDDATA(0x05);
WMLCDCOM(0xE10B); WMLCDDATA(0x06);
WMLCDCOM(0xE10C); WMLCDDATA(0x0e);
WMLCDCOM(0xE10D); WMLCDDATA(0x25);
WMLCDCOM(0xE10E); WMLCDDATA(0x22);
WMLCDCOM(0xE10F); WMLCDDATA(0x05);
					  
// Negative 		  
WMLCDCOM(0xE200); WMLCDDATA(0x09);
WMLCDCOM(0xE201); WMLCDDATA(0x0a);
WMLCDCOM(0xE202); WMLCDDATA(0x0e);
WMLCDCOM(0xE203); WMLCDDATA(0x0d);
WMLCDCOM(0xE204); WMLCDDATA(0x07);
WMLCDCOM(0xE205); WMLCDDATA(0x18);
WMLCDCOM(0xE206); WMLCDDATA(0x0d);
WMLCDCOM(0xE207); WMLCDDATA(0x0d);
WMLCDCOM(0xE208); WMLCDDATA(0x01);
WMLCDCOM(0xE209); WMLCDDATA(0x04);
WMLCDCOM(0xE20A); WMLCDDATA(0x05);
WMLCDCOM(0xE20B); WMLCDDATA(0x06);
WMLCDCOM(0xE20C); WMLCDDATA(0x0e);
WMLCDCOM(0xE20D); WMLCDDATA(0x25);
WMLCDCOM(0xE20E); WMLCDDATA(0x22);
WMLCDCOM(0xE20F); WMLCDDATA(0x05);						

// RGB I/F setting VSYNC for OTM8018 0x0e
	WMLCDCOM(0xC181); //external Vsync(08)  /Vsync,Hsync(0c) /Vsync,Hsync,DE(0e) //V02(0e)  / all  included clk(0f)
	WMLCDDATA(0x77);//66

	WMLCDCOM(0xC489); //pre-charge Enable
	WMLCDDATA(0x08);	//04/30 08->00	P-CH ON

	WMLCDCOM(0xC0a2); //pre-charge //PULL-LOW 04/30
	WMLCDDATA(0x04);

	WMLCDCOM(0xC0a3); //pre-charge //P-CH	04/30
	WMLCDDATA(0x00);

	WMLCDCOM(0xC0a4); //pre-charge //P-CH	04/30
	WMLCDDATA(0x02);

	WMLCDCOM(0xC480); //Proch Source Output level
	WMLCDDATA(0x30);

	WMLCDCOM(0xC1A1);
	WMLCDDATA(0x0e);

	WMLCDCOM(0xC1A6);
	WMLCDDATA(0x01);

	WMLCDCOM(0xC5c0); //external Vsync(08)  /Vsync,Hsync(0c) /Vsync,Hsync,DE(0e) //V02(0e)  / all  included clk(0f)
	WMLCDDATA(0x00);

	WMLCDCOM(0xB08B); //
	WMLCDDATA(0x40);

	WMLCDCOM(0xF5B2); //VRGH Disable
	WMLCDDATA(0x15);
	WMLCDCOM(0xF5B3); //
	WMLCDDATA(0x00);
	WMLCDCOM(0xF5B4); //
	WMLCDDATA(0x15);
	WMLCDCOM(0xF5B5); //
	WMLCDDATA(0x00);
WMLCDCOM(0xC481); //source bias //V02
WMLCDDATA(0x83);

	WMLCDCOM(0xC593); //VRGH minimum,
	WMLCDDATA(0x03);
WMLCDCOM(0xC592); //Pump45
WMLCDDATA(0x01);//(01)

WMLCDCOM(0xC5B1);  //DC voltage setting ;[0]GVDD output, default: 0xa8
WMLCDDATA(0xA9);
	
	//--------------------------------------------------------------------------------
	//		initial setting 2 < tcon_goa_wave >  480x800
	//--------------------------------------------------------------------------------
	// C09x : mck_shift1/mck_shift2/mck_shift3
	WMLCDCOM(0xC090);	// c091[7:0] : mck_shift1_hi[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC091);	// c092[7:0] : mck_shift1_lo[7:0]
	WMLCDDATA(0x44);		//44->56
	WMLCDCOM(0xC092);	// c093[7:0] : mck_shift2_hi[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC093);	// c094[7:0] : mck_shift2_lo[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC094);	// c095[7:0] : mck_shift3_hi[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC095);	// c096[7:0] : mck_shift3_lo[7:0]
	WMLCDDATA(0x03);	                                                                                                                      

	// C1Ax : hs_shift/vs_shift
	WMLCDCOM(0xC1a6);	// c1a6[7:0] : hs_shift[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC1a7);	// c1a7[7:0] : vs_shift_hi[7:0]
	WMLCDDATA(0x00);
	WMLCDCOM(0xC1a8);	// c1a8[7:0] : vs_shift_lo[7:0]
	WMLCDDATA(0x00);
	//----------------------------------------------------
// CE8x : vst1, vst2, vst3, vst4
WMLCDCOM(0xCE80); // ce81[7:0] : vst1_shift[7:0]
	WMLCDDATA(0x87);
WMLCDCOM(0xCE81); // ce82[7:0] : 0000,	vst1_width[3:0]
WMLCDDATA(0x03);
WMLCDCOM(0xCE82); // ce83[7:0] : vst1_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCE83); // ce84[7:0] : vst2_shift[7:0]
	WMLCDDATA(0x85);
WMLCDCOM(0xCE84); // ce85[7:0] : 0000,	vst2_width[3:0]
WMLCDDATA(0x03);
WMLCDCOM(0xCE85); // ce86[7:0] : vst2_tchop[7:0]
WMLCDDATA(0x00);								
WMLCDCOM(0xCE86); // ce87[7:0] : vst3_shift[7:0]											  
	WMLCDDATA(0x86);																														  
WMLCDCOM(0xCE87); // ce88[7:0] : 0000,	vst3_width[3:0] 																			  
WMLCDDATA(0x03);															  
WMLCDCOM(0xCE88); // ce89[7:0] : vst3_tchop[7:0]								  
WMLCDDATA(0x00);																	  
WMLCDCOM(0xCE89); // ce8a[7:0] : vst4_shift[7:0]									  
	WMLCDDATA(0x84);
WMLCDCOM(0xCE8a); // ce8b[7:0] : 0000,	vst4_width[3:0]
WMLCDDATA(0x03);
WMLCDCOM(0xCE8b); // ce8c[7:0] : vst4_tchop[7:0]
WMLCDDATA(0x00);

//CEAx : clka1, clka2
WMLCDCOM(0xCEa0); // cea1[7:0] : clka1_width[3:0], clka1_shift[11:8]
WMLCDDATA(0x38);
WMLCDCOM(0xCEa1); // cea2[7:0] : clka1_shift[7:0]
	WMLCDDATA(0x03);
WMLCDCOM(0xCEa2); // cea3[7:0] : clka1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEa3); // cea4[7:0] : clka1_switch[7:0]
	WMLCDDATA(0x20);
WMLCDCOM(0xCEa4); // cea5[7:0] : clka1_extend[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEa5); // cea6[7:0] : clka1_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEa6); // cea7[7:0] : clka1_tglue[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEa7); // cea8[7:0] : clka2_width[3:0], clka2_shift[11:8]
WMLCDDATA(0x38);
WMLCDCOM(0xCEa8); // cea9[7:0] : clka2_shift[7:0]
	WMLCDDATA(0x02);
WMLCDCOM(0xCEa9); // ceaa[7:0] : clka2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEaa); // ceab[7:0] : clka2_switch[7:0]
	WMLCDDATA(0x21);
WMLCDCOM(0xCEab); // ceac[7:0] : clka2_extend
WMLCDDATA(0x00);
WMLCDCOM(0xCEac); // cead[7:0] : clka2_tchop
WMLCDDATA(0x00);
WMLCDCOM(0xCEad); // ceae[7:0] : clka2_tglue
WMLCDDATA(0x00);

//CEBx : clka3, clka4
WMLCDCOM(0xCEb0); // ceb1[7:0] : clka3_width[3:0], clka3_shift[11:8]
WMLCDDATA(0x38);
WMLCDCOM(0xCEb1); // ceb2[7:0] : clka3_shift[7:0]
	WMLCDDATA(0x01);
WMLCDCOM(0xCEb2); // ceb3[7:0] : clka3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEb3); // ceb4[7:0] : clka3_switch[7:0]
	WMLCDDATA(0x22);
WMLCDCOM(0xCEb4); // ceb5[7:0] : clka3_extend[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEb5); // ceb6[7:0] : clka3_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEb6); // ceb7[7:0] : clka3_tglue[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEb7); // ceb8[7:0] : clka4_width[3:0], clka2_shift[11:8]
	WMLCDDATA(0x38);
WMLCDCOM(0xCEb8); // ceb9[7:0] : clka4_shift[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEb9); // ceba[7:0] : clka4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEba); // cebb[7:0] : clka4_switch[7:0]
	WMLCDDATA(0x23);
WMLCDCOM(0xCEbb); // cebc[7:0] : clka4_extend
WMLCDDATA(0x00);
WMLCDCOM(0xCEbc); // cebd[7:0] : clka4_tchop
WMLCDDATA(0x00);
WMLCDCOM(0xCEbd); // cebe[7:0] : clka4_tglue
WMLCDDATA(0x00);

//CECx : clkb1, clkb2
WMLCDCOM(0xCEc0); // cec1[7:0] : clkb1_width[3:0], clkb1_shift[11:8]
WMLCDDATA(0x30);
WMLCDCOM(0xCEc1); // cec2[7:0] : clkb1_shift[7:0]
	WMLCDDATA(0x00);
WMLCDCOM(0xCEc2); // cec3[7:0] : clkb1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEc3); // cec4[7:0] : clkb1_switch[7:0]
	WMLCDDATA(0x24);
WMLCDCOM(0xCEc4); // cec5[7:0] : clkb1_extend[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEc5); // cec6[7:0] : clkb1_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEc6); // cec7[7:0] : clkb1_tglue[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEc7); // cec8[7:0] : clkb2_width[3:0], clkb2_shift[11:8]
WMLCDDATA(0x30);
WMLCDCOM(0xCEc8); // cec9[7:0] : clkb2_shift[7:0]
	WMLCDDATA(0x01);
WMLCDCOM(0xCEc9); // ceca[7:0] : clkb2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEca); // cecb[7:0] : clkb2_switch[7:0]
	WMLCDDATA(0x25);
WMLCDCOM(0xCEcb); // cecc[7:0] : clkb2_extend
WMLCDDATA(0x00);
WMLCDCOM(0xCEcc); // cecd[7:0] : clkb2_tchop
WMLCDDATA(0x00);
WMLCDCOM(0xCEcd); // cece[7:0] : clkb2_tglue
WMLCDDATA(0x00);

//CEDx : clkb3, clkb4
WMLCDCOM(0xCEd0); // ced1[7:0] : clkb3_width[3:0], clkb3_shift[11:8]
WMLCDDATA(0x30);
WMLCDCOM(0xCEd1); // ced2[7:0] : clkb3_shift[7:0]
	WMLCDDATA(0x02);
WMLCDCOM(0xCEd2); // ced3[7:0] : clkb3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEd3); // ced4[7:0] : clkb3_switch[7:0]
	WMLCDDATA(0x26);
WMLCDCOM(0xCEd4); // ced5[7:0] : clkb3_extend[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEd5); // ced6[7:0] : clkb3_tchop[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEd6); // ced7[7:0] : clkb3_tglue[7:0]
WMLCDDATA(0x00);
WMLCDCOM(0xCEd7); // ced8[7:0] : clkb4_width[3:0], clkb4_shift[11:8]
WMLCDDATA(0x30);
WMLCDCOM(0xCEd8); // ced9[7:0] : clkb4_shift[7:0]
	WMLCDDATA(0x03);
WMLCDCOM(0xCEd9); // ceda[7:0] : clkb4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]
WMLCDDATA(0x03);
WMLCDCOM(0xCEda); // cedb[7:0] : clkb4_switch[7:0]
	WMLCDDATA(0x27);
WMLCDCOM(0xCEdb); // cedc[7:0] : clkb4_extend
WMLCDDATA(0x00);
WMLCDCOM(0xCEdc); // cedd[7:0] : clkb4_tchop
WMLCDDATA(0x00);
WMLCDCOM(0xCEdd); // cede[7:0] : clkb4_tglue
WMLCDDATA(0x00);

	//CFCx :

	WMLCDCOM(0xCFc7); // cfc8[7:0] : reg_goa_gnd_opt, reg_goa_dpgm_tail_set, reg_goa_f_gating_en, reg_goa_f_odd_gating, toggle_mod1, 2, 3, 4
	WMLCDDATA(0x00);
	WMLCDCOM(0xCFc9); // cfca[7:0] : reg_goa_gnd_period[7:0]
	WMLCDDATA(0x00);

//CFDx :
WMLCDCOM(0xCFd0); // cfd1[7:0] : 0000000, reg_goa_frame_odd_high
WMLCDDATA(0x00);	// Parameter 1

	//--------------------------------------------------------------------------------
//		initial setting 3 < Panel setting >
//--------------------------------------------------------------------------------
// cbcx
WMLCDCOM(0xCBc0); //cbc1[7:0] : enmode H-byte of sig1  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBc1); //cbc2[7:0] : enmode H-byte of sig2  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBc2); //cbc3[7:0] : enmode H-byte of sig3  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBc3); //cbc4[7:0] : enmode H-byte of sig4  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBc4); //cbc5[7:0] : enmode H-byte of sig5  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc5); //cbc6[7:0] : enmode H-byte of sig6  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc6); //cbc7[7:0] : enmode H-byte of sig7  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc7); //cbc8[7:0] : enmode H-byte of sig8  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc8); //cbc9[7:0] : enmode H-byte of sig9  (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBc9); //cbca[7:0] : enmode H-byte of sig10 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBca); //cbcb[7:0] : enmode H-byte of sig11 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBcb); //cbcc[7:0] : enmode H-byte of sig12 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBcc); //cbcd[7:0] : enmode H-byte of sig13 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBcd); //cbce[7:0] : enmode H-byte of sig14 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBce); //cbcf[7:0] : enmode H-byte of sig15 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);

// cbdx
WMLCDCOM(0xCBd0); //cbd1[7:0] : enmode H-byte of sig16 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd1); //cbd2[7:0] : enmode H-byte of sig17 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd2); //cbd3[7:0] : enmode H-byte of sig18 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd3); //cbd4[7:0] : enmode H-byte of sig19 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd4); //cbd5[7:0] : enmode H-byte of sig20 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd5); //cbd6[7:0] : enmode H-byte of sig21 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd6); //cbd7[7:0] : enmode H-byte of sig22 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd7); //cbd8[7:0] : enmode H-byte of sig23 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd8); //cbd9[7:0] : enmode H-byte of sig24 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBd9); //cbda[7:0] : enmode H-byte of sig25 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBda); //cbdb[7:0] : enmode H-byte of sig26 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBdb); //cbdc[7:0] : enmode H-byte of sig27 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBdc); //cbdd[7:0] : enmode H-byte of sig28 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBdd); //cbde[7:0] : enmode H-byte of sig29 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);
WMLCDCOM(0xCBde); //cbdf[7:0] : enmode H-byte of sig30 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x04);

// cbex
WMLCDCOM(0xCBe0); //cbe1[7:0] : enmode H-byte of sig31 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe1); //cbe2[7:0] : enmode H-byte of sig32 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe2); //cbe3[7:0] : enmode H-byte of sig33 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe3); //cbe4[7:0] : enmode H-byte of sig34 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe4); //cbe5[7:0] : enmode H-byte of sig35 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe5); //cbe6[7:0] : enmode H-byte of sig36 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe6); //cbe7[7:0] : enmode H-byte of sig37 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe7); //cbe8[7:0] : enmode H-byte of sig38 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe8); //cbe9[7:0] : enmode H-byte of sig39 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);
WMLCDCOM(0xCBe9); //cbea[7:0] : enmode H-byte of sig40 (pwrof_0, pwrof_1, norm, pwron_4 )
WMLCDDATA(0x00);

// cc8x
WMLCDCOM(0xCC80); //cc81[7:0] : reg setting for signal01 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC81); //cc82[7:0] : reg setting for signal02 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC82); //cc83[7:0] : reg setting for signal03 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC83); //cc84[7:0] : reg setting for signal04 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC84); //cc85[7:0] : reg setting for signal05 selection with u2d mode
WMLCDDATA(0x0C);

WMLCDCOM(0xCC85); //cc86[7:0] : reg setting for signal06 selection with u2d mode
WMLCDDATA(0x0A);
WMLCDCOM(0xCC86); //cc87[7:0] : reg setting for signal07 selection with u2d mode
WMLCDDATA(0x10);
WMLCDCOM(0xCC87); //cc88[7:0] : reg setting for signal08 selection with u2d mode
WMLCDDATA(0x0E);
WMLCDCOM(0xCC88); //cc89[7:0] : reg setting for signal09 selection with u2d mode
WMLCDDATA(0x03);
WMLCDCOM(0xCC89); //cc8a[7:0] : reg setting for signal10 selection with u2d mode
WMLCDDATA(0x04);

// cc9x
WMLCDCOM(0xCC90); //cc91[7:0] : reg setting for signal11 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC91); //cc92[7:0] : reg setting for signal12 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC92); //cc93[7:0] : reg setting for signal13 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC93); //cc94[7:0] : reg setting for signal14 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC94); //cc95[7:0] : reg setting for signal15 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC95); //cc96[7:0] : reg setting for signal16 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC96); //cc97[7:0] : reg setting for signal17 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC97); //cc98[7:0] : reg setting for signal18 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC98); //cc99[7:0] : reg setting for signal19 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC99); //cc9a[7:0] : reg setting for signal20 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9a); //cc9b[7:0] : reg setting for signal21 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9b); //cc9c[7:0] : reg setting for signal22 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9c); //cc9d[7:0] : reg setting for signal23 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9d); //cc9e[7:0] : reg setting for signal24 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCC9e); //cc9f[7:0] : reg setting for signal25 selection with u2d mode
WMLCDDATA(0x0B);

// ccax
WMLCDCOM(0xCCa0); //cca1[7:0] : reg setting for signal26 selection with u2d mode
WMLCDDATA(0x09);
WMLCDCOM(0xCCa1); //cca2[7:0] : reg setting for signal27 selection with u2d mode
WMLCDDATA(0x0F);
WMLCDCOM(0xCCa2); //cca3[7:0] : reg setting for signal28 selection with u2d mode
WMLCDDATA(0x0D);
WMLCDCOM(0xCCa3); //cca4[7:0] : reg setting for signal29 selection with u2d mode
WMLCDDATA(0x01);
WMLCDCOM(0xCCa4); //cca5[7:0] : reg setting for signal20 selection with u2d mode
WMLCDDATA(0x02);
WMLCDCOM(0xCCa5); //cca6[7:0] : reg setting for signal31 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCa6); //cca7[7:0] : reg setting for signal32 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCa7); //cca8[7:0] : reg setting for signal33 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCa8); //cca9[7:0] : reg setting for signal34 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCa9); //ccaa[7:0] : reg setting for signal35 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCaa); //ccab[7:0] : reg setting for signal36 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCab); //ccac[7:0] : reg setting for signal37 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCac); //ccad[7:0] : reg setting for signal38 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCad); //ccae[7:0] : reg setting for signal39 selection with u2d mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCae); //ccaf[7:0] : reg setting for signal40 selection with u2d mode
WMLCDDATA(0x00);

// ccbx
WMLCDCOM(0xCCb0); //ccb1[7:0] : reg setting for signal01 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCb1); //ccb2[7:0] : reg setting for signal02 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCb2); //ccb3[7:0] : reg setting for signal03 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCb3); //ccb4[7:0] : reg setting for signal04 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCb4); //ccb5[7:0] : reg setting for signal05 selection with d2u mode
WMLCDDATA(0x0D);
WMLCDCOM(0xCCb5); //ccb6[7:0] : reg setting for signal06 selection with d2u mode
WMLCDDATA(0x0F);
WMLCDCOM(0xCCb6); //ccb7[7:0] : reg setting for signal07 selection with d2u mode
WMLCDDATA(0x09);
WMLCDCOM(0xCCb7); //ccb8[7:0] : reg setting for signal08 selection with d2u mode
WMLCDDATA(0x0B);
WMLCDCOM(0xCCb8); //ccb9[7:0] : reg setting for signal09 selection with d2u mode
WMLCDDATA(0x02);
WMLCDCOM(0xCCb9); //ccba[7:0] : reg setting for signal10 selection with d2u mode
WMLCDDATA(0x01);

// cccx
WMLCDCOM(0xCCc0); //ccc1[7:0] : reg setting for signal11 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc1); //ccc2[7:0] : reg setting for signal12 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc2); //ccc3[7:0] : reg setting for signal13 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc3); //ccc4[7:0] : reg setting for signal14 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc4); //ccc5[7:0] : reg setting for signal15 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc5); //ccc6[7:0] : reg setting for signal16 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc6); //ccc7[7:0] : reg setting for signal17 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc7); //ccc8[7:0] : reg setting for signal18 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc8); //ccc9[7:0] : reg setting for signal19 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCc9); //ccca[7:0] : reg setting for signal20 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCca); //cccb[7:0] : reg setting for signal21 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCcb); //cccc[7:0] : reg setting for signal22 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCcc); //cccd[7:0] : reg setting for signal23 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCcd); //ccce[7:0] : reg setting for signal24 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCce); //cccf[7:0] : reg setting for signal25 selection with d2u mode
WMLCDDATA(0x0E);

// ccdx
WMLCDCOM(0xCCd0); //ccd1[7:0] : reg setting for signal26 selection with d2u mode
WMLCDDATA(0x10);
WMLCDCOM(0xCCd1); //ccd2[7:0] : reg setting for signal27 selection with d2u mode
WMLCDDATA(0x0A);
WMLCDCOM(0xCCd2); //ccd3[7:0] : reg setting for signal28 selection with d2u mode
WMLCDDATA(0x0C);
WMLCDCOM(0xCCd3); //ccd4[7:0] : reg setting for signal29 selection with d2u mode
WMLCDDATA(0x04);
WMLCDCOM(0xCCd4); //ccd5[7:0] : reg setting for signal30 selection with d2u mode
WMLCDDATA(0x03);
WMLCDCOM(0xCCd5); //ccd6[7:0] : reg setting for signal31 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCd6); //ccd7[7:0] : reg setting for signal32 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCd7); //ccd8[7:0] : reg setting for signal33 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCd8); //ccd9[7:0] : reg setting for signal34 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCd9); //ccda[7:0] : reg setting for signal35 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCda); //ccdb[7:0] : reg setting for signal36 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCdb); //ccdc[7:0] : reg setting for signal37 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCdc); //ccdd[7:0] : reg setting for signal38 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCdd); //ccde[7:0] : reg setting for signal39 selection with d2u mode
WMLCDDATA(0x00);
WMLCDCOM(0xCCde); //ccdf[7:0] : reg setting for signal40 selection with d2u mode
WMLCDDATA(0x00);

WMLCDCOM(0x2000);
//WMLCDCOM(0x2100);

WMLCDCOM(0x3A00);  //  RGB 18bits D[17:0]
WMLCDDATA(0x77);			

WMLCDCOM(0x1100);	
mdelay(120);
	
WMLCDCOM(0x2900);	
mdelay(50);
		CS(0);
		truly_state.disp_initialized = TRUE;
	}
	
	data = 0;
	printk(KERN_INFO "qupw truly_disp_on: id=%x\n", data);

#endif
}
#endif


/*tydrv niuli add for sleep in&out on 20111115  begin*/
void lcd_sleep_in(void) 
{ 
	WMLCDCOM(0x2800);	//Display off
	//WMLCDCOM(0x4f00);

      //WMLCDCOM(0xF004);
      //WMLCDDATA(0x00);
      //WMLCDCOM(0Xb300);//�л��ڲ�ʱ��ָ�
    // WMLCDDATA(0x01);//ʹ���ڲ�ʱ��
      WMLCDCOM(0x1000); //Sleep in
       mdelay (10);//����50MS����   
       
	   WMLCDCOM(0x4f00);
	   WMLCDDATA(0x1);
	   mdelay (50);//����50MS����	
} 
 
void lcd_sleep_out(void) 
{ 

    WMLCDCOM(0xF004);
    
     WMLCDDATA(0x00);
     WMLCDCOM(0Xb300);
     WMLCDDATA(0x00); //use internel clock
     WMLCDCOM(0x1100); //Sleep out
     
     mdelay(100);
	 WMLCDCOM(0x2900);	 //Display on
     mdelay(100);
	 
}     
/*tydrv niuli add for sleep in&out on 20111115  end*/
static int lcdc_truly_panel_on(struct platform_device *pdev)
{
	int flag = 0;
	printk("lichm %s init:%d, display_on:%d \n",__func__,truly_state.disp_initialized,truly_state.display_on);
	if (!truly_state.display_on) {
		
/*tydrv niuli add for sleep in&out on 20111115  begin*/
#ifdef DRV_WAKE_IN_AND_OUT
	//if (truly_state.disp_initialized) 	
	//lcd_sleep_out();
#endif
gpio_set_value(109,0);	 // enable RT9293	 
/*tydrv niuli add for sleep in&out on 20111115  end*/
		truly_disp_powerup();
		
		if (!truly_state.disp_initialized) {
		/* Configure reset GPIO that drives DAC */
		if (lcdc_truly_pdata->panel_config_gpio)
			lcdc_truly_pdata->panel_config_gpio(1);

        spi_init(); /* LCD needs SPI */
			flag = 1;		 
			truly_disp_on();
			}
		truly_state.display_on = TRUE;

		/*tydrv qupw add for RT9293B pwm config*/
#if DRV_BACKLIGHT_CTL_RT9293B_SUPPORT	

		if (! IS_ERR(gp_clkto_pwm)) 
			{
			//tydrv pengwei added for change the clk enable mode of 1045 - 1050
			#if 1
			  clk_prepare_enable(gp_clkto_pwm);
			#else
			  clk_enable(gp_clkto_pwm);
			#endif
			}
		#if 0
		if(blpwm_cntx.bl_level)
		{
		  	backlight_onoff(blpwm_cntx.bl_level);
		}
					//printk("qupw truly panel on gp-clk enable\n");
					
	 msleep(10);
	gpio_set_value(109,1);	 // enable RT9293	 
	#endif
#endif
	printk("lcdc_truly_panel_on\r\n");
	}
	return 0;
}

static int lcdc_truly_panel_off(struct platform_device *pdev)
{
printk("lichm %s \n",__func__);
	if (truly_state.disp_powered_up && truly_state.display_on) {

					/*tydrv qupw add for RT9293B pwm config*/
					
#if DRV_BACKLIGHT_CTL_RT9293B_SUPPORT
				gpio_set_value(109,0);	// disable RT9293	
//pengwei add for kpd backlight turn off while lcd panel off
				msleep(16);
				kpd_bl_ctrl(0);
				
				if (! IS_ERR(gp_clkto_pwm)) 
					{	
						  //tydrv pengwei added for change the clk disable mode of 1045 - 1050
				#if 1
						  clk_disable_unprepare(gp_clkto_pwm);
				#else
						  clk_disable(gp_clkto_pwm);
				#endif
					  
					}
				//printk("qupw truly panel on gp-clk diable\n");
#endif

		/* Main panel power off (Deep standby in) */
/*tydrv niuli add for sleep in&out on 20111115  begin*/
#ifdef DRV_WAKE_IN_AND_OUT
		//lcd_sleep_in();
#endif
/*tydrv niuli add for sleep in&out on 20111115  end*/  
		
		gpio_set_value(spi_sdo, 0);
		gpio_set_value(spi_clk, 0);
		gpio_set_value(spi_cs, 0);
		gpio_set_value(lcd_reset, 0);
			
		mdelay(1);
		
		if (lcdc_truly_pdata->panel_config_gpio)
			lcdc_truly_pdata->panel_config_gpio(0);

		truly_state.display_on = FALSE;
		truly_state.disp_initialized = FALSE;
		blpwm_cntx.bl_pre_level = 0;
		printk("lcdc_truly_panel_off\r\n");
	}
	return 0;
}

static void lcdc_truly_set_backlight(struct msm_fb_data_type *mfd)
{
	int bl_level;
	//int ret = -EPERM;

	bl_level = mfd->bl_level;
printk("lichm %s level:%d \n",__func__,bl_level);
/*pengwei added for press powerkey to light on screen abnormal*/
#ifdef TYQ_BACKLIGHT_POWERKEY_ON_FLAG
	ty_bl_level = bl_level;
	if(ty_bl_flag)
		{	
			printk("pengwei set flag int lcdc level =%d\n",ty_bl_level);

			return;
		}
#endif

#ifdef DRV_KPDLIGHT_CTL_PMU_SUPPORT
		kpd_bl_ctrl(bl_level);
#endif
#if DRV_BACKLIGHT_CTL_RT9293B_SUPPORT
        RT9293B_Level_Setting(bl_level);
#endif
}


/*pengwei added for press powerkey to light on screen abnormal*/
#ifdef TYQ_BACKLIGHT_POWERKEY_ON_FLAG
void Ty_lcdc_set_backlight(int on)
{
	if (on)
	{	
		ty_bl_flag = 0;
		printk(" do not set flag \n");
	#ifdef DRV_KPDLIGHT_CTL_PMU_SUPPORT
		kpd_bl_ctrl(ty_bl_level);
	#endif
	#if DRV_BACKLIGHT_CTL_RT9293B_SUPPORT
       		RT9293B_Level_Setting(ty_bl_level);
	#endif
	}
	else
	{
		ty_bl_flag = 1;
		printk(" set flag \n");
	#if DRV_BACKLIGHT_CTL_RT9293B_SUPPORT
       		gpio_set_value(109 , 0);
	#endif
	}

}
#endif

static int lcdc_sdopin_gpio_alloc(unsigned int gpionum)
{
	int rc = 2;

	rc = gpio_tlmm_config(GPIO_CFG(gpionum, 0, GPIO_CFG_INPUT, 
				          GPIO_CFG_NO_PULL, GPIO_CFG_2MA), 
			              GPIO_CFG_ENABLE);
	if (rc) {
		printk(KERN_ERR "lcd_read_id config failed\n");
		return -3;
	}

	rc = gpio_request(gpionum, "lcd_read_ic");

	if(rc){
		printk(KERN_ERR "%s: Failed to request GPIO %d\n",
		       __func__, gpionum);
		gpio_free(gpionum);
	}else{
		rc = gpio_direction_input(gpionum);
		if (!rc){
			printk("tyq_lcd_config_input----->>>%d \n",rc);
		}
	}

	return rc;
}

static int __devinit truly_probe(struct platform_device *pdev)
{
	printk("lichm %s lcd id: %d \n",__func__,pdev->id);
	if (pdev->id == 0) {
		lcdc_truly_pdata = pdev->dev.platform_data;
        	spi_init();
			/*ty:thomasyang add config gpio status for read lcd ic begin 20120606*/ 
#ifdef TYQ_LCDC_COMP_SUPPORT	
					gpio_set_value(lcd_reset,1);
					msleep(10);
			
					lcdc_sdopin_gpio_alloc(spi_sdi);
					mdelay(100);
					//WMLCDCOM(0x11);
					mdelay(100);		
					lcddrvic_id = RMLCDCOM(0xda00);
					printk("thomas dbginfo lcddrvic_id = %x \n", lcddrvic_id);
#else
					lcdc_sdopin_gpio_alloc(spi_sdi);
					mdelay(100);	
					lcddrvic_id = RMLCDCOM(0xda00);
#endif

			/*ty:thomasyang add config gpio status for read lcd ic end 20120606*/
			/*ty:niuli add config gpio status for lcd_id pin begin 20120625*/
#if TYQ_LCDC_NO_CURRENT
					msleep(10);
	
					if(lcddrvic_id == OTM8018B_CHIP_ID){
						//lcdc_gpio_output_high(77);
					}
#endif
			/*ty:niuli add config gpio status for lcd_id pin end 20120625*/

		return 0;
	}
	
#if DRV_BACKLIGHT_CTL_RT9293B_SUPPORT
	   //rt9293b_bl_init();	
#endif

#if DRV_KPDLIGHT_MISC_IOCTL
	truly_misc_data = pdev;
	misc_register(&truly_device);
#endif

	msm_fb_add_device(pdev);
	return 0;
}

#ifdef CONFIG_SPI_QSD
static int __devinit lcdc_truly_spi_probe(struct spi_device *spi)
{
	lcdc_truly_spi_client = spi;
	lcdc_truly_spi_client->bits_per_word = 32;
	return 0;
}
static int __devexit lcdc_truly_spi_remove(struct spi_device *spi)
{
	lcdc_truly_spi_client = NULL;
	return 0;
}

static struct spi_driver lcdc_truly_spi_driver = {
	.driver = {
		.name  = LCDC_truly_SPI_DEVICE_NAME,
		.owner = THIS_MODULE,
	},
	.probe         = lcdc_truly_spi_probe,
	.remove        = __devexit_p(lcdc_truly_spi_remove),
};
#endif
static int lcd_suspend(struct platform_device *pdev,pm_message_t state)
{
	printk("lcd_suspend id=%d\r\n ",pdev->id);
	if(pdev->id==0)
		return 0;
	if(truly_state.disp_initialized)
	{
		gpio_set_value(lcd_reset, 0);			
		mdelay(1);
		

		if (lcdc_truly_pdata->panel_config_gpio)
			lcdc_truly_pdata->panel_config_gpio(0);

		truly_state.disp_initialized=FALSE;
	}
	

	return 0;
}

static int lcd_resume(struct platform_device *pdev)
{
	printk("lcd_resume id=%d\r\n ",pdev->id);
	if(pdev->id==0)		
		return 0;
	if(!truly_state.disp_initialized)
	{
		if (lcdc_truly_pdata->panel_config_gpio)
			lcdc_truly_pdata->panel_config_gpio(1);
	
		spi_init(); /* LCD needs SPI */
		truly_disp_on();

	}
	



	return 0;
}
static struct platform_driver this_driver = {
	.probe  = truly_probe,
	.driver = {
		.name   = "lcdc_truly_wvga",
	},
	.suspend = lcd_suspend,
	.resume  = lcd_resume,
};

static struct msm_fb_panel_data truly_panel_data = {
	.on = lcdc_truly_panel_on,
	.off = lcdc_truly_panel_off,
	.set_backlight = lcdc_truly_set_backlight,
};

static struct platform_device this_device = {
	.name   = "lcdc_truly_wvga",
	.id	= 1,
	.dev	= {
		.platform_data = &truly_panel_data,
	}
};

//pengwei added for gp_clk get
#if defined (TYQ_PWM_CLK_GET_SUPPORT)
struct device gp_clk_device= {
	.init_name ="gp_clk.0",
		};
#endif

static int __init lcdc_truly_panel_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;
#if DRV_BACKLIGHT_CTL_RT9293B_SUPPORT

#if defined (TYQ_PWM_CLK_GET_SUPPORT)
			gp_clkto_pwm = clk_get(&gp_clk_device, "core_clk");
#else
			gp_clkto_pwm = clk_get(&(this_device.dev), "core_clk");
#endif
		
		
			if(IS_ERR(gp_clkto_pwm))
				printk(" clk_get gp_clk fail error !!!!!!\r\n");
			
#endif

	ret = platform_driver_register(&this_driver);
	if (ret)
		return ret;

	pinfo = &truly_panel_data.panel_info;
	pinfo->xres = 480;
	pinfo->yres = 800;
	MSM_FB_SINGLE_MODE_PANEL(pinfo);
	pinfo->type = LCDC_PANEL;
	pinfo->pdest = DISPLAY_1;
	pinfo->wait_cycle = 0;
	pinfo->bpp = 24;
	pinfo->fb_num = 2;
	/* 30Mhz mdp_lcdc_pclk and mdp_lcdc_pad_pcl */
	pinfo->clk_rate = 24576000; //24576000; //8192000; //30720000; 
	//pinfo->clk_rate =8192000;
	/*tydrv qupw modify for reduce blue noise LCD  50HZ  default 24576000 , according BP clkrgm_bsp_7627.c setting */
	
#if DRV_BACKLIGHT_CTL_RT9293B_SUPPORT
	pinfo->bl_max = 128;  //tydrv qupw modify for pwm duty 128 level DX2=255
#else
        pinfo->bl_max = 15;
#endif
	pinfo->bl_min = 1;

if(lcddrvic_id == OTM8018B_CHIP_ID)
	{
	 printk(" OTM8018B_CHIP_ID!!!!!!\r\n");
	pinfo->lcdc.h_back_porch = 28;	/* hsw = 8 + hbp=184 */
	pinfo->lcdc.h_front_porch = 28;
	pinfo->lcdc.h_pulse_width = 16;
	pinfo->lcdc.v_back_porch = 15;	/* vsw=1 + vbp = 2 */
	pinfo->lcdc.v_front_porch = 40;
	pinfo->lcdc.v_pulse_width = 3;
	}
else
	{
	printk(" HX_8369_CHIP_ID!!!!!!\r\n");
	pinfo->lcdc.h_back_porch = 21;	/* hsw = 8 + hbp=184 */
	pinfo->lcdc.h_front_porch = 21;
	pinfo->lcdc.h_pulse_width = 4;
	pinfo->lcdc.v_back_porch = 5;	/* vsw=1 + vbp = 2 */
	pinfo->lcdc.v_front_porch = 11;
	pinfo->lcdc.v_pulse_width = 4;
	}
	pinfo->lcdc.border_clr = 0;     /* blk */
	pinfo->lcdc.underflow_clr = 0xff;       /* blue */
	pinfo->lcdc.hsync_skew = 0;
      printk(" lcdinit!!!!!!\r\n");

	ret = platform_device_register(&this_device);
	if (ret) {
		printk(KERN_ERR "%s not able to register the device\n",
			 __func__);
		goto fail_driver;
	}
	return ret;

fail_driver:
	platform_driver_unregister(&this_driver);
	return ret;
}

module_init(lcdc_truly_panel_init);

