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

enum sensor_state_t {
	NORMAL_CAPTURE = 0,//res  = MSM_SENSOR_RES_FULL
	NORMAL_PREVIEW,//res  = MSM_SENSOR_RES_QTR
	NORMAL_VIDEO,//res  = MSM_SENSOR_RES_2
	NORMAL_INIT = 10,
	PIP_INIT,
	PIP_CAPTURE,
	PIP_PREVIEW,
	PIP_VIDEO,
	PIP_SUB,//sub camera mode
	STATE_INVALID,
};

enum PIP_confs_t {
	PIP_OV5648_INIT,
	PIP_OV7695_INIT,
	PIP_OV5648_PREVIEW,
	PIP_OV7695_PREVIEW,
	PIP_OV5648_VIDEO,
	PIP_OV7695_VIDEO,
	PIP_OV5648_CAPTURE,
	PIP_OV7695_CAPTURE,
	PIP_OV5648_SUB,
	PIP_OV7695_SUB,
	PIP_INDEX_INVALID,
};

extern struct msm_camera_i2c_conf_array ov5648_ov7695_PIP_confs[];
#endif
