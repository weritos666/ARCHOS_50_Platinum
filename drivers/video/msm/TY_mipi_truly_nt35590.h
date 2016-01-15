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
 */

#ifndef MIPI_NT35590_H
#define MIPI_NT35590_H

/* #define MIPI_TRULY_FAKE_PANEL */	/* FAKE PANEL for test */

int mipi_nt35590_device_register(struct msm_panel_info *pinfo,
		u32 channel, u32 panel);

#endif  /* MIPI_TRULY_H */