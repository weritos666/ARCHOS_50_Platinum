/*
 *  Copyright (c) 2012-2013, The Linux Foundation. All Rights Reserved.
 *  Copyright (C) 2011, Bosch Sensortec GmbH All Rights Reserved
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
 *  Date: 2011/4/8 11:00:00
 *  Revision: 2.5
 *
 * file BMA250.c
 * brief This file contains all function implementations for the BMA250 in linux
 *
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/earlysuspend.h>

/*tianyu niuli add bma reg backedup on 20120628 for "bma sensor data" begin*/
#define TYQ_USE_BMA_INIT
#define TYQ_USE_BMA_OFFSET



#ifdef TYQ_USE_BMA_INIT
#include <mach/proc_comm.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#endif
/*tianyu niuli add bma reg backedup on 20120628 for "bma sensor data" end*/



#define SENSOR_NAME 			"bma250"
#define GRAVITY_EARTH                   9806550
#define ABSMIN_2G                       (-GRAVITY_EARTH * 2)
#define ABSMAX_2G                       (GRAVITY_EARTH * 2)
#define SLOPE_THRESHOLD_VALUE 		32
#define SLOPE_DURATION_VALUE 		1
#define INTERRUPT_LATCH_MODE 		13
#define INTERRUPT_ENABLE 		1
#define INTERRUPT_DISABLE 		0
#define MAP_SLOPE_INTERRUPT 		2
#define SLOPE_X_INDEX 			5
#define SLOPE_Y_INDEX 			6
#define SLOPE_Z_INDEX 			7
#define BMA250_MAX_DELAY		200
#define BMA250_CHIP_ID			3
#define BMA250E_CHIP_ID			0xF9
//pengwei added for identify 8bit or 10bits sensor
#ifdef TYQ_G_SENSOR_COMPATIBLE_SUPPORT
#define BMA2x2E_CHIP_ID                    0XF8
#endif
#if 0
#define BMA250E_CHIP_ID1			0xF8
#define BMA250E_CHIP_ID2			0xF9
#endif
#define BMA250_RANGE_SET		0
#define BMA250_BW_SET			4

/*
 *
 *      register definitions
 *
 */

#define BMA250_CHIP_ID_REG                      0x00
#define BMA250_VERSION_REG                      0x01
#define BMA250_X_AXIS_LSB_REG                   0x02
#define BMA250_X_AXIS_MSB_REG                   0x03
#define BMA250_Y_AXIS_LSB_REG                   0x04
#define BMA250_Y_AXIS_MSB_REG                   0x05
#define BMA250_Z_AXIS_LSB_REG                   0x06
#define BMA250_Z_AXIS_MSB_REG                   0x07
#define BMA250_TEMP_RD_REG                      0x08
#define BMA250_STATUS1_REG                      0x09
#define BMA250_STATUS2_REG                      0x0A
#define BMA250_STATUS_TAP_SLOPE_REG             0x0B
#define BMA250_STATUS_ORIENT_HIGH_REG           0x0C
#define BMA250_RANGE_SEL_REG                    0x0F
#define BMA250_BW_SEL_REG                       0x10
#define BMA250_MODE_CTRL_REG                    0x11
#define BMA250_LOW_NOISE_CTRL_REG               0x12
#define BMA250_DATA_CTRL_REG                    0x13
#define BMA250_RESET_REG                        0x14
#define BMA250_INT_ENABLE1_REG                  0x16
#define BMA250_INT_ENABLE2_REG                  0x17
#define BMA250_INT1_PAD_SEL_REG                 0x19
#define BMA250_INT_DATA_SEL_REG                 0x1A
#define BMA250_INT2_PAD_SEL_REG                 0x1B
#define BMA250_INT_SRC_REG                      0x1E
#define BMA250_INT_SET_REG                      0x20
#define BMA250_INT_CTRL_REG                     0x21
#define BMA250_LOW_DURN_REG                     0x22
#define BMA250_LOW_THRES_REG                    0x23
#define BMA250_LOW_HIGH_HYST_REG                0x24
#define BMA250_HIGH_DURN_REG                    0x25
#define BMA250_HIGH_THRES_REG                   0x26
#define BMA250_SLOPE_DURN_REG                   0x27
#define BMA250_SLOPE_THRES_REG                  0x28
#define BMA250_TAP_PARAM_REG                    0x2A
#define BMA250_TAP_THRES_REG                    0x2B
#define BMA250_ORIENT_PARAM_REG                 0x2C
#define BMA250_THETA_BLOCK_REG                  0x2D
#define BMA250_THETA_FLAT_REG                   0x2E
#define BMA250_FLAT_HOLD_TIME_REG               0x2F
#define BMA250_STATUS_LOW_POWER_REG             0x31
#define BMA250_SELF_TEST_REG                    0x32
#define BMA250_EEPROM_CTRL_REG                  0x33
#define BMA250_SERIAL_CTRL_REG                  0x34
#define BMA250_CTRL_UNLOCK_REG                  0x35
#define BMA250_OFFSET_CTRL_REG                  0x36
#define BMA250_OFFSET_PARAMS_REG                0x37
#define BMA250_OFFSET_FILT_X_REG                0x38
#define BMA250_OFFSET_FILT_Y_REG                0x39
#define BMA250_OFFSET_FILT_Z_REG                0x3A
#define BMA250_OFFSET_UNFILT_X_REG              0x3B
#define BMA250_OFFSET_UNFILT_Y_REG              0x3C
#define BMA250_OFFSET_UNFILT_Z_REG              0x3D
#define BMA250_SPARE_0_REG                      0x3E
#define BMA250_SPARE_1_REG                      0x3F

#define BMA250_ACC_X_LSB__POS           6
#define BMA250_ACC_X_LSB__LEN           2
#define BMA250_ACC_X_LSB__MSK           0xC0
#define BMA250_ACC_X_LSB__REG           BMA250_X_AXIS_LSB_REG

#define BMA250_ACC_X_MSB__POS           0
#define BMA250_ACC_X_MSB__LEN           8
#define BMA250_ACC_X_MSB__MSK           0xFF
#define BMA250_ACC_X_MSB__REG           BMA250_X_AXIS_MSB_REG

#define BMA250_ACC_Y_LSB__POS           6
#define BMA250_ACC_Y_LSB__LEN           2
#define BMA250_ACC_Y_LSB__MSK           0xC0
#define BMA250_ACC_Y_LSB__REG           BMA250_Y_AXIS_LSB_REG

#define BMA250_ACC_Y_MSB__POS           0
#define BMA250_ACC_Y_MSB__LEN           8
#define BMA250_ACC_Y_MSB__MSK           0xFF
#define BMA250_ACC_Y_MSB__REG           BMA250_Y_AXIS_MSB_REG

#define BMA250_ACC_Z_LSB__POS           6
#define BMA250_ACC_Z_LSB__LEN           2
#define BMA250_ACC_Z_LSB__MSK           0xC0
#define BMA250_ACC_Z_LSB__REG           BMA250_Z_AXIS_LSB_REG

#define BMA250_ACC_Z_MSB__POS           0
#define BMA250_ACC_Z_MSB__LEN           8
#define BMA250_ACC_Z_MSB__MSK           0xFF
#define BMA250_ACC_Z_MSB__REG           BMA250_Z_AXIS_MSB_REG

#define BMA250_RANGE_SEL__POS             0
#define BMA250_RANGE_SEL__LEN             4
#define BMA250_RANGE_SEL__MSK             0x0F
#define BMA250_RANGE_SEL__REG             BMA250_RANGE_SEL_REG

#define BMA250_BANDWIDTH__POS             0
#define BMA250_BANDWIDTH__LEN             5
#define BMA250_BANDWIDTH__MSK             0x1F
#define BMA250_BANDWIDTH__REG             BMA250_BW_SEL_REG

#define BMA250_EN_LOW_POWER__POS          6
#define BMA250_EN_LOW_POWER__LEN          1
#define BMA250_EN_LOW_POWER__MSK          0x40
#define BMA250_EN_LOW_POWER__REG          BMA250_MODE_CTRL_REG

#define BMA250_EN_SUSPEND__POS            7
#define BMA250_EN_SUSPEND__LEN            1
#define BMA250_EN_SUSPEND__MSK            0x80
#define BMA250_EN_SUSPEND__REG            BMA250_MODE_CTRL_REG

#define BMA250_EN_LOW__POS            1
#define BMA250_EN_LOW__LEN            4
#define BMA250_EN_LOW__MSK            0x1E
#define BMA250_EN_LOW__REG            BMA250_MODE_CTRL_REG

//pengwei added for 8bits bma222E
#define BMA250_ACC_Z8_LSB__POS           0
#define BMA250_ACC_Z8_LSB__LEN           0
#define BMA250_ACC_Z8_LSB__MSK           0x00
#define BMA250_ACC_Z8_LSB__REG           BMA250_Z_AXIS_LSB_REG

#define BMA250_ACC_Y8_LSB__POS           0
#define BMA250_ACC_Y8_LSB__LEN           0
#define BMA250_ACC_Y8_LSB__MSK           0x00
#define BMA250_ACC_Y8_LSB__REG           BMA250_Y_AXIS_LSB_REG

#define BMA250_ACC_X8_LSB__POS           0
#define BMA250_ACC_X8_LSB__LEN           0
#define BMA250_ACC_X8_LSB__MSK           0x00
#define BMA250_ACC_X8_LSB__REG           BMA250_X_AXIS_LSB_REG




//----------------------------//
#ifdef TYQ_USE_BMA_INIT
#define TY_CTS_BMA250E_FILE_ATTR_SUPPORT 1
#define BMA250_IO				0xA1
#define BMA250_GET_DATA				_IOR(BMA250_IO, 0x00, int[3])
/*tianyu niuli add bma offset backedup on 20120823 for prod100838780 begin*/
#define BMA250_GET_OFFSET				_IO(BMA250_IO, 0x01)
#define BMA250_SET_OFFSET				_IO(BMA250_IO, 0x02)

#define BMA250_CUT_OFF                  0
#define BMA250_OFFSET_TRIGGER_X         1
#define BMA250_OFFSET_TRIGGER_Y         2
#define BMA250_OFFSET_TRIGGER_Z         3

#define BMA250_OFFSET_CTRL_REG                  0x36
#define BMA250_OFFSET_PARAMS_REG                0x37
#define BMA250_OFFSET_X_AXIS_REG                0x38
#define BMA250_OFFSET_Y_AXIS_REG                0x39
#define BMA250_OFFSET_Z_AXIS_REG                0x3A

#define BMA250_EN_SLOW_COMP_X__POS              0
#define BMA250_EN_SLOW_COMP_X__LEN              1
#define BMA250_EN_SLOW_COMP_X__MSK              0x01
#define BMA250_EN_SLOW_COMP_X__REG              BMA250_OFFSET_CTRL_REG

#define BMA250_EN_SLOW_COMP_Y__POS              1
#define BMA250_EN_SLOW_COMP_Y__LEN              1
#define BMA250_EN_SLOW_COMP_Y__MSK              0x02
#define BMA250_EN_SLOW_COMP_Y__REG              BMA250_OFFSET_CTRL_REG

#define BMA250_EN_SLOW_COMP_Z__POS              2
#define BMA250_EN_SLOW_COMP_Z__LEN              1
#define BMA250_EN_SLOW_COMP_Z__MSK              0x04
#define BMA250_EN_SLOW_COMP_Z__REG              BMA250_OFFSET_CTRL_REG

#define BMA250_FAST_CAL_RDY_S__POS             4
#define BMA250_FAST_CAL_RDY_S__LEN             1
#define BMA250_FAST_CAL_RDY_S__MSK             0x10
#define BMA250_FAST_CAL_RDY_S__REG             BMA250_OFFSET_CTRL_REG

#define BMA250_CAL_TRIGGER__POS                5
#define BMA250_CAL_TRIGGER__LEN                2
#define BMA250_CAL_TRIGGER__MSK                0x60
#define BMA250_CAL_TRIGGER__REG                BMA250_OFFSET_CTRL_REG


#define BMA250_COMP_CUTOFF__POS                 0
#define BMA250_COMP_CUTOFF__LEN                 1
#define BMA250_COMP_CUTOFF__MSK                 0x01
#define BMA250_COMP_CUTOFF__REG                 BMA250_OFFSET_PARAMS_REG


#define BMA250_COMP_TARGET_OFFSET_X__POS        1
#define BMA250_COMP_TARGET_OFFSET_X__LEN        2
#define BMA250_COMP_TARGET_OFFSET_X__MSK        0x06
#define BMA250_COMP_TARGET_OFFSET_X__REG        BMA250_OFFSET_PARAMS_REG

#define BMA250_COMP_TARGET_OFFSET_Y__POS        3
#define BMA250_COMP_TARGET_OFFSET_Y__LEN        2
#define BMA250_COMP_TARGET_OFFSET_Y__MSK        0x18
#define BMA250_COMP_TARGET_OFFSET_Y__REG        BMA250_OFFSET_PARAMS_REG

#define BMA250_COMP_TARGET_OFFSET_Z__POS        5
#define BMA250_COMP_TARGET_OFFSET_Z__LEN        2
#define BMA250_COMP_TARGET_OFFSET_Z__MSK        0x60
#define BMA250_COMP_TARGET_OFFSET_Z__REG        BMA250_OFFSET_PARAMS_REG

#endif


//----------------------------//


#define BMA250_GET_BITSLICE(regvar, bitname)\
			((regvar & bitname##__MSK) >> bitname##__POS)

#define BMA250_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))

/* range and bandwidth */

#define BMA250_RANGE_2G                 0
#define BMA250_RANGE_4G                 1
#define BMA250_RANGE_8G                 2
#define BMA250_RANGE_16G                3

#define BMA250_BW_7_81HZ        0x08
#define BMA250_BW_15_63HZ       0x09
#define BMA250_BW_31_25HZ       0x0A
#define BMA250_BW_62_50HZ       0x0B
#define BMA250_BW_125HZ         0x0C
#define BMA250_BW_250HZ         0x0D
#define BMA250_BW_500HZ         0x0E
#define BMA250_BW_1000HZ        0x0F

/* mode settings */

#define BMA250_MODE_NORMAL      0
#define BMA250_MODE_LOWPOWER    1
#define BMA250_MODE_SUSPEND     2


struct bma250acc {
	s16 x, y, z;
};

struct bma250_data {
	struct i2c_client *bma250_client;
	atomic_t delay;
	atomic_t enable;
	unsigned char mode;
	struct input_dev *input;
	struct bma250acc value;
	struct mutex value_mutex;
	struct mutex enable_mutex;
	struct mutex mode_mutex;
	struct delayed_work work;
	struct work_struct irq_work;
	struct early_suspend early_suspend;
};

static void bma250_early_suspend(struct early_suspend *h);
static void bma250_late_resume(struct early_suspend *h);

//pengwei
#ifdef TYQ_USE_BMA_INIT
static struct bma250_data *bma250_misc_data;
static int bma250_open(struct inode *inode, struct file *file);
static long bma250_ioctl(struct file *file, unsigned int cmd,
		   unsigned long arg);
struct bma250acc acc_read_value;

#endif



static int bma250_smbus_read_byte(struct i2c_client *client,
				  unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_read_byte_data(client, reg_addr);
	if (dummy < 0)
		return -1;
	*data = dummy & 0x000000ff;

	return 0;
}

static int bma250_smbus_write_byte(struct i2c_client *client,
				   unsigned char reg_addr, unsigned char *data)
{
	s32 dummy;
	dummy = i2c_smbus_write_byte_data(client, reg_addr, *data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int bma250_smbus_read_byte_block(struct i2c_client *client,
					unsigned char reg_addr,
					unsigned char *data, unsigned char len)
{
	s32 dummy;
	dummy = i2c_smbus_read_i2c_block_data(client, reg_addr, len, data);
	if (dummy < 0)
		return -1;
	return 0;
}

static int bma250_set_mode(struct i2c_client *client, unsigned char Mode)
{
	int comres = 0;
	unsigned char data1 = 0;

	if (client == NULL) {
		comres = -1;
	} else {
		if (Mode < 3) {
			comres = bma250_smbus_read_byte(client,
							BMA250_EN_LOW_POWER__REG,
							&data1);
			switch (Mode) {
			case BMA250_MODE_NORMAL:
				data1 = BMA250_SET_BITSLICE(data1,
							    BMA250_EN_LOW_POWER,
							    0);
				data1 =
				    BMA250_SET_BITSLICE(data1,
							BMA250_EN_SUSPEND, 0);
				break;
			case BMA250_MODE_LOWPOWER:
				data1 = BMA250_SET_BITSLICE(data1,
							    BMA250_EN_LOW_POWER,
							    1);
				data1 =
				    BMA250_SET_BITSLICE(data1,
							BMA250_EN_SUSPEND, 0);
				break;
			case BMA250_MODE_SUSPEND:
				data1 = BMA250_SET_BITSLICE(data1,
							    BMA250_EN_LOW_POWER,
							    0);
				data1 =
				    BMA250_SET_BITSLICE(data1,
							BMA250_EN_SUSPEND, 1);
				break;
			default:
				break;
			}

			comres += bma250_smbus_write_byte(client,
							  BMA250_EN_LOW_POWER__REG,
							  &data1);
		} else {
			comres = -1;
		}
	}

	return comres;
}

static int bma250_get_mode(struct i2c_client *client, unsigned char *Mode)
{
	int comres = 0;

	if (client == NULL) {
		comres = -1;
	} else {
		comres = bma250_smbus_read_byte(client,
						BMA250_EN_LOW_POWER__REG, Mode);
		*Mode = (*Mode) >> 6;
	}

	return comres;
}

static int bma250_set_range(struct i2c_client *client, unsigned char Range)
{
	int comres = 0;
	unsigned char data1 = '\0';

	if (client == NULL) {
		comres = -1;
	} else {
		if (Range < 4) {
			comres = bma250_smbus_read_byte(client,
							BMA250_RANGE_SEL_REG,
							&data1);
			switch (Range) {
			case 0:
				data1 = BMA250_SET_BITSLICE(data1,
							    BMA250_RANGE_SEL,
							    0);
				break;
			case 1:
				data1 = BMA250_SET_BITSLICE(data1,
							    BMA250_RANGE_SEL,
							    5);
				break;
			case 2:
				data1 = BMA250_SET_BITSLICE(data1,
							    BMA250_RANGE_SEL,
							    8);
				break;
			case 3:
				data1 = BMA250_SET_BITSLICE(data1,
							    BMA250_RANGE_SEL,
							    12);
				break;
			default:
				break;
			}
			comres += bma250_smbus_write_byte(client,
							  BMA250_RANGE_SEL_REG,
							  &data1);
		} else {
			comres = -1;
		}
	}

	return comres;
}

static int bma250_get_range(struct i2c_client *client, unsigned char *Range)
{
	int comres = 0;
	unsigned char data = '\0';

	if (client == NULL) {
		comres = -1;
	} else {
		comres = bma250_smbus_read_byte(client, BMA250_RANGE_SEL__REG,
						&data);
		data = BMA250_GET_BITSLICE(data, BMA250_RANGE_SEL);
		*Range = data;
	}

	return comres;
}

static int bma250_set_bandwidth(struct i2c_client *client, unsigned char BW)
{
	int comres = 0;
	unsigned char data = '\0';
	int Bandwidth = 0;

	if (client == NULL) {
		comres = -1;
	} else {
		if (BW < 8) {
			switch (BW) {
			case 0:
				Bandwidth = BMA250_BW_7_81HZ;
				break;
			case 1:
				Bandwidth = BMA250_BW_15_63HZ;
				break;
			case 2:
				Bandwidth = BMA250_BW_31_25HZ;
				break;
			case 3:
				Bandwidth = BMA250_BW_62_50HZ;
				break;
			case 4:
				Bandwidth = BMA250_BW_125HZ;
				break;
			case 5:
				Bandwidth = BMA250_BW_250HZ;
				break;
			case 6:
				Bandwidth = BMA250_BW_500HZ;
				break;
			case 7:
				Bandwidth = BMA250_BW_1000HZ;
				break;
			default:
				break;
			}
			comres = bma250_smbus_read_byte(client,
							BMA250_BANDWIDTH__REG,
							&data);
			data =
			    BMA250_SET_BITSLICE(data, BMA250_BANDWIDTH,
						Bandwidth);
			comres +=
			    bma250_smbus_write_byte(client,
						    BMA250_BANDWIDTH__REG,
						    &data);
		} else {
			comres = -1;
		}
	}

	return comres;
}

static int bma250_get_bandwidth(struct i2c_client *client, unsigned char *BW)
{
	int comres = 0;
	unsigned char data = '\0';

	if (client == NULL) {
		comres = -1;
	} else {
		comres = bma250_smbus_read_byte(client, BMA250_BANDWIDTH__REG,
						&data);
		data = BMA250_GET_BITSLICE(data, BMA250_BANDWIDTH);
		if (data <= 8) {
			*BW = 0;
		} else {
			if (data >= 0x0F)
				*BW = 7;
			else
				*BW = data - 8;

		}
	}

	return comres;
}

//pengwei added for identify 8bit or 10bits sensor
#ifdef TYQ_G_SENSOR_COMPATIBLE_SUPPORT
static int sensor_chip_id;
#endif
//pendwei added end


static int bma250_read_accel_xyz(struct i2c_client *client,
				 struct bma250acc *acc)
{
	int comres;
	unsigned char data[6];
	if (client == NULL) {
		comres = -1;
	} 
	else
	{
//pengwei added for identify 8bit or 10bits sensor
#ifdef TYQ_G_SENSOR_COMPATIBLE_SUPPORT
	switch(sensor_chip_id)
		{
		case BMA250_CHIP_ID:
		case BMA250E_CHIP_ID:
#endif
		comres = bma250_smbus_read_byte_block(client,
						      BMA250_ACC_X_LSB__REG,
						      data, 6);

		acc->x = BMA250_GET_BITSLICE(data[0], BMA250_ACC_X_LSB)
		    | (BMA250_GET_BITSLICE(data[1],
					   BMA250_ACC_X_MSB) <<
		       BMA250_ACC_X_LSB__LEN);
		acc->x =
		    acc->x << (sizeof(short) * 8 -
			       (BMA250_ACC_X_LSB__LEN + BMA250_ACC_X_MSB__LEN));
		acc->x =
		    acc->x >> (sizeof(short) * 8 -
			       (BMA250_ACC_X_LSB__LEN + BMA250_ACC_X_MSB__LEN));
		acc->y = BMA250_GET_BITSLICE(data[2], BMA250_ACC_Y_LSB)
		    | (BMA250_GET_BITSLICE(data[3],
					   BMA250_ACC_Y_MSB) <<
		       BMA250_ACC_Y_LSB__LEN);
		acc->y =
		    acc->y << (sizeof(short) * 8 -
			       (BMA250_ACC_Y_LSB__LEN + BMA250_ACC_Y_MSB__LEN));
		acc->y =
		    acc->y >> (sizeof(short) * 8 -
			       (BMA250_ACC_Y_LSB__LEN + BMA250_ACC_Y_MSB__LEN));

		acc->z = BMA250_GET_BITSLICE(data[4], BMA250_ACC_Z_LSB)
		    | (BMA250_GET_BITSLICE(data[5],
					   BMA250_ACC_Z_MSB) <<
		       BMA250_ACC_Z_LSB__LEN);
		acc->z =
		    acc->z << (sizeof(short) * 8 -
			       (BMA250_ACC_Z_LSB__LEN + BMA250_ACC_Z_MSB__LEN));
		acc->z =
		    acc->z >> (sizeof(short) * 8 -
			       (BMA250_ACC_Z_LSB__LEN + BMA250_ACC_Z_MSB__LEN));
		//pengwei added for identify 8bit or 10bits sensor
#ifdef TYQ_G_SENSOR_COMPATIBLE_SUPPORT
		break;
		
	case BMA2x2E_CHIP_ID:
			comres = bma250_smbus_read_byte_block(client,
				BMA250_ACC_X8_LSB__REG, data, 6);
		acc->x = BMA250_GET_BITSLICE(data[0], BMA250_ACC_X8_LSB)|
			(BMA250_GET_BITSLICE(data[1],
				BMA250_ACC_X_MSB)<<(BMA250_ACC_X8_LSB__LEN));
		acc->x = acc->x << (sizeof(short)*8-(BMA250_ACC_X8_LSB__LEN +
					BMA250_ACC_X_MSB__LEN));
		acc->x = acc->x >> (sizeof(short)*8-(BMA250_ACC_X8_LSB__LEN +
					BMA250_ACC_X_MSB__LEN));

		acc->y = BMA250_GET_BITSLICE(data[2], BMA250_ACC_Y8_LSB)|
			(BMA250_GET_BITSLICE(data[3],
				BMA250_ACC_Y_MSB)<<(BMA250_ACC_Y8_LSB__LEN
									));
		acc->y = acc->y << (sizeof(short)*8-(BMA250_ACC_Y8_LSB__LEN +
					BMA250_ACC_Y_MSB__LEN));
		acc->y = acc->y >> (sizeof(short)*8-(BMA250_ACC_Y8_LSB__LEN +
					BMA250_ACC_Y_MSB__LEN));

		acc->z = BMA250_GET_BITSLICE(data[4], BMA250_ACC_Z8_LSB)|
			(BMA250_GET_BITSLICE(data[5],
				BMA250_ACC_Z_MSB)<<(BMA250_ACC_Z8_LSB__LEN));
		acc->z = acc->z << (sizeof(short)*8-(BMA250_ACC_Z8_LSB__LEN +
					BMA250_ACC_Z_MSB__LEN));
		acc->z = acc->z >> (sizeof(short)*8-(BMA250_ACC_Z8_LSB__LEN +
					BMA250_ACC_Z_MSB__LEN));
		/*ty:niuli ==>>>>>8-bit convert to 10-bit as reporting value 20120525 begin*/
		acc->x = 4*acc->x;
		acc->y = 4*acc->y;
		acc->z = 4*acc->z;
		/*ty:niuli ==>>>>>8-bit convert to 10-bit as reporting value 20120525 end*/
		break;
			break;
		default:
			break;
		}
#endif
	}

		
	return comres;
}

static void bma250_work_func(struct work_struct *work)
{
	struct bma250_data *bma250 = container_of((struct delayed_work *)work,
						  struct bma250_data, work);
	static struct bma250acc acc,acc_report;
//		static struct bma250acc acc,acc_report;
	unsigned long delay = msecs_to_jiffies(atomic_read(&bma250->delay));

	bma250_read_accel_xyz(bma250->bma250_client, &acc);
	/*adjust the data output for skud compatible */
//pengwei add for 5932 g-sensor with ACC = -g
#if defined (TYQ_TBW5932_SUPPORT) || defined(TYQ_TBW5939_SUPPORT) 
				acc_report.x= acc.x;
				acc_report.y = acc.y;
				acc_report.z = -acc.z;
/*tydrv qupw add for 5935 5933 */
//tydrv pengwei added for 5937
#elif defined(TYQ_GSENSOR_NEGATIVE_XYZ)
acc_report.x= -acc.x;
acc_report.y = -acc.y;
acc_report.z = -acc.z;

#else 
				acc_report.x= acc.x;
				acc_report.y = -acc.y;
				acc_report.z = acc.z;
#endif

	input_report_rel(bma250->input, REL_RX, acc_report.y * 4);
	input_report_rel(bma250->input, REL_RY, acc_report.x * 4);
	input_report_rel(bma250->input, REL_RZ, acc_report.z * 4);
	input_sync(bma250->input);
	mutex_lock(&bma250->value_mutex);
	bma250->value = acc_report;	
	acc_read_value = acc_report; //ty:niuli modify for compass
	mutex_unlock(&bma250->value_mutex);
	schedule_delayed_work(&bma250->work, delay);
}

static ssize_t bma250_range_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_range(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma250_range_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma250_set_range(bma250->bma250_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma250_bandwidth_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_bandwidth(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);

}

static ssize_t bma250_bandwidth_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma250_set_bandwidth(bma250->bma250_client,
				 (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma250_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	unsigned char data;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	if (bma250_get_mode(bma250->bma250_client, &data) < 0)
		return sprintf(buf, "Read error\n");

	return sprintf(buf, "%d\n", data);
}

static ssize_t bma250_mode_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (bma250_set_mode(bma250->bma250_client, (unsigned char)data) < 0)
		return -EINVAL;

	return count;
}

static ssize_t bma250_value_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct bma250_data *bma250 = input_get_drvdata(input);
	struct bma250acc acc_value;

	mutex_lock(&bma250->value_mutex);
	acc_value = bma250->value;
	mutex_unlock(&bma250->value_mutex);

	return sprintf(buf, "%d %d %d\n", acc_value.x, acc_value.y,
		       acc_value.z);
}

static ssize_t bma250_delay_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma250->delay));

}

static ssize_t bma250_delay_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	unsigned long data;
	int error;
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (data > BMA250_MAX_DELAY)
		data = BMA250_MAX_DELAY;
	atomic_set(&bma250->delay, (unsigned int)data);

	return count;
}

static ssize_t bma250_enable_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&bma250->enable));

}

static void bma250_set_enable(struct device *dev, int enable)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct bma250_data *bma250 = i2c_get_clientdata(client);
	int pre_enable = atomic_read(&bma250->enable);

	mutex_lock(&bma250->enable_mutex);
	if (enable) {
		if (pre_enable == 0) {
			bma250_set_mode(bma250->bma250_client,
					BMA250_MODE_NORMAL);
			schedule_delayed_work(&bma250->work,
					      msecs_to_jiffies(atomic_read
							       (&bma250->
								delay)));
			atomic_set(&bma250->enable, 1);
		}

	} else {
		if (pre_enable == 1) {
			bma250_set_mode(bma250->bma250_client,
					BMA250_MODE_SUSPEND);
			cancel_delayed_work_sync(&bma250->work);
			atomic_set(&bma250->enable, 0);
		}
	}
	mutex_unlock(&bma250->enable_mutex);

}

static ssize_t bma250_enable_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if ((data == 0) || (data == 1)) {
		bma250_set_enable(dev, data);
	}

	return count;
}

static DEVICE_ATTR(range, S_IRUGO | S_IWUSR | S_IWGRP,
		   bma250_range_show, bma250_range_store);
static DEVICE_ATTR(bandwidth, S_IRUGO | S_IWUSR | S_IWGRP,
		   bma250_bandwidth_show, bma250_bandwidth_store);
static DEVICE_ATTR(mode, S_IRUGO | S_IWUSR | S_IWGRP,
		   bma250_mode_show, bma250_mode_store);
static DEVICE_ATTR(value, S_IRUGO, bma250_value_show, NULL);
static DEVICE_ATTR(poll_delay, S_IRUGO | S_IWUSR | S_IWGRP,
		   bma250_delay_show, bma250_delay_store);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR | S_IWGRP,
		   bma250_enable_show, bma250_enable_store);

static struct attribute *bma250_attributes[] = {
	&dev_attr_range.attr,
	&dev_attr_bandwidth.attr,
	&dev_attr_mode.attr,
	&dev_attr_value.attr,
	&dev_attr_poll_delay.attr,
	&dev_attr_enable.attr,
	NULL
};

static struct attribute_group bma250_attribute_group = {
	.attrs = bma250_attributes
};

static int bma250_input_init(struct bma250_data *bma250)
{
	struct input_dev *dev;
	int err;

	dev = input_allocate_device();
	if (!dev)
		return -ENOMEM;
	dev->name = "acc";	//SENSOR_NAME; changed for sku1 compatible
	dev->id.bustype = BUS_I2C;

	/* X */
	input_set_capability(dev, EV_REL, REL_RX);
	input_set_abs_params(dev, REL_RX, ABSMIN_2G, ABSMAX_2G, 0, 0);
	/* Y */
	input_set_capability(dev, EV_REL, REL_RY);
	input_set_abs_params(dev, REL_RY, ABSMIN_2G, ABSMAX_2G, 0, 0);
	/* Z */
	input_set_capability(dev, EV_REL, REL_RZ);
	input_set_abs_params(dev, REL_RZ, ABSMIN_2G, ABSMAX_2G, 0, 0);
	input_set_drvdata(dev, bma250);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		return err;
	}
	bma250->input = dev;

	return 0;
}

static void bma250_input_delete(struct bma250_data *bma250)
{
	struct input_dev *dev = bma250->input;

	input_unregister_device(dev);
	input_free_device(dev);
}

//pengwei add for bma sensor
#ifdef TYQ_USE_BMA_INIT

static struct file_operations bma250_fops = {
	.owner = THIS_MODULE,
	.open = bma250_open,
	.unlocked_ioctl = bma250_ioctl,
};

static struct miscdevice bma250_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "bma250",
	.fops = &bma250_fops,
};
static struct bma250_data *data;
static int ty_sensor_bma_backup_or_restore(struct i2c_client *client);
static int ty_sensor_bma_offset_restore(struct i2c_client *client);

#endif

static int bma250_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	int err = 0;
	int tempvalue;
#ifdef TYQ_USE_BMA_INIT
		//niuli add for test sensor ic work ok 
	int bma_init_flag =-1;
#else
	struct bma250_data *data;
#endif
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		printk(KERN_INFO "i2c_check_functionality error\n");
		goto exit;
	}
	data = kzalloc(sizeof(struct bma250_data), GFP_KERNEL);
	if (!data) {
		err = -ENOMEM;
		goto exit;
	}
	

	i2c_set_clientdata(client, data);
	data->bma250_client = client;
	mutex_init(&data->value_mutex);
	mutex_init(&data->mode_mutex);
	mutex_init(&data->enable_mutex);
	bma250_set_bandwidth(client, BMA250_BW_SET);
	bma250_set_range(client, BMA250_RANGE_SET);

	INIT_DELAYED_WORK(&data->work, bma250_work_func);
	atomic_set(&data->delay, BMA250_MAX_DELAY);
	atomic_set(&data->enable, 0);


	/*tianyu niuli add bma reg backedup on 20120628 for "bma sensor data" begin*/
#ifdef TYQ_USE_BMA_INIT
	bma_init_flag = ty_sensor_bma_backup_or_restore(client);	
	if(bma_init_flag == -1){
		printk("tysensor:bmainit ic trim is not ok flag=%d\n",bma_init_flag);
		goto kfree_exit;
	}
#endif
	/*tianyu niuli add bma reg backedup on 20120628 for "bma sensor data" end*/
	
	/*tianyu niuli add bma offset backedup on 20120823 for prod100838780 begin*/
#ifdef TYQ_USE_BMA_OFFSET
		err = ty_sensor_bma_offset_restore(client);
		if(err<0){
			/*it indicate that bma has't offset so use default value */
			printk("--TY--bma2x2:ty_sensor_bma_offset_restore failed  \n");
		}
#endif	
	/*tianyu niuli add bma offset backedup on 20120823 for prod100838780 end*/

	/* read chip id */
		tempvalue = 0;
		tempvalue = i2c_smbus_read_word_data(client, BMA250_CHIP_ID_REG);	
#ifdef TYQ_G_SENSOR_COMPATIBLE_SUPPORT
			sensor_chip_id = tempvalue & 0x00FF;
			printk(KERN_INFO"tydrv show chip_id is %d\n",sensor_chip_id);
#endif
		/* SKUD board is using BMA250E chip! */
		if ((tempvalue & 0x00FF) == BMA250E_CHIP_ID) {
			printk(KERN_INFO "Bosch Sensortec Device detected!ID=%d\n",tempvalue);
		}
		/*tyqdrv pengwei added for bma250 compatible*/
#ifdef TYQ_G_SENSOR_COMPATIBLE_SUPPORT 
		else if ((tempvalue & 0x00FF) == BMA250_CHIP_ID){
			printk(KERN_INFO "Bosch Sensortec Device detected!ID=%d\n",tempvalue);
				}
		else if ((tempvalue & 0x00FF) == BMA2x2E_CHIP_ID){
			printk(KERN_INFO "Bosch Sensortec Device detected!ID=%d\n",tempvalue);
				}
#endif
		/*tyqdrv pengwei added for bma250 compatible end*/
		else {
#if 0
		tempvalue = i2c_smbus_read_word_data(client, BMA250_CHIP_ID_REG) & 0x00FF;
		/* SKUD board is using BMA250E chip! */
		if (tempvalue == BMA250E_CHIP_ID1 || tempvalue == BMA250E_CHIP_ID2) {
			printk(KERN_INFO "Bosch Sensortec Device detected!\n"
				   "BMA250 registered I2C driver!\n");
		} else {
#endif
			printk(KERN_INFO "Bosch Sensortec Device not found, \
					i2c error %d \n", tempvalue);
			err = -1;
			goto kfree_exit;
		}

	
	err = bma250_input_init(data);
	if (err < 0)
		goto kfree_exit;

	err = sysfs_create_group(&data->input->dev.kobj,
				 &bma250_attribute_group);
	if (err < 0)
		goto error_sysfs;

	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = bma250_early_suspend;
	data->early_suspend.resume = bma250_late_resume;
	register_early_suspend(&data->early_suspend);
//pengwei add for ioctl
#ifdef TYQ_USE_BMA_INIT
	bma250_misc_data = data;
	misc_register(&bma250_device);//wang_gj add for ioctl
#endif
	/* try to reduce the power comsuption */
	bma250_set_mode(data->bma250_client, BMA250_MODE_SUSPEND);

	return 0;

error_sysfs:
	bma250_input_delete(data);

kfree_exit:
	kfree(data);
exit:
	return err;
}


//pengwei add for bma sensor
#ifdef TYQ_USE_BMA_INIT




#define TYQ_BMA_BLOCK_SIZE 26
int CalcCRC8(unsigned char *p_MemAddr, unsigned char BlockSize)
{
	unsigned char CrcReg = 0xFF; 
	unsigned char MemByte;
	unsigned char BitNo;	

	while (BlockSize)
	{
		MemByte = *p_MemAddr;
		for (BitNo = 0; BitNo < 8; BitNo++)
		{
			if ((CrcReg^MemByte) & 0x80)
			{
				CrcReg = (CrcReg << 1) ^ 0x11D; 
			}
			else
			{
				CrcReg <<= 1;
			}
			MemByte <<= 1;
		}
		BlockSize--;
		p_MemAddr++;
	}

	CrcReg = ~CrcReg;
	return CrcReg;
}

static int ty_get_bma_trim_info(unsigned char *buf)
{
	uint32_t value1=0,value2=0;
	int ret=-1;
	unsigned char value1_control_bit=0x00;
	unsigned char return_flag;
	unsigned char recycle_flag;
	for(recycle_flag=0;recycle_flag<4;recycle_flag++){
		value1|=((value1_control_bit|recycle_flag)<<24);
		ret = msm_proc_comm(PCOM_CUSTOMER_CMD3,&value1,&value2);
		return_flag=(unsigned char)((value1>>24)&0x40);
		if(return_flag==0x00){
			buf[recycle_flag*7+0] = (value2&0x000000ff);
			buf[recycle_flag*7+1] = (value2&0x0000ff00)>>8;
			buf[recycle_flag*7+2] = (value2&0x00ff0000)>>16;
			buf[recycle_flag*7+3] = (value2&0xff000000)>>24;
			buf[recycle_flag*7+4] = (value1&0x000000ff);
			buf[recycle_flag*7+5] = (value1&0x0000ff00)>>8;
			buf[recycle_flag*7+6] = (value1&0x00ff0000)>>16;
			ret=0;
//			printk("ty_get_bma_trim_info:return_flag=%d\n",return_flag);
			printk("bma--rpc--<<(%d),(%x,%x,%x,%x,%x,%x,%x)\n",recycle_flag,buf[recycle_flag*7+0]\
				,buf[recycle_flag*7+1],buf[recycle_flag*7+2],buf[recycle_flag*7+3],buf[recycle_flag*7+4],buf[recycle_flag*7+5],buf[recycle_flag*7+6]);
//			printk("ty_get_trim_info read ok\n");
		}else{
			printk("ty_get_trim_info read error\n");
			ret = -1;
			break;
		}
		value1=0;
		value2=0;
	}
	return ret;
	
}

static int ty_set_bma_trim_info(unsigned char *buf)	
{
	uint32_t value1=0,value2=0;
	int ret=-1;
	unsigned char value1_control_bit=0x80;
	unsigned char recycle_flag;
	for(recycle_flag=0;recycle_flag<4;recycle_flag++){
		value1 = ((value1_control_bit|recycle_flag)<<24);
		value1 |=(buf[recycle_flag*7+6]<<16) |(buf[recycle_flag*7+5]<<8)|(buf[recycle_flag*7+4]);
		value2 = (buf[recycle_flag*7+3]<<24) |(buf[recycle_flag*7+2]<<16) |(buf[recycle_flag*7+1]<<8)|(buf[recycle_flag*7+0]);
		ret = msm_proc_comm(PCOM_CUSTOMER_CMD3, &value1, &value2);
		printk("bma--rpc-->>(%d),(%x,%x,%x,%x,%x,%x,%x)\n",recycle_flag,buf[recycle_flag*7+0]\
				,buf[recycle_flag*7+1],buf[recycle_flag*7+2],buf[recycle_flag*7+3],buf[recycle_flag*7+4],buf[recycle_flag*7+5],buf[recycle_flag*7+6]);
		value1 =0;
		value2 =0;
	}
	ret = 0;
	return ret;
}

static int ty_sensor_bma_backup_or_restore(struct i2c_client *client){
	unsigned char regs[TYQ_BMA_BLOCK_SIZE + 1] = {0};
	unsigned char bma_init[28] = {0};
	int ii;
	unsigned char crc8;
	int fs_ret = 0;
	unsigned char buf;
	int flag_cmp = -1;

/*open super thone to control sensor*/
	buf = 0x00;
	bma250_smbus_write_byte(client,0x11,&buf);

	msleep(2);

	buf = 0xAA;
	bma250_smbus_write_byte(client, 0x35, &buf);
	bma250_smbus_write_byte(client, 0x35, &buf);

	bma250_smbus_read_byte(client,0x05,&buf);
	buf = (buf&0x1F)|0x80;
	bma250_smbus_write_byte(client, 0x05, &buf);

/*i2c read ic*/
	printk("tysensor:bma i2c read ic begin [");
	for(ii=0x00;ii<=0x1A;ii++){
		bma250_smbus_read_byte(client, ii, &regs[ii]);
		printk("%x ",regs[ii]);
	}
	printk("] end\n");
/*read bp fs*/
	fs_ret = ty_get_bma_trim_info(bma_init);
	if(fs_ret == 0){
		printk("tysensor:bma backedup from bp success\n");
	}else{
		printk("tysensor:bma backedup from bp error\n");
	}

/*crc flag process*/
	crc8 = CalcCRC8(regs, TYQ_BMA_BLOCK_SIZE);
	if(crc8 == regs[0x1A]){
/*restore to bp when test if it's not same sensor*/
		flag_cmp = memcmp(regs,bma_init,27);
		if(flag_cmp){
			printk("tysensor:bma is not a same sensor flag=%d\n",flag_cmp);
			for(ii =0x00;ii<=0x1A;ii++){
			bma_init[ii] = regs[ii];
			}
			bma_init[27] = 1;
			fs_ret = ty_set_bma_trim_info(bma_init);
		}else{
			printk("tysensor:bma is a same sensor flag=%d\n",flag_cmp);
		}
	}else{
/*bp flag process*/
		if(bma_init[27] == 1){	
			/*i2c write ic*/
			for(ii=0x00;ii<=0x1A;ii++){
				bma250_smbus_write_byte(client,ii,&bma_init[ii]);
			}
			printk("tysensor:bma init success (bp_flag,crc)(1,0)\n");
		}else{
			/*close super thone to control sensor*/		
			buf = 0x0A;
			bma250_smbus_write_byte(client, 0x35, &buf);

			buf = 0x0A;
			for(ii = 0x38;ii<=0x3A;ii++){
				bma250_smbus_write_byte(client, ii, &buf);
			}
			printk("tysensor:bma init error (bp_flag,crc)(0,0)\n");
			return -1;
		}
	}
/*close super thone to control sensor*/
	buf = 0x0A;
	bma250_smbus_write_byte(client, 0x35, &buf);

	buf = 0x0A;
	for(ii = 0x38;ii<=0x3A;ii++){
		bma250_smbus_write_byte(client, ii, &buf);
	}	
	return 0;	
}


#endif


//pengwei--------------------------------------------------------//



#ifdef TYQ_USE_BMA_INIT


static int bma250_open(struct inode *inode, struct file *file)
{
	file->private_data = bma250_misc_data;
	return nonseekable_open(inode,file);
}


static int ty_get_bma_cal_offset(unsigned char *buf)
{
	uint32_t value1=0,value2=0;
	int ret=-1;
	unsigned char recycle_flag = 4;
	unsigned char return_flag;
	value1 |=(recycle_flag<<24);
	ret = msm_proc_comm(PCOM_CUSTOMER_CMD3, &value1, &value2);
	return_flag = (unsigned char)((value1>>24)&0x40);
	if(return_flag==0x00){
		buf[3] = ((value2&0xff000000)>>24);
		buf[2] = ((value2&0x00ff0000)>>16);
		buf[1] = ((value2&0x0000ff00)>>8);
		buf[0] = (value2&0x000000ff);
//		printk("ty_get_bma_cal_offset read ok\n");
		printk("ty_get_bma_cal_offset(%d):obtain offset data %x,%x,%x,%x\n",return_flag,buf[0],buf[1],buf[2],buf[3]);
	}else{
		printk("ty_get_bma_cal_offset read error\n");
	}
	value1=0;
	value2=0;
	return ret;
}
static int ty_set_bma_cal_offset(unsigned char *buf)
{
	uint32_t value1=0,value2=0;
	int ret=-1;
	unsigned char value1_control_bit=0x80;
	unsigned char recycle_flag = 4;
	value1|=((value1_control_bit|recycle_flag)<<24);
	value2 =(buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|buf[0];
	ret = msm_proc_comm(PCOM_CUSTOMER_CMD3, &value1, &value2);
	printk("ty_set_bma_cal_offset:obtain offset data %x,%x,%x,%x\n",buf[0],buf[1],buf[2],buf[3]);
	value1=0;
	value2=0;
	return ret;
}

static int bma250_set_offset_x(struct i2c_client *client, unsigned char
		offsetfilt)
{
	int comres = 0;
	unsigned char data = 0;

	data =  offsetfilt;
	comres = bma250_smbus_write_byte(client, BMA250_OFFSET_X_AXIS_REG,
						&data);

	return comres;
}


static int bma250_get_offset_x(struct i2c_client *client, unsigned char
						*offsetfilt)
{
	int comres = 0;
	unsigned char data = 0;

	comres = bma250_smbus_read_byte(client, BMA250_OFFSET_X_AXIS_REG,
						&data);
	*offsetfilt = data;

	return comres;
}

static int bma250_set_offset_y(struct i2c_client *client, unsigned char
						offsetfilt)
{
	int comres = 0;
	unsigned char data = 0;

	data =  offsetfilt;
	comres = bma250_smbus_write_byte(client, BMA250_OFFSET_Y_AXIS_REG,
						&data);

	return comres;
}

static int bma250_get_offset_y(struct i2c_client *client, unsigned char
						*offsetfilt)
{
	int comres = 0;
	unsigned char data = 0;

	comres = bma250_smbus_read_byte(client, BMA250_OFFSET_Y_AXIS_REG,
						&data);
	*offsetfilt = data;

	return comres;
}

static int bma250_set_offset_z(struct i2c_client *client, unsigned char
						offsetfilt)
{
	int comres = 0;
	unsigned char data = 0;

	data =  offsetfilt;
	comres = bma250_smbus_write_byte(client, BMA250_OFFSET_Z_AXIS_REG,
						&data);

	return comres;
}

static int bma250_get_offset_z(struct i2c_client *client, unsigned char
						*offsetfilt)
{
	int comres = 0;
	unsigned char data = 0;

	comres = bma250_smbus_read_byte(client, BMA250_OFFSET_Z_AXIS_REG,
						&data);
	*offsetfilt = data;

	return comres;
}
/*
static int bma250_get_offset_target(struct i2c_client *client, unsigned char
		channel, unsigned char *offset)
{
	unsigned char data;
	int comres = 0;

	switch (channel) {
	case BMA250_CUT_OFF:
		comres = bma250_smbus_read_byte(client,
				BMA250_COMP_CUTOFF__REG, &data);
		*offset = BMA250_GET_BITSLICE(data, BMA250_COMP_CUTOFF);
		break;
	case BMA250_OFFSET_TRIGGER_X:
		comres = bma250_smbus_read_byte(client,
			BMA250_COMP_TARGET_OFFSET_X__REG, &data);
		*offset = BMA250_GET_BITSLICE(data,
				BMA250_COMP_TARGET_OFFSET_X);
		break;
	case BMA250_OFFSET_TRIGGER_Y:
		comres = bma250_smbus_read_byte(client,
			BMA250_COMP_TARGET_OFFSET_Y__REG, &data);
		*offset = BMA250_GET_BITSLICE(data,
				BMA250_COMP_TARGET_OFFSET_Y);
		break;
	case BMA250_OFFSET_TRIGGER_Z:
		comres = bma250_smbus_read_byte(client,
			BMA250_COMP_TARGET_OFFSET_Z__REG, &data);
		*offset = BMA250_GET_BITSLICE(data,
				BMA250_COMP_TARGET_OFFSET_Z);
		break;
	default:
		comres = -1;
		break;
	}

	return comres;
}*/

static int bma250_set_offset_target(struct i2c_client *client, unsigned char
		channel, unsigned char offset)
{
	unsigned char data = 0;
	int comres = 0;
	switch (channel) {
	case BMA250_CUT_OFF:
		comres = bma250_smbus_read_byte(client,
				BMA250_COMP_CUTOFF__REG, &data);
		data = BMA250_SET_BITSLICE(data, BMA250_COMP_CUTOFF,
				offset);
		comres = bma250_smbus_write_byte(client,
				BMA250_COMP_CUTOFF__REG, &data);
		break;
	case BMA250_OFFSET_TRIGGER_X:
		comres = bma250_smbus_read_byte(client,
				BMA250_COMP_TARGET_OFFSET_X__REG,
				&data);
		data = BMA250_SET_BITSLICE(data,
				BMA250_COMP_TARGET_OFFSET_X,
				offset);
		comres = bma250_smbus_write_byte(client,
				BMA250_COMP_TARGET_OFFSET_X__REG,
				&data);
		break;
	case BMA250_OFFSET_TRIGGER_Y:
		comres = bma250_smbus_read_byte(client,
				BMA250_COMP_TARGET_OFFSET_Y__REG,
				&data);
		data = BMA250_SET_BITSLICE(data,
				BMA250_COMP_TARGET_OFFSET_Y,
				offset);
		comres = bma250_smbus_write_byte(client,
				BMA250_COMP_TARGET_OFFSET_Y__REG,
				&data);
		break;
	case BMA250_OFFSET_TRIGGER_Z:
		comres = bma250_smbus_read_byte(client,
				BMA250_COMP_TARGET_OFFSET_Z__REG,
				&data);
		data = BMA250_SET_BITSLICE(data,
				BMA250_COMP_TARGET_OFFSET_Z,
				offset);
		comres = bma250_smbus_write_byte(client,
				BMA250_COMP_TARGET_OFFSET_Z__REG,
				&data);
		break;
	default:
		comres = -1;
		break;
	}

	return comres;
}

static int bma250_get_cal_ready(struct i2c_client *client, unsigned char *calrdy
		)
{
	int comres = 0 ;
	unsigned char data = 0;

	comres = bma250_smbus_read_byte(client, BMA250_FAST_CAL_RDY_S__REG,
			&data);
	data = BMA250_GET_BITSLICE(data, BMA250_FAST_CAL_RDY_S);
	*calrdy = data;

	return comres;
}

static int bma250_set_cal_trigger(struct i2c_client *client, unsigned char
		caltrigger)
{
	int comres = 0;
	unsigned char data = 0;

	comres = bma250_smbus_read_byte(client, BMA250_CAL_TRIGGER__REG, &data);
	data = BMA250_SET_BITSLICE(data, BMA250_CAL_TRIGGER, caltrigger);
	comres = bma250_smbus_write_byte(client, BMA250_CAL_TRIGGER__REG,
			&data);

	return comres;
}

/*tianyu niuli add bma offset backedup on 20120823 for prod100838780 begin*/
static int ty_sensor_bma_offset_restore(struct i2c_client *client)
{
	int ret=-1;
	unsigned char offset_set_reg[4]={0};
	ret = ty_get_bma_cal_offset(offset_set_reg);
			
//	printk("%s:offset(x,y,z)(%d,%d,%d) flag=%x\n",__func__,(int)offset_set_reg[0],(int)offset_set_reg[1],(int)offset_set_reg[2],offset_set_reg[3]);
	if(offset_set_reg[3]==0x01){
		ret = bma250_set_offset_x(client,offset_set_reg[0]);
		ret = bma250_set_offset_y(client,offset_set_reg[1]);
		ret = bma250_set_offset_z(client,offset_set_reg[2]);
		if(ret<0){
			printk("%s:bma250_set_offset failed %d\n",__func__,ret);
			return ret;
		}
	}else{
		printk("%s:bma250_set_offset default value \n",__func__);
	}
	return ret;
}
/*tianyu niuli add bma offset backedup on 20120823 for prod100838780 end*/


static long
bma250_ioctl(struct file *file, unsigned int cmd,
		   unsigned long arg)
	{
		void __user *argp = (void __user *)arg;
		int ret = 0;
		int tmp_data[3];
	/*tianyu niuli add bma offset backedup on 20120823 for prod100838780 begin*/
#ifdef TYQ_USE_BMA_OFFSET
		unsigned char offset_get_reg[4] = {0,0,0,0};
		unsigned char offset_get_reg_x,offset_get_reg_y,offset_get_reg_z,offset_get_flag;
		unsigned char offset_set_reg[4] = {0,0,0,0};
		unsigned char calibrate_value = 0;
		signed char tmp = 0;
		unsigned char timeout = 0;
#endif
	/*tianyu niuli add bma offset backedup on 20120823 for prod100838780 end*/
	
		switch( cmd ){
			case BMA250_GET_DATA:
				printk(">>> %s BMA250_GET_DATA\n",__func__);
				//mutex_lock(&bma2x2->value_mutex);
				tmp_data[0] = acc_read_value.x;
				tmp_data[1] = acc_read_value.y;
				tmp_data[2] = acc_read_value.z; 	
				//mutex_unlock(&bma2x2->value_mutex);
				if (copy_to_user(argp, &tmp_data, sizeof(tmp_data))) {
					pr_info(">> %s GET_DATA error!\n",__func__);
					ret = -EFAULT;
					break;
				}
				break;
	/*tianyu niuli add bma offset backedup on 20120823 for prod100838780 begin*/
#ifdef TYQ_USE_BMA_OFFSET
			case BMA250_GET_OFFSET:
				printk(">>>%s BMA250_GET_OFFSET\n",__func__);
				/*niuli add calibration in kernel begin */
		
				calibrate_value = 0;

				if (bma250_set_offset_target(data->bma250_client, 1, calibrate_value) < 0)
					return -EINVAL;

				if (bma250_set_cal_trigger(data->bma250_client, 1) < 0)
					return -EINVAL;
	
				do {
					mdelay(2);
					bma250_get_cal_ready(data->bma250_client, &tmp);
	
			/*		printk(KERN_INFO "wait 2ms cal ready flag is %d\n", tmp);
			 */
					timeout++;
					if (timeout == 50) {
						printk(KERN_INFO "get fast calibration ready error\n");
						return -EINVAL;
					};
	
				} while (tmp == 0);
	
				printk(KERN_INFO "x axis fast calibration finished\n");
				calibrate_value = 0;
				timeout = 0;
				if (bma250_set_offset_target(data->bma250_client, 2, calibrate_value) < 0)
					return -EINVAL;
	
				if (bma250_set_cal_trigger(data->bma250_client, 2) < 0)
					return -EINVAL;
	
				do {
					mdelay(2);
					bma250_get_cal_ready(data->bma250_client, &tmp);
	
			/*		printk(KERN_INFO "wait 2ms cal ready flag is %d\n", tmp);
			 */
					timeout++;
					if (timeout == 50) {
						printk(KERN_INFO "get fast calibration ready error\n");
						return -EINVAL;
					};
	
				} while (tmp == 0);
	
				printk(KERN_INFO "y axis fast calibration finished\n");
				/*calibrate_value =2 backside z=-1g
				(ty Project:5915,5918,5920,5921,5922,5923,5927)
				calibrate_value =1 frontside z=+1g
				(ty Project:5925,5926,5928,5929,5930)
				*/
			#if defined (TYQ_BMA_BACK_SIDE_SUPPORT )
				calibrate_value = 2;// z=-1g
			#else
				calibrate_value = 1;//z=+1g
			#endif
				timeout =0;
				if (bma250_set_offset_target(data->bma250_client, 3, calibrate_value) < 0)
					return -EINVAL;
	
				if (bma250_set_cal_trigger(data->bma250_client, 3) < 0)
					return -EINVAL;
	
				do {
					mdelay(2);
					bma250_get_cal_ready(data->bma250_client, &tmp);
	
			/*		printk(KERN_INFO "wait 2ms cal ready flag is %d\n", tmp);
			 */
					timeout++;
					if (timeout == 50) {
						printk(KERN_INFO "get fast calibration ready error\n");
						return -EINVAL;
					};
	
				} while (tmp == 0);
	
				printk(KERN_INFO "z axis fast calibration finished\n");
				/*niuli add calibration in kernel end */
		
				ret = bma250_get_offset_x(data->bma250_client,&offset_get_reg_x);
				ret = bma250_get_offset_y(data->bma250_client,&offset_get_reg_y);
				ret = bma250_get_offset_z(data->bma250_client,&offset_get_reg_z);
				if(ret<0){
					printk("%s:bma250_get_offset failed %d\n",__func__,ret);
					break;
				}
	
	//			printk("%s:offset(x,y,z)(%d,%d,%d)\n",__func__,offset_get_reg_x,offset_get_reg_y,offset_get_reg_z);
				offset_get_flag = 0x01;
				offset_get_reg[0]=offset_get_reg_x;
				offset_get_reg[1]=offset_get_reg_y;
				offset_get_reg[2]=offset_get_reg_z;
				offset_get_reg[3]=offset_get_flag;
	
				ret = ty_set_bma_cal_offset(offset_get_reg);
				break;
			case BMA250_SET_OFFSET:
				printk(">>>%s BMA2X2_SET_OFFSET\n", __func__);
				ret = ty_get_bma_cal_offset(offset_set_reg);
				
	//			printk("%s:offset(x,y,z)(%d,%d,%d) flag=%x\n",__func__,(int)offset_set_reg[0],(int)offset_set_reg[1],(int)offset_set_reg[2],offset_set_reg[3]);
				if(offset_set_reg[3]==0x01){
					ret = bma250_set_offset_x(data->bma250_client,offset_set_reg[0]);
					ret = bma250_set_offset_y(data->bma250_client,offset_set_reg[1]);
					ret = bma250_set_offset_z(data->bma250_client,offset_set_reg[2]);
					if(ret<0){
						printk("%s:bma2x2_set_offset failed %d\n",__func__,ret);
					}
				}else{
					printk("%s:bma2x2_set_offset default value \n",__func__);
				}
				break;
#endif			
	/*tianyu niuli add bma offset backedup on 20120823 for prod100838780 end*/			
			default:
				printk(">>> %s invalid argument!\n",__func__);			
				ret = -EINVAL;
		}
		return ret;
	}
#endif







static void bma250_early_suspend(struct early_suspend *h)
{
	struct bma250_data *data =
	    container_of(h, struct bma250_data, early_suspend);

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		bma250_set_mode(data->bma250_client, BMA250_MODE_SUSPEND);
		cancel_delayed_work_sync(&data->work);
	}
	mutex_unlock(&data->enable_mutex);
}

static void bma250_late_resume(struct early_suspend *h)
{
	struct bma250_data *data =
	    container_of(h, struct bma250_data, early_suspend);

	mutex_lock(&data->enable_mutex);
	if (atomic_read(&data->enable) == 1) {
		bma250_set_mode(data->bma250_client, BMA250_MODE_NORMAL);
		schedule_delayed_work(&data->work,
				      msecs_to_jiffies(atomic_read
						       (&data->delay)));
	}
	mutex_unlock(&data->enable_mutex);
}

static int bma250_remove(struct i2c_client *client)
{
	struct bma250_data *data = i2c_get_clientdata(client);

	bma250_set_enable(&client->dev, 0);
	unregister_early_suspend(&data->early_suspend);
	sysfs_remove_group(&data->input->dev.kobj, &bma250_attribute_group);
	bma250_input_delete(data);
	kfree(data);
	return 0;
}

static const struct i2c_device_id bma250_id[] = {
	{SENSOR_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, bma250_id);

static struct i2c_driver bma250_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = SENSOR_NAME,
		   },
	.id_table = bma250_id,
	.probe = bma250_probe,
	.remove = bma250_remove,

};

static int __init BMA250_init(void)
{
	return i2c_add_driver(&bma250_driver);
}

static void __exit BMA250_exit(void)
{
	i2c_del_driver(&bma250_driver);
}

MODULE_AUTHOR("Albert Zhang <xu.zhang@bosch-sensortec.com>");
MODULE_DESCRIPTION("BMA250 driver");
MODULE_LICENSE("GPL");

module_init(BMA250_init);
module_exit(BMA250_exit);
