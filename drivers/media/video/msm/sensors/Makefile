GCC_VERSION      := $(shell $(CONFIG_SHELL) $(PWD)/scripts/gcc-version.sh $(CROSS_COMPILE)gcc)
EXTRA_CFLAGS += -Idrivers/media/video/msm
EXTRA_CFLAGS += -Idrivers/media/video/msm/io
EXTRA_CFLAGS += -Idrivers/media/video/msm/eeprom
EXTRA_CFLAGS += -Idrivers/media/video/msm/csi
obj-$(CONFIG_MSM_CAMERA_SENSOR) += msm_sensor.o
obj-$(CONFIG_OV5647) += ov5647_v4l2.o
obj-$(CONFIG_OV8825) += ov8825_v4l2.o
obj-$(CONFIG_AR0543) += ar0543_v4l2.o
obj-$(CONFIG_AR0542) += ar0542_v4l2.o
obj-$(CONFIG_IMX074) += imx074_v4l2.o
obj-$(CONFIG_S5K3L1YX) += s5k3l1yx.o
obj-$(CONFIG_IMX091) += imx091.o
obj-$(CONFIG_OV2720) += ov2720.o
obj-$(CONFIG_MT9M114) += mt9m114_v4l2.o
obj-$(CONFIG_S5K4E1) += s5k4e1_v4l2.o
obj-$(CONFIG_MT9E013) += mt9e013_v4l2.o
obj-$(CONFIG_WEBCAM_OV9726) += ov9726_v4l2.o
#wuchx delete
#obj-$(CONFIG_OV5647_TRULY_CM6868) += ov5647_truly_cm6868_v4l2.o
ifeq (y,$(findstring y,$(filter TYQ_CAMERA_PIP_SUPPORT=%,$(TY_CP_OPTION))))
obj-y += ov5648_ov7695_PIP.o ov5648_truly_cm8352_v4l2_PIP.o
obj-y += ov7695_v4l2_PIP.o
else
ifeq (y,$(findstring y,$(filter TYQ_CAMERA_OV5647_SUPPORT=%,$(TY_CP_OPTION))))
obj-y += ov5647_truly_cm6868_v4l2.o
endif
ifeq (y,$(findstring y,$(filter TYQ_CAMERA_OV5648_SUPPORT=%,$(TY_CP_OPTION))))
obj-y += ov5648_truly_cm8352_v4l2.o
endif
ifeq (y,$(findstring y,$(filter TYQ_CAMERA_OV8830_SUPPORT=%,$(TY_CP_OPTION))))
obj-y += ov8830_v4l2.o
endif
ifeq (y,$(findstring y,$(filter TYQ_CAMERA_OV7695_SUPPORT=%,$(TY_CP_OPTION))))
obj-y += ov7695_v4l2.o
endif
ifeq (y,$(findstring y,$(filter TYQ_CAMERA_OV7695_RAW_SUPPORT=%,$(TY_CP_OPTION))))
obj-y += ov7695_raw_v4l2.o
endif
ifeq (y,$(findstring y,$(filter TYQ_CAMERA_OV2675_SUPPORT=%,$(TY_CP_OPTION))))
obj-y += ov2675_v4l2.o
endif
endif
obj-$(CONFIG_OV8825) += ov7692_v4l2.o
obj-$(CONFIG_VX6953) += vx6953.o  
#wuchx delete
#obj-$(CONFIG_OV5648_TRULY_CM8352) += ov5648_truly_cm8352_v4l2.o
obj-$(CONFIG_OV7692) += ov7692_v4l2.o
#wuchx delete
#obj-$(CONFIG_OV7695) += ov7695_v4l2.o
#obj-$(CONFIG_OV7695_RAW) += ov7695_raw_v4l2.o
obj-$(CONFIG_MT9V115) += mt9v115_v4l2.o
obj-$(CONFIG_VX6953) += vx6953.o
obj-$(CONFIG_A8140) += a8140_v4l2.o
obj-$(CONFIG_S5K3H2_SUNNY_Q8S02E) += s5k3h2_sunny_q8s02e_v4l2.o
