/*
 *  thermal.c - Generic Thermal Management Sysfs support.
 *
 *  Copyright (C) 2008 Intel Corp
 *  Copyright (C) 2008 Zhang Rui <rui.zhang@intel.com>
 *  Copyright (C) 2008 Sujith Thomas <sujith.thomas@intel.com>
 *
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>
#include <linux/idr.h>
#include <linux/thermal.h>
#include <linux/spinlock.h>
#include <linux/reboot.h>
#include <net/netlink.h>
#include <net/genetlink.h>

MODULE_AUTHOR("Tony Chen <chenjp@k-touch.cn>");
MODULE_DESCRIPTION("thermal driver");
MODULE_LICENSE("GPL");

int temperature;

static ssize_t temp_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf )
{
	return sprintf(buf, "%d\n", temperature);
}

static ssize_t temp_store(struct kobject *kobj, struct kobj_attribute *attr, const char* buf, size_t n)
{
	if (!sscanf(buf, "%d", &temperature))
		return -EINVAL;

	return n;
}

static struct kobj_attribute thermal_attr = {
	.attr = {
		.name = "temp",
		.mode = 0666,
	},
	.show = &temp_show,
	.store = &temp_store,
};

static struct attribute *thermal_attrs[] = {
	&thermal_attr.attr,
	NULL,
};

static struct attribute_group thermal_attr_group = {
	.attrs = thermal_attrs,
};

struct kobject *thermal_kobj;

static int __init thermal_init(void)
{
	printk(KERN_INFO "thermal_init\n");
	thermal_kobj = kobject_create_and_add("thermal", NULL);
	if(!thermal_kobj)
	{
		printk(KERN_INFO "kobject don't create \n");
		return -ENOMEM;
	}
	return  sysfs_create_group(thermal_kobj, &thermal_attr_group);
}

static void __exit thermal_exit(void)
{
	printk(KERN_INFO "thermal_exit\n");
	sysfs_remove_group(thermal_kobj, &thermal_attr_group);	
	kobject_del(thermal_kobj);
}

module_init(thermal_init);
module_exit(thermal_exit);
