# Makefile for the DSM_RECOVERY drivers.
#
# Each configuration option enables a list of files.

obj-$(CONFIG_HUAWEI_DSM)  += dsm_recovery.o
obj-$(CONFIG_HUAWEI_DSM)  += recovery_misc.o

EXTRA_CFLAGS := -I$(srctree)/lib/libc_sec/securec_v2/include
