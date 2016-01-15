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
 
#include "msm_sensor.h"
#include "msm.h"
#include <linux/gpio.h>
#include <mach/gpio.h>

#ifdef TYQ_CAMERA_COMMON_DEBUG
#ifdef CDBG
#undef CDBG
#define CDBG printk
#endif
#endif

#define SENSOR_NAME "ov2675"
#define PLATFORM_DRIVER_NAME "msm_camera_ov2675"
#define ov2675_obj ov2675_##obj
#define MSB                             1
#define LSB                             0
#define ov2675_I2C_ADDR 0x60
#define ov2675_MODEL_I2C_ADDR (0x60>>1)

#ifdef CDBG
#undef CDBG
#endif
#ifdef CDBG_HIGH
#undef CDBG_HIGH
#endif

#define ov2675_VERBOSE_DGB

#ifdef ov2675_VERBOSE_DGB
#define CDBG(fmt, args...) printk(fmt, ##args)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#define CDBG_HIGH(fmt, args...) printk(fmt, ##args)
#endif

#define ov2675_MASTER_CLK_RATE             24000000
static unsigned int ov2675_preview_shutter;
static unsigned int ov2675_preview_gain16;
static unsigned short ov2675_preview_binning;
static unsigned int ov2675_preview_sysclk;
static unsigned int ov2675_preview_HTS;

static unsigned int ov2675_CAMERA_WB_AUTO = 1;
static uint16_t ov2675_preview_R_gain;
static uint16_t ov2675_preview_G_gain;
static uint16_t ov2675_preview_B_gain;

static struct msm_sensor_ctrl_t *ov2675_s_ctrl_p;

DEFINE_MUTEX(ov2675_mut);

static int cur_effect = MSM_V4L2_EFFECT_OFF;
static int cur_scene = CAMERA_BESTSHOT_MODE_OFF;
static int cur_exposure = MSM_V4L2_EXPOSURE_D;
static int cur_antibanding = MSM_V4L2_POWER_LINE_OFF;

 enum cam_res_mode{
     cam_snapshot=0,
	 cam_preview,
	 cam_zsl,
};
 
static struct msm_camera_i2c_reg_conf ov2675_start_settings[] = {
	{0x3086, 0x00},  /* streaming on */
};

static struct msm_camera_i2c_reg_conf ov2675_stop_settings[] = {
	{0x3086, 0x0f},  /* streaming off*/
};

static struct msm_sensor_ctrl_t ov2675_s_ctrl;

/*init settings for preview*///wang_gj modify
static struct msm_camera_i2c_reg_conf ov2675_init_settings[] = {
  {0x308c,0x80},
  {0x308d,0x0e},
  {0x360b,0x00},
  {0x30b0,0xff},
  {0x30b1,0xff},
  {0x30b2,0x04},
  {0x300e,0x34},
  {0x300f,0xa6},
  {0x3010,0x81},
  {0x3082,0x01},
  {0x30f4,0x01},
  {0x3090,0x43},
  {0x3091,0xc0},
  {0x30ac,0x42},
  {0x30d1,0x08},
  {0x30a8,0x54},
  {0x3015,0x02},
  {0x3093,0x00},
  {0x307e,0xe5},
  {0x3079,0x00},
  {0x30aa,0x52},
  {0x3017,0x40},
  {0x30f3,0x83},
  {0x306a,0x0c},
  {0x306d,0x00},
  {0x336a,0x3c},
  {0x3076,0x6a},
  {0x30d9,0x95},
  {0x3016,0x52},
  {0x3601,0x30},
  {0x304e,0x88},
  {0x30f1,0x82},
  {0x306f,0x14},
  {0x302a,0x02},
  {0x302b,0x6a},
  {0x3012,0x10},
  {0x3011,0x01},//15fps
  //;AEC/AGC
  {0x301a,0xd4},
  {0x301c,0x13},
  {0x301d,0x17},
  {0x3070,0x5d},
  {0x3072,0x4d},
  //;D5060
  {0x30af,0x00},
  {0x3048,0x1f},
  {0x3049,0x4e},
  {0x304a,0x20},
  {0x304f,0x20},
  {0x304b,0x02},
  {0x304c,0x00},
  {0x304d,0x02},
  {0x304f,0x20},
  {0x30a3,0x10},
  {0x3013,0xf7},
  {0x3014,0xe4},
  {0x3071,0x00},
  {0x3070,0x5d},
  {0x3073,0x00},
  {0x3072,0x4d},
  {0x301c,0x05},
  {0x301d,0x06},
  {0x304d,0x42},
  {0x304a,0x40},
  {0x304f,0x40},
  {0x3095,0x07},
  {0x3096,0x16},
  {0x3097,0x1d},
  //;Window Setup
  {0x3020,0x01},
  {0x3021,0x1a},//wukai modif at 20130130 0x19 Y channal dis-matched with UV channal
  {0x3022,0x00},
  {0x3023,0x06},
  {0x3024,0x06},
  {0x3025,0x58},
  {0x3026,0x02},
  {0x3027,0x61},
  {0x3088,0x02},
  {0x3089,0x80},
  {0x308a,0x01},
  {0x308b,0xe0},
  {0x3316,0x64},
  {0x3317,0x25},
  {0x3318,0x80},
  {0x3319,0x08},
  {0x331a,0x28},
  {0x331b,0x1e},
  {0x331c,0x00},
  {0x331d,0x38},
  {0x3100,0x00},
  //;AWB
  {0x3320,0xf8},
  {0x3321,0x11},
  {0x3322,0x92},
  {0x3323,0x01},
  {0x3324,0x97},
  {0x3325,0x02},
  {0x3326,0xff},
  {0x3327,0x14},
  {0x3328,0x10},
  {0x3329,0x12},
  {0x332a,0x58},
  {0x332b,0x57},
  {0x332c,0xac},
  {0x332d,0xb7},
  {0x332e,0x36},
  {0x332f,0x31},
  {0x3330,0x4d},
  {0x3331,0x42},
  {0x3332,0Xff},
  {0x3333,0x00},
  {0x3334,0xf0},
  {0x3335,0xf0},
  {0x3336,0xf0},
  {0x3337,0x40},
  {0x3338,0x40},
  {0x3339,0x40},
  {0x333a,0x00},
  {0x333b,0x00},
  //;Color Matrix
  {0x3380,0x27},
  {0x3381,0x4F},
  {0x3382,0x0A},
  {0x3383,0x2F},
  {0x3384,0xA3},
  {0x3385,0xD2},
  {0x3386,0x9C},
  {0x3387,0x8A},
  {0x3388,0x13},
  {0x3389,0x98},
  {0x338A,0x01},
  //;Gamma
  {0x3340,0x09},
  {0x3341,0x19},
  {0x3342,0x2f},
  {0x3343,0x45},
  {0x3344,0x5a},
  {0x3345,0x69},
  {0x3346,0x75},
  {0x3347,0x7e},
  {0x3348,0x88},
  {0x3349,0x96},
  {0x334a,0xa3},
  {0x334b,0xaf},
  {0x334c,0xc4},
  {0x334d,0xd7},
  {0x334e,0xe8},
  {0x334f,0x20},
  //;Lens correction
  {0x3350,0x30},
  {0x3351,0x25},
  {0x3352,0x96},
  {0x3353,0x61},
  {0x3354,0x1c},
  {0x3355,0x66},
  {0x3356,0x30},
  {0x3357,0x25},
  {0x3358,0xE8},
  {0x3359,0x4D},
  {0x335a,0x21},
  {0x335b,0x66},
  {0x335c,0x30},
  {0x335d,0x25},
  {0x335e,0xE0},
  {0x335f,0x49},
  {0x3360,0x18},
  {0x3361,0x66},
  {0x3363,0x70},
  {0x3364,0x7f},
  {0x3365,0x00},
  {0x3366,0x00},
  //;UV adjust
  {0x3301,0xff},
  {0x338b,0x1b},//1b wukai modify 2012-09-12
  {0x338c,0x1f},//10 wukai modify 2012-09-12
  {0x338d,0x40},
  //;Sharpness/De-noise
  {0x3370,0xd0},
  {0x3371,0x00},
  {0x3372,0x00},
  {0x3373,0x40},
  {0x3374,0x10},
  {0x3375,0x10},
  {0x3376,0x04},
  {0x3377,0x00},
  {0x3378,0x04},
  {0x3379,0x80},
  //;BLC
  {0x3069,0x86},
  {0x307c,0x10},
  {0x3087,0x02},
  //;blacksun
  //;Avdd 2.55~3.0V
  {0x3090,0x03},
  {0x30a8,0x54},
  {0x30aa,0x52},//82
  {0x30a3,0x80},//91
  {0x30a1,0x41},
  //;Other functions
  {0x3300,0xfc},
  {0x3302,0x11},
  {0x3400,0x00},
  {0x3606,0x20},
  {0x3601,0x30},
  {0x300e,0x34},
  {0x30f3,0x83},
  {0x304e,0x88},

  {0x3300,0xFC},
  {0x3350,0x30},
  {0x3351,0x25},
  {0x3352,0x96},
  {0x3353,0x59},
  {0x3354,0x19},
  {0x3355,0x66},
  //;G
  {0x3356,0x30},
  {0x3357,0x25},
  {0x3358,0xE8},
  {0x3359,0x46},
  {0x335A,0x1e},
  {0x335B,0x66},
  //;B
  {0x335C,0x30},
  {0x335D,0x25},
  {0x335E,0xE0},
  {0x335F,0x42},
  {0x3360,0x16},
  {0x3361,0x66},
  {0x307c,0x10},
  {0x3090,0x33},
  {0x3363,0x01},
  {0x3364,0x03},
  {0x3365,0x02},
  {0x3366,0x00},
  //gamma
  {0x334f,0x1d},
  {0x3340,0x6},
  {0x3341,0x14},
  {0x3342,0x26},
  {0x3343,0x40},
  {0x3344,0x55},
  {0x3345,0x63},
  {0x3346,0x6f},
  {0x3347,0x7c},
  {0x3348,0x88},
  {0x3349,0x99},
  {0x334a,0xa7},
  {0x334b,0xb3},
  {0x334c,0xc9},
  {0x334d,0xda},
  {0x334e,0xea},
  {0x3047,0x05},
  {0x3018,0x88},
  {0x3019,0x78},
  {0x301a,0xd4},
  {0x3015,0x02},
//wuchx 2012-3-31 added mirror/flip begin
  {0x307c,0x10},
  {0x3090,0x03},//08  wukai modify 2012-09-07
//wuchx 2012-3-31 added mirror/flip end
};

static struct msm_camera_i2c_reg_conf ov2675_prev_settings[] = {
  //pclk=18M
  //framerate:15fps
  //YUVVGA(640x480)
  {0x300e,0x3a},
  {0x3010,0x81},
  {0x3012,0x10},
  //{0x3015,0x02,1,0},
  {0x3016,0x82},
  {0x3023,0x06},
  {0x3026,0x02},
  {0x3027,0x5e},
  {0x302a,0x03},//0x02 wukai modif at 20130217 for antibanding
  {0x302b,0x10},//0x6a wukai modif at 20130217 for antibanding
  {0x330c,0x00},
  {0x3301,0xff},
  {0x3069,0x80},
  {0x306f,0x14},
  {0x3088,0x03},
  {0x3089,0x20},
  {0x308a,0x02},
  {0x308b,0x58},
  {0x308e,0x00},
  {0x30a1,0x41},
  {0x30a3,0x80},
  {0x30d9,0x95},
  {0x3302,0x11},
  {0x3317,0x25},
  {0x3318,0x80},
  {0x3319,0x08},
  {0x331d,0x38},
  {0x3373,0x30},
  {0x3376,0x08},//0x04
  {0x3362,0x90},
  //svga->vga
  {0x3302,0x11},
  {0x3088,0x02},
  {0x3089,0x80},
  {0x308a,0x01},
  {0x308b,0xe0},
  {0x331a,0x28},
  {0x331b,0x1e},
  {0x331c,0x00},
  {0x3302,0x11},
  //mipi
  {0x363b,0x01},
  {0x309e,0x08},
  {0x3606,0x00},
  {0x3630,0x35},
  {0x3086,0x0f},
  {0x3086,0x00},
  {0x304e,0x04},
  {0x363b,0x01},
  {0x309e,0x08},
  {0x3606,0x00},
  {0x3084,0x01},
  {0x3010,0x81},
  {0x3011,0x00},
  {0x300e,0x3a},
  {0x3634,0x26},
  {0x3086,0x0f},
  {0x3086,0x00},
  //avoid black screen slash
  {0x3000,0x15},
  {0x3002,0x02},
  {0x3003,0x28},
  //;blacksun wukai add 2012-09-07 add
  //;Avdd 2.55~3.0V
  {0x3090,0x03},
  {0x30aa,0x52},
  {0x30a3,0x80},
  {0x30a1,0x41},
  {0x3013,0xf7},//wukai add 2012-09-12
};

static struct msm_camera_i2c_reg_conf ov2675_snap_settings[] = {
  //pclk=24M
  //framerate:7.5ps
  {0x300e,0x34},
  {0x3011,0x01},
  {0x3010,0x81},
  {0x3012,0x00},
  //{0x3015,0x02,1,0},
  {0x3016,0xc2},
  {0x3023,0x0c},
  {0x3026,0x04},
  {0x3027,0xbc},
  {0x302a,0x04},
  {0x302b,0xd4},
  {0x3069,0x80},
  {0x306f,0x54},
  {0x3088,0x06},
  {0x3089,0x40},
  {0x308a,0x04},
  {0x308b,0xb0},
  {0x308e,0x64},
  {0x30a1,0x41},
  {0x30a3,0x80},
  {0x30d9,0x95},
  {0x3302,0x01},
  {0x3317,0x4b},
  {0x3318,0x00},
  {0x3319,0x4c},
  {0x331d,0x6c},
  {0x3362,0x80},
  {0x3373,0x40},
  {0x3376,0x03},//0x08
  //;blacksun wukai add 2012-09-07 add
  //;Avdd 2.55~3.0V
  {0x3090,0x03},
  {0x30aa,0x52},
  {0x30a3,0x80},
  {0x30a1,0x41},
};

static struct msm_camera_i2c_reg_conf ov2675_zsl_settings[] = {
  //pclk=24M
  //framerate:15ps
  
  {0x300e,0x3a},
  {0x3010,0x81},
  {0x3012,0x10},
  //{0x3015,0x02,1,0},
  {0x3016,0x82},
  {0x3023,0x06},
  {0x3026,0x02},
  {0x3027,0x5e},
  {0x302a,0x03},//0x02 wukai modif at 20130217 for antibanding
  {0x302b,0x10},//0x6a wukai modif at 20130217 for antibanding
  {0x330c,0x00},
  {0x3301,0xff},
  {0x3069,0x80},
  {0x306f,0x14},
  {0x3088,0x03},
  {0x3089,0x20},
  {0x308a,0x02},
  {0x308b,0x58},
  {0x308e,0x00},
  {0x30a1,0x41},
  {0x30a3,0x80},
  {0x30d9,0x95},
  {0x3302,0x11},
  {0x3317,0x25},
  {0x3318,0x80},
  {0x3319,0x08},
  {0x331d,0x38},
  {0x3373,0x30},
  {0x3376,0x08},//0x04
  {0x3362,0x90},
  //svga->vga
  {0x3302,0x11},
  {0x3088,0x02},
  {0x3089,0x80},
  {0x308a,0x01},
  {0x308b,0xe0},
  {0x331a,0x28},
  {0x331b,0x1e},
  {0x331c,0x00},
  {0x3302,0x11},
  //mipi
  {0x363b,0x01},
  {0x309e,0x08},
  {0x3606,0x00},
  {0x3630,0x35},
  {0x3086,0x0f},
  {0x3086,0x00},
  {0x304e,0x04},
  {0x363b,0x01},
  {0x309e,0x08},
  {0x3606,0x00},
  {0x3084,0x01},
  {0x3010,0x81},
  {0x3011,0x00},
  {0x300e,0x3a},
  {0x3634,0x26},
  {0x3086,0x0f},
  {0x3086,0x00},
  //avoid black screen slash
  {0x3000,0x15},
  {0x3002,0x02},
  {0x3003,0x28},
  //;blacksun wukai add 2012-09-07 add
  //;Avdd 2.55~3.0V
  {0x3090,0x03},
  {0x30aa,0x52},
  {0x30a3,0x80},
  {0x30a1,0x41},
  {0x3013,0xf7},//wukai add 2012-09-12
  {0x300e,0x34},
  {0x3011,0x00},
  {0x3010,0x80},
  {0x3012,0x00},
  {0x3015,0x03},
  {0x3016,0xc2},
  {0x3023,0x0c},
  {0x3026,0x04},
  {0x3027,0xbc},
  {0x302a,0x04},
  {0x302b,0xd4},
  {0x3069,0x80},
  {0x306f,0x54},
  {0x3088,0x06},
  {0x3089,0x40},
  {0x308a,0x04},
  {0x308b,0xb0},
  {0x308e,0x64},
  {0x30a1,0x41},
  {0x30a3,0x80},
  {0x30d9,0x95},
  {0x3302,0x01},
  {0x3317,0x4b},
  {0x3318,0x00},
  {0x3319,0x4c},
  {0x331d,0x6c},
  {0x3362,0x80},
  {0x3373,0x40},
  {0x3376,0x03},//0x08
  //;blacksun wukai add 2012-09-07 add
  //;Avdd 2.55~3.0V
  {0x3090,0x03},
  {0x30aa,0x52},
  {0x30a3,0x80},
  {0x30a1,0x41},
  {0x3013,0xf7},//wukai add 2012-09-12
};

/*tydrv qupw add for fcam detect*/
#ifdef TYQ_FCAM_OV2675_DETECT
static struct msm_camera_i2c_reg_conf ov2675_truly_lens_settings[] = {
	{0x3300,0xFC}, 
	{0x3350,0x2e},//30 
	{0x3351,0x26},//25
	{0x3352,0x72},//96
	{0x3353,0x94},//99
	{0x3354,0x19},
	{0x3355,0x66},
	{0x3356,0x2e},//30
	{0x3357,0x26},//25
	{0x3358,0x8c},//E8
	{0x3359,0x7e},//86
	{0x335A,0x1e},
	{0x335B,0x66},
	{0x335C,0x2d},//30
	{0x335D,0x26},//25
	{0x335E,0x84},//e0
	{0x335F,0x76},//82
	{0x3360,0x16}, 
	{0x3361,0x66}, 
	{0x307c,0x10}, 
	{0x3090,0x33}, 
	{0x3363,0x01}, 
	{0x3364,0x03}, 
	{0x3365,0x02}, 
	{0x3366,0x00},

};
#endif
static struct v4l2_subdev_info ov2675_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_YUYV8_2X8,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
	/* more can be supported, to be added later */
};

/* add delay information to the init settings */
static struct msm_camera_i2c_conf_array ov2675_init_conf[] = {
	{&ov2675_init_settings[0],
	ARRAY_SIZE(ov2675_init_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};	

static struct msm_camera_i2c_conf_array ov2675_confs[] = {
	{&ov2675_snap_settings[0],
	ARRAY_SIZE(ov2675_snap_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov2675_prev_settings[0],
	ARRAY_SIZE(ov2675_prev_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
	{&ov2675_zsl_settings[0],
	ARRAY_SIZE(ov2675_zsl_settings), 0, MSM_CAMERA_I2C_BYTE_DATA},
};

static struct msm_sensor_output_info_t ov2675_dimensions[] = {
	//cam_snapshot =0
	{
		.x_output = 0x0640,
		.y_output = 0x04b0,
		.line_length_pclk = 0x793,
		.frame_length_lines = 0x04d4,
		.vt_pixel_clk = 0x793 * 0x04d4* 15,
		.op_pixel_clk = 0x793 * 0x04d4 * 30,
		.binning_factor = 0,
	},
	//cam_preview =1
	{
		.x_output = 0x0280,
		.y_output = 0x01e0,
		.line_length_pclk = 0x793,
		.frame_length_lines = 0x26a,
		.vt_pixel_clk = 0x793 * 0x26a* 30,
		.op_pixel_clk = 0x793 * 0x26a * 30,
		.binning_factor = 0,
	},
	//cam_zsl =2
	{
		.x_output = 0x0640,
		.y_output = 0x04b0,
		.line_length_pclk = 0x793,
		.frame_length_lines = 0x04d4,
		.vt_pixel_clk = 0x793 * 0x04d4* 15,
		.op_pixel_clk = 0x793 * 0x04d4 * 30,
		.binning_factor = 0,
	},
};
static struct msm_sensor_output_reg_addr_t ov2675_reg_addr = {
	.x_output = 0x3088,
	.y_output = 0x308a,
	.line_length_pclk = 0x3028,
	.frame_length_lines = 0x302A,
};

static struct msm_sensor_id_info_t ov2675_id_info = {
	.sensor_id_reg_addr = 0x300a,
	.sensor_id = 0x2656,
};

//Effect Settings
static struct msm_camera_i2c_reg_conf ov2675_no_effect[] = {};

static struct msm_camera_i2c_conf_array ov2675_no_effect_confs[] = {
	{&ov2675_no_effect[0], ARRAY_SIZE(ov2675_no_effect), 30, MSM_CAMERA_I2C_BYTE_DATA},
};
/////////bestshot scene//////////////////////////////////////////////////////////////////
static struct msm_camera_i2c_reg_conf ov2675_special_scene[][4] = {
	{/*  CAMERA_BESTSHOT_OFF */
	{0x3014, 0x84},
	{0x3015, 0x22},
	{0x302e, 0x00},
	{0x302d, 0x00},
	},
	{/* CAMERA_BESTSHOT_ACTION*/
	{0x3014, 0x84},
	{0x3015, 0x21},
	{0x302e, 0x00},
	{0x302d, 0x00},
	},
	{/* CAMERA_BESTSHOT_NIGHT*/
	{0x3014, 0x8c},
	{0x3015, 0x24},
	//{0x302e, 0x00},
	//{0x302d, 0x00},
	},
};

static struct msm_camera_i2c_conf_array ov2675_special_scene_confs[][1] = {
	{{ov2675_special_scene[0],  ARRAY_SIZE(ov2675_special_scene[0]),
		5, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_special_scene[1],  ARRAY_SIZE(ov2675_special_scene[1]),
		5, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_special_scene[2],  ARRAY_SIZE(ov2675_special_scene[2]),
		5, MSM_CAMERA_I2C_BYTE_DATA},},
};



static int ov2675_special_scene_enum_map[] = {
	CAMERA_BESTSHOT_MODE_OFF,
	CAMERA_BESTSHOT_MODE_ACTION,
	CAMERA_BESTSHOT_MODE_NIGHT,
};

static struct msm_camera_i2c_enum_conf_array
		 ov2675_special_scene_enum_confs = {
	.conf = &ov2675_special_scene_confs[0][0],
	.conf_enum = ov2675_special_scene_enum_map,
	.num_enum = ARRAY_SIZE(ov2675_special_scene_enum_map),
	.num_index = ARRAY_SIZE(ov2675_special_scene_confs),
	.num_conf = ARRAY_SIZE(ov2675_special_scene_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
int ov2675_scene_msm_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	//CDBG("--CAMERA-- %s IN, value =%d,cur=%d\n", __func__, value,cur_scene);	
	switch(value)
	{
		case 0:/*  CAMERA_BESTSHOT_OFF */
			value = 0;break;
		case 17:/* CAMERA_BESTSHOT_ACTION*/
			value = 1;break;
		case 6:/* CAMERA_BESTSHOT_NIGHT*/
			value = 2;break;
		default:
			value = 0;break;
	}
	
	if(value == cur_scene)
		return rc;
	//CDBG("--CAMERA-- %s SWITCH, value:%d\n", __func__, value);
	rc = msm_sensor_write_enum_conf_array(
			s_ctrl->sensor_i2c_client,
			ctrl_info->enum_cfg_settings, value);
	if (rc < 0) {
		CDBG("write faield\n");
		return rc;
	}
	cur_scene = value;	
       CDBG("--CAMERA-- %s END, value =%d,cur=%d\n", __func__, value,cur_scene);
	return rc;
}
/////////antibanding//////////////////////////////////////////////////////////////////
static struct msm_camera_i2c_reg_conf ov2675_antibanding[][1] = {
	{//OFF
	{0x3014,0xe4}, // MCU_ADDRESS [FD_MODE]
	},
	{//60HZ
	{0x3014,0x04}, // MCU_ADDRESS [FD_MODE]
	},
	{//50HZ
	{0x3014,0x84}, // MCU_ADDRESS [FD_MODE]
	},
};

static struct msm_camera_i2c_conf_array ov2675_antibanding_confs[][1] = {
	{{ov2675_antibanding[0], ARRAY_SIZE(ov2675_antibanding[0]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_antibanding[1], ARRAY_SIZE(ov2675_antibanding[1]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_antibanding[2], ARRAY_SIZE(ov2675_antibanding[2]),  0,
		MSM_CAMERA_I2C_BYTE_DATA},},
};

static int ov2675_antibanding_enum_map[] = {
	MSM_V4L2_POWER_LINE_OFF,
	MSM_V4L2_POWER_LINE_60HZ,
	MSM_V4L2_POWER_LINE_50HZ,
};


static struct msm_camera_i2c_enum_conf_array ov2675_antibanding_enum_confs = {
	.conf = &ov2675_antibanding_confs[0][0],
	.conf_enum = ov2675_antibanding_enum_map,
	.num_enum = ARRAY_SIZE(ov2675_antibanding_enum_map),
	.num_index = ARRAY_SIZE(ov2675_antibanding_confs),
	.num_conf = ARRAY_SIZE(ov2675_antibanding_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
int ov2675_antibanding_msm_sensor_s_ctrl_by_enum(
		struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	//CDBG("--CAMERA-- %s IN, value:%d\n", __func__, value);
	if(value == cur_antibanding)
		return rc;
	
	rc = msm_sensor_write_enum_conf_array(
		s_ctrl->sensor_i2c_client,
		ctrl_info->enum_cfg_settings, value);
	if (rc < 0) {
		CDBG("write faield\n");
		return rc;
	}

	cur_antibanding = value;	
	CDBG("--CAMERA-- %s END, value:%d\n", __func__, value);
	return rc;
}
/////////exposure//////////////////////////////////////////////////////////////////
static struct msm_camera_i2c_reg_conf ov2675_exposure[][3] = {
	{/*EXPOSURECOMPENSATIONP2*/
		//Exposure Compensation -1.7EV
		{0x3018,0x40},
  		{0x3019,0x30},
  		{0x301a,0x71},
  		//{0x3018,0x5a},
  		//{0x3019,0x4a},
  		//{0x301a,0xc2},
	},
	{/*EXPOSURECOMPENSATIONP1*/
		//Exposure Compensation -1.0EV
		{0x3018,0x6a},
		{0x3019,0x5a},
		{0x301a,0xd4},
	},	
	{/*EXPOSURECOMPENSATIOND*/
		//Exposure Compensation default
		{0x3018,0x78},
		{0x3019,0x68},
		//{0x301a,0xa5},
		{0x301a,0xd4},
	},
	{/*EXPOSURECOMPENSATIONN1*/
		//Exposure Compensation +1.0EV
		{0x3018,0x88},
		{0x3019,0x78},
		{0x301a,0xd4},
	},
	{/*EXPOSURECOMPENSATIONN2*/
		//Exposure Compensation +1.7EV
		{0x3018,0xc8},
		{0x3019,0xb8},
		{0x301a,0xf7},
		//{0x3018,0x98},
		//{0x3019,0x88},
		//{0x301a,0xd4},
	},
};

static struct msm_camera_i2c_conf_array ov2675_exposure_confs[][1] = {
	{{ov2675_exposure[0], ARRAY_SIZE(ov2675_exposure[0]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_exposure[1], ARRAY_SIZE(ov2675_exposure[1]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_exposure[2], ARRAY_SIZE(ov2675_exposure[2]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_exposure[3], ARRAY_SIZE(ov2675_exposure[3]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_exposure[4], ARRAY_SIZE(ov2675_exposure[4]), 0, MSM_CAMERA_I2C_BYTE_DATA},},
};

static int ov2675_exposure_enum_map[] = {
	MSM_V4L2_EXPOSURE_N2,
	MSM_V4L2_EXPOSURE_N1,
	MSM_V4L2_EXPOSURE_D, /* default vaule */
	MSM_V4L2_EXPOSURE_P1,
	MSM_V4L2_EXPOSURE_P2,
};

static struct msm_camera_i2c_enum_conf_array ov2675_exposure_enum_confs = {
	.conf = &ov2675_exposure_confs[0][0],
	.conf_enum = ov2675_exposure_enum_map,
	.num_enum = ARRAY_SIZE(ov2675_exposure_enum_map),
	.num_index = ARRAY_SIZE(ov2675_exposure_confs),
	.num_conf = ARRAY_SIZE(ov2675_exposure_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
int ov2675_exposure_msm_sensor_s_ctrl_by_enum(
		struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	//CDBG("--CAMERA-- %s IN, exposure:%d\n", __func__, value);

	if(value == cur_exposure)
		return rc;
	
	rc = msm_sensor_write_enum_conf_array(
		s_ctrl->sensor_i2c_client,
		ctrl_info->enum_cfg_settings, value);
	if (rc < 0) {
		CDBG("write faield\n");
		return rc;
	}

	cur_exposure= value;	
	CDBG("--CAMERA-- %s END, value:%d\n", __func__, value);
	return rc;
}
/////////special effect//////////////////////////////////////////////////////////////////
static struct msm_camera_i2c_reg_conf ov2675_special_effect[][4] = {
	{/* CAMERA_EFFECT_OFF  */
	{0x3391,0x00},
	},
	{/* CAMERA_EFFECT_MONO */
	{0x3391,0x20},
	},
	{/* CAMERA_EFFECT_Negative */
	{0x3391,0x40},
	},
	{/*Solarize is not supported by sensor*/
	//{-1,-1},
	},
	{/* CAMERA_EFFECT_SEPIA */
	{0x3391,0x18},
	{0x3396,0x40},
	{0x3397,0xa6},
	},
};

static struct msm_camera_i2c_conf_array ov2675_special_effect_confs[][1] = {
	{{ov2675_special_effect[0],  ARRAY_SIZE(ov2675_special_effect[0]),
		5, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_special_effect[1],  ARRAY_SIZE(ov2675_special_effect[1]),
		5, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_special_effect[2],  ARRAY_SIZE(ov2675_special_effect[2]),
		5, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_special_effect[3],  ARRAY_SIZE(ov2675_special_effect[3]),
		5, MSM_CAMERA_I2C_BYTE_DATA},},
	{{ov2675_special_effect[4],  ARRAY_SIZE(ov2675_special_effect[4]),
		5, MSM_CAMERA_I2C_BYTE_DATA},},
};

static int ov2675_special_effect_enum_map[] = {
	CAMERA_EFFECT_OFF,
	CAMERA_EFFECT_MONO,
        CAMERA_EFFECT_NEGATIVE,
        CAMERA_EFFECT_SOLARIZE,
	CAMERA_EFFECT_SEPIA,

};

static struct msm_camera_i2c_enum_conf_array
		 ov2675_special_effect_enum_confs = {
	.conf = &ov2675_special_effect_confs[0][0],
	.conf_enum = ov2675_special_effect_enum_map,
	.num_enum = ARRAY_SIZE(ov2675_special_effect_enum_map),
	.num_index = ARRAY_SIZE(ov2675_special_effect_confs),
	.num_conf = ARRAY_SIZE(ov2675_special_effect_confs[0]),
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
};
int ov2675_effect_msm_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	//CDBG("--CAMERA-- %s IN, effect:%d\n", __func__, value);

	if(value == cur_effect)
		return rc;

	//CDBG("--CAMERA-- %s SWITCH, effect:%d\n", __func__, value);
	rc = msm_sensor_write_enum_conf_array(
			s_ctrl->sensor_i2c_client,
			ctrl_info->enum_cfg_settings, value);
	if (rc < 0) {
		CDBG("write faield\n");
		return rc;
	}
	cur_effect= value;	
       //CDBG("--CAMERA-- %s END, value:%d\n", __func__, value);
	return rc;
}

int ov2675_msm_sensor_s_ctrl_by_enum(struct msm_sensor_ctrl_t *s_ctrl,
		struct msm_sensor_v4l2_ctrl_info_t *ctrl_info, int value)
{
	int rc = 0;
	CDBG("--CAMERA-- %s IN, value:%d\n", __func__, value);
	rc = msm_sensor_write_enum_conf_array(
		s_ctrl->sensor_i2c_client,
		ctrl_info->enum_cfg_settings, value);
	if (rc < 0) {
		CDBG("write faield\n");
		return rc;
	}
	return rc;
}

struct msm_sensor_v4l2_ctrl_info_t ov2675_v4l2_ctrl_info[] = {
	{
		.ctrl_id = V4L2_CID_EXPOSURE,
		.min = MSM_V4L2_EXPOSURE_N2,
		.max = MSM_V4L2_EXPOSURE_P2,
		.step = 1,
		.enum_cfg_settings = &ov2675_exposure_enum_confs,
		.s_v4l2_ctrl = ov2675_exposure_msm_sensor_s_ctrl_by_enum,
	},
	{
		/*WE use V4L2_CID_SHARPNESS stand for bestshot scene mode*/
		.ctrl_id = V4L2_CID_SHARPNESS,
		.min = CAMERA_BESTSHOT_MODE_OFF,
		.max = CAMERA_BESTSHOT_MODE_NIGHT,
		.step = 1,
		.enum_cfg_settings = &ov2675_special_scene_enum_confs,
		.s_v4l2_ctrl = ov2675_scene_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_SPECIAL_EFFECT,
		.min = MSM_V4L2_EFFECT_OFF,
		.max = MSM_V4L2_EFFECT_SEPIA,
		.step = 1,
		.enum_cfg_settings = &ov2675_special_effect_enum_confs,
		.s_v4l2_ctrl = ov2675_effect_msm_sensor_s_ctrl_by_enum,
	},
	{
		.ctrl_id = V4L2_CID_POWER_LINE_FREQUENCY,
		.min = MSM_V4L2_POWER_LINE_OFF,
		.max = MSM_V4L2_POWER_LINE_50HZ,
		.step = 1,
		.enum_cfg_settings = &ov2675_antibanding_enum_confs,
		.s_v4l2_ctrl = ov2675_antibanding_msm_sensor_s_ctrl_by_enum,
	},
};

static struct msm_camera_csi_params ov2675_csi_params = {
	.data_format = CSI_8BIT,
	.lane_cnt    = 1,
	.lane_assign = 0xe4,
	.dpcm_scheme = 0,
	.settle_cnt  = 20,
};

static struct msm_camera_csi_params *ov2675_csi_params_array[] = {
	&ov2675_csi_params,  //cam_snapshot
	&ov2675_csi_params, //cam_preview
	&ov2675_csi_params, //cam_zsl
};


static struct msm_sensor_exp_gain_info_t ov2675_exp_gain_info = {
	.coarse_int_time_addr = 0x004f,
	.global_gain_addr = 0x0012,
	.vert_offset = 4,
};


static const struct i2c_device_id ov2675_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&ov2675_s_ctrl},
	{ }
};

int32_t ov2675_sensor_i2c_probe(struct i2c_client *client,
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

	usleep_range(5000, 5100);

	return rc;

}

static struct i2c_driver ov2675_i2c_driver = {
	.id_table = ov2675_i2c_id,
	.probe  = ov2675_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov2675_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};
/*tydrv qupw add for fcam detect*/
#ifdef TYQ_FCAM_OV2675_DETECT
static int fcam_id_flag;
#define FCAM_ID_GPIO 126

static int tyq_read_fcam_id(unsigned int gpio)
{	
	int rc = 0;

	rc = gpio_tlmm_config(GPIO_CFG(gpio, 0, GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),GPIO_CFG_ENABLE);
	if(rc){
		printk(KERN_ERR "tyq_read_fcam_id config failed\n");
		return -3;
	}
	
	rc = gpio_request(gpio, "fcam_id");
	if(rc){
		printk(KERN_ERR "%s: Failed to request GPIO %d\n",
		       __func__, gpio);
		gpio_free(gpio);
	}else{
		rc = gpio_direction_input(gpio);
		if (!rc){
			rc = gpio_get_value(gpio);
			printk("qupw tyq_fcam_read_id----->>>%d \n",rc);
		}
		gpio_free(gpio);
	}

	return rc;
}

static void tyq_fcam_lens_adjust(void)
{
  int index;
  
  if (fcam_id_flag ==1)  //xinli
  	{
      for (index=0;index< ARRAY_SIZE(ov2675_truly_lens_settings);index++)  
	  	 msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client,ov2675_truly_lens_settings[index].reg_addr, ov2675_truly_lens_settings[index].reg_data,MSM_CAMERA_I2C_BYTE_DATA);
    }

}
#endif
static int __init msm_sensor_init_module(void)
{
	int rc = 0;
	CDBG("%s: %d\n", __func__, __LINE__);

	rc = i2c_add_driver(&ov2675_i2c_driver);

/*tydrv qupw add for fcamid   0: shunyu  1:xinli*/
#ifdef TYQ_FCAM_OV2675_DETECT
	fcam_id_flag = tyq_read_fcam_id(FCAM_ID_GPIO);
#endif
	return rc;
}

static struct v4l2_subdev_core_ops ov2675_subdev_core_ops = {
	.s_ctrl = msm_sensor_v4l2_s_ctrl,
	.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops ov2675_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops ov2675_subdev_ops = {
	.core = &ov2675_subdev_core_ops,
	.video  = &ov2675_subdev_video_ops,
};

static int32_t ov2675_write_exp_gain(struct msm_sensor_ctrl_t *s_ctrl,
		uint16_t gain, uint32_t line)
{
	//CDBG("ov2675_write_exp_gain : Not supported\n");
	return 0;
}

int32_t ov2675_sensor_set_fps(struct msm_sensor_ctrl_t *s_ctrl,
		struct fps_cfg *fps)
{
	//CDBG("ov2675_sensor_set_fps: Not supported\n");
	return 0;
}



/******************************************************************************

******************************************************************************/
static unsigned int ov2675_get_shutter(void)
{
  // read shutter, in number of line period
  unsigned int shutter = 0, extra_line = 0;
  uint16_t ret_l,ret_h;
  ret_l = ret_h = 0;

  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3002, &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3003, &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
 
 shutter = (ret_h << 8) | (ret_l & 0xff) ;
  ret_l = ret_h = 0;
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x302d, &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x302e, &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
  extra_line = (ret_h << 8) | (ret_l & 0xff) ;
  return shutter + extra_line;
}
/******************************************************************************
+
+******************************************************************************/
static int ov2675_set_shutter(unsigned int shutter)
{
  // write shutter, in number of line period
  int rc = 0;
  uint16_t temp;
  shutter = shutter & 0xffff;
  temp = shutter & 0xff;
  msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3003, temp,MSM_CAMERA_I2C_BYTE_DATA);
  temp = shutter >> 8;
  msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3002, temp,MSM_CAMERA_I2C_BYTE_DATA);
   return rc;
 }

/******************************************************************************

******************************************************************************/
static unsigned int ov2675_get_gain16(void)
{
  unsigned int gain16;
  uint16_t temp;

  temp = 0;
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3000, &temp,MSM_CAMERA_I2C_BYTE_DATA);
  CDBG("%s:Reg(0x3000) = 0x%x\n",__func__,temp);
  gain16 = ((temp>>4) + 1) * (16 + (temp & 0x0f));
  return gain16;
}

/******************************************************************************
+
+******************************************************************************/
static int ov2675_set_gain16(unsigned int gain16)
 {
   int rc = 0;
  uint16_t reg;
  gain16 = gain16 & 0x1ff;	// max gain is 32x
  reg = 0;
  if (gain16 > 32){
    gain16 = gain16 /2;
    reg = 0x10;
   }

  if (gain16 > 32){
    gain16 = gain16 /2;
    reg = reg | 0x20;
   }

  if (gain16 > 32){
    gain16 = gain16 /2;
    reg = reg | 0x40;
   }

  if (gain16 > 32){
    gain16 = gain16 /2;
    reg = reg | 0x80;
  }
  reg = reg | (gain16 -16);
  rc = msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client,0x3000,reg + 1,MSM_CAMERA_I2C_BYTE_DATA);
  msleep(100);
  rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client,0x3000,reg,MSM_CAMERA_I2C_BYTE_DATA);
  return rc;
}
/******************************************************************************

******************************************************************************/
/******************************************************************************
+
+******************************************************************************/
static unsigned int ov2675_get_sysclk(void)
{
  // calculate sysclk
  unsigned int  XVCLK;
  uint16_t temp1, temp2;
  unsigned int Indiv2x, Bit8Div, FreqDiv2x, PllDiv, SensorDiv, ScaleDiv, DvpDiv, ClkDiv, VCO, sysclk;
  unsigned int Indiv2x_map[] = { 2, 3, 4, 6, 4, 6, 8, 12};
  unsigned int Bit8Div_map[] = { 1, 1, 4, 5};
  unsigned int FreqDiv2x_map[] = { 2, 3, 4, 6};
  unsigned int DvpDiv_map[] = { 1, 2, 8, 16};
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x300e, &temp1,MSM_CAMERA_I2C_BYTE_DATA);
  // bit[5:0] PllDiv
  PllDiv = 64 - (temp1 & 0x3f);
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x300f, &temp1,MSM_CAMERA_I2C_BYTE_DATA);
  // bit[2:0] Indiv
  temp2 = temp1 & 0x07;
 Indiv2x = Indiv2x_map[temp2];
  // bit[5:4] Bit8Div
  temp2 = (temp1 >> 4) & 0x03;
  Bit8Div = Bit8Div_map[temp2];
  // bit[7:6] FreqDiv
  temp2 = temp1 >> 6;
  FreqDiv2x = FreqDiv2x_map[temp2];
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3010, &temp1,MSM_CAMERA_I2C_BYTE_DATA);
  //bit[3:0] ScaleDiv
  temp2 = temp1 & 0x0f;
  if(temp2==0) {
    ScaleDiv = 1;
  } else {
    ScaleDiv = temp2 * 2;
  }
  // bit[4] SensorDiv
  if(temp1 & 0x10) {
    SensorDiv = 2;
  } else {
    SensorDiv = 1;
  }
  // bit[5] LaneDiv
  // bit[7:6] DvpDiv
  temp2 = temp1 >> 6;
  DvpDiv = DvpDiv_map[temp2];
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3011, &temp1,MSM_CAMERA_I2C_BYTE_DATA);
  // bit[5:0] ClkDiv
  temp2 = temp1 & 0x3f;
  ClkDiv = temp2 + 1;
  XVCLK = ov2675_MASTER_CLK_RATE/10000;
  CDBG("%s:XVCLK = 0x%x\n",__func__,XVCLK);
  CDBG("%s:Bit8Div = 0x%x\n",__func__,Bit8Div);
  CDBG("%s:FreqDiv2x = 0x%x\n",__func__,FreqDiv2x);
  CDBG("%s:PllDiv = 0x%x\n",__func__,PllDiv);
  CDBG("%s:Indiv2x = 0x%x\n",__func__,Indiv2x);
  VCO = XVCLK * Bit8Div * FreqDiv2x * PllDiv / Indiv2x ;
  sysclk = VCO / Bit8Div / SensorDiv / ClkDiv / 4;
  CDBG("%s:ClkDiv = 0x%x\n",__func__,ClkDiv);
  CDBG("%s:SensorDiv = 0x%x\n",__func__,SensorDiv);
  CDBG("%s:sysclk = 0x%x\n",__func__,sysclk);
  return sysclk;
}

/******************************************************************************
+
+******************************************************************************/
static unsigned int ov2675_get_HTS(void)
{
  // read HTS from register settings
  unsigned int HTS, extra_HTS;
  uint16_t ret_l,ret_h;
  ret_l = ret_h = 0;
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3028, &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3029, &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
  HTS = (ret_h << 8) | (ret_l & 0xff) ;
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x302c, &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
  extra_HTS = ret_l;
  return HTS + extra_HTS;
}

/******************************************************************************
+
+******************************************************************************/
static unsigned int ov2675_get_VTS(void)
{
  // read VTS from register settings
  unsigned int VTS, extra_VTS;
  uint16_t ret_l,ret_h;
  ret_l = ret_h = 0;
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x302a, &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x302b, &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
  VTS = (ret_h << 8) | (ret_l & 0xff) ;
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x302d, &ret_h,MSM_CAMERA_I2C_BYTE_DATA);
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x302e, &ret_l,MSM_CAMERA_I2C_BYTE_DATA);
  extra_VTS = (ret_h << 8) | (ret_l & 0xff) ;
  return VTS + extra_VTS;
}

/******************************************************************************
+
+******************************************************************************/
static int ov2675_set_VTS(unsigned int VTS)
{
  // write VTS to registers
  int rc = 0;
  uint16_t temp;
  temp = VTS & 0xff;
  rc = msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x302b, temp,MSM_CAMERA_I2C_BYTE_DATA);
  temp = VTS>>8;
  rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x302a, temp,MSM_CAMERA_I2C_BYTE_DATA);
  return rc;
}

/******************************************************************************
+
+******************************************************************************/
static unsigned int ov2675_get_binning(void)
{
  // write VTS to registers
  unsigned int  binning;
  uint16_t temp;
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x300b, &temp,MSM_CAMERA_I2C_BYTE_DATA);
  if(temp==0x52){
    // OV2650
    binning = 2;
  } else {
    // ov2675
    binning = 1;
  }
  return binning;
}

/******************************************************************************
+
+******************************************************************************/
static unsigned int ov2675_get_light_frequency(void)
{
  // get banding filter value
  unsigned int  light_frequency;
  uint16_t temp;
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3014, &temp,MSM_CAMERA_I2C_BYTE_DATA);
  if (temp & 0x40) {
    // auto
    msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x508e, &temp,MSM_CAMERA_I2C_BYTE_DATA);
    if (temp & 0x01){
      light_frequency = 50;
    } else {
      light_frequency = 60;
    }
  } else {
    // manual
    if (temp & 0x80){
      // 50Hz
      light_frequency = 50;
    } else {
      // 60Hz
      light_frequency = 60;
     }
   }

  return light_frequency;
}

/******************************************************************************
+
+******************************************************************************/
static int ov2675_dummyline = 0;
static int ov2675_get_dummyline(void)
{
// read shutter, in number of line period
int dummyline;
uint16_t temp;
dummyline = msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x302d, &temp,MSM_CAMERA_I2C_BYTE_DATA);

//ov2675_read_i2c(0x302d) ;
dummyline = (dummyline<<8) + msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x302e, &temp,MSM_CAMERA_I2C_BYTE_DATA);
//ov2675_read_i2c(0x302e);
return dummyline;
}
/******************************************************************************
+
+******************************************************************************/
static int ov2675_set_bandingfilter(void)
{
  int rc = 0;
  unsigned int preview_VTS;
  //int dummyline = 0;
  uint16_t band_step60, max_band60, band_step50, max_band50;
  // read preview PCLK
  ov2675_preview_sysclk = ov2675_get_sysclk();
  // read preview HTS
  ov2675_preview_HTS = ov2675_get_HTS();
  // read preview VTS
  preview_VTS = ov2675_get_VTS();
  // calculate banding filter
  CDBG("%s:ov2675_preview_sysclk = 0x%x\n",__func__,ov2675_preview_sysclk);
  CDBG("%s:ov2675_preview_HTS = 0x%x\n",__func__,ov2675_preview_HTS);
  CDBG("%s:preview_VTS = 0x%x\n",__func__,preview_VTS);
  // 60Hz
  band_step60 = ov2675_preview_sysclk * 100/ov2675_preview_HTS * 100/120;
  rc = msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3073, (band_step60 >> 8),MSM_CAMERA_I2C_BYTE_DATA);
  rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3072, (band_step60 & 0xff),MSM_CAMERA_I2C_BYTE_DATA);
  max_band60 = ((preview_VTS-4)/band_step60);
  rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x301d, max_band60-1,MSM_CAMERA_I2C_BYTE_DATA);
  // 50Hz
  CDBG("%s:band_step60 = 0x%x\n",__func__,band_step60);
  CDBG("%s:max_band60 = 0x%x\n",__func__,max_band60);
  band_step50 = ov2675_preview_sysclk * 100/ov2675_preview_HTS;
  rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3071, (band_step50 >> 8),MSM_CAMERA_I2C_BYTE_DATA);
  rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3070, (band_step50 & 0xff),MSM_CAMERA_I2C_BYTE_DATA);
  max_band50 = ((preview_VTS-4)/band_step50);
  rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x301c, max_band50-1,MSM_CAMERA_I2C_BYTE_DATA);
  CDBG("%s:band_step50 = 0x%x\n",__func__,band_step50 );
  CDBG("%s:max_band50 = 0x%x\n",__func__,max_band50);
  //dummyline = ov2675_get_dummyline();
  return rc;
}

/******************************************************************************
+
+******************************************************************************/
static int ov2675_set_nightmode(int NightMode)

{
  int rc = 0;
  uint16_t temp;

  CDBG("%s:NightMode = %d\n",__func__,NightMode);
  switch (NightMode) {
    case 0:{//Off
        msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3014, &temp,MSM_CAMERA_I2C_BYTE_DATA);
        temp = temp & 0xf7;			// night mode off, bit[3] = 0
        msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3014, temp,MSM_CAMERA_I2C_BYTE_DATA);
        // clear dummy lines
        msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x302d, 0,MSM_CAMERA_I2C_BYTE_DATA);
        msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x302e, 0,MSM_CAMERA_I2C_BYTE_DATA);
      }
      break;
    case 1: {// On
        msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3014, &temp,MSM_CAMERA_I2C_BYTE_DATA);
        temp = temp | 0x08;			// night mode on, bit[3] = 1
        msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3014, temp,MSM_CAMERA_I2C_BYTE_DATA);
      }
      break;
    default:
      break;
   }
  return rc;
}

/******************************************************************************
+
+******************************************************************************/
static int ov2675_get_preview_exposure_gain(void)
{
  int rc = 0;
  ov2675_preview_shutter = ov2675_get_shutter();
  // read preview gain
  ov2675_preview_gain16 = ov2675_get_gain16();
  ov2675_preview_binning = ov2675_get_binning();
  // turn off night mode for capture
  rc = ov2675_set_nightmode(0);
  ov2675_dummyline = ov2675_get_dummyline();
  return rc;
}

/******************************************************************************
+
+******************************************************************************/
static int ov2675_set_preview_exposure_gain(void)
{
  int rc = 0;
  rc = ov2675_set_shutter(ov2675_preview_shutter);
  rc = ov2675_set_gain16(ov2675_preview_gain16);
  if(ov2675_CAMERA_WB_AUTO)
   {

	rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3306, 0x00,MSM_CAMERA_I2C_BYTE_DATA); //set to WB_AUTO
   }

  return rc;
}

/******************************************************************************
+
+******************************************************************************/
static int ov2675_set_capture_exposure_gain(void)
{
  int rc = 0;
  unsigned int capture_shutter, capture_gain16, capture_sysclk, capture_HTS, capture_VTS;
  unsigned int light_frequency, capture_bandingfilter, capture_max_band;
  unsigned long capture_gain16_shutter;
  uint16_t temp;

  //Step3: calculate and set capture exposure and gain
  // turn off AEC, AGC
  msm_camera_i2c_read(ov2675_s_ctrl_p->sensor_i2c_client,0x3013, &temp,MSM_CAMERA_I2C_BYTE_DATA);
  temp = temp & 0xfa;
  msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3013, temp,MSM_CAMERA_I2C_BYTE_DATA);
  // read capture sysclk
  capture_sysclk = ov2675_get_sysclk();
  // read capture HTS
  capture_HTS = ov2675_get_HTS();
  // read capture VTS
  capture_VTS = ov2675_get_VTS();
  // calculate capture banding filter
  light_frequency = ov2675_get_light_frequency();
  if (light_frequency == 60) {
    // 60Hz
    capture_bandingfilter = capture_sysclk * 100 / capture_HTS * 100 / 120;
  } else {
    // 50Hz
    capture_bandingfilter = capture_sysclk * 100 / capture_HTS;
   }

  capture_max_band = ((capture_VTS-4)/capture_bandingfilter);
  // calculate capture shutter
  //capture_shutter = ov2675_preview_shutter+ ov2675_dummyline;
  capture_shutter = ov2675_preview_shutter;
  // gain to shutter
  capture_gain16 = ov2675_preview_gain16 * capture_sysclk/ov2675_preview_sysclk
  * ov2675_preview_HTS/capture_HTS * ov2675_preview_binning;
  if (capture_gain16 < 16) {
    capture_gain16 = 16;
   }

  capture_gain16_shutter = capture_gain16 * capture_shutter;
  if(capture_gain16_shutter < (capture_bandingfilter * 16)) {
    // shutter < 1/100
    capture_shutter = capture_gain16_shutter/16;
    capture_gain16 = capture_gain16_shutter/capture_shutter;
  } else {
    if(capture_gain16_shutter > (capture_bandingfilter*capture_max_band*16)) {
      // exposure reach max
      capture_shutter = capture_bandingfilter*capture_max_band;
      capture_gain16 = capture_gain16_shutter / capture_shutter;
    } else {
      // 1/100 < capture_shutter < max, capture_shutter = n/100
      capture_shutter = (capture_gain16_shutter/16/capture_bandingfilter)
        * capture_bandingfilter;
      capture_gain16 = capture_gain16_shutter / capture_shutter;
    }
   }

  // write capture gain
  rc |= ov2675_set_gain16(capture_gain16);
  // write capture shutter
  if (capture_shutter > (capture_VTS - 4)) {
    capture_VTS = capture_shutter + 4;
    rc |= ov2675_set_VTS(capture_VTS);
   }

  rc |= ov2675_set_shutter(capture_shutter);
  if(ov2675_CAMERA_WB_AUTO)
   {

    rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3306, 0x02,MSM_CAMERA_I2C_BYTE_DATA);
    rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3337, ov2675_preview_R_gain,MSM_CAMERA_I2C_BYTE_DATA);
    rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3338, ov2675_preview_G_gain,MSM_CAMERA_I2C_BYTE_DATA);
    rc |= msm_camera_i2c_write(ov2675_s_ctrl_p->sensor_i2c_client, 0x3339, ov2675_preview_B_gain,MSM_CAMERA_I2C_BYTE_DATA);
  }
   return rc;
 }
int32_t ov2675_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;
 	uint16_t temp;	
	static int csi_config;

	CDBG("%s: %d\n", __func__, __LINE__);
	if (update_type == MSM_SENSOR_REG_INIT) {
		CDBG("Register INIT\n");
		ov2675_s_ctrl_p = s_ctrl;
		s_ctrl->curr_csi_params = NULL;
		csi_config = 0;
		/*init yuv sensor*/
		cur_effect = MSM_V4L2_EFFECT_OFF;
		cur_scene = CAMERA_BESTSHOT_MODE_OFF;
		cur_exposure = MSM_V4L2_EXPOSURE_D;
		cur_antibanding = MSM_V4L2_POWER_LINE_OFF;
		//set sensor init&quality setting
		CDBG("-+CAMERA+- set sensor init setting\n");
		rc = msm_camera_i2c_write(s_ctrl->sensor_i2c_client,0x3012, 0x80,MSM_CAMERA_I2C_BYTE_DATA);
   		msleep(20);		 
		msm_sensor_write_init_settings(s_ctrl);
/*tydrv qupw add for cam id detect*/
#ifdef TYQ_FCAM_OV2675_DETECT

		tyq_fcam_lens_adjust();
#endif
		ov2675_preview_shutter = ov2675_get_shutter();
		ov2675_preview_gain16 = ov2675_get_gain16();

	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
		CDBG("PERIODIC : %d\n", res);
   		 if(ov2675_CAMERA_WB_AUTO)
		{
			rc |= msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x330c, 0x02,MSM_CAMERA_I2C_BYTE_DATA);
			msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x330f, &ov2675_preview_R_gain,MSM_CAMERA_I2C_BYTE_DATA);
      			rc |= msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x330c, 0x03,MSM_CAMERA_I2C_BYTE_DATA);
			msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x330f, &ov2675_preview_G_gain,MSM_CAMERA_I2C_BYTE_DATA);
			rc |= msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x330c, 0x04,MSM_CAMERA_I2C_BYTE_DATA);
      			msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x330f, &ov2675_preview_B_gain,MSM_CAMERA_I2C_BYTE_DATA);
    		}

		msm_sensor_stop_stream(s_ctrl);
		if (!csi_config) {
			s_ctrl->curr_csic_params = s_ctrl->csic_params[res];
			CDBG("+CSI config in progress\n");
			v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
				NOTIFY_CSIC_CFG,
				s_ctrl->curr_csic_params);
			CDBG("+CSI config is done\n");
			mb();
			msleep(30);
			csi_config = 1;
		}
		/*PREVIEW*/
		if(res == cam_preview){
			CDBG("PREVIEW\n");
			msm_sensor_write_conf_array(
				s_ctrl->sensor_i2c_client,
				s_ctrl->msm_sensor_reg->mode_settings, res);
 			rc |= ov2675_set_bandingfilter();

			//TODO:restore night mode for Preview...

      			// turn on AEC, AGC
			msm_camera_i2c_read(s_ctrl->sensor_i2c_client,0x3013, &temp,MSM_CAMERA_I2C_BYTE_DATA);
      			temp = temp | 0x05;
      			rc |= msm_camera_i2c_write(s_ctrl->sensor_i2c_client, 0x3013, temp,MSM_CAMERA_I2C_BYTE_DATA);
     		 	ov2675_set_preview_exposure_gain();
		/*SNAPSHOT*/		
		}else if(res == cam_snapshot){
			CDBG("SNAPSHOT\n");
			ov2675_get_preview_exposure_gain();
			msm_sensor_write_conf_array(
				s_ctrl->sensor_i2c_client,
				s_ctrl->msm_sensor_reg->mode_settings, res);
			ov2675_set_capture_exposure_gain();
		}
		else if(res == cam_zsl){
					CDBG("zsl mode\n");		
					
					msm_sensor_write_conf_array(
						s_ctrl->sensor_i2c_client,
						s_ctrl->msm_sensor_reg->mode_settings, res);
				}

		msm_sensor_start_stream(s_ctrl);
		
		if(res)/*PREVIEW*/
			msleep(800);
		else/*SNAPSHOT*/
			msleep(200);
	}
	return rc;
}

static int ov2675_pwdn_gpio;
static int ov2675_reset_gpio;
static int ov2675_probe_init_gpio(const struct msm_camera_sensor_info *data)
{
	int rc = 0;
	CDBG("%s: entered\n", __func__);

	ov2675_pwdn_gpio = data->sensor_pwd;
	ov2675_reset_gpio = data->sensor_reset;

	CDBG("%s ov2675: pwdn_gpio:%d, reset_gpio:%d\n", __func__,
			ov2675_pwdn_gpio, ov2675_reset_gpio);
	return rc;
}

int32_t ov2675_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	int value = -1;
	struct msm_camera_sensor_info *info = NULL;

	CDBG("%s: %d\n", __func__, __LINE__);
	info = s_ctrl->sensordata;
	/*request gpios and enable MCLK*/
	rc = msm_sensor_power_up(s_ctrl);
	if (rc < 0) {
		CDBG("%s: msm_sensor_power_up failed\n", __func__);
		return rc;
	}
	/*initial front/rear camera gpios*/
	rc = ov2675_probe_init_gpio(info);
	if (rc < 0) {
		CDBG("%s: gpio init failed\n", __func__);
		goto power_up_fail;
	}

	msleep(20);
	gpio_direction_output(info->sensor_pwd, 0);
	msleep(20);
	gpio_direction_output(info->sensor_reset, 0);
	msleep(8);
	gpio_direction_output(info->sensor_reset, 1);
	msleep(20);

	value = gpio_get_value(info->sensor_reset);
	CDBG("Exit %s: %d:  sensor_reset(gpio-%d) = %d,\n", 
			__func__, __LINE__,info->sensor_reset,value);
	value = gpio_get_value(info->sensor_pwd);
	CDBG("Exit %s: %d:  sensor_pwd(gpio-%d) = %d,\n", 
			__func__, __LINE__,info->sensor_pwd,value);
	return rc;

power_up_fail:
	CDBG("ov2675_sensor_power_up: ov2675 SENSOR POWER UP FAILS!\n");
	return rc;
}
int32_t ov2675_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
	struct msm_camera_sensor_info *info = NULL;
	int rc = 0;
	int value = -1;
   
	CDBG("%s: %d\n", __func__, __LINE__);

	info = s_ctrl->sensordata;
	
	if (info->sensor_reset_enable)
		gpio_direction_output(info->sensor_reset, 0);
	
	/*power down before mclk disable*/
	gpio_direction_output(info->sensor_pwd, 1);
	msleep(50);

	value = gpio_get_value(info->sensor_pwd);

	msm_sensor_power_down(s_ctrl);
	msleep(10);

	value = gpio_get_value(info->sensor_reset);
	CDBG("%s: %d: After delay 50ms: sensor_reset(gpio-%d) = %d,\n", 
			__func__, __LINE__,info->sensor_reset,value);
	value = gpio_get_value(info->sensor_pwd);
	CDBG("%s: %d: After delay 50ms: sensor_pwd(gpio-%d) = %d,\n", 
			__func__, __LINE__,info->sensor_pwd,value);
	return rc;
}


static struct msm_sensor_fn_t ov2675_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_group_hold_on = msm_sensor_group_hold_on,
	.sensor_group_hold_off = msm_sensor_group_hold_off,
	.sensor_set_fps = ov2675_sensor_set_fps,
	.sensor_csi_setting = ov2675_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
	.sensor_power_up = ov2675_sensor_power_up, 
	.sensor_power_down = ov2675_sensor_power_down, 
	.sensor_write_exp_gain = ov2675_write_exp_gain,
	.sensor_write_snapshot_exp_gain = ov2675_write_exp_gain,
};

static struct msm_sensor_reg_t ov2675_regs = {
	.default_data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.start_stream_conf = ov2675_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(ov2675_start_settings),
	.stop_stream_conf = ov2675_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(ov2675_stop_settings),
	.group_hold_on_conf = NULL,
	.group_hold_on_conf_size = 0,
	.group_hold_off_conf = NULL,
	.group_hold_off_conf_size = 0,

	.init_settings = &ov2675_init_conf[0],
	.init_size = ARRAY_SIZE(ov2675_init_conf),
	.mode_settings = &ov2675_confs[0],
	.no_effect_settings = &ov2675_no_effect_confs[0],
	.output_settings = &ov2675_dimensions[0],
	.num_conf = ARRAY_SIZE(ov2675_confs),
};

static struct msm_sensor_ctrl_t ov2675_s_ctrl = {
	.msm_sensor_reg = &ov2675_regs,
	.msm_sensor_v4l2_ctrl_info = ov2675_v4l2_ctrl_info,
	.num_v4l2_ctrl = ARRAY_SIZE(ov2675_v4l2_ctrl_info),
	.sensor_i2c_client = &ov2675_sensor_i2c_client,
	.sensor_i2c_addr = ov2675_I2C_ADDR,//?
	.sensor_output_reg_addr = &ov2675_reg_addr,
	.sensor_id_info = &ov2675_id_info,
	.sensor_exp_gain_info = &ov2675_exp_gain_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
	.csic_params = &ov2675_csi_params_array[0],
	.msm_sensor_mutex = &ov2675_mut,
	.sensor_i2c_driver = &ov2675_i2c_driver,
	.sensor_v4l2_subdev_info = ov2675_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov2675_subdev_info),
	.sensor_v4l2_subdev_ops = &ov2675_subdev_ops,
	.func_tbl = &ov2675_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,//?
};

module_init(msm_sensor_init_module);
MODULE_DESCRIPTION("APTINA VGA YUV sensor driver");
MODULE_LICENSE("GPL v2");
