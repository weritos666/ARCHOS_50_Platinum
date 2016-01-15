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
#include "TY_mipi_otm8018b.h"
/*tydrv niuli add for keypad light misc control on 20111117*/
#include <linux/miscdevice.h>

/*tydrv niuli add for keypad light misc control on 20111117  begin*/
#define DRV_READ_LCDID_MISC_IOCTL 1
/*tydrv niuli add ffor keypad light misc control on 20111117  end*/

#define OTM8018BDISPLAY_CMD_DELAY 0
#define OTM8018BDISPLAY_CMD_DELAY		0
#define OTM8018BDISPLAY_SLEEP_OFF_DELAY	150
#define OTM8018BDISPLAY_DISPLAY_ON_DELAY	150

static struct msm_panel_common_pdata *mipi_otm8018b_pdata;
static struct dsi_buf otm8018b_tx_buf;
static struct dsi_buf otm8018b_rx_buf;

static int mipi_otm8018b_bl_ctrl = 0;

/* common setting */
static char exit_sleep[2] = {0x11, 0x00};
static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

static char video_0[] = {0x00,0x00};
static char video_1[] = {0xFF,0x80,0x09,0x01}; 

static char video_2[2] = {0x00,0x80}; 
static char video_3[3]= {0xFF,0x80,0x09}; 

static char video_4[2]= {0x00,0x03}; 
static char video_5[2]={0xFF,0x01}; 

static char video_6[2]={0x00,0xB4};    ///C0B4 
static char video_7[2]={0xC0,0x10}; 

static char video_8[2]={0x00,0x89}; 
static char video_9[2]={0xC4,0x08}; 

static char video_10[2]={0x00,0x82}; 
static char video_11[2]={0xC5,0xA3}; 

static char video_12[2]={0x00,0x90};  
static char video_13[3]={0xC5,0xd6,0x76}; 

static char video_14[2]={0x00,0x00}; 
static char video_15[3]={0xD8,0x97,0x97}; 

static char video_16[2]={0x00,0x00}; 
static char video_17[2]={0xD9,0x2D}; 

static char video_18[2]={0x00,0x81}; 
//static char video_19[2]={0xC1,0x55};
static char video_19[2]={0xC1,0x66}; 


//static char video_20[2]={0x00,0xA0}; 
//static char video_21[2]={0xC1,0xea}; 
static char video_20[2]={0x00,0xA1};
static char video_21[2]={0xC1,0x08}; 

static char video_22[2]={0x00,0x81};  
static char video_23[2]={0xC4,0x83};  

static char video_24[2]={0x00,0x92}; 
static char video_25[2]={0xC5,0x01}; 

static char video_26[2]={0x00,0xB1}; 
static char video_27[2]={0xC5,0xA9}; 

static char video_28[2]={0x00,0xA6}; 
static char video_29[2]={0xC1,0x01};  

static char video_30[2]={0x00,0xC0}; 
static char video_31[2]={0xC5,0x00};  

static char video_32[2]={0x00,0x8B};  
static char video_33[2]={0xB0,0x40}; 

static char video_34[2]={0x00,0xB2}; 
static char video_35[5]={0xF5,0x15,0x00,0x15,0x00}; 

static char video_36[2]={0x00,0x93}; 
static char video_37[2]={0xC5,0x03}; 

static char video_exten0[2]={0x00,0xa0};
static char video_exten1[2]={0xc1,0xea};


static char video_38[2]={0x00,0x92}; 
static char video_39[2]={0xB3,0x45}; 

static char video_40[2]={0x00,0x90}; 
static char video_41[2]={0xB3,0x02}; 

static char video_exten2[2]={0x00,0x93};
static char video_exten3[2]={0xB3,0x45};


static char video_42[2]={0x00,0x80}; 
static char video_43[10]={0xC0,0x00,0x58,0x00,0x15,0x15,0x00,0x58,0x15,0x15}; 

static char video_44[2]={0x00,0x90}; 
static char video_45[7]={0xC0,0x00,0x44,0x00,0x00,0x00,0x03};
//static char video_45[7]={0xC0,0x00,0x56,0x00,0x00,0x00,0x03}; 


static char video_46[2]={0x00,0xA6};  
static char video_47[4]={0xC1,0x01,0x00,0x00}; 

static char video_48[2]={0x00,0x80}; 
static char video_49[7]={0xCE,0x85,0x01,0x00,0x84,0x01,0x00}; 

static char video_50[2]={0x00,0xA0}; 
static char video_51[15]={0xCE,0x18,0x03,0x03,0x59,0x00,0x00,0x00,0x18,0x02,0x03,0x5a,0x00,0x00,0x00}; 

static char video_52[2]={0x00,0xB0}; 
static char video_53[15]={0xCE,0x18,0x01,0x03,0x5b,0x00,0x00,0x00,0x18,0x00,0x03,0x5c,0x00,0x00,0x00}; 

static char video_54[2]={0x00,0xC6}; 
static char video_55[2]={0xCF,0x01}; 

static char video_56[2]={0x00,0xC0}; 
static char video_57[3]={0xCB,0x04,0x04}; 

static char video_58[2]={0x00,0xC4}; 
static char video_59[3]={0xCB,0x04,0x04}; 

static char video_60[2]={0x00,0xC8}; 
static char video_61[3]={0xCB,0x04,0x04}; 

static char video_62[2]={0x00,0xD5}; 
static char video_63[3]={0xCB,0x04,0x04}; 

static char video_64[2]={0x00,0xD9}; 
static char video_65[3]={0xCB,0x04,0x04}; 

static char video_66[2]={0x00,0xDD}; 
static char video_67[3]={0xCB,0x04,0x04}; 

static char video_68[2]={0x00,0x80}; 
static char video_69[3]={0xCC,0x26,0x25};  

static char video_70[2]={0x00,0x84}; 
static char video_71[3]={0xCC,0x0A,0x0C}; 

static char video_72[2]={0x00,0x88}; 
static char video_73[3]={0xCC,0x02,0x02};  

static char video_74[2]={0x00,0x9A};  
static char video_75[3]={0xCC,0x26,0x25};  

static char video_76[2]={0x00,0x9E};  
static char video_77[2]={0xCC,0x09};  

static char video_78[2]={0x00,0xA0};  
static char video_79[2]={0xCC,0x0B}; 

static char video_80[2]={0x00,0xA3}; 
static char video_81[3]={0xCC,0x01,0x01}; 

static char video_82[2]={0x00,0xB0}; 
static char video_83[3]={0xCC,0x25,0x26}; 

static char video_84[2]={0x00,0xB4};  
static char video_85[3]={0xCC,0x09,0x0B}; 

static char video_86[2]={0x00,0xB8}; 
static char video_87[3]={0xCC,0x01,0x01}; 

static char video_88[2]={0x00,0xCA}; 
static char video_89[3]={0xCC,0x25,0x26}; 

static char video_90[2]={0x00,0xCE}; 
static char video_91[2]={0xCC,0x0A}; 

static char video_92[2]={0x00,0xD0};  
static char video_93[2]={0xCC,0x0C};  

static char video_94[2]={0x00,0xD3}; 
static char video_95[3]={0xCC,0x02,0x02}; 

static char video_96[2]={0x00,0x00}; 
static char video_97[17]={0xE1,0x01,0x06,0x12,0x11,0x0a,0x1e,0x0e,0x0d,0x00,0x04,0x03,0x08,0x0f,0x22,0x1e,0x0f};  //2.2+                                 
static char video_98[2]={0x00,0x00};  
static char video_99[17]={0xE1,0x01,0x06,0x12,0x11,0x0a,0x1e,0x0e,0x0d,0x00,0x04,0x03,0x08,0x0f,0x22,0x1e,0x0f};  //2.2-


static struct dsi_cmd_desc otm8018bdisplay_tianma_display_on_cmds[] = {
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_0) ,video_0 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_1) ,video_1 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_2) ,video_2 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_3) ,video_3 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_4) ,video_4 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_5) ,video_5 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_6) ,video_6 },

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_7) ,video_7 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_8) ,video_8 },

	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_9) , video_9},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_10) , video_10},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_11) ,video_11 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_12) , video_12},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_13) , video_13},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_14) ,video_14 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_15) ,video_15 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_16) ,video_16 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_17) ,video_17 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_18) ,video_18 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_19) ,video_19 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_20) ,video_20 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_21) ,video_21 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_22) , video_22},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_23) ,video_23 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_24) , video_24},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_25) , video_25},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_26) ,video_26 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_27) ,video_27 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_28) , video_28},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_29) , video_29},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_30) , video_30},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_31) ,video_31 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_32) ,video_32 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_33) , video_33},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_34) ,video_34 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_35),video_35},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_36) ,video_36 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_37),video_37},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_exten0) ,video_exten1 },
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_38),video_38},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_39),video_39},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_40) ,video_40 },
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_exten2) ,video_exten3 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_41),video_41},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_42) ,video_42 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_43),video_43},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_44) ,video_44 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_45),video_45},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_46) , video_46},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_47) ,video_47 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_48) ,video_48 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_49),video_49},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_50) , video_50},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_51),video_51},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_52) ,video_52 },
	
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_53),video_53},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_54),video_54},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_55) , video_55},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_56),video_56},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_57) ,video_57 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_58),video_58},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_59) ,video_59 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_60),video_60},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_61) , video_61},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_62),video_62},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_63) ,video_63 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_64),video_64},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_65) ,video_65 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_66),video_66},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_67) ,video_67 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_68),video_68},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_69) ,video_69 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_70),video_70},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_71) ,video_71 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_72),video_72},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_73) , video_73},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_74),video_74},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_75) , video_75},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_76),video_76},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_77) ,video_77 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_78),video_78},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_79) , video_79},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_80),video_80},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_81),video_81},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_82) ,video_82 },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_83),video_83},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_84) , video_84},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_85),video_85},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_86),video_86},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_87) , video_87},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_88),video_88},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_89),video_89},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_90) , video_90},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_91),video_91},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_92),video_92},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_93) , video_93},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_94),video_94},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_95),video_95},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_96) , video_96},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_97),video_97},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_98) , video_98},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(video_99),video_99},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_SLEEP_OFF_DELAY,sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_DISPLAY_ON_DELAY,sizeof(display_on), display_on},
};



static struct dsi_cmd_desc otm8018b_display_off_cmds[] = {
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
static int tp_id_flag;
//extern int msm_fb_lcdc_config(int on);
#define LCD_ID 114
#define TP_ID    113
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
	char * str3 = "tianma_lcd";
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
			}
			else if(tp_id_flag==0){
				strcpy(buf,str2);

			}
			else{
				strcpy(buf,str3);
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

static int mipi_otm8018b_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;

	pr_debug("mipi_otm8018b_lcd_on E\n");

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
		mipi_dsi_cmds_tx(&otm8018b_tx_buf, otm8018bdisplay_tianma_display_on_cmds,
				ARRAY_SIZE(otm8018bdisplay_tianma_display_on_cmds));
	}

	pr_debug("mipi_otm8018b_lcd_on X\n");

	return 0;
}

static int mipi_otm8018b_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	pr_debug("mipi_otm8018b_lcd_off E\n");

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(&otm8018b_tx_buf, otm8018b_display_off_cmds,
			ARRAY_SIZE(otm8018b_display_off_cmds));

	pr_debug("mipi_otm8018b_lcd_off X\n");
	return 0;
}

static ssize_t mipi_otm8018b_wta_bl_ctrl(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int err;

	err =  kstrtoint(buf, 0, &mipi_otm8018b_bl_ctrl);
	if (err)
		return ret;

	pr_info("%s: bl ctrl set to %d\n", __func__, mipi_otm8018b_bl_ctrl);

	return ret;
}

static DEVICE_ATTR(bl_ctrl, S_IWUSR, NULL, mipi_otm8018b_wta_bl_ctrl);

static struct attribute *mipi_otm8018b_fs_attrs[] = {
	&dev_attr_bl_ctrl.attr,
	NULL,
};

static struct attribute_group mipi_otm8018b_fs_attr_group = {
	.attrs = mipi_otm8018b_fs_attrs,
};

static int mipi_otm8018b_create_sysfs(struct platform_device *pdev)
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
		&mipi_otm8018b_fs_attr_group);
	if (rc) {
		pr_err("%s: sysfs group creation failed, rc=%d\n",
			__func__, rc);
		return rc;
	}

	return 0;
}

static int __devinit mipi_otm8018b_lcd_probe(struct platform_device *pdev)
{
	struct platform_device *pthisdev = NULL;
	pr_debug("%s\n", __func__);

	if (pdev->id == 0) {
		mipi_otm8018b_pdata = pdev->dev.platform_data;
		if (mipi_otm8018b_pdata->bl_lock)
			spin_lock_init(&mipi_otm8018b_pdata->bl_spinlock);

                /* SKUD use PWM as backlight control method */
                if(machine_is_msm8625q_skud()) {
                        mipi_otm8018b_bl_ctrl = 1;
                }

		return 0;
	}
#if DRV_READ_LCDID_MISC_IOCTL
		msleep(2);
		lcd_id_flag = tyq_lcd_read_id(LCD_ID);
		tp_id_flag = tyq_lcd_read_id(TP_ID);
		printk(KERN_ERR "--TY--lcd:truly_probe lcd_id=%d,tp_id=%d\n",lcd_id_flag,tp_id_flag);
		truly_misc_data = pdev;
		misc_register(&truly_device);
#endif

	pthisdev = msm_fb_add_device(pdev);
	mipi_otm8018b_create_sysfs(pthisdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_otm8018b_lcd_probe,
	.driver = {
		.name   = "mipi_otm8018b",
	},
};

static int old_bl_level;

static void mipi_otm8018b_set_backlight(struct msm_fb_data_type *mfd)
{
	int bl_level;
	unsigned long flags;
	bl_level = mfd->bl_level;

	if (mipi_otm8018b_pdata->bl_lock) {
		if (!mipi_otm8018b_bl_ctrl) {
			/* Level received is of range 1 to bl_max,
			   We need to convert the levels to 1
			   to 31 */
			bl_level = (2 * bl_level * 31 + mfd->panel_info.bl_max)
					/(2 * mfd->panel_info.bl_max);
			if (bl_level == old_bl_level)
				return;

			if (bl_level == 0)
				mipi_otm8018b_pdata->backlight(0, 1);

			if (old_bl_level == 0)
				mipi_otm8018b_pdata->backlight(50, 1);

			spin_lock_irqsave(&mipi_otm8018b_pdata->bl_spinlock,
						flags);
			mipi_otm8018b_pdata->backlight(bl_level, 0);
			spin_unlock_irqrestore(&mipi_otm8018b_pdata->bl_spinlock,
						flags);
			old_bl_level = bl_level;
		} else {
			mipi_otm8018b_pdata->backlight(bl_level, 1);
		}
	} else {
		mipi_otm8018b_pdata->backlight(bl_level, mipi_otm8018b_bl_ctrl);
	}
}

static struct msm_fb_panel_data otm8018b_panel_data = {
	.on	= mipi_otm8018b_lcd_on,
	.off = mipi_otm8018b_lcd_off,
	.set_backlight = mipi_otm8018b_set_backlight,
};

static int ch_used[3];

static int mipi_otm8018b_lcd_init(void)
{
	mipi_dsi_buf_alloc(&otm8018b_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&otm8018b_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

int mipi_otm8018b_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	ret = mipi_otm8018b_lcd_init();
	if (ret) {
		pr_err("mipi_otm8018b_lcd_init() failed with ret %u\n", ret);
		return ret;
	}

	pdev = platform_device_alloc("mipi_otm8018b", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	otm8018b_panel_data.panel_info = *pinfo;
	ret = platform_device_add_data(pdev, &otm8018b_panel_data,
				sizeof(otm8018b_panel_data));
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
