#
# Makefile for DUBAI.
#

obj-$(CONFIG_HUAWEI_DUBAI) += \
	dubai.o \
	dubai_battery_stats.o \
	dubai_cpu_stats.o \
	dubai_display_stats.o \
	dubai_ddr_stats.o \
	dubai_gpu_stats.o \
	dubai_misc_stats.o \
	dubai_peri_stats.o \
	dubai_sr_stats.o \
	cpu_stats/dubai_cpu_configs.o \
	cpu_stats/dubai_proc_decompose.o \
	utils/buffered_log_sender.o \
	utils/dubai_hashtable.o \
	utils/dubai_utils.o

ifeq ($(CONFIG_INPUTHUB_20),y)
obj-y += dubai_sensorhub_stats.o
ccflags-y += -I$(srctree)/drivers/huawei_platform/
endif
