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

     
#include "msm_sensor.h"
#include "msm.h"
#include "ov5648_ov7695_PIP.h"

#define SENSOR_NAME "ov5648_truly_cm8352"
#define PLATFORM_DRIVER_NAME "msm_camera_ov5648_truly_cm8352"
#define ov5648_truly_cm8352_obj ov5648_truly_cm8352_##obj

#ifdef CDBG
#undef CDBG
#endif
#ifdef CDBG_HIGH
#undef CDBG_HIGH
#endif

#define OV5648_TRULY_CM8352_VERBOSE_DGB

#ifdef OV5648_TRULY_CM8352_VERBOSE_DGB
#define CDBG(fmt, args...) printk(fmt, ##args)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#endif

//#define OV5648_TRULY_CM8352_OTP_FEATURE
#ifdef OV5648_TRULY_CM8352_OTP_FEATURE
#undef OV5648_TRULY_CM8352_OTP_FEATURE
#endif

static struct msm_sensor_ctrl_t ov5648_truly_cm8352_s_ctrl;
extern  struct msm_camera_i2c_client *ov7695_i2c_client;
extern  struct msm_camera_i2c_client *ov5648_i2c_client;

int PIP_mode = 0;
int ov5648_sensor_state = STATE_INVALID;
static int csi_config = 0;
/*TODO: Reading gpios from board configuration*/
int ov7695_Xshutdown = 85;
int ov5648_pwdn =23;
int ov5648_reset =79;

DEFINE_MUTEX(ov5648_truly_cm8352_mut);

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_start_settings[] = {
	{0x4202, 0x00},  /* streaming on */
};

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_stop_settings[] = {
	{0x4202, 0x0f},  /* streaming off*/
};

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_groupon_settings[] = {
	{0x3208, 0x0},
};

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_groupoff_settings[] = {
	{0x3208, 0x10},
	{0x3208, 0xa0},
};

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_prev_settings[] = {
	/*1296*972 preview*/
	// 1296x972 30fps 2 lane MIPI 420Mbps/lane
	{0x3501, 0x3d},//exposure
	{0x3502, 0x00},//exposure
	{0x3708, 0x66},
	{0x3709, 0x52},
	{0x370c, 0xc3},

	{0x3800, 0x00},//xstart = 0
	{0x3801, 0x00},//x start
	{0x3802, 0x00},//y start = 0
	{0x3803, 0x00},//y start
	{0x3804, 0x0a},//xend = 2623
	{0x3805, 0x3f},//xend
	{0x3806, 0x07},//yend = 1955
	{0x3807, 0xa3},//yend
	{0x3808, 0x05},//x output size = 1296
	{0x3809, 0x10},//x output size
	{0x380a, 0x03},//y output size = 972
	{0x380b, 0xcc},//y output size
	{0x380c, 0x0B},//hts = 2816
	{0x380d, 0x00},//hts
	{0x380e, 0x03},//vts = 992
	{0x380f, 0xe0},//vts
	{0x3810, 0x00},//isp x win = 8
	{0x3811, 0x08},//isp x win
	{0x3812, 0x00},//isp y win = 4
	{0x3813, 0x04},//isp y win
	{0x3814, 0x31},//x inc
	{0x3815, 0x31},//y inc
	{0x3817, 0x00},//hsync start
	{0x3820, 0x08},//flip off, v bin off
	{0x3821, 0x07},//mirror on, h bin on

	{0x4004, 0x02},//black line number
	{0x4005, 0x18},//BLC normal freeze
	{0x350b, 0x80},//gain = 8x
	{0x4837, 0x17},//MIPI global timing
};

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_snap_settings[] = {
	/* 2592*1944 capture */
	//2592x1944 15fps 2 lane MIPI 420Mbps/lane
	{0x3501, 0x7b}, //exposure
	{0x2502, 0x00}, //exposure
	{0x3708, 0x63}, //
	{0x3709, 0x12}, //
	{0x370c, 0xc0}, //

	{0x3800, 0x00}, //xstart = 0
	{0x3801, 0x00}, //xstart
	{0x3802, 0x00}, //ystart = 0
	{0x3803, 0x00}, //ystart
	{0x3804, 0x0a}, //xend = 2623
	{0x3805, 0x3f}, //xend
	{0x3806, 0x07}, //yend = 1955
	{0x3807, 0xa3}, //yend
	{0x3808, 0x0a}, //x output size = 2592
	{0x3809, 0x20}, //x output size
	{0x380a, 0x07}, //y output size = 1944
	{0x380b, 0x98}, //y output size
	{0x380c, 0x0b}, //hts = 2816
	{0x380d, 0x00}, //hts
	{0x380e, 0x07}, //vts = 1984
	{0x380f, 0xc0}, //vts
	{0x3810, 0x00}, //isp x win = 16
	{0x3811, 0x10}, //isp x win
	{0x3812, 0x00}, //isp y win = 6
	{0x3813, 0x06}, //isp y win
	{0x3814, 0x11}, //x inc
	{0x3815, 0x11}, //y inc
	{0x3817, 0x00}, //hsync start
	{0x3820, 0x40}, //flip off, v bin off
	{0x3821, 0x06}, //mirror on, v bin off

	{0x4004, 0x04}, //black line number
	{0x4005, 0x1a}, //BLC always update
	{0x350b, 0x40}, //gain = 4x
	{0x4837, 0x17}, //MIPI global timing
};

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_video_60fps_settings[] = {
	//640x480 60fps 2 lane MIPI 280Mbps/lane
	{0x3035, 0x31}, //PLL
	{0x2501, 0x1e}, //exposure
	{0x3502, 0xc0}, //exposure
	{0x3708, 0x69},
	{0x3709, 0x92},
	{0x370c, 0xc3},

	{0x3800, 0x00}, //xstart = 0
	{0x3801, 0x00}, //xstart
	{0x3802, 0x00}, //ystart = 2
	{0x3803, 0x02}, //ystart
	{0x3804, 0x0a}, //xend = 2623
	{0x3805, 0x3f}, //xend
	{0x3806, 0x07}, //yend = 1953
	{0x3807, 0xa1}, //yend
	{0x3808, 0x02}, //x output size = 640
	{0x3809, 0x80}, //x output size
	{0x380a, 0x01}, //y output size = 480
	{0x380b, 0xe0}, //y output size
	{0x380c, 0x07}, //hts = 1832
	{0x380d, 0x28}, //hts
	{0x380e, 0x01}, //vts = 508
	{0x380f, 0xfc}, //vts
	{0x3810, 0x00}, //isp x win = 8
	{0x3811, 0x08}, //isp x win
	{0x3812, 0x00}, //isp y win = 4
	{0x3813, 0x04}, //isp y win
	{0x3814, 0x71}, //x inc
	{0x3815, 0x53}, //y inc
	{0x3817, 0x00}, //hsync start
	{0x3820, 0x08}, //flip off, v bin off
	{0x3821, 0x07}, //mirror on, h bin on

	{0x4004, 0x02}, //number of black line
	{0x4005, 0x18}, //BLC normal freeze
	{0x350b, 0xf0}, //gain = 8x
	{0x4837, 0x23}, //MIPI global timing
};

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_video_90fps_settings[] = {
	// 640x480 90fps 2 lane MIPI 420Mbps/lane
	{0x3035, 0x21}, //PLL
	{0x2501, 0x1e}, //exposure
	{0x3502, 0xc0}, //exposure
	{0x3708, 0x69},
	{0x3709, 0x92},
	{0x370c, 0xc3},

	{0x3800, 0x00}, //xstart = 0
	{0x3801, 0x00}, //xstart
	{0x3802, 0x00}, //ystart = 2
	{0x3803, 0x02}, //ystart
	{0x3804, 0x0a}, //xend = 2623
	{0x3805, 0x3f}, //xend
	{0x3806, 0x07}, //yend = 1953
	{0x3807, 0xa1}, //yend
	{0x3808, 0x02}, //x output size = 640
	{0x3809, 0x80}, //x output size
	{0x380a, 0x01}, //y output size = 480
	{0x380b, 0xe0}, //y output size
	{0x380c, 0x07}, //hts = 1832
	{0x380d, 0x28}, //hts
	{0x380e, 0x01}, //vts = 508
	{0x380f, 0xfc}, //vts
	{0x3810, 0x00}, //isp x win = 8
	{0x3811, 0x08}, //isp x win
	{0x3812, 0x00}, //isp y win = 4
	{0x3813, 0x04}, //isp y win
	{0x3814, 0x71}, //x inc
	{0x3815, 0x53}, //y inc
	{0x3817, 0x00}, //hsync start
	{0x3820, 0x08}, //flip off, v bin off
	{0x3821, 0x07}, //mirror on, h bin on

	{0x4004, 0x02}, //number of black line
	{0x4005, 0x18}, //BLC normal freeze
	{0x350b, 0xf0}, //gain = 8x
	{0x4837, 0x17}, //MIPI global timing
};

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_zsl_settings[] = {
	{0x3501, 0x7b}, //exposure
	{0x2502, 0x00}, //exposure
	{0x3708, 0x63}, //
	{0x3709, 0x12}, //
	{0x370c, 0xc0}, //

	{0x3800, 0x00}, //xstart = 0
	{0x3801, 0x00}, //xstart
	{0x3802, 0x00}, //ystart = 0
	{0x3803, 0x00}, //ystart
	{0x3804, 0x0a}, //xend = 2623
	{0x3805, 0x3f}, //xend
	{0x3806, 0x07}, //yend = 1955
	{0x3807, 0xa3}, //yend
	{0x3808, 0x0a}, //x output size = 2592
	{0x3809, 0x20}, //x output size
	{0x380a, 0x07}, //y output size = 1944
	{0x380b, 0x98}, //y output size
	{0x380c, 0x0b}, //hts = 2816
	{0x380d, 0x00}, //hts
	{0x380e, 0x07}, //vts = 1984
	{0x380f, 0xc0}, //vts
	{0x3810, 0x00}, //isp x win = 16
	{0x3811, 0x10}, //isp x win
	{0x3812, 0x00}, //isp y win = 6
	{0x3813, 0x06}, //isp y win
	{0x3814, 0x11}, //x inc
	{0x3815, 0x11}, //y inc
	{0x3817, 0x00}, //hsync start
	{0x3820, 0x40}, //flip off, v bin off
	{0x3821, 0x06}, //mirror on, v bin off

	{0x4004, 0x04}, //black line number
	{0x4005, 0x1a}, //BLC always update
	{0x350b, 0x40}, //gain = 4x
	{0x4837, 0x17}, //MIPI global timing
};

static struct msm_camera_i2c_reg_conf ov5648_truly_cm8352_recommend_settings[] =
{
//	{0x0103, 0x01}, // software reset
	{0x3001, 0x00}, // D[7:0] set to input
	{0x3002, 0x00}, // Vsync, PCLK, FREX, Strobe, CSD, CSK, GPIO input
	{0x3011, 0x02}, // Drive strength 2x
	{0x3018, 0x4c}, // MIPI 2 lane
	{0x3022, 0x00},
	{0x3034, 0x1a}, // 10-bit mode
	{0x3035, 0x21}, // PLL
	{0x3036, 0x69}, // PLL
	{0x3037, 0x03}, // PLL
	{0x3038, 0x00}, // PLL
	{0x3039, 0x00}, // PLL
	{0x303a, 0x00}, // PLLS
	{0x303b, 0x19}, // PLLS
	{0x303c, 0x11}, // PLLS
	{0x303d, 0x30}, //  PLLS
	{0x3105, 0x11},
	{0x3106, 0x05}, // PLL
	{0x3304, 0x28},
	{0x3305, 0x41},
	{0x3306, 0x30},
	{0x3308, 0x00},
	{0x3309, 0xc8},
	{0x330a, 0x01},
	{0x330b, 0x90},
	{0x330c, 0x02},
	{0x330d, 0x58},
	{0x330e, 0x03},
	{0x330f, 0x20},
	{0x3300, 0x00},

	{0x3500, 0x00}, // exposure [19:16]
	{0x3501, 0x3d}, // exposure [15:8]
	{0x3502, 0x00}, // exposure [7:0], exposure = 0x3d0 = 976
	{0x3503, 0x07}, // gain has no delay, manual agc/aec
	{0x350a, 0x00}, // gain[9:8]
	{0x350b, 0x40}, // gain[7:0], gain = 4x

	{0x3601, 0x33}, // analog control
	{0x3602, 0x00}, // analog control
	{0x3611, 0x0e}, // analog control
	{0x3612, 0x2b}, // analog control
	{0x3614, 0x50}, // analog control
	{0x3620, 0x33}, // analog control
	{0x3622, 0x00}, // analog control
	{0x3630, 0xad}, // analog control
	{0x3631, 0x00}, // analog control
	{0x3632, 0x94}, // analog control
	{0x3633, 0x17}, // analog control
	{0x3634, 0x14}, // analog control
	{0x3705, 0x2a}, // analog control
	{0x3708, 0x66}, // analog control
	{0x3709, 0x52}, // analog control
	{0x370b, 0x23}, // analog control
	{0x370c, 0xc3}, // analog control
	{0x370d, 0x00}, // analog control
	{0x370e, 0x00}, // analog control
	{0x371c, 0x07}, // analog control
	{0x3739, 0xd2}, // analog control
	{0x373c, 0x00},

	{0x3800, 0x00}, // xstart = 0
	{0x3801, 0x00}, // xstart
	{0x3802, 0x00}, // ystart = 0
	{0x3803, 0x00}, // ystart
	{0x3804, 0x0a}, // xend = 2623
	{0x3805, 0x3f}, // yend
	{0x3806, 0x07}, // yend = 1955
	{0x3807, 0xa3}, // yend
	{0x3808, 0x05}, // x output size = 1296
	{0x3809, 0x10}, // x output size
	{0x380a, 0x03}, // y output size = 972
	{0x380b, 0xcc}, // y output size
	{0x380c, 0x05}, // hts = 1408
	{0x380d, 0x80}, // hts
	{0x380e, 0x03}, // vts = 992
	{0x380f, 0xe0}, // vts
	{0x3810, 0x00}, // isp x win = 8
	{0x3811, 0x08}, // isp x win
	{0x3812, 0x00}, // isp y win = 4
	{0x3813, 0x04}, // isp y win
	{0x3814, 0x31}, // x inc
	{0x3815, 0x31}, // y inc
	{0x3817, 0x00}, // hsync start
	{0x3820, 0x08}, // flip off, v bin off
	{0x3821, 0x07}, // mirror on, h bin on
	{0x3826, 0x03},
	{0x3829, 0x00},
	{0x382b, 0x0b},
	{0x3830, 0x00},
	{0x3836, 0x00},
	{0x3837, 0x00},
	{0x3838, 0x00},
	{0x3839, 0x04},
	{0x383a, 0x00},
	{0x383b, 0x01},

	{0x3b00, 0x00}, // strobe off
	{0x3b02, 0x08}, // shutter delay
	{0x3b03, 0x00}, // shutter delay
	{0x3b04, 0x04}, // frex_exp
	{0x3b05, 0x00}, // frex_exp
	{0x3b06, 0x04},
	{0x3b07, 0x08}, // frex inv
	{0x3b08, 0x00}, // frex exp req
	{0x3b09, 0x02}, // frex end option
	{0x3b0a, 0x04}, // frex rst length
	{0x3b0b, 0x00}, // frex strobe width
	{0x3b0c, 0x3d}, // frex strobe width

	{0x3f01, 0x0d},
	{0x3f0f, 0xf5},

	{0x4000, 0x89}, // blc enable
	{0x4001, 0x02}, // blc start line
	{0x4002, 0x45}, // blc auto, reset frame number = 5
	{0x4004, 0x02}, // black line number
	{0x4005, 0x18}, // blc normal freeze
	{0x4006, 0x08},
	{0x4007, 0x10},
	{0x4008, 0x00},

	{0x4300, 0xf8},
	{0x4303, 0xff},
	{0x4304, 0x00},
	{0x4307, 0xff},
	{0x4520, 0x00},
	{0x4521, 0x00},
	{0x4511, 0x22},

	{0x481f, 0x3c}, // MIPI clk prepare min
	{0x4826, 0x00}, // MIPI hs prepare min
	{0x4837, 0x18}, // MIPI global timing
	{0x4b00, 0x06},
	{0x4b01, 0x0a},
	{0x5000, 0xff}, // bpc on, wpc on
	{0x5001, 0x00}, // awb disable
	{0x5002, 0x41}, // win enable, awb gain enable
	{0x5003, 0x0a}, // buf en, bin auto en
	{0x5004, 0x00}, // size man off
	{0x5043, 0x00},
	{0x5013, 0x00},
	{0x501f, 0x03}, // ISP output data
	{0x503d, 0x00}, // test pattern off
	{0x5180, 0x08}, // manual gain enable
	{0x5a00, 0x08},
	{0x5b00, 0x01},
	{0x5b01, 0x40},
	{0x5b02, 0x00},
	{0x5b03, 0xf0},
	{0x0100, 0x01}, // wake up from software sleep

	{0x350b, 0x80}, // gain = 8x
	{0x4837, 0x17}, // MIPI global timing
};

static struct msm_camera_i2c_conf_array ov5648_truly_cm8352_init_conf[] = {
	{&ov5648_truly_cm8352_recommend_settings[0],
	ARRAY_SIZE(ov5648_truly_cm8352_recommend_settings), 0, MSM_CAMERA_I2C_BYTE_DATA}
};

static struct msm_camera_i2c_conf_array ov5648_truly_cm8352_confs[] = {
	{&ov5648_truly_cm8352_snap_settings[0],
	ARRAY_SIZE(ov5648_truly_cm8352_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov5648_truly_cm8352_prev_settings[0],
	ARRAY_SIZE(ov5648_truly_cm8352_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov5648_truly_cm8352_video_60fps_settings[0],
	ARRAY_SIZE(ov5648_truly_cm8352_video_60fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov5648_truly_cm8352_video_90fps_settings[0],
	ARRAY_SIZE(ov5648_truly_cm8352_video_90fps_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov5648_truly_cm8352_zsl_settings[0],
	ARRAY_SIZE(ov5648_truly_cm8352_zsl_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_camera_csi_params ov5648_truly_cm8352_csi_params = {
	.data_format = CSI_10BIT,
	.lane_cnt    = 2,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 10,
};

static struct v4l2_subdev_info ov5648_truly_cm8352_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};
enum DIMENS_t{
	NORMAL_4_3_SNAPSHOT,
	NORMAL_4_3_PREVIEW,
	PIP_16_9_SNAPSHOT,
	PIP_16_9_PREVIEW,
};
static struct msm_sensor_output_info_t sensor_reconfig_dimensions[] = {
	{ /* For 4:3 SNAPSHOT */
		.x_output = 0xa20,         /*2592*/
		.y_output = 0x798,         /*1944*/
		.line_length_pclk = 0xb00,
		.frame_length_lines = 0x7c0,
		.vt_pixel_clk = 79000000,
		.op_pixel_clk = 158000000,
		.binning_factor = 0x0,
	},
	{ /* For 4:3 PREVIEW */
		.x_output = 0x510,         /*1296*/
		.y_output = 0x3cc,         /*972*/
		.line_length_pclk = 0xb00,
		.frame_length_lines = 0x3e0,
		.vt_pixel_clk = 55000000,
		.op_pixel_clk = 110000000,
		.binning_factor = 0x0,
	},
	{ /* For 16:9 SNAPSHOT */
		.x_output = 0xa00,         /*2560*/
		.y_output = 0x5a0,         /*1440*/
		.line_length_pclk = 0x160a,//HTS = 5642
		.frame_length_lines = 0x05a0,// VTS = 1480
		.vt_pixel_clk = 84000000,
		.op_pixel_clk = 84000000,
		.binning_factor = 0x0,
	},
	{ /* For 16:9 PREVIEW */
		.x_output = 0x500,         /*1280*/
		.y_output = 0x2d0,         /*720*/
		.line_length_pclk = 0xec4,//3780 
		.frame_length_lines = 0x2e6,//742
		.vt_pixel_clk = 84000000,
		.op_pixel_clk = 84000000,
		.binning_factor = 0x0,
	},
};
static struct msm_sensor_output_info_t ov5648_truly_cm8352_dimensions[] = {
	{ /* For SNAPSHOT */
		.x_output = 0xa20,         /*2592*/
		.y_output = 0x798,         /*1944*/
		.line_length_pclk = 0xb00,
		.frame_length_lines = 0x7c0,
		.vt_pixel_clk = 79000000,
		.op_pixel_clk = 158000000,
		.binning_factor = 0x0,
	},
	{ /* For PREVIEW */
		.x_output = 0x510,         /*1296*/
		.y_output = 0x3cc,         /*972*/
		.line_length_pclk = 0xb00,
		.frame_length_lines = 0x3e0,
		.vt_pixel_clk = 55000000,
		.op_pixel_clk = 110000000,
		.binning_factor = 0x0,
	},

	{ /* For 60fps */
		.x_output = 0x280,  /*640*/
		.y_output = 0x1E0,   /*480*/
		.line_length_pclk = 0x728,
		.frame_length_lines = 0x1FC,
		.vt_pixel_clk = 56004480,
		.op_pixel_clk = 159408000,
		.binning_factor = 0x0,
	},
	{ /* For 90fps */
		.x_output = 0x280,  /*640*/
		.y_output = 0x1E0,   /*480*/
		.line_length_pclk = 0x728,
		.frame_length_lines = 0x1FC,
		.vt_pixel_clk = 56004480,
		.op_pixel_clk = 159408000,
		.binning_factor = 0x0,
	},
	{ /* For ZSL */
		.x_output = 0xa20,         /*2592*/
		.y_output = 0x798,         /*1944*/
		.line_length_pclk = 0xb00,
		.frame_length_lines = 0x7c0,
		.vt_pixel_clk = 79000000,
		.op_pixel_clk = 158000000,
		.binning_factor = 0x0,
	},

};

static struct msm_sensor_output_reg_addr_t ov5648_truly_cm8352_reg_addr = {
	.x_output = 0x3808,
	.y_output = 0x380A,
	.line_length_pclk = 0x380C,
	.frame_length_lines = 0x380E,
};

static struct msm_camera_csi_params *ov5648_truly_cm8352_csi_params_array[] = {
	&ov5648_truly_cm8352_csi_params, /* Snapshot */
	&ov5648_truly_cm8352_csi_params, /* Preview */
	&ov5648_truly_cm8352_csi_params, /* 60fps */
	&ov5648_truly_cm8352_csi_params, /* 90fps */
	&ov5648_truly_cm8352_csi_params, /* ZSL */
};    
///////////////////////////////////////////////
int32_t ov5648_truly_cm8352_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res);
static ssize_t sensor_show_PIP_mode(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d", PIP_mode);
}
static ssize_t sensor_store_PIP_mode(struct device *dev,
			struct device_attribute *attr,const char *buf,size_t count)
{
	unsigned long value = simple_strtoul(buf, NULL, 10);
	CDBG("Enter %s:PIP_mode = %lu,ov5648_sensor_state = %d",
		__func__,value,ov5648_sensor_state);
	PIP_mode = value;

	return count;	
}
static DEVICE_ATTR(PIP_mode, 0644,sensor_show_PIP_mode, sensor_store_PIP_mode);

static struct attribute *sensor_attributes[] = {
	&dev_attr_PIP_mode.attr,
	NULL
};
static const struct attribute_group sensor_attr_group = {
	.attrs = sensor_attributes,
};

///////////////////////////////////////////////

static struct msm_sensor_id_info_t ov5648_truly_cm8352_id_info = {
	.sensor_id_reg_addr = 0x300a,
	.sensor_id = 0x5648,
};

static struct msm_sensor_exp_gain_info_t ov5648_truly_cm8352_exp_gain_info = {
	.coarse_int_time_addr = 0x3500,
	.global_gain_addr = 0x350A,
	.vert_offset = 4,
};

void ov5648_truly_cm8352_sensor_reset_stream(struct msm_sensor_ctrl_t *s_ctrl)
{
	msm_camera_i2c_write(
		s_ctrl->sensor_i2c_client,
		0x103, 0x1,
		MSM_CAMERA_I2C_BYTE_DATA);
}

#ifdef OV5648_TRULY_CM8352_OTP_FEATURE
/*******************************************************************************
*
*******************************************************************************/
struct otp_struct {
	uint8_t customer_id;
	uint8_t module_integrator_id;
	uint8_t lens_id;
	uint8_t rg_ratio;
	uint8_t bg_ratio;
	uint8_t user_data[5];
} st_ov5648_truly_cm8352_otp;

/*******************************************************************************
* index: index of otp group. (0, 1, 2)
* return value:
*     0, group index is empty
*     1, group index has invalid data
*     2, group index has valid data
*******************************************************************************/
uint16_t ov5648_truly_cm8352_check_otp(struct msm_sensor_ctrl_t *s_ctrl, uint16_t index)
{
	uint16_t temp, i;
	uint16_t address;
	/* read otp into buffer */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d21, 0x01,
		MSM_CAMERA_I2C_BYTE_DATA);

	usleep_range(2000, 2500);
	address = 0x3d05 + index*9;
	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, address, &temp,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* disable otp read */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d21, 0x00,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* clear otp buffer */
	for (i = 0; i < 32; i++) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, (0x3d00+i),
				0x00, MSM_CAMERA_I2C_BYTE_DATA);
	}

	if (!temp)
		return 0;
	else if ((!(temp & 0x80)) && (temp&0x7f))
		return 2;
	else
		return 1;
}
/*******************************************************************************
*
*******************************************************************************/
void ov5648_truly_cm8352_read_otp(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t index, struct otp_struct *potp)
{
	uint16_t temp, i;
	uint16_t address;
	/* read otp into buffer */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d21, 0x01,
		MSM_CAMERA_I2C_BYTE_DATA);

	usleep_range(2000, 2500);
	address = 0x3d05 + index*9;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, address, &temp,
		MSM_CAMERA_I2C_BYTE_DATA);

	potp->module_integrator_id = temp;
	potp->customer_id = temp & 0x7f;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+1), &temp,
		MSM_CAMERA_I2C_BYTE_DATA);
	potp->lens_id = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+2), &temp,
		MSM_CAMERA_I2C_BYTE_DATA);
	potp->rg_ratio = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+3), &temp,
		MSM_CAMERA_I2C_BYTE_DATA);
	potp->bg_ratio = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+4), &temp,
		MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[0] = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+5), &temp,
		MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[1] = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+6), &temp,
		MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[2] = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+7), &temp,
		MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[3] = temp;

	msm_camera_i2c_read(s_ctrl->sensor_i2c_client, (address+8), &temp,
		MSM_CAMERA_I2C_BYTE_DATA);
	potp->user_data[4] = temp;

	CDBG("%s customer_id  = 0x%02x\r\n", __func__, potp->customer_id);
	CDBG("%s lens_id      = 0x%02x\r\n", __func__, potp->lens_id);
	CDBG("%s rg_ratio     = 0x%02x\r\n", __func__, potp->rg_ratio);
	CDBG("%s bg_ratio     = 0x%02x\r\n", __func__, potp->bg_ratio);
	CDBG("%s user_data[0] = 0x%02x\r\n", __func__, potp->user_data[0]);
	CDBG("%s user_data[1] = 0x%02x\r\n", __func__, potp->user_data[1]);
	CDBG("%s user_data[2] = 0x%02x\r\n", __func__, potp->user_data[2]);
	CDBG("%s user_data[3] = 0x%02x\r\n", __func__, potp->user_data[3]);
	CDBG("%s user_data[4] = 0x%02x\r\n", __func__, potp->user_data[4]);

	/* disable otp read */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3d21, 0x00,
		MSM_CAMERA_I2C_BYTE_DATA);
	/* clear otp buffer */
	for (i = 0; i < 32; i++) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, (0x3d00+i),
				0x00, MSM_CAMERA_I2C_BYTE_DATA);
	}
}

/*******************************************************************************
* R_gain, sensor red gain of AWB, 0x400 =1
* G_gain, sensor green gain of AWB, 0x400 =1
* B_gain, sensor blue gain of AWB, 0x400 =1
*******************************************************************************/
void ov5648_truly_cm8352_update_awb_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t R_gain, uint16_t G_gain, uint16_t B_gain)
{
	CDBG("%s R_gain = 0x%04x\r\n", __func__, R_gain);
	CDBG("%s G_gain = 0x%04x\r\n", __func__, G_gain);
	CDBG("%s B_gain = 0x%04x\r\n", __func__, B_gain);
	if (R_gain > 0x400) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x5186,
				(R_gain>>8), MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x5187,
				(R_gain&0xff), MSM_CAMERA_I2C_BYTE_DATA);
	}
	if (G_gain > 0x400) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x5188,
				(G_gain>>8), MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x5189,
				(G_gain&0xff), MSM_CAMERA_I2C_BYTE_DATA);
	}
	if (B_gain > 0x400) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x518a,
				(B_gain>>8), MSM_CAMERA_I2C_BYTE_DATA);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x518b,
				(B_gain&0xff), MSM_CAMERA_I2C_BYTE_DATA);
	}
}

/*******************************************************************************
* R/G and B/G of typical camera module is defined here
*******************************************************************************/
#define OV5648_TRULY_CM8352_RG_RATIO_TYPICAL_VALUE 64
#define OV5648_TRULY_CM8352_BG_RATIO_TYPICAL_VALUE 105
/*******************************************************************************
* call this function after OV5648_TRULY_CM8352 initialization
* return value:
*     0, update success
*     1, no OTP
*******************************************************************************/
uint16_t ov5648_truly_cm8352_update_otp(struct msm_sensor_ctrl_t *s_ctrl)
{
	uint16_t i;
	uint16_t otp_index;
	uint16_t temp;
	uint16_t R_gain, G_gain, B_gain, G_gain_R, G_gain_B;
	/* R/G and B/G of current camera module is read out from sensor OTP */
	/* check first OTP with valid data */
	for (i = 0; i < 3; i++) {
		temp = ov5648_truly_cm8352_check_otp(s_ctrl, i);
		if (temp == 2) {
			otp_index = i;
			break;
		}
	}
	if (i == 3) {
		/* no valid wb OTP data */
		CDBG("no valid wb OTP data\r\n");
		return 1;
	}
	ov5648_truly_cm8352_read_otp(s_ctrl, otp_index, &st_ov5648_truly_cm8352_otp);
	/* calculate G_gain */
	/* 0x400 = 1x gain */
	if (st_ov5648_truly_cm8352_otp.bg_ratio < OV5648_TRULY_CM8352_BG_RATIO_TYPICAL_VALUE) {
		if (st_ov5648_truly_cm8352_otp.rg_ratio < OV5648_TRULY_CM8352_RG_RATIO_TYPICAL_VALUE) {
			G_gain = 0x400;
			B_gain = 0x400 *
				OV5648_TRULY_CM8352_BG_RATIO_TYPICAL_VALUE /
				st_ov5648_truly_cm8352_otp.bg_ratio;
			R_gain = 0x400 *
				OV5648_TRULY_CM8352_RG_RATIO_TYPICAL_VALUE /
				st_ov5648_truly_cm8352_otp.rg_ratio;
		} else {
			R_gain = 0x400;
			G_gain = 0x400 *
				st_ov5648_truly_cm8352_otp.rg_ratio /
				OV5648_TRULY_CM8352_RG_RATIO_TYPICAL_VALUE;
			B_gain = G_gain *
				OV5648_TRULY_CM8352_BG_RATIO_TYPICAL_VALUE /
				st_ov5648_truly_cm8352_otp.bg_ratio;
		}
	} else {
		if (st_ov5648_truly_cm8352_otp.rg_ratio < OV5648_TRULY_CM8352_RG_RATIO_TYPICAL_VALUE) {
			B_gain = 0x400;
			G_gain = 0x400 *
				st_ov5648_truly_cm8352_otp.bg_ratio /
				OV5648_TRULY_CM8352_BG_RATIO_TYPICAL_VALUE;
			R_gain = G_gain *
				OV5648_TRULY_CM8352_RG_RATIO_TYPICAL_VALUE /
				st_ov5648_truly_cm8352_otp.rg_ratio;
		} else {
			G_gain_B = 0x400 *
				st_ov5648_truly_cm8352_otp.bg_ratio /
				OV5648_TRULY_CM8352_BG_RATIO_TYPICAL_VALUE;
			G_gain_R = 0x400 *
				st_ov5648_truly_cm8352_otp.rg_ratio /
				OV5648_TRULY_CM8352_RG_RATIO_TYPICAL_VALUE;
			if (G_gain_B > G_gain_R) {
				B_gain = 0x400;
				G_gain = G_gain_B;
				R_gain = G_gain *
					OV5648_TRULY_CM8352_RG_RATIO_TYPICAL_VALUE /
					st_ov5648_truly_cm8352_otp.rg_ratio;
			} else {
				R_gain = 0x400;
				G_gain = G_gain_R;
				B_gain = G_gain *
					OV5648_TRULY_CM8352_BG_RATIO_TYPICAL_VALUE /
					st_ov5648_truly_cm8352_otp.bg_ratio;
			}
		}
	}
	ov5648_truly_cm8352_update_awb_gain(s_ctrl, R_gain, G_gain, B_gain);
	return 0;
}
#endif

static int32_t ov5648_truly_cm8352_write_pict_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{

	static uint16_t max_line;
	uint8_t gain_lsb, gain_hsb;
	u8 intg_time_hsb, intg_time_msb, intg_time_lsb;

	gain_lsb = (uint8_t) (gain);
	gain_hsb = (uint8_t)((gain & 0x300)>>8);

	CDBG(KERN_ERR "snapshot exposure seting 0x%x, 0x%x, %d"
		, gain, line, line);
	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);

	max_line = 1964;
	/*PIP_mode:fixed fps*/
	if(!PIP_mode){
	if (line > 1964) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			(uint8_t)((line+4) >> 8),
			MSM_CAMERA_I2C_BYTE_DATA);

		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			(uint8_t)((line+4) & 0x00FF),
			MSM_CAMERA_I2C_BYTE_DATA);
		max_line = line + 4;
	} else if (max_line > 1968) {
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines,
			(uint8_t)(1968 >> 8),
			MSM_CAMERA_I2C_BYTE_DATA);

		 msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
			s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
			(uint8_t)(1968 & 0x00FF),
			MSM_CAMERA_I2C_BYTE_DATA);
			max_line = 1968;
	}
	}

	line = line<<4;
	/* ov5648_truly_cm8352 need this operation */
	intg_time_hsb = (u8)(line>>16);
	intg_time_msb = (u8) ((line & 0xFF00) >> 8);
	intg_time_lsb = (u8) (line & 0x00FF);

	/* FIXME for BLC trigger */
	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		intg_time_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		intg_time_msb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 2,
		intg_time_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* gain */

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		gain_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		gain_lsb^0x1,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		intg_time_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		intg_time_msb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 2,
		intg_time_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* gain */

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		gain_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		gain_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);


	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);
	return 0;

}


static int32_t ov5648_truly_cm8352_write_prev_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
						uint16_t gain, uint32_t line)
{
	u8 intg_time_hsb, intg_time_msb, intg_time_lsb;
	uint8_t gain_lsb, gain_hsb;
	uint32_t fl_lines = s_ctrl->curr_frame_length_lines;
	uint8_t offset = s_ctrl->sensor_exp_gain_info->vert_offset;

	CDBG(KERN_ERR "preview exposure setting 0x%x, 0x%x, %d",
		 gain, line, line);

	gain_lsb = (uint8_t) (gain);
	gain_hsb = (uint8_t)((gain & 0x300)>>8);

	fl_lines = (fl_lines * s_ctrl->fps_divider) / Q10;

	s_ctrl->func_tbl->sensor_group_hold_on(s_ctrl);

	/* adjust frame rate */
	if (line > (fl_lines - offset))
		fl_lines = line + offset;

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines,
		(uint8_t)(fl_lines >> 8),
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_output_reg_addr->frame_length_lines + 1,
		(uint8_t)(fl_lines & 0x00FF),
		MSM_CAMERA_I2C_BYTE_DATA);

	line = line<<4;
	/* ov5648_truly_cm8352 need this operation */
	intg_time_hsb = (u8)(line>>16);
	intg_time_msb = (u8) ((line & 0xFF00) >> 8);
	intg_time_lsb = (u8) (line & 0x00FF);


	/* Coarse Integration Time */
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr,
		intg_time_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 1,
		intg_time_msb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->coarse_int_time_addr + 2,
		intg_time_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	/* gain */

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr,
		gain_hsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		s_ctrl->sensor_exp_gain_info->global_gain_addr + 1,
		gain_lsb,
		MSM_CAMERA_I2C_BYTE_DATA);

	s_ctrl->func_tbl->sensor_group_hold_off(s_ctrl);

	return 0;
}

static const struct i2c_device_id ov5648_truly_cm8352_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov5648_truly_cm8352_s_ctrl},
	{ }
};
extern void camera_af_software_powerdown(struct i2c_client *client);
int32_t ov5648_truly_cm8352_sensor_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int32_t rc = 0;
	struct msm_sensor_ctrl_t *s_ctrl;

	CDBG("%s IN\r\n", __func__);
	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
	s_ctrl->sensor_i2c_addr = s_ctrl->sensor_i2c_addr;

	rc = msm_sensor_i2c_probe(client, id);

	if (client->dev.platform_data == NULL) {
		CDBG_HIGH("%s: NULL sensor data\n", __func__);
		return -EFAULT;
	}

	CDBG("%s: Create SYSFS for PIP mode\n", __func__);
	rc = sysfs_create_group(&client->dev.kobj, &sensor_attr_group);

	/* send software powerdown cmd to AF motor, avoid current leak */
	if(0 == rc)
	{
		camera_af_software_powerdown(client);
	}
	usleep_range(5000, 5100);

	return rc;
}

static struct i2c_driver ov5648_truly_cm8352_i2c_driver = {
	.id_table = ov5648_truly_cm8352_i2c_id,
	.probe  = ov5648_truly_cm8352_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

struct msm_camera_i2c_client ov5648_truly_cm8352_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};
struct msm_camera_i2c_client *ov5648_i2c_client = &ov5648_truly_cm8352_sensor_i2c_client ;

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&ov5648_truly_cm8352_i2c_driver);
}

static struct v4l2_subdev_core_ops ov5648_truly_cm8352_subdev_core_ops = {
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov5648_truly_cm8352_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov5648_truly_cm8352_subdev_ops = {
	.core = &ov5648_truly_cm8352_subdev_core_ops,
	.video  = &ov5648_truly_cm8352_subdev_video_ops,
};

int32_t ov5648_truly_cm8352_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *info = NULL;
	unsigned short rdata;
	int rc;
	CDBG("%s IN\r\n", __func__);

	info = s_ctrl->sensordata;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x4202, 0xf,
		MSM_CAMERA_I2C_BYTE_DATA);
	msleep(40);
	rc = msm_camera_i2c_read(s_ctrl->sensor_i2c_client, 0x3018,
			&rdata, MSM_CAMERA_I2C_BYTE_DATA);
	CDBG("ov5648_truly_cm8352_sensor_power_down: %d\n", rc);
	rdata |= 0x18;
	msm_camera_i2c_write(s_ctrl->sensor_i2c_client,
		0x3018, rdata,
		MSM_CAMERA_I2C_BYTE_DATA);
	msleep(40);
	gpio_direction_output(info->sensor_pwd, 0);
	/*Power down ov7695 */
	/*TODO:Right?*/
	if(PIP_mode){
		gpio_direction_output(ov7695_Xshutdown, 0);				
	}

	usleep_range(5000, 5100);
	msm_sensor_power_down(s_ctrl);
	msleep(40);
	//if (s_ctrl->sensordata->pmic_gpio_enable){
	//	lcd_camera_power_onoff(0);
	//}
	return 0;
}

int32_t ov5648_truly_cm8352_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	struct msm_camera_sensor_info *info = s_ctrl->sensordata;

	CDBG("%s IN\r\n", __func__);

	CDBG("%s, sensor_pwd:%d, sensor_reset:%d\r\n",__func__, info->sensor_pwd, info->sensor_reset);

	gpio_direction_output(info->sensor_pwd, 0);
	gpio_direction_output(info->sensor_reset, 0);
	usleep_range(5000, 6000);
#if 0
	for(rc = 0; rc<1000; rc++)
	{
		CDBG("GPIO cycle...\n");
		gpio_direction_output(info->sensor_pwd, 1);
		gpio_direction_output(info->sensor_reset, 0);
		usleep_range(5000, 6000);
		gpio_direction_output(info->sensor_pwd, 0);
		gpio_direction_output(info->sensor_reset, 1);
		usleep_range(5000, 6000);
	}
#endif
	//if (info->pmic_gpio_enable) {
	//	lcd_camera_power_onoff(1);
	//}
	usleep_range(5000, 6000);
	rc = msm_sensor_power_up(s_ctrl);
	if (rc < 0) {
		CDBG("%s: msm_sensor_power_up failed\n", __func__);
		return rc;
	}
	
	/* turn on ldo and vreg */
	usleep_range(1000, 1100);
	gpio_direction_output(info->sensor_pwd, 1);
	msleep(10);
	
	/*Power up ov7695 */
	if(PIP_mode){
		gpio_direction_output(ov7695_Xshutdown, 1);				
	}
	gpio_direction_output(info->sensor_reset, 1);
	msleep(25);

		if(!PIP_mode){
		/*Reconfig for Normal mode*/
			memcpy(&ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0],
				&sensor_reconfig_dimensions[NORMAL_4_3_SNAPSHOT],
				2*sizeof(struct msm_sensor_output_info_t));

		}else{
		/*Reconfig for PIP mode*/
			memcpy(&ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0],
				&sensor_reconfig_dimensions[PIP_16_9_SNAPSHOT],
				2*sizeof(struct msm_sensor_output_info_t));
		}
		
		printk("[0]x_output = 0x%x,x_output = 0x%x,line_length_pclk = 0x%x,frame_length_lines = 0x%x\n",
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0].x_output,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0].y_output,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0].line_length_pclk,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0].frame_length_lines);
		printk("[1]x_output = 0x%x,x_output = 0x%x,line_length_pclk = 0x%x,frame_length_lines = 0x%x\n",
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[1].x_output,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[1].y_output,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[1].line_length_pclk,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[1].frame_length_lines);

	return rc;

}

static int32_t vfe_clk = 266667000;
static int is_first_preview = 1;
int32_t ov5648_truly_cm8352_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
	int32_t index = 0;
	s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);

	if (update_type != MSM_SENSOR_REG_INIT)
	{
		if ( csi_config == 0 || res == 0)
			msleep(66);
		else
			msleep(266);

		msm_camera_i2c_write(
				s_ctrl->sensor_i2c_client,
				0x4800, 0x25,
				MSM_CAMERA_I2C_BYTE_DATA);
	}
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		s_ctrl->curr_csi_params = NULL;
		msm_sensor_enable_debugfs(s_ctrl);
#if 0
		if(!PIP_mode){
		/*Reconfig for Normal mode*/
			memcpy(&ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0],
				&sensor_reconfig_dimensions[NORMAL_4_3_SNAPSHOT],
				2*sizeof(struct msm_sensor_output_info_t));

		}else{
		/*Reconfig for PIP mode*/
			memcpy(&ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0],
				&sensor_reconfig_dimensions[PIP_16_9_SNAPSHOT],
				2*sizeof(struct msm_sensor_output_info_t));
		}
		
		printk("[0]x_output = 0x%x,x_output = 0x%x,line_length_pclk = 0x%x,frame_length_lines = 0x%x\n",
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0].x_output,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0].y_output,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0].line_length_pclk,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[0].frame_length_lines);
		printk("[1]x_output = 0x%x,x_output = 0x%x,line_length_pclk = 0x%x,frame_length_lines = 0x%x\n",
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[1].x_output,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[1].y_output,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[1].line_length_pclk,
			ov5648_truly_cm8352_s_ctrl.msm_sensor_reg->output_settings[1].frame_length_lines);
#endif
		if(!PIP_mode){
			msm_camera_i2c_write(
				s_ctrl->sensor_i2c_client,
				0x103, 0x1,
				MSM_CAMERA_I2C_BYTE_DATA);
			msleep(5);
			msm_sensor_write_init_settings(s_ctrl);
			ov5648_sensor_state = NORMAL_INIT;
		}else{
			//ov5648 reset
			msm_camera_i2c_write(
				s_ctrl->sensor_i2c_client,
				0x103, 0x1,
				MSM_CAMERA_I2C_BYTE_DATA);
			msleep(5);
			//ov5648 PIP init
			msm_sensor_write_conf_array(
				s_ctrl->sensor_i2c_client,
				ov5648_ov7695_PIP_confs, PIP_OV5648_INIT);
			//ov7695 reset
			msm_camera_i2c_write(
				ov7695_i2c_client,
				0x103, 0x1,
				MSM_CAMERA_I2C_BYTE_DATA);
			msleep(5);
			//ov7695 PIP init
			msm_sensor_write_conf_array(
				ov7695_i2c_client,
				ov5648_ov7695_PIP_confs, PIP_OV7695_INIT);
			ov5648_sensor_state = PIP_INIT;
		}
			
#ifdef OV5648_TRULY_CM8352_OTP_FEATURE
		CDBG("Update OTP\n");
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x100, 0x1,
				MSM_CAMERA_I2C_BYTE_DATA);
		ov5648_truly_cm8352_update_otp(s_ctrl);
		usleep_range(5000, 6000);
		msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x100, 0x0,
		  MSM_CAMERA_I2C_BYTE_DATA);
#endif
		csi_config = 0;
		is_first_preview = 1;
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
					
		if(!PIP_mode){
			/*Normal preview/video/Capture*/
			msm_sensor_write_conf_array(
				s_ctrl->sensor_i2c_client,
				s_ctrl->msm_sensor_reg->mode_settings, res);
			ov5648_sensor_state = res;//state == res in normal mode
		}else{
			/*PIP preview/video/Capture*/
			//ov5648 standby
			msm_camera_i2c_write(
				s_ctrl->sensor_i2c_client,
				0x100, 0x00,
				MSM_CAMERA_I2C_BYTE_DATA);
			msleep(50);
			switch(res){
			case MSM_SENSOR_RES_QTR://preview
				index = PIP_OV5648_PREVIEW;
				ov5648_sensor_state = PIP_PREVIEW;
				break;
			case MSM_SENSOR_RES_2://video
				index = PIP_OV5648_VIDEO;
				ov5648_sensor_state = PIP_VIDEO;
				break;
			case MSM_SENSOR_RES_FULL://snapshot
				index = PIP_OV5648_CAPTURE;
				ov5648_sensor_state = PIP_CAPTURE;
				break;
			default:
				printk("ERROR:res(%d) is error!\n",res);
				index = PIP_OV5648_PREVIEW;
				ov5648_sensor_state = PIP_PREVIEW;
				break;
			}
			CDBG("PIP index = %d\n",index);
			CDBG("PIP settings in progress\n");
			#if 0
			//ov5648 standby
			msm_camera_i2c_write(
				s_ctrl->sensor_i2c_client,
				0x100, 0x00,
				MSM_CAMERA_I2C_BYTE_DATA);
			msleep(50);
			#endif
			//ov5648 PIP settings
			msm_sensor_write_conf_array(
				s_ctrl->sensor_i2c_client,
				ov5648_ov7695_PIP_confs, index);
			//ov7695 PIP settings
			msm_sensor_write_conf_array(
				ov7695_i2c_client,
				ov5648_ov7695_PIP_confs, index+1);
			CDBG("PIP settings is done\n");
		}
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

		if(!is_first_preview)
		{
			msleep(266);
		}
		else
		{
			msleep(10);
			is_first_preview = 0;
		}

		if (res == MSM_SENSOR_RES_4)
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
					NOTIFY_PCLK_CHANGE,
					&vfe_clk);

		s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		msleep(50);
	}
	return rc;
}

static struct msm_sensor_fn_t ov5648_truly_cm8352_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = msm_sensor_set_fps,
	.sensor_write_exp_gain = ov5648_truly_cm8352_write_prev_exp_gain,
	.sensor_write_snapshot_exp_gain = ov5648_truly_cm8352_write_pict_exp_gain,
	.sensor_csi_setting = ov5648_truly_cm8352_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = ov5648_truly_cm8352_sensor_power_up,
	.sensor_power_down = ov5648_truly_cm8352_sensor_power_down,
};

static struct msm_sensor_reg_t ov5648_truly_cm8352_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov5648_truly_cm8352_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov5648_truly_cm8352_start_settings),
	.stop_stream_conf = ov5648_truly_cm8352_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov5648_truly_cm8352_stop_settings),
	.group_hold_on_conf = ov5648_truly_cm8352_groupon_settings,
	.group_hold_on_conf_size = ARRAY_SIZE(ov5648_truly_cm8352_groupon_settings),
	.group_hold_off_conf = ov5648_truly_cm8352_groupoff_settings,
	.group_hold_off_conf_size =
		ARRAY_SIZE(ov5648_truly_cm8352_groupoff_settings),
	.init_settings = &ov5648_truly_cm8352_init_conf[0],
	.init_size = ARRAY_SIZE(ov5648_truly_cm8352_init_conf),
	.mode_settings = &ov5648_truly_cm8352_confs[0],
	.output_settings = &ov5648_truly_cm8352_dimensions[0],
	.num_conf = ARRAY_SIZE(ov5648_truly_cm8352_confs),
};

static struct msm_sensor_ctrl_t ov5648_truly_cm8352_s_ctrl = {
	.msm_sensor_reg = &ov5648_truly_cm8352_regs,
	.sensor_i2c_client = &ov5648_truly_cm8352_sensor_i2c_client,
	.sensor_i2c_addr =  0x36 << 1 ,
	.sensor_output_reg_addr = &ov5648_truly_cm8352_reg_addr,
	.sensor_id_info = &ov5648_truly_cm8352_id_info,
	.sensor_exp_gain_info = &ov5648_truly_cm8352_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov5648_truly_cm8352_csi_params_array[0],
	.msm_sensor_mutex = &ov5648_truly_cm8352_mut,
	.sensor_i2c_driver = &ov5648_truly_cm8352_i2c_driver,
	.sensor_v4l2_subdev_info = ov5648_truly_cm8352_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov5648_truly_cm8352_subdev_info),
	.sensor_v4l2_subdev_ops = &ov5648_truly_cm8352_subdev_ops,
	.func_tbl = &ov5648_truly_cm8352_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("Omnivision WXGA Bayer sensor driver");
MODULE_LICENSE("GPL v2");
