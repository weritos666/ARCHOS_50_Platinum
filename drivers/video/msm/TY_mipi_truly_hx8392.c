/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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
#if defined(TYQ_HX8392_MIPI_HD_SUPPORT)
#include <mach/socinfo.h>
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "TY_mipi_truly_hx8392.h"
#include <mach/gpio.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <mach/pmic.h>
/*tydrv niuli add for keypad light misc control on 20111117*/
#include <linux/miscdevice.h>


static struct msm_panel_common_pdata *mipi_hx8392_pdata;
static struct dsi_buf hx8392_tx_buf;
static struct dsi_buf hx8392_rx_buf;


#define HX8392_CMD_DELAY		0
#define HX8392_SLEEP_OFF_DELAY	150
#define HX8392_DISPLAY_ON_DELAY	20

static int mipi_hx8392_bl_ctrl = 0;

/*tydrv niuli add for keypad light misc control on 20111117  begin*/
#define DRV_READ_LCDID_MISC_IOCTL 1
/*tydrv niuli add ffor keypad light misc control on 20111117  end*/

static char video0[]= {0xB9,0xFF,0x83,0x92};
static char video1[] ={0x11, 0x00};


static char video2[] = {0x35,0x00};
static char video3[] = {0xB1,0x7C,0x00,0x44,0x14,0x00,0x11,0x11,0x24,0x2c,0x3F,0x3F,0x42,0x72,};//Set Power 130606
static char video4[] = {0xB4,0x00,0x00,0x05,0x00,0xA0,0x05,0x16,0x9f,0x30,0x03,0x06,0x00,0x03,0x03,0x00,0x23,0x04,0x03,0x03,0x01,0x00,0x1A,0x9f};//SET MPU/Command CYC 130503
static char video5[] = {0xB6,0x77};//VCOMDC 70-77 130711
static char video6[] = {0xBA,0x13,0x83};
static char video7[] = {0xC2,0x03};//0x08->Command Mode
static char video8[] = {0xBF,0x05,0xE0,0x02,0x00};
static char video9[] = {0xC0,0x01,0x94};

static char video11[] = {0xC6,0x45,0x02,0x10,0x04};
static char video12[] = {0xC7,0x00,0x40};
static char video13[] = {0xCC,0x08};//SETPANEL
static char video14[] = {0xD4,0x0C};//Set EQ function for LTPS
static char video15[] = {0xD5,0x00,0x08,0x08,0x00,0x44,0x55,0x66,0x77,0xCC,0xCC,0xCC,0xCC,0x00,0x77,0x66,0x55,0x44,0xCC,0xCC,0xCC,0xCC};//Set LTPS control output 
static char video16[] = {0xD8,0x00,0x00,0x05,0x00,0xA0,0x05,0x16,0x9f,0x30,0x03,0x06,0x00,0x03,0x03,0x00,0x23,0x04,0x03,0x03,0x01,0x00,0x1A,0x9f};//SET RGB/Video CYC
//---------------gamma2.2--gamma2.4  130711-----------------------
static char video17[] ={0xE0,0x03,0x1B,0x1f,0x3f,0x3f,0x3f,0x2B,0x42,0x05,0x0d,0x0d,0x11,0x13,0x11,0x11,0x0d,0x1A,
      0x03,0x1B,0x1f,0x3f,0x3f,0x3f,0x35,0x4b,0x05,0x0d,0x0d,0x11,0x13,0x11,0x11,0x0d,0x1A,};//R Gamma 
static char video18[] = {0xE1,0x03,0x1B,0x1f,0x3f,0x3f,0x3f,0x2B,0x42,0x05,0x0d,0x0d,0x11,0x13,0x11,0x11,0x0d,0x1A,
      0x03,0x1B,0x1f,0x3f,0x3f,0x3f,0x35,0x4b,0x05,0x0d,0x0d,0x11,0x13,0x11,0x11,0x0d,0x1A,};//G Gamma 
static char video19[] = {0xE2,0x03,0x1B,0x1f,0x3f,0x3f,0x3f,0x2B,0x42,0x05,0x0d,0x0d,0x11,0x13,0x11,0x11,0x0d,0x1A,
      0x03,0x1B,0x1f,0x3f,0x3f,0x3f,0x35,0x4b,0x05,0x0d,0x0d,0x11,0x13,0x11,0x11,0x0d,0x1A,};//B Gamma 

static char video20[] = {0xB2,0x08,0xC8,0x05,0x0F,0x08,0x44,0x00,0xFF,0x05,0x0F,0x04,0x20};//Set display related register
//static char video20[] = {0xB2, 0x00, 0x00, 0x78,
  //      0x08, 0x03, 0x00, 0xF0,
//};
static char video21[] = {0x29, 0x00};

//static char reg_addr21[2] = {0x21, 0x00};

//static char exit_sleep[2] = {0x11, 0x00};
//static char display_on[2] = {0x29, 0x00};
#if 0
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

static char display_off1[] = {0xba, 0x13,0x83,0x00,0x16,0xc5,0x10};

//static char enter_sleep1[] = {0x78, 0x00};

static struct dsi_cmd_desc hx8392_display_off_cmds[] = {
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video0), video0},

	{DTYPE_DCS_WRITE, 1, 0, 0, 10, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep},
	
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video0), video0},	
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 20, sizeof(display_off1), display_off1}
	//{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(enter_sleep1), enter_sleep1}
};
#endif
static struct dsi_cmd_desc hx8392_video_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video0), video0},
	{DTYPE_DCS_WRITE, 1, 0, 0, 150, sizeof(video1), video1},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video2), video2},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video3), video3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video4), video4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video5), video5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video6), video6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video7), video7},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video8), video8},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video9), video9},
	//{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video10), video10},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video11), video11},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video12), video12},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video13), video13},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video14), video14},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video15), video15},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video16), video16},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video17), video17},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video18), video18},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video19), video19},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 0, sizeof(video20), video20},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(video21), video21},

};

static char tianma_exit_sleep[2] = {0x11, 0x00};
static char tianma_display_on[2] = {0x29, 0x00};

static char tianma_video0[] = {
	0xF0,0x55,0xAA,0x52,
	0x08,0x00
};
static char tianma_video1[] = {
	0xFF,0xAA,0x55,0xA5,
	0x80
};
static char tianma_video2[] = {
	0x6F,0x11,0x00
};
static char tianma_video3[] = {
	0xF7,0x20,0x00
};
static char tianma_video4[] = {
	0xBD,0x01,0xA0,0x0C,
	0x08,0x01
};
static char tianma_video5[] = {
	0x6F,0x02
};

static char tianma_video6[] = {
	0xB8,0x02
};
static char tianma_video7[] = {
	0xBB,0x11,0x11
};
static char tianma_video8[] = {
	0xBC,0x00,0x00
};

static char tianma_video9[] = { 0xB6,0x01 };
static char tianma_video10[] = { 0xF0,0x55,0xAA,0x52,0x08,0x01 };
static char tianma_video11[] = { 0xB0,0x09,0x09 };
static char tianma_video12[] = { 0xB1,0x09,0x09 };
static char tianma_video13[] = { 0xBC,0x68,0x00 };
static char tianma_video14[] = { 0xBD,0x68,0x00 };
static char tianma_video15[] = { 0xCA,0x00 };
static char tianma_video16[] = { 0xC0,0x04 };
static char tianma_video17[] = { 0xB5,0x03,0x03 };
static char tianma_video18[] = { 0xBE,0x6B };
static char tianma_video19[] = { 0xB3,0x1B,0x1B };
static char tianma_video20[] = { 0xB4,0x0F,0x0F };
static char tianma_video21[] = { 0xB9,0x26,0x26 };
static char tianma_video22[] = { 0xBA,0x14,0x14 };

static char tianma_video23[] = { 0xF0,0x55,0xAA,0x52,0x08,0x02 };
static char tianma_video24[] = { 0xEE,0x01 };
static char tianma_video25[] = { 0xB0,0x00,0x20,0x00,0x36,0x00,0x5A,0x00,0x77,0x00,0x8F,0x00,0xBA,0x00,0xDC,0x01,0x12 };
static char tianma_video26[] = { 0xB1,0x01,0x3E,0x01,0x83,0x01,0xB8,0x02,0x0A,0x02,0x4B,0x02,0x4D,0x02,0x88,0x02,0xC9 };
static char tianma_video27[] = { 0xB2,0x02,0xF2,0x03,0x29,0x03,0x4D,0x03,0x7A,0x03,0x97,0x03,0xB9,0x03,0xD1,0x03,0xD8 };
static char tianma_video28[] = { 0xB3,0x03,0xD8,0x03,0xFF };
static char tianma_video29[] = { 0x6F,0x02 };
static char tianma_video30[] = { 0xF7,0x47 };

static char tianma_video31[] = { 0x6F,0x0A };
static char tianma_video32[] = { 0xF7,0x02 };
	         
static char tianma_video33[] = { 0x6F,0x17 };
static char tianma_video34[] = { 0xF4,0x70 };
	         
static char tianma_video35[] = { 0x6F,0x11 };
static char tianma_video36[] = { 0xF3,0x01 };


static char tianma_video37[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x06 };

static char tianma_video38[] = { 0xB0, 0x10,0x12 };
static char tianma_video39[] = { 0xB1, 0x16,0x18 };
static char tianma_video40[] = { 0xB2, 0x00,0x02 };
static char tianma_video41[] = { 0xB3, 0x31,0x31 };
static char tianma_video42[] = { 0xB4, 0x31,0x31 };
static char tianma_video43[] = { 0xB5, 0x31,0x31 };
static char tianma_video44[] = { 0xB6, 0x31,0x31 };
static char tianma_video45[] = { 0xB7, 0x31,0x31 };
static char tianma_video46[] = { 0xB8, 0x31,0x08 };
static char tianma_video47[] = { 0xB9, 0x2D,0x2E };
static char tianma_video48[] = { 0xBA, 0x2E,0x2D };
static char tianma_video49[] = { 0xBB, 0x09,0x31 };
static char tianma_video50[] = { 0xBC, 0x31,0x31 };
static char tianma_video51[] = { 0xBD, 0x31,0x31 };
static char tianma_video52[] = { 0xBE, 0x31,0x31 };
static char tianma_video53[] = { 0xBF, 0x31,0x31 };
static char tianma_video54[] = { 0xC0, 0x31,0x31 };
static char tianma_video55[] = { 0xC1, 0x03,0x01 };
static char tianma_video56[] = { 0xC2, 0x19,0x17 };
static char tianma_video57[] = { 0xC3, 0x13,0x11 };
static char tianma_video58[] = { 0xE5, 0x31,0x31 };

static char tianma_video59[] = { 0xC4, 0x19,0x17 };
static char tianma_video60[] = { 0xC5, 0x13,0x11 };
static char tianma_video61[] = { 0xC6, 0x03,0x01 };
static char tianma_video62[] = { 0xC7, 0x31,0x31 };
static char tianma_video63[] = { 0xC8, 0x31,0x31 };
static char tianma_video64[] = { 0xC9, 0x31,0x31 };
static char tianma_video65[] = { 0xCA, 0x31,0x31 };
static char tianma_video66[] = { 0xCB, 0x31,0x31 };
static char tianma_video67[] = { 0xCC, 0x31,0x09 };
static char tianma_video68[] = { 0xCD, 0x2E,0x2D };
static char tianma_video69[] = { 0xCE, 0x2D,0x2E };
static char tianma_video70[] = { 0xCF, 0x08,0x31 };
static char tianma_video71[] = { 0xD0, 0x31,0x31 };
static char tianma_video72[] = { 0xD1, 0x31,0x31 };
static char tianma_video73[] = { 0xD2, 0x31,0x31 };
static char tianma_video74[] = { 0xD3, 0x31,0x31 };
static char tianma_video75[] = { 0xD4, 0x31,0x31 };
static char tianma_video76[] = { 0xD5, 0x00,0x02 };
static char tianma_video77[] = { 0xD6, 0x10,0x12 };
static char tianma_video78[] = { 0xD7, 0x16,0x18 };
static char tianma_video79[] = { 0xD8, 0x00,0x00,0x00,0x00,0x00 };
static char tianma_video80[] = { 0xD9, 0x00,0x00,0x00,0x00,0x00 };
static char tianma_video81[] = { 0xE7, 0x00 };

static char tianma_video82[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x05 };
static char tianma_video83[] = { 0xED, 0x30 };
	                              	
	                              	
	                              	
static char tianma_video84[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x03 };
static char tianma_video85[] = { 0xB1, 0x00,0x00 };
static char tianma_video86[] = { 0xB0, 0x00,0x00 };
	                              	
	                              	
	                              	
static char tianma_video87[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x05 };
static char tianma_video88[] = { 0xE5, 0x00 };

static char tianma_video89[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x05 };
static char tianma_video90[] = { 0xB0, 0x17,0x06 };
static char tianma_video91[] = { 0xB8, 0x00 };
		
static char tianma_video92[] = { 0xBD, 0x03,0x03,0x01,0x00,0x03 };
static char tianma_video93[] = { 0xB1, 0x17,0x06 };
static char tianma_video94[] = { 0xB9, 0x00,0x03 };
static char tianma_video95[] = { 0xB2, 0x17,0x06 };
static char tianma_video96[] = { 0xBA, 0x00,0x00 };
static char tianma_video97[] = { 0xB3, 0x17,0x06 };
static char tianma_video98[] = { 0xBB, 0x00,0x00 };
static char tianma_video99[] = { 0xB4, 0x17,0x06 };
static char tianma_video100[] = { 0xB5, 0x17,0x06 };
static char tianma_video101[] = { 0xB6, 0x17,0x06 };
static char tianma_video102[] = { 0xB7, 0x17,0x06 };
static char tianma_video103[] = { 0xBC, 0x00,0x03 };
static char tianma_video104[] = { 0xE5, 0x06 };
static char tianma_video105[] = { 0xE6, 0x06 };
static char tianma_video106[] = { 0xE7, 0x06 };
static char tianma_video107[] = { 0xE8, 0x06 };
static char tianma_video108[] = { 0xE9, 0x06 };
static char tianma_video109[] = { 0xEA, 0x06 };
static char tianma_video110[] = { 0xEB, 0x06 };
static char tianma_video111[] = { 0xEC, 0x06 };

static char tianma_video112[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x05 };
static char tianma_video113[] = { 0xC0, 0x0B };
static char tianma_video114[] = { 0xC1, 0x09 };
static char tianma_video115[] = { 0xC2, 0x0B };
static char tianma_video116[] = { 0xC3, 0x09 };
		
static char tianma_video117[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x03 };
static char tianma_video118[] = { 0xB2, 0x05,0x00,0x00,0x00,0x00 };
static char tianma_video119[] = { 0xB3, 0x05,0x00,0x00,0x00,0x00 };
static char tianma_video120[] = { 0xB4, 0x05,0x00,0x00,0x00,0x00 };
static char tianma_video121[] = { 0xB5, 0x05,0x00,0x00,0x00,0x00 };
		
static char tianma_video122[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x05 };
static char tianma_video123[] = { 0xC4, 0x10 };
static char tianma_video124[] = { 0xC5, 0x10 };
static char tianma_video125[] = { 0xC6, 0x10 };
static char tianma_video126[] = { 0xC7, 0x10 };
		
static char tianma_video127[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x03 };
static char tianma_video128[] = { 0xB6, 0x05,0x00,0x00,0x00,0x00 };
static char tianma_video129[] = { 0xB7, 0x05,0x00,0x00,0x00,0x00 };
static char tianma_video130[] = { 0xB8, 0x05,0x00,0x00,0x00,0x00 };
static char tianma_video131[] = { 0xB9, 0x05,0x00,0x00,0x00,0x00 };
		
static char tianma_video132[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x05 };
static char tianma_video133[] = { 0xC8, 0x07,0x20 };
static char tianma_video134[] = { 0xC9, 0x03,0x20 };
static char tianma_video135[] = { 0xCA, 0x07,0x00 };
static char tianma_video136[] = { 0xCB, 0x03,0x00 };
		
static char tianma_video137[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x03 };
static char tianma_video138[] = { 0xBA, 0x44,0x00,0xD0,0x00,0xD0 };
static char tianma_video139[] = { 0xBB, 0x44,0x00,0xD0,0x00,0xD0 };
static char tianma_video140[] = { 0xBC, 0x44,0x00,0xD0,0x00,0xD0 };
static char tianma_video141[] = { 0xBD, 0x44,0x00,0xD0,0x00,0xD0 };
	                                                            	
	                                                            	
static char tianma_video142[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x05 };
static char tianma_video143[] = { 0xD1, 0x00,0x05,0x00,0x07,0x10 };
static char tianma_video144[] = { 0xD2, 0x00,0x05,0x04,0x07,0x10 };
static char tianma_video145[] = { 0xD3, 0x00,0x00,0x0A,0x07,0x10 };
static char tianma_video146[] = { 0xD4, 0x00,0x00,0x0A,0x07,0x10 };

static char tianma_video147[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x05 };
static char tianma_video148[] = { 0xD0, 0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
static char tianma_video149[] = { 0xD5, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
static char tianma_video150[] = { 0xD6, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
static char tianma_video151[] = { 0xD7, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
		
static char tianma_video152[] = { 0xD8, 0x00,0x00,0x00,0x00,0x00 };
		
static char tianma_video153[] = { 0xF0, 0x55,0xAA,0x52,0x08,0x03 };
static char tianma_video154[] = { 0xC4, 0x60 };
static char tianma_video155[] = { 0xC5, 0x40 };
static char tianma_video156[] = { 0xC6, 0x60 };
static char tianma_video157[] = { 0xC7, 0x40 };
		
static char tianma_video158[] = { 0x6F,0x01 };
static char tianma_video159[] = { 0xF9,0x46 };

static struct dsi_cmd_desc nt35521_tianma_video_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video0 ), tianma_video0 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video1 ), tianma_video1 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video2 ), tianma_video2 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video3 ), tianma_video3 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video4 ), tianma_video4 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video5 ), tianma_video5 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video6 ), tianma_video6 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video7 ), tianma_video7 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video8 ), tianma_video8 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video9 ), tianma_video9 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video10 ), tianma_video10 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video11 ), tianma_video11 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video12 ), tianma_video12 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video13 ), tianma_video13 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video14 ), tianma_video14 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video15 ), tianma_video15 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video16 ), tianma_video16 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video17 ), tianma_video17 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video18 ), tianma_video18 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video19 ), tianma_video19 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video20 ), tianma_video20 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video21 ), tianma_video21 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video22 ), tianma_video22 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video23 ), tianma_video23 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video24 ), tianma_video24 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video25 ), tianma_video25 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video26 ), tianma_video26 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video27 ), tianma_video27 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video28 ), tianma_video28 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video29 ), tianma_video29 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video30 ), tianma_video30 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video31 ), tianma_video31 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video32 ), tianma_video32 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video33 ), tianma_video33 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video34 ), tianma_video34 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video35 ), tianma_video35 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video36 ), tianma_video36 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video37 ), tianma_video37 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video38 ), tianma_video38 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video39 ), tianma_video39 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video40 ), tianma_video40 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video41 ), tianma_video41 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video42 ), tianma_video42 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video43 ), tianma_video43 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video44 ), tianma_video44 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video45 ), tianma_video45 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video46 ), tianma_video46 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video47 ), tianma_video47 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video48 ), tianma_video48 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video49 ), tianma_video49 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video50 ), tianma_video50 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video51 ), tianma_video51 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video52 ), tianma_video52 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video53 ), tianma_video53 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video54 ), tianma_video54 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video55 ), tianma_video55 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video56 ), tianma_video56 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video57 ), tianma_video57 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video58 ), tianma_video58 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video59 ), tianma_video59 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video60 ), tianma_video60 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video61 ), tianma_video61 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video62 ), tianma_video62 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video63 ), tianma_video63 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video64 ), tianma_video64 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video65 ), tianma_video65 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video66 ), tianma_video66 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video67 ), tianma_video67 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video68 ), tianma_video68 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video69 ), tianma_video69 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video70 ), tianma_video70 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video71 ), tianma_video71 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video72 ), tianma_video72 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video73 ), tianma_video73 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video74 ), tianma_video74 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video75 ), tianma_video75 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video76 ), tianma_video76 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video77 ), tianma_video77 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video78 ), tianma_video78 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video79 ), tianma_video79 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video80 ), tianma_video80 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video81 ), tianma_video81 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video82 ), tianma_video82 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video83 ), tianma_video83 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video84 ), tianma_video84 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video85 ), tianma_video85 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video86 ), tianma_video86 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video87 ), tianma_video87 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video88 ), tianma_video88 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video89 ), tianma_video89 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video90 ), tianma_video90 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video91 ), tianma_video91 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video92 ), tianma_video92 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video93 ), tianma_video93 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video94 ), tianma_video94 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video95 ), tianma_video95 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video96 ), tianma_video96 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video97 ), tianma_video97 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video98 ), tianma_video98 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video99 ), tianma_video99 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video100 ), tianma_video100 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video101 ), tianma_video101 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video102 ), tianma_video102 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video103 ), tianma_video103 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video104 ), tianma_video104 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video105 ), tianma_video105 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video106 ), tianma_video106 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video107 ), tianma_video107 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video108 ), tianma_video108 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video109 ), tianma_video109 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video110 ), tianma_video110 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video111 ), tianma_video111 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video112 ), tianma_video112 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video113 ), tianma_video113 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video114 ), tianma_video114 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video115 ), tianma_video115 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video116 ), tianma_video116 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video117 ), tianma_video117 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video118 ), tianma_video118 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video119 ), tianma_video119 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video120 ), tianma_video120 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video121 ), tianma_video121 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video122 ), tianma_video122 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video123 ), tianma_video123 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video124 ), tianma_video124 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video125 ), tianma_video125 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video126 ), tianma_video126 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video127 ), tianma_video127 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video128 ), tianma_video128 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video129 ), tianma_video129 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video130 ), tianma_video130 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video131 ), tianma_video131 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video132 ), tianma_video132 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video133 ), tianma_video133 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video134 ), tianma_video134 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video135 ), tianma_video135 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video136 ), tianma_video136 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video137 ), tianma_video137 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video138 ), tianma_video138 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video139 ), tianma_video139 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video140 ), tianma_video140 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video141 ), tianma_video141 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video142 ), tianma_video142 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video143 ), tianma_video143 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video144 ), tianma_video144 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video145 ), tianma_video145 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video146 ), tianma_video146 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video147 ), tianma_video147 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video148 ), tianma_video148 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video149 ), tianma_video149 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video150 ), tianma_video150 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video151 ), tianma_video151 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video152 ), tianma_video152 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video153 ), tianma_video153 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video154 ), tianma_video154 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video155 ), tianma_video155 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video156 ), tianma_video156 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video157 ), tianma_video157 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video158 ), tianma_video158 },
	{DTYPE_DCS_LWRITE,1,0,0,0 , sizeof( tianma_video159 ), tianma_video159 },
	{DTYPE_DCS_WRITE, 1,0,0,120 ,sizeof(tianma_exit_sleep), tianma_exit_sleep },
	{DTYPE_DCS_WRITE, 1,0,0,0 ,sizeof(tianma_display_on), tianma_display_on },

};

#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT) && defined (TYQ_SSD2080M_MIPI_HD_SUPPORT)
static char ssd2080m_exit_sleep[2] = { 0x11, 0x00};
static char ssd2080m_display_on[2] = { 0x29, 0x00};
static char ssd2080m_video0[] = { 0xFF,0x01};
static char ssd2080m_video1[] = { 0xC6,0x63,0x00,0x81,0x31};
static char ssd2080m_video2[] = { 0xCB,0xE7,0x80,0x73,0x33};
static char ssd2080m_video3[] = { 0xEC,0xD2};
static char ssd2080m_video4[] = { 0xB3,0x04,0x9F};
static char ssd2080m_video5[] = { 0xB2,0x16,0x1E,0x10};     // 0xB2,0x16,0x1E,0x10   0x23
static char ssd2080m_video6[] = { 0xB4,0x00};//inversion mode
static char ssd2080m_video7[] = { 0xC1,0x04};
static char ssd2080m_video8[] = { 0xC2,0xBE,0x00,0x58}; //0x00<=0x11
static char ssd2080m_video9[] = { 0xC3,0x01,0x22,0x11,0x21,0x0E,0x80,0x80,0x24};
static char ssd2080m_video10[] = { 0xB6,0x09,0x16,0x42,0x01,0x13,0x00,0x00};
static char ssd2080m_video11[] = { 0xB7,0x24,0x26,0x43};
static char ssd2080m_video12[] = { 0xB8,0x16,0x08,0x25,0x44,0x08};
static char ssd2080m_video13[] = { 0xB9,0x06,0x08,0x07,0x09,0x00,0x00,0x00,0x00};
static char ssd2080m_video14[] = { 0xBA,0x0E,0x10,0x0A,0x0C,0x16,0x05,0x00,0x00};
static char ssd2080m_video15[] = { 0xBB,0xA1,0xA1,0xA1,0xA1,0x00,0x00,0x00,0x00};
static char ssd2080m_video16[] = { 0xBC,0x0F,0x11,0x0B,0x0D,0x16,0x05,0x00,0x00};
static char ssd2080m_video17[] = { 0xBD,0xA1,0xA1,0xA1,0xA1,0x00,0x00,0x00,0x00};
static char ssd2080m_video18[] = { 0xE6,0xFF,0xFF,0x0F};
static char ssd2080m_video19[] = { 0xC7,0x2F};
static char ssd2080m_video20[] = { 0xB5,0x47,0x00,0x00,0x08,0x00,0x01};   //5F    ssl thange to 0x47
static char ssd2080m_video21[] = { 0xC4,0xDF,0x72,0x12,0x12,0x66,0xE3,0x99};//0x66 VCOM  c4,d3
static char ssd2080m_video22[] = { 0xD0,0x0A,0x00,0x0D,0x15,0x1F,0x2E};
static char ssd2080m_video23[] = { 0xD1,0x28,0x27,0x14,0x02,0x01};
static char ssd2080m_video24[] = { 0xD2,0x0A,0x00,0x0D,0x15,0x1F,0x2E};
static char ssd2080m_video25[] = { 0xD3,0x28,0x27,0x14,0x02,0x01};
static char ssd2080m_video26[] = { 0xD4,0x0A,0x00,0x0D,0x15,0x1F,0x2E};
static char ssd2080m_video27[] = { 0xD5,0x28,0x27,0x14,0x02,0x01};
static char ssd2080m_video28[] = { 0xD6,0x0A,0x00,0x0D,0x15,0x1F,0x2E};
static char ssd2080m_video29[] = { 0xD7,0x28,0x27,0x14,0x02,0x01};
static char ssd2080m_video30[] = { 0xD8,0x0A,0x00,0x0D,0x15,0x1F,0x2E};
static char ssd2080m_video31[] = { 0xD9,0x28,0x27,0x14,0x02,0x01};
static char ssd2080m_video32[] = { 0xDA,0x0A,0x00,0x0D,0x15,0x1F,0x2E};
static char ssd2080m_video33[] = { 0xDB,0x28,0x27,0x14,0x02,0x01};
static char ssd2080m_video34[] = { 0xCC,0x10,0x00};
static char ssd2080m_video35[] = { 0xCE,0x4E,0x55,0xA5};
static char ssd2080m_video36[] = { 0xE0,0x01,0x02,0x02};
static char ssd2080m_video37[] = { 0xF6,0x00,0x00,0x00,0x00};
static char ssd2080m_video38[] = { 0xF7,0x00,0x00,0x00,0x00};
static char ssd2080m_video39[] = { 0xE1,0x90,0x00};
static char ssd2080m_video40[] = { 0xDE,0x95,0xCF};
static char ssd2080m_video41[] = { 0xCF,0x46};
static char ssd2080m_video42[] = { 0xC5,0x55,0x47};//0X33
static char ssd2080m_video43[] = { 0xED,0x00,0x20};
static char ssd2080m_video44[] = { 0x53,0x2C};

static struct dsi_cmd_desc ssd2080m_truly_display_on_cmds[] = {					
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video0), ssd2080m_video0},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video1), ssd2080m_video1},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video2), ssd2080m_video2},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video3), ssd2080m_video3},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video4), ssd2080m_video4},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video5), ssd2080m_video5},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video6), ssd2080m_video6},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video7), ssd2080m_video7},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video8), ssd2080m_video8},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video9), ssd2080m_video9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video10), ssd2080m_video10},
	{DTYPE_GEN_LWRITE,  1, 0, 0, 0, sizeof(ssd2080m_video11), ssd2080m_video11},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video12), ssd2080m_video12},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video13), ssd2080m_video13},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video14), ssd2080m_video14},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video15), ssd2080m_video15},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video16), ssd2080m_video16},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video17), ssd2080m_video17},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video18), ssd2080m_video18},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video19), ssd2080m_video19},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video20), ssd2080m_video20},	
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video21), ssd2080m_video21},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video22), ssd2080m_video22},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video23), ssd2080m_video23},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video24), ssd2080m_video24},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video25), ssd2080m_video25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video26), ssd2080m_video26},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video27), ssd2080m_video27},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video28), ssd2080m_video28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video29), ssd2080m_video29},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video30), ssd2080m_video30},
	{DTYPE_GEN_LWRITE,  1, 0, 0, 0,   sizeof(ssd2080m_video31), ssd2080m_video31},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video32), ssd2080m_video32},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video33), ssd2080m_video33},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video34), ssd2080m_video34},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video35), ssd2080m_video35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video36), ssd2080m_video36},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video37), ssd2080m_video37},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video38), ssd2080m_video38},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video39), ssd2080m_video39},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video40), ssd2080m_video40},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video41), ssd2080m_video41},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video42), ssd2080m_video42},
	{DTYPE_GEN_LWRITE, 1, 0, 0, 0, sizeof(ssd2080m_video43), ssd2080m_video43},	
	{DTYPE_DCS_WRITE, 1, 0, 0, 180, sizeof(ssd2080m_exit_sleep), ssd2080m_exit_sleep },
	{DTYPE_DCS_WRITE, 1, 0, 0, 130, sizeof(ssd2080m_display_on), ssd2080m_display_on },
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(ssd2080m_video44), ssd2080m_video44}, 
};
#endif

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
#define LCD_ID 77
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
	
			if(lcd_id_flag){
				strcpy(buf,str1);
			}else{
				strcpy(buf,str2);
			}
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

#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT) && defined (TYQ_SSD2080M_MIPI_HD_SUPPORT)
static char ssd2080m_enter_sleep[2] = {0x10, 0x00};
static char ssd2080m_exit_sleep_qc[2] = { 0x53, 0x00};
static char ssd2080m_exit_sleep_qc2[2] = { 0xC2, 0x00};
static char ssd2080m_exit_sleep_qc3[2] = { 0xCF, 0x40};
static char ssd2080m_exit_sleep_qc4[3] = { 0xDE, 0x84,0x00};
static char ssd2080m_exit_sleep_qcb5[2] = { 0xCB, 0x22};
static char ssd2080m_exit_sleep_qcb6[2] = { 0xC3, 0x00};

static struct dsi_cmd_desc ssd2080m_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 50, sizeof(ssd2080m_enter_sleep), ssd2080m_enter_sleep},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 50, sizeof(ssd2080m_exit_sleep_qc), ssd2080m_exit_sleep_qc},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(ssd2080m_exit_sleep_qc2), ssd2080m_exit_sleep_qc2},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(ssd2080m_exit_sleep_qc3),  ssd2080m_exit_sleep_qc3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 80, sizeof(ssd2080m_exit_sleep_qc4), ssd2080m_exit_sleep_qc4},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(ssd2080m_exit_sleep_qcb5), ssd2080m_exit_sleep_qcb5},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0, sizeof(ssd2080m_exit_sleep_qcb6), ssd2080m_exit_sleep_qcb6},

};
#endif

static int mipi_hx8392_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT) && defined (TYQ_SSD2080M_MIPI_HD_SUPPORT)
	char *pstrid_hx, *pstrid_ssd;
	char def_strid[] = "hx8392";

	pstrid_hx = strstr(get_aboot_cmd, "hx8392");
	pstrid_ssd = strstr(get_aboot_cmd, "ssd2080m");

	if ( (pstrid_hx == NULL) && (lcd_id_flag == 1) 
		&& (pstrid_ssd == NULL) ) {
		pstrid_hx = def_strid;
	}
#endif
	pr_debug("mipi_hx8392_lcd_on E\n");

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
	#ifdef DRV_READ_LCDID_MISC_IOCTL
		if(lcd_id_flag){
#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT) && defined (TYQ_SSD2080M_MIPI_HD_SUPPORT)
			if (pstrid_hx != NULL) {
				msleep(50);
				gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 0);
				msleep(20);
				gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 1);
				msleep(120);
				mipi_dsi_cmds_tx(&hx8392_tx_buf, hx8392_video_display_on_cmds,
					ARRAY_SIZE(hx8392_video_display_on_cmds));
printk("mipi_hx8392_lcd_on truly cmd lcd_id_flag = %d\r\n", lcd_id_flag);
			} else if (pstrid_ssd != NULL) {
				mipi_dsi_cmds_tx(&hx8392_tx_buf, ssd2080m_truly_display_on_cmds,
					ARRAY_SIZE(ssd2080m_truly_display_on_cmds));
printk("mipi_hx8392_lcd_on lcd chip id pstrid_ssd = %c\r\n", *pstrid_ssd);
			}
#else
			msleep(50);
			gpio_set_value_cansleep(78, 0);
			msleep(20);
			gpio_set_value_cansleep(78, 1);
			msleep(120);
printk("mipi_hx8392_lcd_on truly cmd lcd_id_flag = %d\r\n", lcd_id_flag);
			mipi_dsi_cmds_tx(&hx8392_tx_buf, hx8392_video_display_on_cmds,
					ARRAY_SIZE(hx8392_video_display_on_cmds));
#endif
		} else {
			mipi_dsi_cmds_tx(&hx8392_tx_buf, nt35521_tianma_video_display_on_cmds,
				ARRAY_SIZE(nt35521_tianma_video_display_on_cmds));
printk("mipi_nt35521_lcd_on tianma cmd lcd_id_flag = %d\r\n", lcd_id_flag);
		}
	#else
		mipi_dsi_cmds_tx(&hx8392_tx_buf, hx8392_video_display_on_cmds,
				ARRAY_SIZE(hx8392_video_display_on_cmds));
	#endif
	}

	pr_debug("mipi_hx8392_lcd_on X\n");

	return 0;
}

static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};
static struct dsi_cmd_desc hx8392_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 20, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(enter_sleep), enter_sleep},
	
};

static int mipi_hx8392_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT) && defined (TYQ_SSD2080M_MIPI_HD_SUPPORT)
	char *pstrid_ssd;
	pstrid_ssd = strstr(get_aboot_cmd, "ssd2080m");
#endif
	pr_debug("mipi_hx8392_lcd_off E\n");

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
	
#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT) && defined (TYQ_SSD2080M_MIPI_HD_SUPPORT)
	if (pstrid_ssd != NULL) {
		mipi_dsi_cmds_tx(&hx8392_tx_buf, ssd2080m_display_off_cmds,
				ARRAY_SIZE(ssd2080m_display_off_cmds));
		printk("mipi_hx8392_lcd_off lcd chip id pstrid_ssd = %c\r\n", *pstrid_ssd);
	} else {
		mdelay(10);
		gpio_set_value_cansleep(78, 0);
		msleep(10);
		gpio_set_value_cansleep(78, 1);
		msleep(120);
	}
#else
	mipi_dsi_cmds_tx(&hx8392_tx_buf, hx8392_display_off_cmds,
		ARRAY_SIZE(hx8392_display_off_cmds));
	mdelay(100);
	gpio_set_value_cansleep(78, 0);
	msleep(10);
	gpio_set_value_cansleep(78, 1);
	msleep(120);
#endif

	//dump_stack();
	//mipi_dsi_cmds_tx(&hx8392_tx_buf, hx8392_display_off_cmds,
	//	ARRAY_SIZE(hx8392_display_off_cmds));

	pr_debug("mipi_hx8392_lcd_off X\n");
	return 0;
}

static ssize_t mipi_hx8392_wta_bl_ctrl(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int err;

	err =  kstrtoint(buf, 0, &mipi_hx8392_bl_ctrl);
	if (err)
		return ret;

	pr_info("%s: bl ctrl set to %d\n", __func__, mipi_hx8392_bl_ctrl);

	return ret;
}

static DEVICE_ATTR(bl_ctrl, S_IWUSR, NULL, mipi_hx8392_wta_bl_ctrl);

static struct attribute *mipi_hx8392_fs_attrs[] = {
	&dev_attr_bl_ctrl.attr,
	NULL,
};

static struct attribute_group mipi_hx8392_fs_attr_group = {
	.attrs = mipi_hx8392_fs_attrs,
};

static int mipi_hx8392_create_sysfs(struct platform_device *pdev)
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
		&mipi_hx8392_fs_attr_group);
	if (rc) {
		pr_err("%s: sysfs group creation failed, rc=%d\n",
			__func__, rc);
		return rc;
	}

	return 0;
}

static int __devinit mipi_hx8392_lcd_probe(struct platform_device *pdev)
{
	struct platform_device *pthisdev = NULL;
	pr_debug("%s\n", __func__);

	if (pdev->id == 0) {
		mipi_hx8392_pdata = pdev->dev.platform_data;
		if (mipi_hx8392_pdata->bl_lock)
			spin_lock_init(&mipi_hx8392_pdata->bl_spinlock);

                /* SKUD use PWM as backlight control method */
                if(machine_is_msm8625q_skud()) {
                        mipi_hx8392_bl_ctrl = 1;
                }

		return 0;
	}
#if DRV_READ_LCDID_MISC_IOCTL
		msleep(2);
		lcd_id_flag = tyq_lcd_read_id(LCD_ID);
		printk(KERN_ERR "--TY--lcd:truly_probe id=%d(0:tianma 1:truly)\n",lcd_id_flag);
		truly_misc_data = pdev;
		misc_register(&truly_device);
#endif

	pthisdev = msm_fb_add_device(pdev);
	mipi_hx8392_create_sysfs(pthisdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_hx8392_lcd_probe,
	.driver = {
		.name   = "mipi_hx8392",
	},
};

static int old_bl_level;

static void mipi_hx8392_set_backlight(struct msm_fb_data_type *mfd)
{
	int bl_level;
	unsigned long flags;
	bl_level = mfd->bl_level;

	if (mipi_hx8392_pdata->bl_lock) {
		if (!mipi_hx8392_bl_ctrl) {
			/* Level received is of range 1 to bl_max,
			   We need to convert the levels to 1
			   to 31 */
			bl_level = (2 * bl_level * 31 + mfd->panel_info.bl_max)
					/(2 * mfd->panel_info.bl_max);
			if (bl_level == old_bl_level)
				return;

			if (bl_level == 0)
				mipi_hx8392_pdata->backlight(0, 1);

			if (old_bl_level == 0)
				mipi_hx8392_pdata->backlight(50, 1);

			spin_lock_irqsave(&mipi_hx8392_pdata->bl_spinlock,
						flags);
			mipi_hx8392_pdata->backlight(bl_level, 0);
			spin_unlock_irqrestore(&mipi_hx8392_pdata->bl_spinlock,
						flags);
			old_bl_level = bl_level;
		} else {
			mipi_hx8392_pdata->backlight(bl_level, 1);
		}
	} else {
		mipi_hx8392_pdata->backlight(bl_level, mipi_hx8392_bl_ctrl);
	}
}

static struct msm_fb_panel_data hx8392_panel_data = {
	.on	= mipi_hx8392_lcd_on,
	.off = mipi_hx8392_lcd_off,
	.set_backlight = mipi_hx8392_set_backlight,
};

static int ch_used[3];

static int mipi_hx8392_lcd_init(void)
{
	mipi_dsi_buf_alloc(&hx8392_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&hx8392_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

int mipi_hx8392_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	ret = mipi_hx8392_lcd_init();
	if (ret) {
		pr_err("mipi_hx8392_lcd_init() failed with ret %u\n", ret);
		return ret;
	}

	pdev = platform_device_alloc("mipi_hx8392", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	hx8392_panel_data.panel_info = *pinfo;
	ret = platform_device_add_data(pdev, &hx8392_panel_data,
				sizeof(hx8392_panel_data));
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

#endif
