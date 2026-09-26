#ifdef CONFIG_CONTEXTHUB_SHMEM
ccflags-y  += -I$(srctree)/drivers/contexthub/
subdir-ccflags-y += -I$(srctree)/drivers/contexthub/
#endif

#ifdef CONFIG_HUAWEI_HOST_TS_KIT
ccflags-y  += -I$(srctree)/drivers/devkit/tpkit/hostprocessing/
subdir-ccflags-y += -I$(srctree)/drivers/devkit/tpkit/hostprocessing/
#endif

ifeq ($(TARGET_BOARD_PLATFORM),baltimore)
EXTRA_CFLAGS += -Idrivers/video/fbdev/hisi/dss/effect/
EXTRA_CFLAGS += -Idrivers/video/fbdev/hisi/dss/
EXTRA_CFLAGS += -Idrivers/huawei_platform/blpwm/
EXTRA_CFLAGS += -DDSS_ALSC_NOISE
EXTRA_CFLAGS += -DCONFIG_HISI_FB_ALSC
subdir-ccflags-y += -Idrivers/video/fbdev/hisi/dss/effect/
subdir-ccflags-y += -Idrivers/video/fbdev/hisi/dss/
subdir-ccflags-y += -DDSS_ALSC_NOISE
subdir-ccflags-y += -DCONFIG_HISI_FB_ALSC
endif

subdir-ccflags-y += -I$(src)/
subdir-ccflags-y += -I$(src)/acc/
subdir-ccflags-y += -I$(src)/airpress/
subdir-ccflags-y += -I$(src)/als/
subdir-ccflags-y += -I$(src)/cap_prox/
subdir-ccflags-y += -I$(src)/gyro/
subdir-ccflags-y += -I$(src)/handpress/
subdir-ccflags-y += -I$(src)/mag/
subdir-ccflags-y += -I$(src)/motion/
subdir-ccflags-y += -I$(src)/ps/
subdir-ccflags-y += -I$(src)/therm/
subdir-ccflags-y += -I$(src)/vibrator/

# contexthub Driver
obj-$(CONFIG_INPUTHUB_20)		+= contexthub_boot.o
obj-$(CONFIG_INPUTHUB_20)		+= contexthub_route.o
obj-$(CONFIG_INPUTHUB_20)		+= contexthub_route_plu.o
obj-$(CONFIG_INPUTHUB_20)		+= sensor_config.o
obj-$(CONFIG_INPUTHUB_20)       += contexthub_logbuff.o
obj-$(CONFIG_INPUTHUB_20)       += contexthub_debug.o
obj-$(CONFIG_INPUTHUB_20)	+= sensor_channel.o
obj-$(CONFIG_INPUTHUB_20)  += motion_channel.o
obj-$(CONFIG_INPUTHUB_20)  += sensor_detect.o
obj-$(CONFIG_INPUTHUB_20)  += sensor_sysfs.o
obj-$(CONFIG_INPUTHUB_20)  += sensor_info.o
obj-$(CONFIG_INPUTHUB_20)  += sensor_feima.o
obj-$(CONFIG_INPUTHUB_20)  += contexthub_recovery.o
obj-$(CONFIG_INPUTHUB_20)  += contexthub_pm.o
obj-$(CONFIG_INPUTHUB_20)  += fingerprint_channel.o
obj-$(CONFIG_INPUTHUB_20)  += contexthub_ext_log.o
obj-$(CONFIG_INPUTHUB_20)  += fingerprint_ud_channel.o
obj-$(CONFIG_INPUTHUB_20)  += vibrator_channel.o
obj-$(CONFIG_INPUTHUB_20)  += big_data_channel.o
obj-$(CONFIG_INPUTHUB_20)  += kbhub_channel.o
obj-$(CONFIG_INPUTHUB_20)  += modem_route.o
obj-$(CONFIG_INPUTHUB_20)  += thp_channel.o
obj-$(CONFIG_INPUTHUB_20) += acc/
obj-$(CONFIG_INPUTHUB_20) += airpress/
obj-$(CONFIG_INPUTHUB_20) += als/
obj-$(CONFIG_INPUTHUB_20) += cap_prox/
obj-$(CONFIG_INPUTHUB_20) += gyro/
obj-$(CONFIG_INPUTHUB_20) += handpress/
obj-$(CONFIG_INPUTHUB_20) += mag/
obj-$(CONFIG_INPUTHUB_20) += motion/
obj-$(CONFIG_INPUTHUB_20) += ps/
obj-$(CONFIG_INPUTHUB_20) += therm/
obj-$(CONFIG_INPUTHUB_20) += vibrator/
