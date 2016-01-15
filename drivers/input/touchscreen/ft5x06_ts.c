/*
 *
 * FocalTech ft5x06 TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 #define TY_FT5X0X_CODE
#define FT5X0X_CREATE_APK_DEBUG_CHANNEL

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/input/ft5x06_ts.h>


#ifdef FT5X0X_CREATE_APK_DEBUG_CHANNEL
#include <linux/firmware.h>
#include <linux/syscalls.h>
#include <asm/uaccess.h>

#include <linux/wakelock.h>
#include <mach/pmic.h>
#include <linux/debugfs.h>


struct ft5x0x_debug_ctl {
	u8 		read_reg;
	bool 	update_success;
	int 	ic_type;
	struct wake_lock 	wake_lock;
	struct mutex 		device_mutex;
	struct dentry		*debugfs_root;
	struct i2c_client 	*client;
};

static struct ft5x0x_debug_ctl debug_ctl_data = {
	.read_reg = 0x00,
	.update_success = 0,
	.ic_type = IC_TYPE_MAX,
	.debugfs_root = NULL,
};

#endif /*!FT5X0X_CREATE_APK_DEBUG_CHANNEL*/


#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
/* Early-suspend level */
#define FT5X06_SUSPEND_LEVEL 1
#endif

#define CONFIG_SUPPORT_FTS_CTP_UPG

#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
#define FT_FIVE_FINGERS_SUPPORT
static int touch_ctrl_init(void *touch);

static struct ft5x06_ts_data* g_stpTouchData = NULL;
#endif

#define CFG_MAX_TOUCH_POINTS	5

#define FT_STARTUP_DLY		250
#define FT_RESET_DLY		20
#define FT_DELAY_DFLT		20
#define FT_NUM_RETRY		10

#define FT_PRESS		0x7F
#define FT_MAX_ID		0x0F
#define FT_TOUCH_STEP		6
#define FT_TOUCH_X_H_POS	3
#define FT_TOUCH_X_L_POS	4
#define FT_TOUCH_Y_H_POS	5
#define FT_TOUCH_Y_L_POS	6
#define FT_TOUCH_EVENT_POS	3
#define FT_TOUCH_ID_POS		5

#define POINT_READ_BUF	(3 + FT_TOUCH_STEP * CFG_MAX_TOUCH_POINTS)

/*register address*/
#define FT5X06_REG_IC_TYPE	0xA3
#define FT5X06_REG_PMODE	0xA5
#define FT5X06_REG_FW_VER	0xA6
#define FT5X06_REG_POINT_RATE	0x88
#define FT5X06_REG_THGROUP	0x80

/* power register bits*/
#define FT5X06_PMODE_ACTIVE		0x00
#define FT5X06_PMODE_MONITOR		0x01
#define FT5X06_PMODE_STANDBY		0x02
#define FT5X06_PMODE_HIBERNATE		0x03

#define FT5X06_VTG_MIN_UV	2600000
#define FT5X06_VTG_MAX_UV	3300000
#define FT5X06_I2C_VTG_MIN_UV	1800000
#define FT5X06_I2C_VTG_MAX_UV	1800000

struct ts_event {
	u16 x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	/* touch event: 0 -- down; 1-- contact; 2 -- contact */
	u8 touch_event[CFG_MAX_TOUCH_POINTS];
	u8 finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure;
	u8 touch_point;
};

struct ft5x06_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	const struct ft5x06_ts_platform_data *pdata;
	struct regulator *vdd;
	struct regulator *vcc_i2c;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
};

static int ft5x06_i2c_read(const struct i2c_client *client, char *writebuf,
			   int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
				 .addr = client->addr,
				 .flags = 0,
				 .len = writelen,
				 .buf = writebuf,
			 },
			{
				 .addr = client->addr,
				 .flags = I2C_M_RD,
				 .len = readlen,
				 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
				 .addr = client->addr,
				 .flags = I2C_M_RD,
				 .len = readlen,
				 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}

static int ft5x06_i2c_write(const struct i2c_client *client, char *writebuf,
			    int writelen)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
		 },
	};
	ret = i2c_transfer(client->adapter, msgs, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s: i2c write error.\n", __func__);

	return ret;
}

static void ft5x06_report_value(struct ft5x06_ts_data *data)
{
	struct ts_event *event = &data->event;
	int i;
	int fingerdown = 0;

	for (i = 0; i < event->touch_point; i++) {
		if (event->touch_event[i] == 0 || event->touch_event[i] == 2) {
			event->pressure = FT_PRESS;
			fingerdown++;
		} else {
			event->pressure = 0;
		}
		#if defined (TY_FT5X0X_CODE)
		input_report_abs(data->input_dev, ABS_MT_POSITION_X,
				 event->x[i]);
		input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
				 event->y[i]);
		input_report_abs(data->input_dev, ABS_MT_PRESSURE,
				 event->pressure);
		input_report_abs(data->input_dev, ABS_MT_TRACKING_ID,
				 event->finger_id[i]);
		input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
				 event->pressure);
		input_mt_sync(data->input_dev);
		#else
		input_mt_slot(data->input_dev, event->finger_id[i]);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER,
						!!event->pressure);
		if (event->pressure == FT_PRESS) {
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					event->x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					event->y[i]);
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
					event->pressure);
		}
		#endif
	}

	input_report_key(data->input_dev, BTN_TOUCH, !!fingerdown);
	input_sync(data->input_dev);
}

static int ft5x06_handle_touchdata(struct ft5x06_ts_data *data)
{
	struct ts_event *event = &data->event;
	int ret, i;
	u8 buf[POINT_READ_BUF] = { 0 };
	//u8 pointid = FT_MAX_ID;
	u8 point_num = 0;
	ret = ft5x06_i2c_read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	#if 1
	if((buf[0] & 0x70 )!= 0)
	{
		printk("%s:error mode.",__func__);
		return  -1;
	}
	#endif
	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = 0;
	point_num = buf[2] & 0x0f;
	if(point_num >CFG_MAX_TOUCH_POINTS)
	{
		printk("%s obtain point_num error!(point_num=%d) \n",__func__,point_num);
		return -1 ;
	}
	for (i = 0; i < point_num; i++) {
		#if 0
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
		#endif
			event->touch_point++;
		event->x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
		event->touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
	}

	ft5x06_report_value(data);

	return 0;
}

static irqreturn_t ft5x06_ts_interrupt(int irq, void *dev_id)
{
	struct ft5x06_ts_data *data = dev_id;
	int rc;

	rc = ft5x06_handle_touchdata(data);
	if (rc)
		pr_err("%s: handling touchdata failed\n", __func__);

	return IRQ_HANDLED;
}

static int ft5x06_power_on(struct ft5x06_ts_data *data, bool on)
{
	int rc;

	if (!on)
		goto power_off;

	rc = regulator_enable(data->vdd);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vdd enable failed rc=%d\n", rc);
		return rc;
	}

	rc = regulator_enable(data->vcc_i2c);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vcc_i2c enable failed rc=%d\n", rc);
		regulator_disable(data->vdd);
	}

	return rc;

power_off:
	rc = regulator_disable(data->vdd);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vdd disable failed rc=%d\n", rc);
		return rc;
	}

	rc = regulator_disable(data->vcc_i2c);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vcc_i2c disable failed rc=%d\n", rc);
		regulator_enable(data->vdd);
	}

	return rc;
}

static int ft5x06_power_init(struct ft5x06_ts_data *data, bool on)
{
	int rc;

	if (!on)
		goto pwr_deinit;

	data->vdd = regulator_get(&data->client->dev, "vdd");
	if (IS_ERR(data->vdd)) {
		rc = PTR_ERR(data->vdd);
		dev_err(&data->client->dev,
			"Regulator get failed vdd rc=%d\n", rc);
		return rc;
	}

	if (regulator_count_voltages(data->vdd) > 0) {
		rc = regulator_set_voltage(data->vdd, FT5X06_VTG_MIN_UV,
					   FT5X06_VTG_MAX_UV);
		if (rc) {
			dev_err(&data->client->dev,
				"Regulator set_vtg failed vdd rc=%d\n", rc);
			goto reg_vdd_put;
		}
	}

	data->vcc_i2c = regulator_get(&data->client->dev, "vcc_i2c");
	if (IS_ERR(data->vcc_i2c)) {
		rc = PTR_ERR(data->vcc_i2c);
		dev_err(&data->client->dev,
			"Regulator get failed vcc_i2c rc=%d\n", rc);
		goto reg_vdd_set_vtg;
	}

	if (regulator_count_voltages(data->vcc_i2c) > 0) {
		rc = regulator_set_voltage(data->vcc_i2c, FT5X06_I2C_VTG_MIN_UV,
					   FT5X06_I2C_VTG_MAX_UV);
		if (rc) {
			dev_err(&data->client->dev,
			"Regulator set_vtg failed vcc_i2c rc=%d\n", rc);
			goto reg_vcc_i2c_put;
		}
	}

	return 0;

reg_vcc_i2c_put:
	regulator_put(data->vcc_i2c);
reg_vdd_set_vtg:
	if (regulator_count_voltages(data->vdd) > 0)
		regulator_set_voltage(data->vdd, 0, FT5X06_VTG_MAX_UV);
reg_vdd_put:
	regulator_put(data->vdd);
	return rc;

pwr_deinit:
	if (regulator_count_voltages(data->vdd) > 0)
		regulator_set_voltage(data->vdd, 0, FT5X06_VTG_MAX_UV);

	regulator_put(data->vdd);

	if (regulator_count_voltages(data->vcc_i2c) > 0)
		regulator_set_voltage(data->vcc_i2c, 0, FT5X06_I2C_VTG_MAX_UV);

	regulator_put(data->vcc_i2c);
	return 0;
}

#ifdef CONFIG_PM
static int ft5x06_ts_suspend(struct device *dev)
{
	struct ft5x06_ts_data *data = dev_get_drvdata(dev);
	#if defined (TY_FT5X0X_CODE)
	char txbuf[2];
	#else
	char txbuf[2], i;
	#endif
	disable_irq(data->client->irq);
	#if defined (TY_FT5X0X_CODE)
	#else
	/* release all touches */
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		input_mt_slot(data->input_dev, i);
		input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER, 0);
	}
	#endif
	input_report_key(data->input_dev, BTN_TOUCH, 0);
	input_sync(data->input_dev);
	if (gpio_is_valid(data->pdata->reset_gpio)) {
		txbuf[0] = FT5X06_REG_PMODE;
		txbuf[1] = FT5X06_PMODE_HIBERNATE;
		ft5x06_i2c_write(data->client, txbuf, sizeof(txbuf));
	}

	return 0;
}

static int ft5x06_ts_resume(struct device *dev)
{
	struct ft5x06_ts_data *data = dev_get_drvdata(dev);

	if (gpio_is_valid(data->pdata->reset_gpio)) {
		gpio_set_value_cansleep(data->pdata->reset_gpio, 0);
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(data->pdata->reset_gpio, 1);
	}
	enable_irq(data->client->irq);

	return 0;
}
#ifdef CONFIG_HAS_EARLYSUSPEND
static void ft5x06_ts_early_suspend(struct early_suspend *handler)
{
	struct ft5x06_ts_data *data = container_of(handler,
						   struct ft5x06_ts_data,
						   early_suspend);
	ft5x06_ts_suspend(&data->client->dev);
}

static void ft5x06_ts_late_resume(struct early_suspend *handler)
{
	struct ft5x06_ts_data *data = container_of(handler,
						   struct ft5x06_ts_data,
						   early_suspend);
	ft5x06_ts_resume(&data->client->dev);
}
#endif

static const struct dev_pm_ops ft5x06_ts_pm_ops = {
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = ft5x06_ts_suspend,
	.resume = ft5x06_ts_resume,
#endif
};
#endif


#ifdef FT5X0X_CREATE_APK_DEBUG_CHANNEL
/*create apk debug channel*/
static int ft5x0x_write_reg(const struct i2c_client *client, u8 regaddr, const u8 regvalue)
{
	unsigned char buf[2] = {0};
	buf[0] = regaddr;
	buf[1] = regvalue;

	return ft5x06_i2c_write(client, buf, sizeof(buf));
}


static int ft5x0x_read_reg(const struct i2c_client *client, u8 regaddr, u8 *regvalue)
{
	return ft5x06_i2c_read(client, &regaddr, 1, regvalue, 1);
}


static int ft5x0x_ctpm_auto_clb(const struct i2c_client *client)
{
	unsigned char uc_temp = 0x00;
	unsigned char i = 0;

	/*start auto CLB */
	msleep(200);

	ft5x0x_write_reg(client, 0, FTS_FACTORYMODE_VALUE);
	/*make sure already enter factory mode */
	msleep(100);
	/*write command to start calibration */
	ft5x0x_write_reg(client, 2, 0x4);
	msleep(300);
	for (i = 0; i < 100; i++) {
		ft5x0x_read_reg(client, 0, &uc_temp);
		/*return to normal mode, calibration finish */
		if (0x0 == ((uc_temp & 0x70) >> 4))
			break;
	}

	//msleep(200);
	/*calibration OK */
	msleep(300);
	ft5x0x_write_reg(client, 0, FTS_FACTORYMODE_VALUE);	/*goto factory mode for store */
	msleep(100);	/*make sure already enter factory mode */
	ft5x0x_write_reg(client, 2, 0x5);	/*store CLB result */
	msleep(300);
	ft5x0x_write_reg(client, 0, FTS_WORKMODE_VALUE);	/*return to normal mode */
	msleep(300);

	/*store CLB result OK */
	return 0;
}

static int ft5x0x_get_upgrade_info(struct Upgrade_Info *upgrade_info)
{
	switch (debug_ctl_data.ic_type) {
	case IC_FT5306I:
		upgrade_info->delay_55 = FT5X06_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5X06_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5X06_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5X06_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5X06_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT5X06_UPGRADE_EARSE_DELAY;
		pr_debug("%s: Device type IC_FT5X06.\n",__func__);
		break;
	case IC_FT5316:
		upgrade_info->delay_55 = FT5316_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5316_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5316_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5316_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5316_UPGRADE_READID_DELAY;
		upgrade_info->delay_earse_flash = FT5316_UPGRADE_EARSE_DELAY;
		pr_debug("%s: Device type IC_FT5316.\n",__func__);
		break;
	default:
		upgrade_info->delay_55 = 0;
		upgrade_info->delay_aa = 0;
		upgrade_info->upgrade_id_1 = 0;
		upgrade_info->upgrade_id_2 = 0;
		upgrade_info->delay_readid = 0;
		upgrade_info->delay_earse_flash = 0;
		pr_debug("%s: Unknow device type!\n",__func__);
		return -EINVAL;
	}
	return 0;
}


static int ft5x0x_ctpm_fw_upgrade(const struct i2c_client *client, const u8 *pbt_buf,
			  u32 dw_lenth)
{
	u8 reg_val[2] = {0};
	u32 i = 0;
	u32 packet_number;
	u32 j;
	u32 temp;
	u32 lenght;
	u8 packet_buf[FTS_PACKET_LENGTH + 6];
	u8 auc_i2c_write_buf[10];
	u8 bt_ecc;
	int i_ret;
	struct Upgrade_Info upgradeinfo;

	if(ft5x0x_get_upgrade_info(&upgradeinfo)){
		dev_err(&client->dev, "Cannot get upgrade information!\n");
		return -EINVAL;
	}

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
		/*********Step 1:Reset  CTPM *****/
		dev_info(&client->dev,"Update Step 1:Reset  CTPM.\n");
		/*write 0xaa to register 0xfc */
		ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_AA);
		msleep(upgradeinfo.delay_aa);

		/*write 0x55 to register 0xfc */
		ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_55);
		msleep(upgradeinfo.delay_55);
		/*********Step 2:Enter upgrade mode *****/
		dev_info(&client->dev,"Update Step 2:Entering upgrade mode.\n");
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		do {
			i++;
			i_ret = ft5x06_i2c_write(client, auc_i2c_write_buf, 2);
			msleep(5);
		} while (i_ret <= 0 && i < 5);


		/*********Step 3:check READ-ID***********************/
		msleep(upgradeinfo.delay_readid);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =
			0x00;
		ft5x06_i2c_read(client, auc_i2c_write_buf, 4, reg_val, 2);


		if (reg_val[0] == upgradeinfo.upgrade_id_1
			&& reg_val[1] == upgradeinfo.upgrade_id_2) {
			dev_info(&client->dev,"Update Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x.\n",
				reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev, "ERROR Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x, updateID1=0x%x, ID2=0x%x.\n",
				reg_val[0], reg_val[1], upgradeinfo.upgrade_id_1, upgradeinfo.upgrade_id_2);
		}
	}
	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;
	auc_i2c_write_buf[0] = 0xcd;

	ft5x06_i2c_read(client, auc_i2c_write_buf, 1, reg_val, 1);


	/*Step 4:erase app and panel paramenter area*/
	dev_info(&client->dev,"Update Step 4:erase app and panel paramenter area.\n");
	auc_i2c_write_buf[0] = 0x61;
	ft5x06_i2c_write(client, auc_i2c_write_buf, 1);	/*erase app area */
	msleep(upgradeinfo.delay_earse_flash);
	/*erase panel parameter area */
	auc_i2c_write_buf[0] = 0x63;
	ft5x06_i2c_write(client, auc_i2c_write_buf, 1);
	msleep(100);

	/*********Step 5:write firmware(FW) to ctpm flash*********/
	bt_ecc = 0;
	dev_info(&client->dev,"Update Step 5:write firmware(FW) to ctpm flash.\n");

	dw_lenth = dw_lenth - 8;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;

	for (j = 0; j < packet_number; j++) {
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		lenght = FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (lenght >> 8);
		packet_buf[5] = (u8) lenght;

		for (i = 0; i < FTS_PACKET_LENGTH; i++) {
			packet_buf[6 + i] = pbt_buf[j * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}

		ft5x06_i2c_write(client, packet_buf, FTS_PACKET_LENGTH + 6);
		msleep(FTS_PACKET_LENGTH / 6 + 1);
		//DBG("write bytes:0x%04x\n", (j+1) * FTS_PACKET_LENGTH);
		//delay_qt_ms(FTS_PACKET_LENGTH / 6 + 1);
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0) {
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (u8) (temp >> 8);
		packet_buf[5] = (u8) temp;

		for (i = 0; i < temp; i++) {
			packet_buf[6 + i] = pbt_buf[packet_number * FTS_PACKET_LENGTH + i];
			bt_ecc ^= packet_buf[6 + i];
		}

		ft5x06_i2c_write(client, packet_buf, temp + 6);
		msleep(20);
	}


	/*send the last six byte */
	for (i = 0; i < 6; i++) {
		temp = 0x6ffa + i;
		packet_buf[2] = (u8) (temp >> 8);
		packet_buf[3] = (u8) temp;
		temp = 1;
		packet_buf[4] = (u8) (temp >> 8);
		packet_buf[5] = (u8) temp;
		packet_buf[6] = pbt_buf[dw_lenth + i];
		bt_ecc ^= packet_buf[6];
		ft5x06_i2c_write(client, packet_buf, 7);
		msleep(20);
	}


	/*********Step 6: read out checksum***********************/
	/*send the opration head */
	dev_info(&client->dev,"Update Step 6: read out checksum.\n");
	auc_i2c_write_buf[0] = 0xcc;
	ft5x06_i2c_read(client, auc_i2c_write_buf, 1, reg_val, 1);
	if (reg_val[0] != bt_ecc) {
		dev_err(&client->dev, "[FTS]--ecc error! FW=%02x bt_ecc=%02x.\n",
					reg_val[0],
					bt_ecc);
		return -EIO;
	}

	/*********Step 7: reset the new FW***********************/
	dev_info(&client->dev,"Update Step 7: reset the new FW.\n");
	auc_i2c_write_buf[0] = 0x07;
	ft5x06_i2c_write(client, auc_i2c_write_buf, 1);
	msleep(300);	/*make sure CTP startup normally */

	return 0;
}


/*
upgrade with *.bin file
*/

static int ft5x0x_ctpm_fw_upgrade_with_app_file(struct i2c_client *client,
				      const char *firmware_name)
{
	int i_ret;
	u8 read_val = 0;
	const struct firmware *fw = NULL;

	i_ret = request_firmware(&fw, firmware_name, &client->dev);
	if (i_ret) {
		dev_err(&client->dev, "%s:Unable to request firmware, ret=%d\n",
			__func__, i_ret);
		dev_err(&client->dev, "fw name=(%s).\n", firmware_name);
		return -EIO;
	}

	if (fw->size < 8 || fw->size > 32 * 1024) {
		dev_dbg(&client->dev, "%s:FW length error\n", __func__);
		i_ret = -EIO;
		goto release_firmware;
	}

	/* check firmware version */
	i_ret = ft5x0x_read_reg(client, FT5X06_REG_FW_VER, &read_val);
	if(i_ret < 0){
		dev_err(&client->dev, "%s ERROR:Get firmware version failed.\n",__func__);
		i_ret = -EFAULT;
		goto release_firmware;
	}

	if(read_val == fw->data[fw->size - 1]){
		dev_err(&client->dev, "%s ERROR:Same Firmware version(%d), no need to update.\n",
					__func__, read_val);
		i_ret = -EFAULT;
		goto release_firmware;
	}

	i_ret = ft5x0x_read_reg(client, FT5X06_REG_IC_TYPE, &read_val);
	if(i_ret < 0){
		dev_err(&client->dev, "%s ERROR:Get IC type failed.\n",__func__);
		i_ret = -EFAULT;
		goto release_firmware;
	}

	debug_ctl_data.ic_type = read_val;

	if ((fw->data[fw->size - 8] ^ fw->data[fw->size - 6]) == 0xFF
		&& (fw->data[fw->size - 7] ^ fw->data[fw->size - 5]) == 0xFF
		&& (fw->data[fw->size - 3] ^ fw->data[fw->size - 4]) == 0xFF) {
		/*call the upgrade function */
		i_ret = ft5x0x_ctpm_fw_upgrade(client, fw->data, fw->size);
		if (i_ret)
			dev_dbg(&client->dev, "%s() - ERROR:upgrade failed.\n",
						__func__);
		else {
			ft5x0x_ctpm_auto_clb(client);	/*start auto CLB*/
		 }
	} else {
		dev_dbg(&client->dev, "%s:FW format error\n", __func__);
		i_ret = -EIO;
	}

release_firmware:
	release_firmware(fw);
	return i_ret;

}


static ssize_t ft5x0x_update_fw_write(struct file *file,const char __user *userbuf,
				 size_t count, loff_t *ppos)
{
	struct i2c_client *client = debug_ctl_data.client;
	unsigned char *writebuf;
	int buflen = count;
	int ret = 0;

	if( FTS_PACKET_LENGTH - 1 < buflen){
		dev_err(&client->dev, "%s:not enough buffer to copy data from user.\n", __func__);
		return -EFAULT;
	}

	writebuf = kmalloc(FTS_PACKET_LENGTH,GFP_KERNEL);
	if(writebuf == NULL){
		dev_err(&client->dev, "%s:cannot alloc enough memory.\n", __func__);
		return -EFAULT;
	}
	memset(writebuf,0,FTS_PACKET_LENGTH);

	if (copy_from_user(writebuf, userbuf, buflen)) {
		dev_err(&client->dev, "%s:copy from user error.\n", __func__);
		ret = -EFAULT;
		goto free_mem;
	}

	dev_info(&client->dev,"%s:input string=(%s)\n",__func__, writebuf);

	mutex_lock(&debug_ctl_data.device_mutex);
	wake_lock(&debug_ctl_data.wake_lock);

	disable_irq(client->irq);
	ret = ft5x0x_ctpm_fw_upgrade_with_app_file(client, writebuf);

	enable_irq(client->irq);
	if (ret < 0) {
		debug_ctl_data.update_success = 0;
		dev_err(&client->dev, "%s:upgrade failed.\n", __func__);
		goto free_lock;
	}
	else{
		debug_ctl_data.update_success = 1;
		dev_info(&client->dev, "%s upgrade success.\n", __func__);
	}

	wake_unlock(&debug_ctl_data.wake_lock);
	mutex_unlock(&debug_ctl_data.device_mutex);
	kfree(writebuf);

	return count;

free_lock:
	wake_unlock(&debug_ctl_data.wake_lock);
	mutex_unlock(&debug_ctl_data.device_mutex);

free_mem:
	kfree(writebuf);
	return ret;
}

static ssize_t ft5x0x_update_fw_read(struct file *file, char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct i2c_client *client = debug_ctl_data.client;
	u8	reg_val = 0;
	int ret = 0;
	int num_read_chars = 0;
	unsigned char out_buf[50];

	mutex_lock(&debug_ctl_data.device_mutex);
	wake_lock(&debug_ctl_data.wake_lock);
	if(debug_ctl_data.update_success){
		/*after calling ft5x0x_debug_write to upgrade*/
		ret = ft5x0x_read_reg(client, FT5X06_REG_FW_VER, &reg_val);
		if (ret < 0)
			num_read_chars = sprintf(out_buf, "Get fw version failed.\n");
		else
			num_read_chars = sprintf(out_buf, "Update success! Current firmware version:0x%02x\n", reg_val);
	}
	else{
		num_read_chars = sprintf(out_buf, "Update firmware fail!.\n");
	}
	mutex_unlock(&debug_ctl_data.device_mutex);
	wake_unlock(&debug_ctl_data.wake_lock);

	if( sizeof(out_buf) < num_read_chars){
		dev_err(&client->dev, "%s:Output buffer overflow! data size=%d.\n", __func__, num_read_chars);
		return -EFAULT;
	}

	return simple_read_from_buffer(user_buf, count, ppos, out_buf, num_read_chars);
}

static const struct file_operations firmware_update ={
	.write = ft5x0x_update_fw_write,
	.read = ft5x0x_update_fw_read,
	.open = simple_open,
	.owner = THIS_MODULE,
};

static ssize_t ft5x0x_read_reg_write(struct file *file,const char __user *userbuf,
				 size_t count, loff_t *ppos)
{
	struct i2c_client *client = debug_ctl_data.client;
	unsigned char writebuf[16];
	u8 read_reg = 0;

	if( (sizeof(writebuf) - 1) < count){
		dev_err(&client->dev, "%s:not enough buffer to copy, count=%d.\n", __func__, count);
		return -EFAULT;
	}

	memset(writebuf, 0, sizeof(writebuf));

	if (copy_from_user(writebuf, userbuf, count)){
		dev_err(&client->dev, "%s:copy from user failed..\n", __func__);
		return -EFAULT;
	}

	dev_info(&client->dev,"%s:input string=%s",__func__, writebuf);

	mutex_lock(&debug_ctl_data.device_mutex);
	wake_lock(&debug_ctl_data.wake_lock);
	if(!kstrtou8(writebuf, 0, &read_reg)){
		dev_info(&client->dev, "%s read register %d.\n", __func__, read_reg);
		debug_ctl_data.read_reg = read_reg;
	}
	else{
		dev_err(&client->dev,"%s:invalid register number.\n", __func__);
	}
	wake_unlock(&debug_ctl_data.wake_lock);
	mutex_unlock(&debug_ctl_data.device_mutex);

	return count;
}

static ssize_t ft5x0x_read_reg_read(struct file *file, char __user *user_buf,
						size_t count, loff_t *ppos)
{
	struct i2c_client *client = debug_ctl_data.client;
	int ret = 0;
	int num_read_chars = 0;
	u8	reg_value = 0;
	unsigned char out_buf[50];

	mutex_lock(&debug_ctl_data.device_mutex);
	wake_lock(&debug_ctl_data.wake_lock);
	/* return status and register value of last read */
	ret = ft5x0x_read_reg(client, debug_ctl_data.read_reg, &reg_value);
	if ( ret < 0 ) {
		num_read_chars = sprintf(out_buf, "Get register failed!.\n");
	}else{
		num_read_chars = sprintf(out_buf, "Register(0x%x)=0x%02x.\n", debug_ctl_data.read_reg,reg_value);
	}
	mutex_unlock(&debug_ctl_data.device_mutex);
	wake_unlock(&debug_ctl_data.wake_lock);

	if( sizeof(out_buf) < num_read_chars){
		dev_err(&client->dev, "%s:Output buffer overflow! data size=%d.\n", __func__, num_read_chars);
		return -EFAULT;
	}

	return simple_read_from_buffer(user_buf, count, ppos, out_buf, num_read_chars);
}

static const struct file_operations read_register ={
	.write = ft5x0x_read_reg_write,
	.read = ft5x0x_read_reg_read,
	.open = simple_open,
	.owner = THIS_MODULE,
};

static ssize_t ft5x0x_detection_pin_read(struct file *file, char __user *user_buf,
						size_t count, loff_t *ppos)
{
	char buf[3];
	int ret;

	pmic_gpio_direction_input(FT5X0X_DETECTION_PIN_NUM);
	ret = pmic_gpio_get_value(FT5X0X_DETECTION_PIN_NUM);
	if(ret < 0){
		return ret;
	}
	else{
		strncpy(buf, ret ? "1\n": "0\n" , sizeof(buf));
	}
	return simple_read_from_buffer(user_buf, count, ppos, buf, sizeof(buf));
}

static const struct file_operations detection_pin_ops ={
	.read = ft5x0x_detection_pin_read,
	.open = simple_open,
	.owner = THIS_MODULE,
};


int ft5x0x_create_apk_debug_channel(struct i2c_client *client)
{
	debug_ctl_data.debugfs_root = debugfs_create_dir(FT5X0X_DEBUG_IF_NAME, NULL);
	if(debug_ctl_data.debugfs_root){
		debugfs_create_file("detection_pin", S_IFREG | S_IRUGO, debug_ctl_data.debugfs_root, NULL, &detection_pin_ops);
		debugfs_create_file("firmware_update", S_IFREG | S_IWUGO | S_IRUGO, debug_ctl_data.debugfs_root, NULL, &firmware_update);
		debugfs_create_file("read_register", S_IFREG | S_IWUGO | S_IRUGO, debug_ctl_data.debugfs_root, NULL, &read_register);

		debug_ctl_data.client = client;
		dev_info(&client->dev, "Create debugfs file success!\n");
	}
	else{
		dev_err(&client->dev, "Couldn't create debugfs entry!\n");
	}

	wake_lock_init(&debug_ctl_data.wake_lock, WAKE_LOCK_SUSPEND, FT5X0X_DEBUG_IF_NAME);
	mutex_init(&debug_ctl_data.device_mutex);

	return 0;
}

void ft5x0x_release_apk_debug_channel(void)
{
	mutex_destroy(&debug_ctl_data.device_mutex);
	wake_lock_destroy(&debug_ctl_data.wake_lock);

	debugfs_remove_recursive(debug_ctl_data.debugfs_root);
}
#endif /* !FT5X0X_CREATE_APK_DEBUG_CHANNEL */


static int ft5x06_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	const struct ft5x06_ts_platform_data *pdata = client->dev.platform_data;
	struct ft5x06_ts_data *data;
	struct input_dev *input_dev;
	u8 reg_value;
	u8 reg_addr;
	int err;
	int tries;

	if (!pdata) {
		dev_err(&client->dev, "Invalid pdata\n");
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "I2C not supported\n");
		return -ENODEV;
	}

	data = kzalloc(sizeof(struct ft5x06_ts_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Not enough memory\n");
		return -ENOMEM;
	}
	#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
	g_stpTouchData = data;
	#endif
	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto free_mem;
	}

	data->input_dev = input_dev;
	data->client = client;
	data->pdata = pdata;

	input_dev->name = "ft5x06_ts";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;

	input_set_drvdata(input_dev, data);
	i2c_set_clientdata(client, data);

	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);
	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	#if defined (TY_FT5X0X_CODE)
	input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0,
			     CFG_MAX_TOUCH_POINTS, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, FT_PRESS, 0, 0);
	#else
	input_mt_init_slots(input_dev, CFG_MAX_TOUCH_POINTS);
	#endif
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0,
			     pdata->x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0,
			     pdata->y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, FT_PRESS, 0, 0);

	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev, "Input device registration failed\n");
		goto free_inputdev;
	}

	if (pdata->power_init) {
		err = pdata->power_init(true);
		if (err) {
			dev_err(&client->dev, "power init failed");
			goto unreg_inputdev;
		}
	} else {
		err = ft5x06_power_init(data, true);
		if (err) {
			dev_err(&client->dev, "power init failed");
			goto unreg_inputdev;
		}
	}

	if (pdata->power_on) {
		err = pdata->power_on(true);
		if (err) {
			dev_err(&client->dev, "power on failed");
			goto pwr_deinit;
		}
	} else {
		err = ft5x06_power_on(data, true);
		if (err) {
			dev_err(&client->dev, "power on failed");
			goto pwr_deinit;
		}
	}

	if (gpio_is_valid(pdata->irq_gpio)) {
		err = gpio_request(pdata->irq_gpio, "ft5x06_irq_gpio");
		if (err) {
			dev_err(&client->dev, "irq gpio request failed");
			goto pwr_off;
		}
		err = gpio_direction_input(pdata->irq_gpio);
		if (err) {
			dev_err(&client->dev,
				"set_direction for irq gpio failed\n");
			goto free_irq_gpio;
		}
	}

	if (gpio_is_valid(pdata->reset_gpio)) {
		err = gpio_request(pdata->reset_gpio, "ft5x06_reset_gpio");
		if (err) {
			dev_err(&client->dev, "reset gpio request failed");
			goto free_irq_gpio;
		}

		err = gpio_direction_output(pdata->reset_gpio, 0);
		if (err) {
			dev_err(&client->dev,
				"set_direction for reset gpio failed\n");
			goto free_reset_gpio;
		}
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(data->pdata->reset_gpio, 1);
	}

	/* make sure CTP already finish startup process */
	msleep(FT_STARTUP_DLY);

	/*get some register information */
	/* read firmware version */
	reg_addr = FT5X06_REG_FW_VER;
	tries = FT_NUM_RETRY;
	do {
		err = ft5x06_i2c_read(client, &reg_addr, 1, &reg_value, 1);
		msleep(FT_DELAY_DFLT);
	} while ((err < 0) && tries--);

	if (err < 0) {
		dev_err(&client->dev, "version read failed");
		goto free_reset_gpio;
	}
	dev_info(&client->dev, "[FTS] Firmware version = 0x%x\n", reg_value);

	/* Read report rate */
	reg_addr = FT5X06_REG_POINT_RATE;
	tries = FT_NUM_RETRY;
	do {
		err = ft5x06_i2c_read(client, &reg_addr, 1, &reg_value, 1);
		msleep(FT_DELAY_DFLT);
	} while ((err < 0) && tries--);

	if (err < 0) {
		dev_err(&client->dev, "report rate read failed");
		goto free_reset_gpio;
	}
	dev_info(&client->dev, "[FTS] report rate is %dHz.\n", reg_value * 10);

	/* read touch threshold */
	reg_addr = FT5X06_REG_THGROUP;
	tries = FT_NUM_RETRY;
	do {
		err = ft5x06_i2c_read(client, &reg_addr, 1, &reg_value, 1);
		msleep(FT_DELAY_DFLT);
	} while ((err < 0) && tries--);

	if (err < 0) {
		dev_err(&client->dev, "threshold read failed");
		goto free_reset_gpio;
	}
	dev_dbg(&client->dev, "[FTS] touch threshold is %d.\n", reg_value * 4);

	/* Requesting irq */
	err = request_threaded_irq(client->irq, NULL,
				   ft5x06_ts_interrupt, pdata->irqflags,
				   client->dev.driver->name, data);
	if (err) {
		dev_err(&client->dev, "request irq failed\n");
		goto free_reset_gpio;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN +
	    FT5X06_SUSPEND_LEVEL;
	data->early_suspend.suspend = ft5x06_ts_early_suspend;
	data->early_suspend.resume = ft5x06_ts_late_resume;
	register_early_suspend(&data->early_suspend);
#endif
	#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
	touch_ctrl_init(data);
	#endif
	return 0;

free_reset_gpio:
	if (gpio_is_valid(pdata->reset_gpio))
		gpio_free(pdata->reset_gpio);
free_irq_gpio:
	if (gpio_is_valid(pdata->irq_gpio))
		gpio_free(pdata->reset_gpio);
pwr_off:
	if (pdata->power_on)
		pdata->power_on(false);
	else
		ft5x06_power_on(data, false);
pwr_deinit:
	if (pdata->power_init)
		pdata->power_init(false);
	else
		ft5x06_power_init(data, false);
unreg_inputdev:
	input_unregister_device(input_dev);
	input_dev = NULL;
free_inputdev:
	input_free_device(input_dev);
free_mem:
	kfree(data);
	return err;
}

static int __devexit ft5x06_ts_remove(struct i2c_client *client)
{
	struct ft5x06_ts_data *data = i2c_get_clientdata(client);

#ifdef FT5X0X_CREATE_APK_DEBUG_CHANNEL
	ft5x0x_release_apk_debug_channel();
#endif /*!FT5X0X_CREATE_APK_DEBUG_CHANNEL*/

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	free_irq(client->irq, data);

	if (gpio_is_valid(data->pdata->reset_gpio))
		gpio_free(data->pdata->reset_gpio);

	if (gpio_is_valid(data->pdata->irq_gpio))
		gpio_free(data->pdata->reset_gpio);

	if (data->pdata->power_on)
		data->pdata->power_on(false);
	else
		ft5x06_power_on(data, false);

	if (data->pdata->power_init)
		data->pdata->power_init(false);
	else
		ft5x06_power_init(data, false);

	input_unregister_device(data->input_dev);
	kfree(data);

	return 0;
}

static const struct i2c_device_id ft5x06_ts_id[] = {
	{"ft5x06_ts", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, ft5x06_ts_id);

static struct i2c_driver ft5x06_ts_driver = {
	.probe = ft5x06_ts_probe,
	.remove = __devexit_p(ft5x06_ts_remove),
	.driver = {
		   .name = "ft5x06_ts",
		   .owner = THIS_MODULE,
#ifdef CONFIG_PM
		   .pm = &ft5x06_ts_pm_ops,
#endif
		   },
	.id_table = ft5x06_ts_id,
};

static int __init ft5x06_ts_init(void)
{
	return i2c_add_driver(&ft5x06_ts_driver);
}


static void __exit ft5x06_ts_exit(void)
{
	i2c_del_driver(&ft5x06_ts_driver);
}


///////////////////////////////////////////////////////////////////////
/*
* firmware upgrade area
*/

//*****************************************************************************//
	

#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit

#define FTS_NULL               0x0
#define FTS_TRUE               0x01
#define FTS_FALSE              0x0

#define I2C_CTPM_ADDRESS       0x76
#define FT_DEFAULT_POINTERS     CFG_MAX_TOUCH_POINTS
//interrupt mode setting
#define INTERRUPT_TRIGGER_MODE   1
#define INTERRUPT_QUERY_MODE     0 
#define INTERRUPT_MODE_TRIGGER

#define TRUlY_VENDOR_ID_NO		0x5a
#define TRUlY_VENDOR_ID_STR		"TRULY_TP"
#define TRUlY_VENDOR_ID_NO_2		0xda
#define TRUlY_VENDOR_ID_STR_2		"TRULY_TP-2"
#define BYD_VENDOR_ID_NO		0x59
#define BYD_VENDOR_ID_STR		"BYD_TP"
#define O_FILM_VENDOR_ID_NO		0x51
#define O_FILM_VENDOR_ID_STR		"O-FILM_TP"
#define O_FILM_VENDOR_ID_NO_2		0xd1
#define O_FILM_VENDOR_ID_STR_2		"O-FILM_TP-2"
#define BM_VENDOR_ID_NO		0x5d
#define BM_VENDOR_ID_STR		"BM_TP"
#define LCE_VENDOR_ID_NO		0x87
#define LCE_VENDOR_ID_STR		"LCE_TP"




typedef enum
{
	ERR_OK,
	ERR_MODE,
	ERR_READID,
	ERR_ERASE,
	ERR_STATUS,
	ERR_ECC
}E_UPGRADE_ERR_TYPE;

struct ty_touch_fmupgrade_S
{
  unsigned int	touchType;
  unsigned long ulCRC;
  unsigned long ulReserved;
  unsigned long bufLength;
  void* 		bufAddr;
};

DEFINE_SEMAPHORE(upgrade_lock);
static int g_iINTMode =  INTERRUPT_TRIGGER_MODE;

static int   g_iMaxFingers = CFG_MAX_TOUCH_POINTS;

static int g_bConfigPointer = FTS_FALSE;
static int g_bConfigINTMode = FTS_FALSE;
u8   g_u8UpgradeFWResult = 0;

static int fts_i2c_read(
	struct ft5x06_ts_data* hTouch,
    u8  reg,
    u8* buffer,
    u32 len)
{
	int ret;
	ret = ft5x06_i2c_read(hTouch->client,&reg,1, buffer, len);
	if(ret<0)
		return ret;
	else
	 	return FTS_TRUE;
}

static int fts_i2c_write(
    struct ft5x06_ts_data* hTouch,
    u8  reg,
    u8* buffer,
    u32 len)
{
	int ret;
	ret = ft5x06_i2c_write(hTouch->client, buffer,len);
	if(ret < 0 ) 
		return ret;
	else
	 	return FTS_TRUE;
}

int ty_ft5x0x_write_reg(struct ft5x06_ts_data* hTouch, u8 regaddr, u8 regvalue)
{
	unsigned char buf[2] = {0};
	buf[0] = regaddr;
	buf[1] = regvalue;

	return fts_i2c_write(hTouch,0,buf,sizeof(buf));
}


int ty_ft5x0x_read_reg(struct ft5x06_ts_data* hTouch, u8 regaddr, u8 *regvalue)
{
	return fts_i2c_read(hTouch,regaddr,regvalue,1);//ft5x0x_i2c_Read(client, &regaddr, 1, regvalue, 1);
}

int fts_ctpm_auto_clb(struct ft5x06_ts_data* hTouch)
{
	unsigned char uc_temp = 0x00;
	int  i = 0;

	/*start auto CLB */
	msleep(500);

	ty_ft5x0x_write_reg(hTouch, 0, 0x40);
	/*make sure already enter factory mode */
	msleep(300);
	/*write command to start calibration */
	ty_ft5x0x_write_reg(hTouch, 2, 0x4);
	msleep(300);
	for (i = 0; i < 300; i++) {
		ty_ft5x0x_read_reg(hTouch, 0, &uc_temp);
		/*return to normal mode, calibration finish */
		if (0x0 == ((uc_temp & 0x70) >> 4))
		{
			printk("ft calibration finish !\n");
			break;
		}
	}

	//msleep(200);
	/*calibration OK */
	msleep(300);
	ty_ft5x0x_write_reg(hTouch, 0, 0x40);	/*goto factory mode for store */
	msleep(100);	/*make sure already enter factory mode */
	ty_ft5x0x_write_reg(hTouch, 2, 0x5);	/*store CLB result */
	msleep(300);
	ty_ft5x0x_write_reg(hTouch, 0, 0x00);	/*return to normal mode */
	msleep(300);

	/*store CLB result OK */
	return 0;
}

void delay_qt_ms(unsigned long  w_ms)
{


	msleep(w_ms);
	return;
}

#ifdef TP_UPGRADE_DEBUG

#define fts_debug(fmt, args...) \
	do {\
			printk("fts: "fmt, ##args);\
	} while (0)
#else

#define fts_debug(fmt, args...)

#endif 


/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
static FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
	
    //ret=fts_i2c_read(g_stpTouchData, pbt_buf[0],&pbt_buf[1], dw_lenth-1);
    ret = i2c_master_recv(g_stpTouchData->client,pbt_buf, dw_lenth);
    if(ret<=0)
    {
        printk("[TSP]i2c_read_interface error\n");
        return FTS_FALSE;
    }

    return FTS_TRUE;
}

/*
[function]: 
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
static FTS_BOOL i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    
	//ret = fts_i2c_write(g_stpTouchData, pbt_buf[0],&pbt_buf[1], dw_lenth-1);
	ret = i2c_master_send(g_stpTouchData->client,pbt_buf, dw_lenth);
	if(ret < 0)
	{
		
		printk("%s fail err = %x!!!\n", __func__,  ret);
		return 0;
	}
	
    return 1; 
}


/*
[function]: 
	write a value to register.
[parameters]:
	e_reg_name[in]	:register name;
	pbt_buf[in]		:the returned register value;
[return]:
	FTS_TRUE	:success;
	FTS_FALSE	:io fail;
*/
#if 0
static FTS_BOOL fts_register_write(FTS_BYTE e_reg_name, FTS_BYTE b_data)
{
	FTS_BYTE write_cmd[2] = {0};

	write_cmd[0] = e_reg_name;
	write_cmd[1] = b_data;

	/*call the write callback function*/
	return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, 2);
	
}
#endif

/*
[function]: 
    send a command to ctpm.
[parameters]:
    btcmd[in]        :command code;
    btPara1[in]    :parameter 1;    
    btPara2[in]    :parameter 2;    
    btPara3[in]    :parameter 3;    
    num[in]        :the valid input parameter numbers, if only command code needed and no parameters followed,then the num is 1;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
static FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE btPara3,FTS_BYTE num)
{
    FTS_BYTE write_cmd[4] = {0};

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
    return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

/*
[function]: 
    write data to ctpm , the destination address is 0.
[parameters]:
    pbt_buf[in]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
static FTS_BOOL byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
    
    return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

/*
[function]: 
    read out data from ctpm,the destination address is 0.
[parameters]:
    pbt_buf[out]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
static FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
    return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}

static FTS_BOOL fts_Upgraded(void)
{
	u8   u8Val;
    u8   u8SleepCommand[2] = {0}; 

	if(!g_stpTouchData)
	{
		printk("%s:g_stpTouchData is null.\n",__func__);
		return 0;
	}

	fts_debug("enter %s.\n",__func__);	
	//NvOsSleepMS(50);
	
#ifdef FT_FIVE_FINGERS_SUPPORT

	mdelay(1);
	u8Val = 0xFE;
	
	//version
	if(!fts_i2c_read(g_stpTouchData, 0xA6, &u8Val, 1))
	{
		fts_debug("%s:get FW ver faild.\n",__func__);
		u8Val = 0x01;
	}
	fts_debug("FT_Upgraded:Version=0x%x \n",u8Val);
	
	if((u8Val >= 0x1a))
	{
		mdelay(1);
		u8Val = 0xFE;
		if( FTS_TRUE == fts_i2c_read(g_stpTouchData, 0x90, &u8Val, 1))
		{
			if(g_iMaxFingers != u8Val)
			{
				mdelay(1);
				g_bConfigPointer = FTS_TRUE;
				
				u8SleepCommand[0] = 0x90;
				u8SleepCommand[1] = g_iMaxFingers;
			    fts_i2c_write(g_stpTouchData, 0x0, u8SleepCommand, 2);
				fts_debug("FT_Upgraded:Fingers=0x%x,fwfingers=0x%x\n",g_iMaxFingers,u8Val);
			}
		}
		else
		{
			fts_debug("%s:get fingers faild.\n",__func__);
			u8Val = FT_DEFAULT_POINTERS;
		}
		fts_debug("FT_Upgraded:Fingers=0x%x \n",g_iMaxFingers);
	}
#endif	
	
	u8Val = 0xFE;
	if(!fts_i2c_read(g_stpTouchData, 0xA4, &u8Val, 1))
	{
		printk("%s:get FW ver faild.\n",__func__);
	}
	fts_debug("fts_Upgraded:IntMode=0x%x \n",u8Val);
	
#ifdef INTERRUPT_MODE_QUERY
	if(u8Val == INTERRUPT_TRIGGER_MODE )
	{
		msleep(50);
		//write
		u8SleepCommand[0] = 0xa4;
		u8SleepCommand[1] = INTERRUPT_QUERY_MODE; //QUERY
	    fts_i2c_write(g_stpTouchData, 0x0, u8SleepCommand, 2);	

		msleep(50);
		
		//read&check
		u8Val = 20;
		fts_i2c_read(g_stpTouchData, 0xa4, &u8Val, 1);
		fts_debug("fts_Upgraded:intmode=0x%x\n",u8Val);
		g_iINTMode = INTERRUPT_QUERY_MODE;
		g_bConfigINTMode = FTS_TRUE;
	}
#endif

#ifdef INTERRUPT_MODE_TRIGGER
	if(u8Val == INTERRUPT_QUERY_MODE )
	{
		msleep(50);

		//write
		u8SleepCommand[0] = 0xa4;
		u8SleepCommand[1] = INTERRUPT_TRIGGER_MODE; 
	    fts_i2c_write(g_stpTouchData, 0x0, u8SleepCommand, 2);	

		msleep(50);
		//read&check
		u8Val = 20;
		fts_i2c_read(g_stpTouchData, 0xa4, &u8Val, 1);
		fts_debug("fts_Upgraded:intmode=0x%x\n",u8Val);
		g_iINTMode = INTERRUPT_TRIGGER_MODE;
		g_bConfigINTMode = FTS_TRUE;
	}
#endif

	return FTS_TRUE;
	
}


/*
[function]: 
    burn the FW to ctpm.
[parameters]:(ref. SPEC)
    pbt_buf[in]    :point to Head+FW ;
    dw_lenth[in]:the length of the FW + 6(the Head length);    
    bt_ecc[in]    :the ECC of the FW
[return]:
    ERR_OK        :no error;
    ERR_MODE    :fail to switch to UPDATE mode;
    ERR_READID    :read id fail;
    ERR_ERASE    :erase chip fail;
    ERR_STATUS    :status error;
    ERR_ECC        :ecc error.
*/


#define    TY_FTS_PACKET_LENGTH        32
#if 0
static unsigned char CTPM_FW_FILE[]=
{
//#include "ft_app.i"
};
#endif
#define BL_VERSION_LZ4	0
#define BL_VERSION_Z7	1
#define BL_VERSION_GZF	2 

static E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    FTS_BYTE reg_val[2] = {0};
    FTS_DWRD i = 0;
    u8 is_5336_new_bootloader = 0;
    u8 is_5336_fwsize_30 = 0;
   u8 readid=0;
    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp = 0;
    FTS_DWRD  lenght;
    FTS_BYTE  packet_buf[TY_FTS_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE bt_ecc;
    int      i_is_new_protocol = 0;
    int      i_ret;
////////////////////////
unsigned char au_delay_timing[44] = {30, 33, 36, 39, 42, 45, 27, 24,21,18,15,20,21,22,23,25,26,28,29,
								31,32,34,35,37,38,40,10,11,12,13,14,16,17,18,19,1,2,3,4,5,6,7,8,9};
    unsigned char jj = 0;
///////////////////////

	if(pbt_buf[dw_lenth-12] == 30)
	{
		is_5336_fwsize_30 = 1;
	}
	else 
	{
		is_5336_fwsize_30 = 0;
	}

	fts_debug("[TSP] :enter %s\n",__func__);
/////////////////
UPG_START:
////////////
	
    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    //fts_register_write(0xfc,0xaa);
/*********Step 1:HardReset  CTPM wang_gj  add begin*****/
	gpio_direction_output(26, 0);
	delay_qt_ms(50);
	gpio_direction_output(26, 1);
/*********Step 1:HardReset  CTPM wang_gj  add end*****/
	//delay_qt_ms(50);
    fts_debug("fts_register_write:%d\n",__LINE__);
     /*write 0x55 to register 0xfc*/
    //fts_register_write(0xfc,0x55);
    fts_debug("[TSP] Step 1: Reset CTPM test\n");
    fts_debug("fts_register_write:%d\n",__LINE__);
    //delay_qt_ms(10);
/*#if defined (TYQ_TBW5925_SUPPORT)   
  //  delay_qt_ms(30); //25~40    30
#else
   delay_qt_ms(20);
#endif
*/
////////////////////////////////////////////////
	delay_qt_ms(au_delay_timing[jj]);
	printk("********jj=%d***********\n",jj);
/////////////////////////////////////////
    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = i2c_write_interface(I2C_CTPM_ADDRESS, auc_i2c_write_buf, 2);
        fts_debug("i=%d ",i);
        delay_qt_ms(5); //25
    }while(i_ret <= 0 && i < 10 );

	fts_debug("\ni=%d\n",i);

    if (i > 1)
    {
        i_is_new_protocol = 1;
    }
	msleep(1);
    /*********Step 3:check READ-ID***********************/        
    cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
  //  #if defined (TYQ_TBW5925_SUPPORT)
    if (((reg_val[0] == 0x79) && (reg_val[1] == 0x7 ||reg_val[1] == 0x3 || reg_val[1] == 0x11)) || ((reg_val[0] == 0x79) && (reg_val[1] == 0x8)))
   // #else
  //  if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
   // #endif
    {
    	if((reg_val[0] == 0x79) && (reg_val[1] == 0x11))
			readid = 1;
    	printk("[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
        fts_debug("[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
    	/*printk("error:ERR_READID,0x%x,0x%x,%d\n",reg_val[0],reg_val[1],__LINE__);
        return ERR_READID;
	*/
if (jj < 43)
        {
             jj ++;
		printk("***************jj=%d**************\n",jj);
             msleep(200);
             goto UPG_START; 
        }
        else
        {
		printk("error:ERR_READID,0x%x,0x%x,%d\n",reg_val[0],reg_val[1],__LINE__);
            return ERR_READID;
        }
/////////////////////////////////////////
        //i_is_new_protocol = 1;
    }

	auc_i2c_write_buf[0] = 0xcd;
	i2c_write_interface(I2C_CTPM_ADDRESS, auc_i2c_write_buf, 1);
	  byte_read(reg_val,1);
	//ft5x0x_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);

	/*********0705 mshl ********************/
	/*if (reg_val[0] > 4)
		is_5336_new_bootloader = 1;*/

	if (reg_val[0] <= 4)
	{
		is_5336_new_bootloader = BL_VERSION_LZ4 ;
	}
	else if(reg_val[0] == 7)
	{
		is_5336_new_bootloader = BL_VERSION_Z7 ;
	}
	else if(reg_val[0] >= 0x0f)
	{
		is_5336_new_bootloader = BL_VERSION_GZF ;
	}

	printk("****reg_val[0] =%d,is_5336_new_bootloader =%d*****is_5336_fwsize_30=%d**********\n",reg_val[0],is_5336_new_bootloader,is_5336_fwsize_30);
     /*********Step 4:erase app*******************************/
    if (is_5336_fwsize_30 && (readid ==1))
    {
      // auc_i2c_write_buf[0] = 0x61;
		//ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1); /*erase app area*/	
		 cmd_write(0x61,0x00,0x00,0x00,1);
   		 msleep(1500); 

		// auc_i2c_write_buf[0] = 0x63;
		//ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1); /*erase app area*/
		cmd_write(0x63,0x00,0x00,0x00,1);
   		 msleep(50);
    }
    else
    {
        cmd_write(0x61,0x00,0x00,0x00,1);
	 delay_qt_ms(2000);
    }
    //delay_qt_ms(1500);
   
    fts_debug("[TSP] Step 4: erase. \n");
    fts_debug("[TSP] TY_FTS_PACKET_LENGTH = %d\n", TY_FTS_PACKET_LENGTH);

    /*********Step 5:write firmware(FW) to ctpm flash*********/
    bt_ecc = 0;
    fts_debug("[TSP] Step 5: start upgrade. \n");
  if(readid == 0 || is_5336_new_bootloader == BL_VERSION_LZ4 || is_5336_new_bootloader == BL_VERSION_Z7 )
  {
   	 dw_lenth = dw_lenth - 8;
  }
  else if(is_5336_new_bootloader == BL_VERSION_GZF) dw_lenth = dw_lenth - 14;
  
    packet_number = (dw_lenth) / TY_FTS_PACKET_LENGTH;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    for (j=0;j<packet_number;j++)
    {
        temp = j * TY_FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        lenght = TY_FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(lenght>>8);
        packet_buf[5] = (FTS_BYTE)lenght;

        for (i=0;i<TY_FTS_PACKET_LENGTH;i++)
        {
            packet_buf[6+i] = pbt_buf[j*TY_FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }
        
        byte_write(&packet_buf[0],TY_FTS_PACKET_LENGTH + 6);
        delay_qt_ms(TY_FTS_PACKET_LENGTH/6 + 8);
        if ((j * TY_FTS_PACKET_LENGTH % 1024) == 0)
        {
              fts_debug("[TSP] upgrade the 0x%x th byte.\n", ((unsigned int)j) * TY_FTS_PACKET_LENGTH);
        }
    }

    if ((dw_lenth) % TY_FTS_PACKET_LENGTH > 0)
    {
        temp = packet_number * TY_FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;

        temp = (dw_lenth) % TY_FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;

        for (i=0;i<temp;i++)
        {
            packet_buf[6+i] = pbt_buf[ packet_number*TY_FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }

        byte_write(&packet_buf[0],temp+6);    
        delay_qt_ms(20);
    }

    //send the last six byte
    if(readid == 0 || is_5336_new_bootloader == BL_VERSION_LZ4 || is_5336_new_bootloader == BL_VERSION_Z7 )
	{
    for (i = 0; i<6; i++)
    {
    	if(readid &&( is_5336_new_bootloader  == BL_VERSION_Z7))
		 temp = 0x7bfa + i;
	else if(readid ==0 || is_5336_new_bootloader == BL_VERSION_LZ4)
        	temp = 0x6ffa + i;
	
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        temp =1;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;
        packet_buf[6] = pbt_buf[ dw_lenth + i]; 
        bt_ecc ^= packet_buf[6];

        byte_write(&packet_buf[0],7);  
        delay_qt_ms(20);
    }
    }
	else if(is_5336_new_bootloader == BL_VERSION_GZF)
	{
		for (i = 0; i<12; i++)
		{
			if (is_5336_fwsize_30 && readid) 
			{
				temp = 0x7ff4 + i;
			}
			else if (readid) 
			{
				temp = 0x7bf4 + i;
			}
			packet_buf[2] = (u8)(temp>>8);
			packet_buf[3] = (u8)temp;
			temp =1;
			packet_buf[4] = (u8)(temp>>8);
			packet_buf[5] = (u8)temp;
			packet_buf[6] = pbt_buf[ dw_lenth + i]; 
			bt_ecc ^= packet_buf[6];
  			 byte_write(&packet_buf[0],7);  
			//ft5x0x_i2c_Write(client, packet_buf, 7);
			msleep(10);

		}
	}
    /*********Step 6: read out checksum***********************/
    /*send the opration head*/
    cmd_write(0xcc,0x00,0x00,0x00,1);
    byte_read(reg_val,1);
    fts_debug("[TSP] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
    if(reg_val[0] != bt_ecc)
    {
        return ERR_ECC;
    }

    /*********Step 7: reset the new FW***********************/
    cmd_write(0x07,0x00,0x00,0x00,1);

	//added by wuz
	delay_qt_ms(300);
	fts_Upgraded();
	//
    return ERR_OK;
}

/**********************************************************
*CTPM_FW:firmware file,the size less often 26k byte
***********************************************************/
static int fts_ctpm_fw_upgrade_with_i_file(unsigned char* CTPM_FW,int len)
//int fts_ctpm_fw_upgrade_with_i_file()
{
   FTS_BYTE*     pbt_buf = FTS_NULL;
   int i_ret;
   g_u8UpgradeFWResult = 0;
   
    //=========FW upgrade========================*/
   pbt_buf = CTPM_FW;
   /*call the upgrade function*/
   i_ret =  fts_ctpm_fw_upgrade(pbt_buf,len);
   g_u8UpgradeFWResult = i_ret;
   if (i_ret != 0)
   {
       //error handling ...
       //TBD
       printk("upgrade the tp firmware failed.\n");
   }

   return i_ret;
}
#if 0
static int fts_ctpm_fw_upgrade_inline(void)
{
   FTS_BYTE*     pbt_buf = FTS_NULL;
   int i_ret;
   g_u8UpgradeFWResult = 0;
   
    //=========FW upgrade========================*/
   pbt_buf = CTPM_FW_FILE;
   printk("%s:enter,0x%x \n",__func__,*pbt_buf); //wang_gj add *
   /*call the upgrade function*/
   i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW_FILE));
   g_u8UpgradeFWResult = i_ret;
   if (i_ret != 0)
   {
       //error handling ...
       //TBD
       printk("upgrade the tp firmware failed.\n");
   }

   return i_ret;
}

static unsigned char fts_ctpm_get_upg_ver(void)
{
    unsigned int ui_sz;
    ui_sz = sizeof(CTPM_FW_FILE);
    if (ui_sz > 2)
    {
        return CTPM_FW_FILE[ui_sz - 2];
    }
    else
    {
        //TBD, error handling?
        return 0xff; //default value
    }
}
#endif

/*
* registry the touch as the char device
*/
#define	NV_TOUCH_FT

//Tianyu ADD
#define	TYN_TOUCH_FMUPGRADE 0x6501
#define	TYN_TOUCH_FWVER     (TYN_TOUCH_FMUPGRADE +1 )
#define   TYN_TOUCH_VENDOR   (TYN_TOUCH_FMUPGRADE +2 )
//tp vendor
#define TYN_TOUCH_FOCALTECH    1
#define TYN_TOUCH_ILITEK	   2

static struct touch_ctrl_dev
{
  dev_t devno;
  struct cdev cdev;
  struct class *class;
}ty_touch_ctrl;

static int touch_ctrl_open(struct inode *inode, struct file *filp) 
{ 
  printk("%s\n", __func__); 
  return 0;   
} 
 
static ssize_t touch_ctrl_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) 
{		  
  printk("%s, calibration %s %d\n", __func__, buf, count);	
  
  //NvOdmTouchSetAutoCalibration(touch_context->hTouchDevice, FTS_TRUE);
  return 0;
} 
static ssize_t touch_ctrl_read (struct file *file, char *buf, size_t count, loff_t *offset)
{
  printk("touch_ctrl_read calibration\n"); 
  return 0; 
}

//int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
//struct serial_struct __user *

static int fts_GetFWVer(unsigned long * pu32)
{
	
	int ret;
	if((NULL == g_stpTouchData) || (NULL == pu32))
	{
		printk("%s:g_stpTouchData is null.\n",__func__);
		return 0;
	}
	
	msleep(50);
	ret = fts_i2c_read(g_stpTouchData, 0xA6, (u8*)pu32, 1);//wang_gj add u8*
	if(ret< 0)
	{
		printk("%s:i2c error.\n",__func__);
		return ret;
	}
	msleep(50);

	printk("%s:ver=0x%x.\n",__func__,(unsigned int)(*pu32));//wang_gj modify
	
	return 1;

}

unsigned char FTS_CTPM_GetVendorID_info(unsigned char* ptr_vendor_id)
{
	 FTS_BYTE reg_val[2] = {0};
		FTS_DWRD i = 0;
		FTS_BYTE  auc_i2c_write_buf[10];
		int 	 i_is_new_protocol = 0;
		int 	 i_ret;
	unsigned char au_delay_timing[11] = {30, 33, 36, 39, 42, 45, 27, 24,21,18,15};
		unsigned char jj = 0;
		fts_debug("[TSP] :enter %s\n",__func__);
	UPG_START:
		
	/*********Step 1:HardReset	CTPM wang_gj  add begin*****/
		gpio_direction_output(26, 0);
		delay_qt_ms(50);
		gpio_direction_output(26, 1);
	/*********Step 1:HardReset	CTPM wang_gj  add end*****/
		delay_qt_ms(au_delay_timing[jj]);
	
	/////////////////////////////////////////
		/*********Step 2:Enter upgrade mode *****/
		auc_i2c_write_buf[0] = 0x55;
		auc_i2c_write_buf[1] = 0xaa;
		do
		{
			i ++;
			i_ret = i2c_write_interface(I2C_CTPM_ADDRESS, auc_i2c_write_buf, 2);
			fts_debug("i=%d ",i);
			delay_qt_ms(5); //25
		}while(i_ret <= 0 && i < 10 );
	
		fts_debug("\ni=%d\n",i);
	
		if (i > 1)
		{
			i_is_new_protocol = 1;
		}
	
		/*********Step 3:check READ-ID***********************/		  
		cmd_write(0x90,0x00,0x00,0x00,4);
		byte_read(reg_val,2);
	  //  #if defined (TYQ_TBW5925_SUPPORT)
		if (((reg_val[0] == 0x79) && (reg_val[1] == 0x7 ||reg_val[1] == 0x3))||((reg_val[0] == 0x79) && (reg_val[1] == 0x8)))
	   // #else
	  //  if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
	   // #endif
		{
			fts_debug("[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
		}
		else
		{
			/*printk("error:ERR_READID,0x%x,0x%x,%d\n",reg_val[0],reg_val[1],__LINE__);
			return ERR_READID;
		*/
	//////////////////////////////////////////
	if (jj < 10)
			{
				 jj ++;
			printk("***************jj=%d**************\n",jj);
				 msleep(200);
				 goto UPG_START; 
			}
			else
			{
			printk("error:ERR_READID,0x%x,0x%x,%d\n",reg_val[0],reg_val[1],__LINE__);
				return ERR_READID;
			}
	/////////////////////////////////////////
			//i_is_new_protocol = 1;
		}
		fts_i2c_read(g_stpTouchData, 0xcd, reg_val, 1);
		printk("bootloader version =0x%x\n",reg_val[0]);
		cmd_write(0x03,0x00,0x78,0x04,4);
		byte_read(ptr_vendor_id,1);
		auc_i2c_write_buf[0]=0x07;
		byte_write(auc_i2c_write_buf,1);
		msleep(300);
		return 0;

}
static long touch_ctrl_ioctl(struct file * file, unsigned int cmd, unsigned long arg)
{
	struct ty_touch_fmupgrade_S  bufarg;
	int iret;
	unsigned char *pBuf;
	u8 bRet = 0;
	unsigned long  ulVer;
	u8 vendor = 0;
	//unsigned long i;
	if (down_interruptible(&upgrade_lock)) {
		printk("obtain upgrade_lock faild!\n ");
		return  -EFAULT;
	}
	printk("**********************entery touch_ctrl_ioctl****************************\n");
	if (0 == arg)
	{
		printk("%s:arg null pointer.\n",__func__);
		up(&upgrade_lock);
		return -EFAULT;
	}
	
	if (copy_from_user(&bufarg, (void *) arg, sizeof(bufarg))) 
	{
		printk("%s:null pointer.\n",__func__);
		up(&upgrade_lock);
		return -EFAULT;
	}

	printk("%s:cmd=0x%x.\n",__func__,cmd);
	
	switch (cmd) 
	{
		case TYN_TOUCH_FWVER:
			printk("%s:getver.\n",__func__);
			if (NULL == bufarg.bufAddr)
			{
				printk("%s:bufAddr null pointer.\n",__func__);
				up(&upgrade_lock);
				return -EFAULT;
			}
			printk("%s:getver 1.\n",__func__);
			
			if ((0>= bufarg.bufLength )||( sizeof(unsigned long) < bufarg.bufLength ))
			{
				printk("%s:bufLength null pointer.\n",__func__);
				up(&upgrade_lock);
				return -EFAULT;
			}			
			//printk("%s:lenth=%x,addr=0x%x\n",__func__,(unsigned int)bufarg.bufLength,(bufarg.bufAddr));
			printk("%s:getver 2\n",__func__);
			
			pBuf = kzalloc(bufarg.bufLength, GFP_KERNEL);
			if (0 == pBuf)
			{
				printk("%s:alloc buffer failed.\n",__func__);
				up(&upgrade_lock);
				return 0;
			}

			if (copy_from_user(pBuf, (void *)bufarg.bufAddr, bufarg.bufLength)) 
			{
				printk("%s:get buffer error.\n",__func__);
				up(&upgrade_lock);
				return -EFAULT;
			}
			printk("%s:getver 3.\n",__func__);

			
			ulVer = 0;
			
			//get firmware version
	#if defined(NV_TOUCH_FT)
			bRet = fts_GetFWVer(&ulVer);
			if(0 > bRet) 
			{
				*(unsigned long*)bufarg.bufAddr = 0;
				printk("%s:FT_GetFWVer failed. \n",__func__);
				kfree(pBuf);
				up(&upgrade_lock);
				return -EFAULT;
			}
			
	#endif

			*(( unsigned long *)bufarg.bufAddr) = ulVer;
			
			//printk("%s:ver=0x%x \n",__func__,*(unsigned long*)bufarg.bufAddr);
			if( pBuf ) kfree(pBuf);
			
			break;
			
		case TYN_TOUCH_FMUPGRADE:
			fts_GetFWVer(&ulVer);
			printk("old tp_version = 0x%x\n",(u8)ulVer);
			if (0 == bufarg.bufAddr)
			{
				printk("%s:null pointer.\n",__func__);
				up(&upgrade_lock);
				return 0;
			}
			
			pBuf = kzalloc(bufarg.bufLength, GFP_KERNEL);
			if (0 == pBuf)
			{
				printk("%s:alloc buffer failed.\n",__func__);
				up(&upgrade_lock);
				return 0;
			}
			
			if (copy_from_user(pBuf, (void *) bufarg.bufAddr, bufarg.bufLength)) 
			{
				printk("%s:get buffer error.\n",__func__);
				up(&upgrade_lock);
				return -EAGAIN;
			}

			//get crc and check crc 	  
			//
			
			//printk("%s:type=0x%x,len=0x%x \n",__func__,bufarg.touchType,bufarg.bufLength);
			//printk("ver=0x%x,0x%x,0x%x \n",pBuf[bufarg.bufLength-3],pBuf[bufarg.bufLength-2],pBuf[bufarg.bufLength-1]);
			
     #if defined(NV_TOUCH_FT)
			if( TYN_TOUCH_FOCALTECH == bufarg.touchType)
			{
				iret = fts_ctpm_fw_upgrade_with_i_file(pBuf,bufarg.bufLength);
				if(iret)
				{
					printk("update focaltech firmware failed.\n");
				}
				kfree(pBuf);
				fts_ctpm_auto_clb(g_stpTouchData);
				fts_GetFWVer(&ulVer);
				printk("current tp_version = 0x%x\n",(u8)ulVer);
				up(&upgrade_lock);
				return iret;
			}
			
    #endif
			if( pBuf ) kfree(pBuf);
			break;
		case TYN_TOUCH_VENDOR:
			msleep(200);
			ty_ft5x0x_read_reg(g_stpTouchData,0xa8,&vendor);
			if(vendor == 0xa8 || vendor ==0)
			{
				FTS_CTPM_GetVendorID_info(&vendor);
			}
			printk("pengwei test touch vendor id is %d",vendor);
			if(vendor==TRUlY_VENDOR_ID_NO)
			{
				if(copy_to_user(bufarg.bufAddr,TRUlY_VENDOR_ID_STR,strlen(TRUlY_VENDOR_ID_STR)))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			else if(vendor==TRUlY_VENDOR_ID_NO_2)
			{
				if(copy_to_user(bufarg.bufAddr,TRUlY_VENDOR_ID_STR_2,strlen(TRUlY_VENDOR_ID_STR_2)))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			else if(vendor==BYD_VENDOR_ID_NO)
			{
				if(copy_to_user(bufarg.bufAddr,BYD_VENDOR_ID_STR,strlen(BYD_VENDOR_ID_STR)))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			else if(vendor==O_FILM_VENDOR_ID_NO)
			{
				if(copy_to_user(bufarg.bufAddr,O_FILM_VENDOR_ID_STR,strlen(O_FILM_VENDOR_ID_STR)))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			else if(vendor==O_FILM_VENDOR_ID_NO_2)
			{
				if(copy_to_user(bufarg.bufAddr,O_FILM_VENDOR_ID_STR_2,strlen(O_FILM_VENDOR_ID_STR_2)))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			else if(vendor==BM_VENDOR_ID_NO)
			{
				if(copy_to_user(bufarg.bufAddr,BM_VENDOR_ID_STR,strlen(BM_VENDOR_ID_STR)))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			else if(vendor==LCE_VENDOR_ID_NO)
			{
				if(copy_to_user(bufarg.bufAddr,LCE_VENDOR_ID_STR,strlen(LCE_VENDOR_ID_STR)))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			else
			{
				if(copy_to_user(bufarg.bufAddr,"unknown_tp",strlen("unknown_tp")))
				{
					printk("%s:get tp_vendor info err!\n",__func__);
				}
			}
			break;
		default:
			printk("%s:Invalid command.\n",__func__);
	}
	up(&upgrade_lock);
	return 0;
}

static int touch_ctrl_close(struct inode *inode, struct file *filp) 
{ 
  printk("%s\n", __func__); 
  return 0; 
} 

struct file_operations ty_touch_ctrl_fops = 
{ 
	.write = touch_ctrl_write,
	.read = touch_ctrl_read, 
	.open = touch_ctrl_open, 
	.release = touch_ctrl_close, 
	.unlocked_ioctl	 = touch_ctrl_ioctl,//wang_gj
}; 

static int touch_ctrl_init(void *touch) 
{ 
  int ret = 0; 
  struct device *fts_chr_device;

  ret = alloc_chrdev_region(&ty_touch_ctrl.devno, 0, 1, "touch_ctrl"); 
  if(ret)
  { 
	printk("%s, can't alloc chrdev\n", __func__); 
  } 
  printk("%s, register chrdev(%d, %d)\n", __func__, MAJOR(ty_touch_ctrl.devno), MINOR(ty_touch_ctrl.devno)); 
		 
  cdev_init(&ty_touch_ctrl.cdev, &ty_touch_ctrl_fops);
 
  ty_touch_ctrl.cdev.owner = THIS_MODULE; 
  ty_touch_ctrl.cdev.ops = &ty_touch_ctrl_fops;
  
  ret = cdev_add(&ty_touch_ctrl.cdev, ty_touch_ctrl.devno, 1); 
  if(ret < 0)
  { 
	printk("%s, add char devive error, ret %d\n", __func__, ret); 
  } 

  ty_touch_ctrl.class = class_create(THIS_MODULE, "fts_touch_ctrl"); 
  if(IS_ERR(ty_touch_ctrl.class)){ 
	printk("%s, creating class error\n", __func__); 
  } 

  fts_chr_device=device_create(ty_touch_ctrl.class, NULL, ty_touch_ctrl.devno, NULL, "fts_fw_entry");
  if(NULL == fts_chr_device){
	printk("%s, create device, error\n", __func__);
	class_destroy(ty_touch_ctrl.class);
  }

  return (int)touch; 
} 

#endif
//*****************************************************************************//

///////////////////////////////////////////////////////////////////////


module_init(ft5x06_ts_init);

module_exit(ft5x06_ts_exit);

MODULE_DESCRIPTION("FocalTech ft5x06 TouchScreen driver");
MODULE_LICENSE("GPL v2");
