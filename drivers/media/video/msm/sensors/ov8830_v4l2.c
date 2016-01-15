/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
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

#include <linux/module.h>
#include "msm_sensor.h"
#include "msm.h"
#define SENSOR_NAME "ov8830"
#define PLATFORM_DRIVER_NAME "msm_camera_ov8830"
#define ov8830_obj ov8830_##obj

#ifdef CDBG
#undef CDBG
#endif
#ifdef CDBG_HIGH
#undef CDBG_HIGH
#endif

//#define OV8830_DGB

#ifdef OV8830_DGB
#define CDBG(fmt, args...) printk(fmt, ##args)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#define CDBG_HIGH(fmt, args...)  do { } while (0)
#endif

#define OV8830_OTP_FEATURE

#ifdef OV8830_OTP_FEATURE

#ifdef OV8830_DGB
#define TRACE(fmt, args...) printk(fmt, ##args)
#else
#define TRACE(fmt, args...) do { } while (0)
#endif

#define OTP_DATA_ADDR         0x3D00
#define OTP_LOAD_ADDR         0x3D81
#define OTP_BANK_ADDR         0x3D84

#define LENC_START_ADDR       0x5800
#define LENC_REG_SIZE         62

#define OTP_LENC_GROUP_ADDR   0x3D00

#define OTP_WB_GROUP_ADDR     0x3D00
#define OTP_WB_GROUP_SIZE     16

#define GAIN_RH_ADDR          0x3400
#define GAIN_RL_ADDR          0x3401
#define GAIN_GH_ADDR          0x3402
#define GAIN_GL_ADDR          0x3403
#define GAIN_BH_ADDR          0x3404
#define GAIN_BL_ADDR          0x3405

#define GAIN_DEFAULT_VALUE    0x0400 // 1x gain

#define MODULE_ID_SUNNY 0x01
#define MODULE_ID_TRULY  0x02
#define MODULE_ID_OFLIM  0x07

// R/G and B/G of current camera module
unsigned short rg_ratio = 0;
unsigned short bg_ratio = 0;

unsigned char otp_lenc_data[62];

#endif
DEFINE_MUTEX(ov8830_mut);
static struct msm_sensor_ctrl_t ov8830_s_ctrl;

static struct msm_camera_i2c_reg_conf ov8830_start_settings[] = {
	{0x4202, 0x00},
};

static struct msm_camera_i2c_reg_conf ov8830_stop_settings[] = {
	{0x4202, 0x0f},
};

static struct msm_camera_i2c_reg_conf ov8830_groupon_settings[] = {
	{0x3208, 0x00},
};

static struct msm_camera_i2c_reg_conf ov8830_groupoff_settings[] = {
	{0x3208, 0x10},
	{0x3208, 0xA0},
};

static struct msm_camera_i2c_reg_conf ov8830_prev_settings[] = {
	//{0x0100,0x00},// software standby
	{0x3708,0xe6},// sensor control
	{0x3709,0xc3},// sensor control
	{0x3800,0x00},// HS = 8
	{0x3801,0x08},// HS
	{0x3802,0x00},// VS = 8
	{0x3803,0x08},// VS
	{0x3804,0x0c},// HE = 3287
	{0x3805,0xd7},// HE
	{0x3806,0x09},// VE = 2471
	{0x3807,0xa7},// VE
	{0x3808,0x06},// HO = 1632
	{0x3809,0x60},// HO
	{0x380a,0x04},// VO = 1224
	{0x380b,0xc8},// VO
	{0x380c,0x0e},// HTS = 3608
	{0x380d,0x18},// HTS
	{0x380e,0x05},// VTS = 1293
	{0x380f,0x0d},// VTS
	{0x3810,0x00},// H OFFSET = 4
	{0x3811,0x04},// H OFFSET
	{0x3812,0x00},// V OFFSET = 4
	{0x3813,0x04},// V OFFSET
	{0x3814,0x31},// X INC
	{0x3815,0x31},// Y INC
	{0x3820,0x11},//
	{0x3821,0x0f},//
	{0x3a04,0x04},//
	{0x3a05,0xc9},//
	{0x4004,0x02},//
	{0x4005,0x18},//yangzhe add for blc update on 20130309
	{0x404f,0xe0},//yangzhe add for blc update on 20130309
	{0x4512,0x00},// vertical sum
	{0x3011,0x21},// MIPI 2 lane, MIPI enable
	{0x3015,0xc8},// MIPI 2 lane on, select MIPI

	{0x3090,0x03},// pll2_prediv
	{0x3091,0x23},// pll2_multiplier
	{0x3092,0x01},// pll2_divs
	{0x3093,0x00},// pll2_seld5
	{0x3094,0x00},//

	{0x30b3,0x50},// pll1_multiplier
	{0x30b4,0x03},// pll1_prediv
	{0x30b5,0x04},// pll1_op_pix_div
	{0x30b6,0x01},// pll1_op_sys_div
	{0x4837,0x0d},// MIPI global timing
};

static struct msm_camera_i2c_reg_conf ov8830_snap_settings[] = {
	{0x3708,0xe3},
	{0x3709,0x03},
	{0x3800,0x00},
	{0x3801,0x0c},
	{0x3802,0x00},
	{0x3803,0x0c},
	{0x3804,0x0c},
	{0x3805,0xd3},
	{0x3806,0x09},
	{0x3807,0xa3},
	{0x3808,0x0c},
	{0x3809,0xc0},
	{0x380a,0x09},
	{0x380b,0x90},
	{0x380c,0x0e},
	{0x380d,0x18},
	{0x380e,0x09},
	{0x380f,0xb4},
	{0x3810,0x00},
	{0x3811,0x04},
	{0x3812,0x00},
	{0x3813,0x04},
	{0x3814,0x11},
	{0x3815,0x11},
	{0x3820,0x10},
	{0x3821,0x0e},
	{0x3a04,0x09},
	{0x3a05,0xa9},
	{0x4004,0x08},
	{0x4005,0x1a},//yangzhe add for blc update on 20130309
	{0x4512,0x01},
	{0x3011,0x21},
	{0x3015,0xc8},
	{0x3090,0x03},
	{0x3091,0x22},
	{0x3092,0x01},
	{0x3093,0x00},
	{0x3094,0x00},
	{0x30b3,0x50},
	{0x30b4,0x03},
	{0x30b5,0x04},
	{0x30b6,0x01},
	{0x4837,0x0d},
	{0x0100,0x01},

};


static struct msm_camera_i2c_reg_conf ov8830_reset_settings[] = {
	{0x0103, 0x01},
};

static struct msm_camera_i2c_reg_conf ov8830_recommend_settings[] = {
	{0x0102,0x01},
	{0x3001,0x2a},
	{0x3002,0x88},
	{0x3005,0x00},
	{0x301b,0xb4},
	{0x301d,0x02},
	{0x3021,0x00},
	{0x3022,0x00},
	{0x3081,0x02},
	{0x3083,0x01},
	{0x3093,0x00},
	{0x3094,0x00},
	{0x3098,0x03},
	{0x3099,0x1e},
	{0x309a,0x00},
	{0x309b,0x00},
	{0x30a2,0x01},
	{0x30b0,0x05},
	{0x30b2,0x00},
	{0x30b5,0x04},
	{0x30b6,0x01},
	{0x3104,0xa1},
	{0x3106,0x01},
	{0x3503,0x07},
	{0x3504,0x00},
	{0x3505,0x30},
	{0x3506,0x00},
	{0x3508,0x80},
	{0x3509,0x10},
	{0x350a,0x00},
	{0x350b,0x38},
	{0x3600,0x78},
	{0x3601,0x0a},
	{0x3602,0x9c},
	{0x3612,0x00},
	{0x3604,0x38},
	{0x3620,0x64},
	{0x3621,0xb5},
	{0x3622,0x03},
	{0x3625,0x64},
	{0x3630,0x55},
	{0x3631,0xd2},
	{0x3632,0x00},
	{0x3633,0x34},
	{0x3634,0x03},
	{0x364d,0x00},
	{0x364f,0x00},
	{0x3660,0x80},
	{0x3662,0x10},
	{0x3665,0x00},
	{0x3666,0x00},
	{0x3667,0x00},
	{0x366a,0x80},
	{0x366c,0x00},
	{0x366d,0x00},
	{0x366e,0x00},
	{0x366f,0x20},
	{0x3680,0xe0},
	{0x3681,0x00},
	{0x3701,0x14},
	{0x3702,0xbf},
	{0x3703,0x8c},
	{0x3704,0x78},
	{0x3705,0x02},
	{0x370a,0x00},
	{0x370b,0x20},
	{0x370c,0x0c},
	{0x370d,0x11},
	{0x370e,0x00},
	{0x370f,0x00},
	{0x3710,0x00},
	{0x371c,0x01},
	{0x371f,0x0c},
	{0x3721,0x00},
	{0x3724,0x10},
	{0x3726,0x00},
	{0x372a,0x01},
	{0x3730,0x18},
	{0x3738,0x22},
	{0x3739,0x08},
	{0x373a,0x51},
	{0x373b,0x02},
	{0x373c,0x20},
	{0x373f,0x02},
	{0x3740,0x42},
	{0x3741,0x02},
	{0x3742,0x18},
	{0x3743,0x01},
	{0x3744,0x02},
	{0x3747,0x10},
	{0x374c,0x04},
	{0x3751,0xf0},
	{0x3752,0x00},
	{0x3753,0x00},
	{0x3754,0xc0},
	{0x3755,0x00},
	{0x3756,0x1a},
	{0x3758,0x00},
	{0x3759,0x0f},
	{0x375c,0x04},
	{0x3767,0x01},
	{0x376b,0x44},
	{0x3774,0x10},
	{0x3776,0x00},
	{0x377f,0x08},
	{0x3780,0x22},
	{0x3781,0x0c},
	{0x3784,0x2c},
	{0x3785,0x1e},
	{0x3786,0x16},
	{0x378f,0xf5},
	{0x3791,0xb0},
	{0x3795,0x00},
	{0x3796,0x64},
	{0x3797,0x11},
	{0x3798,0x30},
	{0x3799,0x41},
	{0x379a,0x07},
	{0x379b,0xb0},
	{0x379c,0x0c},
	{0x37c5,0x00},
	{0x37c6,0xa0},
	{0x37c7,0x00},
	{0x37c9,0x00},
	{0x37ca,0x00},
	{0x37cb,0x00},
	{0x37cc,0x00},
	{0x37cd,0x00},
	{0x37ce,0x01},
	{0x37cf,0x00},
	{0x37d1,0x01},
	{0x37de,0x00},
	{0x37df,0x00},
	{0x3823,0x00},
	{0x3824,0x00},
	{0x3825,0x00},
	{0x3826,0x00},
	{0x3827,0x00},
	{0x382a,0x04},
	{0x3a06,0x00},
	{0x3a07,0xf8},
	{0x3b00,0x00},
	{0x3b02,0x00},
	{0x3b03,0x00},
	{0x3b04,0x00},
	{0x3b05,0x00},
	{0x3d00,0x00},
	{0x3d01,0x00},
	{0x3d02,0x00},
	{0x3d03,0x00},
	{0x3d04,0x00},
	{0x3d05,0x00},
	{0x3d06,0x00},
	{0x3d07,0x00},
	{0x3d08,0x00},
	{0x3d09,0x00},
	{0x3d0a,0x00},
	{0x3d0b,0x00},
	{0x3d0c,0x00},
	{0x3d0d,0x00},
	{0x3d0e,0x00},
	{0x3d0f,0x00},
	{0x3d80,0x00},
	{0x3d81,0x00},
	{0x3d84,0x00},
	{0x4000,0x18},
	{0x4001,0x04},
	{0x4002,0x45},
	{0x4004,0x02},
	{0x4005,0x18},
	{0x4006,0x20},
	{0x4008,0x20},
	{0x4009,0x10},
	{0x404f,0x90},
	{0x4100,0x10},
	{0x4101,0x12},
	{0x4102,0x24},
	{0x4103,0x00},
	{0x4104,0x5b},
	{0x4307,0x30},
	{0x4315,0x00},
	{0x4511,0x05},
	{0x4805,0x21},
	{0x4806,0x00},
	{0x481f,0x36},
	{0x4831,0x6c},
	{0x4a00,0xaa},
	{0x4a03,0x01},
	{0x4a05,0x08},
	{0x4a0a,0x88},
	{0x4d03,0xbb},
	{0x5000,0x86},
	{0x5001,0x01},
	{0x5002,0x80},
	{0x5003,0x20},
	{0x5013,0x00},
	{0x5046,0x4a},
	{0x5780,0x1c},
	{0x5786,0x20},
	{0x5787,0x10},
	{0x5788,0x18},
	{0x578a,0x04},
	{0x578b,0x02},
	{0x578c,0x02},
	{0x578e,0x06},
	{0x578f,0x02},
	{0x5790,0x02},
	{0x5791,0xff},
	{0x5a08,0x02},
	{0x5e00,0x00},
	{0x5e10,0x0c},
	{0x5000,0x86},
	{0x5001,0x01},
	{0x3400,0x04},
	{0x3401,0x00},
	{0x3402,0x04},
	{0x3403,0x00},
	{0x3404,0x04},
	{0x3405,0x00},
	{0x3406,0x01},
	{0x4000,0x18},
	{0x4002,0x45},
	{0x4005,0x18},
	{0x4009,0x10},
	{0x3503,0x07},
	{0x3500,0x00},
	{0x3501,0x29},
	{0x3502,0x00},
	{0x350b,0x78},
	{0x30b3,0x50},
	{0x30b4,0x03},
	{0x30b5,0x04},
	{0x30b6,0x01},
	{0x4837,0x0d},

};
static struct msm_camera_i2c_reg_conf ov8830_video_60fps_settings[] = {
	{0x3708,0xe6},// sensor control
	{0x3709,0xc3},// sensor control
	{0x3800,0x00},// HS = 40
	{0x3801,0x28},// HS
	{0x3802,0x01},// VS = 332
	{0x3803,0x4c},// VS
	{0x3804,0x0c},// HE = 3255
	{0x3805,0xb7},// HE
	{0x3806,0x08},// VE = 2147
	{0x3807,0x63},// VE
	{0x3808,0x05},// HO = 1280
	{0x3809,0x00},// HO
	{0x380a,0x02},// VO = 720
	{0x380b,0xd0},// VO
	{0x380c,0x0e},// HTS = 3608
	{0x380d,0x18},// HTS
	{0x380e,0x03},// VTS = 984
	{0x380f,0xd8},// VTS
	{0x3810,0x00},// H OFFSET = 4
	{0x3811,0x04},// H OFFSET
	{0x3812,0x00},// V OFFSET = 4
	{0x3813,0x04},// V OFFSET
	{0x3814,0x31},// X INC
	{0x3815,0x31},// Y INC
	{0x3820,0x11},//
	{0x3821,0x0f},//
	{0x3a04,0x03},//
	{0x3a05,0x99},//
	{0x4004,0x02},//
	{0x4512,0x00},// vertical sum
	{0x3011,0x21},// MIPI 2 lane, MIPI enable
	{0x3015,0xc8},// MIPI 2 lane on, select MIPI
								//SCLK = 108Mhz
	{0x3090,0x02},// pll2_prediv
	{0x3091,0x12},// pll2_multiplier
	{0x3092,0x01},// pll2_divs
	{0x3093,0x00},// pll2_seld5
	{0x3094,0x00},//
								//PI data rate = 640Mbps
	{0x30b3,0x50},// pll1_multiplier
	{0x30b4,0x03},// pll1_prediv
	{0x30b5,0x04},// pll1_op_pix_div
	{0x30b6,0x01},// pll1_op_sys_div
	{0x4837,0x0d},// MIPI global timing

};

static struct msm_camera_i2c_reg_conf ov8830_video_90fps_settings[] = {
	{0x3708,0xe9},// sensor control
	{0x3709,0xc3},// sensor control
	{0x3800,0x00},// HS = 32
	{0x3801,0x20},// HS
	{0x3802,0x00},// VS = 24
	{0x3803,0x18},// VS
	{0x3804,0x0c},// HE = 3263
	{0x3805,0xbf},// HE
	{0x3806,0x09},// VE = 2455
	{0x3807,0x97},// VE
	{0x3808,0x02},// HO = 640
	{0x3809,0x80},// HO
	{0x380a,0x01},// VO = 480
	{0x380b,0xe0},// VO
	{0x380c,0x0e},// HTS = 3608
	{0x380d,0x18},// HTS
	{0x380e,0x02},// VTS = 664
	{0x380f,0x98},// VTS
	{0x3810,0x00},// H OFFSET = 4
	{0x3811,0x04},// H OFFSET
	{0x3812,0x00},// V OFFSET = 4
	{0x3813,0x04},// V OFFSET
	{0x3814,0x71},// X INC
	{0x3815,0x35},// Y INC
	{0x3820,0x14},//
	{0x3821,0x0f},//
	{0x3a04,0x02},//
	{0x3a05,0x69},//
	{0x4004,0x02},//
	{0x4512,0x01},// vertical average
	{0x3011,0x21},// MIPI 2 lane, MIPI enable
	{0x3015,0xc8},// MIPI 2 lane on, select MIPI
	//SCLK = 216Mhz
	{0x3090,0x02},// pll2_prediv
	{0x3091,0x12},// pll2_multiplier
	{0x3092,0x00},// pll2_divs
	{0x3093,0x00},// pll2_seld5
	{0x3094,0x00},//MIPI data rate = 640Mbps
	{0x30b3,0x50},// pll1_multiplier
	{0x30b4,0x03},// pll1_prediv
	{0x30b5,0x04},// pll1_op_pix_div
	{0x30b6,0x01},// pll1_op_sys_div
	{0x4837,0x0d},// MIPI global timing
};
static struct msm_camera_i2c_reg_conf ov8830_zsl_settings[] = {
	{0x3708,0xe3},
	{0x3709,0x03},
	{0x3800,0x00},
	{0x3801,0x0c},
	{0x3802,0x00},
	{0x3803,0x0c},
	{0x3804,0x0c},
	{0x3805,0xd3},
	{0x3806,0x09},
	{0x3807,0xa3},
	{0x3808,0x0c},
	{0x3809,0xc0},
	{0x380a,0x09},
	{0x380b,0x90},
	{0x380c,0x0e},
	{0x380d,0x18},
	{0x380e,0x09},
	{0x380f,0xb4},
	{0x3810,0x00},
	{0x3811,0x04},
	{0x3812,0x00},
	{0x3813,0x04},
	{0x3814,0x11},
	{0x3815,0x11},
	{0x3820,0x10},
	{0x3821,0x0e},
	{0x3a04,0x09},
	{0x3a05,0xa9},
	{0x4004,0x08},
	{0x4512,0x01},
	{0x3011,0x21},
	{0x3015,0xc8},
	{0x3090,0x03},
	{0x3091,0x22},
	{0x3092,0x01},
	{0x3093,0x00},
	{0x3094,0x00},
	{0x30b3,0x50},
	{0x30b4,0x03},
	{0x30b5,0x04},
	{0x30b6,0x01},
	{0x4837,0x0d},
//	{0x0100,0x01},

};

static struct v4l2_subdev_info ov8830_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};

static struct msm_camera_i2c_conf_array ov8830_init_conf[] = {
	{&ov8830_reset_settings[0],
	ARRAY_SIZE(ov8830_reset_settings), 50, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8830_recommend_settings[0],
	ARRAY_SIZE(ov8830_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array ov8830_confs[] = {
	{&ov8830_snap_settings[0],
	ARRAY_SIZE(ov8830_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8830_prev_settings[0],
	ARRAY_SIZE(ov8830_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8830_video_60fps_settings[0],
	ARRAY_SIZE(ov8830_video_60fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8830_video_90fps_settings[0],
	ARRAY_SIZE(ov8830_video_90fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov8830_zsl_settings[0],
	ARRAY_SIZE(ov8830_zsl_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t ov8830_dimensions[] = {
	{
		.x_output = 0xCC0,
		.y_output = 0x990,
		.line_length_pclk = 0xE18,
		.frame_length_lines = 0x9B4,
		.vt_pixel_clk = 0xE18*0x9B4*15,
		.op_pixel_clk = 266667000,
		.binning_factor = 1,
	},
	{
		.x_output = 0x660,
		.y_output = 0x4c8,
		.line_length_pclk = 0xe18,
		.frame_length_lines = 0x50d,
		.vt_pixel_clk = 0xe18*0x50d*30,
		.op_pixel_clk = 266667000,
		.binning_factor = 2,
	},
	{
		.x_output = 0x500,
		.y_output = 0x2d0,
		.line_length_pclk = 0xe18,
		.frame_length_lines = 0x3d8,
		.vt_pixel_clk = 0xe18*0x3d8*60,
		.op_pixel_clk = 266667000,
		.binning_factor = 2,
	},
	{
		.x_output = 0x280,
		.y_output = 0x1e0,
		.line_length_pclk = 0xe18,
		.frame_length_lines = 0x298,
		.vt_pixel_clk = 0xe18*0x298*90,
		.op_pixel_clk = 266667000,
		.binning_factor = 2,
	},
	{
		.x_output = 0xCC0,
		.y_output = 0x990,
		.line_length_pclk = 0xE18,
		.frame_length_lines = 0x9B4,
		.vt_pixel_clk = 0xE18*0x9B4*15,
		.op_pixel_clk = 266667000,
		.binning_factor = 1,
	},
};

static struct msm_camera_csi_params ov8830_csi_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 14,
};

static struct msm_camera_csi_params *ov8830_csi_params_array[] = {
	&ov8830_csi_params, /* Snapshot */
	&ov8830_csi_params, /* Preview */
	&ov8830_csi_params, /* 60fps */
	&ov8830_csi_params, /* 90fps */
	&ov8830_csi_params, /* ZSL */
};

static struct msm_sensor_output_reg_addr_t ov8830_reg_addr = {
	.x_output = 0x3808,
	.y_output = 0x380a,
	.line_length_pclk = 0x380c,
	.frame_length_lines = 0x380e,
};

static struct msm_sensor_id_info_t ov8830_id_info = {
	.sensor_id_reg_addr = 0x300A,
	.sensor_id = 0x8830,
};

static struct msm_sensor_exp_gain_info_t ov8830_exp_gain_info = {
	.coarse_int_time_addr = 0x3500, //0x3501 wukai modif at 20130118
	.global_gain_addr = 0x350A,
	.vert_offset = 6,
};

static int is_first_preview = 1;

static int32_t ov8830_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint32_t fl_lines, offset;
	uint8_t int_time[3];

	fl_lines =
		(s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;
        fl_lines += (fl_lines & 0x1); //wukai modif at 20130118
	CDBG("%s : %d %d %d\n",__func__, fl_lines, gain, line);
	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);
	int_time[0] = line >> 12;
	int_time[1] = line >> 4;
	int_time[2] = line << 4;
	msm_camera_i2c_write_seq(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,//wukai modif at 20130118
		&int_time[0], 3);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
		MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	return 0;
}

static int32_t ov8830_write_priv_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	uint32_t fl_lines, offset;
	uint8_t int_time[3];

	fl_lines =
		(s_ctrl->curr_frame_length_lines * s_ctrl->fps_divider) / Q10;
	offset = s_ctrl->sensor_exp_gain_info->vert_offset;
	if (line > (fl_lines - offset))
		fl_lines = line + offset;
	fl_lines += (fl_lines & 0x1); //wukai modif at 20130118
	CDBG("%s : %d %d %d\n",__func__, fl_lines, gain, line);
	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines, fl_lines,
		MSM_CAMERA_I2C_WORD_DATA);
	int_time[0] = line >> 12;
	int_time[1] = line >> 4;
	int_time[2] = line << 4;
	msm_camera_i2c_write_seq(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,//wukai modif at 20130118
		&int_time[0], 3);
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr, gain,
		MSM_CAMERA_I2C_WORD_DATA);
	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	if(!is_first_preview)
	{
		msleep(300);
		is_first_preview = 1;
	}
	return 0;
}
#ifdef OV8830_OTP_FEATURE
static struct msm_sensor_ctrl_t *otp_s_ctrl = NULL;

int32_t OV8830_write_cmos_sensor(uint16_t addr, uint16_t data)
{
	return msm_camera_i2c_write(otp_s_ctrl->sensor_i2c_client, addr, data,
		MSM_CAMERA_I2C_BYTE_DATA);
}


uint16_t OV8830_read_cmos_sensor(uint16_t addr)
{
    uint16_t temp;
	msm_camera_i2c_read(otp_s_ctrl->sensor_i2c_client, addr, &temp,
		MSM_CAMERA_I2C_BYTE_DATA);
    return temp;
}



void otp_read_enable(void)
{
	OV8830_write_cmos_sensor(OTP_LOAD_ADDR, 0x01);
	msleep(10); // sleep > 10ms
}

// Disable OTP read function
void otp_read_disable(void)
{
	OV8830_write_cmos_sensor(OTP_LOAD_ADDR, 0x00);
	//msleep(15); // sleep > 10ms
}

void otp_read(unsigned short otp_addr, unsigned char* otp_data)
{
	otp_read_enable();
	*otp_data = OV8830_read_cmos_sensor(otp_addr);
	otp_read_disable();
}

/*******************************************************************************
* Function    :  otp_clear
* Description :  Clear OTP buffer 
* Parameters  :  none
* Return      :  none
*******************************************************************************/	
void otp_clear(void)
{
	// After read/write operation, the OTP buffer should be cleared to avoid accident write
	unsigned char i;
	for (i=0; i<16; i++) 
	{
		OV8830_write_cmos_sensor(OTP_DATA_ADDR+i, 0x00);
	}
}

/*******************************************************************************
* Function    :  otp_check_wb_group
* Description :  Check OTP Space Availability
* Parameters  :  [in] index : index of otp group (0, 1, 2)
* Return      :  0, group index is empty
                 1, group index has invalid data
                 2, group index has valid data
                -1, group index error
*******************************************************************************/	
signed char otp_check_wb_group(unsigned char index)
{   
	unsigned short otp_addr = OTP_WB_GROUP_ADDR;
	unsigned char  flag;

    if (index > 2)
	{
		TRACE("OTP input wb group index %d error\n", index);
		return -1;
	}
		
	// select bank 1-3
	OV8830_write_cmos_sensor(OTP_BANK_ADDR, 0xc0 | (index+1));

	otp_read(otp_addr, &flag);
	otp_clear();

	// Check all bytes of a group. If all bytes are '0', then the group is empty. 
	// Check from group 1 to group 2, then group 3.
	
	flag &= 0xc0;
	if (!flag)
	{
		TRACE("wb group %d is empty\n", index);
		return 0;
	}
	else if (flag == 0x40)
	{
		TRACE("wb group %d has valid data\n", index);
		return 2;
	}
	else
	{
		TRACE("wb group %d has invalid data\n", index);
		return 1;
	}
}

/*******************************************************************************
* Function    :  otp_read_wb_group
* Description :  Read group value and store it in OTP Struct 
* Parameters  :  [in] index : index of otp group (0, 1, 2)
* Return      :  group index (0, 1, 2)
                 -1, error
*******************************************************************************/	
signed char otp_read_wb_group(signed char index)
{
	unsigned short otp_addr;
	unsigned char  mid, AWB_light_LSB, rg_ratio_MSB, bg_ratio_MSB;

	if (index == -1)
	{
		// Check first OTP with valid data
		for (index=0; index<3; index++)
		{
			if (otp_check_wb_group(index) == 2)
			{
				TRACE("read wb from group %d\n", index);
				break;
			}
		}

		if (index > 2)
		{
			TRACE("no group has valid data\n");
			return -1;
		}
	}
	else
	{
		if (otp_check_wb_group(index) != 2)
		{
			TRACE("read wb from group %d failed\n", index);
			return -1;
		}
	}

	otp_addr = OTP_WB_GROUP_ADDR;

	// select bank 1-3
	OV8830_write_cmos_sensor(OTP_BANK_ADDR, 0xc0 | (index+1));

	otp_read(otp_addr+1, &mid);
	if ((mid != MODULE_ID_SUNNY)&&(mid != MODULE_ID_TRULY)&&(mid != MODULE_ID_OFLIM))
	{
		printk("module id err , mid = 0x%x\n",mid);
		return -1;
	}

	otp_read(otp_addr+6, &rg_ratio_MSB);
	otp_read(otp_addr+7, &bg_ratio_MSB);
	otp_read(otp_addr+10, &AWB_light_LSB);	
	otp_clear();
	rg_ratio = (rg_ratio_MSB<<2) | ((AWB_light_LSB & 0xC0)>>6);
	bg_ratio = (bg_ratio_MSB<<2) | ((AWB_light_LSB & 0x30)>>4);
	TRACE("read wb finished  rg_ratio = 0x%x bg_ratio = 0x%x\n",rg_ratio,bg_ratio);
	return index;
}

#ifdef SUPPORT_FLOATING //Use this if support floating point values
/*******************************************************************************
* Function    :  otp_apply_wb
* Description :  Calcualte and apply R, G, B gain to module
* Parameters  :  [in] golden_rg : R/G of golden camera module
                 [in] golden_bg : B/G of golden camera module
* Return      :  1, success; 0, fail
*******************************************************************************/	
bool otp_apply_wb(unsigned short golden_rg, unsigned short golden_bg)
{
	unsigned short gain_r = GAIN_DEFAULT_VALUE;
	unsigned short gain_g = GAIN_DEFAULT_VALUE;
	unsigned short gain_b = GAIN_DEFAULT_VALUE;

	double ratio_r, ratio_g, ratio_b;
	double cmp_rg, cmp_bg;

	if (!golden_rg || !golden_bg)
	{
		TRACE("golden_rg / golden_bg can not be zero\n");
		return 0;
	}

	// Calcualte R, G, B gain of current module from R/G, B/G of golden module
        // and R/G, B/G of current module
	cmp_rg = 1.0 * rg_ratio / golden_rg;
	cmp_bg = 1.0 * bg_ratio / golden_bg;

	if ((cmp_rg<1) && (cmp_bg<1))
	{
		// R/G < R/G golden, B/G < B/G golden
		ratio_g = 1;
		ratio_r = 1 / cmp_rg;
		ratio_b = 1 / cmp_bg;
	}
	else if (cmp_rg > cmp_bg)
	{
		// R/G >= R/G golden, B/G < B/G golden
		// R/G >= R/G golden, B/G >= B/G golden
		ratio_r = 1;
		ratio_g = cmp_rg;
		ratio_b = cmp_rg / cmp_bg;
	}
	else
	{
		// B/G >= B/G golden, R/G < R/G golden
		// B/G >= B/G golden, R/G >= R/G golden
		ratio_b = 1;
		ratio_g = cmp_bg;
		ratio_r = cmp_bg / cmp_rg;
	}

	// write sensor wb gain to registers
	// 0x0400 = 1x gain
	if (ratio_r != 1)
	{
		gain_r = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_r);
		OV8830_write_cmos_sensor(GAIN_RH_ADDR, gain_r >> 8);
		OV8830_write_cmos_sensor(GAIN_RL_ADDR, gain_r & 0x00ff);
	}

	if (ratio_g != 1)
	{
		gain_g = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_g);
		OV8830_write_cmos_sensor(GAIN_GH_ADDR, gain_g >> 8);
		OV8830_write_cmos_sensor(GAIN_GL_ADDR, gain_g & 0x00ff);
	}

	if (ratio_b != 1)
	{
		gain_b = (unsigned short)(GAIN_DEFAULT_VALUE * ratio_b);
		OV8830_write_cmos_sensor(GAIN_BH_ADDR, gain_b >> 8);
		OV8830_write_cmos_sensor(GAIN_BL_ADDR, gain_b & 0x00ff);
	}

	TRACE("cmp_rg=%f, cmp_bg=%f\n", cmp_rg, cmp_bg);
	TRACE("ratio_r=%f, ratio_g=%f, ratio_b=%f\n", ratio_r, ratio_g, ratio_b);
	TRACE("gain_r=0x%x, gain_g=0x%x, gain_b=0x%x\n", gain_r, gain_g, gain_b);
	return 1;
}

#else //Use this if not support floating point values

#define OTP_MULTIPLE_FAC	10000
bool otp_apply_wb(unsigned short golden_rg, unsigned short golden_bg)
{
	unsigned short gain_r = GAIN_DEFAULT_VALUE;
	unsigned short gain_g = GAIN_DEFAULT_VALUE;
	unsigned short gain_b = GAIN_DEFAULT_VALUE;

	unsigned short ratio_r, ratio_g, ratio_b;
	unsigned short cmp_rg, cmp_bg;

	if (!golden_rg || !golden_bg)
	{
		TRACE("golden_rg / golden_bg can not be zero\n");
		return 0;
	}

	// Calcualte R, G, B gain of current module from R/G, B/G of golden module
    // and R/G, B/G of current module
	cmp_rg = OTP_MULTIPLE_FAC * rg_ratio / golden_rg;
	cmp_bg = OTP_MULTIPLE_FAC * bg_ratio / golden_bg;

	if ((cmp_rg < 1 * OTP_MULTIPLE_FAC) && (cmp_bg < 1 * OTP_MULTIPLE_FAC))
	{
		// R/G < R/G golden, B/G < B/G golden
		ratio_g = 1 * OTP_MULTIPLE_FAC;
		ratio_r = 1 * OTP_MULTIPLE_FAC * OTP_MULTIPLE_FAC / cmp_rg;
		ratio_b = 1 * OTP_MULTIPLE_FAC * OTP_MULTIPLE_FAC / cmp_bg;
	}
	else if (cmp_rg > cmp_bg)
	{
		// R/G >= R/G golden, B/G < B/G golden
		// R/G >= R/G golden, B/G >= B/G golden
		ratio_r = 1 * OTP_MULTIPLE_FAC;
		ratio_g = cmp_rg;
		ratio_b = OTP_MULTIPLE_FAC * cmp_rg / cmp_bg;
	}
	else
	{
		// B/G >= B/G golden, R/G < R/G golden
		// B/G >= B/G golden, R/G >= R/G golden
		ratio_b = 1 * OTP_MULTIPLE_FAC;
		ratio_g = cmp_bg;
		ratio_r = OTP_MULTIPLE_FAC * cmp_bg / cmp_rg;
	}

	// write sensor wb gain to registers
	// 0x0400 = 1x gain
	if (ratio_r != 1 * OTP_MULTIPLE_FAC)
	{
		gain_r = GAIN_DEFAULT_VALUE * ratio_r / OTP_MULTIPLE_FAC;
		OV8830_write_cmos_sensor(GAIN_RH_ADDR, gain_r >> 8);
		OV8830_write_cmos_sensor(GAIN_RL_ADDR, gain_r & 0x00ff);
	}

	if (ratio_g != 1 * OTP_MULTIPLE_FAC)
	{
		gain_g = GAIN_DEFAULT_VALUE * ratio_g / OTP_MULTIPLE_FAC;
		OV8830_write_cmos_sensor(GAIN_GH_ADDR, gain_g >> 8);
		OV8830_write_cmos_sensor(GAIN_GL_ADDR, gain_g & 0x00ff);
	}

	if (ratio_b != 1 * OTP_MULTIPLE_FAC)
	{
		gain_b = GAIN_DEFAULT_VALUE * ratio_b / OTP_MULTIPLE_FAC;
		OV8830_write_cmos_sensor(GAIN_BH_ADDR, gain_b >> 8);
		OV8830_write_cmos_sensor(GAIN_BL_ADDR, gain_b & 0x00ff);
	}

	TRACE("cmp_rg=%d, cmp_bg=%d\n", cmp_rg, cmp_bg);
	TRACE("ratio_r=%d, ratio_g=%d, ratio_b=%d\n", ratio_r, ratio_g, ratio_b);
	TRACE("gain_r=0x%x, gain_g=0x%x, gain_b=0x%x\n", gain_r, gain_g, gain_b);
	return 1;
}
#endif /* SUPPORT_FLOATING */

/*******************************************************************************
* Function    :  otp_update_wb
* Description :  Update white balance settings from OTP
* Parameters  :  [in] golden_rg : R/G of golden camera module
                 [in] golden_bg : B/G of golden camera module
* Return      :  1, success; 0, fail
*******************************************************************************/	
bool otp_update_wb(unsigned short golden_rg, unsigned short golden_bg) 
{
	TRACE("start wb update golden_rg = 0x%x  golden_bg = 0x%x \n",golden_rg,golden_bg);

	if (otp_read_wb_group(-1) != -1)
	{
		if (otp_apply_wb(golden_rg, golden_bg) == 1)
		{
			TRACE("wb update finished\n");
			return 1;
		}
	}

	TRACE("wb update failed\n");
	return 0;
}

/*******************************************************************************
* Function    :  otp_check_lenc_group
* Description :  Check OTP Space Availability
* Parameters  :  [in] BYTE index : index of otp group (0, 1, 2)
* Return      :  0, group index is empty
                 1, group index has invalid data
                 2, group index has valid data
                -1, group index error
*******************************************************************************/	
signed char otp_check_lenc_group(unsigned char index)
{   
	unsigned short otp_addr = OTP_LENC_GROUP_ADDR;
	unsigned char  flag;
	unsigned char  bank;

    if (index > 2)
	{
		TRACE("OTP input lenc group index %d error\n", index);
		return -1;
	}
		
	// select bank: index*4 + 4
	bank = 0xc0 | (index*4 + 4);
	OV8830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_read(otp_addr, &flag);
	otp_clear();

	flag &= 0xc0;

	// Check all bytes of a group. If all bytes are '0', then the group is empty. 
	// Check from group 1 to group 2, then group 3.
	if (!flag)
	{
		TRACE("lenc group %d is empty\n", index);
		return 0;
	}
	else if (flag == 0x40)
	{
		TRACE("lenc group %d has valid data\n", index);
		return 2;
	}
	else
	{
		TRACE("lenc group %d has invalid data\n", index);
		return 1;
	}
}

/*******************************************************************************
* Function    :  otp_read_lenc_group
* Description :  Read group value and store it in OTP Struct 
* Parameters  :  [in] int index : index of otp group (0, 1, 2)
* Return      :  group index (0, 1, 2)
                 -1, error
*******************************************************************************/	
signed char otp_read_lenc_group(int index)
{
	unsigned short otp_addr;
	unsigned char  bank;
	unsigned char  i;

	if (index == -1)
	{
		// Check first OTP with valid data
		for (index=0; index<3; index++)
		{
			if (otp_check_lenc_group(index) == 2)
			{
				TRACE("read lenc from group %d\n", index);
				break;
			}
		}

		if (index > 2)
		{
			TRACE("no group has valid data\n");
			return -1;
		}
	}
	else
	{
		if (otp_check_lenc_group(index) != 2) 
		{
			TRACE("read lenc from group %d failed\n", index);
			return -1;
		}
	}

	// select bank: index*4 + 4
	bank = 0xc0 | (index*4 + 4);
	OV8830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR+1;

	otp_read_enable();
	for (i=0; i<15; i++) 
	{
		otp_lenc_data[i] = OV8830_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();

	// select next bank
	bank++;
	OV8830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR;

	otp_read_enable();
	for (i=15; i<31; i++) 
	{
		otp_lenc_data[i] = OV8830_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();
	
	// select next bank
	bank++;
	OV8830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR;

	otp_read_enable();
	for (i=31; i<47; i++) 
	{
		otp_lenc_data[i] = OV8830_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();
	
	// select next bank
	bank++;
	OV8830_write_cmos_sensor(OTP_BANK_ADDR, bank);

	otp_addr = OTP_LENC_GROUP_ADDR;

	otp_read_enable();
	for (i=47; i<62; i++) 
	{
		otp_lenc_data[i] = OV8830_read_cmos_sensor(otp_addr);
		otp_addr++;
	}
	otp_read_disable();
	otp_clear();
	
	TRACE("read lenc finished\n");
	return index;
}

/*******************************************************************************
* Function    :  otp_apply_lenc
* Description :  Apply lens correction setting to module
* Parameters  :  none
* Return      :  none
*******************************************************************************/	
void otp_apply_lenc(void)
{
	// write lens correction setting to registers
	unsigned char i;
	TRACE("apply lenc setting\n");


	for (i=0; i<LENC_REG_SIZE; i++)
	{
		OV8830_write_cmos_sensor(LENC_START_ADDR+i, otp_lenc_data[i]);
		TRACE("0x%x, 0x%x\n", LENC_START_ADDR+i, otp_lenc_data[i]);
	}
}

/*******************************************************************************
* Function    :  otp_update_lenc
* Description :  Get lens correction setting from otp, then apply to module
* Parameters  :  none
* Return      :  1, success; 0, fail
*******************************************************************************/	
bool otp_update_lenc(void) 
{
	TRACE("start lenc update\n");

	if (otp_read_lenc_group(-1) != -1)
	{
		otp_apply_lenc();
		TRACE("lenc update finished\n");
		return 1;
	}

	TRACE("lenc update failed\n");
	return 0;
}

/*******************************************************************************
* Function    :  otp_update_BLC
* Description :  Get BLC value from otp, then apply to module
* Parameters  :  none
* Return      :  none
*******************************************************************************/
void otp_update_BLC(void) 
{
	unsigned char  regh;
	//select bank 31
	OV8830_write_cmos_sensor(0x3d84, 0xdf);
	otp_read(0x3d0b, &regh);
	if(regh == 0)
	{
   		OV8830_write_cmos_sensor(0x4000, 0x18);
   		msleep(20); 
   		otp_read(0x3d0a, &regh);
   		msleep(20); 
   		OV8830_write_cmos_sensor(0x4006, regh);
   		TRACE("BLC value from 0x3d0a\n");
	}
	else
	{
		OV8830_write_cmos_sensor(0x4000, 0x10);
		TRACE("BLC value from 0x3d0b\n");
	}
	OV8830_write_cmos_sensor(0x4009, 0x10);
	msleep(20); 
   	TRACE("BLC update finished\n");
}

#endif

static const struct i2c_device_id ov8830_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov8830_s_ctrl},
	{ }
};
#ifdef OV8830_OTP_FEATURE
int32_t camera_module_id= MODULE_ID_SUNNY;
static unsigned char is_probed=0;
enum msm_camera_actuator_name get_actutor_id(int32_t md_id)
{
	enum msm_camera_actuator_name m_id = 0;
switch(md_id)
{
case MODULE_ID_SUNNY:
	m_id = MSM_ACTUATOR_MAIN_CAM_100;
	printk("%s  module_id is sunny \n",__func__);
	break;
case MODULE_ID_TRULY:
	m_id = MSM_ACTUATOR_MAIN_CAM_101;
	printk("%s  module_id is truly \n",__func__);
	break;
case MODULE_ID_OFLIM:
	m_id = MSM_ACTUATOR_MAIN_CAM_101;
	printk("%s  module_id is oflim \n",__func__);
	break;
default:
	m_id = MSM_ACTUATOR_MAIN_CAM_100;
	printk("%s  module_id is unkonwn mid = 0x%x\n",__func__,m_id );
	break;
}
// sunny:MSM_ACTUATOR_MAIN_CAM_100
// truly:MSM_ACTUATOR_MAIN_CAM_101
	return m_id;
};
int32_t ov8830_get_module_id(struct msm_sensor_ctrl_t *s_ctrl,
	int32_t * module_id){

	CDBG("%s IN, module id is %d\r\n", __func__, camera_module_id);
	*module_id = camera_module_id;
	CDBG("%s, X\n",__func__);
	return 0;	
}

#endif
int32_t ov8830_sensor_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int32_t rc = 0;
	//struct msm_sensor_ctrl_t *s_ctrl;
#ifdef OV8830_OTP_FEATURE
	struct msm_camera_sensor_info *info = client->dev.platform_data;
#endif
	CDBG("\n in ov8830_sensor_i2c_probe\n");
	rc = msm_sensor_i2c_probe(client, id);
	if (client->dev.platform_data == NULL) {
		pr_err("%s: NULL sensor data\n", __func__);
		return -EFAULT;
	}
#ifdef OV8830_OTP_FEATURE
	info->actuator_info->cam_name = get_actutor_id(camera_module_id);
	is_probed = 1;//wang_gj add for get module id
#endif
	return rc;
}

int32_t ov8830_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *info = s_ctrl->sensordata;
	CDBG("%s IN\r\n", __func__);

	CDBG("%s, sensor_pwd:%d, sensor_reset:%d\r\n",__func__, info->sensor_pwd, info->sensor_reset);

	gpio_direction_output(info->sensor_pwd, 0);
	gpio_direction_output(info->sensor_reset, 1);
	usleep_range(5000, 6000);

//	if (info->pmic_gpio_enable) {
//		lcd_camera_power_onoff(1);
//	}

	rc = msm_sensor_power_up(s_ctrl);
	if (rc < 0) {
		CDBG("%s: msm_sensor_power_up failed\n", __func__);
		return rc;
	}

	/* turn on ldo and vreg */
	usleep_range(1000, 1100);
	gpio_direction_output(info->sensor_pwd, 1);
	msleep(10);
	gpio_direction_output(info->sensor_reset, 1);
	msleep(25);
#ifdef OV8830_OTP_FEATURE
	if(0==is_probed)
		{
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(30);

			TRACE("wang_gj Update OTP\n");
			otp_s_ctrl=s_ctrl;
			msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x100, 0x1,
					MSM_CAMERA_I2C_BYTE_DATA);
			msleep(66);
			OV8830_write_cmos_sensor(OTP_BANK_ADDR, 0xc0 | (1));
			otp_read(0x3d01, (unsigned char*)(&camera_module_id));// read module id reg
			otp_clear();
			TRACE("wang_gj module_id = %d \n",camera_module_id);
			//ov8830_update_otp(s_ctrl);
		}
#endif	
	return rc;
}

int32_t ov8830_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *info = s_ctrl->sensordata;
	int rc = 0;
	CDBG("%s IN\r\n", __func__);

	//Stop stream first
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	msleep(40);

	gpio_direction_output(info->sensor_pwd, 0);
	gpio_direction_output(info->sensor_reset, 0);
	usleep_range(5000, 5100);
	msm_sensor_power_down(s_ctrl);
	msleep(40);
//	if (info->pmic_gpio_enable){
//		lcd_camera_power_onoff(0);
//	}
	return rc;
}

static struct i2c_driver ov8830_i2c_driver = {
	.id_table = ov8830_i2c_id,
	.probe  = ov8830_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov8830_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};



static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&ov8830_i2c_driver);
}

static struct v4l2_subdev_core_ops ov8830_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov8830_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov8830_subdev_ops = {
	.core = &ov8830_subdev_core_ops,
	.video  = &ov8830_subdev_video_ops,
};

int32_t ov8830_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	static int csi_config;
	CDBG("8830 sensor setting in, update_type:0x%x, res:0x%x\r\n",update_type, res);

	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);


	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
		msm_sensor_write_init_settings(s_ctrl);
#ifdef OV8830_OTP_FEATURE
		CDBG("Update OTP\n");
		otp_s_ctrl=s_ctrl;
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x100, 0x1,
				MSM_CAMERA_I2C_BYTE_DATA);

		if(MODULE_ID_SUNNY==camera_module_id)
		otp_update_wb(0x147,0x14d);
		else	if(MODULE_ID_TRULY==camera_module_id)
		otp_update_wb(0x153,0x119);	
		else	if(MODULE_ID_OFLIM==camera_module_id)
		otp_update_wb(0x155,0x146);	
		otp_update_lenc();
		otp_update_BLC();
#endif
		usleep_range(5000, 6000);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x100, 0x0,
		  MSM_CAMERA_I2C_BYTE_DATA);
		csi_config = 0;
		is_first_preview = 1;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
		msm_sensor_write_conf_array(
			s_ctrl->sensor_i2c_client,
			s_ctrl->msm_sensor_reg->mode_settings, res);
		msleep(30);
		if (!csi_config) {
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			CDBG("CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			CDBG("CSI config is done\n");
			mb();
			msleep(30);
			csi_config = 1;
		msm_camera_i2c_write(
				s_ctrl->sensor_i2c_client,
				0x100, 0x1,
				MSM_CAMERA_I2C_BYTE_DATA);
		}
		msm_camera_i2c_write(
				s_ctrl->sensor_i2c_client,
				0x4800, 0x4,
				MSM_CAMERA_I2C_BYTE_DATA);
		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE,
			&s_ctrl->sensordata->pdata->ioclk.vfe_clk_rate);

		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);

		if(res == 1)
		{
			//msleep(10);
			is_first_preview = 0;
		}

		msleep(50);
	}
	return rc;
}

static struct msm_sensor_fn_t ov8830_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = ov8830_write_priv_exp_gain,
	.sensor_write_snapshot_exp_gain = ov8830_write_pict_exp_gain,
	.sensor_csi_setting = ov8830_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = ov8830_sensor_power_up,
	.sensor_power_down = ov8830_sensor_power_down,
	#ifdef OV8830_OTP_FEATURE
	.sensor_get_module_id = ov8830_get_module_id,
	#endif
};

static struct msm_sensor_reg_t ov8830_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov8830_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov8830_start_settings),
	.stop_stream_conf = ov8830_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov8830_stop_settings),
	.group_hold_on_conf = ov8830_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(ov8830_groupon_settings),
	.group_hold_off_conf = ov8830_groupoff_settings,
	.group_hold_off_conf_size =	ARRAY_SIZE(ov8830_groupoff_settings),
	.init_settings = &ov8830_init_conf[0],
	.init_size = ARRAY_SIZE(ov8830_init_conf),
	.mode_settings = &ov8830_confs[0],
	.output_settings = &ov8830_dimensions[0],
	.num_conf = ARRAY_SIZE(ov8830_confs),
};

static struct msm_sensor_ctrl_t ov8830_s_ctrl = {
	.msm_sensor_reg = &ov8830_regs,
	.sensor_i2c_client = &ov8830_sensor_i2c_client,
	.sensor_i2c_addr = 0x20,
	.sensor_output_reg_addr = &ov8830_reg_addr,
	.sensor_id_info = &ov8830_id_info,
	.sensor_exp_gain_info = &ov8830_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov8830_csi_params_array[0],
	.msm_sensor_mutex = &ov8830_mut,
	.sensor_i2c_driver = &ov8830_i2c_driver,
	.sensor_v4l2_subdev_info = ov8830_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov8830_subdev_info),
	.sensor_v4l2_subdev_ops = &ov8830_subdev_ops,
	.func_tbl = &ov8830_func_tbl,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivison 8MP Bayer sensor driver");
MODULE_LICENSE("GPL v2");
