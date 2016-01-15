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
#if defined(TYQ_NT35590_MIPI_HD_SUPPORT)
#include <mach/socinfo.h>
#include "msm_fb.h"
#include "mipi_dsi.h"
#include "TY_mipi_truly_nt35590.h"
#include <mach/gpio.h>
#include <linux/gpio.h>
#include <mach/pmic.h>
/*tydrv niuli add for keypad light misc control on 20111117*/
#include <linux/miscdevice.h>


static struct msm_panel_common_pdata *mipi_nt35590_pdata;
static struct dsi_buf nt35590_tx_buf;
static struct dsi_buf nt35590_rx_buf;


#define NT35590_CMD_DELAY		0
#define NT35590_SLEEP_OFF_DELAY	150
#define NT35590_DISPLAY_ON_DELAY	20

static int mipi_nt35590_bl_ctrl = 0;

/*tydrv niuli add for keypad light misc control on 20111117  begin*/
#define DRV_READ_LCDID_MISC_IOCTL 1
/*tydrv niuli add ffor keypad light misc control on 20111117  end*/

static	char	reg_1[2]	=	{0xFF,0xEE};								
static	char	reg_2[2]	=	{0x26,0x08};								
static	char	reg_3[2]	=	{0x26,0x00};								
static	char	reg_4[2]	=	{0xFF,0x00};								
static	char	reg_5[2]	=	{0xBA,0x03};//MIPI	4	lanes 00 1  01 2  02 3 03 4						
static	char	reg_6[2]	=	{0xC2,0x0b};//Setting	0x08	for	MIPI	command	mode 03 for video mode			0b video ram
static	char	reg_7[2]	=	{0xFF,0x01};//CMD	page	select						
static	char	reg_8[2]	=	{0xFB,0x01};//RELOAD	CMD1							
static	char	reg_9[2]	=	{0x00,0x4A};								
static	char	reg_10[2]	=	{0x01,0x33};								
static	char	reg_11[2]	=	{0x02,0x53};								
static	char	reg_12[2]	=	{0x03,0x55};								
static	char	reg_13[2]	=	{0x04,0x55};								
static	char	reg_14[2]	=	{0x05,0x33};								
static	char	reg_15[2]	=	{0x06,0x22};								
static	char	reg_16[2]	=	{0x08,0x56};								
static	char	reg_17[2]	=	{0x09,0x8F};								
static	char	reg_18[2]	=	{0x36,0x73};								
static	char	reg_19[2]	=	{0x0B,0x9F};								
static	char	reg_20[2]	=	{0x0C,0x9F};								
static	char	reg_21[2]	=	{0x0D,0x2F};								
static	char	reg_22[2]	=	{0x0E,0x24};								
static	char	reg_23[2]	=	{0x11,0x83};								
static	char	reg_24[2]	=	{0x12,0x03};								
static	char	reg_25[2]	=	{0x71,0x2C};								
static	char	reg_26[2]	=	{0x6F,0x03};								
static	char	reg_27[2]	=	{0x0F,0x0A};								
												
static	char	reg_28[2]	=	{0xFF,0x05};//CMD	page	select						
static	char	reg_29[2]	=	{0xFB,0x01};//RELOAD	CMD1							
static	char	reg_30[2]	=	{0x01,0x00};								
static	char	reg_31[2]	=	{0x02,0x82};								
static	char	reg_32[2]	=	{0x03,0x82};								
static	char	reg_33[2]	=	{0x04,0x82};								
static	char	reg_34[2]	=	{0x05,0x30};								
static	char	reg_35[2]	=	{0x06,0x33};								
static	char	reg_36[2]	=	{0x07,0x01};								
static	char	reg_37[2]	=	{0x08,0x00};								
static	char	reg_38[2]	=	{0x09,0x46};								
static	char	reg_39[2]	=	{0x0A,0x46};								
static	char	reg_40[2]	=	{0x0D,0x0B};								
static	char	reg_41[2]	=	{0x0E,0x1D};								
static	char	reg_42[2]	=	{0x0F,0x08};								
static	char	reg_43[2]	=	{0x10,0x53};								
static	char	reg_44[2]	=	{0x11,0x00};								
static	char	reg_45[2]	=	{0x12,0x00};								
static	char	reg_46[2]	=	{0x14,0x01};								
static	char	reg_47[2]	=	{0x15,0x00};								
static	char	reg_48[2]	=	{0x16,0x05};								
static	char	reg_49[2]	=	{0x17,0x00};								
static	char	reg_50[2]	=	{0x19,0x7F};								
static	char	reg_51[2]	=	{0x1A,0xFF};								
static	char	reg_52[2]	=	{0x1B,0x0F};								
static	char	reg_53[2]	=	{0x1C,0x00};								
static	char	reg_54[2]	=	{0x1D,0x00};								
static	char	reg_55[2]	=	{0x1E,0x00};								
static	char	reg_56[2]	=	{0x1F,0x07};								
static	char	reg_57[2]	=	{0x20,0x00};								
static	char	reg_58[2]	=	{0x21,0x00};								
static	char	reg_59[2]	=	{0x22,0x55};								
static	char	reg_60[2]	=	{0x23,0x4D};								
static	char	reg_61[2]	=	{0x2D,0x02};								
static	char	reg_62[2]	=	{0x28,0x01};								
static	char	reg_63[2]	=	{0x2F,0x02};								
static	char	reg_64[2]	=	{0x83,0x01};								
static	char	reg_65[2]	=	{0x9E,0x58};								
static	char	reg_66[2]	=	{0x9F,0x6A};								
static	char	reg_67[2]	=	{0xA0,0x01};								
static	char	reg_68[2]	=	{0xA2,0x10};								
static	char	reg_69[2]	=	{0xBB,0x0A};								
static	char	reg_70[2]	=	{0xBC,0x0A};								
static	char	reg_71[2]	=	{0x32,0x08};								
static	char	reg_72[2]	=	{0x33,0xB8};								
static	char	reg_73[2]	=	{0x36,0x01};								
static	char	reg_74[2]	=	{0x37,0x00};								
static	char	reg_75[2]	=	{0x43,0x00};								
static	char	reg_76[2]	=	{0x4B,0x21};								
static	char	reg_77[2]	=	{0x4C,0x03};								
static	char	reg_78[2]	=	{0x50,0x21};								
static	char	reg_79[2]	=	{0x51,0x03};								
static	char	reg_80[2]	=	{0x58,0x21};								
static	char	reg_81[2]	=	{0x59,0x03};								
static	char	reg_82[2]	=	{0x5D,0x21};								
static	char	reg_83[2]	=	{0x5E,0x03};								
static	char	reg_84[2]	=	{0x6C,0x00};								
static	char	reg_85[2]	=	{0x6D,0x00};								
												
//for	gamma	2.2	setting									
static	char	reg_86[2]	=	{0xFB,0x01};								
static	char	reg_87[2]	=	{0xFF,0x01};//Enter	CMD2,Page0							
static	char	reg_88[2]	=	{0xFB,0x01};								
static	char	reg_89[2]	=	{0x75,0x00};//Gamma+	R							
static	char	reg_90[2]	=	{0x76,0x7D};//Gamma+	R							
static	char	reg_91[2]	=	{0x77,0x00};//Gamma+	R							
static	char	reg_92[2]	=	{0x78,0x8A};//Gamma+	R							
static	char	reg_93[2]	=	{0x79,0x00};//Gamma+	R							
static	char	reg_94[2]	=	{0x7A,0x9C};//Gamma+	R							
static	char	reg_95[2]	=	{0x7B,0x00};//Gamma+	R							
static	char	reg_96[2]	=	{0x7C,0xB1};//Gamma+	R							
static	char	reg_97[2]	=	{0x7D,0x00};//Gamma+	R							
static	char	reg_98[2]	=	{0x7E,0xBF};//Gamma+	R							
static	char	reg_99[2]	=	{0x7F,0x00};//Gamma+	R							
static	char	reg_100[2]	=	{0x80,0xCF};//Gamma+	R							
static	char	reg_101[2]	=	{0x81,0x00};//Gamma+	R							
static	char	reg_102[2]	=	{0x82,0xDD};//Gamma+	R							
static	char	reg_103[2]	=	{0x83,0x00};//Gamma+	R							
static	char	reg_104[2]	=	{0x84,0xE8};//Gamma+	R							
static	char	reg_105[2]	=	{0x85,0x00};//Gamma+	R							
static	char	reg_106[2]	=	{0x86,0xF2};//Gamma+	R							
static	char	reg_107[2]	=	{0x87,0x01};//Gamma+	R							
static	char	reg_108[2]	=	{0x88,0x1F};//Gamma+	R							
static	char	reg_109[2]	=	{0x89,0x01};//Gamma+	R							
static	char	reg_110[2]	=	{0x8A,0x41};//Gamma+	R							
static	char	reg_111[2]	=	{0x8B,0x01};//Gamma+	R							
static	char	reg_112[2]	=	{0x8C,0x78};//Gamma+	R							
static	char	reg_113[2]	=	{0x8D,0x01};//Gamma+	R							
static	char	reg_114[2]	=	{0x8E,0xA5};//Gamma+	R							
static	char	reg_115[2]	=	{0x8F,0x01};//Gamma+	R							
static	char	reg_116[2]	=	{0x90,0xEE};//Gamma+	R							
static	char	reg_117[2]	=	{0x91,0x02};//Gamma+	R							
static	char	reg_118[2]	=	{0x92,0x29};//Gamma+	R							
static	char	reg_119[2]	=	{0x93,0x02};//Gamma+	R							
static	char	reg_120[2]	=	{0x94,0x2A};//Gamma+	R							
static	char	reg_121[2]	=	{0x95,0x02};//Gamma+	R							
static	char	reg_122[2]	=	{0x96,0x5D};//Gamma+	R							
static	char	reg_123[2]	=	{0x97,0x02};//Gamma+	R							
static	char	reg_124[2]	=	{0x98,0x93};//Gamma+	R							
static	char	reg_125[2]	=	{0x99,0x02};//Gamma+	R							
static	char	reg_126[2]	=	{0x9A,0xB8};//Gamma+	R							
static	char	reg_127[2]	=	{0x9B,0x02};//Gamma+	R							
static	char	reg_128[2]	=	{0x9C,0xE7};//Gamma+	R							
static	char	reg_129[2]	=	{0x9D,0x03};//Gamma+	R							
static	char	reg_130[2]	=	{0x9E,0x07};//Gamma+	R							
static	char	reg_131[2]	=	{0x9F,0x03};//Gamma+	R							
static	char	reg_132[2]	=	{0xA0,0x37};//Gamma+	R							
static	char	reg_133[2]	=	{0xA2,0x03};//Gamma+	R							
static	char	reg_134[2]	=	{0xA3,0x46};//Gamma+	R							
static	char	reg_135[2]	=	{0xA4,0x03};//Gamma+	R							
static	char	reg_136[2]	=	{0xA5,0x56};//Gamma+	R							
static	char	reg_137[2]	=	{0xA6,0x03};//Gamma+	R							
static	char	reg_138[2]	=	{0xA7,0x66};//Gamma+	R							
static	char	reg_139[2]	=	{0xA9,0x03};//Gamma+	R							
static	char	reg_140[2]	=	{0xAA,0x7A};//Gamma+	R							
static	char	reg_141[2]	=	{0xAB,0x03};//Gamma+	R							
static	char	reg_142[2]	=	{0xAC,0x93};//Gamma+	R							
static	char	reg_143[2]	=	{0xAD,0x03};//Gamma+	R							
static	char	reg_144[2]	=	{0xAE,0xA3};//Gamma+	R							
static	char	reg_145[2]	=	{0xAF,0x03};//Gamma+	R							
static	char	reg_146[2]	=	{0xB0,0xB4};//Gamma+	R							
static	char	reg_147[2]	=	{0xB1,0x03};//Gamma+	R							
static	char	reg_148[2]	=	{0xB2,0xCB};//Gamma+	R							
												
//Setting	for	Gamma-	R									
static	char	reg_149[2]	=	{0xB3,0x00};//Gamma-	R							
static	char	reg_150[2]	=	{0xB4,0x7D};//Gamma-	R							
static	char	reg_151[2]	=	{0xB5,0x00};//Gamma-	R							
static	char	reg_152[2]	=	{0xB6,0x8A};//Gamma-	R							
static	char	reg_153[2]	=	{0xB7,0x00};//Gamma-	R							
static	char	reg_154[2]	=	{0xB8,0x9C};//Gamma-	R							
static	char	reg_155[2]	=	{0xB9,0x00};//Gamma-	R							
static	char	reg_156[2]	=	{0xBA,0xB1};//Gamma-	R							
static	char	reg_157[2]	=	{0xBB,0x00};//Gamma-	R							
static	char	reg_158[2]	=	{0xBC,0xBF};//Gamma-	R							
static	char	reg_159[2]	=	{0xBD,0x00};//Gamma-	R							
static	char	reg_160[2]	=	{0xBE,0xCF};//Gamma-	R							
static	char	reg_161[2]	=	{0xBF,0x00};//Gamma-	R							
static	char	reg_162[2]	=	{0xC0,0xDD};//Gamma-	R							
static	char	reg_163[2]	=	{0xC1,0x00};//Gamma-	R							
static	char	reg_164[2]	=	{0xC2,0xE8};//Gamma-	R							
static	char	reg_165[2]	=	{0xC3,0x00};//Gamma-	R							
static	char	reg_166[2]	=	{0xC4,0xF2};//Gamma-	R							
static	char	reg_167[2]	=	{0xC5,0x01};//Gamma-	R							
static	char	reg_168[2]	=	{0xC6,0x1F};//Gamma-	R							
static	char	reg_169[2]	=	{0xC7,0x01};//Gamma-	R							
static	char	reg_170[2]	=	{0xC8,0x41};//Gamma-	R							
static	char	reg_171[2]	=	{0xC9,0x01};//Gamma-	R							
static	char	reg_172[2]	=	{0xCA,0x78};//Gamma-	R							
static	char	reg_173[2]	=	{0xCB,0x01};//Gamma-	R							
static	char	reg_174[2]	=	{0xCC,0xA5};//Gamma-	R							
static	char	reg_175[2]	=	{0xCD,0x01};//Gamma-	R							
static	char	reg_176[2]	=	{0xCE,0xEE};//Gamma-	R							
static	char	reg_177[2]	=	{0xCF,0x02};//Gamma-	R							
static	char	reg_178[2]	=	{0xD0,0x29};//Gamma-	R							
static	char	reg_179[2]	=	{0xD1,0x02};//Gamma-	R							
static	char	reg_180[2]	=	{0xD2,0x2A};//Gamma-	R							
static	char	reg_181[2]	=	{0xD3,0x02};//Gamma-	R							
static	char	reg_182[2]	=	{0xD4,0x5D};//Gamma-	R							
static	char	reg_183[2]	=	{0xD5,0x02};//Gamma-	R							
static	char	reg_184[2]	=	{0xD6,0x93};//Gamma-	R							
static	char	reg_185[2]	=	{0xD7,0x02};//Gamma-	R							
static	char	reg_186[2]	=	{0xD8,0xB8};//Gamma-	R							
static	char	reg_187[2]	=	{0xD9,0x02};//Gamma-	R							
static	char	reg_188[2]	=	{0xDA,0xE7};//Gamma-	R							
static	char	reg_189[2]	=	{0xDB,0x03};//Gamma-	R							
static	char	reg_190[2]	=	{0xDC,0x07};//Gamma-	R							
static	char	reg_191[2]	=	{0xDD,0x03};//Gamma-	R							
static	char	reg_192[2]	=	{0xDE,0x37};//Gamma-	R							
static	char	reg_193[2]	=	{0xDF,0x03};//Gamma-	R							
static	char	reg_194[2]	=	{0xE0,0x46};//Gamma-	R							
static	char	reg_195[2]	=	{0xE1,0x03};//Gamma-	R							
static	char	reg_196[2]	=	{0xE2,0x56};//Gamma-	R							
static	char	reg_197[2]	=	{0xE3,0x03};//Gamma-	R							
static	char	reg_198[2]	=	{0xE4,0x66};//Gamma-	R							
static	char	reg_199[2]	=	{0xE5,0x03};//Gamma-	R							
static	char	reg_200[2]	=	{0xE6,0x7A};//Gamma-	R							
static	char	reg_201[2]	=	{0xE7,0x03};//Gamma-	R							
static	char	reg_202[2]	=	{0xE8,0x93};//Gamma-	R							
static	char	reg_203[2]	=	{0xE9,0x03};//Gamma-	R							
static	char	reg_204[2]	=	{0xEA,0xA3};//Gamma-	R							
static	char	reg_205[2]	=	{0xEB,0x03};//Gamma-	R							
static	char	reg_206[2]	=	{0xEC,0xB4};//Gamma-	R							
static	char	reg_207[2]	=	{0xED,0x03};//Gamma-	R							
static	char	reg_208[2]	=	{0xEE,0xCB};//Gamma-	R							
												
//Setting	for	Gamma+	G									
static	char	reg_209[2]	=	{0xEF,0x00};//Gamma+	G							
static	char	reg_210[2]	=	{0xF0,0xED};//Gamma+	G							
static	char	reg_211[2]	=	{0xF1,0x00};//Gamma+	G							
static	char	reg_212[2]	=	{0xF2,0xF3};//Gamma+	G							
static	char	reg_213[2]	=	{0xF3,0x00};//Gamma+	G							
static	char	reg_214[2]	=	{0xF4,0xFE};//Gamma+	G							
static	char	reg_215[2]	=	{0xF5,0x01};//Gamma+	G							
static	char	reg_216[2]	=	{0xF6,0x09};//Gamma+	G							
static	char	reg_217[2]	=	{0xF7,0x01};//Gamma+	G							
static	char	reg_218[2]	=	{0xF8,0x13};//Gamma+	G							
static	char	reg_219[2]	=	{0xF9,0x01};//Gamma+	G							
static	char	reg_220[2]	=	{0xFA,0x1D};//Gamma+	G							
static	char	reg_221[2]	=	{0xFF,0x02};//Enter	CMD							
static	char	reg_222[2]	=	{0xFB,0x01};								
static	char	reg_223[2]	=	{0x00,0x01};//Gamma+	G							
static	char	reg_224[2]	=	{0x01,0x26};//Gamma+	G							
static	char	reg_225[2]	=	{0x02,0x01};//Gamma+	G							
static	char	reg_226[2]	=	{0x03,0x2F};//Gamma+	G							
static	char	reg_227[2]	=	{0x04,0x01};//Gamma+	G							
static	char	reg_228[2]	=	{0x05,0x37};//Gamma+	G							
static	char	reg_229[2]	=	{0x06,0x01};//Gamma+	G							
static	char	reg_230[2]	=	{0x07,0x56};//Gamma+	G							
static	char	reg_231[2]	=	{0x08,0x01};//Gamma+	G							
static	char	reg_232[2]	=	{0x09,0x70};//Gamma+	G							
static	char	reg_233[2]	=	{0x0A,0x01};//Gamma+	G							
static	char	reg_234[2]	=	{0x0B,0x9D};//Gamma+	G							
static	char	reg_235[2]	=	{0x0C,0x01};//Gamma+	G							
static	char	reg_236[2]	=	{0x0D,0xC2};//Gamma+	G							
static	char	reg_237[2]	=	{0x0E,0x01};//Gamma+	G							
static	char	reg_238[2]	=	{0x0F,0xFF};//Gamma+	G							
static	char	reg_239[2]	=	{0x10,0x02};//Gamma+	G							
static	char	reg_240[2]	=	{0x11,0x31};//Gamma+	G							
static	char	reg_241[2]	=	{0x12,0x02};//Gamma+	G							
static	char	reg_242[2]	=	{0x13,0x32};//Gamma+	G							
static	char	reg_243[2]	=	{0x14,0x02};//Gamma+	G							
static	char	reg_244[2]	=	{0x15,0x60};//Gamma+	G							
static	char	reg_245[2]	=	{0x16,0x02};//Gamma+	G							
static	char	reg_246[2]	=	{0x17,0x94};//Gamma+	G							
static	char	reg_247[2]	=	{0x18,0x02};//Gamma+	G							
static	char	reg_248[2]	=	{0x19,0xB5};//Gamma+	G							
static	char	reg_249[2]	=	{0x1A,0x02};//Gamma+	G							
static	char	reg_250[2]	=	{0x1B,0xE3};//Gamma+	G							
static	char	reg_251[2]	=	{0x1C,0x03};//Gamma+	G							
static	char	reg_252[2]	=	{0x1D,0x03};//Gamma+	G							
static	char	reg_253[2]	=	{0x1E,0x03};//Gamma+	G							
static	char	reg_254[2]	=	{0x1F,0x2D};//Gamma+	G							
static	char	reg_255[2]	=	{0x20,0x03};//Gamma+	G							
static	char	reg_256[2]	=	{0x21,0x3A};//Gamma+	G							
static	char	reg_257[2]	=	{0x22,0x03};//Gamma+	G							
static	char	reg_258[2]	=	{0x23,0x48};//Gamma+	G							
static	char	reg_259[2]	=	{0x24,0x03};//Gamma+	G							
static	char	reg_260[2]	=	{0x25,0x57};//Gamma+	G							
static	char	reg_261[2]	=	{0x26,0x03};//Gamma+	G							
static	char	reg_262[2]	=	{0x27,0x68};//Gamma+	G							
static	char	reg_263[2]	=	{0x28,0x03};//Gamma+	G							
static	char	reg_264[2]	=	{0x29,0x7B};//Gamma+	G							
static	char	reg_265[2]	=	{0x2A,0x03};//Gamma+	G							
static	char	reg_266[2]	=	{0x2B,0x90};//Gamma+	G							
static	char	reg_267[2]	=	{0x2D,0x03};//Gamma+	G							
static	char	reg_268[2]	=	{0x2F,0xA0};//Gamma+	G							
static	char	reg_269[2]	=	{0x30,0x03};//Gamma+	G							
static	char	reg_270[2]	=	{0x31,0xCB};//Gamma+	G							
												
//Setting	for	Gamma-	G									
static	char	reg_271[2]	=	{0x32,0x00};//Gamma-	G							
static	char	reg_272[2]	=	{0x33,0xED};//Gamma-	G							
static	char	reg_273[2]	=	{0x34,0x00};//Gamma-	G							
static	char	reg_274[2]	=	{0x35,0xF3};//Gamma-	G							
static	char	reg_275[2]	=	{0x36,0x00};//Gamma-	G							
static	char	reg_276[2]	=	{0x37,0xFE};//Gamma-	G							
static	char	reg_277[2]	=	{0x38,0x01};//Gamma-	G							
static	char	reg_278[2]	=	{0x39,0x09};//Gamma-	G							
static	char	reg_279[2]	=	{0x3A,0x01};//Gamma-	G							
static	char	reg_280[2]	=	{0x3B,0x13};//Gamma-	G							
static	char	reg_281[2]	=	{0x3D,0x01};//Gamma-	G							
static	char	reg_282[2]	=	{0x3F,0x1D};//Gamma-	G							
static	char	reg_283[2]	=	{0x40,0x01};//Gamma-	G							
static	char	reg_284[2]	=	{0x41,0x26};//Gamma-	G	2						
static	char	reg_285[2]	=	{0x42,0x01};//Gamma-	G							
static	char	reg_286[2]	=	{0x43,0x2F};//Gamma-	G							
static	char	reg_287[2]	=	{0x44,0x01};//Gamma-	G							
static	char	reg_288[2]	=	{0x45,0x37};//Gamma-	G							
static	char	reg_289[2]	=	{0x46,0x01};//Gamma-	G							
static	char	reg_290[2]	=	{0x47,0x56};//Gamma-	G							
static	char	reg_291[2]	=	{0x48,0x01};//Gamma-	G							
static	char	reg_292[2]	=	{0x49,0x70};//Gamma-	G							
static	char	reg_293[2]	=	{0x4A,0x01};//Gamma-	G							
static	char	reg_294[2]	=	{0x4B,0x9D};//Gamma-	G							
static	char	reg_295[2]	=	{0x4C,0x01};//Gamma-	G							
static	char	reg_296[2]	=	{0x4D,0xC2};//Gamma-	G							
static	char	reg_297[2]	=	{0x4E,0x01};//Gamma-	G							
static	char	reg_298[2]	=	{0x4F,0xFF};//Gamma-	G							
static	char	reg_299[2]	=	{0x50,0x02};//Gamma-	G							
static	char	reg_300[2]	=	{0x51,0x31};//Gamma-	G							
static	char	reg_301[2]	=	{0x52,0x02};//Gamma-	G							
static	char	reg_302[2]	=	{0x53,0x32};//Gamma-	G							
static	char	reg_303[2]	=	{0x54,0x02};//Gamma-	G							
static	char	reg_304[2]	=	{0x55,0x60};//Gamma-	G							
static	char	reg_305[2]	=	{0x56,0x02};//Gamma-	G							
static	char	reg_306[2]	=	{0x58,0x94};//Gamma-	G							
static	char	reg_307[2]	=	{0x59,0x02};//Gamma-	G							
static	char	reg_308[2]	=	{0x5A,0xB5};//Gamma-	G							
static	char	reg_309[2]	=	{0x5B,0x02};//Gamma-	G							
static	char	reg_310[2]	=	{0x5C,0xE3};//Gamma-	G							
static	char	reg_311[2]	=	{0x5D,0x03};//Gamma-	G							
static	char	reg_312[2]	=	{0x5E,0x03};//Gamma-	G							
static	char	reg_313[2]	=	{0x5F,0x03};//Gamma-	G							
static	char	reg_314[2]	=	{0x60,0x2D};//Gamma-	G							
static	char	reg_315[2]	=	{0x61,0x03};//Gamma-	G							
static	char	reg_316[2]	=	{0x62,0x3A};//Gamma-	G							
static	char	reg_317[2]	=	{0x63,0x03};//Gamma-	G							
static	char	reg_318[2]	=	{0x64,0x48};//Gamma-	G							
static	char	reg_319[2]	=	{0x65,0x03};//Gamma-	G							
static	char	reg_320[2]	=	{0x66,0x57};//Gamma-	G							
static	char	reg_321[2]	=	{0x67,0x03};//Gamma-	G							
static	char	reg_322[2]	=	{0x68,0x68};//Gamma-	G							
static	char	reg_323[2]	=	{0x69,0x03};//Gamma-	G							
static	char	reg_324[2]	=	{0x6A,0x7B};//Gamma-	G							
static	char	reg_325[2]	=	{0x6B,0x03};//Gamma-	G							
static	char	reg_326[2]	=	{0x6C,0x90};//Gamma-	G							
static	char	reg_327[2]	=	{0x6D,0x03};//Gamma-	G							
static	char	reg_328[2]	=	{0x6E,0xA0};//Gamma-	G							
static	char	reg_329[2]	=	{0x6F,0x03};//Gamma-	G							
static	char	reg_330[2]	=	{0x70,0xCB};//Gamma-	G							
												
//Setting	for	Gamma+	B									
static	char	reg_331[2]	=	{0x71,0x00};//Gamma+	B							
static	char	reg_332[2]	=	{0x72,0x19};//Gamma+	B							
static	char	reg_333[2]	=	{0x73,0x00};//Gamma+	B							
static	char	reg_334[2]	=	{0x74,0x36};//Gamma+	B							
static	char	reg_335[2]	=	{0x75,0x00};//Gamma+	B							
static	char	reg_336[2]	=	{0x76,0x55};//Gamma+	B							
static	char	reg_337[2]	=	{0x77,0x00};//Gamma+	B							
static	char	reg_338[2]	=	{0x78,0x70};//Gamma+	B							
static	char	reg_339[2]	=	{0x79,0x00};//Gamma+	B							
static	char	reg_340[2]	=	{0x7A,0x83};//Gamma+	B							
static	char	reg_341[2]	=	{0x7B,0x00};//Gamma+	B							
static	char	reg_342[2]	=	{0x7C,0x99};//Gamma+	B							
static	char	reg_343[2]	=	{0x7D,0x00};//Gamma+	B							
static	char	reg_344[2]	=	{0x7E,0xA8};//Gamma+	B							
static	char	reg_345[2]	=	{0x7F,0x00};//Gamma+	B							
static	char	reg_346[2]	=	{0x80,0xB7};//Gamma+	B							
static	char	reg_347[2]	=	{0x81,0x00};//Gamma+	B							
static	char	reg_348[2]	=	{0x82,0xC5};//Gamma+	B							
static	char	reg_349[2]	=	{0x83,0x00};//Gamma+	B							
static	char	reg_350[2]	=	{0x84,0xF7};//Gamma+	B							
static	char	reg_351[2]	=	{0x85,0x01};//Gamma+	B							
static	char	reg_352[2]	=	{0x86,0x1E};//Gamma+	B							
static	char	reg_353[2]	=	{0x87,0x01};//Gamma+	B							
static	char	reg_354[2]	=	{0x88,0x60};//Gamma+	B							
static	char	reg_355[2]	=	{0x89,0x01};//Gamma+	B							
static	char	reg_356[2]	=	{0x8A,0x95};//Gamma+	B							
static	char	reg_357[2]	=	{0x8B,0x01};//Gamma+	B							
static	char	reg_358[2]	=	{0x8C,0xE1};//Gamma+	B							
static	char	reg_359[2]	=	{0x8D,0x02};//Gamma+	B							
static	char	reg_360[2]	=	{0x8E,0x20};//Gamma+	B							
static	char	reg_361[2]	=	{0x8F,0x02};//Gamma+	B							
static	char	reg_362[2]	=	{0x90,0x23};//Gamma+	B							
static	char	reg_363[2]	=	{0x91,0x02};//Gamma+	B							
static	char	reg_364[2]	=	{0x92,0x59};//Gamma+	B							
static	char	reg_365[2]	=	{0x93,0x02};//Gamma+	B							
static	char	reg_366[2]	=	{0x94,0x94};//Gamma+	B							
static	char	reg_367[2]	=	{0x95,0x02};//Gamma+	B							
static	char	reg_368[2]	=	{0x96,0xB4};//Gamma+	B							
static	char	reg_369[2]	=	{0x97,0x02};//Gamma+	B							
static	char	reg_370[2]	=	{0x98,0xE1};//Gamma+	B							
static	char	reg_371[2]	=	{0x99,0x03};//Gamma+	B							
static	char	reg_372[2]	=	{0x9A,0x01};//Gamma+	B							
static	char	reg_373[2]	=	{0x9B,0x03};//Gamma+	B							
static	char	reg_374[2]	=	{0x9C,0x28};//Gamma+	B							
static	char	reg_375[2]	=	{0x9D,0x03};//Gamma+	B							
static	char	reg_376[2]	=	{0x9E,0x30};//Gamma+	B							
static	char	reg_377[2]	=	{0x9F,0x03};//Gamma+	B							
static	char	reg_378[2]	=	{0xA0,0x37};//Gamma+	B							
static	char	reg_379[2]	=	{0xA2,0x03};//Gamma+	B							
static	char	reg_380[2]	=	{0xA3,0x3B};//Gamma+	B							
static	char	reg_381[2]	=	{0xA4,0x03};//Gamma+	B							
static	char	reg_382[2]	=	{0xA5,0x40};//Gamma+	B							
static	char	reg_383[2]	=	{0xA6,0x03};//Gamma+	B							
static	char	reg_384[2]	=	{0xA7,0x50};//Gamma+	B							
static	char	reg_385[2]	=	{0xA9,0x03};//Gamma+	B							
static	char	reg_386[2]	=	{0xAA,0x6D};//Gamma+	B							
static	char	reg_387[2]	=	{0xAB,0x03};//Gamma+	B							
static	char	reg_388[2]	=	{0xAC,0x80};//Gamma+	B							
static	char	reg_389[2]	=	{0xAD,0x03};//Gamma+	B							
static	char	reg_390[2]	=	{0xAE,0xCB};//Gamma+	B							
												
//Setting	for	Gamma-	B									
static	char	reg_391[2]	=	{0xAF,0x00};//Gamma-	B							
static	char	reg_392[2]	=	{0xB0,0x19};//Gamma-	B							
static	char	reg_393[2]	=	{0xB1,0x00};//Gamma-	B							
static	char	reg_394[2]	=	{0xB2,0x36};//Gamma-	B							
static	char	reg_395[2]	=	{0xB3,0x00};//Gamma-	B							
static	char	reg_396[2]	=	{0xB4,0x55};//Gamma-	B							
static	char	reg_397[2]	=	{0xB5,0x00};//Gamma-	B							
static	char	reg_398[2]	=	{0xB6,0x70};//Gamma-	B							
static	char	reg_399[2]	=	{0xB7,0x00};//Gamma-	B							
static	char	reg_400[2]	=	{0xB8,0x83};//Gamma-	B							
static	char	reg_401[2]	=	{0xB9,0x00};//Gamma-	B							
static	char	reg_402[2]	=	{0xBA,0x99};//Gamma-	B							
static	char	reg_403[2]	=	{0xBB,0x00};//Gamma-	B							
static	char	reg_404[2]	=	{0xBC,0xA8};//Gamma-	B							
static	char	reg_405[2]	=	{0xBD,0x00};//Gamma-	B							
static	char	reg_406[2]	=	{0xBE,0xB7};//Gamma-	B							
static	char	reg_407[2]	=	{0xBF,0x00};//Gamma-	B							
static	char	reg_408[2]	=	{0xC0,0xC5};//Gamma-	B							
static	char	reg_409[2]	=	{0xC1,0x00};//Gamma-	B							
static	char	reg_410[2]	=	{0xC2,0xF7};//Gamma-	B							
static	char	reg_411[2]	=	{0xC3,0x01};//Gamma-	B							
static	char	reg_412[2]	=	{0xC4,0x1E};//Gamma-	B							
static	char	reg_413[2]	=	{0xC5,0x01};//Gamma-	B							
static	char	reg_414[2]	=	{0xC6,0x60};//Gamma-	B							
static	char	reg_415[2]	=	{0xC7,0x01};//Gamma-	B							
static	char	reg_416[2]	=	{0xC8,0x95};//Gamma-	B							
static	char	reg_417[2]	=	{0xC9,0x01};//Gamma-	B							
static	char	reg_418[2]	=	{0xCA,0xE1};//Gamma-	B							
static	char	reg_419[2]	=	{0xCB,0x02};//Gamma-	B							
static	char	reg_420[2]	=	{0xCC,0x20};//Gamma-	B							
static	char	reg_421[2]	=	{0xCD,0x02};//Gamma-	B							
static	char	reg_422[2]	=	{0xCE,0x23};//Gamma-	B							
static	char	reg_423[2]	=	{0xCF,0x02};//Gamma-	B							
static	char	reg_424[2]	=	{0xD0,0x59};//Gamma-	B							
static	char	reg_425[2]	=	{0xD1,0x02};//Gamma-	B							
static	char	reg_426[2]	=	{0xD2,0x94};//Gamma-	B							
static	char	reg_427[2]	=	{0xD3,0x02};//Gamma-	B							
static	char	reg_428[2]	=	{0xD4,0xB4};//Gamma-	B							
static	char	reg_429[2]	=	{0xD5,0x02};//Gamma-	B							
static	char	reg_430[2]	=	{0xD6,0xE1};//Gamma-	B							
static	char	reg_431[2]	=	{0xD7,0x03};//Gamma-	B							
static	char	reg_432[2]	=	{0xD8,0x01};//Gamma-	B							
static	char	reg_433[2]	=	{0xD9,0x03};//Gamma-	B							
static	char	reg_434[2]	=	{0xDA,0x28};//Gamma-	B							
static	char	reg_435[2]	=	{0xDB,0x03};//Gamma-	B							
static	char	reg_436[2]	=	{0xDC,0x30};//Gamma-	B							
static	char	reg_437[2]	=	{0xDD,0x03};//Gamma-	B							
static	char	reg_438[2]	=	{0xDE,0x37};//Gamma-	B							
static	char	reg_439[2]	=	{0xDF,0x03};//Gamma-	B							
static	char	reg_440[2]	=	{0xE0,0x3B};//Gamma-	B							
static	char	reg_441[2]	=	{0xE1,0x03};//Gamma-	B							
static	char	reg_442[2]	=	{0xE2,0x40};//Gamma-	B							
static	char	reg_443[2]	=	{0xE3,0x03};//Gamma-	B							
static	char	reg_444[2]	=	{0xE4,0x50};//Gamma-	B							
static	char	reg_445[2]	=	{0xE5,0x03};//Gamma-	B							
static	char	reg_446[2]	=	{0xE6,0x6D};//Gamma-	B							
static	char	reg_447[2]	=	{0xE7,0x03};//Gamma-	B							
static	char	reg_448[2]	=	{0xE8,0x80};//Gamma-	B							
static	char	reg_449[2]	=	{0xE9,0x03};//Gamma-	B							
static	char	reg_450[2]	=	{0xEA,0xCB};//Gamma-	B							
static	char	reg_451[2]	=	{0xFF,0x01};								
static	char	reg_452[2]	=	{0xFB,0x01};								
static	char	reg_453[2]	=	{0xFF,0x02};								
static	char	reg_454[2]	=	{0xFB,0x01};								
static	char	reg_455[2]	=	{0xFF,0x04};								
static	char	reg_456[2]	=	{0xFB,0x01};								
static	char	reg_457[2]	=	{0xFF,0x00};								
static	char	reg_458[2]	=	{0x11,0x00};	//Sleep	out						
//Delayms(100);												
static	char	reg_459[2]	=	{0xFF,0xEE};								
static	char	reg_460[2]	=	{0x12,0x50};								
static	char	reg_461[2]	=	{0x13,0x02};								
static	char	reg_462[2]	=	{0x6A,0x60};								
static	char	reg_463[2]	=	{0xFF,0x00};								
static	char	reg_464[2]	=	{0x29,0x00};	//Display	on	     

//static char reg_addr21[2] = {0x21, 0x00};

//static char exit_sleep[2] = {0x11, 0x00};
//static char display_on[2] = {0x29, 0x00};
static char display_off[2] = {0x28, 0x00};
static char enter_sleep[2] = {0x10, 0x00};

static struct dsi_cmd_desc nt35590_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 40, sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 150, sizeof(enter_sleep), enter_sleep}
};

static struct dsi_cmd_desc nt35590_video_display_on_cmds[] = {

{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_1	),	reg_1	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_2	),	reg_2	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_3	),	reg_3	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 10,				
sizeof(	reg_4	),	reg_4	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_5	),	reg_5	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_6	),	reg_6	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_7	),	reg_7	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_8	),	reg_8	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_9	),	reg_9	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_10	),	reg_10	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_11	),	reg_11	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_12	),	reg_12	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_13	),	reg_13	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_14	),	reg_14	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_15	),	reg_15	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_16	),	reg_16	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_17	),	reg_17	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_18	),	reg_18	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_19	),	reg_19	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_20	),	reg_20	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_21	),	reg_21	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_22	),	reg_22	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_23	),	reg_23	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_24	),	reg_24	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_25	),	reg_25	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_26	),	reg_26	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_27	),	reg_27	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_28	),	reg_28	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_29	),	reg_29	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_30	),	reg_30	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_31	),	reg_31	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_32	),	reg_32	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_33	),	reg_33	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_34	),	reg_34	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_35	),	reg_35	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_36	),	reg_36	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_37	),	reg_37	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_38	),	reg_38	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_39	),	reg_39	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_40	),	reg_40	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_41	),	reg_41	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_42	),	reg_42	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_43	),	reg_43	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_44	),	reg_44	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_45	),	reg_45	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_46	),	reg_46	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_47	),	reg_47	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_48	),	reg_48	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_49	),	reg_49	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_50	),	reg_50	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_51	),	reg_51	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_52	),	reg_52	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_53	),	reg_53	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_54	),	reg_54	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_55	),	reg_55	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_56	),	reg_56	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_57	),	reg_57	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_58	),	reg_58	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_59	),	reg_59	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_60	),	reg_60	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_61	),	reg_61	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_62	),	reg_62	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_63	),	reg_63	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_64	),	reg_64	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_65	),	reg_65	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_66	),	reg_66	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_67	),	reg_67	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_68	),	reg_68	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_69	),	reg_69	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_70	),	reg_70	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_71	),	reg_71	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_72	),	reg_72	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_73	),	reg_73	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_74	),	reg_74	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_75	),	reg_75	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_76	),	reg_76	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_77	),	reg_77	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_78	),	reg_78	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_79	),	reg_79	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_80	),	reg_80	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_81	),	reg_81	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_82	),	reg_82	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_83	),	reg_83	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_84	),	reg_84	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_85	),	reg_85	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_86	),	reg_86	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_87	),	reg_87	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_88	),	reg_88	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_89	),	reg_89	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_90	),	reg_90	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_91	),	reg_91	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_92	),	reg_92	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_93	),	reg_93	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_94	),	reg_94	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_95	),	reg_95	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_96	),	reg_96	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_97	),	reg_97	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_98	),	reg_98	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_99	),	reg_99	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_100	),	reg_100	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_101	),	reg_101	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_102	),	reg_102	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_103	),	reg_103	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_104	),	reg_104	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_105	),	reg_105	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_106	),	reg_106	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_107	),	reg_107	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_108	),	reg_108	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_109	),	reg_109	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_110	),	reg_110	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_111	),	reg_111	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_112	),	reg_112	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_113	),	reg_113	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_114	),	reg_114	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_115	),	reg_115	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_116	),	reg_116	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_117	),	reg_117	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_118	),	reg_118	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_119	),	reg_119	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_120	),	reg_120	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_121	),	reg_121	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_122	),	reg_122	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_123	),	reg_123	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_124	),	reg_124	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_125	),	reg_125	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_126	),	reg_126	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_127	),	reg_127	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_128	),	reg_128	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_129	),	reg_129	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_130	),	reg_130	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_131	),	reg_131	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_132	),	reg_132	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_133	),	reg_133	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_134	),	reg_134	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_135	),	reg_135	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_136	),	reg_136	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_137	),	reg_137	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_138	),	reg_138	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_139	),	reg_139	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_140	),	reg_140	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_141	),	reg_141	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_142	),	reg_142	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_143	),	reg_143	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_144	),	reg_144	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_145	),	reg_145	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_146	),	reg_146	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_147	),	reg_147	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_148	),	reg_148	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_149	),	reg_149	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_150	),	reg_150	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_151	),	reg_151	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_152	),	reg_152	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_153	),	reg_153	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_154	),	reg_154	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_155	),	reg_155	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_156	),	reg_156	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_157	),	reg_157	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_158	),	reg_158	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_159	),	reg_159	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_160	),	reg_160	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_161	),	reg_161	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_162	),	reg_162	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_163	),	reg_163	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_164	),	reg_164	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_165	),	reg_165	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_166	),	reg_166	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_167	),	reg_167	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_168	),	reg_168	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_169	),	reg_169	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_170	),	reg_170	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_171	),	reg_171	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_172	),	reg_172	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_173	),	reg_173	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_174	),	reg_174	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_175	),	reg_175	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_176	),	reg_176	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_177	),	reg_177	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_178	),	reg_178	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_179	),	reg_179	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_180	),	reg_180	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_181	),	reg_181	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_182	),	reg_182	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_183	),	reg_183	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_184	),	reg_184	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_185	),	reg_185	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_186	),	reg_186	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_187	),	reg_187	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_188	),	reg_188	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_189	),	reg_189	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_190	),	reg_190	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_191	),	reg_191	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_192	),	reg_192	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_193	),	reg_193	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_194	),	reg_194	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_195	),	reg_195	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_196	),	reg_196	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_197	),	reg_197	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_198	),	reg_198	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_199	),	reg_199	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_200	),	reg_200	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_201	),	reg_201	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_202	),	reg_202	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_203	),	reg_203	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_204	),	reg_204	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_205	),	reg_205	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_206	),	reg_206	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_207	),	reg_207	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_208	),	reg_208	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_209	),	reg_209	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_210	),	reg_210	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_211	),	reg_211	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_212	),	reg_212	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_213	),	reg_213	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_214	),	reg_214	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_215	),	reg_215	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_216	),	reg_216	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_217	),	reg_217	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_218	),	reg_218	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_219	),	reg_219	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_220	),	reg_220	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_221	),	reg_221	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_222	),	reg_222	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_223	),	reg_223	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_224	),	reg_224	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_225	),	reg_225	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_226	),	reg_226	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_227	),	reg_227	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_228	),	reg_228	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_229	),	reg_229	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_230	),	reg_230	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_231	),	reg_231	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_232	),	reg_232	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_233	),	reg_233	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_234	),	reg_234	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_235	),	reg_235	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_236	),	reg_236	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_237	),	reg_237	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_238	),	reg_238	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_239	),	reg_239	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_240	),	reg_240	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_241	),	reg_241	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_242	),	reg_242	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_243	),	reg_243	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_244	),	reg_244	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_245	),	reg_245	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_246	),	reg_246	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_247	),	reg_247	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_248	),	reg_248	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_249	),	reg_249	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_250	),	reg_250	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_251	),	reg_251	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_252	),	reg_252	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_253	),	reg_253	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_254	),	reg_254	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_255	),	reg_255	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_256	),	reg_256	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_257	),	reg_257	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_258	),	reg_258	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_259	),	reg_259	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_260	),	reg_260	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_261	),	reg_261	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_262	),	reg_262	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_263	),	reg_263	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_264	),	reg_264	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_265	),	reg_265	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_266	),	reg_266	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_267	),	reg_267	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_268	),	reg_268	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_269	),	reg_269	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_270	),	reg_270	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_271	),	reg_271	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_272	),	reg_272	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_273	),	reg_273	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_274	),	reg_274	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_275	),	reg_275	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_276	),	reg_276	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_277	),	reg_277	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_278	),	reg_278	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_279	),	reg_279	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_280	),	reg_280	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_281	),	reg_281	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_282	),	reg_282	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_283	),	reg_283	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_284	),	reg_284	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_285	),	reg_285	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_286	),	reg_286	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_287	),	reg_287	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_288	),	reg_288	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_289	),	reg_289	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_290	),	reg_290	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_291	),	reg_291	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_292	),	reg_292	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_293	),	reg_293	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_294	),	reg_294	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_295	),	reg_295	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_296	),	reg_296	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_297	),	reg_297	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_298	),	reg_298	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_299	),	reg_299	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_300	),	reg_300	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_301	),	reg_301	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_302	),	reg_302	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_303	),	reg_303	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_304	),	reg_304	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_305	),	reg_305	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_306	),	reg_306	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_307	),	reg_307	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_308	),	reg_308	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_309	),	reg_309	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_310	),	reg_310	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_311	),	reg_311	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_312	),	reg_312	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_313	),	reg_313	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_314	),	reg_314	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_315	),	reg_315	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_316	),	reg_316	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_317	),	reg_317	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_318	),	reg_318	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_319	),	reg_319	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_320	),	reg_320	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_321	),	reg_321	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_322	),	reg_322	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_323	),	reg_323	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_324	),	reg_324	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_325	),	reg_325	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_326	),	reg_326	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_327	),	reg_327	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_328	),	reg_328	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_329	),	reg_329	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_330	),	reg_330	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_331	),	reg_331	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_332	),	reg_332	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_333	),	reg_333	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_334	),	reg_334	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_335	),	reg_335	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_336	),	reg_336	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_337	),	reg_337	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_338	),	reg_338	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_339	),	reg_339	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_340	),	reg_340	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_341	),	reg_341	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_342	),	reg_342	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_343	),	reg_343	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_344	),	reg_344	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_345	),	reg_345	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_346	),	reg_346	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_347	),	reg_347	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_348	),	reg_348	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_349	),	reg_349	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_350	),	reg_350	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_351	),	reg_351	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_352	),	reg_352	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_353	),	reg_353	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_354	),	reg_354	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_355	),	reg_355	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_356	),	reg_356	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_357	),	reg_357	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_358	),	reg_358	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_359	),	reg_359	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_360	),	reg_360	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_361	),	reg_361	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_362	),	reg_362	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_363	),	reg_363	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_364	),	reg_364	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_365	),	reg_365	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_366	),	reg_366	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_367	),	reg_367	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_368	),	reg_368	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_369	),	reg_369	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_370	),	reg_370	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_371	),	reg_371	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_372	),	reg_372	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_373	),	reg_373	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_374	),	reg_374	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_375	),	reg_375	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_376	),	reg_376	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_377	),	reg_377	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_378	),	reg_378	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_379	),	reg_379	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_380	),	reg_380	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_381	),	reg_381	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_382	),	reg_382	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_383	),	reg_383	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_384	),	reg_384	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_385	),	reg_385	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_386	),	reg_386	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_387	),	reg_387	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_388	),	reg_388	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_389	),	reg_389	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_390	),	reg_390	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_391	),	reg_391	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_392	),	reg_392	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_393	),	reg_393	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_394	),	reg_394	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_395	),	reg_395	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_396	),	reg_396	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_397	),	reg_397	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_398	),	reg_398	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_399	),	reg_399	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_400	),	reg_400	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_401	),	reg_401	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_402	),	reg_402	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_403	),	reg_403	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_404	),	reg_404	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_405	),	reg_405	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_406	),	reg_406	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_407	),	reg_407	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_408	),	reg_408	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_409	),	reg_409	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_410	),	reg_410	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_411	),	reg_411	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_412	),	reg_412	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_413	),	reg_413	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_414	),	reg_414	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_415	),	reg_415	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_416	),	reg_416	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_417	),	reg_417	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_418	),	reg_418	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_419	),	reg_419	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_420	),	reg_420	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_421	),	reg_421	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_422	),	reg_422	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_423	),	reg_423	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_424	),	reg_424	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_425	),	reg_425	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_426	),	reg_426	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_427	),	reg_427	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_428	),	reg_428	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_429	),	reg_429	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_430	),	reg_430	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_431	),	reg_431	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_432	),	reg_432	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_433	),	reg_433	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_434	),	reg_434	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_435	),	reg_435	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_436	),	reg_436	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_437	),	reg_437	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_438	),	reg_438	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_439	),	reg_439	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_440	),	reg_440	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_441	),	reg_441	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_442	),	reg_442	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_443	),	reg_443	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_444	),	reg_444	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_445	),	reg_445	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_446	),	reg_446	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_447	),	reg_447	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_448	),	reg_448	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_449	),	reg_449	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_450	),	reg_450	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_451	),	reg_451	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_452	),	reg_452	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_453	),	reg_453	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_454	),	reg_454	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_455	),	reg_455	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_456	),	reg_456	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_457	),	reg_457	},
{DTYPE_DCS_WRITE, 1, 0, 0, 100,				
sizeof(	reg_458	),	reg_458	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_459	),	reg_459	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_460	),	reg_460	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_461	),	reg_461	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_462	),	reg_462	},
{DTYPE_DCS_LWRITE, 1, 0, 0, 0,				
sizeof(	reg_463	),	reg_463	},
{DTYPE_DCS_WRITE, 1, 0, 0, 0,				
sizeof(	reg_464	),	reg_464	}
};

static void kpd_bl_ctrl(unsigned int onoff)
{
    int ret;

    if (onoff) {
	ret=pmic_secure_mpp_config_i_sink(PM_MPP_8,PM_MPP__I_SINK__LEVEL_5mA,PM_MPP__I_SINK__SWITCH_ENA);
    } else {
	ret=pmic_secure_mpp_config_i_sink(PM_MPP_8,PM_MPP__I_SINK__LEVEL_5mA,PM_MPP__I_SINK__SWITCH_DIS);
    }

   if (ret)
	   printk("kpd_bl_ctrl led fail ret=%d\n",ret);

   return;

}

#if DRV_READ_LCDID_MISC_IOCTL
static int lcd_id_flag;
//extern int msm_fb_lcdc_config(int on);
#define LCD_ID 114
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

/*tydrv niuli add for keypad light misc control on 20111117*/
#if DRV_READ_LCDID_MISC_IOCTL
static struct platform_device *truly_misc_data;

#define TRULYIO 0x1f
#define TRULY_IOCTL_KPD_SET _IOW(TRULYIO, 0x01, int)
#define TRULY_IOCTL_GET_LCD_ID _IOR(TRULYIO, 0x02, char[11])	

static int truly_open(struct inode *inode, struct file *file)
{
	return nonseekable_open(inode, file);
}

static long
truly_ioctl(struct file *file, unsigned int cmd,
		   unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int ret ;
	unsigned int kpd_state=0;
	char buf[11];
	char * str1 = "truly_lcd ";
	char * str2 = "byd_lcd";

	switch (cmd) {
		case TRULY_IOCTL_KPD_SET://set keppad light 	
			if (copy_from_user(&kpd_state, argp, sizeof(kpd_state))){
				ret = -EFAULT;
				break;
			}
			printk(KERN_INFO "truly_ioctl:--->>>>TRULY_IOCTL_KPD_SET  kpd_state=%d\r\n", kpd_state);	
			if(kpd_state){
				kpd_bl_ctrl(1);
			}
			else{
				kpd_bl_ctrl(0);
			}
				
			break;
 		case TRULY_IOCTL_GET_LCD_ID:
	
			if(lcd_id_flag){
				strcpy(buf,str1);
			}else{
				strcpy(buf,str2);
			}
			if(copy_to_user(argp,buf,strlen(buf))){
				printk("ty_lcd:TRULY_IOCTL_GET_LCD_ID read failed \n");
				return -EFAULT;
			}
			printk(KERN_INFO "truly_ioctl:--->>>>:TRULY_IOCTL_GET_LCD_ID\n lcd_id=%d device is (%s)\n",lcd_id_flag,buf);
				
			break;

		default:
			break;	
		}
	return ret;
}

static struct file_operations truly_fops = {
	.owner = THIS_MODULE,
	.open = truly_open,
	.unlocked_ioctl = truly_ioctl,
};

static struct miscdevice truly_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "truly_misc_device",
	.fops = &truly_fops,
};
#endif

static int mipi_nt35590_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;

	pr_debug("mipi_nt35590_lcd_on E\n");

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi  = &mfd->panel_info.mipi;

	if (!mfd->cont_splash_done) {
		mfd->cont_splash_done = 1;
		return 0;
	}

	if (mipi->mode == DSI_VIDEO_MODE) {
		mipi_dsi_cmds_tx(&nt35590_tx_buf,
			nt35590_video_display_on_cmds,
			ARRAY_SIZE(nt35590_video_display_on_cmds));
	}

	pr_debug("mipi_nt35590_lcd_on X\n");

	return 0;
}

static int mipi_nt35590_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	pr_debug("mipi_nt35590_lcd_off E\n");

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_cmds_tx(&nt35590_tx_buf, nt35590_display_off_cmds,
			ARRAY_SIZE(nt35590_display_off_cmds));

	pr_debug("mipi_nt35590_lcd_off X\n");
	return 0;
}

static ssize_t mipi_nt35590_wta_bl_ctrl(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int err;

	err =  kstrtoint(buf, 0, &mipi_nt35590_bl_ctrl);
	if (err)
		return ret;

	pr_info("%s: bl ctrl set to %d\n", __func__, mipi_nt35590_bl_ctrl);

	return ret;
}

static DEVICE_ATTR(bl_ctrl, S_IWUSR, NULL, mipi_nt35590_wta_bl_ctrl);

static struct attribute *mipi_nt35590_fs_attrs[] = {
	&dev_attr_bl_ctrl.attr,
	NULL,
};

static struct attribute_group mipi_nt35590_fs_attr_group = {
	.attrs = mipi_nt35590_fs_attrs,
};

static int mipi_nt35590_create_sysfs(struct platform_device *pdev)
{
	int rc;
	struct msm_fb_data_type *mfd = platform_get_drvdata(pdev);

	if (!mfd) {
		pr_err("%s: mfd not found\n", __func__);
		return -ENODEV;
	}
	if (!mfd->fbi) {
		pr_err("%s: mfd->fbi not found\n", __func__);
		return -ENODEV;
	}
	if (!mfd->fbi->dev) {
		pr_err("%s: mfd->fbi->dev not found\n", __func__);
		return -ENODEV;
	}
	rc = sysfs_create_group(&mfd->fbi->dev->kobj,
		&mipi_nt35590_fs_attr_group);
	if (rc) {
		pr_err("%s: sysfs group creation failed, rc=%d\n",
			__func__, rc);
		return rc;
	}

	return 0;
}

static int __devinit mipi_nt35590_lcd_probe(struct platform_device *pdev)
{
	struct platform_device *pthisdev = NULL;
	pr_debug("%s\n", __func__);

	if (pdev->id == 0) {
		mipi_nt35590_pdata = pdev->dev.platform_data;
		if (mipi_nt35590_pdata->bl_lock)
			spin_lock_init(&mipi_nt35590_pdata->bl_spinlock);

                /* SKUD use PWM as backlight control method */
                if(machine_is_msm8625q_skud()) {
                        mipi_nt35590_bl_ctrl = 1;
                }

		return 0;
	}
#if DRV_READ_LCDID_MISC_IOCTL
		msleep(2);
		lcd_id_flag = tyq_lcd_read_id(LCD_ID);
		printk(KERN_ERR "--TY--lcd:truly_probe id=%d(0:tianma 1:truly)\n",lcd_id_flag);
		truly_misc_data = pdev;
		misc_register(&truly_device);
#endif

	pthisdev = msm_fb_add_device(pdev);
	mipi_nt35590_create_sysfs(pthisdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_nt35590_lcd_probe,
	.driver = {
		.name   = "mipi_nt35590",
	},
};

static int old_bl_level;

static void mipi_nt35590_set_backlight(struct msm_fb_data_type *mfd)
{
	int bl_level;
	unsigned long flags;
	bl_level = mfd->bl_level;

	if (mipi_nt35590_pdata->bl_lock) {
		if (!mipi_nt35590_bl_ctrl) {
			/* Level received is of range 1 to bl_max,
			   We need to convert the levels to 1
			   to 31 */
			bl_level = (2 * bl_level * 31 + mfd->panel_info.bl_max)
					/(2 * mfd->panel_info.bl_max);
			if (bl_level == old_bl_level)
				return;

			if (bl_level == 0)
				mipi_nt35590_pdata->backlight(0, 1);

			if (old_bl_level == 0)
				mipi_nt35590_pdata->backlight(50, 1);

			spin_lock_irqsave(&mipi_nt35590_pdata->bl_spinlock,
						flags);
			mipi_nt35590_pdata->backlight(bl_level, 0);
			spin_unlock_irqrestore(&mipi_nt35590_pdata->bl_spinlock,
						flags);
			old_bl_level = bl_level;
		} else {
			mipi_nt35590_pdata->backlight(bl_level, 1);
		}
	} else {
		mipi_nt35590_pdata->backlight(bl_level, mipi_nt35590_bl_ctrl);
	}
}

static struct msm_fb_panel_data nt35590_panel_data = {
	.on	= mipi_nt35590_lcd_on,
	.off = mipi_nt35590_lcd_off,
	.set_backlight = mipi_nt35590_set_backlight,
};

static int ch_used[3];

static int mipi_nt35590_lcd_init(void)
{
	mipi_dsi_buf_alloc(&nt35590_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&nt35590_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

int mipi_nt35590_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	ret = mipi_nt35590_lcd_init();
	if (ret) {
		pr_err("mipi_nt35590_lcd_init() failed with ret %u\n", ret);
		return ret;
	}

	pdev = platform_device_alloc("mipi_nt35590", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	nt35590_panel_data.panel_info = *pinfo;
	ret = platform_device_add_data(pdev, &nt35590_panel_data,
				sizeof(nt35590_panel_data));
	if (ret) {
		pr_debug("%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		pr_debug("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

#endif
