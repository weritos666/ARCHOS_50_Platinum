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
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>
#include <mach/rpc_pmapp.h>
#include "devices.h"
#include "board-msm7627a.h"
#include <linux/kernel.h>
#include <linux/module.h>

/*TYRD wuxh add for gpio 27 pwm backligth control  on 20121206*/
#ifdef TYQ_QCT_SUPPORT
#include <linux/clk.h>
#endif

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#define MSM_FB_SIZE		0x4BF000
#define MSM7x25A_MSM_FB_SIZE    0x1C2000
#define MSM8x25_MSM_FB_SIZE	0x5FA000
#define MSM8x25Q_MSM_FB_SIZE	0xAC8000
#else
#define MSM_FB_SIZE		0x32A000
#define MSM7x25A_MSM_FB_SIZE	0x12C000
#define MSM8x25_MSM_FB_SIZE	0x3FC000
#define MSM8x25Q_MSM_FB_SIZE	0x730000
#endif


/*tydrv qupw add for truly nt35516 lcd */
#ifdef TYQ_TRULY_SLEEP_IN_RESET_HIGH
 extern int tyq_get_lcd_id(void);
#endif

//xiangdong modify for 5937 LCD issue
#ifdef TYQ_TRULY_SLEEP_IN_DISPLAY_OFF
 extern int tyq_get_lcd_id(void);
#endif


/*
 * Reserve enough v4l2 space for a double buffered full screen
 * res image (864x480x1.5x2)
 */
#define MSM_V4L2_VIDEO_OVERLAY_BUF_SIZE 1244160

static unsigned fb_size = MSM_FB_SIZE;
static int __init fb_size_setup(char *p)
{
	fb_size = memparse(p, NULL);
	return 0;
}

early_param("fb_size", fb_size_setup);
#if defined(TYQ_LCD_SUPPORT)
static struct regulator_bulk_data regs_lcdc[] = {
	{ .supply = "gp2",   .min_uV = 2850000, .max_uV = 2850000 },
	{ .supply = "bt", .min_uV = 1800000, .max_uV = 1800000 },
};
#else
static uint32_t lcdc_truly_gpio_initialized = 0;
#endif
static struct regulator_bulk_data regs_truly_lcdc[] = {
	{ .supply = "rfrx1",   .min_uV = 1800000, .max_uV = 1800000 },
};

#define SKU3_LCDC_GPIO_DISPLAY_RESET	90
#define SKU3_LCDC_GPIO_SPI_MOSI		19
#define SKU3_LCDC_GPIO_SPI_CLK		20
#define SKU3_LCDC_GPIO_SPI_CS0_N	21
#define SKU3_LCDC_LCD_CAMERA_LDO_2V8	35  /*LCD_CAMERA_LDO_2V8*/
#define SKU3_LCDC_LCD_CAMERA_LDO_1V8	34  /*LCD_CAMERA_LDO_1V8*/
#define SKU3_1_LCDC_LCD_CAMERA_LDO_1V8	58  /*LCD_CAMERA_LDO_1V8*/
#if defined(TYQ_LCD_SUPPORT)
#else
static uint32_t lcdc_truly_gpio_table[] = {
	19,
	20,
	21,
	89,
	90,
};

static char *lcdc_gpio_name_table[5] = {
	"spi_mosi",
	"spi_clk",
	"spi_cs",
	"gpio_bkl_en",
	"gpio_disp_reset",
};

static char lcdc_splash_is_enabled(void);
static int lcdc_truly_gpio_init(void)
{
	int i;
	int rc = 0;

	if (!lcdc_truly_gpio_initialized) {
		for (i = 0; i < ARRAY_SIZE(lcdc_truly_gpio_table); i++) {
			rc = gpio_request(lcdc_truly_gpio_table[i],
				lcdc_gpio_name_table[i]);
			if (rc < 0) {
				pr_err("Error request gpio %s\n",
					lcdc_gpio_name_table[i]);
				goto truly_gpio_fail;
			}
			rc = gpio_tlmm_config(GPIO_CFG(lcdc_truly_gpio_table[i],
				0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			if (rc < 0) {
				pr_err("Error config lcdc gpio:%d\n",
					lcdc_truly_gpio_table[i]);
				goto truly_gpio_fail;
			}
			if (lcdc_splash_is_enabled())
				rc = gpio_direction_output(
					lcdc_truly_gpio_table[i], 1);
			else
				rc = gpio_direction_output(
					lcdc_truly_gpio_table[i], 0);
			if (rc < 0) {
				pr_err("Error direct lcdc gpio:%d\n",
					lcdc_truly_gpio_table[i]);
				goto truly_gpio_fail;
			}
		}

			lcdc_truly_gpio_initialized = 1;
	}

	return rc;

truly_gpio_fail:
	for (; i >= 0; i--) {
		pr_err("Freeing GPIO: %d", lcdc_truly_gpio_table[i]);
		gpio_free(lcdc_truly_gpio_table[i]);
	}

	lcdc_truly_gpio_initialized = 0;
	return rc;
}

#endif
void sku3_lcdc_lcd_camera_power_init(void)
{
	int rc = 0;
	u32 socinfo = socinfo_get_platform_type();

	  /* LDO_EXT2V8 */
	if (gpio_request(SKU3_LCDC_LCD_CAMERA_LDO_2V8, "lcd_camera_ldo_2v8")) {
		pr_err("failed to request gpio lcd_camera_ldo_2v8\n");
		return;
	}

	rc = gpio_tlmm_config(GPIO_CFG(SKU3_LCDC_LCD_CAMERA_LDO_2V8, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
		GPIO_CFG_ENABLE);

	if (rc < 0) {
		pr_err("%s:unable to enable lcd_camera_ldo_2v8!\n", __func__);
		goto fail_gpio2;
	}

	/* LDO_EVT1V8 */
	if (socinfo == 0x0B) {
		if (gpio_request(SKU3_LCDC_LCD_CAMERA_LDO_1V8,
				"lcd_camera_ldo_1v8")) {
			pr_err("failed to request gpio lcd_camera_ldo_1v8\n");
			goto fail_gpio1;
		}

		rc = gpio_tlmm_config(GPIO_CFG(SKU3_LCDC_LCD_CAMERA_LDO_1V8, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);

		if (rc < 0) {
			pr_err("%s: unable to enable lcdc_camera_ldo_1v8!\n",
				__func__);
			goto fail_gpio1;
		}
	} else if (socinfo == 0x0F || machine_is_msm8625_qrd7()) {
		if (gpio_request(SKU3_1_LCDC_LCD_CAMERA_LDO_1V8,
				"lcd_camera_ldo_1v8")) {
			pr_err("failed to request gpio lcd_camera_ldo_1v8\n");
			goto fail_gpio1;
		}

		rc = gpio_tlmm_config(GPIO_CFG(SKU3_1_LCDC_LCD_CAMERA_LDO_1V8,
			0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);

		if (rc < 0) {
			pr_err("%s: unable to enable lcdc_camera_ldo_1v8!\n",
				__func__);
			goto fail_gpio1;
		}
	}

	rc = regulator_bulk_get(NULL, ARRAY_SIZE(regs_truly_lcdc),
			regs_truly_lcdc);
	if (rc)
		pr_err("%s: could not get regulators: %d\n", __func__, rc);

	rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_truly_lcdc),
			regs_truly_lcdc);
	if (rc)
		pr_err("%s: could not set voltages: %d\n", __func__, rc);

	return;

fail_gpio1:
	if (socinfo == 0x0B)
		gpio_free(SKU3_LCDC_LCD_CAMERA_LDO_1V8);
	else if (socinfo == 0x0F || machine_is_msm8625_qrd7())
		gpio_free(SKU3_1_LCDC_LCD_CAMERA_LDO_1V8);
fail_gpio2:
	gpio_free(SKU3_LCDC_LCD_CAMERA_LDO_2V8);
	return;
}

int sku3_lcdc_lcd_camera_power_onoff(int on)
{
	int rc = 0;
	u32 socinfo = socinfo_get_platform_type();
	static int refcount = 0;

	if (on) {
		if (refcount > 0)
		{
			refcount++;
			return rc;
		}
		if (socinfo == 0x0B)
			gpio_set_value_cansleep(SKU3_LCDC_LCD_CAMERA_LDO_1V8,
				1);
		else if (socinfo == 0x0F || machine_is_msm8625_qrd7())
			gpio_set_value_cansleep(SKU3_1_LCDC_LCD_CAMERA_LDO_1V8,
				1);

		gpio_set_value_cansleep(SKU3_LCDC_LCD_CAMERA_LDO_2V8, 1);

		rc = regulator_bulk_enable(ARRAY_SIZE(regs_truly_lcdc),
				regs_truly_lcdc);
		if (rc)
			pr_err("%s: could not enable regulators: %d\n",
				__func__, rc);
		else
			refcount++;
	} else {
		if (refcount > 1)
		{
			refcount--;
			return rc;
		}
		if (socinfo == 0x0B)
			gpio_set_value_cansleep(SKU3_LCDC_LCD_CAMERA_LDO_1V8,
				0);
		else if (socinfo == 0x0F || machine_is_msm8625_qrd7())
			gpio_set_value_cansleep(SKU3_1_LCDC_LCD_CAMERA_LDO_1V8,
				0);

		gpio_set_value_cansleep(SKU3_LCDC_LCD_CAMERA_LDO_2V8, 0);

		rc = regulator_bulk_disable(ARRAY_SIZE(regs_truly_lcdc),
				regs_truly_lcdc);
		if (rc)
			pr_err("%s: could not disable regulators: %d\n",
				__func__, rc);
		else
			refcount--;
	}

	return rc;
}
EXPORT_SYMBOL(sku3_lcdc_lcd_camera_power_onoff);
#if defined(TYQ_LCD_SUPPORT)
#else
static int sku3_lcdc_power_save(int on)
{
	int rc = 0;
	static int cont_splash_done;

	if (on) {
		sku3_lcdc_lcd_camera_power_onoff(1);
		rc = lcdc_truly_gpio_init();
		if (rc < 0) {
			pr_err("%s(): Truly GPIO initializations failed",
				__func__);
			return rc;
		}

		if (lcdc_splash_is_enabled() && !cont_splash_done) {
			cont_splash_done = 1;
			return rc;
		}

		if (lcdc_truly_gpio_initialized) {
			/*LCD reset*/
			gpio_set_value(SKU3_LCDC_GPIO_DISPLAY_RESET, 1);
			msleep(20);
			gpio_set_value(SKU3_LCDC_GPIO_DISPLAY_RESET, 0);
			msleep(20);
			gpio_set_value(SKU3_LCDC_GPIO_DISPLAY_RESET, 1);
			msleep(20);
		}
	} else {
		/* pull down LCD IO to avoid current leakage */
		gpio_set_value(SKU3_LCDC_GPIO_SPI_MOSI, 0);
		gpio_set_value(SKU3_LCDC_GPIO_SPI_CLK, 0);
		gpio_set_value(SKU3_LCDC_GPIO_SPI_CS0_N, 0);
		gpio_set_value(SKU3_LCDC_GPIO_DISPLAY_RESET, 0);

		sku3_lcdc_lcd_camera_power_onoff(0);
	}
	return rc;
}

static struct msm_panel_common_pdata lcdc_truly_panel_data = {
	.panel_config_gpio = NULL,
	.gpio_num	  = lcdc_truly_gpio_table,
};

static struct platform_device lcdc_truly_panel_device = {
	.name   = "lcdc_truly_hvga_ips3p2335_pt",
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_truly_panel_data,
	}
};

static struct regulator_bulk_data regs_lcdc[] = {
	{ .supply = "gp2",   .min_uV = 2850000, .max_uV = 2850000 },
	{ .supply = "msme1", .min_uV = 1800000, .max_uV = 1800000 },
};
#endif
static uint32_t lcdc_gpio_initialized = 0;
#if defined(TYQ_LCD_SUPPORT)
#define LCD_RST			78
#define LCDC_SPI_CS		33
#define LCDC_SPI_SCLK		97
#define LCDC_SPI_SDO		98
#define LCDC_SPI_SDI		96
#define LCD_BL_EN		109
static const char * const msm_fb_lcdc_vreg[] = {
		"gp2",
		"bt",
};

static const int msm_fb_lcdc_vreg_mV[] = {
	2800,
	1800,
};

struct vreg *lcdc_vreg[ARRAY_SIZE(msm_fb_lcdc_vreg)];

//static struct msm_rpc_endpoint *lcdc_ep;

#define LCDC_CONFIG_PROC          21
#define LCDC_UN_CONFIG_PROC       22
#define LCDC_API_PROG             0x30000066
#define LCDC_API_VERS             0x00010001

#if 0
static int msm_fb_lcdc_config(int on)
{
	int rc = 0;
	struct rpc_request_hdr hdr;

	if (on)
		pr_info("lcdc config\n");
	else
		pr_info("lcdc un-config\n");

	lcdc_ep = msm_rpc_connect_compatible(LCDC_API_PROG, LCDC_API_VERS, 0);
	if (IS_ERR(lcdc_ep)) {
		printk(KERN_ERR "%s: msm_rpc_connect failed! rc = %ld\n",
			__func__, PTR_ERR(lcdc_ep));
		return -EINVAL;
	}

	rc = msm_rpc_call(lcdc_ep,
				(on) ? LCDC_CONFIG_PROC : LCDC_UN_CONFIG_PROC,
				&hdr, sizeof(hdr),
				5 * HZ);
	if (rc)
		printk(KERN_ERR
			"%s: msm_rpc_call failed! rc = %d\n", __func__, rc);

	msm_rpc_close(lcdc_ep);
	return rc;
}
#endif

static int gpio_array_num[] = {
				LCDC_SPI_SCLK, /* spi_clk */
				LCDC_SPI_CS,    /* spi_cs  */
				LCDC_SPI_SDO, /* spi_sdoi */
				LCD_RST,
				LCD_BL_EN,
				};

static void lcdc_truly_gpio_init(void)
{
	int rc = 0;
		
	if (!lcdc_gpio_initialized) {
		if (gpio_request(LCDC_SPI_SCLK, "spi_clk")) {
			pr_err("failed to request gpio spi_clk\n");
			return;
		}
		if (gpio_request(LCDC_SPI_CS, "spi_cs")) {
			pr_err("failed to request gpio spi_cs0_N\n");
			goto fail_gpio5;
		}
		if (gpio_request(LCDC_SPI_SDO, "spi_sdo")) {
			pr_err("failed to request gpio spi_mosi\n");
			goto fail_gpio4;
		}
	      if (gpio_request(LCD_RST, "gpio_rst")) {
			pr_err("failed to request gpio spi_mosi\n");
			goto fail_gpio3;
		}

		if (gpio_request(LCD_BL_EN, "gpio_en")) {
			pr_err("failed to request gpio_bkl_en\n");
			goto fail_gpio2;
		}

		rc = regulator_bulk_get(NULL, ARRAY_SIZE(regs_lcdc), regs_lcdc);
		if (rc) {
			pr_err("%s: could not get regulators: %d\n",
					__func__, rc);
			goto fail_gpio1;
		}
		rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_lcdc),
				regs_lcdc);
		/*tyrd pengwei added for enable set ldo voltage 20120706*/
		regulator_bulk_enable(ARRAY_SIZE(regs_lcdc),
				regs_lcdc);
		if (rc) {
			pr_err("%s: could not set voltages: %d\n",
					__func__, rc);
			goto fail_vreg;
		}
		lcdc_gpio_initialized = 1;
	}
	return;

fail_vreg:
	regulator_bulk_free(ARRAY_SIZE(regs_lcdc), regs_lcdc);

fail_gpio1:
	gpio_free(LCD_BL_EN);
fail_gpio2:
	gpio_free(LCD_RST);
fail_gpio3:
	gpio_free(LCDC_SPI_SDO);
fail_gpio4:
	gpio_free(LCDC_SPI_CS);
fail_gpio5:
	gpio_free(LCDC_SPI_SCLK);
	
	lcdc_gpio_initialized = 0;
}


static uint32_t lcdc_gpio_table[] = {
	GPIO_CFG(LCDC_SPI_SCLK, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(LCDC_SPI_CS, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(LCDC_SPI_SDO, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(LCD_RST,  0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(LCD_BL_EN,  0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
};
static uint32_t lcdc_sleep_gpio_table[] = {
	GPIO_CFG(LCDC_SPI_SCLK, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	GPIO_CFG(LCDC_SPI_CS, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	GPIO_CFG(LCDC_SPI_SDO, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	GPIO_CFG(LCD_RST,  0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
	GPIO_CFG(LCD_BL_EN,  0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),

};

static void config_lcdc_gpio_table(uint32_t *table, int len, unsigned enable)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n],
			enable ? GPIO_CFG_ENABLE : GPIO_CFG_DISABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

static void lcdc_truly_config_gpios(int enable)
{
printk("lcdc_truly_config_gpios %d \n",enable);
	if(enable == 1)
	{
	config_lcdc_gpio_table(lcdc_gpio_table,
		ARRAY_SIZE(lcdc_gpio_table), enable);
	}
	else
	{
		config_lcdc_gpio_table(lcdc_sleep_gpio_table,
			ARRAY_SIZE(lcdc_sleep_gpio_table), 1);
	}
}

static int msm_fb_lcdc_power_save(int on)
{
//      int i, rc = 0;
int rc;
	/* Doing the init of the LCDC GPIOs very late as they are from
		an I2C-controlled IO Expander */
//	lcdc_truly_gpio_init();
printk("%s  on is :%d\n",__func__,on);
if (1){//(lcdc_gpio_initialized) {

		rc = 1 ? regulator_bulk_enable(
				ARRAY_SIZE(regs_lcdc), regs_lcdc) :
			  regulator_bulk_disable(
				ARRAY_SIZE(regs_lcdc), regs_lcdc);

		if (rc)
			pr_err("%s: could not %sable regulators: %d\n",
					__func__, 1 ? "en" : "dis", rc);
	}
#if 0
	if (lcdc_gpio_initialized) {
		for (i = 0; i < ARRAY_SIZE(msm_fb_lcdc_vreg); i++) {
			if (on) {
				rc = vreg_enable(lcdc_vreg[i]);

				if (rc) {
					printk(KERN_ERR "vreg_enable: %s vreg"
						"operation failed\n",
						msm_fb_lcdc_vreg[i]);
						goto lcdc_vreg_fail;
				}
			} else {
/*tydrv niuli delete for sleep in&out on 20111115  */
		#if 0
				rc = vreg_disable(lcdc_vreg[i]);

				if (rc) {
					printk(KERN_ERR "vreg_disable: %s vreg "
						"operation failed\n",
						msm_fb_lcdc_vreg[i]);
					goto lcdc_vreg_fail;
				}
		#endif		
			}
		}

	}
	return rc;

lcdc_vreg_fail:
	if (on) {
		for (; i > 0; i--)
			vreg_disable(lcdc_vreg[i - 1]);
	} else {
		for (; i > 0; i--)
			vreg_enable(lcdc_vreg[i - 1]);
	}

   return rc; 
 #endif

   return on;
}

static struct lcdc_platform_data lcdc_pdata = {
	//.lcdc_gpio_config = msm_fb_lcdc_config,
	.lcdc_gpio_config = NULL,
	.lcdc_power_save   = msm_fb_lcdc_power_save,
};

static struct msm_panel_common_pdata lcdc_truly_panel_data = {
	.panel_config_gpio = lcdc_truly_config_gpios,
	.gpio_num          = gpio_array_num,
}; 
#if defined(TYQ_38INCH_WVGA_LCD_SUPPORT)||\
	defined(TYQ_40INCH_WVGA_LCD_SUPPORT)||\
	defined(TYQ_40INCH_WVGA_TRULY_LCD_SUPPORT)||\
	defined(TYQ_43INCH_WVGA_TRULY_OTM8018B_LCD_SUPPORT)
static struct platform_device lcdc_truly_panel_device = {
	.name   = "lcdc_truly_wvga",
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_truly_panel_data,
	}
};
#elif defined(TYQ_35INCH_HVGA_TIANMA_SUPPORT) 
static struct platform_device lcdc_truly_panel_device = {
	.name   = "lcdc_truly_hvga",
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_truly_panel_data,
	}
};
#endif
#if 1
void lcd_display_init(void)
{
	//int i, rc = 0;

	printk("%s  init: %d \n",__func__,lcdc_gpio_initialized);
	lcdc_truly_gpio_init();
	/*if (lcdc_gpio_initialized) {
		gpio_set_value_cansleep(GPIO_DISPLAY_PWR_EN, 1);
		gpio_set_value_cansleep(GPIO_BACKLIGHT_EN, 1);

		rc = 1 ? regulator_bulk_enable(
				ARRAY_SIZE(regs_lcdc), regs_lcdc) :
			  regulator_bulk_disable(
				ARRAY_SIZE(regs_lcdc), regs_lcdc);

		if (rc)
			pr_err("%s: could not %sable regulators: %d\n",
					__func__, 1 ? "en" : "dis", rc);
	}*/
	/*if (lcdc_gpio_initialized) {
		for (i = 0; i < ARRAY_SIZE(msm_fb_lcdc_vreg); i++) {
				rc = vreg_enable(lcdc_vreg[i]);

				if (rc) {
					printk(KERN_ERR "vreg_enable: %s vreg"
						"operation failed\n",
						msm_fb_lcdc_vreg[i]);
				}
			}
	}*/
}
#endif
#else
static void lcdc_toshiba_gpio_init(void)
{
	int rc = 0;
	if (!lcdc_gpio_initialized) {
		if (gpio_request(GPIO_SPI_CLK, "spi_clk")) {
			pr_err("failed to request gpio spi_clk\n");
			return;
		}
		if (gpio_request(GPIO_SPI_CS0_N, "spi_cs")) {
			pr_err("failed to request gpio spi_cs0_N\n");
			goto fail_gpio6;
		}
		if (gpio_request(GPIO_SPI_MOSI, "spi_mosi")) {
			pr_err("failed to request gpio spi_mosi\n");
			goto fail_gpio5;
		}
		if (gpio_request(GPIO_SPI_MISO, "spi_miso")) {
			pr_err("failed to request gpio spi_miso\n");
			goto fail_gpio4;
		}
		if (gpio_request(GPIO_DISPLAY_PWR_EN, "gpio_disp_pwr")) {
			pr_err("failed to request gpio_disp_pwr\n");
			goto fail_gpio3;
		}
		if (gpio_request(GPIO_BACKLIGHT_EN, "gpio_bkl_en")) {
			pr_err("failed to request gpio_bkl_en\n");
			goto fail_gpio2;
		}
		pmapp_disp_backlight_init();

		rc = regulator_bulk_get(NULL, ARRAY_SIZE(regs_lcdc),
					regs_lcdc);
		if (rc) {
			pr_err("%s: could not get regulators: %d\n",
					__func__, rc);
			goto fail_gpio1;
		}

		rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_lcdc),
				regs_lcdc);
		if (rc) {
			pr_err("%s: could not set voltages: %d\n",
					__func__, rc);
			goto fail_vreg;
		}
		lcdc_gpio_initialized = 1;
	}
	return;
fail_vreg:
	regulator_bulk_free(ARRAY_SIZE(regs_lcdc), regs_lcdc);
fail_gpio1:
	gpio_free(GPIO_BACKLIGHT_EN);
fail_gpio2:
	gpio_free(GPIO_DISPLAY_PWR_EN);
fail_gpio3:
	gpio_free(GPIO_SPI_MISO);
fail_gpio4:
	gpio_free(GPIO_SPI_MOSI);
fail_gpio5:
	gpio_free(GPIO_SPI_CS0_N);
fail_gpio6:
	gpio_free(GPIO_SPI_CLK);
	lcdc_gpio_initialized = 0;
}

static uint32_t lcdc_gpio_table[] = {
	GPIO_SPI_CLK,
	GPIO_SPI_CS0_N,
	GPIO_SPI_MOSI,
	GPIO_DISPLAY_PWR_EN,
	GPIO_BACKLIGHT_EN,
	GPIO_SPI_MISO,
};

static void config_lcdc_gpio_table(uint32_t *table, int len, unsigned enable)
{
	int n;

	if (lcdc_gpio_initialized) {
		/* All are IO Expander GPIOs */
		for (n = 0; n < (len - 1); n++)
			gpio_direction_output(table[n], 1);
	}
}

static void lcdc_toshiba_config_gpios(int enable)
{
	config_lcdc_gpio_table(lcdc_gpio_table,
		ARRAY_SIZE(lcdc_gpio_table), enable);
}

static int msm_fb_lcdc_power_save(int on)
{
	int rc = 0;
	/* Doing the init of the LCDC GPIOs very late as they are from
		an I2C-controlled IO Expander */
	lcdc_toshiba_gpio_init();

	if (lcdc_gpio_initialized) {
		gpio_set_value_cansleep(GPIO_DISPLAY_PWR_EN, on);
		gpio_set_value_cansleep(GPIO_BACKLIGHT_EN, on);

		rc = on ? regulator_bulk_enable(
				ARRAY_SIZE(regs_lcdc), regs_lcdc) :
			  regulator_bulk_disable(
				ARRAY_SIZE(regs_lcdc), regs_lcdc);

		if (rc)
			pr_err("%s: could not %sable regulators: %d\n",
					__func__, on ? "en" : "dis", rc);
	}

	return rc;
}

static int lcdc_toshiba_set_bl(int level)
{
	int ret;

	ret = pmapp_disp_backlight_set_brightness(level);
	if (ret)
		pr_err("%s: can't set lcd backlight!\n", __func__);

	return ret;
}


static int msm_lcdc_power_save(int on)
{
	int rc = 0;
	if (machine_is_msm7627a_qrd3() || machine_is_msm8625_qrd7())
		rc = sku3_lcdc_power_save(on);
	else
		rc = msm_fb_lcdc_power_save(on);

	return rc;
}

static struct lcdc_platform_data lcdc_pdata = {
	.lcdc_gpio_config = NULL,
	.lcdc_power_save   = msm_lcdc_power_save,
};

static int lcd_panel_spi_gpio_num[] = {
		GPIO_SPI_MOSI,  /* spi_sdi */
		GPIO_SPI_MISO,  /* spi_sdoi */
		GPIO_SPI_CLK,   /* spi_clk */
		GPIO_SPI_CS0_N, /* spi_cs  */
};

static struct msm_panel_common_pdata lcdc_toshiba_panel_data = {
	.panel_config_gpio = lcdc_toshiba_config_gpios,
	.pmic_backlight = lcdc_toshiba_set_bl,
	.gpio_num	 = lcd_panel_spi_gpio_num,
};

static struct platform_device lcdc_toshiba_panel_device = {
	.name   = "lcdc_toshiba_fwvga_pt",
	.id     = 0,
	.dev    = {
		.platform_data = &lcdc_toshiba_panel_data,
	}
};
#endif

static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

#ifdef CONFIG_MSM_V4L2_VIDEO_OVERLAY_DEVICE
static struct resource msm_v4l2_video_overlay_resources[] = {
	{
		.flags = IORESOURCE_DMA,
	}
};
#endif

#define LCDC_TOSHIBA_FWVGA_PANEL_NAME   "lcdc_toshiba_fwvga_pt"
#define MIPI_CMD_RENESAS_FWVGA_PANEL_NAME       "mipi_cmd_renesas_fwvga"

static int msm_fb_detect_panel(const char *name)
{
	int ret = -ENODEV;

	if (machine_is_msm7x27a_surf() || machine_is_msm7625a_surf() ||
			machine_is_msm8625_surf()) {
		if (!strncmp(name, "lcdc_toshiba_fwvga_pt", 21) ||
				!strncmp(name, "mipi_cmd_renesas_fwvga", 22))
			ret = 0;
	} else if (machine_is_msm7x27a_ffa() || machine_is_msm7625a_ffa()
					|| machine_is_msm8625_ffa()) {
		if (!strncmp(name, "mipi_cmd_renesas_fwvga", 22))
			ret = 0;
	} else if (machine_is_msm7627a_qrd1()) {
		if (!strncmp(name, "mipi_video_truly_wvga", 21))
			ret = 0;
	} else if (machine_is_msm7627a_qrd3() || machine_is_msm8625_qrd7()) {
		if (!strncmp(name, "lcdc_truly_hvga_ips3p2335_pt", 28))
			ret = 0;
	} else if (machine_is_msm7627a_evb() || machine_is_msm8625_evb() ||
			machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a()) {
		if (!strncmp(name, "mipi_cmd_nt35510_wvga", 21))
			ret = 0;
	} else if (machine_is_msm8625q_skud() || machine_is_msm8625q_evbd()) {
#if defined(TYQ_NT35516_MIPI_QHD_SUPPORT)
       if (!strncmp(name, "mipi_video_nt35516_qhd", 22))
       	    ret = 0;
#elif defined(TYQ_NT35590_MIPI_HD_SUPPORT)
       if (!strncmp(name, "mipi_video_nt35590_hd", 21))
       	    ret = 0;
#elif defined(TYQ_HX8392_MIPI_HD_SUPPORT)
       if (!strncmp(name, "mipi_video_hx8392_hd", 20))
       	    ret = 0;
#elif defined(TYQ_OTM8018B_MIPI_WVGA_SUPPORT)
	if ( !strncmp(name,"mipi_video_otm8018b_wvga",24))
	    ret =0;
/*tydrv pengwei added for fwvga tianma 8018*/
#elif defined(TYQ_OTM8018B_MIPI_FWVGA_SUPPORT)
	if ( !strncmp(name,"mipi_video_otm8018b_fwvga",25))
		ret =0;

//xiangdong add for TBW5937 5.3inch LCD
#elif defined(TYQ_53INCH_NT35516_MIPI_QHD_SUPPORT)
		if ( !strncmp(name,"mipi_video_53inch_nt35516_qhd",29))
			ret =0;
#elif defined(TYQ_HX8379A_MIPI_WVGA_SUPPORT)
		if (!strncmp(name, "mipi_video_hx8379a_wvga", 23)) 
        		ret = 0;
#else
		if (!strncmp(name, "mipi_video_hx8389b_qhd", 22)) 
                        ret = 0;
#endif
	
	} else if (machine_is_msm8625q_skue()) {
		if (!strncmp(name, "mipi_video_otm9605a_qhd", 23))
                        ret = 0;
        }

#if !defined(CONFIG_FB_MSM_LCDC_AUTO_DETECT) && \
	!defined(CONFIG_FB_MSM_MIPI_PANEL_AUTO_DETECT) && \
	!defined(CONFIG_FB_MSM_LCDC_MIPI_PANEL_AUTO_DETECT)
		if (machine_is_msm7x27a_surf() ||
			machine_is_msm7625a_surf() ||
			machine_is_msm8625_surf()) {
			if (!strncmp(name, LCDC_TOSHIBA_FWVGA_PANEL_NAME,
				strnlen(LCDC_TOSHIBA_FWVGA_PANEL_NAME,
					PANEL_NAME_MAX_LEN)))
				return 0;
		}
#endif

	return ret;
}

static int mipi_truly_set_bl(int on)
{
	gpio_set_value_cansleep(QRD_GPIO_BACKLIGHT_EN, !!on);

	return 1;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
	.dev    = {
		.platform_data = &msm_fb_pdata,
	}
};

#ifdef CONFIG_MSM_V4L2_VIDEO_OVERLAY_DEVICE
static struct platform_device msm_v4l2_video_overlay_device = {
		.name   = "msm_v4l2_overlay_pd",
		.id     = 0,
		.num_resources  = ARRAY_SIZE(msm_v4l2_video_overlay_resources),
		.resource       = msm_v4l2_video_overlay_resources,
	};
#endif


#ifdef CONFIG_FB_MSM_MIPI_DSI
static int mipi_renesas_set_bl(int level)
{
	int ret;

	ret = pmapp_disp_backlight_set_brightness(level);

	if (ret)
		pr_err("%s: can't set lcd backlight!\n", __func__);

	return ret;
}

static struct msm_panel_common_pdata mipi_renesas_pdata = {
	.pmic_backlight = mipi_renesas_set_bl,
};


static struct platform_device mipi_dsi_renesas_panel_device = {
	.name = "mipi_renesas",
	.id = 0,
	.dev    = {
		.platform_data = &mipi_renesas_pdata,
	}
};
#endif

/*TYRD wuxh add for gpio 27 pwm backligth control  on 20121206*/
#ifdef TYQ_QCT_SUPPORT
#if defined(TYQ_40INCH_WVGA_TRULY_LCD_SUPPORT)
#else
#define LCD_BL_EN 109

static int pwm_backlight_control(int level, int mode)
{
	static struct clk *gp_clkto_pwm =NULL; 
	static int bl_pre_level=0;
    int   bl_duty_num;
	int rc =0;

	
	if(	gp_clkto_pwm==NULL)
	{
		
		gp_clkto_pwm = clk_get(NULL, "core_clk");
		if (IS_ERR(gp_clkto_pwm))
		{
			printk(" clk_get gp_clk fail error !!!!!!\r\n");
			return 0;
		}
		rc = gpio_request(LCD_BL_EN, "backlight_en");
		if(rc<0)
		{
			printk("%s,gpio_reqest error\r\n", __func__);
			return 0;
		}
		rc = gpio_tlmm_config(GPIO_CFG(LCD_BL_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		
		if(rc<0)
		{			
			printk("%s,gpio_tlmm_config error\r\n", __func__);
			return 0;
		}
	}
	printk("pwm_backlight_control level =%d\r\n",level);


	if(level==0)
	{
	   gpio_set_value(LCD_BL_EN,0); 

	   
	   printk("pwm_backlight_control level clkdisable =%d\r\n",level);
	   clk_disable_unprepare(gp_clkto_pwm);
	}
	else
	{
	
		if(bl_pre_level == 0)
		{
			clk_prepare_enable(gp_clkto_pwm);
			
			printk("pwm_backlight_control level clkenable =%d\r\n",level);
		}
		
		{
			#if 1
			if(level<13)
				bl_duty_num =level*1;/*TYDRV qupw for level enchange*/
			else
				bl_duty_num =level*2-12;/*TYDRV niuli for 90% brightness*/
			#endif 
			/*pengwei modify duty ratio of 4%-88.7%*/

			
			clk_set_rate(gp_clkto_pwm,bl_duty_num);
			//clk_set_rate(gp_clkto_pwm,level);

			if(bl_pre_level == 0)
			{
				#if defined(TYQ_TBW5932_SUPPORT)
				msleep(50);
				#endif
				gpio_set_value(LCD_BL_EN,1);	
			}
		}
	}


	bl_pre_level = level;
	return 0;

}

#endif
#endif
static int evb_backlight_control(int level, int mode)
{

	int i = 0;
	int remainder, ret = 0;
	u32 socinfo = socinfo_get_version();

	/* device address byte = 0x72 */
	if (!mode) {
		if (socinfo != 0x10000 && level == 0)
			level = 10;
		gpio_set_value(96, 0);
		udelay(67);
		gpio_set_value(96, 1);
		udelay(33);
		gpio_set_value(96, 0);
		udelay(33);
		gpio_set_value(96, 1);
		udelay(67);
		gpio_set_value(96, 0);
		udelay(33);
		gpio_set_value(96, 1);
		udelay(67);
		gpio_set_value(96, 0);
		udelay(33);
		gpio_set_value(96, 1);
		udelay(67);
		gpio_set_value(96, 0);
		udelay(67);
		gpio_set_value(96, 1);
		udelay(33);
		gpio_set_value(96, 0);
		udelay(67);
		gpio_set_value(96, 1);
		udelay(33);
		gpio_set_value(96, 0);
		udelay(33);
		gpio_set_value(96, 1);
		udelay(67);
		gpio_set_value(96, 0);
		udelay(67);
		gpio_set_value(96, 1);
		udelay(33);

		/* t-EOS and t-start */
		gpio_set_value(96, 0);
		ndelay(4200);
		gpio_set_value(96, 1);
		ndelay(9000);

		/* data byte */
		/* RFA = 0 */
		gpio_set_value(96, 0);
		udelay(67);
		gpio_set_value(96, 1);
		udelay(33);

		/* Address bits */
		gpio_set_value(96, 0);
		udelay(67);
		gpio_set_value(96, 1);
		udelay(33);
		gpio_set_value(96, 0);
		udelay(67);
		gpio_set_value(96, 1);
		udelay(33);

		/* Data bits */
		for (i = 0; i < 5; i++) {
			remainder = (level) & (16);
			if (remainder) {
				gpio_set_value(96, 0);
				udelay(33);
				gpio_set_value(96, 1);
				udelay(67);
			} else {
				gpio_set_value(96, 0);
				udelay(67);
				gpio_set_value(96, 1);
				udelay(33);
			}
			level = level << 1;
		}

		/* t-EOS */
		gpio_set_value(96, 0);
		ndelay(12000);
		gpio_set_value(96, 1);
	} else {
		ret = pmapp_disp_backlight_set_brightness(level);
		 if (ret)
			pr_err("%s: can't set lcd backlight!\n", __func__);
	}

	return ret;
}

#ifdef TYQ_TBW5939_SUPPORT

#define LCD_BL_EN 109

static int gpio_enable_backlight_control(int level, int mode)
{
	static bool bl_gpio_init = false; 
	static int bl_pre_level = 0;
	int rc =0;

	
	if ( bl_gpio_init == false ) {
		
		rc = gpio_request(LCD_BL_EN, "backlight_en");
		
		if (rc < 0)	{
			printk("%s,gpio_reqest error\r\n", __func__);
			return 0;
		}

		rc = gpio_tlmm_config(GPIO_CFG(LCD_BL_EN, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
		
		if(rc<0)
		{			
			printk("%s,gpio_tlmm_config error\r\n", __func__);
			return 0;
		}

		bl_gpio_init = true;
	}

	printk("gpio enable backlight_control level =%d\r\n",level);

	if ( level==0 )	{	   
	   evb_backlight_control(255-level, mode);
	   gpio_set_value(LCD_BL_EN, 0); 
	   printk("gpio enable backlight_control level clkdisable =%d\r\n",level);
	} else {
						
			evb_backlight_control(255-level, mode);
			if(bl_pre_level == 0)
			{
				gpio_set_value(LCD_BL_EN, 1);	
			}
	}

	bl_pre_level = level;
	return 0;
}

#endif

static int mipi_NT35510_rotate_panel(void)
{
	int rotate = 0;
	if (machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a())
		rotate = 1;

	return rotate;
}

static int skue_backlight_control(int level, int mode)
{
        int ret = 0;

        ret = pmapp_disp_backlight_set_brightness(level);
        if (ret)
                pr_err("%s: can't set lcd backlight!\n", __func__);

        return ret;
}

static struct msm_panel_common_pdata mipi_truly_pdata = {
	.pmic_backlight = mipi_truly_set_bl,
};

static struct platform_device mipi_dsi_truly_panel_device = {
	.name   = "mipi_truly",
	.id     = 0,
	.dev    = {
		.platform_data = &mipi_truly_pdata,
	}
};

static struct msm_panel_common_pdata mipi_NT35510_pdata = {
	.backlight    = evb_backlight_control,
	.rotate_panel = mipi_NT35510_rotate_panel,
};

static struct platform_device mipi_dsi_NT35510_panel_device = {
	.name = "mipi_NT35510",
	.id   = 0,
	.dev  = {
		.platform_data = &mipi_NT35510_pdata,
	}
};

/*qupw add defs */
#ifdef CONFIG_FB_MSM_MIPI_DSI_HX8389B

static struct msm_panel_common_pdata mipi_hx8389b_pdata = {
	/*TYRD wuxh add for gpio 27 pwm backligth control  on 20121206*/
#ifdef TYQ_QCT_SUPPORT
	.backlight	  = pwm_backlight_control,
#else
	.backlight    = evb_backlight_control,
#endif
	.rotate_panel = NULL,
};

static struct platform_device mipi_dsi_hx8389b_panel_device = {
	.name = "mipi_hx8389b",
	.id   = 0,
	.dev  = {
		.platform_data = &mipi_hx8389b_pdata,
	}
};
#endif

/*thomasyang add def*/
#ifdef CONFIG_FB_MSM_MIPI_DSI_HX8379A
static struct msm_panel_common_pdata mipi_hx8379a_pdata = {
	/*TYRD wuxh add for gpio 27 pwm backligth control  on 20121206*/
#if defined (TYQ_TBW5939_SUPPORT)
	.backlight	  = gpio_enable_backlight_control,
#else
	.backlight    = evb_backlight_control,
#endif
	.rotate_panel = NULL,
};

static struct platform_device mipi_dsi_hx8379a_panel_device = {
	.name = "mipi_hx8379a",
	.id   = 0,
	.dev  = {
		.platform_data = &mipi_hx8379a_pdata,
	}
};
#endif
static struct msm_panel_common_pdata mipi_hx8392a_pdata = {
	.backlight = evb_backlight_control,
	.rotate_panel = NULL,
};

static struct platform_device mipi_dsi_hx8392a_panel_device = {
	.name   = "mipi_hx8392a",
	.id     = 0,
	.dev    = {
		.platform_data = &mipi_hx8392a_pdata,
	}
};

static struct msm_panel_common_pdata mipi_otm9605a_pdata = {
	.backlight    = skue_backlight_control,
	.rotate_panel = NULL,
};

static struct platform_device mipi_dsi_otm9605a_panel_device = {
	.name = "mipi_otm9605a",
	.id   = 0,
	.dev  = {
		.platform_data = &mipi_otm9605a_pdata,
	}
};

static struct msm_panel_common_pdata mipi_NT35516_pdata = {
	.backlight = evb_backlight_control,
};

static struct platform_device mipi_dsi_NT35516_panel_device = {
	.name   = "mipi_truly_tft540960_1_e",
	.id     = 0,
	.dev    = {
		.platform_data = &mipi_NT35516_pdata,
	}
};

static struct msm_panel_common_pdata mipi_NT35590_pdata = {
	.backlight = evb_backlight_control,
};

static struct platform_device mipi_dsi_NT35590_panel_device = {
	.name   = "mipi_NT35590",
	.id     = 0,
	.dev    = {
		.platform_data = &mipi_NT35590_pdata,
	}
};
static struct platform_device *msm_fb_devices[] __initdata = {
	&msm_fb_device,
#if defined(TYQ_LCD_SUPPORT)
#else
	&lcdc_toshiba_panel_device,
#endif
#ifdef CONFIG_FB_MSM_MIPI_DSI
	&mipi_dsi_renesas_panel_device,
#endif
#ifdef CONFIG_MSM_V4L2_VIDEO_OVERLAY_DEVICE
	&msm_v4l2_video_overlay_device,
#endif
};

static struct platform_device *qrd_fb_devices[] __initdata = {
	&msm_fb_device,
	&mipi_dsi_truly_panel_device,
};

static struct platform_device *qrd3_fb_devices[] __initdata = {
	&msm_fb_device,
	&lcdc_truly_panel_device,
};

static struct platform_device *evb_fb_devices[] __initdata = {
	&msm_fb_device,
	&mipi_dsi_NT35510_panel_device,
	&mipi_dsi_NT35516_panel_device,
};
/*tydrv pengwei added for fwvga*/
#if defined(TYQ_OTM8018B_MIPI_WVGA_SUPPORT) || defined(TYQ_OTM8018B_MIPI_FWVGA_SUPPORT)
static struct msm_panel_common_pdata mipi_otm8018b_pdata = {
	/*TYRD wuxh add for gpio 27 pwm backligth control  on 20121206*/
#if defined (TYQ_QCT_SUPPORT)
	.backlight	  = pwm_backlight_control,
#elif defined (TYQ_TBW5939_SUPPORT)
	.backlight	  = gpio_enable_backlight_control,
#else
	.backlight    = evb_backlight_control,
#endif
	.rotate_panel = NULL,
};

static struct platform_device mipi_dsi_otm8018b_panel_device = {
	.name = "mipi_otm8018b",
	.id   = 0,
	.dev  = {
		.platform_data = &mipi_otm8018b_pdata,
	}
};

#endif

/*tydrv qupw add for nt35516 mipi lcd */
#if defined(TYQ_NT35516_MIPI_QHD_SUPPORT)

static struct msm_panel_common_pdata mipi_NT35516_truly_pdata = {
	.backlight = evb_backlight_control,
};

static struct platform_device mipi_dsi_NT35516_truly_panel_device = {
	.name   = "mipi_nt35516",
	.id     = 0,
	.dev    = {
		.platform_data = &mipi_NT35516_truly_pdata,
	}
};

#endif
/*tydrv wang_gj add for nt35590 mipi lcd */
#if defined(TYQ_NT35590_MIPI_HD_SUPPORT)

static struct msm_panel_common_pdata mipi_NT35590_truly_pdata = {
	.backlight = evb_backlight_control,
};

static struct platform_device mipi_dsi_NT35590_truly_panel_device = {
	.name   = "mipi_nt35590",
	.id     = 0,
	.dev    = {
		.platform_data = &mipi_NT35590_truly_pdata,
	}
};

#endif
/*tydrv wang_gj add for hx8392 mipi lcd */
#if defined(TYQ_HX8392_MIPI_HD_SUPPORT)

static struct msm_panel_common_pdata mipi_HX8392_truly_pdata = {
	.backlight = evb_backlight_control,
};

static struct platform_device mipi_dsi_HX8392_truly_panel_device = {
	.name   = "mipi_hx8392",
	.id     = 0,
	.dev    = {
		.platform_data = &mipi_HX8392_truly_pdata,
	}
};

#endif

//xiangdong add for TBW5937 5.3inch LCD
#if defined(TYQ_53INCH_NT35516_MIPI_QHD_SUPPORT)

static struct msm_panel_common_pdata mipi_53inch_NT35516_truly_pdata = {
	.backlight = evb_backlight_control,
};

static struct platform_device mipi_dsi_53inch_NT35516_truly_panel_device = {
	.name   = "mipi_53inch_nt35516",
	.id     = 0,
	.dev    = {
		.platform_data = &mipi_53inch_NT35516_truly_pdata,
	}
};

#endif


static struct platform_device *skud_fb_devices[] __initdata = {
	&msm_fb_device,
#if defined (TYQ_LCD_SUPPORT)
	&lcdc_truly_panel_device,
#endif
		/*qupw add defs*/
#ifdef CONFIG_FB_MSM_MIPI_DSI_HX8389B		
	&mipi_dsi_hx8389b_panel_device,
#endif
#ifdef CONFIG_FB_MSM_MIPI_DSI_HX8379A
	&mipi_dsi_hx8379a_panel_device,
#endif
#if defined(TYQ_OTM8018B_MIPI_WVGA_SUPPORT)  || defined(TYQ_OTM8018B_MIPI_FWVGA_SUPPORT)
	&mipi_dsi_otm8018b_panel_device,
#endif
#if defined(TYQ_NT35516_MIPI_QHD_SUPPORT)
	&mipi_dsi_NT35516_truly_panel_device,
#endif
#if defined(TYQ_NT35590_MIPI_HD_SUPPORT)
	&mipi_dsi_NT35590_truly_panel_device,
#endif
#if defined(TYQ_HX8392_MIPI_HD_SUPPORT)
	&mipi_dsi_HX8392_truly_panel_device,
#endif

//xiangdong add for TBW5937 5.3inch LCD
#if defined(TYQ_53INCH_NT35516_MIPI_QHD_SUPPORT)
	&mipi_dsi_53inch_NT35516_truly_panel_device,
#endif
	&mipi_dsi_NT35590_panel_device,
	&mipi_dsi_hx8392a_panel_device,
};

static struct platform_device *skue_fb_devices[] __initdata = {
	&msm_fb_device,
	&mipi_dsi_otm9605a_panel_device,
};
void __init msm_msm7627a_allocate_memory_regions(void)
{
	void *addr;
	unsigned long fb_size;

	if (machine_is_msm7625a_surf() || machine_is_msm7625a_ffa())
		fb_size = MSM7x25A_MSM_FB_SIZE;
	else if (machine_is_msm7627a_evb() || machine_is_msm8625_evb()
                        || machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a()
                        || machine_is_msm8625q_skue()
                        )
                        
#if defined(TYQ_NT35590_MIPI_HD_SUPPORT)|| defined(TYQ_HX8392_MIPI_HD_SUPPORT)
		fb_size = MSM8x25Q_MSM_FB_SIZE;
#else 
		fb_size = MSM8x25_MSM_FB_SIZE;
#endif
	else if (machine_is_msm8625q_evbd() || machine_is_msm8625q_skud())
		fb_size = MSM8x25Q_MSM_FB_SIZE;
	else
		fb_size = MSM_FB_SIZE;

	addr = alloc_bootmem_align(fb_size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + fb_size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n", fb_size,
						addr, __pa(addr));

#ifdef CONFIG_MSM_V4L2_VIDEO_OVERLAY_DEVICE
	fb_size = MSM_V4L2_VIDEO_OVERLAY_BUF_SIZE;
	addr = alloc_bootmem_align(fb_size, 0x1000);
	msm_v4l2_video_overlay_resources[0].start = __pa(addr);
	msm_v4l2_video_overlay_resources[0].end =
		msm_v4l2_video_overlay_resources[0].start + fb_size - 1;
	pr_debug("allocating %lu bytes at %p (%lx physical) for v4l2\n",
		fb_size, addr, __pa(addr));
#endif

}

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 97,
	.mdp_rev = MDP_REV_303,
#if defined(TYQ_LCD_SUPPORT)
	.cont_splash_enabled = 0x0,
#else
	.cont_splash_enabled = 0x1,
#endif
};

#if defined(TYQ_LCD_SUPPORT)
#else
static char lcdc_splash_is_enabled()
{
	return mdp_pdata.cont_splash_enabled;
}
#endif

#define GPIO_LCDC_BRDG_PD	128
#define GPIO_LCDC_BRDG_RESET_N	129
#define GPIO_LCD_DSI_SEL	125
#define LCDC_RESET_PHYS		0x90008014

static  void __iomem *lcdc_reset_ptr;

static unsigned mipi_dsi_gpio[] = {
		GPIO_CFG(GPIO_LCDC_BRDG_RESET_N, 0, GPIO_CFG_OUTPUT,
		GPIO_CFG_NO_PULL, GPIO_CFG_2MA), /* LCDC_BRDG_RESET_N */
		GPIO_CFG(GPIO_LCDC_BRDG_PD, 0, GPIO_CFG_OUTPUT,
		GPIO_CFG_NO_PULL, GPIO_CFG_2MA), /* LCDC_BRDG_PD */
};

static unsigned lcd_dsi_sel_gpio[] = {
	GPIO_CFG(GPIO_LCD_DSI_SEL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
			GPIO_CFG_2MA),
};

enum {
	DSI_SINGLE_LANE = 1,
	DSI_TWO_LANES,
};

static int msm_fb_get_lane_config(void)
{
	/* For MSM7627A SURF/FFA and QRD */
	int rc = DSI_TWO_LANES;
	if (machine_is_msm7625a_surf() || machine_is_msm7625a_ffa()) {
		rc = DSI_SINGLE_LANE;
		pr_info("DSI_SINGLE_LANES\n");
	} else {
		pr_info("DSI_TWO_LANES\n");
	}
	return rc;
}

static int msm_fb_dsi_client_msm_reset(void)
{
	int rc = 0;

	rc = gpio_request(GPIO_LCDC_BRDG_RESET_N, "lcdc_brdg_reset_n");
	if (rc < 0) {
		pr_err("failed to request lcd brdg reset_n\n");
		return rc;
	}

	rc = gpio_request(GPIO_LCDC_BRDG_PD, "lcdc_brdg_pd");
	if (rc < 0) {
		pr_err("failed to request lcd brdg pd\n");
		return rc;
	}

	rc = gpio_tlmm_config(mipi_dsi_gpio[0], GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("Failed to enable LCDC Bridge reset enable\n");
		goto gpio_error;
	}

	rc = gpio_tlmm_config(mipi_dsi_gpio[1], GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("Failed to enable LCDC Bridge pd enable\n");
		goto gpio_error2;
	}

	rc = gpio_direction_output(GPIO_LCDC_BRDG_RESET_N, 1);
	rc |= gpio_direction_output(GPIO_LCDC_BRDG_PD, 1);
	gpio_set_value_cansleep(GPIO_LCDC_BRDG_PD, 0);

	if (!rc) {
		if (machine_is_msm7x27a_surf() || machine_is_msm7625a_surf()
				|| machine_is_msm8625_surf()) {
			lcdc_reset_ptr = ioremap_nocache(LCDC_RESET_PHYS,
				sizeof(uint32_t));

			if (!lcdc_reset_ptr)
				return 0;
		}
		return rc;
	} else {
		goto gpio_error;
	}

gpio_error2:
	pr_err("Failed GPIO bridge pd\n");
	gpio_free(GPIO_LCDC_BRDG_PD);

gpio_error:
	pr_err("Failed GPIO bridge reset\n");
	gpio_free(GPIO_LCDC_BRDG_RESET_N);
	return rc;
}

static int mipi_truly_sel_mode(int video_mode)
{
	int rc = 0;

	rc = gpio_request(GPIO_LCD_DSI_SEL, "lcd_dsi_sel");
	if (rc < 0)
		goto gpio_error;

	rc = gpio_tlmm_config(lcd_dsi_sel_gpio[0], GPIO_CFG_ENABLE);
	if (rc)
		goto gpio_error;

	rc = gpio_direction_output(GPIO_LCD_DSI_SEL, 1);
	if (!rc) {
		gpio_set_value_cansleep(GPIO_LCD_DSI_SEL, video_mode);
		return rc;
	} else {
		goto gpio_error;
	}

gpio_error:
	pr_err("mipi_truly_sel_mode failed\n");
	gpio_free(GPIO_LCD_DSI_SEL);
	return rc;
}

static int msm_fb_dsi_client_qrd1_reset(void)
{
	int rc = 0;

	rc = gpio_request(GPIO_LCDC_BRDG_RESET_N, "lcdc_brdg_reset_n");
	if (rc < 0) {
		pr_err("failed to request lcd brdg reset_n\n");
		return rc;
	}

	rc = gpio_tlmm_config(mipi_dsi_gpio[0], GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("Failed to enable LCDC Bridge reset enable\n");
		return rc;
	}

	rc = gpio_direction_output(GPIO_LCDC_BRDG_RESET_N, 1);
	if (rc < 0) {
		pr_err("Failed GPIO bridge pd\n");
		gpio_free(GPIO_LCDC_BRDG_RESET_N);
		return rc;
	}

	mipi_truly_sel_mode(1);

	return rc;
}

#define GPIO_QRD3_LCD_BRDG_RESET_N	85
#define GPIO_QRD3_LCD_BACKLIGHT_EN	96
#define GPIO_QRD3_LCD_EXT_2V85_EN	35
#define GPIO_QRD3_LCD_EXT_1V8_EN	40

static unsigned qrd3_mipi_dsi_gpio[] = {
	GPIO_CFG(GPIO_QRD3_LCD_BRDG_RESET_N, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_NO_PULL,
			GPIO_CFG_2MA), /* GPIO_QRD3_LCD_BRDG_RESET_N */
	GPIO_CFG(GPIO_QRD3_LCD_BACKLIGHT_EN, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_NO_PULL,
			GPIO_CFG_2MA), /* GPIO_QRD3_LCD_BACKLIGHT_EN */
	GPIO_CFG(GPIO_QRD3_LCD_EXT_2V85_EN, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_NO_PULL,
			GPIO_CFG_2MA), /* GPIO_QRD3_LCD_EXT_2V85_EN */
	GPIO_CFG(GPIO_QRD3_LCD_EXT_1V8_EN, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_NO_PULL,
			GPIO_CFG_2MA), /* GPIO_QRD3_LCD_EXT_1V8_EN */
};

static int msm_fb_dsi_client_qrd3_reset(void)
{
	int rc = 0;

	rc = gpio_request(GPIO_QRD3_LCD_BRDG_RESET_N, "qrd3_lcd_brdg_reset_n");
	if (rc < 0) {
		pr_err("failed to request qrd3 lcd brdg reset_n\n");
		return rc;
	}

	return rc;
}

#define GPIO_SKUD_LCD_BRDG_RESET_N	78

static unsigned skud_mipi_dsi_gpio[] = {
	GPIO_CFG(GPIO_SKUD_LCD_BRDG_RESET_N, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_NO_PULL,
			GPIO_CFG_2MA), /* GPIO_SKUD_LCD_BRDG_RESET_N */
};

static int msm_fb_dsi_client_skud_reset(void)
{
	int rc = 0;

	rc = gpio_request(GPIO_SKUD_LCD_BRDG_RESET_N, "skud_lcd_brdg_reset_n");
	if (rc < 0) {
		pr_err("failed to request skud lcd brdg reset_n\n");
		return rc;
	}

	return rc;
}

#define GPIO_SKUE_LCD_BRDG_RESET_N	78

static unsigned skue_mipi_dsi_gpio[] = {
	GPIO_CFG(GPIO_SKUE_LCD_BRDG_RESET_N, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_NO_PULL,
			GPIO_CFG_2MA), /* GPIO_SKUE_LCD_BRDG_RESET_N */
};

static int msm_fb_dsi_client_skue_reset(void)
{
	int rc = 0;

	pr_debug("%s\n", __func__);

	rc = gpio_request(GPIO_SKUE_LCD_BRDG_RESET_N, "skue_lcd_brdg_reset_n");
	if (rc < 0) {
		pr_err("failed to request skue lcd brdg reset_n\n");
		return rc;
	}

	return rc;
}


static int msm_fb_dsi_client_reset(void)
{
	int rc = 0;

	if (machine_is_msm7627a_qrd1())
		rc = msm_fb_dsi_client_qrd1_reset();
	else if (machine_is_msm7627a_evb() || machine_is_msm8625_evb()
                        || machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a())
		rc = msm_fb_dsi_client_qrd3_reset();
        else if (machine_is_msm8625q_skud() || machine_is_msm8625q_evbd())
                rc = msm_fb_dsi_client_skud_reset();
        else if (machine_is_msm8625q_skue())
                rc = msm_fb_dsi_client_skue_reset();
	else
		rc = msm_fb_dsi_client_msm_reset();

	return rc;

}

static struct regulator_bulk_data regs_dsi[] = {
	{ .supply = "gp2",   .min_uV = 2850000, .max_uV = 2850000 },
	{ .supply = "msme1", .min_uV = 1800000, .max_uV = 1800000 },
};

static int dsi_gpio_initialized = 0;

static int mipi_dsi_panel_msm_power(int on)
{
	int rc = 0;
	uint32_t lcdc_reset_cfg;

	/* I2C-controlled GPIO Expander -init of the GPIOs very late */
	if (unlikely(!dsi_gpio_initialized)) {
		pmapp_disp_backlight_init();

		rc = gpio_request(GPIO_DISPLAY_PWR_EN, "gpio_disp_pwr");
		if (rc < 0) {
			pr_err("failed to request gpio_disp_pwr\n");
			return rc;
		}

		if (machine_is_msm7x27a_surf() || machine_is_msm7625a_surf()
				|| machine_is_msm8625_surf()) {
			rc = gpio_direction_output(GPIO_DISPLAY_PWR_EN, 1);
			if (rc < 0) {
				pr_err("failed to enable display pwr\n");
				goto fail_gpio1;
			}

			rc = gpio_request(GPIO_BACKLIGHT_EN, "gpio_bkl_en");
			if (rc < 0) {
				pr_err("failed to request gpio_bkl_en\n");
				goto fail_gpio1;
			}

			rc = gpio_direction_output(GPIO_BACKLIGHT_EN, 1);
			if (rc < 0) {
				pr_err("failed to enable backlight\n");
				goto fail_gpio2;
			}
		}

		rc = regulator_bulk_get(NULL, ARRAY_SIZE(regs_dsi), regs_dsi);
		if (rc) {
			pr_err("%s: could not get regulators: %d\n",
					__func__, rc);
			goto fail_gpio2;
		}

		rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_dsi),
						regs_dsi);
		if (rc) {
			pr_err("%s: could not set voltages: %d\n",
					__func__, rc);
			goto fail_vreg;
		}
		if (pmapp_disp_backlight_set_brightness(100))
			pr_err("backlight set brightness failed\n");

		dsi_gpio_initialized = 1;
	}
	if (machine_is_msm7x27a_surf() || machine_is_msm7625a_surf() ||
			machine_is_msm8625_surf()) {
		gpio_set_value_cansleep(GPIO_DISPLAY_PWR_EN, on);
		gpio_set_value_cansleep(GPIO_BACKLIGHT_EN, on);
	} else if (machine_is_msm7x27a_ffa() || machine_is_msm7625a_ffa()
					|| machine_is_msm8625_ffa()) {
		if (on) {
			/* This line drives an active low pin on FFA */
			rc = gpio_direction_output(GPIO_DISPLAY_PWR_EN, !on);
			if (rc < 0)
				pr_err("failed to set direction for "
					"display pwr\n");
		} else {
			gpio_set_value_cansleep(GPIO_DISPLAY_PWR_EN, !on);
			rc = gpio_direction_input(GPIO_DISPLAY_PWR_EN);
			if (rc < 0)
				pr_err("failed to set direction for "
					"display pwr\n");
		}
	}

	if (on) {
		gpio_set_value_cansleep(GPIO_LCDC_BRDG_PD, 0);

		if (machine_is_msm7x27a_surf() ||
				 machine_is_msm7625a_surf() ||
				 machine_is_msm8625_surf()) {
			lcdc_reset_cfg = readl_relaxed(lcdc_reset_ptr);
			rmb();
			lcdc_reset_cfg &= ~1;
			writel_relaxed(lcdc_reset_cfg, lcdc_reset_ptr);
			msleep(20);
			wmb();
			lcdc_reset_cfg |= 1;
			writel_relaxed(lcdc_reset_cfg, lcdc_reset_ptr);
			msleep(20);
		} else {
			gpio_set_value_cansleep(GPIO_LCDC_BRDG_RESET_N, 0);
			msleep(20);
			gpio_set_value_cansleep(GPIO_LCDC_BRDG_RESET_N, 1);
			msleep(20);
		}
	} else {
		gpio_set_value_cansleep(GPIO_LCDC_BRDG_PD, 1);
	}

	rc = on ? regulator_bulk_enable(ARRAY_SIZE(regs_dsi), regs_dsi) :
		  regulator_bulk_disable(ARRAY_SIZE(regs_dsi), regs_dsi);

	if (rc)
		pr_err("%s: could not %sable regulators: %d\n",
				__func__, on ? "en" : "dis", rc);

	return rc;
fail_vreg:
	regulator_bulk_free(ARRAY_SIZE(regs_dsi), regs_dsi);
fail_gpio2:
	gpio_free(GPIO_BACKLIGHT_EN);
fail_gpio1:
	gpio_free(GPIO_DISPLAY_PWR_EN);
	dsi_gpio_initialized = 0;
	return rc;
}

static int mipi_dsi_panel_qrd1_power(int on)
{
	int rc = 0;

	if (!dsi_gpio_initialized) {
		rc = gpio_request(QRD_GPIO_BACKLIGHT_EN, "gpio_bkl_en");
		if (rc < 0)
			return rc;

		rc = gpio_tlmm_config(GPIO_CFG(QRD_GPIO_BACKLIGHT_EN, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		if (rc < 0) {
			pr_err("failed GPIO_BACKLIGHT_EN tlmm config\n");
			return rc;
		}

		rc = gpio_direction_output(QRD_GPIO_BACKLIGHT_EN, 1);
		if (rc < 0) {
			pr_err("failed to enable backlight\n");
			gpio_free(QRD_GPIO_BACKLIGHT_EN);
			return rc;
		}
		dsi_gpio_initialized = 1;
	}

	gpio_set_value_cansleep(QRD_GPIO_BACKLIGHT_EN, !!on);

	if (on) {
		gpio_set_value_cansleep(GPIO_LCDC_BRDG_RESET_N, 1);
		msleep(20);
		gpio_set_value_cansleep(GPIO_LCDC_BRDG_RESET_N, 0);
		msleep(20);
		gpio_set_value_cansleep(GPIO_LCDC_BRDG_RESET_N, 1);

	}

	return rc;
}

static int qrd3_dsi_gpio_initialized;
static struct regulator *gpio_reg_2p85v, *gpio_reg_1p8v;

static int mipi_dsi_panel_qrd3_power(int on)
{
	int rc = 0;

	if (!qrd3_dsi_gpio_initialized) {
		pmapp_disp_backlight_init();
		rc = gpio_request(GPIO_QRD3_LCD_BACKLIGHT_EN,
			"qrd3_gpio_bkl_en");
		if (rc < 0)
			return rc;

		qrd3_dsi_gpio_initialized = 1;

		if (mdp_pdata.cont_splash_enabled) {
			rc = gpio_tlmm_config(GPIO_CFG(
			     GPIO_QRD3_LCD_BACKLIGHT_EN, 0, GPIO_CFG_OUTPUT,
			     GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
			if (rc < 0) {
				pr_err("failed QRD3 GPIO_BACKLIGHT_EN tlmm config\n");
				return rc;
			}
			rc = gpio_direction_output(GPIO_QRD3_LCD_BACKLIGHT_EN,
			     1);
			if (rc < 0) {
				pr_err("failed to enable backlight\n");
				gpio_free(GPIO_QRD3_LCD_BACKLIGHT_EN);
				return rc;
			}

			/*Configure LCD Bridge reset*/
			rc = gpio_tlmm_config(qrd3_mipi_dsi_gpio[0],
			     GPIO_CFG_ENABLE);
			if (rc < 0) {
				pr_err("Failed to enable LCD Bridge reset enable\n");
				return rc;
			}

			rc = gpio_direction_output(GPIO_QRD3_LCD_BRDG_RESET_N,
			     1);

			if (rc < 0) {
				pr_err("Failed GPIO bridge Reset\n");
				gpio_free(GPIO_QRD3_LCD_BRDG_RESET_N);
				return rc;
			}
			return 0;
		}
	}

	if (on) {
		rc = gpio_tlmm_config(GPIO_CFG(GPIO_QRD3_LCD_BACKLIGHT_EN, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
		if (rc < 0) {
			pr_err("failed QRD3 GPIO_BACKLIGHT_EN tlmm config\n");
			return rc;
		}
		rc = gpio_direction_output(GPIO_QRD3_LCD_BACKLIGHT_EN, 1);
		if (rc < 0) {
			pr_err("failed to enable backlight\n");
			gpio_free(GPIO_QRD3_LCD_BACKLIGHT_EN);
			return rc;
		}
		/*Toggle Backlight GPIO*/
		gpio_set_value_cansleep(GPIO_QRD3_LCD_BACKLIGHT_EN, 1);
		udelay(100);
		gpio_set_value_cansleep(GPIO_QRD3_LCD_BACKLIGHT_EN, 0);
		udelay(430);
		gpio_set_value_cansleep(GPIO_QRD3_LCD_BACKLIGHT_EN, 1);
		/* 1 wire mode starts from this low to high transition */
		udelay(50);

		/*Enable EXT_2.85 and 1.8 regulators*/
		rc = regulator_enable(gpio_reg_2p85v);
		if (rc < 0)
			pr_err("%s: reg enable failed\n", __func__);
		rc = regulator_enable(gpio_reg_1p8v);
		if (rc < 0)
			pr_err("%s: reg enable failed\n", __func__);

		/*Configure LCD Bridge reset*/
		rc = gpio_tlmm_config(qrd3_mipi_dsi_gpio[0], GPIO_CFG_ENABLE);
		if (rc < 0) {
			pr_err("Failed to enable LCD Bridge reset enable\n");
			return rc;
		}

		rc = gpio_direction_output(GPIO_QRD3_LCD_BRDG_RESET_N, 1);

		if (rc < 0) {
			pr_err("Failed GPIO bridge Reset\n");
			gpio_free(GPIO_QRD3_LCD_BRDG_RESET_N);
			return rc;
		}

		/*Toggle Bridge Reset GPIO*/
		msleep(20);
		gpio_set_value_cansleep(GPIO_QRD3_LCD_BRDG_RESET_N, 0);
		msleep(20);
		gpio_set_value_cansleep(GPIO_QRD3_LCD_BRDG_RESET_N, 1);
		msleep(20);

	} else {
                /* FIXME: GPIO_QRD3_LCD_BACKLIGHT_EN can't be set as
                 * input by below statement, so just keep it output low
                 */
                gpio_set_value_cansleep(GPIO_QRD3_LCD_BACKLIGHT_EN, 0);
#if 0
                gpio_tlmm_config(GPIO_CFG(GPIO_QRD3_LCD_BACKLIGHT_EN, 0,
			GPIO_CFG_INPUT, GPIO_CFG_PULL_DOWN, GPIO_CFG_2MA),
			GPIO_CFG_DISABLE);
#endif
		gpio_tlmm_config(GPIO_CFG(GPIO_QRD3_LCD_BRDG_RESET_N, 0,
			GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			GPIO_CFG_DISABLE);

		rc = regulator_disable(gpio_reg_2p85v);
		if (rc < 0)
			pr_err("%s: reg disable failed\n", __func__);
		rc = regulator_disable(gpio_reg_1p8v);
		if (rc < 0)
			pr_err("%s: reg disable failed\n", __func__);

	}

	return rc;
}

static int skud_dsi_gpio_initialized = 0;
static int mipi_dsi_panel_skud_power(int on)
{
	int rc = 0;
	if (!skud_dsi_gpio_initialized) {
		pmapp_disp_backlight_init();

		skud_dsi_gpio_initialized = 1;

		if (mdp_pdata.cont_splash_enabled) {
			/*Configure LCD Bridge reset*/
			rc = gpio_tlmm_config(skud_mipi_dsi_gpio[0],
			     GPIO_CFG_ENABLE);
			if (rc < 0) {
				pr_err("Failed to enable LCD Bridge reset enable\n");
				return rc;
			}

			rc = gpio_direction_output(GPIO_SKUD_LCD_BRDG_RESET_N, 1);

			if (rc < 0) {
				pr_err("Failed GPIO bridge Reset\n");
				gpio_free(GPIO_SKUD_LCD_BRDG_RESET_N);
				return rc;
			}
			return 0;
		}
	}

	if (on) {
		/*Configure LCD Bridge reset*/
		rc = gpio_tlmm_config(skud_mipi_dsi_gpio[0], GPIO_CFG_ENABLE);
		if (rc < 0) {
			pr_err("Failed to enable LCD Bridge reset enable\n");
			return rc;
		}

		rc = gpio_direction_output(GPIO_SKUD_LCD_BRDG_RESET_N, 1);

		if (rc < 0) {
			pr_err("Failed GPIO bridge Reset\n");
			gpio_free(GPIO_SKUD_LCD_BRDG_RESET_N);
			return rc;
		}
//xiangdong modify for 5937 LCD issue
#ifdef TYQ_TRULY_SLEEP_IN_DISPLAY_OFF
		if(tyq_get_lcd_id() == 1){
			//truly lcd
		/*Toggle Bridge Reset GPIO*/
		msleep(30);
		gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 0);
		msleep(50);
		gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 1);
		msleep(30);

		gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 0);
		msleep(50);
		gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 1);
		msleep(30);
		}else{			
			/*Toggle Bridge Reset GPIO*/
			msleep(20);
			gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 0);
			msleep(5);
			gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 1);
			msleep(10);
		}
#else
	#ifndef TYQ_HX8392_MIPI_HD_SUPPORT 
		/*Toggle Bridge Reset GPIO*/
		msleep(20);
		gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 0);
		msleep(5);
		gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 1);
		msleep(10);
	#endif
#endif

	} else {

   /*tydrv qupw add for truly nt35516 lcd */
   #ifdef TYQ_TRULY_SLEEP_IN_RESET_HIGH
	  if (tyq_get_lcd_id()!=1)  // if truly do nothing keep reset high
		gpio_tlmm_config(GPIO_CFG(GPIO_SKUD_LCD_BRDG_RESET_N, 0,
			GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			GPIO_CFG_DISABLE);
   #else
	//xiangdong modify for 5937 LCD issue
	#ifdef TYQ_TRULY_SLEEP_IN_DISPLAY_OFF				
	if(tyq_get_lcd_id() == 1){
			//truly lcd
		gpio_tlmm_config(GPIO_CFG(GPIO_SKUD_LCD_BRDG_RESET_N, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);		
		msleep(10);
		gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 1);
		}else{			
			gpio_tlmm_config(GPIO_CFG(GPIO_SKUD_LCD_BRDG_RESET_N, 0,
			GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			GPIO_CFG_DISABLE);
		}
	#else
	#ifndef TYQ_HX8392_MIPI_HD_SUPPORT
       gpio_tlmm_config(GPIO_CFG(GPIO_SKUD_LCD_BRDG_RESET_N, 0,
       	GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
       	GPIO_CFG_DISABLE);
     #endif
	#endif
   #endif
	}

	return rc;
}

static int skue_dsi_gpio_initialized = 0;
static int mipi_dsi_panel_skue_power(int on)
{
	int rc = 0;

        pr_debug("%s: on = %d\n", __func__, on);

	if (!skue_dsi_gpio_initialized) {
		pmapp_disp_backlight_init();

		skue_dsi_gpio_initialized = 1;

		if (mdp_pdata.cont_splash_enabled) {
			/*Configure LCD Bridge reset*/
			rc = gpio_tlmm_config(skue_mipi_dsi_gpio[0],
			     GPIO_CFG_ENABLE);
			if (rc < 0) {
				pr_err("Failed to enable LCD Bridge reset enable\n");
				return rc;
			}

			rc = gpio_direction_output(GPIO_SKUE_LCD_BRDG_RESET_N, 1);

			if (rc < 0) {
				pr_err("Failed GPIO bridge Reset\n");
				gpio_free(GPIO_SKUE_LCD_BRDG_RESET_N);
				return rc;
			}
			return 0;
		}
	}

	if (on) {
		/*Configure LCD Bridge reset*/
		rc = gpio_tlmm_config(skue_mipi_dsi_gpio[0], GPIO_CFG_ENABLE);
		if (rc < 0) {
			pr_err("Failed to enable LCD Bridge reset enable\n");
			return rc;
		}

		rc = gpio_direction_output(GPIO_SKUE_LCD_BRDG_RESET_N, 1);

		if (rc < 0) {
			pr_err("Failed GPIO bridge Reset\n");
			gpio_free(GPIO_SKUE_LCD_BRDG_RESET_N);
			return rc;
		}

		/*Toggle Bridge Reset GPIO*/
		msleep(20);
		gpio_set_value_cansleep(GPIO_SKUE_LCD_BRDG_RESET_N, 0);
		msleep(20);
		gpio_set_value_cansleep(GPIO_SKUE_LCD_BRDG_RESET_N, 1);
		msleep(20);

	} else {
			/*Pull down Reset GPIO*/
			gpio_tlmm_config(GPIO_CFG(GPIO_SKUE_LCD_BRDG_RESET_N, 0,
						 GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
						 GPIO_CFG_DISABLE);
	}

	return rc;
}

static int evbd_dsi_gpio_initialized;

static int mipi_dsi_panel_evbd_power(int on)
{
        int rc = 0;

        if (!evbd_dsi_gpio_initialized) {
                pmapp_disp_backlight_init();

                evbd_dsi_gpio_initialized = 1;

                if (mdp_pdata.cont_splash_enabled) {
                        /*Configure LCD Bridge reset*/
                        rc = gpio_tlmm_config(skud_mipi_dsi_gpio[0],
                                        GPIO_CFG_ENABLE);
                        if (rc < 0) {
                                pr_err("Failed to enable LCD Bridge reset enable\n");
                                return rc;
                        }

                        rc = gpio_direction_output(GPIO_SKUD_LCD_BRDG_RESET_N,
                                        1);

                        if (rc < 0) {
                                pr_err("Failed GPIO bridge Reset\n");
                                gpio_free(GPIO_SKUD_LCD_BRDG_RESET_N);
                                return rc;
                        }
                        return 0;
                }
        }

        if (on) {
                /*Enable EXT_2.85 and 1.8 regulators*/
                rc = regulator_enable(gpio_reg_2p85v);
                if (rc < 0)
                        pr_err("%s: reg enable failed\n", __func__);
                rc = regulator_enable(gpio_reg_1p8v);
                if (rc < 0)
                        pr_err("%s: reg enable failed\n", __func__);

                /*Configure LCD Bridge reset*/
                rc = gpio_tlmm_config(skud_mipi_dsi_gpio[0], GPIO_CFG_ENABLE);
                if (rc < 0) {
                        pr_err("Failed to enable LCD Bridge reset enable\n");
                        return rc;
                }

                rc = gpio_direction_output(GPIO_SKUD_LCD_BRDG_RESET_N, 1);

                if (rc < 0) {
                        pr_err("Failed GPIO bridge Reset\n");
                        gpio_free(GPIO_SKUD_LCD_BRDG_RESET_N);
                        return rc;
                }

                /*Toggle Bridge Reset GPIO*/
                msleep(20);
                gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 0);
                msleep(20);
                gpio_set_value_cansleep(GPIO_SKUD_LCD_BRDG_RESET_N, 1);
                msleep(20);

        } else {
                gpio_tlmm_config(GPIO_CFG(GPIO_SKUD_LCD_BRDG_RESET_N, 0,
                                        GPIO_CFG_INPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
                                GPIO_CFG_DISABLE);

                rc = regulator_disable(gpio_reg_2p85v);
                if (rc < 0)
                        pr_err("%s: reg disable failed\n", __func__);
                rc = regulator_disable(gpio_reg_1p8v);
                if (rc < 0)
                        pr_err("%s: reg disable failed\n", __func__);

        }

        return rc;
}

static char mipi_dsi_splash_is_enabled(void);
static int mipi_dsi_panel_power(int on)
{
	int rc = 0;

	if (machine_is_msm7627a_qrd1())
		rc = mipi_dsi_panel_qrd1_power(on);
	else if (machine_is_msm7627a_evb() || machine_is_msm8625_evb()
                        || machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a())
		rc = mipi_dsi_panel_qrd3_power(on);
        else if (machine_is_msm8625q_skud())
                rc = mipi_dsi_panel_skud_power(on);
        else if (machine_is_msm8625q_evbd())
                rc = mipi_dsi_panel_evbd_power(on);
        else if (machine_is_msm8625q_skue())
                rc = mipi_dsi_panel_skue_power(on);
	else
		rc = mipi_dsi_panel_msm_power(on);
	return rc;
}

#define MDP_303_VSYNC_GPIO 97

static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.vsync_gpio		= MDP_303_VSYNC_GPIO,
	.dsi_power_save		= mipi_dsi_panel_power,
	.dsi_client_reset   = msm_fb_dsi_client_reset,
	.get_lane_config	= msm_fb_get_lane_config,
	.splash_is_enabled	= mipi_dsi_splash_is_enabled,
	.dlane_swap		= 0x1,
};

static char mipi_dsi_splash_is_enabled(void)
{
	return mdp_pdata.cont_splash_enabled;
}

static char prim_panel_name[PANEL_NAME_MAX_LEN];
static int __init prim_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(prim_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("prim_display", prim_display_setup);

static int disable_splash;

void msm7x27a_set_display_params(char *prim_panel)
{
	if (strnlen(prim_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.prim_panel_name, prim_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.prim_panel_name %s\n",
			msm_fb_pdata.prim_panel_name);
	}
	if (strnlen(msm_fb_pdata.prim_panel_name, PANEL_NAME_MAX_LEN)) {
		if (strncmp((char *)msm_fb_pdata.prim_panel_name,
			"mipi_cmd_nt35510_wvga",
			strnlen("mipi_cmd_nt35510_wvga",
				PANEL_NAME_MAX_LEN)) &&
		    strncmp((char *)msm_fb_pdata.prim_panel_name,
			"mipi_video_nt35510_wvga",
			strnlen("mipi_video_nt35510_wvga",
				PANEL_NAME_MAX_LEN)))
			disable_splash = 1;
	}

        if (machine_is_msm8625q_skue())
                disable_splash = 1;
}

void __init msm_fb_add_devices(void)
{
	int rc = 0;
  #if defined(TYQ_LCD_SUPPORT)// tydrv qupw  add for truly NT35510 lcd
	lcd_display_init();
	#endif
        msm7x27a_set_display_params(prim_panel_name);

	if (machine_is_msm7627a_qrd1())
		platform_add_devices(qrd_fb_devices,
				ARRAY_SIZE(qrd_fb_devices));
	else if (machine_is_msm7627a_evb() || machine_is_msm8625_evb()
                        || machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a()) {
		mipi_NT35510_pdata.bl_lock = 1;
		mipi_NT35516_pdata.bl_lock = 1;
		if (disable_splash)
			mdp_pdata.cont_splash_enabled = 0x0;

		platform_add_devices(evb_fb_devices,
				ARRAY_SIZE(evb_fb_devices));
	} else if (machine_is_msm7627a_qrd3() || machine_is_msm8625_qrd7()) {
		sku3_lcdc_lcd_camera_power_init();
		mdp_pdata.cont_splash_enabled = 0x0;
		platform_add_devices(qrd3_fb_devices,
						ARRAY_SIZE(qrd3_fb_devices));
	} else if (machine_is_msm8625q_skud() || machine_is_msm8625q_evbd()) {
		if (disable_splash)
			mdp_pdata.cont_splash_enabled = 0x0;

                /* SKUD and SKUD' use different lane connection */
#if 1
                if (cpu_is_msm8625q())
#if defined(TYQ_NT35590_MIPI_HD_SUPPORT)||defined(TYQ_HX8392_MIPI_HD_SUPPORT)
                        mipi_dsi_pdata.dlane_swap = 3;//dlane_swap :1 2 3 0
#else
                        mipi_dsi_pdata.dlane_swap = 0;
#endif
#else
                if (cpu_is_msm8625q()){
					if(!strncmp((char *)prim_panel_name, "mipi_video_hx8392a_720p", 23))
						mipi_dsi_pdata.dlane_swap = 3;
					else
						mipi_dsi_pdata.dlane_swap = 0;

					pr_info("[DISP] The prim panel name is %s, Swap lane is %d\n", prim_panel_name,mipi_dsi_pdata.dlane_swap);
				}
#endif
                platform_add_devices(skud_fb_devices,
				ARRAY_SIZE(skud_fb_devices));
        } else if (machine_is_msm8625q_skue()) {
		if (disable_splash)
			mdp_pdata.cont_splash_enabled = 0x0;

                /* SKUE and SKUE' use different lane connection */
                if (cpu_is_msm8625q())
                        mipi_dsi_pdata.dlane_swap = 0;

		platform_add_devices(skue_fb_devices,
				ARRAY_SIZE(skue_fb_devices));
        } else {
		mdp_pdata.cont_splash_enabled = 0x0;
		platform_add_devices(msm_fb_devices,
				ARRAY_SIZE(msm_fb_devices));
	}

	msm_fb_register_device("mdp", &mdp_pdata);
	#if defined(TYQ_LCD_SUPPORT)
	msm_fb_register_device("lcdc", &lcdc_pdata);
	#else
	if (machine_is_msm7625a_surf() || machine_is_msm7x27a_surf() ||
			machine_is_msm8625_surf() || machine_is_msm7627a_qrd3()
			|| machine_is_msm8625_qrd7())
		msm_fb_register_device("lcdc", &lcdc_pdata);
	#endif
#ifdef CONFIG_FB_MSM_MIPI_DSI
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
#endif
	if (machine_is_msm7627a_evb() || machine_is_msm8625_evb()
                        || machine_is_msm8625_qrd5() || machine_is_msm7x27a_qrd5a()
                        || machine_is_msm8625q_evbd()) {
		gpio_reg_2p85v = regulator_get(&mipi_dsi_device.dev,
								"lcd_vdd");
		if (IS_ERR(gpio_reg_2p85v))
			pr_err("%s:ext_2p85v regulator get failed", __func__);

		gpio_reg_1p8v = regulator_get(&mipi_dsi_device.dev,
								"lcd_vddi");
		if (IS_ERR(gpio_reg_1p8v))
			pr_err("%s:ext_1p8v regulator get failed", __func__);

		if (mdp_pdata.cont_splash_enabled) {
			/*Enable EXT_2.85 and 1.8 regulators*/
			rc = regulator_enable(gpio_reg_2p85v);
			if (rc < 0)
				pr_err("%s: reg enable failed\n", __func__);
			rc = regulator_enable(gpio_reg_1p8v);
			if (rc < 0)
				pr_err("%s: reg enable failed\n", __func__);
		}
	}
}
