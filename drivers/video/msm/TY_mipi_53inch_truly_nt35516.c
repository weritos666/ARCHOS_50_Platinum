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
#if defined(TYQ_53INCH_NT35516_MIPI_QHD_SUPPORT)
#include <mach/socinfo.h>
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "TY_mipi_53inch_truly_nt35516.h"
#include <mach/gpio.h>
#include <linux/gpio.h>
#include <mach/pmic.h>
/*tydrv niuli add for keypad light misc control on 20111117*/
#include <linux/miscdevice.h>

#define TYQ_LCD_DETECT

static struct msm_panel_common_pdata *mipi_53inch_nt35516_pdata;
static struct dsi_buf nt35516_53inch_tx_buf;
static struct dsi_buf nt35516_53inch_rx_buf;

#define GPIO_SKUD_LCD_BRDG_RESET_PIN	78

#define NT35516_53inch_CMD_DELAY		0
#define NT35516_53inch_SLEEP_OFF_DELAY	200
#define NT35516_53inch_DISPLAY_ON_DELAY	20

static int mipi_53inch_nt35516_bl_ctrl = 0;

/*tydrv niuli add for keypad light misc control on 20111117  begin*/
#define DRV_READ_LCDID_MISC_IOCTL 1
/*tydrv niuli add ffor keypad light misc control on 20111117  end*/

//tianma hx8389b start
static char TM_REG_ARRAY_01[]={0xF0,0x55,0xAA,0x52,0x08,0x00}; //Command 3 Enable
static char TM_REG_ARRAY_02[]={0xB0,0x00,0x05,0x02,0x05,0x02};//add desense code 130322
static char TM_REG_ARRAY_03[]={0xB1,0xFC,0x00,0x00};//Command Select
static char TM_REG_ARRAY_04[]={0xB6,0x05};//add desense code 130322 
static char TM_REG_ARRAY_05[]={0xB7,0x70,0x70};//Command Select
static char TM_REG_ARRAY_06[]={0xB8,0x01,0x06,0x06,0x06};//VCOM Offset
static char TM_REG_ARRAY_07[]={0xBA,0x05};//Gradient Control for Gamma 
static char TM_REG_ARRAY_08[]={0xBC,0x00,0x00,0x00};//gamma ok jy
static char TM_REG_ARRAY_09[]={0xC7,0x70};//7b--7f 
static char TM_REG_ARRAY_10[]={0xC9,0x61,0x06,0x0D,0x3A,0x17,0x00};//Positive Gamma//Red Positive(not 1~8)
static char TM_REG_ARRAY_11[]={0xCA,0x00,0x1B,0x1B,0x1B,0x1B,0x1B,0x1B,0x02,0x02,0x00,0x00};
static char TM_REG_ARRAY_12[]={0xCC,0x01};
static char TM_REG_ARRAY_13[]={0x3A,0x77};
static char TM_REG_ARRAY_14[]={0x36,0x00};
//---------- gamma  and power setting  //jy-----------------------------
static char TM_REG_ARRAY_15[]={0xF0,0x55,0xAA,0x52,0x08,0x01};
static char TM_REG_ARRAY_16[]={0xD1,0x00,0x00,0x00,0x0D,0x00,0x62,0x00,0x8E,0x00,0xAD,0x00,0xE0,0x00,0xFC,0x01,0x2A};
static char TM_REG_ARRAY_17[]={0xD2,0x01,0x3E,0x01,0x68,0x01,0x8B,0x01,0xCB,0x01,0xF9,0x01,0xFD,0x02,0x28,0x02,0x53};
static char TM_REG_ARRAY_18[]={0xD3,0x02,0x6A,0x02,0x88,0x02,0x97,0x02,0xAD,0x02,0xBA,0x02,0xCC,0x02,0xD5,0x02,0xDF};
static char TM_REG_ARRAY_19[]={0xD4,0x02,0xF2,0x02,0xF8};
static char TM_REG_ARRAY_20[]={0xD5,0x00,0x00,0x00,0x0D,0x00,0x62,0x00,0x8E,0x00,0xAD,0x00,0xE0,0x00,0xFC,0x01,0x2A};
static char TM_REG_ARRAY_21[]={0xD6,0x01,0x3E,0x01,0x68,0x01,0x8B,0x01,0xCB,0x01,0xF9,0x01,0xFD,0x02,0x28,0x02,0x53};
static char TM_REG_ARRAY_22[]={0xD7,0x02,0x6A,0x02,0x88,0x02,0x97,0x02,0xAD,0x02,0xBA,0x02,0xCC,0x02,0xD5,0x02,0xDF};
static char TM_REG_ARRAY_23[]={0xD8,0x02,0xF2,0x02,0xF8};
static char TM_REG_ARRAY_24[]={0xD9,0x00,0x00,0x00,0x0D,0x00,0x62,0x00,0x8E,0x00,0xAD,0x00,0xE0,0x00,0xFC,0x01,0x2A};
static char TM_REG_ARRAY_25[]={0xDD,0x01,0x3E,0x01,0x68,0x01,0x8B,0x01,0xCB,0x01,0xF9,0x01,0xFD,0x02,0x28,0x02,0x53};
static char TM_REG_ARRAY_26[]={0xDE,0x02,0x6A,0x02,0x88,0x02,0x97,0x02,0xAD,0x02,0xBA,0x02,0xCC,0x02,0xD5,0x02,0xDF};
static char TM_REG_ARRAY_27[]={0xDF,0x02,0xF2,0x02,0xF8};
static char TM_REG_ARRAY_28[]={0xE0,0x00,0x00,0x00,0x0D,0x00,0x62,0x00,0x8E,0x00,0xAD,0x00,0xE0,0x00,0xFC,0x01,0x2A};
static char TM_REG_ARRAY_29[]={0xE1,0x01,0x3E,0x01,0x68,0x01,0x8B,0x01,0xCB,0x01,0xF9,0x01,0xFD,0x02,0x28,0x02,0x53};
static char TM_REG_ARRAY_30[]={0xE2,0x02,0x6A,0x02,0x88,0x02,0x97,0x02,0xAD,0x02,0xBA,0x02,0xCC,0x02,0xD5,0x02,0xDF};
static char TM_REG_ARRAY_31[]={0xE3,0x02,0xF2,0x02,0xF8};
static char TM_REG_ARRAY_32[]={0xE4,0x00,0x00,0x00,0x0D,0x00,0x62,0x00,0x8E,0x00,0xAD,0x00,0xE0,0x00,0xFC,0x01,0x2A};
static char TM_REG_ARRAY_33[]={0xE5,0x01,0x3E,0x01,0x68,0x01,0x8B,0x01,0xCB,0x01,0xF9,0x01,0xFD,0x02,0x28,0x02,0x53};
static char TM_REG_ARRAY_34[]={0xE6,0x02,0x6A,0x02,0x88,0x02,0x97,0x02,0xAD,0x02,0xBA,0x02,0xCC,0x02,0xD5,0x02,0xDF};
static char TM_REG_ARRAY_35[]={0xE7,0x02,0xF2,0x02,0xF8};
static char TM_REG_ARRAY_36[]={0xE8,0x00,0x00,0x00,0x0D,0x00,0x62,0x00,0x8E,0x00,0xAD,0x00,0xE0,0x00,0xFC,0x01,0x2A};
static char TM_REG_ARRAY_37[]={0xE9,0x01,0x3E,0x01,0x68,0x01,0x8B,0x01,0xCB,0x01,0xF9,0x01,0xFD,0x02,0x28,0x02,0x53};
static char TM_REG_ARRAY_38[]={0xEA,0x02,0x6A,0x02,0x88,0x02,0x97,0x02,0xAD,0x02,0xBA,0x02,0xCC,0x02,0xD5,0x02,0xDF};
static char TM_REG_ARRAY_39[]={0xEB,0x02,0xF2,0x02,0xF8};

static char TM_REG_ARRAY_40[]={0xB0,0x05};//5.5v 0a
static char TM_REG_ARRAY_41[]={0xB6,0x53};
static char TM_REG_ARRAY_42[]={0xB1,0x05};//0a
static char TM_REG_ARRAY_43[]={0xB7,0x43};
static char TM_REG_ARRAY_44[]={0xB2,0x10};
static char TM_REG_ARRAY_45[]={0xB8,0x13};
static char TM_REG_ARRAY_46[]={0xB3,0x10};//15v
static char TM_REG_ARRAY_47[]={0xB9,0x33};
static char TM_REG_ARRAY_48[]={0xB4,0x0a};//-11v
static char TM_REG_ARRAY_49[]={0xBA,0x23};

static char TM_REG_ARRAY_50[]={0xBC,0x00,0x78,0x00};
static char TM_REG_ARRAY_51[]={0xBD,0x00,0x78,0x00};
static char TM_REG_ARRAY_52[]={0xBE,0x28};//vcom
static char TM_REG_ARRAY_53[]={0xC2,0x00};

//static char reg_addr21[2] = {0x21, 0x00};

static char TM_exit_sleep[2] = {0x11, 0x00};
static char TM_display_on[2] = {0x29, 0x00};
static char TM_display_off[2] = {0x28, 0x00};
static char TM_enter_sleep[2] = {0x10, 0x00};

static struct dsi_cmd_desc tm_53inch_hx8389b_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 40, sizeof(TM_display_off), TM_display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 150, sizeof(TM_enter_sleep), TM_enter_sleep}
};

static struct dsi_cmd_desc tm_53inch_hx8389b_video_display_on_cmds[] = {
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_01),TM_REG_ARRAY_01},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_02),TM_REG_ARRAY_02},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_03),TM_REG_ARRAY_03},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_04),TM_REG_ARRAY_04},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_05),TM_REG_ARRAY_05},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_06),TM_REG_ARRAY_06},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_07),TM_REG_ARRAY_07},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_08),TM_REG_ARRAY_08},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_09),TM_REG_ARRAY_09},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_10),TM_REG_ARRAY_10},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_11),TM_REG_ARRAY_11},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_12),TM_REG_ARRAY_12},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_13),TM_REG_ARRAY_13},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_14),TM_REG_ARRAY_14},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_15),TM_REG_ARRAY_15},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_16),TM_REG_ARRAY_16},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_17),TM_REG_ARRAY_17},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_18),TM_REG_ARRAY_18},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_19),TM_REG_ARRAY_19},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_20),TM_REG_ARRAY_20},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_21),TM_REG_ARRAY_21},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_22),TM_REG_ARRAY_22},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_23),TM_REG_ARRAY_23},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_24),TM_REG_ARRAY_24},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_25),TM_REG_ARRAY_25},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_26),TM_REG_ARRAY_26},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_27),TM_REG_ARRAY_27},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_28),TM_REG_ARRAY_28},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_29),TM_REG_ARRAY_29},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_30),TM_REG_ARRAY_30},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_31),TM_REG_ARRAY_31},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_32),TM_REG_ARRAY_32},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_33),TM_REG_ARRAY_33},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_34),TM_REG_ARRAY_34},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_35),TM_REG_ARRAY_35},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_36),TM_REG_ARRAY_36},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_37),TM_REG_ARRAY_37},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_38),TM_REG_ARRAY_38},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_39),TM_REG_ARRAY_39},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_40),TM_REG_ARRAY_40},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_41),TM_REG_ARRAY_41},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_42),TM_REG_ARRAY_42},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_43),TM_REG_ARRAY_43},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_44),TM_REG_ARRAY_44},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_45),TM_REG_ARRAY_45},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_46),TM_REG_ARRAY_46},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_47),TM_REG_ARRAY_47},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_48),TM_REG_ARRAY_48},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_49),TM_REG_ARRAY_49},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_50),TM_REG_ARRAY_50},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_51),TM_REG_ARRAY_51},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_52),TM_REG_ARRAY_52},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(TM_REG_ARRAY_53),TM_REG_ARRAY_53},

{DTYPE_DCS_WRITE, 1, 0, 0, NT35516_53inch_SLEEP_OFF_DELAY,sizeof(TM_exit_sleep) ,TM_exit_sleep },
{DTYPE_DCS_WRITE, 1, 0, 0, NT35516_53inch_DISPLAY_ON_DELAY,sizeof(TM_display_on) ,TM_display_on },

};
//tianma hx8389b end

static char REG_ARRAY_01[]={0xFF, 0xAA,0x55,0x25,0x01}; //Command 3 Enable
static char REG_ARRAY_02[]={0xF3, 0x02,0x03,0x07,0x40,0x88,0xd4,0x0c};//jy
static char REG_ARRAY_02_1[]={0xF4,0x00,0x48,0x00,0x20};
static char REG_ARRAY_02_2[]={0xFA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x11};
static char REG_ARRAY_03[]={0xF0, 0x55,0xAA,0x52,0x08,0x00};//Command Select
static char REG_ARRAY_04[]={0xB1,0xfc};//Inversion Diving Control 
static char REG_ARRAY_04_1[]={0xB6, 0x05}; //jy 
static char REG_ARRAY_04_2[]={0xBB, 0x63}; //jy

static char REG_ARRAY_05[]={0xB8,0x01,0x02,0x02,0x02};//Command Select
static char REG_ARRAY_06[]={0xBC,0x05,0x05,0x05};//VCOM Offset
static char REG_ARRAY_07[]={0xB7,0x00,0x00};//Gradient Control for Gamma 

static char REG_ARRAY_07_1[]={0xC8,0x01,0x00,0x46,0x1e,0x46,0x1e,0x46,0x1e,0x46,0x1e,0x64,0x3c,0x3c,0x64,0x64,0x3c,0x3c,0x64};
static char REG_ARRAY_07_2[]={0xF0, 0x55,0xAA,0x52,0x08,0x01};
static char REG_ARRAY_07_3[]={0xB0,0x05,0x05,0x05};
static char REG_ARRAY_07_4[]={0xB6,0x44,0x44,0x44};
static char REG_ARRAY_07_5[]={0xB1,0x05,0x05,0x05};
static char REG_ARRAY_07_6[]={0xB7,0x34,0x34,0x34};
static char REG_ARRAY_07_7[]={0xB3,0x12,0x12,0x12};
static char REG_ARRAY_07_8[]={0xB9,0x36,0x36,0x36};
static char REG_ARRAY_07_9[]={0xB4,0x0a,0x0a,0x0a};
static char REG_ARRAY_07_10[]={0xBa,0x24,0x24,0x24};

static char REG_ARRAY_08_1[]={0xBC, 0x00,0x80,0x11};//VGMP & VGSP setting 90-80 jy
static char REG_ARRAY_08_2[]={0xBD, 0x00,0x80,0x11};//VGMP & VGSP setting 90-80 jy

static char REG_ARRAY_09[]={0xBe,0x51};//VGMN & VGSN setting 
static char REG_ARRAY_10[]={0xD1,0x00,0x17,0x00,0x24,0x00,0x3d,0x00,0x52,0x00,0x66,0x00,0x86,0x00,0xa0,0x00,0xcc};//Positive Gamma//Red Positive(not 1~8)
static char REG_ARRAY_11[]={0xD2,0x00,0xf1,0x01,0x26,0x01,0x4e,0x01,0x8c,0x01,0xbc,0x01,0xbe,0x01,0xe7,0x02,0x0e};//Red Positive(not 9~16) 
static char REG_ARRAY_12[]={0xD3,0x02,0x22,0x02,0x3C,0x02,0x4F,0x02,0x71,0x02,0x90,0x02,0xC6,0x02,0xF1,0x03,0x3A};//Red Positive(not 17~24)
static char REG_ARRAY_13[]={0xD4,0x03,0xB5,0x03,0xC1};//Red Positive(not 25~26)
static char REG_ARRAY_14[]={0xD5,0x00,0x17,0x00,0x24,0x00,0x3D,0x00,0x52,0x00,0x66,0x00,0x86,0x00,0xA0,0x00,0xCC};//Green Positive(not 1~8) 
static char REG_ARRAY_15[]={0xD6,0x00,0xF1,0x01,0x26,0x01,0x4E,0x01,0x8C,0x01,0xBC,0x01,0xBE,0x01,0xE7,0x02,0x0E};//Green Positive(not 9~16)
static char REG_ARRAY_16[]={0xD7,0x02,0x22,0x02,0x3C,0x02,0x4F,0x02,0x71,0x02,0x90,0x02,0xC6,0x02,0xF1,0x03,0x3A};//Green Positive(not 17~24)
static char REG_ARRAY_17[]={0xD8,0x03,0xB5,0x03,0xC1};//Green Positive(not 25~26)
static char REG_ARRAY_18[]={0xD9,0x00,0x17,0x00,0x24,0x00,0x3D,0x00,0x52,0x00,0x66,0x00,0x86,0x00,0xA0,0x00,0xCC};//Blue Positive(not 1~8) 
static char REG_ARRAY_19[]={0xDD,0x00,0xF1,0x01,0x26,0x01,0x4E,0x01,0x8C,0x01,0xBC,0x01,0xBE,0x01,0xE7,0x02,0x0E};//Blue Positive(not 9~16)
static char REG_ARRAY_20[]={0xDE,0x02,0x22,0x02,0x3C,0x02,0x4F,0x02,0x71,0x02,0x90,0x02,0xC6,0x02,0xF1,0x03,0x3A};//Blue Positive(not 17~24)
static char REG_ARRAY_21[]={0xDF,0x03,0xB5,0x03,0xC1};//Green Positive(not 25~26)
static char REG_ARRAY_22[]={0xE0,0x00,0x17,0x00,0x24,0x00,0x3D,0x00,0x52,0x00,0x66,0x00,0x86,0x00,0xA0,0x00,0xCC};//Negative Gamma//Red Positive(not 1~8)
static char REG_ARRAY_23[]={0xE1,0x00,0xF1,0x01,0x26,0x01,0x4E,0x01,0x8C,0x01,0xBC,0x01,0xBE,0x01,0xE7,0x02,0x0E};//Red Positive(not 9~16)
static char REG_ARRAY_24[]={0xE2,0x02,0x22,0x02,0x3C,0x02,0x4F,0x02,0x71,0x02,0x90,0x02,0xC6,0x02,0xF1,0x03,0x3A};//Red Positive(not 17~24)
static char REG_ARRAY_25[]={0xE3,0x03,0xB5,0x03,0xC1};//Red Positive(not 25~26) 
static char REG_ARRAY_26[]={0xE4,0x00,0x17,0x00,0x24,0x00,0x3D,0x00,0x52,0x00,0x66,0x00,0x86,0x00,0xA0,0x00,0xCC};//Green Positive(not 1~8)
static char REG_ARRAY_27[]={0xE5,0x00,0xF1,0x01,0x26,0x01,0x4E,0x01,0x8C,0x01,0xBC,0x01,0xBE,0x01,0xE7,0x02,0x0E};//Green Positive(not 9~16)
static char REG_ARRAY_28[]={0xE6,0x02,0x22,0x02,0x3C,0x02,0x4F,0x02,0x71,0x02,0x90,0x02,0xC6,0x02,0xF1,0x03,0x3A};//Green Positive(not 17~24)            
static char REG_ARRAY_29[]={0xE7,0x03,0xB5,0x03,0xC1};//Green Positive(not 25~26)            
static char REG_ARRAY_30[]={0xE8,0x00,0x17,0x00,0x24,0x00,0x3D,0x00,0x52,0x00,0x66,0x00,0x86,0x00,0xA0,0x00,0xCC};//Blue Positive(not 1~8)             
static char REG_ARRAY_31[]={0xE9,0x00,0xF1,0x01,0x26,0x01,0x4E,0x01,0x8C,0x01,0xBC,0x01,0xBE,0x01,0xE7,0x02,0x0E};//Blue Positive(not 9~16)         
static char REG_ARRAY_32[]={0xEA,0x02,0x22,0x02,0x3C,0x02,0x4F,0x02,0x71,0x02,0x90,0x02,0xC6,0x02,0xF1,0x03,0x3A};//Blue Positive(not 17~24)           
static char REG_ARRAY_33[]={0xEB,0x03,0xB5,0x03,0xC1};//Green Positive(not 25~26)   
//static char REG_ARRAY_34[]={0x36,0x00};

static char exit_sleep[2] = {0x11, 0x00};
//static char display_on[2] = {0x29, 0x00};
//static char display_off[2] = {0x28, 0x00};
static char display_off_tes[2] = {0x00, 0x00};


static char REG_ARRAY_sleep_1[]={0xFF, 0xAA, 0x55, 0x25, 0x01};
static char REG_ARRAY_sleep_2[]={0xF3, 0x02, 0x00, 0x00, 0x45, 0x88, 0xD1, 0x0C};
static char REG_ARRAY_sleep_3[]={0xF4, 0x58};
static char REG_ARRAY_sleep_4[]={0xF8, 0x08, 0x03, 0x00, 0x00};
static char REG_ARRAY_sleep_5[]={0xFA, 0x80, 0x80, 0x20, 0x6F, 0x00, 0x0F, 0x00, 0x00, 0xD0, 0x0F, 0x00, 0x00, 0x03, 0x21, 0x14, 0x20, 0xA0, 0x00, 0x00, 0x00};
static char REG_ARRAY_sleep_6[]={0xFC, 0x00, 0x20, 0x25, 0x0A, 0x17, 0x77, 0x00, 0x00, 0x11, 0x11, 0x0F, 0x03, 0x01, 0x55, 0x67, 0x09, 0x00, 0x00, 0x00, 0x05,0x40,0x00,0x0f};
static char REG_ARRAY_sleep_7[]={0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00};
static char REG_ARRAY_sleep_8[]={0xBB, 0x11, 0x01, 0x11};
static char REG_ARRAY_sleep_9[]={0xBD, 0x01, 0xFF, 0x08, 0x40, 0x03};
static char REG_ARRAY_sleep_10[]={0xF0, 0x55, 0xAA, 0x52, 0x08, 0x01};
static char REG_ARRAY_sleep_11[]={0xB6, 0x11};
static char REG_ARRAY_sleep_12[]={0xB7, 0x11};
static char REG_ARRAY_sleep_13[]={0xB8, 0x11};
static char REG_ARRAY_sleep_14[]={0xB9, 0x12};
static char REG_ARRAY_sleep_15[]={0xBA, 0x11};

static struct dsi_cmd_desc nt35516_53inch_PERIPHERAL_off_cmds[] = {
{DTYPE_PERIPHERAL_OFF, 1, 0, 0, 40, sizeof(display_off_tes), display_off_tes},
};

static struct dsi_cmd_desc nt35516_53inch_display_off_cmds[] = {
	//{DTYPE_PERIPHERAL_OFF, 1, 0, 0, 40, sizeof(display_off_tes), display_off_tes},		
	{DTYPE_DCS_WRITE,1,0,0,90,sizeof(exit_sleep),exit_sleep},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_1),REG_ARRAY_sleep_1},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_2),REG_ARRAY_sleep_2},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_3),REG_ARRAY_sleep_3},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_4),REG_ARRAY_sleep_4},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_5),REG_ARRAY_sleep_5},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_6),REG_ARRAY_sleep_6},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_7),REG_ARRAY_sleep_7},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_8),REG_ARRAY_sleep_8},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_9),REG_ARRAY_sleep_9},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_10),REG_ARRAY_sleep_10},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_11),REG_ARRAY_sleep_11},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_12),REG_ARRAY_sleep_12},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_13),REG_ARRAY_sleep_13},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_14),REG_ARRAY_sleep_14},
	{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_sleep_15),REG_ARRAY_sleep_15}	
};

static struct dsi_cmd_desc nt35516_53inch_video_display_on_cmds[] = {
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_01),REG_ARRAY_01},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_02),REG_ARRAY_02},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_02_1),REG_ARRAY_02_1},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_02_2),REG_ARRAY_02_2},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_03),REG_ARRAY_03},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_04),REG_ARRAY_04},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_04_1),REG_ARRAY_04_1},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_04_2),REG_ARRAY_04_2},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_05),REG_ARRAY_05},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_06),REG_ARRAY_06},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07),REG_ARRAY_07},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_1),REG_ARRAY_07_1},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_2),REG_ARRAY_07_2},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_3),REG_ARRAY_07_3},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_4),REG_ARRAY_07_4},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_5),REG_ARRAY_07_5},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_6),REG_ARRAY_07_6},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_7),REG_ARRAY_07_7},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_8),REG_ARRAY_07_8},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_9),REG_ARRAY_07_9},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_07_10),REG_ARRAY_07_10},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_08_1),REG_ARRAY_08_1},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_08_2),REG_ARRAY_08_2},

{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_09),REG_ARRAY_09},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_10),REG_ARRAY_10},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_11),REG_ARRAY_11},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_12),REG_ARRAY_12},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_13),REG_ARRAY_13},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_14),REG_ARRAY_14},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_15),REG_ARRAY_15},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_16),REG_ARRAY_16},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_17),REG_ARRAY_17},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_18),REG_ARRAY_18},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_19),REG_ARRAY_19},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_20),REG_ARRAY_20},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_21),REG_ARRAY_21},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_22),REG_ARRAY_22},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_23),REG_ARRAY_23},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_24),REG_ARRAY_24},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_25),REG_ARRAY_25},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_26),REG_ARRAY_26},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_27),REG_ARRAY_27},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_28),REG_ARRAY_28},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_29),REG_ARRAY_29},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_30),REG_ARRAY_30},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_31),REG_ARRAY_31},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_32),REG_ARRAY_32},
{DTYPE_DCS_LWRITE,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(REG_ARRAY_33),REG_ARRAY_33},
{DTYPE_DCS_WRITE, 1, 0, 0, NT35516_53inch_SLEEP_OFF_DELAY,sizeof(exit_sleep) ,exit_sleep },
{DTYPE_PERIPHERAL_ON,1,0,0,NT35516_53inch_CMD_DELAY,sizeof(display_off_tes),display_off_tes},
//{DTYPE_DCS_WRITE, 1, 0, 0, NT35516_53inch_DISPLAY_ON_DELAY,sizeof(display_on) ,display_on },

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
			printk("tyq_lcd_read_id----->>>%d \n",rc);
		}
		gpio_free(gpio);
	}

	return rc;
}

int tyq_get_lcd_id(void)
{
   return lcd_id_flag;
}
EXPORT_SYMBOL(tyq_get_lcd_id);

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
	char * str2 = "tianma_lcd";

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

static int mipi_53inch_nt35516_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;

	pr_debug("mipi_53inch_nt35516_lcd_on E\n");

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
	if(lcd_id_flag)
	{		
		mipi_dsi_cmds_tx(&nt35516_53inch_tx_buf,
		nt35516_53inch_video_display_on_cmds,
		ARRAY_SIZE(nt35516_53inch_video_display_on_cmds));
		printk("mipi_53inch_nt35516_lcd_on truly cmd\r\n");
	}
	else
	{
		mipi_dsi_cmds_tx(&nt35516_53inch_tx_buf,
		tm_53inch_hx8389b_video_display_on_cmds,
		ARRAY_SIZE(tm_53inch_hx8389b_video_display_on_cmds));
		printk("mipi_53inch_nt35516_lcd_on tianma cmd\r\n");
	}
#else
		mipi_dsi_cmds_tx(&nt35516_53inch_tx_buf,
		nt35516_53inch_video_display_on_cmds,
		ARRAY_SIZE(nt35516_53inch_video_display_on_cmds));
#endif
	}

	pr_debug("mipi_53inch_nt35516_lcd_on X\n");

	return 0;
}

static int mipi_53inch_nt35516_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	pr_debug("mipi_53inch_nt35516_lcd_off E\n");

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;
	
#ifdef TYQ_LCD_DETECT
	if(lcd_id_flag)  //truly
	{
		mipi_dsi_cmds_tx(&nt35516_53inch_tx_buf, nt35516_53inch_PERIPHERAL_off_cmds,
		ARRAY_SIZE(nt35516_53inch_PERIPHERAL_off_cmds));		
		gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_PIN, 0);
		msleep(50);
		gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_PIN, 1);
		msleep(30);
		mipi_dsi_cmds_tx(&nt35516_53inch_tx_buf, nt35516_53inch_display_off_cmds,
		ARRAY_SIZE(nt35516_53inch_display_off_cmds));
	}
	else	
	{
		mipi_dsi_cmds_tx(&nt35516_53inch_tx_buf, tm_53inch_hx8389b_display_off_cmds,
		ARRAY_SIZE(tm_53inch_hx8389b_display_off_cmds));
	
	}
#else
	mipi_dsi_cmds_tx(&nt35516_53inch_tx_buf, nt35516_53inch_PERIPHERAL_off_cmds,
	ARRAY_SIZE(nt35516_53inch_PERIPHERAL_off_cmds));
	gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_PIN, 0);
	msleep(50);
	gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_PIN, 1);
	msleep(30);
	mipi_dsi_cmds_tx(&nt35516_53inch_tx_buf, nt35516_53inch_display_off_cmds,
	ARRAY_SIZE(nt35516_53inch_display_off_cmds));
#endif
	pr_debug("mipi_53inch_nt35516_lcd_off X\n");
	return 0;
}

static ssize_t mipi_53inch_nt35516_wta_bl_ctrl(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int err;

	err =  kstrtoint(buf, 0, &mipi_53inch_nt35516_bl_ctrl);
	if (err)
		return ret;

	pr_info("%s: bl ctrl set to %d\n", __func__, mipi_53inch_nt35516_bl_ctrl);

	return ret;
}

static DEVICE_ATTR(bl_ctrl, S_IWUSR, NULL, mipi_53inch_nt35516_wta_bl_ctrl);

static struct attribute *mipi_53inch_nt35516_fs_attrs[] = {
	&dev_attr_bl_ctrl.attr,
	NULL,
};

static struct attribute_group mipi_53inch_nt35516_fs_attr_group = {
	.attrs = mipi_53inch_nt35516_fs_attrs,
};

static int mipi_53inch_nt35516_create_sysfs(struct platform_device *pdev)
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
		&mipi_53inch_nt35516_fs_attr_group);
	if (rc) {
		pr_err("%s: sysfs group creation failed, rc=%d\n",
			__func__, rc);
		return rc;
	}

	return 0;
}

static int __devinit mipi_53inch_nt35516_lcd_probe(struct platform_device *pdev)
{
	struct platform_device *pthisdev = NULL;
	pr_debug("%s\n", __func__);

	if (pdev->id == 0) {
		mipi_53inch_nt35516_pdata = pdev->dev.platform_data;
		if (mipi_53inch_nt35516_pdata->bl_lock)
			spin_lock_init(&mipi_53inch_nt35516_pdata->bl_spinlock);

                /* SKUD use PWM as backlight control method */
                if(machine_is_msm8625q_skud()) {
                        mipi_53inch_nt35516_bl_ctrl = 1;
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
	mipi_53inch_nt35516_create_sysfs(pthisdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_53inch_nt35516_lcd_probe,
	.driver = {
		.name   = "mipi_53inch_nt35516",
	},
};

static int old_bl_level;

static void mipi_53inch_nt35516_set_backlight(struct msm_fb_data_type *mfd)
{
	int bl_level;
	unsigned long flags;
	bl_level = mfd->bl_level;

	if (mipi_53inch_nt35516_pdata->bl_lock) {
		if (!mipi_53inch_nt35516_bl_ctrl) {
			/* Level received is of range 1 to bl_max,
			   We need to convert the levels to 1
			   to 31 */
			bl_level = (2 * bl_level * 31 + mfd->panel_info.bl_max)
					/(2 * mfd->panel_info.bl_max);
			if (bl_level == old_bl_level)
				return;

			if (bl_level == 0)
				mipi_53inch_nt35516_pdata->backlight(0, 1);

			if (old_bl_level == 0)
				mipi_53inch_nt35516_pdata->backlight(50, 1);

			spin_lock_irqsave(&mipi_53inch_nt35516_pdata->bl_spinlock,
						flags);
			mipi_53inch_nt35516_pdata->backlight(bl_level, 0);
			spin_unlock_irqrestore(&mipi_53inch_nt35516_pdata->bl_spinlock,
						flags);
			old_bl_level = bl_level;
		} else {
			mipi_53inch_nt35516_pdata->backlight(bl_level, 1);
		}
	} else {
		mipi_53inch_nt35516_pdata->backlight(bl_level, mipi_53inch_nt35516_bl_ctrl);
	}
}

static struct msm_fb_panel_data nt35516_53inch_panel_data = {
	.on	= mipi_53inch_nt35516_lcd_on,
	.off = mipi_53inch_nt35516_lcd_off,
	.set_backlight = mipi_53inch_nt35516_set_backlight,
};

static int ch_used[3];

static int mipi_53inch_nt35516_lcd_init(void)
{
	mipi_dsi_buf_alloc(&nt35516_53inch_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&nt35516_53inch_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

int mipi_53inch_nt35516_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	ret = mipi_53inch_nt35516_lcd_init();
	if (ret) {
		pr_err("mipi_53inch_nt35516_lcd_init() failed with ret %u\n", ret);
		return ret;
	}

	pdev = platform_device_alloc("mipi_53inch_nt35516", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	nt35516_53inch_panel_data.panel_info = *pinfo;
	ret = platform_device_add_data(pdev, &nt35516_53inch_panel_data,
				sizeof(nt35516_53inch_panel_data));
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
