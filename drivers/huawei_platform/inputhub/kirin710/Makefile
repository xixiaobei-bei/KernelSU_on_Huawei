#ifdef CONFIG_CONTEXTHUB_SHMEM
ccflags-y  += -I$(srctree)/drivers/contexthub/
subdir-ccflags-y += -I$(srctree)/drivers/contexthub/
#endif

#ifdef CONFIG_HUAWEI_HOST_TS_KIT
ccflags-y  += -I$(srctree)/drivers/devkit/tpkit/hostprocessing/
subdir-ccflags-y += -I$(srctree)/drivers/devkit/tpkit/hostprocessing/
#endif

subdir-ccflags-y += -I$(src)/
subdir-ccflags-y += -I$(src)/als/
subdir-ccflags-y += -I$(src)/cap_prox/
subdir-ccflags-y += -I$(src)/handpress/
subdir-ccflags-y += -I$(src)/motion/
subdir-ccflags-y += -I$(src)/vibrator/

# contexthub Driver
obj-$(CONFIG_INPUTHUB_20)		+= contexthub_boot.o
obj-$(CONFIG_INPUTHUB_20)		+= contexthub_route.o
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
obj-$(CONFIG_INPUTHUB_20)  += ca_channel.o
obj-$(CONFIG_INPUTHUB_20)  += fingerprint_channel.o
obj-$(CONFIG_INPUTHUB_20)  += contexthub_ext_log.o
obj-$(CONFIG_INPUTHUB_VIBRATOR_CHANNEL)  += vibrator_channel.o
obj-$(CONFIG_INPUTHUB_20)  += fingerprint_ud_channel.o
obj-$(CONFIG_INPUTHUB_20)  += big_data_channel.o
obj-$(CONFIG_INPUTHUB_20) += als/
obj-$(CONFIG_INPUTHUB_20) += cap_prox/
obj-$(CONFIG_INPUTHUB_20) += handpress/
obj-$(CONFIG_INPUTHUB_20) += motion/
obj-$(CONFIG_INPUTHUB_20) += vibrator/
