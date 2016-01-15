#
# Makefile for sensor chip drivers.
#

obj-$(CONFIG_THERMAL)		+= thermal_sys.o
#TYDRV:chenjp 2012.11.29 add for thermal sys support
ifeq ($(patsubst TYQ_THERMAL_SYS_SUPPORT=%,%,$(filter TYQ_THERMAL_SYS_SUPPORT%,$(TY_CP_OPTION))),y)
obj-y += tyq_thermal_sys.o
endif
obj-$(CONFIG_THERMAL_MSM_POPMEM)	+= msm_popmem-tm.o
obj-$(CONFIG_THERMAL_TSENS)	+= msm_tsens.o
obj-$(CONFIG_THERMAL_TSENS8960) += msm8960_tsens.o
obj-$(CONFIG_THERMAL_PM8XXX)	+= pm8xxx-tm.o
obj-$(CONFIG_THERMAL_MONITOR)	+= msm_thermal.o
obj-$(CONFIG_SPEAR_THERMAL)		+= spear_thermal.o
