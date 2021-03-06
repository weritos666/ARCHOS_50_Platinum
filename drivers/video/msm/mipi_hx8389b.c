/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
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
 */

#define DEBUG

#include <mach/socinfo.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <mach/pmic.h>
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_hx8389b.h"
/*tydrv niuli add for keypad light misc control on 20111117*/
#include <linux/miscdevice.h>

/*tydrv niuli add for keypad light misc control on 20111117  begin*/
#define DRV_READ_LCDID_MISC_IOCTL 1
/*tydrv niuli add ffor keypad light misc control on 20111117  end*/

static struct msm_panel_common_pdata *mipi_hx8389b_pdata;
static struct dsi_buf hx8389b_tx_buf;
static struct dsi_buf hx8389b_rx_buf;

static int mipi_hx8389b_bl_ctrl = 0;

/* common setting */
static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

#define TYQ_LCD_DETECT

/*TYRD wuxh add for 5933 5 inch qhd mipi lcd support on 20121217*/

#ifdef TYQ_HX8389B_5INCH_QHD_SUPPORT

#include "ty_mipi_hx8389b_5inch.h"

#else //Default is for 5931 4.5 inch QHD lcd 
#if 1 //jy modify for effect 
/* panel setting */
static char video0[] = {
        0xB9, 0xFF, 0x83, 0x89,
};


static char video1[] = {
        0xB1, 0x00, 0x00, 0x07,
        0xEb, 0x50, 0x10, 0x11,
        0xb3, 0xf3, 0x2f, 0x37,
        0x1a, 0x1a, 0x43, 0x01,
        0x58, 0xF3, 0x00, 0xE6,  //jy 1204 
};

static char video2[] = {
        0xB2, 0x00, 0x00, 0x78,
        0x0C, 0x07, 0x00, 0xf0, //30--f0--80  jy 1204
};


static char video3[] = {
        0xB4, 0x92, 0x08, 0x00,
        0x32, 0x10, 0x04, 0x32,
        0x10, 0x00, 0x32, 0x10,
        0x00, 0x37, 0x0A, 0x40,
        0x08, 0x37, 0x08, 0x40,//08--00 40
        0x10, 0x52, 0x53, 0x0a, // jy 2dot 53--50; 0d--0a
};

static char video4[] = {
        0xD5, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00,
        0x00, 0x60, 0x00, 0x88,
        0x88, 0x88, 0x88, 0x88,
        0x23, 0x88, 0x01, 0x88,
        0x67, 0x88, 0x45, 0x01,
        0x23, 0x88, 0x88, 0x88,
        0x88, 0x88, 0x88, 0x88,
        0x88, 0x88, 0x88, 0x54,
        0x88, 0x76, 0x88, 0x10,
        0x88, 0x32, 0x32, 0x10,
        0x88, 0x88, 0x88, 0x88,
        0x88, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00,
};

static char video5[] = {0xCB, 0x07, 0x07};

static char video6[] = {0xBB, 0x00, 0x00, 0xFF, 0x80};

static char video7[] = {0xDE, 0x05, 0x58,0x10};

static char video8[] = {0xB6, 0x00, 0xa1, 0x00, 0xa1};//a4--aa--a3 jy 121226


static char video9[] = {
        0xE0, 0x05, 0x07, 0x16,
        0x31, 0x31, 0x3c, 0x38,
        0x52, 0x08, 0x0e, 0x0e,
        0x14, 0x17, 0x15, 0x17,
        0x11, 0x1a, 
        0x05, 0x07,
        0x16, 0x31, 0x31, 0x3c,
        0x38, 0x52, 0x08, 0x0e,
        0x0e, 0x14, 0x17, 0x15,
        0x17, 0x11, 0x1a,  // jy
}; 


/*static char video10[] = {
        0xC1, 0x01, 0x00, 0x08,
        0x0f, 0x17, 0x1C, 0x23,
        0x2b, 0x30, 0x36, 0x3e,
        0x46, 0x4d, 0x57, 0x5e,
        0x65, 0x6e, 0x76, 0x7e,
        0x87, 0x8e, 0x96, 0x9f,
        0xa8, 0xb0, 0xba, 0xc3,
        0xcb, 0xd3, 0xdd, 0xe4,
        0xed, 0xF7, 0xFd, 0x1f,
        0x57, 0x97, 0x2b, 0xd3,
        0xeb, 0x3f, 0x0c, 0x80,
        0x00, 0x08, 0x0f, 0x16,
        0x1b, 0x22, 0x2A, 0x2f,
        0x34, 0x3a, 0x43, 0x49,
        0x51, 0x59, 0x5F, 0x67,
        0x6f, 0x78, 0x7F, 0x87,
        0x8F, 0x97, 0x9f, 0xa8,
        0xb1, 0xba, 0xc4, 0xcc,
        0xd6, 0xdf, 0xE7, 0xF4,
        0xFD, 0x1f, 0x57, 0x97,
        0x2b, 0xd3, 0xeb, 0x3f,
        0x0c, 0x80, 0x00, 0x08,
        0x0f, 0x16, 0x1a, 0x1f,
        0x26, 0x2b, 0x31, 0x36,
        0x3d, 0x44, 0x4a, 0x50,
        0x58, 0x5e, 0x64, 0x6d,
        0x75, 0x7b, 0x83, 0x8c,
        0x94, 0x9b, 0xa5, 0xaD,
        0xb6, 0xC1, 0xcc, 0xd5,
        0xE0, 0xee, 0xFd, 0x1f,
        0x57, 0x97, 0x2b, 0xd3,
        0xEb, 0x3f, 0x0c, 0x80, 
};*/
static char video12[] = {0xC6, 0x08};//JY
static char video11[] = {0x21, 0x00};
/* HX8389B must use DCS commands */
#ifdef TYQ_LCD_DETECT
/*tydrv pengwei added for byd lcd start*/
static char BYD_video0[] = {
	0xb9, 0xff, 0x83, 0x89
};


static char BYD_video1[] = {
 	0xba,0x41,0x93,0x00, 
	0x16,0xa4,0x10,0x18  
};

static char BYD_video2[] = {
	0xb1,0x00,0x00,0x07, 
	0xeb,0x50,0x10,0x11, 
	0xb5,0x1f,0x2f,0x37, 
	0x1a,0x1a,0x43,0x01, 
	0x58,0xf2,0x00,0xe6  
};   
  
static char BYD_video3[] = {
	0xb2,0x00,0x00,0x78,
	0x0c,0x03,0x00,0xf0
};

static char BYD_video4[] = {          
	0xb4,0x91,0x08,0x00,        
	0x32,0x10,0x04,0x32,        
	0x10,0x00,0x32,0x10,        
	0x00,0x37,0x0a,0x40,        
	0x08,0x37,0x0a,0x40,        
	0x14,0x46,0x50,0x0a         
};             

static char BYD_video5[] = { 
	0xd5,0x00,0x00,0x00,  
	0x00,0x01,0x00,0x00,         
	0x00,0x60,0x00,0x88,  
	0x88,0x88,0x88,0x88,  
	0x23,0x88,0x01,0x88,  
	0x67,0x88,0x45,0x01,  
	0x23,0x88,0x88,0x88,  
	0x88,0x88,0x88,0x88,  
	0x88,0x88,0x88,0x54,  
	0x88,0x76,0x88,0x10,  
	0x88,0x32,0x32,0x10,  
	0x88,0x88,0x88,0x88,  
	0x88,0x00,0x00,0x00,  
	0x00,0x00,0x00,0x00,  
	0x00                  
};

static char BYD_video6[] = { 
	0xcb,0x07,0x07       
};      
       
static char BYD_video7[] = {
	0xbb,0x00,0x00,0xff,
	0x80
};


static char BYD_video8[] = {                             
	0xb6,0x00,0x9d,0x00,
	0x9d
};

static char BYD_video9[] = {
	0xe0,0x05,0x0b,0x14,
	0x2d,0x2e,0x3d,0x3c,
	0x51,0x0b,0x11,0x11,
	0x16,0x18,0x16,0x16,
	0x12,0x18,0x05,0x0b,
	0x14,0x2d,0x2e,0x3d,
	0x3c,0x51,0x0b,0x11,
	0x11,0x16,0x18,0x16,
	0x16,0x12,0x18
};

static char BYD_video10[] = {
	0xde,0x05,0x58
};

static char BYD_cmd[] = {
	0x21,0x00
};

static struct dsi_cmd_desc hx8389b_byd_video_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video0), BYD_video0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video1), BYD_video1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video2), BYD_video2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video3), BYD_video3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video4), BYD_video4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video5), BYD_video5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video6), BYD_video6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video7), BYD_video7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video8), BYD_video8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video9), BYD_video9},
	{DTYPE_DCS_WRITE,  1, 0, 0, 200, sizeof(BYD_cmd), BYD_cmd},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(BYD_video10), BYD_video10},
	{DTYPE_DCS_WRITE,  1, 0, 0, 1000, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE,  1, 0, 0, 200, sizeof(display_on), display_on},
};
/*tydrv pengwei added for byd lcd end*/
#endif

static struct dsi_cmd_desc hx8389b_video_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video0), video0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video1), video1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video2), video2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video3), video3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video4), video4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video5), video5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video6), video6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video7), video7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video8), video8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video9), video9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video12), video12},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video11), video11},
	{DTYPE_DCS_WRITE,  1, 0, 0, 150, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE,  1, 0, 0, 150, sizeof(display_on), display_on},
};

#else

/* panel setting */
static char video0[] = {
        0xB9, 0xFF, 0x83, 0x89,
};

static char video1[] = {
	0xB1, 0x00, 0x00, 0x04,
	0xE8, 0x50, 0x10, 0x11,
	0xB0, 0xF0, 0x2B, 0x33,
	0x1A, 0x1A, 0x43, 0x01,
	0x58, 0xF2, 0x00, 0xE6,
};

static char video2[] = {
	0xB2, 0x00, 0x00, 0x78,
	0x0D, 0x12, 0x00, 0x30,
};

static char video3[] = {
	0xB4, 0x80, 0x08, 0x00,
	0x32, 0x10, 0x05, 0x32,
	0x10, 0x00, 0x32, 0x10,
	0x00, 0x37, 0x0A, 0x40,
	0x08, 0x37, 0x0A, 0x40,
	0x14, 0x46, 0x50, 0x0A,
};

static char video4[] = {
	0xBA, 0x41, 0x93, 0x00,
	0x16, 0xA4, 0x00, 0x18,
	0xFF, 0x0F, 0x21, 0x03,
	0x21, 0x23, 0x25, 0x20,
	0x00, 0x35, 0x40,
};

static char video5[] = {
	0xD5, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00,
	0x00, 0x60, 0x00, 0x88,
	0x88, 0x88, 0x88, 0x88,
	0x23, 0x88, 0x01, 0x88,
	0x67, 0x88, 0x45, 0x01,
	0x23, 0x88, 0x88, 0x88,
	0x88, 0x88, 0x88, 0x88,
	0x88, 0x88, 0x88, 0x54,
	0x88, 0x76, 0x88, 0x10,
	0x88, 0x32, 0x32, 0x10,
	0x88, 0x88, 0x88, 0x88,
	0x88, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00,
};

static char video6[] = {0xCB, 0x07, 0x07};

static char video7[] = {0xBB, 0x00, 0x00, 0xFF, 0x80};

static char video8[] = {0xDE, 0x05, 0x58, 0x10};

static char video9[] = {0xB6, 0x00, 0xa0, 0x00, 0xa0};

static char video10[] = {
	0xE0, 0x05, 0x07, 0x16,
	0x2C, 0x2C, 0x3D, 0x33,
	0x4D, 0x08, 0x0E, 0x11,
	0x16, 0x18, 0x15, 0x16,
	0x10, 0x17, 0x05, 0x07,
	0x16, 0x2C, 0x2C, 0x3D,
	0x33, 0x4D, 0x08, 0x0E,
	0x11, 0x16, 0x18, 0x15,
	0x16, 0x10, 0x17,
};

static char video11[] = {0x21, 0x00};

/* HX8389B must use DCS commands */
static struct dsi_cmd_desc hx8389b_video_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video0), video0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video1), video1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video2), video2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video3), video3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video4), video4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video5), video5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video6), video6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video7), video7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video8), video8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video9), video9},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video10), video10},
	{DTYPE_DCS_WRITE,  1, 0, 0, 0, sizeof(video11), video11},
	{DTYPE_DCS_WRITE,  1, 0, 0, 150, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE,  1, 0, 0, 150, sizeof(display_on), display_on},
};
#endif

#endif
static struct dsi_cmd_desc hx8389b_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep}
};


static void kpd_bl_ctrl(unsigned int onoff)
{
    int ret;

    if (onoff) {
	ret=pmic_secure_mpp_config_i_sink(PM_MPP_8,PM_MPP__I_SINK__LEVEL_5mA,PM_MPP__I_SINK__SWITCH_ENA);
    } else {
	ret=pmic_secure_mpp_config_i_sink(PM_MPP_8,PM_MPP__I_SINK__LEVEL_5mA,PM_MPP__I_SINK__SWITCH_DIS);
    }

   if (ret)
	   printk("kpd_bl_ctrl led fail ret=%d\n",ret);

   return;

}

#if DRV_READ_LCDID_MISC_IOCTL
static int lcd_id_flag;
//extern int msm_fb_lcdc_config(int on);
//tydrv pengwei add for byd lcd
#if 1
#define LCD_ID   77
#else
#define LCD_ID  114
#endif
static int tyq_lcd_read_id(unsigned int gpio)
{	
	int rc = 2;

	rc = gpio_tlmm_config(GPIO_CFG(gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	if(rc){
		printk(KERN_ERR "tyq_lcd_read_id config failed\n");
		return -3;
	}
	
	rc = gpio_request(gpio, "lcd_id");
	if(rc){
		printk(KERN_ERR "%s: Failed to request GPIO %d\n",
		       __func__, gpio);
		gpio_free(gpio);
	}else{
		rc = gpio_direction_input(gpio);
		if (!rc){
			rc = gpio_get_value(gpio);
//			printk("----->>>%d \n",rc);
		}
		gpio_free(gpio);
	}

	return rc;
}
#endif
/*TYRD  WUXH ADD  begin for 5inch lcd support on 20130327*/		

#ifdef TYQ_HX8389B_5INCH_QHD_SUPPORT
int lcd_id = 0xff;
static int __init set_lcd_id(char *str)
{
	printk(KERN_ERR "set_lcd_id:[%s]\n", str);
	get_option(&str, &lcd_id);
	return 1;
}

__setup("lcd_id=", set_lcd_id);
#endif
/*TYRD	WUXH ADD end for 5inch lcd support on 20130327*/	

/*tydrv niuli add for keypad light misc control on 20111117*/
#if DRV_READ_LCDID_MISC_IOCTL
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
	char buf[11];
	char * str1 = "truly_lcd ";
	char * str2 = "byd_lcd";

	switch (cmd) {
		case TRULY_IOCTL_KPD_SET://set keppad light 	
			if (copy_from_user(&kpd_state, argp, sizeof(kpd_state))){
				ret = -EFAULT;
				break;
			}
			printk(KERN_INFO "truly_ioctl:--->>>>TRULY_IOCTL_KPD_SET  kpd_state=%d\r\n", kpd_state);	
			if(kpd_state){
				kpd_bl_ctrl(1);
			}
			else{
				kpd_bl_ctrl(0);
			}
				
			break;
 		case TRULY_IOCTL_GET_LCD_ID:
	
#ifdef TYQ_HX8389B_5INCH_QHD_SUPPORT
			printk("lcd id =%d\r\n",lcd_id);
			switch(lcd_id){
			case 3:				
				strcpy(buf,"tianma lcd");
			break;
			case 2:				
				strcpy(buf,"Truly2_lcd");
			break;
			case 1:				
				strcpy(buf,str1);
			break;
			case 0:				
				strcpy(buf,str2);
			break;
			default:				
				strcpy(buf,str1);
			break;
				}
#else
			if(lcd_id_flag){
				strcpy(buf,str1);
			}else{
				strcpy(buf,str2);
			}
#endif
			if(copy_to_user(argp,buf,strlen(buf))){
				printk("ty_lcd:TRULY_IOCTL_GET_LCD_ID read failed \n");
				return -EFAULT;
			}
			printk(KERN_INFO "truly_ioctl:--->>>>:TRULY_IOCTL_GET_LCD_ID\n lcd_id=%d device is (%s)\n",lcd_id_flag,buf);
				
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

static int mipi_hx8389b_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;

	pr_debug("mipi_hx8389b_lcd_on E\n");

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi  = &mfd->panel_info.mipi;

	if (!mfd->cont_splash_done) {
		mfd->cont_splash_done = 1;
		return 0;
	}

	if (mipi->mode == DSI_VIDEO_MODE) {
#ifdef TYQ_LCD_DETECT
//tydrv add for byd lcd
	/*TYRD	WUXH ADD  begin for 5inch lcd support on 20130327*/ 	
#ifdef TYQ_HX8389B_5INCH_QHD_SUPPORT

	switch(lcd_id){
	case 3:
		
		mipi_dsi_cmds_tx(&hx8389b_tx_buf,
			nt35517_tianma_video_display_on_cmds,
			ARRAY_SIZE(nt35517_tianma_video_display_on_cmds));		
		break;
		
	case 2:
		
		mipi_dsi_cmds_tx(&hx8389b_tx_buf,
			hx8389b_video_display_on_truly2_cmds,
			ARRAY_SIZE(hx8389b_video_display_on_truly2_cmds));		
		break;
	
	case 1:
		
		mipi_dsi_cmds_tx(&hx8389b_tx_buf,
			hx8389b_video_display_on_cmds,
			ARRAY_SIZE(hx8389b_video_display_on_cmds));		
		break;
	
		
	case 0:
		
		mipi_dsi_cmds_tx(&hx8389b_tx_buf,
			hx8389b_byd_video_display_on_cmds,
			ARRAY_SIZE(hx8389b_byd_video_display_on_cmds));	

				
		break;

	case 4:
		
		mipi_dsi_cmds_tx(&hx8389b_tx_buf,
			hx8389b_video_display_on_truly3_cmds,
			ARRAY_SIZE(hx8389b_video_display_on_truly3_cmds));	

				
		break;
		
			
	default:
		
		mipi_dsi_cmds_tx(&hx8389b_tx_buf,
			hx8389b_video_display_on_cmds,
			ARRAY_SIZE(hx8389b_video_display_on_cmds));	
		break;
		}

#else
	if(lcd_id_flag)
		{
		mipi_dsi_cmds_tx(&hx8389b_tx_buf,
			hx8389b_video_display_on_cmds,
			ARRAY_SIZE(hx8389b_video_display_on_cmds));		
			//printk("mipi_hx8389b_lcd_on truly cmd\r\n");
		}
	else
		{
		mipi_dsi_cmds_tx(&hx8389b_tx_buf,
			hx8389b_byd_video_display_on_cmds,
			ARRAY_SIZE(hx8389b_byd_video_display_on_cmds));
			//printk("mipi_hx8389b_lcd_on byd cmd\r\n");
		}

#endif 
#else
	mipi_dsi_cmds_tx(&hx8389b_tx_buf,
		hx8389b_video_display_on_cmds,
		ARRAY_SIZE(hx8389b_video_display_on_cmds));

#endif
	}

	pr_debug("mipi_hx8389b_lcd_on X\n");

	return 0;
}

static int mipi_hx8389b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	pr_debug("mipi_hx8389b_lcd_off E\n");

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(&hx8389b_tx_buf, hx8389b_display_off_cmds,
			ARRAY_SIZE(hx8389b_display_off_cmds));

	pr_debug("mipi_hx8389b_lcd_off X\n");
	return 0;
}

static ssize_t mipi_hx8389b_wta_bl_ctrl(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int err;

	err =  kstrtoint(buf, 0, &mipi_hx8389b_bl_ctrl);
	if (err)
		return ret;

	pr_info("%s: bl ctrl set to %d\n", __func__, mipi_hx8389b_bl_ctrl);

	return ret;
}

static DEVICE_ATTR(bl_ctrl, S_IWUSR, NULL, mipi_hx8389b_wta_bl_ctrl);

static struct attribute *mipi_hx8389b_fs_attrs[] = {
	&dev_attr_bl_ctrl.attr,
	NULL,
};

static struct attribute_group mipi_hx8389b_fs_attr_group = {
	.attrs = mipi_hx8389b_fs_attrs,
};

static int mipi_hx8389b_create_sysfs(struct platform_device *pdev)
{
	int rc;
	struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);

	if (!mfd) {
		pr_err("%s: mfd not found\n", __func__);
		return -ENODEV;
	}
	if (!mfd->fbi) {
		pr_err("%s: mfd->fbi not found\n", __func__);
		return -ENODEV;
	}
	if (!mfd->fbi->dev) {
		pr_err("%s: mfd->fbi->dev not found\n", __func__);
		return -ENODEV;
	}
	rc = sysfs_create_group(&mfd->fbi->dev->kobj,
		&mipi_hx8389b_fs_attr_group);
	if (rc) {
		pr_err("%s: sysfs group creation failed, rc=%d\n",
			__func__, rc);
		return rc;
	}

	return 0;
}

static int __devinit mipi_hx8389b_lcd_probe(struct platform_device *pdev)
{
	struct platform_device *pthisdev = NULL;
	pr_debug("%s\n", __func__);

	if (pdev->id == 0) {
		mipi_hx8389b_pdata = pdev->dev.platform_data;
		if (mipi_hx8389b_pdata->bl_lock)
			spin_lock_init(&mipi_hx8389b_pdata->bl_spinlock);

                /* SKUD use PWM as backlight control method */
                if(machine_is_msm8625q_skud() || machine_is_msm8625q_evbd()) {
                        mipi_hx8389b_bl_ctrl = 1;
                }

		return 0;
	}
#if DRV_READ_LCDID_MISC_IOCTL
		msleep(2);
		lcd_id_flag = tyq_lcd_read_id(LCD_ID);
		printk(KERN_ERR "--TY--lcd:truly_probe id=%d(0:byd 1:truly)\n",lcd_id_flag);
		truly_misc_data = pdev;
		misc_register(&truly_device);
#endif

	pthisdev = msm_fb_add_device(pdev);
	mipi_hx8389b_create_sysfs(pthisdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_hx8389b_lcd_probe,
	.driver = {
		.name   = "mipi_hx8389b",
	},
};

static int old_bl_level;

static void mipi_hx8389b_set_backlight(struct msm_fb_data_type *mfd)
{
	int bl_level;
	unsigned long flags;
	bl_level = mfd->bl_level;

	if (mipi_hx8389b_pdata->bl_lock) {
		if (!mipi_hx8389b_bl_ctrl) {
			/* Level received is of range 1 to bl_max,
			   We need to convert the levels to 1
			   to 31 */
			bl_level = (2 * bl_level * 31 + mfd->panel_info.bl_max)
					/(2 * mfd->panel_info.bl_max);
			if (bl_level == old_bl_level)
				return;

			if (bl_level == 0)
				mipi_hx8389b_pdata->backlight(0, 1);

			if (old_bl_level == 0)
				mipi_hx8389b_pdata->backlight(50, 1);

			spin_lock_irqsave(&mipi_hx8389b_pdata->bl_spinlock,
						flags);
			mipi_hx8389b_pdata->backlight(bl_level, 0);
			spin_unlock_irqrestore(&mipi_hx8389b_pdata->bl_spinlock,
						flags);
			old_bl_level = bl_level;
		} else {
			mipi_hx8389b_pdata->backlight(bl_level, 1);
		}
	} else {
		mipi_hx8389b_pdata->backlight(bl_level, mipi_hx8389b_bl_ctrl);
	}
}

static struct msm_fb_panel_data hx8389b_panel_data = {
	.on	= mipi_hx8389b_lcd_on,
	.off = mipi_hx8389b_lcd_off,
	.set_backlight = mipi_hx8389b_set_backlight,
};

static int ch_used[3];

static int mipi_hx8389b_lcd_init(void)
{
	mipi_dsi_buf_alloc(&hx8389b_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&hx8389b_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

int mipi_hx8389b_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	ret = mipi_hx8389b_lcd_init();
	if (ret) {
		pr_err("mipi_hx8389b_lcd_init() failed with ret %u\n", ret);
		return ret;
	}

	pdev = platform_device_alloc("mipi_hx8389b", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	hx8389b_panel_data.panel_info = *pinfo;
	ret = platform_device_add_data(pdev, &hx8389b_panel_data,
				sizeof(hx8389b_panel_data));
	if (ret) {
		pr_debug("%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		pr_debug("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}
