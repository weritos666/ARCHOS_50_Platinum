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

/* panel setting */
#ifndef DRV_READ_LCDID_MISC_IOCTL
#if 1
static char CMD_FF00_0DATA[2]={0x00,0x00};
static char CMD_FF00_3DATA[4]={0xFF,0x80,0x09,0x01};        //enable EXTC
static char CMD_FF80_0DATA[2]={0x00,0x80};              //Shift address
static char CMD_FF80_2DATA[3]={0xFF,0x80,0x09};           //enable orise mode
static char CMD_FF03_0DATA[2]={0x00,0x03};              // Resolution
static char CMD_FF03_1DATA[2]={0xff,0x01};
static char CMD_C0B4_0DATA[2]={0x00,0xB4};  // inversion-->column(50H),-->dot(00H)-->1+2(10H)      
static char CMD_C0B4_1DATA[2]={0xC0,0x10};
static char CMD_C489_0DATA[2]={0x00,0x89};     // reg ON       
static char CMD_C489_1DATA[2]={0xC4,0x00};
static char CMD_C0A2_0DATA[2]={0x00,0xA2};             
static char CMD_C0A2_3DATA[4]={0xC0,0x0d,0x0b,0x1b};
static char CMD_CFC9_0DATA[2]={0x00,0xC9};                       
static char CMD_CFC9_1DATA[2]={0xCF,0x08};    
static char CMD_C594_0DATA[2]={0x00,0x94};         //Gate pre-charge   //00     
static char CMD_C594_1DATA[2]={0xC5,0x55};        //Pump1 1/4   Pump2 1/4
static char CMD_C582_0DATA[2]={0x00,0x82};              //REG-pump23 
static char CMD_C582_1DATA[2]={0xC5,0xA3};              //
static char CMD_C590_0DATA[2]={0x00,0x90};            //Pump setting (3x=D6)-->(2x=96)//v02 01/11  
static char CMD_C590_2DATA[3]={0xC5,0xD6,0x77} ;         
static char CMD_D800_0DATA[2]={0x00,0x00};              
static char CMD_D800_2DATA[3]={0xD8,0x97,0x97};    
static char CMD_D900_0DATA[2]={0x00,0x00};              //
static char CMD_D900_1DATA[2]={0xD9,0x7A};           //VDD_18V=1.6V GVDD test on
static char CMD_C181_0DATA[2]={0x00,0x81};              //Frame rate = 65Hz  
static char CMD_C181_1DATA[2]={0xC1,0x66};
static char CMD_C1A1_0DATA[2]={0x00,0xA1};         //external Vsync(08)  /Vsync,Hsync(0c) /Vsync,Hsync,DE(0e) //V02(0e)  / all  included clk(0f     
static char CMD_C1A1_1DATA[2]={0xC1,0x08};
static char CMD_3600_0DATA[2]={0x00,0x00};              
static char CMD_3600_1DATA[2]={0x36,0xC0};

/*********************start tianma add for lcd diaplay direction*****************/ 

//static char CMD_B391_0DATA[2]={0x00,0x91};              
// char CMD_B391_1DATA[2]={0xB3,0xC0};

/*********************start tianma add  for lcd diaplay direction*****************/ 

static char CMD_C481_0DATA[2]={0x00,0x81};
static char CMD_C481_1DATA[2]={0xC4,0x83};              
static char CMD_C592_0DATA [2]={0x00,0x92};
static char CMD_C592_1DATA[2] ={0xC5,0x01};     
static char CMD_C5B1_0DATA [2]={0x00,0xB1}; 
static char CMD_C5B1_1DATA[2] ={0xC5,0xA9};    
static char  CMD_C1A60_0DATA[2]={0x00,0xA6};   /***************gaohw********/
static char  CMD_C1A60_1DATA[2]={0xC1,0x01};
static char  CMD_C5C0_0DATA[2]={0x00,0xC0};
static char  CMD_C5C0_1DATA[2]={0xC5,0x00};
static char  CMD_B08B_0DATA[2]={0x00,0x8B};
static char CMD_B08B_1DATA[2]={0xB0,0x40};
 static char CMD_F5B2_0DATA[2]={0x00,0xB2};
 static char  CMD_F5B2_4DATA[5]={0xF5,0x15,0x00,0x15,0x00};
 static char  CMD_C593_0DATA[2]={0x00,0x93};
  static char  CMD_C593_1DATA[2]={0xC5,0x03};
static char  CMD_C090_0DATA[2]={0x00,0x90};
 static char  CMD_C090_6DATA[7]={0xC0,0x00,0x56,0x00,0x00,0x00,0x03};
static char  CMD_C1A6_0DATA[2]={0x00,0xA6};
 static char  CMD_C1A6_3DATA[4]={0xC1,0x01,0x00,0x00};
static char CMD_CE80_0DATA [2]= {0x00,0x80};                                                  //GOA VST Setting, VST1/VST2/VST3/VST4
static char CMD_CE80_6DATA[7]={0xCE,0x83,0x01,0x11,0x82,0x01,0x11};                 //
static char CMD_CEA0_0DATA [2]= {0x00,0xA0};                                                  //GOA CLKA0 Setting, CLKA1/CLKA2
static char CMD_CEA0_14DATA[15]={0xCE,0x18,0x05,0x03,0x24,0x00,0x11,0x00,0x18,0x04,0x03,0x25,0x00,0x11,0x00};           //
static char CMD_CEB0_0DATA [2]= {0x00,0xB0};                                                  //GOA CLKA3 Setting, CLKA3/CLKA4
static char CMD_CEB0_14DATA[15]={0xCE,0x18,0x03,0x03,0x26,0x00,0x11,0x00,0x18,0x02,0x03,0x27,0x00,0x11,0x00};           //

static char CMD_CBC0_0DATA [2]={0x00,0xC0}; 												 //enmode H-byte of sig1--15
static char CMD_CBC0_2DATA[3]={0xCB,0x04,0x04};		  //
	
static char CMD_CBC6_0DATA [2]={0x00,0xC6}; 												 //enmode H-byte of sig1--15
static char CMD_CBC6_3DATA[4]={0xCB,0x04,0x04,0x04};		  //
static char CMD_CBD5_0DATA [2]={0x00,0xD5}; 												 //enmode H-byte of sig16--30
static char CMD_CBD5_2DATA[3]={0xCB,0x04,0x04};
static char CMD_CBDB_0DATA [2]={0x00,0xDB}; 												 //enmode H-byte of sig16--30
static char CMD_CBDB_3DATA[4]={0xCB,0x04,0x04,0x4};

static char CMD_CC80_0DATA [2]={0x00,0x80};                                                  //reg setting for signal01--10 selection with u2d mode
static char CMD_CC80_2DATA[3]={0xCC,0x26,0x25};
	
static char CMD_CC86_0DATA [2]={0x00,0x86}; 												 //reg setting for signal01--10 selection with u2d mode
static char CMD_CC86_3DATA[4]={0xCC,0x0C,0x0A,0x02};

static char CMD_CC9A_0DATA [2]={0x00,0x9a};                                                  //reg setting for signal11--25 selection with u2d mode
static char CMD_CC9A_2DATA[3]={0xCC,0x26,0x25};        //

static char CMD_CCA1_0DATA [2]={0x00,0xA1};                                                  //reg setting for signal26--40 selection with u2d mode
static char CMD_CCA1_3DATA[4]={0xCC,0x0B,0x09,0x01};        //

static char CMD_CCB0_0DATA [2]={0x00,0xB0}; 												 //reg setting for signal01--10 selection with d2u mode
static char CMD_CCB0_2DATA[3]={0xCC,0x25,0x26};

static char CMD_CCB6_0DATA [2]={0x00,0xB6};                                                  //reg setting for signal01--10 selection with d2u mode
static char CMD_CCB6_3DATA[4]={0xCC,0x09,0x0B,0x01};

static char CMD_CCCA_0DATA [2]={0x00,0xCA};                                                  //reg setting for signal11--25 selection with d2u mode
static char CMD_CCCA_2DATA[3]={0xCC,0x25,0x26};        //
static char CMD_CCD1_0DATA [2]={0x00,0xD1};                                                  //reg setting for signal26--40 selection with d2u mode
static char CMD_CCD1_3DATA[4]={0xCC,0x0A,0x0C,0x02};
#endif

#else
#if 1
static char CMD_FF00_0DATA[2]={0x00,0x00};
static char CMD_FF00_3DATA[4]={0xFF,0x80,0x09,0x01};        //enable EXTC
static char CMD_FF80_0DATA[2]={0x00,0x80};              //Shift address
static char CMD_FF80_2DATA[3]={0xFF,0x80,0x09};           //enable orise mode
static char CMD_FF03_0DATA[2]={0x00,0x03};              // Resolution
static char CMD_FF03_1DATA[2]={0xff,0x01};
static char CMD_C0B4_0DATA[2]={0x00,0xB4};  // inversion-->column(50H),-->dot(00H)-->1+2(10H)      
static char CMD_C0B4_1DATA[2]={0xC0,0x10};
static char CMD_C489_0DATA[2]={0x00,0x89};     // reg ON       
static char CMD_C489_1DATA[2]={0xC4,0x00};
static char CMD_C0A2_0DATA[2]={0x00,0xA2};             
static char CMD_C0A2_3DATA[4]={0xC0,0x0d,0x0b,0x1b};
static char CMD_CFC9_0DATA[2]={0x00,0xC9};                       
static char CMD_CFC9_1DATA[2]={0xCF,0x08};    
static char CMD_C594_0DATA[2]={0x00,0x94};         //Gate pre-charge   //00     
static char CMD_C594_1DATA[2]={0xC5,0x55};        //Pump1 1/4   Pump2 1/4
static char CMD_C582_0DATA[2]={0x00,0x82};              //REG-pump23 
static char CMD_C582_1DATA[2]={0xC5,0xA3};              //
static char CMD_C590_0DATA[2]={0x00,0x90};            //Pump setting (3x=D6)-->(2x=96)//v02 01/11  
static char CMD_C590_2DATA[3]={0xC5,0xD6,0x77} ;         
static char CMD_D800_0DATA[2]={0x00,0x00};              
static char CMD_D800_2DATA[3]={0xD8,0x97,0x97};    
static char CMD_D900_0DATA[2]={0x00,0x00};              //
static char CMD_D900_1DATA[2]={0xD9,0x7A};           //VDD_18V=1.6V GVDD test on
static char CMD_C181_0DATA[2]={0x00,0x81};              //Frame rate = 65Hz  
static char CMD_C181_1DATA[2]={0xC1,0x66};
static char CMD_C1A1_0DATA[2]={0x00,0xA1};         //external Vsync(08)  /Vsync,Hsync(0c) /Vsync,Hsync,DE(0e) //V02(0e)  / all  included clk(0f     
static char CMD_C1A1_1DATA[2]={0xC1,0x08};
static char CMD_3600_0DATA[2]={0x00,0x00};              
static char CMD_3600_1DATA[2]={0x36,0xC0};

/*********************start tianma add for lcd diaplay direction*****************/ 

//static char CMD_B391_0DATA[2]={0x00,0x91};              
// char CMD_B391_1DATA[2]={0xB3,0xC0};

/*********************start tianma add  for lcd diaplay direction*****************/ 

static char CMD_C481_0DATA[2]={0x00,0x81};
static char CMD_C481_1DATA[2]={0xC4,0x83};              
static char CMD_C592_0DATA [2]={0x00,0x92};
static char CMD_C592_1DATA[2] ={0xC5,0x01};     
static char CMD_C5B1_0DATA [2]={0x00,0xB1}; 
static char CMD_C5B1_1DATA[2] ={0xC5,0xA9};    
static char  CMD_C1A60_0DATA[2]={0x00,0xA6};   /***************gaohw********/
static char  CMD_C1A60_1DATA[2]={0xC1,0x01};
static char  CMD_C5C0_0DATA[2]={0x00,0xC0};
static char  CMD_C5C0_1DATA[2]={0xC5,0x00};
static char  CMD_B08B_0DATA[2]={0x00,0x8B};
static char CMD_B08B_1DATA[2]={0xB0,0x40};
 static char CMD_F5B2_0DATA[2]={0x00,0xB2};
 static char  CMD_F5B2_4DATA[5]={0xF5,0x15,0x00,0x15,0x00};
 static char  CMD_C593_0DATA[2]={0x00,0x93};
  static char  CMD_C593_1DATA[2]={0xC5,0x03};
static char  CMD_C090_0DATA[2]={0x00,0x90};
 static char  CMD_C090_6DATA[7]={0xC0,0x00,0x56,0x00,0x00,0x00,0x03};
static char  CMD_C1A6_0DATA[2]={0x00,0xA6};
 static char  CMD_C1A6_3DATA[4]={0xC1,0x01,0x00,0x00};
static char CMD_CE80_0DATA [2]= {0x00,0x80};                                                  //GOA VST Setting, VST1/VST2/VST3/VST4
static char CMD_CE80_6DATA[7]={0xCE,0x83,0x01,0x11,0x82,0x01,0x11};                 //
static char CMD_CEA0_0DATA [2]= {0x00,0xA0};                                                  //GOA CLKA0 Setting, CLKA1/CLKA2
static char CMD_CEA0_14DATA[15]={0xCE,0x18,0x05,0x03,0x24,0x00,0x11,0x00,0x18,0x04,0x03,0x25,0x00,0x11,0x00};           //
static char CMD_CEB0_0DATA [2]= {0x00,0xB0};                                                  //GOA CLKA3 Setting, CLKA3/CLKA4
static char CMD_CEB0_14DATA[15]={0xCE,0x18,0x03,0x03,0x26,0x00,0x11,0x00,0x18,0x02,0x03,0x27,0x00,0x11,0x00};           //

static char CMD_CBC0_0DATA [2]={0x00,0xC0}; 												 //enmode H-byte of sig1--15
static char CMD_CBC0_2DATA[3]={0xCB,0x04,0x04};		  //
	
static char CMD_CBC6_0DATA [2]={0x00,0xC6}; 												 //enmode H-byte of sig1--15
static char CMD_CBC6_3DATA[4]={0xCB,0x04,0x04,0x04};		  //
static char CMD_CBD5_0DATA [2]={0x00,0xD5}; 												 //enmode H-byte of sig16--30
static char CMD_CBD5_2DATA[3]={0xCB,0x04,0x04};
static char CMD_CBDB_0DATA [2]={0x00,0xDB}; 												 //enmode H-byte of sig16--30
static char CMD_CBDB_3DATA[4]={0xCB,0x04,0x04,0x4};

static char CMD_CC80_0DATA [2]={0x00,0x80};                                                  //reg setting for signal01--10 selection with u2d mode
static char CMD_CC80_2DATA[3]={0xCC,0x26,0x25};
	
static char CMD_CC86_0DATA [2]={0x00,0x86}; 												 //reg setting for signal01--10 selection with u2d mode
static char CMD_CC86_3DATA[4]={0xCC,0x0C,0x0A,0x02};

static char CMD_CC9A_0DATA [2]={0x00,0x9a};                                                  //reg setting for signal11--25 selection with u2d mode
static char CMD_CC9A_2DATA[3]={0xCC,0x26,0x25};        //

static char CMD_CCA1_0DATA [2]={0x00,0xA1};                                                  //reg setting for signal26--40 selection with u2d mode
static char CMD_CCA1_3DATA[4]={0xCC,0x0B,0x09,0x01};        //

static char CMD_CCB0_0DATA [2]={0x00,0xB0}; 												 //reg setting for signal01--10 selection with d2u mode
static char CMD_CCB0_2DATA[3]={0xCC,0x25,0x26};

static char CMD_CCB6_0DATA [2]={0x00,0xB6};                                                  //reg setting for signal01--10 selection with d2u mode
static char CMD_CCB6_3DATA[4]={0xCC,0x09,0x0B,0x01};

static char CMD_CCCA_0DATA [2]={0x00,0xCA};                                                  //reg setting for signal11--25 selection with d2u mode
static char CMD_CCCA_2DATA[3]={0xCC,0x25,0x26};        //
static char CMD_CCD1_0DATA [2]={0x00,0xD1};                                                  //reg setting for signal26--40 selection with d2u mode
static char CMD_CCD1_3DATA[4]={0xCC,0x0A,0x0C,0x02};
#endif


#if 1
static char BYD_CMD_FF00_0DATA[2]={0x00,0x00};
static char BYD_CMD_FF00_3DATA[4]={0xFF,0x80,0x09,0x01};        //enable EXTC
static char BYD_CMD_FF80_0DATA[2]={0x00,0x80};              //Shift address
static char BYD_CMD_FF80_2DATA[3]={0xFF,0x80,0x09};           //enable orise mode
static char BYD_CMD_FF03_0DATA[2]={0x00,0x03};              // Resolution
static char BYD_CMD_FF03_1DATA[2]={0xff,0x01};
static char BYD_CMD_C5B1_0DATA[2]={0x00,0xb1};				
static char BYD_CMD_C5B1_1DATA[2]={0xC5,0xA9};
static char BYD_CMD_C590_0DATA[2]={0x00,0x90};				
static char BYD_CMD_C590_3DATA[4]={0xC5,0x96,0xA7,0x01};
static char BYD_CMD_C181_0DATA[2]={0x00,0x81};				
static char BYD_CMD_C181_1DATA[2]={0xC1,0x66};
static char BYD_CMD_C1A1_0DATA[2]={0x00,0xA1};				
static char BYD_CMD_C1A1_3DATA[4]={0xC1,0x08,0x02,0x1b};

/*********************start tianma add for lcd diaplay direction*****************/ 

//static char BYD_CMD_B391_0DATA[2]={0x00,0x91};              
//static char BYD_CMD_B391_1DATA[2]={0xB3,0xC0};

/*********************start tianma add  for lcd diaplay direction*****************/ 
static char BYD_CMD_B3A1_0DATA[2]={0x00,0xA1};				
static char BYD_CMD_B3A1_1DATA[2]={0xB3,0x10};
static char BYD_CMD_C0B4_0DATA[2]={0x00,0xB4};				
static char BYD_CMD_C0B4_1DATA[2]={0xC0,0x10};
static char BYD_CMD_D800_0DATA[2]={0x00,0x00};				
static char BYD_CMD_D800_2DATA[3]={0xD8,0x43,0x43};
static char BYD_CMD_D900_0DATA[2]={0x00,0x00};				
static char BYD_CMD_D900_1DATA[2]={0xD9,0x32};
static char BYD_CMD_C090_0DATA[2]={0x00,0x90};				
static char BYD_CMD_C090_6DATA[7]={0xC0,0x00,0x44,0x00,0x00,0x00,0x03};
static char BYD_CMD_C1A6_0DATA[2]={0x00,0xA6};				
static char BYD_CMD_C1A6_3DATA[4]={0xC1,0x00,0x00,0x00};
static char BYD_CMD_CE80_0DATA[2]={0x00,0x80};				
static char BYD_CMD_CE80_6DATA[7]={0xCE,0x87,0x03,0x14,0x86,0x03,0x14};
static char BYD_CMD_CE90_0DATA[2]={0x00,0x90};				
static char BYD_CMD_CE90_6DATA[7]={0xCE,0x33,0x1E,0x14,0x33,0x1f,0x14};
static char BYD_CMD_CEA0_0DATA[2]={0x00,0xA0};				
static char BYD_CMD_CEA0_12DATA[13]={0xCE,0x38,0x03,0x03,0x1c,0x00,0x14,0x00,0x38,0x02,0x03,0x1d,0x00};
static char BYD_CMD_00AD_0DATA[2]={0x00,0xAD};				
static char BYD_CMD_00AD_1DATA[2]={0x00,0xCE};
static char BYD_CMD_CEB0_0DATA[2]={0x00,0xB0};				
static char BYD_CMD_CEB0_14DATA[15]={0xCE,0x38,0x01,0x03,0x1e,0x00,0x14,0x00,0x38,0x00,0x03,0x1f,0x00,0x14,0x00};
static char BYD_CMD_CEC0_0DATA[2]={0x00,0xC0};				
static char BYD_CMD_CEC0_14DATA[15]={0xCE,0x30,0x00,0x03,0x20,0x00,0x14,0x00,0x30,0x01,0x03,0x21,0x00,0x14,0x00};
static char BYD_CMD_CED0_0DATA[2]={0x00,0xD0};				
static char BYD_CMD_CED0_14DATA[15]={0xCE,0x30,0x02,0x03,0x22,0x00,0x14,0x00,0x30,0x03,0x03,0x23,0x00,0x14,0x00};
static char BYD_CMD_CFC6_0DATA[2]={0x00,0xC6};				
static char BYD_CMD_CFC6_2DATA[3]={0xCF,0x01,0x80};
static char BYD_CMD_CFC9_0DATA[2]={0x00,0xC9};				
static char BYD_CMD_CFC9_1DATA[2]={0xCF,0x10};
static char BYD_CMD_CBC0_0DATA[2]={0x00,0xC0};				
static char BYD_CMD_CBC0_15DATA[16]={0xCB,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00};
static char BYD_CMD_CBD0_0DATA[2]={0x00,0xD0};				
static char BYD_CMD_CBD0_15DATA[16]={0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x00,0x00,0x04,0x04,0x04};
static char BYD_CMD_CBE0_0DATA[2]={0x00,0xE0};				
static char BYD_CMD_CBE0_10DATA[11]={0xCB,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char BYD_CMD_CC80_0DATA[2]={0x00,0x80};				
static char BYD_CMD_CC80_10DATA[11]={0xCC,0x00,0x26,0x25,0x02,0x06,0x00,0x00,0x0a,0x0E,0x0c};
static char BYD_CMD_CC90_0DATA[2]={0x00,0x90};				
static char BYD_CMD_CC90_15DATA[16]={0xCC,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x26,0x25,0x01,05};
static char BYD_CMD_CCA0_0DATA[2]={0x00,0xA0};				
static char BYD_CMD_CCA0_15DATA[16]={0xCC,0x00,0x00,0x09,0x0d,0x0b,0x0f,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char BYD_CMD_CCB0_0DATA[2]={0x00,0xB0};				
static char BYD_CMD_CCB0_10DATA[11]={0xCC,0x00,0x25,0x26,0x05,0x01,0x00,0x00,0x0f,0x0b,0x0d};
static char BYD_CMD_CCC0_0DATA[2]={0x00,0xC0};				
static char BYD_CMD_CCC0_15DATA[16]={0xCC,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x25,0x26,0x06,0x02};
static char BYD_CMD_CCD0_0DATA[2]={0x00,0xD0};				
static char BYD_CMD_CCD0_15DATA[16]={0xcc,0x00,0x00,0x10,0x0c,0x0e,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
static char BYD_CMD_0000_0DATA[2]={0x00,0x00};				
static char BYD_CMD_0000_1DATA[2]={0x00,0x00};
static char BYD_CMD_E100_0DATA[2]={0x00,0x00};				
static char BYD_CMD_E100_16DATA[17]={0xe1,0x06,0x07,0x0e,0x0d,0x07,0x16,0x0c,0x0c,0x02,0x06,0x05,0x07,0x0f,0x2b,0x2f,0x0d};
static char BYD_CMD_E200_0DATA[2]={0x00,0x00};				
static char BYD_CMD_E200_16DATA[17]={0xe2,0x06,0x07,0x0e,0x0d,0x07,0x16,0x0c,0x0c,0x02,0x06,0x04,0x07,0x0f,0x2b,0x2f,0x0d};
static char BYD_CMD_2000_0DATA[2]={0x00,0x00};				
static char BYD_CMD_2000_1DATA[2]={0x20,0x00};
static char BYD_CMD_2C00_0DATA[2]={0x2C,0x00};       

#endif

#if 1
static char TRUlY_CMD_FF00_0DATA[2]={0x00,0x00};
static char TRULY_CMD_FF00_3DATA[4]={0xFF,0x80,0x09,0x01};        //enable EXTC
static char TRULY_CMD_FF80_0DATA[2]={0x00,0x80};              //Shift address
static char TRULY_CMD_FF80_2DATA[3]={0xFF,0x80,0x09};           //enable orise mode
static char TRULY_CMD_FF03_0DATA[2]={0x00,0x03};             
static char TRULY_CMD_FF03_1DATA[2]={0xff,0x01};
static char TRULY_CMD_C0B4_0DATA[2]={0x00,0xb4};             
static char TRULY_CMD_C0B4_1DATA[2]={0xc0,0x10};
static char TRULY_CMD_C582_0DATA[2]={0x00,0x82};             
static char TRULY_CMD_C582_1DATA[2]={0xc5,0xa3};
static char TRULY_CMD_C590_0DATA[2]={0x00,0x90};             
static char TRULY_CMD_C590_2DATA[3]={0xc5,0xc6,0x76}; //96-c6
static char TRULY_CMD_D800_0DATA[2]={0x00,0x00};             
static char TRULY_CMD_D800_2DATA[3]={0xd8,0x75,0x75}; //73-75 130115 jy
static char TRULY_CMD_D900_0DATA[2]={0x00,0x00};             
static char TRULY_CMD_D900_1DATA[2]={0xd9,0x62};//5e-60 jy
static char TRULY_CMD_E100_0DATA[2]={0x00,0x00};             
static char TRULY_CMD_E100_16DATA[17]={0xe1,0x09,0x0b,0x0f,0x0e,0x0a,0x1b,0x0a,0x0a,0x01,0x05,0x04,0x06,0x0f,0x23,0x1d,0x03};//gamma+  jy
static char TRULY_CMD_E200_0DATA[2]={0x00,0x00};             
static char TRULY_CMD_E200_16DATA[17]={0xe2,0x09,0x0b,0x0f,0x11,0x0e,0x1f,0x0a,0x0a,0x01,0x05,0x04,0x06,0x0f,0x23,0x1d,0x03};//gamma-  jy
static char TRULY_CMD_C181_0DATA[2]={0x00,0x81};             
static char TRULY_CMD_c181_1DATA[2]={0xc1,0x66};//66-77  jy
static char TRULY_CMD_C1A1_0DATA[2]={0x00,0xa1};             
static char TRULY_CMD_C1A1_1DATA[2]={0xc1,0x08};
static char TRULY_CMD_C489_0DATA[2]={0x00,0x89};             
static char TRULY_CMD_C489_1DATA[2]={0xc4,0x08};
static char TRULY_CMD_C0A2_0DATA[2]={0x00,0xA2};             
static char TRULY_CMD_C0A2_3DATA[4]={0xc0,0x1b,0x00,0x02};
static char TRULY_CMD_C481_0DATA[2]={0x00,0x81};             
static char TRULY_CMD_C481_1DATA[2]={0xc4,0x83};

static char TRULY_CMD_C480_0DATA[2]={0x00,0x80};             
static char TRULY_CMD_C480_1DATA[2]={0xc4,0x30}; //add 130124

static char TRULY_CMD_C592_0DATA[2]={0x00,0x92};             
static char TRULY_CMD_C592_1DATA[2]={0xc5,0x01};
static char TRULY_CMD_C5B1_0DATA[2]={0x00,0xB1};             
static char TRULY_CMD_C5B1_1DATA[2]={0xc5,0xA9};
static char TRULY_CMD_C090_0DATA[2]={0x00,0x90};             
static char TRULY_CMD_C090_6DATA[7]={0xc0,0x00,0x44,0x00,0x00,0x00,0x03};
static char TRULY_CMD_C1A6_0DATA[2]={0x00,0xa6};             
static char TRULY_CMD_C1A6_3DATA[4]={0xc1,0x00,0x00,0x00};
static char TRULY_CMD_CE80_0DATA[2]={0x00,0x80};             
static char TRULY_CMD_CE80_12DATA[13]={0xce,0x87,0x03,0x00,0x85,0x03,0x00,0x86,0x03,0x00,0x84,0x03,0x00};
static char TRULY_CMD_CEA0_0DATA[2]={0x00,0xA0};             
static char TRULY_CMD_CEA0_14DATA[15]={0xce,0x38,0x03,0x03,0x58,0x00,0x00,0x00,0x38,0x02,0x03,0x59,0x00,0x00,0x00};
static char TRULY_CMD_CEB0_0DATA[2]={0x00,0xb0};             
static char TRULY_CMD_CEB0_14DATA[15]={0xce,0x38,0x01,0x03,0x5a,0x00,0x00,0x00,0x38,0x00,0x03,0x5b,0x00,0x00,0x00};
static char TRULY_CMD_CEC0_0DATA[2]={0x00,0xc0};             
static char TRULY_CMD_CEC0_14DATA[15]={0xce,0x30,0x00,0x03,0x5c,0x00,0x00,0x00,0x30,0x01,0x03,0x5d,0x00,0x00,0x00};
static char TRULY_CMD_CED0_0DATA[2]={0x00,0xd0};             
static char TRULY_CMD_CED0_14DATA[15]={0xce,0x30,0x02,0x03,0x5e,0x00,0x00,0x00,0x30,0x03,0x03,0x5f,0x00,0x00,0x00};
static char TRULY_CMD_CFC7_0DATA[2]={0x00,0xC7};             
static char TRULY_CMD_CFC7_1DATA[2]={0xcf,0x00};
static char TRULY_CMD_CFC9_0DATA[2]={0x00,0xc9};             
static char TRULY_CMD_CFC9_1DATA[2]={0xcf,0x00};
static char TRULY_CMD_CBC4_0DATA[2]={0x00,0xC4};             
static char TRULY_CMD_CBC4_6DATA[7]={0xcb,0x04,0x04,0x04,0x04,0x04,0x04};
static char TRULY_CMD_CBD9_0DATA[2]={0x00,0xd9};             
static char TRULY_CMD_CBD9_6DATA[7]={0xcb,0x04,0x04,0x04,0x04,0x04,0x04};
static char TRULY_CMD_CC84_0DATA[2]={0x00,0x84};             
static char TRULY_CMD_CC84_6DATA[7]={0xcc,0x0c,0x0a,0x10,0x0e,0x03,0x04};
static char TRULY_CMD_CC9E_0DATA[2]={0x00,0x9e};             
static char TRULY_CMD_CC9E_1DATA[2]={0xcc,0x0b};
static char TRULY_CMD_CCA0_0DATA[2]={0x00,0xa0};             
static char TRULY_CMD_CCA0_5DATA[6]={0xcc,0x09,0x0f,0x0d,0x01,0x02};
static char TRULY_CMD_CCB4_0DATA[2]={0x00,0xb4};             
static char TRULY_CMD_CCB4_6DATA[7]={0xcc,0x0d,0x0f,0x09,0x0b,0x02,0x01};
static char TRULY_CMD_CCCE_0DATA[2]={0x00,0xce};             
static char TRULY_CMD_CCCE_1DATA[2]={0xcc,0x0e};
static char TRULY_CMD_CCD0_0DATA[2]={0x00,0xd0};             
static char TRULY_CMD_CCD0_5DATA[6]={0xcc,0x10,0x0a,0x0c,0x04,0x03};
static char TRULY_CMD_3A00_0DATA[2]={0x00,0x00};             
static char TRULY_CMD_3A00_1DATA[2]={0x3a,0x77};
static char TRULY_CMD_2C00_0DATA[2]={0x2C,0x00};	

#endif

#endif

/* OTM8018B must use DCS commands */
#ifdef DRV_READ_LCDID_MISC_IOCTL

static struct dsi_cmd_desc otm8018bdisplay_byd_display_on_cmds[] = {
	
	#if 1
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_FF00_0DATA) , BYD_CMD_FF00_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_FF00_3DATA) ,BYD_CMD_FF00_3DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_FF80_0DATA) ,BYD_CMD_FF80_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_FF80_2DATA) ,BYD_CMD_FF80_2DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_FF03_0DATA) ,BYD_CMD_FF03_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_FF03_1DATA) ,BYD_CMD_FF03_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C5B1_0DATA) ,BYD_CMD_C5B1_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C5B1_1DATA) ,BYD_CMD_C5B1_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C590_0DATA) ,BYD_CMD_C590_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C590_3DATA) ,BYD_CMD_C590_3DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C181_0DATA) ,BYD_CMD_C181_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C181_1DATA) , BYD_CMD_C181_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C1A1_0DATA) , BYD_CMD_C1A1_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C1A1_3DATA) ,BYD_CMD_C1A1_3DATA },
	/***************************************start tianma add for lcd diaplay direction**************************************************/
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_B391_0DATA) ,BYD_CMD_B391_0DATA },
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_B391_1DATA) ,BYD_CMD_B391_1DATA },
	/****************************************start tianma add for lcd diaplay direction***************************************************/
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_B3A1_0DATA) ,BYD_CMD_B3A1_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_B3A1_1DATA) , BYD_CMD_B3A1_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C0B4_0DATA) , BYD_CMD_C0B4_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C0B4_1DATA) , BYD_CMD_C0B4_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_D800_0DATA) , BYD_CMD_D800_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_D800_2DATA) ,BYD_CMD_D800_2DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_D900_0DATA) ,BYD_CMD_D900_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_D900_1DATA) ,BYD_CMD_D900_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C090_0DATA) ,BYD_CMD_C090_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C090_6DATA) ,BYD_CMD_C090_6DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C1A6_0DATA) ,BYD_CMD_C1A6_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_C1A6_3DATA) ,BYD_CMD_C1A6_3DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CE80_0DATA) ,BYD_CMD_CE80_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CE80_6DATA) , BYD_CMD_CE80_6DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CE90_0DATA) ,BYD_CMD_CE90_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CE90_6DATA) ,BYD_CMD_CE90_6DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CEA0_0DATA) ,BYD_CMD_CEA0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CEA0_12DATA) ,BYD_CMD_CEA0_12DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_00AD_0DATA) ,BYD_CMD_00AD_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_00AD_1DATA) , BYD_CMD_00AD_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CEB0_0DATA) , BYD_CMD_CEB0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CEB0_14DATA) ,BYD_CMD_CEB0_14DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CEC0_0DATA) , BYD_CMD_CEC0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CEC0_14DATA),BYD_CMD_CEC0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CED0_0DATA) , BYD_CMD_CED0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CED0_14DATA),BYD_CMD_CED0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CFC6_0DATA) , BYD_CMD_CFC6_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CFC6_2DATA),BYD_CMD_CFC6_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CFC9_0DATA) , BYD_CMD_CFC9_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CFC9_1DATA),BYD_CMD_CFC9_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CBC0_0DATA) ,BYD_CMD_CBC0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CBC0_15DATA),BYD_CMD_CBC0_15DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CBD0_0DATA) ,BYD_CMD_CBD0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CBD0_15DATA),BYD_CMD_CBD0_15DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CBE0_0DATA) ,BYD_CMD_CBE0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CBE0_10DATA) ,BYD_CMD_CBE0_10DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CC80_0DATA) ,BYD_CMD_CC80_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CC80_10DATA),BYD_CMD_CC80_10DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CC90_0DATA) ,BYD_CMD_CC90_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CC90_15DATA),BYD_CMD_CC90_15DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CCA0_0DATA) , BYD_CMD_CCA0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CCA0_15DATA),BYD_CMD_CCA0_15DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CCB0_0DATA) , BYD_CMD_CCB0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CCB0_10DATA),BYD_CMD_CCB0_10DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CCC0_0DATA) ,BYD_CMD_CCC0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CCC0_15DATA),BYD_CMD_CCC0_15DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CCD0_0DATA) ,BYD_CMD_CCD0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_CCD0_15DATA),BYD_CMD_CCD0_15DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_0000_0DATA) ,BYD_CMD_0000_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_0000_1DATA),BYD_CMD_0000_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_E100_0DATA) ,BYD_CMD_E100_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_E100_16DATA),BYD_CMD_E100_16DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_E200_0DATA) , BYD_CMD_E200_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_E200_16DATA),BYD_CMD_E200_16DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_2000_0DATA) , BYD_CMD_2000_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(BYD_CMD_2000_1DATA),BYD_CMD_2000_1DATA},	
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_SLEEP_OFF_DELAY,sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_DISPLAY_ON_DELAY,sizeof(display_on), display_on},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_DISPLAY_ON_DELAY,sizeof(BYD_CMD_2C00_0DATA),BYD_CMD_2C00_0DATA },
	
	#endif 
};

static struct dsi_cmd_desc otm8018bdisplay_truly_display_on_cmds[] = {
	
#if 1
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRUlY_CMD_FF00_0DATA) ,TRUlY_CMD_FF00_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_FF00_3DATA) ,TRULY_CMD_FF00_3DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_FF80_0DATA) ,TRULY_CMD_FF80_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_FF80_2DATA) ,TRULY_CMD_FF80_2DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_FF03_0DATA) , TRULY_CMD_FF03_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_FF03_1DATA) , TRULY_CMD_FF03_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C0B4_0DATA) ,TRULY_CMD_C0B4_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C0B4_1DATA) , TRULY_CMD_C0B4_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C582_0DATA) , TRULY_CMD_C582_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C582_1DATA) ,TRULY_CMD_C582_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C590_0DATA) ,TRULY_CMD_C590_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C590_2DATA) ,TRULY_CMD_C590_2DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_D800_0DATA) ,TRULY_CMD_D800_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_D800_2DATA) ,TRULY_CMD_D800_2DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_D900_0DATA) ,TRULY_CMD_D900_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_D900_1DATA) ,TRULY_CMD_D900_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_E100_0DATA) ,TRULY_CMD_E100_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_E100_16DATA) , TRULY_CMD_E100_16DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_E200_0DATA) , TRULY_CMD_E200_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_E200_16DATA) ,TRULY_CMD_E200_16DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C181_0DATA) ,TRULY_CMD_C181_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_c181_1DATA) ,TRULY_CMD_c181_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C1A1_0DATA) , TRULY_CMD_C1A1_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C1A1_1DATA) ,TRULY_CMD_C1A1_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C489_0DATA) ,TRULY_CMD_C489_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C489_1DATA) ,TRULY_CMD_C489_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C0A2_0DATA) ,TRULY_CMD_C0A2_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C0A2_3DATA) ,TRULY_CMD_C0A2_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C481_0DATA) ,TRULY_CMD_C481_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C481_1DATA) ,TRULY_CMD_C481_1DATA },

        {DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C480_0DATA) ,TRULY_CMD_C480_0DATA }, //add 130124
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C480_1DATA) ,TRULY_CMD_C480_1DATA },


	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C592_0DATA) ,TRULY_CMD_C592_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C592_1DATA) ,TRULY_CMD_C592_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C5B1_0DATA) ,TRULY_CMD_C5B1_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C5B1_1DATA) , TRULY_CMD_C5B1_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C090_0DATA) ,TRULY_CMD_C090_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C090_6DATA) ,TRULY_CMD_C090_6DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C1A6_0DATA) ,TRULY_CMD_C1A6_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_C1A6_3DATA),TRULY_CMD_C1A6_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CE80_0DATA) , TRULY_CMD_CE80_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CE80_12DATA),TRULY_CMD_CE80_12DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CEA0_0DATA) , TRULY_CMD_CEA0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CEA0_14DATA),TRULY_CMD_CEA0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CEB0_0DATA) ,TRULY_CMD_CEB0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CEB0_14DATA),TRULY_CMD_CEB0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CEC0_0DATA) ,TRULY_CMD_CEC0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CEC0_14DATA),TRULY_CMD_CEC0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CED0_0DATA) ,TRULY_CMD_CED0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CED0_14DATA),TRULY_CMD_CED0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CFC7_0DATA) , TRULY_CMD_CFC7_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CFC7_1DATA) ,TRULY_CMD_CFC7_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CFC9_0DATA) , TRULY_CMD_CFC9_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CFC9_1DATA),TRULY_CMD_CFC9_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CBC4_0DATA) , TRULY_CMD_CBC4_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CBC4_6DATA),TRULY_CMD_CBC4_6DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CBD9_0DATA) ,TRULY_CMD_CBD9_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CBD9_6DATA),TRULY_CMD_CBD9_6DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CC84_0DATA) , TRULY_CMD_CC84_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CC84_6DATA),TRULY_CMD_CC84_6DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CC9E_0DATA) , TRULY_CMD_CC9E_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CC9E_1DATA),TRULY_CMD_CC9E_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CCA0_0DATA) , TRULY_CMD_CCA0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CCA0_5DATA),TRULY_CMD_CCA0_5DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CCB4_0DATA) ,TRULY_CMD_CCB4_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CCB4_6DATA) ,TRULY_CMD_CCB4_6DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CCCE_0DATA),TRULY_CMD_CCCE_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CCCE_1DATA) , TRULY_CMD_CCCE_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CCD0_0DATA),TRULY_CMD_CCD0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_CCD0_5DATA) ,TRULY_CMD_CCD0_5DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_3A00_0DATA),TRULY_CMD_3A00_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(TRULY_CMD_3A00_1DATA) , TRULY_CMD_3A00_1DATA},	
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_SLEEP_OFF_DELAY,sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_DISPLAY_ON_DELAY,sizeof(display_on), display_on},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_DISPLAY_ON_DELAY,sizeof(TRULY_CMD_2C00_0DATA),TRULY_CMD_2C00_0DATA },
	
#endif 
};

static struct dsi_cmd_desc otm8018bdisplay_tianma_display_on_cmds[] = {
	
	#if 1
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF00_0DATA) ,CMD_FF00_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF00_3DATA) ,CMD_FF00_3DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF80_0DATA) ,CMD_FF80_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF80_2DATA) ,CMD_FF80_2DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF03_0DATA) ,CMD_FF03_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF03_1DATA) ,CMD_FF03_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C0B4_0DATA) , CMD_C0B4_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C0B4_1DATA) , CMD_C0B4_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C489_0DATA) ,CMD_C489_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C489_1DATA) , CMD_C489_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C0A2_0DATA) , CMD_C0A2_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C0A2_3DATA) ,CMD_C0A2_3DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CFC9_0DATA) ,CMD_CFC9_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CFC9_1DATA) ,CMD_CFC9_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C594_0DATA) ,CMD_C594_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C594_1DATA) ,CMD_C594_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C582_0DATA) ,CMD_C582_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C582_1DATA) ,CMD_C582_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C590_0DATA) ,CMD_C590_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C590_2DATA) , CMD_C590_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_D800_0DATA) ,CMD_D800_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_D800_2DATA) , CMD_D800_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_D900_0DATA) , CMD_D900_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_D900_1DATA) , CMD_D900_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C181_0DATA) ,CMD_C181_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C181_1DATA) ,CMD_C181_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A1_0DATA) , CMD_C1A1_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A1_1DATA) , CMD_C1A1_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_3600_0DATA) , CMD_3600_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_3600_1DATA) , CMD_3600_1DATA},
	/*************************start tianma add*******************************************/
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_B391_0DATA) , CMD_B391_0DATA},
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_B391_1DATA) , CMD_B391_1DATA},
	/*************************end tianma add*******************************************/
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C481_0DATA) , CMD_C481_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C481_1DATA) ,CMD_C481_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C592_0DATA) ,CMD_C592_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C592_1DATA) , CMD_C592_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C5B1_0DATA) ,CMD_C5B1_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C5B1_1DATA),CMD_C5B1_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A60_0DATA) ,CMD_C1A60_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A60_1DATA),CMD_C1A60_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C5C0_0DATA) ,CMD_C5C0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C5C0_1DATA),CMD_C5C0_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_B08B_0DATA) ,CMD_B08B_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_B08B_1DATA),CMD_B08B_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_F5B2_0DATA) ,CMD_F5B2_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_F5B2_4DATA),CMD_F5B2_4DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C593_0DATA) ,CMD_C593_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C593_1DATA),CMD_C593_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C090_0DATA) , CMD_C090_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C090_6DATA) ,CMD_C090_6DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A6_0DATA) ,CMD_C1A6_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A6_3DATA),CMD_C1A6_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CE80_0DATA) , CMD_CE80_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CE80_6DATA),CMD_CE80_6DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CEA0_0DATA) ,CMD_CEA0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CEA0_14DATA),CMD_CEA0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CEB0_0DATA) , CMD_CEB0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CEB0_14DATA),CMD_CEB0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBC0_0DATA) ,CMD_CBC0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBC0_2DATA),CMD_CBC0_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBC6_0DATA) ,CMD_CBC6_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBC6_3DATA),CMD_CBC6_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBD5_0DATA) , CMD_CBD5_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBD5_2DATA),CMD_CBD5_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBDB_0DATA) ,CMD_CBDB_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBDB_3DATA),CMD_CBDB_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC80_0DATA) ,CMD_CC80_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC80_2DATA),CMD_CC80_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC86_0DATA) ,CMD_CC86_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC86_3DATA),CMD_CC86_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC9A_0DATA) ,CMD_CC9A_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC9A_2DATA),CMD_CC9A_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCA1_0DATA) ,CMD_CCA1_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCA1_3DATA),CMD_CCA1_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCB0_0DATA) , CMD_CCB0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCB0_2DATA),CMD_CCB0_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCB6_0DATA) , CMD_CCB6_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCB6_3DATA),CMD_CCB6_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCCA_0DATA) ,CMD_CCCA_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCCA_2DATA),CMD_CCCA_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCD1_0DATA) , CMD_CCD1_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCD1_3DATA),CMD_CCD1_3DATA},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_SLEEP_OFF_DELAY,sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_DISPLAY_ON_DELAY,sizeof(display_on), display_on},
	
	#endif
};

#else
static struct dsi_cmd_desc otm8018bdisplay_tianma_display_on_cmds[] = {
	
	#if 1
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF00_0DATA) ,CMD_FF00_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF00_3DATA) ,CMD_FF00_3DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF80_0DATA) ,CMD_FF80_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF80_2DATA) ,CMD_FF80_2DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF03_0DATA) ,CMD_FF03_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_FF03_1DATA) ,CMD_FF03_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C0B4_0DATA) , CMD_C0B4_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C0B4_1DATA) , CMD_C0B4_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C489_0DATA) ,CMD_C489_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C489_1DATA) , CMD_C489_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C0A2_0DATA) , CMD_C0A2_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C0A2_3DATA) ,CMD_C0A2_3DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CFC9_0DATA) ,CMD_CFC9_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CFC9_1DATA) ,CMD_CFC9_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C594_0DATA) ,CMD_C594_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C594_1DATA) ,CMD_C594_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C582_0DATA) ,CMD_C582_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C582_1DATA) ,CMD_C582_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C590_0DATA) ,CMD_C590_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C590_2DATA) , CMD_C590_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_D800_0DATA) ,CMD_D800_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_D800_2DATA) , CMD_D800_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_D900_0DATA) , CMD_D900_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_D900_1DATA) , CMD_D900_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C181_0DATA) ,CMD_C181_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C181_1DATA) ,CMD_C181_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A1_0DATA) , CMD_C1A1_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A1_1DATA) , CMD_C1A1_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_3600_0DATA) , CMD_3600_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_3600_1DATA) , CMD_3600_1DATA},
	/*************************start tianma add*******************************************/
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_B391_0DATA) , CMD_B391_0DATA},
	//{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_B391_1DATA) , CMD_B391_1DATA},
	/*************************end tianma add*******************************************/
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C481_0DATA) , CMD_C481_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C481_1DATA) ,CMD_C481_1DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C592_0DATA) ,CMD_C592_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C592_1DATA) , CMD_C592_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C5B1_0DATA) ,CMD_C5B1_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C5B1_1DATA),CMD_C5B1_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A60_0DATA) ,CMD_C1A60_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A60_1DATA),CMD_C1A60_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C5C0_0DATA) ,CMD_C5C0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C5C0_1DATA),CMD_C5C0_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_B08B_0DATA) ,CMD_B08B_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_B08B_1DATA),CMD_B08B_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_F5B2_0DATA) ,CMD_F5B2_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_F5B2_4DATA),CMD_F5B2_4DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C593_0DATA) ,CMD_C593_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C593_1DATA),CMD_C593_1DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C090_0DATA) , CMD_C090_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C090_6DATA) ,CMD_C090_6DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A6_0DATA) ,CMD_C1A6_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_C1A6_3DATA),CMD_C1A6_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CE80_0DATA) , CMD_CE80_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CE80_6DATA),CMD_CE80_6DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CEA0_0DATA) ,CMD_CEA0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CEA0_14DATA),CMD_CEA0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CEB0_0DATA) , CMD_CEB0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CEB0_14DATA),CMD_CEB0_14DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBC0_0DATA) ,CMD_CBC0_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBC0_2DATA),CMD_CBC0_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBC6_0DATA) ,CMD_CBC6_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBC6_3DATA),CMD_CBC6_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBD5_0DATA) , CMD_CBD5_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBD5_2DATA),CMD_CBD5_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBDB_0DATA) ,CMD_CBDB_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CBDB_3DATA),CMD_CBDB_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC80_0DATA) ,CMD_CC80_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC80_2DATA),CMD_CC80_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC86_0DATA) ,CMD_CC86_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC86_3DATA),CMD_CC86_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC9A_0DATA) ,CMD_CC9A_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CC9A_2DATA),CMD_CC9A_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCA1_0DATA) ,CMD_CCA1_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCA1_3DATA),CMD_CCA1_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCB0_0DATA) , CMD_CCB0_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCB0_2DATA),CMD_CCB0_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCB6_0DATA) , CMD_CCB6_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCB6_3DATA),CMD_CCB6_3DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCCA_0DATA) ,CMD_CCCA_0DATA },
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCCA_2DATA),CMD_CCCA_2DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCD1_0DATA) , CMD_CCD1_0DATA},
	{DTYPE_GEN_LWRITE, 1, 0, 0, OTM8018BDISPLAY_CMD_DELAY,sizeof(CMD_CCD1_3DATA),CMD_CCD1_3DATA},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_SLEEP_OFF_DELAY,sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE, 1, 0, 0, OTM8018BDISPLAY_DISPLAY_ON_DELAY,sizeof(display_on), display_on},
	
	#endif
};

#endif


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
#if defined (TYQ_TBW5939_SUPPORT)
#define LCD_ID 77
#else
#define LCD_ID 114
#endif

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
		
	#ifdef DRV_READ_LCDID_MISC_IOCTL
		if(lcd_id_flag){
			mipi_dsi_cmds_tx(&otm8018b_tx_buf, otm8018bdisplay_truly_display_on_cmds,
				ARRAY_SIZE(otm8018bdisplay_truly_display_on_cmds));
			}
		else if(tp_id_flag==0){
			mipi_dsi_cmds_tx( &otm8018b_tx_buf, otm8018bdisplay_byd_display_on_cmds,
				ARRAY_SIZE(otm8018bdisplay_byd_display_on_cmds));
			}
		else {
			mipi_dsi_cmds_tx(&otm8018b_tx_buf, otm8018bdisplay_tianma_display_on_cmds,
				ARRAY_SIZE(otm8018bdisplay_tianma_display_on_cmds));

		}
	#else	
		mipi_dsi_cmds_tx(&otm8018b_tx_buf, otm8018bdisplay_tianma_display_on_cmds,
				ARRAY_SIZE(otm8018bdisplay_tianma_display_on_cmds));
	#endif
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
