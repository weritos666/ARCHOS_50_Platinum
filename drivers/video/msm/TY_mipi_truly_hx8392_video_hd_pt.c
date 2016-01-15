/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
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
#include "TY_mipi_truly_hx8392.h"
#include <linux/gpio.h>
#include <linux/init.h>
#include <mach/gpio.h>
#if defined(TYQ_HX8392_MIPI_HD_SUPPORT)
#define DRV_READ_LCDID	 1

static struct msm_panel_info pinfo;

static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db_hx8392 = {
	/* DSI Bit Clock at 500 MHz, 2 lane, RGB888 */
	/* regulator */
	{0x03, 0x01, 0x01, 0x00},
	/* timing   */
	{0xb9, 0x8e, 0x1f, 0x00, 0x98, 0x9c, 0x22, 0x90,
	0x18, 0x03, 0x04},
	/* phy ctrl */
	{0x7f, 0x00, 0x00, 0x00},
	/* strength */
	{0xbb, 0x02, 0x06, 0x00},
	/* pll control */
	{0x00, 0xdb, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62,
	0x01, 0x0f, 0x03,
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0},
};

#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT) && defined (TYQ_SSD2080M_MIPI_HD_SUPPORT)
static struct mipi_dsi_phy_ctrl dsi_video_mode_phy_db_ssd2080m = {
	/* DSI Bit Clock at 500 MHz, 2 lane, RGB888 */
	/* regulator */
	{0x03, 0x01, 0x01, 0x00},
	/* timing   */
    {0x8b, 0x33, 0x14, 0x00, 0x45, 0x4d, 0x19, 0x36,
    0x18, 0x03, 0x04},
	/* phy ctrl */
    {0x7f, 0x00, 0x00, 0xef},
	/* strength */
	{0xbb, 0x02, 0x06, 0x00},
	/* pll control */
    {0x00, 0x9e, 0x31, 0xd2, 0x00, 0x40, 0x37, 0x62,
	0x01, 0x0f, 0x03,
	0x05, 0x14, 0x03, 0x0, 0x0, 0x0, 0x20, 0x0, 0x02, 0x0},
};
#endif

#ifdef  DRV_READ_LCDID
static int lcd_id_flag;
//static int tp_id_flag;
//extern int msm_fb_lcdc_config(int on);

#define LCD_ID 77


//#define TP_ID    113
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
			//printk("----->>>%d \n",rc);
		}
		gpio_free(gpio);
	}

	return rc;
}
#endif
static int __init mipi_video_hx8392_wvga_pt_init(void)
{
	int ret;
#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT)
	char *pstrid_hx, *pstrid_ssd;
	char def_strid[] = "hx8392";
#endif

#ifdef DRV_READ_LCDID
		msleep(2);
		lcd_id_flag = tyq_lcd_read_id(LCD_ID);
		//tp_id_flag = tyq_lcd_read_id(TP_ID);
		printk("+++++++lcd_id_flag=%d +\n",lcd_id_flag);
#endif
	printk("%s : mipi_hx8392 enter\n",__func__);

#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT)
	pstrid_hx = strstr(get_aboot_cmd, "hx8392");
	pstrid_ssd = strstr(get_aboot_cmd, "ssd2080m");

	if ( (pstrid_hx == NULL) && (lcd_id_flag == 1) 
		&& (pstrid_ssd == NULL) ) {
		pstrid_hx = def_strid;
	}
#else
	if (msm_fb_detect_client("mipi_video_hx8392_hd"))
		return 0;
#endif
	
	pinfo.xres = 720;
	pinfo.yres = 1280;
	pinfo.type = MIPI_VIDEO_PANEL;
	pinfo.pdest = DISPLAY_1;
	pinfo.wait_cycle = 0;
	pinfo.bpp = 24;

#ifdef DRV_READ_LCDID
	if (lcd_id_flag) {
#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT)
		if (pstrid_hx != NULL) {
			pinfo.lcdc.h_back_porch = 190;
			pinfo.lcdc.h_front_porch = 175;
			pinfo.lcdc.h_pulse_width = 15;
			pinfo.lcdc.v_back_porch = 5;
			pinfo.lcdc.v_front_porch = 20;
			pinfo.lcdc.v_pulse_width = 1;
		} else if (pstrid_ssd != NULL) {
			pinfo.lcdc.h_back_porch = 36;
			pinfo.lcdc.h_front_porch = 80; 
			pinfo.lcdc.h_pulse_width = 14;
			pinfo.lcdc.v_back_porch = 15;
			pinfo.lcdc.v_front_porch = 12;
			pinfo.lcdc.v_pulse_width = 10;
		}
#else
		pinfo.lcdc.h_back_porch = 190;
		pinfo.lcdc.h_front_porch = 175;
		pinfo.lcdc.h_pulse_width = 15;
		pinfo.lcdc.v_back_porch = 5;
		pinfo.lcdc.v_front_porch = 20;
		pinfo.lcdc.v_pulse_width = 1;
#endif
	} else {
		pinfo.lcdc.h_back_porch = 100;
		pinfo.lcdc.h_front_porch = 100;
		pinfo.lcdc.h_pulse_width = 2;
		pinfo.lcdc.v_back_porch = 15;
		pinfo.lcdc.v_front_porch = 8;
		pinfo.lcdc.v_pulse_width = 5;
	}
#else
		pinfo.lcdc.h_back_porch = 190;
		pinfo.lcdc.h_front_porch = 175;
		pinfo.lcdc.h_pulse_width = 15;
		pinfo.lcdc.v_back_porch = 5;
		pinfo.lcdc.v_front_porch = 20;
		pinfo.lcdc.v_pulse_width = 1;

#endif
	pinfo.lcdc.border_clr = 0;	/* blk */
	pinfo.lcdc.underflow_clr = 0xff;	/* blue */
	/* number of dot_clk cycles HSYNC active edge
	   is delayed from VSYNC active edge */
	pinfo.lcdc.hsync_skew = 0;
#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT)
	if (pstrid_ssd != NULL) 
		pinfo.clk_rate = 420000000;
	else
	pinfo.clk_rate = 482000000;
#else
	pinfo.clk_rate = 482000000;
#endif
	
	pinfo.bl_max = 255;
	pinfo.bl_min = 1;

	pinfo.fb_num = 2;

	pinfo.mipi.mode = DSI_VIDEO_MODE;
	pinfo.mipi.pulse_mode_hsa_he = TRUE; /* send HSA and HE following
						VS/VE packet */
	pinfo.mipi.hfp_power_stop = TRUE; /* LP-11 during the HFP period */
	pinfo.mipi.hbp_power_stop =FALSE; /* LP-11 during the HBP period *///ssd2080m set
	pinfo.mipi.hsa_power_stop = TRUE; /* LP-11 during the HSA period */
	/* LP-11 or let Command Mode Engine send packets in
	HS or LP mode for the BLLP of the last line of a frame */
	pinfo.mipi.eof_bllp_power_stop = TRUE;
	/* LP-11 or let Command Mode Engine send packets in
	HS or LP mode for packets sent during BLLP period */
	pinfo.mipi.bllp_power_stop = FALSE;

	pinfo.mipi.traffic_mode = DSI_BURST_MODE;
	pinfo.mipi.dst_format =  DSI_VIDEO_DST_FORMAT_RGB888;
	pinfo.mipi.vc = 0;
	pinfo.mipi.rgb_swap = DSI_RGB_SWAP_RGB; /* RGB */
	pinfo.mipi.data_lane0 = TRUE;
	pinfo.mipi.data_lane1 = TRUE;
	pinfo.mipi.data_lane2 = TRUE;
	pinfo.mipi.data_lane3 = TRUE;
#if defined (TYQ_MIPI_LCD_DRV_COMPAT_SUPPORT) && defined (TYQ_SSD2080M_MIPI_HD_SUPPORT)
	if (pstrid_ssd != NULL) {
		pinfo.mipi.t_clk_post = 0x20;
		pinfo.mipi.t_clk_pre = 0x2f;
		pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db_ssd2080m;
		//pinfo.mipi.force_clk_lane_hs = 1;
	} else {
		pinfo.mipi.hbp_power_stop = TRUE;
		pinfo.mipi.bllp_power_stop = TRUE;
		pinfo.mipi.t_clk_post = 0x3;
		pinfo.mipi.t_clk_pre = 0x24;
		pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db_hx8392;
	}
#else
	pinfo.mipi.hbp_power_stop = TRUE;
	pinfo.mipi.bllp_power_stop = TRUE;
	pinfo.mipi.t_clk_post = 0x3;
	pinfo.mipi.t_clk_pre = 0x24;
	pinfo.mipi.dsi_phy_db = &dsi_video_mode_phy_db_hx8392;
#endif
	pinfo.mipi.stream = 0; /* dma_p */
	pinfo.mipi.mdp_trigger = DSI_CMD_TRIGGER_NONE;
	pinfo.mipi.dma_trigger = DSI_CMD_TRIGGER_SW;
	pinfo.mipi.frame_rate = 60; /* FIXME */

	//pinfo.mipi.dlane_swap = 0x01;
	pinfo.mipi.tx_eot_append = 0x01; /* append EOT at the end
					    of data burst */
							
	ret = mipi_hx8392_device_register(&pinfo, MIPI_DSI_PRIM,
						MIPI_DSI_PANEL_720P_PT);

	if (ret)
		pr_err("%s: failed to register device!\n", __func__);
    printk("%s ok !__+++++++++++++++\n",__func__);

	return ret;
}

module_init(mipi_video_hx8392_wvga_pt_init);
#endif
