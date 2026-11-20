COMILE_BY_MODULE := online

ifeq ($(COMILE_BY_MODULE),online)
CONFIG_HWSDP := y
endif

obj-$(CONFIG_HWSDP) += src/hwsdp_ioctl.o src/hwsdp_ioctl_teec.o

EXTRA_CFLAGS := -I../../include \
				-I. \
				-Iinc \
				-Idrivers/hisi/tzdriver \
				-I$(srctree)/lib/libc_sec/securec_v2/include
