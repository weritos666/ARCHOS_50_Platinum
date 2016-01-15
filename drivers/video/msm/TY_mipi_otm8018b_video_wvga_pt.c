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

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "TY_mipi_otm8018b.h"
#include <linux/gpio.h>
#include <mach/gpio.h>

#define DRV_READ_LCDID	 1
static struct msm_panel_info pinfo;

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db = {
	/* DSI Bit Clock at 500 MHz, 2 lane, RGB888 */
	/* regulator */
	{0x13, 0x01, 0x01, 0x00},
	/* timing   */
	{0xb9, 0x8e, 0x1f, 0x00, 0x98, 0x9c, 0x22, 0x90,
	0x18, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xbb, 0x02, 0x06, 0x00},
	/* pll control */
	{0x00, 0xec, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62,
	0x01, 0x0f, 0x07,
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0},
};

#ifdef  DRV_READ_LCDID
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

static int mipi_video_otm8018b_wvga_pt_init(void)
{
	int ret;

	if (msm_fb_detect_client("mipi_video_otm8018b_wvga"))
		return 0;
	#ifdef DRV_READ_LCDID
		msleep(2);
		lcd_id_flag = tyq_lcd_read_id(LCD_ID);
		tp_id_flag = tyq_lcd_read_id(TP_ID);
		printk("+++++++lcd_id_flag=%d,tp_id_flag=%d+++++++++++++++++++++++++++++++++\n",lcd_id_flag,tp_id_flag);
	#endif
	pinfo.xres = 480;
	pinfo.yres = 800;
	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;
	#if 0
	pinfo.lcdc.h_back_porch = 55;
	pinfo.lcdc.h_front_porch = 105;
	pinfo.lcdc.h_pulse_width = 8;
	pinfo.lcdc.v_back_porch = 26; //15
	pinfo.lcdc.v_front_porch = 26; //20
	pinfo.lcdc.v_pulse_width = 10;  //1
	#endif

	
#if 1
#ifdef DRV_READ_LCDID
	if((lcd_id_flag == 0) && (tp_id_flag ==1)){
	pinfo.lcdc.h_back_porch = 165;
	pinfo.lcdc.h_front_porch = 165;
	}
	else
#endif
	{
	pinfo.lcdc.h_back_porch = 125;
	pinfo.lcdc.h_front_porch = 125;
	}
	pinfo.lcdc.h_pulse_width = 8;
	pinfo.lcdc.v_back_porch = 50;
	pinfo.lcdc.v_front_porch = 50;
	pinfo.lcdc.v_pulse_width = 1;
#endif
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	/* number of dot_clk cycles HSYNC active edge is
	delayed from VSYNC active edge */
	pinfo.lcdc.hsync_skew = 0;
	pinfo.clk_rate = 499000000;
	
	/*TYRD wuxh add for gpio 27 pwm backligth control  on 20121206*/
#ifdef TYQ_QCT_SUPPORT
	pinfo.bl_max = 128;
#else
	pinfo.bl_max = 255;
#endif
	pinfo.bl_min = 1;
	pinfo.fb_num = 2;

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	/* send HSA and HE following VS/VE packet */
	pinfo.mipi.pulse_mode_hsa_he = TRUE;
	pinfo.mipi.hfp_power_stop = TRUE; /* LP-11 during the HFP period */
	pinfo.mipi.hbp_power_stop = TRUE; /* LP-11 during the HBP period */
	pinfo.mipi.hsa_power_stop = TRUE; /* LP-11 during the HSA period */
	/* LP-11 or let Command Mode Engine send packets in
	HS or LP mode for the BLLP of the last line of a frame */
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	/* LP-11 or let Command Mode Engine send packets in
	HS or LP mode for packets sent during BLLP period */
	pinfo.mipi.bllp_power_stop = TRUE;

	pinfo.mipi.traffic_mode = DSI_BURST_MODE;
	pinfo.mipi.dst_format =  DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB; /* RGB */
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;

	pinfo.mipi.t_clk_post = 0x3;
	pinfo.mipi.t_clk_pre = 0x24;

	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_NONE;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.frame_rate = 63; /* FIXME */

	pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db;
	//pinfo.mipi.dlane_swap = 0x01;
	/* append EOT at the end of data burst */
	pinfo.mipi.tx_eot_append = 0x01;

	ret = mipi_otm8018b_device_register(&pinfo, MIPI_DSI_PRIM,MIPI_DSI_PANEL_WVGA_PT);

	if (ret)
		pr_err("%s: failed to register device!\n", __func__);

	return ret;
}

module_init(mipi_video_otm8018b_wvga_pt_init);
