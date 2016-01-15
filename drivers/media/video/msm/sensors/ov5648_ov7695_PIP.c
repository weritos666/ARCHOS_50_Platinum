/* Copyright (c) 2012, The Linux Foundation. All Rights Reserved.
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
       
#ifndef _PIP_H_
#define _PIP_H_

#include "msm_sensor.h"
#include "msm.h"

#define SUB_180_PIP 1
#define SUB_180_SUB 1       

/////////////////////PIP init///////////////////////////////////////////////////////
struct msm_camera_i2c_reg_conf regPIP_Init_OV7695[] = 
{ 
//// OV7695
{ 0x0103,0x01},// software reset
{ 0x0101,0x00},	// Mirror off
{ 0x3811,0x06},
{ 0x3813,0x06},
{ 0x382c,0x05},	// manual control ISP window offset 0x3810~0x3813 during subsampled modes
{ 0x3620,0x28},
{ 0x3623,0x12},
{ 0x3718,0x88},
{ 0x3632,0x05},
{ 0x3013,0xd0},
{ 0x3717,0x19},
{ 0x034c,0x01},
{ 0x034d,0x40},
{ 0x034e,0x00},
{ 0x034f,0xf0},
{ 0x0383,0x03},// x odd inc
{ 0x4500,0x47},// x sub control
{ 0x0387,0x03},// y odd inc
{ 0x4300,0xf8},// output RGB raw
{ 0x0301,0x0a},// PLL
{ 0x0307,0x50},// PLL
{ 0x030b,0x04},// PLL
{ 0x3106,0x91},// PLL
{ 0x301e,0x60},
{ 0x3014,0x30},
{ 0x301a,0x44},
{ 0x4803,0x08},// HS prepare in UI
{ 0x370a,0x63},
{ 0x520a,0xf4},// red gain from 0x400 to 0xfff
{ 0x520b,0xf4},// green gain from 0x400 to 0xfff
{ 0x520c,0xf4},// blue gain from 0x400 to 0xfff

{0x4008, 0x01	},// bl start
{0x4009, 0x04	},// bl end
{0x0340, 0x03	},// VTS = 992
{0x0341, 0xe0	},// VTS
{0x0342, 0x02	},// HTS = 540
{0x0343, 0x1c	},// HTS
{0x0348, 0x02	},// x end = 639
{0x0349, 0x7f	},// x end
{0x034a, 0x01	},// y end = 479
{0x034b, 0xdf	},// y end

{0x3820, 0x94	},// by pass MIPI, v bin off,
{0x4f05, 0x01	},// SPI 2 lane on
{0x4f06, 0x02	},// SPI on
{0x3012, 0x00 } ,
{0x3005, 0x0f	},// SPI pin on
{0x3001, 0x1f	},// SPI drive 3.75x, FSIN drive 1.75x
{0x3823, 0x30 } ,
{0x4303, 0xee	},// Y max L
{0x4307, 0xee	},// U max L
{0x430b, 0xee	},// V max L
{0x3620, 0x2f } ,
{0x3621, 0x77 } ,
{0x0100, 0x01	},// stream on

//// PIP Demo
{0x3503 ,0x30},// delay gain for one frame, and mannual enable.
{0x5000 ,0x8f},// Gama OFF, AWB OFF
{0x5002 ,0x8a},// 50Hz/60Hz select£¬ 88 for 60Hz, 8a for 50hz
{0x3a0f ,0x38},	// AEC in H
{0x3a10 ,0x30},// AEC in L
{0x3a1b ,0x38},// AEC out H
{0x3a1e ,0x30},// AEC out L
{0x3a11 ,0x70},// control zone H
{0x3a1f ,0x18},// control zone L
{0x3a18 ,0x00},// gain ceiling = 16x
{0x3a19 ,0xf8},// gain ceiling
{0x3a08 ,0x00},	// B50
{0x3a09 ,0xde},// B50
{0x3a0a ,0x00},// B60
{0x3a0b ,0xb9},// B60
{0x3a0e ,0x03},	// B50 step
{0x3a0d ,0x04},	// B60 step
{0x3a02 ,0x02},// max expo 60
{0x3a03 ,0xe6},// max expo 60
{0x3a14 ,0x02},// max expo 50
{0x3a15 ,0xe6},// max expo 50

//lens A Light
{0x5100 ,0x01},	// red x0
{0x5101 ,0xc0},	// red x0
{0x5102 ,0x01},	// red y0
{0x5103 ,0x00},	// red y0
{0x5104 ,0x4b},	// red a1
{0x5105 ,0x04},	// red a2
{0x5106 ,0xc2},	// red b1
{0x5107 ,0x08},	// red b2
{0x5108 ,0x01},	// green x0
{0x5109 ,0xd0},	// green x0
{0x510a ,0x01},	// green y0
{0x510b ,0x00},	// green y0
{0x510c ,0x30},	// green a1
{0x510d ,0x04},	// green a2
{0x510e ,0xc2},	// green b1
{0x510f ,0x08},	// green b2
{0x5110 ,0x01},	// blue x0
{0x5111 ,0xd0},	// blue x0
{0x5112 ,0x01},	// blue y0
{0x5113 ,0x00},	// blue y0
{0x5114 ,0x26}, 	// blue a1
{0x5115 ,0x04}, 	// blue a2
{0x5116 ,0xc2},	// blue b1
{0x5117 ,0x08},	// blue b2
};
    
struct msm_camera_i2c_reg_conf regPIP_Init_OV5648[] = 
{
//// OV5648
{0x0103 ,0x01},// software reset
{0x3001 ,0x00},// D[7:0] set to input
{0x3002 ,0x80},// Vsync output, PCLK, FREX, Strobe, CSD, CSK, GPIO input
{0x3011 ,0x02},// Drive strength 2x
{0x3018 ,0x4c},// MIPI 2 lane
{0x301c ,0xf0},// OTP enable
{0x3022 ,0x00},// MIP lane enable
{0x3034 ,0x1a},// 10-bit mode
{0x3035 ,0x21},// PLL
{0x3036 ,0x69},// PLL
{0x3037 ,0x03},// PLL
{0x3038 ,0x00},// PLL
{0x3039 ,0x00},// PLL
{0x303a ,0x00},// PLLS
{0x303b ,0x19},// PLLS
{0x303c ,0x11},// PLLS
{0x303d ,0x30},// PLLS
{0x3105 ,0x11},
{0x3106 ,0x05},// PLL
{0x3304 ,0x28},
{0x3305 ,0x41},
{0x3306 ,0x30},
{0x3308 ,0x00},
{0x3309 ,0xc8},
{0x330a ,0x01},
{0x330b ,0x90},
{0x330c ,0x02},
{0x330d ,0x58},
{0x330e ,0x03},
{0x330f ,0x20},
{0x3300 ,0x00},

{0x3500 ,0x00},// exposure [19:16]
{0x3501 ,0x2d},// exposure [15:8]
{0x3502 ,0x60},// exposure [7:0], exposure = 0x2d6 = 726
{0x3503 ,0x07},// gain has no delay, manual agc/aec
{0x350a ,0x00},// gain[9:8]
{0x350b ,0x40},// gain[7:0], gain = 4x

//analog control
{0x3601 ,0x33},
{0x3602 ,0x00},
{0x3611 ,0x0e},
{0x3612 ,0x2b},
{0x3614 ,0x50},
{0x3620 ,0x33},
{0x3622 ,0x00},
{0x3630 ,0xad},
{0x3631 ,0x00},
{0x3632 ,0x94},
{0x3633 ,0x17},
{0x3634 ,0x14},
{0x3705 ,0x2a},
{0x3708 ,0x66},
{0x3709 ,0x52},
{0x370b ,0x23},
{0x370c ,0xc3},
{0x370d ,0x00},
{0x370e ,0x00},
{0x371c ,0x07},
{0x3739 ,0xd2},
{0x373c ,0x00},

{0x3800 ,0x00},	// xstart = 0
{0x3801 ,0x00},	// xstart
{0x3802 ,0x00},	// ystart = 0
{0x3803 ,0x00},	// ystart
{0x3804 ,0x0a},	// xend = 2639
{0x3805 ,0x4f},	// xend
{0x3806 ,0x07},	// yend = 1955
{0x3807 ,0xa3},	// yend
{0x3808 ,0x05},	// x output size = 1296
{0x3809 ,0x10},	// x output size
{0x380a ,0x03},	// y output size = 972
{0x380b ,0xcc},	// y output size
{0x380c ,0x0e},	// hts = 3780
{0x380d ,0xc4},	// hts
{0x380e ,0x03},	// vts = 992
{0x380f ,0xe0},	// vts
{0x3810 ,0x00},	// isp x win = 8
{0x3811 ,0x08},	// isp x win
{0x3812 ,0x00},	// isp y win = 2
{0x3813 ,0x02},	// isp y win
{0x3814 ,0x31},	// x inc
{0x3815 ,0x31},	// y inc
{0x3817 ,0x00},	// hsync start
{0x3820 ,0x08},	// v flip off
{0x3821 ,0x07}, 	// h mirror on
{0x3826 ,0x03},
{0x3829 ,0x00},
{0x382b ,0x0b},
{0x3830 ,0x00},
{0x3836 ,0x01},
{0x3837 ,0xd4},
{0x3838 ,0x02},
{0x3839 ,0xe0},
{0x383a ,0x0a},
{0x383b ,0x50},

{0x3b00 ,0x00},// strobe off
{0x3b02 ,0x08},// shutter delay 
{0x3b03 ,0x00},// shutter delay 
{0x3b04 ,0x04},// frex_exp
{0x3b05 ,0x00},// frex_exp
{0x3b06 ,0x04},
{0x3b07 ,0x08},// frex inv
{0x3b08 ,0x00},// frex exp req
{0x3b09 ,0x02},// frex end option
{0x3b0a ,0x04},// frex rst length
{0x3b0b ,0x00},// frex strobe width
{0x3b0c ,0x3d},// frex strobe width

{0x3f01 ,0x0d},
{0x3f0f ,0xf5},

{0x4000 ,0x89},// blc enable
{0x4001 ,0x02},// blc start line
{0x4002 ,0x45},// blc auto, reset frame number = 5
{0x4004 ,0x02},// black line number
{0x4005 ,0x18},// blc normal freeze
{0x4006 ,0x08},
{0x4007 ,0x10},
{0x4008 ,0x00},

{0x4300 ,0xf8},
{0x4303 ,0xff},
{0x4304 ,0x00},
{0x4307 ,0xff},
{0x4520 ,0x00},
{0x4521 ,0x00},
{0x4511 ,0x22},

{0x481f ,0x3c},// MIPI clk prepare min
{0x4826 ,0x00},// MIPI hs prepare min
{0x4837 ,0x18},// MIPI global timing
{0x4b00 ,0x44},
{0x4b01 ,0x0a},
{0x4b04 ,0x30},
{0x5000 ,0xff},// bpc on, wpc on
{0x5001 ,0x00},// AWB ON 00
{0x5002 ,0x40},	// AWB Gain ON 40
{0x5003 ,0x0b},// buf en, bin auto en
{0x5004 ,0x0c},// size man on
{0x5043 ,0x00},
{0x5013 ,0x00},
{0x501f ,0x03},// ISP output data
{0x503d ,0x00},// test pattern off
{0x5a00 ,0x08},
{0x5b00 ,0x01},
{0x5b01 ,0x40},
{0x5b02 ,0x00},
{0x5b03 ,0xf0},

//ISP AEC/AGC setting from here
{0x3503 ,0x33},      //delay gain for one frame, and mannual enable.
{0x5000 ,0x06}, 	// DPC On
{0x5001 ,0x00}, 	// AWB ON 
{0x5002 ,0x41}, 	// AWB Gain ON 
{0x383b ,0x80}, 	// 
{0x3a0f ,0x38},	// AEC in H
{0x3a10 ,0x30},	// AEC in L
{0x3a1b ,0x38},	// AEC out H
{0x3a1e ,0x30},	// AEC out L
{0x3a11 ,0x70},	// Control Zone H
{0x3a1f ,0x18},	// Conrol Zone L

//// @@ OV5648 Gain Table Alight
{0x5180 ,0x08}, 	// Manual AWB
//// A light Different with OV7695
{0x5186	,0x05}, 	// 04
{0x5187	,0x26}, 	// 57
{0x5188	,0x04}, 	// 04
{0x5189	,0xBF}, 	// 00
{0x518a	,0x04}, 	// 03
{0x518b	,0x00}, 	// 5E

};
/////////////////////PIP preview///////////////////////////////////////////////////////
struct msm_camera_i2c_reg_conf regPIP_Preview_OV7695[] =            
{                                        
//// OV7695
{0x382c, 0x05}, 	// manual control ISP window offset 0x3810~0x3813 during subsampled modes
// pip normal
//42 0101 00
//42 3811 06 	// shift one pixel
// pip rotate 180
{0x0101 ,0x03},
{0x3811 ,0x05},
{0x3813 ,0x06},

{0x034c ,0x01},// x output size = 320
{0x034d ,0x40},// x output size
{0x034e ,0x00},// y output size = 240
{0x034f ,0xf0},// y output size
{0x0383 ,0x03},// x odd inc
{0x4500 ,0x47},// h sub sample on
{0x0387 ,0x03},// y odd inc
{0x3820 ,0x94},// v bin off
{0x3014 ,0x30},
{0x301a ,0x44},
{0x370a ,0x63},
{0x4008 ,0x01},// blc start line
{0x4009 ,0x04},// blc end line
{0x0340 ,0x02},// VTS = 742
{0x0341 ,0xe6},// VTS
{0x0342 ,0x02},// HTS = 540
{0x0343 ,0x1c},// HTS
{0x0348 ,0x02},// x end = 639
{0x0349 ,0x7f},// x end
{0x034a ,0x01},// y end = 479
{0x034b ,0xdf},// y end

{0x3a09 ,0xde},// B50
{0x3a0b ,0xb9},	// B60
{0x3a0e ,0x03},	// B50 Max
{0x3a0d ,0x04},	// B60 Max
{0x3a02 ,0x02},// max expo 60
{0x3a03 ,0xe6},// max expo 60
{0x3a14 ,0x02},// max expo 50
{0x3a15 ,0xe6},// max expo 50          
};                                       
                                          
struct msm_camera_i2c_reg_conf regPIP_Preview_OV5648[] =            
{    
//1280 *720                                    
//// OV5648 wake up
{0x3022 ,0x00},	// MIP lane enable
{0x3012 ,0x01},
{0x3013 ,0x00},
{0x3014 ,0x0b},
{0x3005 ,0xf0},
{0x301a ,0xf0},
{0x301b ,0xf0},
{0x5000 ,0x06},	// turn on ISP
{0x5001 ,0x00},	// turn on ISP
{0x5002 ,0x41},	// turn on ISP
{0x5003 ,0x0B},	// turn on ISP


{0x3501 ,0x2d},
{0x3502 ,0x60},
{0x3708 ,0x66},
{0x3709 ,0x52},
{0x370c ,0xc3},

{0x3800 ,0x00},	// x start = 16
{0x3801 ,0x10},	// x start
{0x3802 ,0x00},	// y start = 254
{0x3803 ,0xfe},	// y start
{0x3804 ,0x0a},	// x end = 2607
{0x3805 ,0x2f},	// x end
{0x3806 ,0x06},	// y end = 1701
{0x3807 ,0xa5},	// y end
{0x3808 ,0x05},	// x output size = 1280
{0x3809 ,0x00},	// x output size
{0x380a ,0x02},	// y output size = 720
{0x380b ,0xd0},	// y output size
{0x380c ,0x0e},	// HTS = 3780
{0x380d ,0xc4},	// HTS
{0x380e ,0x02},	// VTS = 742
{0x380f ,0xe6},	// VTS
{0x3810 ,0x00},	// x offset = 8
{0x3811 ,0x08},	// x offset
{0x3812 ,0x00},	// y offset = 2
{0x3813 ,0x02},	// y offset
{0x3814 ,0x31},	// x inc
{0x3815 ,0x31},	// y inc
{0x3817 ,0x00},	// hsync start
{0x3820 ,0x08},	// v flip off, bin off
{0x3821 ,0x07}, 	// h mirror on, bin on

{0x3830 ,0x00},
{0x3838 ,0x02},
{0x3839 ,0xe0},
// pip normal
//6c 3836 01
//6c 3837 d4
//6c 383a 0a
//6c 383b 80
// pip rotate 180
{0x3836 ,0x01},
{0x3837 ,0xd7},
{0x383a ,0x0a},
{0x383b ,0x88},

{0x4004 ,0x02},	// black line number

{0x5b00 ,0x01},
{0x5b01 ,0x40},
{0x5b02 ,0x00},
{0x5b03 ,0xf0},

{0x4202, 0x0f},	// MIPI stream off        
{0x0100, 0x01},	// wake up from standby   
 };                                       
/////////////////////PIP video///////////////////////////////////////////////////////
struct msm_camera_i2c_reg_conf regPIP_Video_OV7695[] ={ 
#if 0                                                                                                                 
	//// OV7695                                                                                                      
	{0x382c, 0x05}, 	// manual control ISP window offset 0x3810~0x3813 during subsampled modes                        
#ifdef SUB_180_PIP
		{0x0101,0x03},
		{0x3811,0x05},
#else
		{0x0101,0x00},
		{0x3811,0x06},
#endif
	{0x3813, 0x06},                           
	{0x034c, 0x01},	// x output size = 320                                                                             
	{0x034d, 0x40},	// x output size                                                                                   
	{0x034e, 0x00},	// y output size = 240                                                                             
	{0x034f, 0xf0},	// y output size                                                                                   
	{0x0383, 0x03},	// x odd inc                                                                                       
	{0x4500, 0x47},	// h sub sample on                                                                                 
	{0x0387, 0x03},	// y odd inc                                                                                       
	{0x3820, 0x94},	// v bin off                                                                                       
	{0x3014, 0x30},                                                                                                    
	{0x301a, 0x44},                                                                                                    
	{0x370a, 0x63},                                                                                                    
	{0x4008, 0x01},	// blc start line                                                                                  
	{0x4009, 0x04},	// blc end line                                                                                    
	{0x0340, 0x02},	// VTS = 742                                                                                       
	{0x0341, 0xe6},	// VTS                                                                                             
	{0x0342, 0x02},	// HTS = 540                                                                                       
	{0x0343, 0x1c},	// HTS                                                                                             
                                                                                                    
	{0x3503, 0x30},	// AGC/AEC on                                                                                      
	{0x3a09, 0xde},	// B50                                                                                             
	{0x3a0b, 0xb9}, 	// B60                                                                                           
	{0x3a0e, 0x03}, 	// B50 Max                                                                                       
	{0x3a0d, 0x04}, 	// B60 Max                                                                                       
	{0x3a02, 0x02},	// max expo 60                                                                                     
	{0x3a03, 0xe6},	// max expo 60                                                                                     
	{0x3a14, 0x02},	// max expo 50                                                                                     
	{0x3a15, 0xe6},	// max expo 50
#endif                                                                                     
};                                                                                                                
                                                                                                                   
struct msm_camera_i2c_reg_conf regPIP_Video_OV5648[] ={ 
#if 0                                                                                                          
	//// OV5648 wake up                                                                                              
	{0x3022, 0x00},	// MIP lane enable                                                                                 
	{0x3012, 0x01},                                                                                                    
	{0x3013, 0x00},                                                                                                    
	{0x3014, 0x0b},                                                                                                    
	{0x3005, 0xf0},                                                                                                    
	{0x301a, 0xf0},                                                                                                    
	{0x301b, 0xf0},                                                                                                    
	{0x5000, 0x06},	// turn on ISP                                                                                     
	{0x5001, 0x00},	// turn on ISP                                                                                     
	{0x5002, 0x41},	// turn on ISP                                                                                     
	{0x5003, 0x0b},	// turn on ISP                                                                                     
                                                                                                   
	{0x3501, 0x2d},                                                                                                    
	{0x3502, 0x60},                                                                                                    
	{0x3708, 0x66},                                                                                                    
	{0x3709, 0x52},                                                                                                    
	{0x370c, 0xc3},                                                                                                    
                                                                                                  
	{0x3800, 0x00},	// x start = 16                                                                                    
	{0x3801, 0x10},	// x start                                                                                         
	{0x3802, 0x00},	// y start = 254                                                                                   
	{0x3803, 0xfe},	// y start                                                                                         
	{0x3804, 0x0a},	// x end = 2607                                                                                    
	{0x3805, 0x2f},	// x end                                                                                           
	{0x3806, 0x06},	// y end = 1701                                                                                    
	{0x3807, 0xa5},	// y end                                                                                           
	{0x3808, 0x05},	// x output size = 1280                                                                            
	{0x3809, 0x00},	// x output size                                                                                   
	{0x380a, 0x02},	// y output size = 720                                                                             
	{0x380b, 0xd0},	// y output size                                                                                   
	{0x380c, 0x0e},	// HTS = 3780                                                                                      
	{0x380d, 0xc4},	// HTS                                                                                             
	{0x380e, 0x02},	// VTS = 742                                                                                       
	{0x380f, 0xe6},	// VTS                                                                                             
	{0x3810, 0x00},	// x offset = 8                                                                                    
	{0x3811, 0x08},	// x offset                                                                                        
	{0x3812, 0x00},	// y offset = 2                                                                                    
	{0x3813, 0x02},	// y offset                                                                                        
	{0x3814, 0x31},	// x inc                                                                                           
	{0x3815, 0x31},	// y inc                                                                                           
	{0x3817, 0x00},	// hsync start                                                                                     
	{0x3820, 0x08},	// v flip off, bin off                                                                             
	{0x3821, 0x07}, 	// h mirror on, bin on                                                                           
                                                                                                   
	{0x3830, 0x00},                                                                                                    
	{0x3838, 0x02},                                                                                                    
	{0x3839, 0xe0}, 
#ifdef SUB_180_PIP
	{0x3836, 0x01},                                                                                                    
	{0x3837, 0xd7},                                                                                                    
	{0x383a, 0x0a},                                                                                                    
	{0x383b, 0x88},                                                                                                    
#else                                                                                                     
	{0x3836, 0x01},                                                                                                    
	{0x3837, 0xd4},                                                                                                    
	{0x383a, 0x0a},                                                                                                    
	{0x383b, 0x80},                                                                                                    
#endif                                                                                              
	{0x4004, 0x02},	// black line number                                                                               
                                                                                                    
	{0x5b00, 0x01},                                                                                                    
	{0x5b01, 0x40},                                                                                                    
	{0x5b02, 0x00},                                                                                                    
	{0x5b03, 0xf0},                                                                                                    
                                                                                                  
	{0x4202, 0x0f},	// MIPI stream off                                                                                 
	{0x0100, 0x01},	// wake up fromm software standby  
#endif                                                               
};                                                                                                                
/////////////////////PIP capture///////////////////////////////////////////////////////
struct msm_camera_i2c_reg_conf regPIP_Capture_OV7695[] =                                                                     
{                                                                                                 
//// OV7695
{0x382c ,0xc5}, 	// manual control ISP window offset 0x3810~0x3813 during subsampled modes
//pip normal normal
//42 0101 00
//42 3811 06
//pip rotate 180
{0x0101 ,0x03},
{0x3811 ,0x05}, 	//

{0x3813 ,0x06},
{0x034c ,0x02},	// x output size = 640
{0x034d ,0x80},	// x output size
{0x034e ,0x01},	// y output size = 480
{0x034f ,0xe0},	// y output size
{0x0383 ,0x01},	// x odd inc
{0x4500 ,0x25},	// h sub sample off
{0x0387 ,0x01},	// y odd inc
{0x3820 ,0x90},	// v bin off
{0x3014 ,0x0f},
{0x301a ,0xf0},
{0x370a ,0x23},
{0x4008 ,0x02},	// blc start line
{0x4009 ,0x09},	// blc end line
{0x0340 ,0x05},	// VTS = 1480
{0x0341 ,0xc8},	// VTS
{0x0342 ,0x03},	// HTS = 806
{0x0343 ,0x26},	// HTS
{0x0348 ,0x02},	// x end = 639
{0x0349 ,0x7f},	// x end
{0x034a ,0x01},	// y end = 479
{0x034b ,0xdf},	// y end

{0x3a09 ,0x95},	// B50 L
{0x3a0b ,0x7c}, 	// B60 L
{0x3a0e ,0x0a}, 	// B50Max
{0x3a0d ,0x0c}, 	// B60Max
{0x3a02 ,0x05},	// max expo 60
{0x3a03 ,0xc8},	// max expo 60
{0x3a14 ,0x05},	// max expo 50
{0x3a15 ,0xc8},	// max expo 50                                                                 
};                                                                                                 
                                                                                                   
struct msm_camera_i2c_reg_conf regPIP_Capture_OV5648[] =                                                                      
{                                                                                                  
//// OV5648 wake up
{0x3022 ,0x00},// MIP lane enable
{0x3012 ,0x01},
{0x3013 ,0x00},
{0x3014 ,0x0b},
{0x3005 ,0xf0},
{0x301a ,0xf0},
{0x301b ,0xf0},
{0x5000 ,0x06},// turn on ISP
{0x5001 ,0x00},// turn on ISP
{0x5002 ,0x41},// turn on ISP
{0x5003 ,0x0B},// turn on ISP
{0x3501 ,0x7b},
{0x3502 ,0x00},
{0x3708 ,0x63},
{0x3709 ,0x12},
{0x370c ,0xc0},

{0x3800 ,0x00},// x start = 16
{0x3801 ,0x10},// x start
{0x3802 ,0x00},// y start = 254
{0x3803 ,0xfe},// y start
{0x3804 ,0x0a},// x end = 2607
{0x3805 ,0x2f},// x end
{0x3806 ,0x06},// y end = 1701
{0x3807 ,0xa5},// y end

//6c 3800 00	// x start = 0
//6c 3801 00	// x start
//6c 3802 00	// y start = 0
//6c 3803 00	// y start
//6c 3804 0a	// x end = 2623
//6c 3805 3f	// x end
//6c 3806 07	// y end = 1955
//6c 3807 a3	// y end

{0x3808 ,0x0a},	// x output size = 2560
{0x3809 ,0x00},	// x output size
{0x380a ,0x05},	// y output size = 1440
{0x380b ,0xa0},	// y output size

{0x380c ,0x16},	// HTS = 5642
{0x380d ,0x0a},	// HTS
{0x380e ,0x05},	// VTS = 1480
{0x380f ,0xc8},	// VTS
{0x3810 ,0x00},	// x offset = 16
{0x3811 ,0x10},	// x offset
{0x3812 ,0x00},	// y offset = 6
{0x3813 ,0x06},	// y offset
{0x3814 ,0x11},	// x inc
{0x3815 ,0x11},	// y inc
{0x3817 ,0x00},	// h sync start
{0x3820 ,0x40},	// ISP vertical flip on, v bin off
{0x3821 ,0x06}, 	// mirror on, h bin off

{0x3830 ,0x00},
{0x3838 ,0x06},
{0x3839 ,0x00},
// pip normal
//6c 3836 0b
//6c 3837 39
//6c 383a 01
//6c 383b b8 	// 80 PIP Location
// pip rotate 180
{0x3836 ,0x07},
{0x3837 ,0x3f},
{0x383a ,0x01},
{0x383b ,0xa8}, 	// 80 PIP Location

{0x4004 ,0x04},	// black line number

{0x5b00 ,0x02},
{0x5b01 ,0x80},
{0x5b02 ,0x01},
{0x5b03 ,0xe0},     

{0x4202, 0x0f},	// MIPI stream off                                                                 
{0x0100, 0x01},	// wake up from software standby                                                       
};
///////////////////////////PIP sub camera//////////////////////////////////
 // OV5648 SCLK = 84Mhz
	 // OV5648 MIPI 2 lane
	 // OV5648 MIPI data rate = 420Mbps
struct msm_camera_i2c_reg_conf regPIP_sub_OV7695[] = 
{
//@@ 0 11 OV7695 Sub Camera 640x480 30fps
// OV5648 SCLK = 84Mhz
// OV5648 MIPI 2 lane
// OV5648 MIPI data rate = 420Mbps

//100 99 640 480
//102 84 1 ffff
//102 3800 1   //use defined register-slave camera

//6c 0100 00	// stream off

{0x382c ,0xc5},	// manual control ISP window offset 0x3810~0x3813 during subsampled modes
//normal
//42 0101 00
//42 3811 06
//42 3813 06
// sub mirror
{0x0101 ,0x01},
{0x3811 ,0x05},
{0x3813 ,0x06},

{0x034c ,0x02},	// x output size = 640
{0x034d ,0x80},	// x output size
{0x034e ,0x01},	// y output size = 480
{0x034f ,0xe0},	// y output size
{0x0383 ,0x01},	// x odd inc
{0x4500 ,0x25},	// h sub sample off
{0x0387 ,0x01},	// y odd inc
{0x3820 ,0x94},	// v bin off
{0x3014 ,0x0f},
{0x301a ,0xf0},
{0x370a ,0x23},
{0x4008 ,0x02},	// blc start line
{0x4009 ,0x09},	// blc end line

{0x0340 ,0x02},	// OV7695 VTS = 536
{0x0341 ,0x18},	// OV6595 VTS
{0x0342 ,0x02},	// OV7695 HTS = 746
{0x0343 ,0xea},	// OV7695 HTS

{0x3a09 ,0xa1},	// B50
{0x3a0b ,0x86}, 	// B60
{0x3a0e ,0x03}, 	// B50 steps
{0x3a0d ,0x04}, 	// B60 steps
{0x3a02 ,0x02},	// max expo 60
{0x3a03 ,0x18},	// max expo 60
{0x3a14 ,0x02},	// max expo 50
{0x3a15 ,0x18},	// max expo 50
};

struct msm_camera_i2c_reg_conf regPIP_sub_OV5648[] =
{
{0x3830 ,0x40},
{0x3808 ,0x02},	// Timing X output Size = 640
{0x3809 ,0x80},	// Timing X output Size
{0x380a ,0x01},	// Timing Y output Size = 480
{0x380b ,0xe0},	// Timing Y output Size

{0x380e ,0x02},	// OV5648 VTS = 536
{0x380f ,0x18},	// OV5648 VTS
{0x380c ,0x14},	// OV5648 HTS = 5222
{0x380d ,0x66},	// OV5648 HTS
{0x0348 ,0x02},	// x end = 639
{0x0349 ,0x7f},	// x end
{0x034a ,0x01},	// y end = 479
{0x034b ,0xdf},	// y end

//// OV5648 Block Power OFF 
{0x3022 ,0x03},	// pull down data lane 1/2
{0x3012 ,0xa1},	// debug mode
{0x3013 ,0xf0},	// debug mode
{0x3014 ,0x7b},	// 
{0x3005 ,0xf3},	// debug mode

{0x301a ,0x96},	// debug mode
{0x301b ,0x5a},	// debug mode
{0x5000 ,0x00},	// turn off ISP
{0x5001 ,0x00},	// turn off ISP
{0x5002 ,0x00},	// turn off ISP
{0x5003 ,0x00},	// turn off ISP

{0x0100 ,0x01},	// stream on
};

struct msm_camera_i2c_conf_array ov5648_ov7695_PIP_confs[] = {
	{&regPIP_Init_OV5648[0],
	ARRAY_SIZE(regPIP_Init_OV5648), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&regPIP_Init_OV7695[0],
	ARRAY_SIZE(regPIP_Init_OV7695), 0, MSM_CAMERA_I2C_BYTE_DATA}, 

	{&regPIP_Preview_OV5648[0],
	ARRAY_SIZE(regPIP_Preview_OV5648), 0, MSM_CAMERA_I2C_BYTE_DATA}, 
	{&regPIP_Preview_OV7695[0],
	ARRAY_SIZE(regPIP_Preview_OV7695), 0, MSM_CAMERA_I2C_BYTE_DATA}, 

	{&regPIP_Video_OV5648[0],
	ARRAY_SIZE(regPIP_Video_OV5648), 0, MSM_CAMERA_I2C_BYTE_DATA}, 
	{&regPIP_Video_OV7695[0],
	ARRAY_SIZE(regPIP_Video_OV7695), 0, MSM_CAMERA_I2C_BYTE_DATA}, 

	{&regPIP_Capture_OV5648[0],
	ARRAY_SIZE(regPIP_Capture_OV5648), 0, MSM_CAMERA_I2C_BYTE_DATA}, 
	{&regPIP_Capture_OV7695[0],
	ARRAY_SIZE(regPIP_Capture_OV7695), 0, MSM_CAMERA_I2C_BYTE_DATA},  

	{&regPIP_sub_OV5648[0],
	ARRAY_SIZE(regPIP_sub_OV5648), 0, MSM_CAMERA_I2C_BYTE_DATA}, 
	{&regPIP_sub_OV7695[0],
	ARRAY_SIZE(regPIP_sub_OV7695), 0, MSM_CAMERA_I2C_BYTE_DATA},  
};

#endif

