obj-$(CONFIG_L3CACHE_SHARE)        += l3cache_share.o
obj-$(CONFIG_L3CACHE_EXTENSION)    += l3cache_extension.o
obj-$(CONFIG_L3CACHE_PARTITION_CTRL)    += l3cache_partition_ctrl.o
obj-$(CONFIG_NAMSTO_FLUSH_CMO)     += namsto_flush_cmo.o
obj-$(CONFIG_NONSHAREABLE)         += nonshareable.o
obj-$(CONFIG_L3CACHE_LP_DEBUG)     += l3cache_lp_debug.o
obj-$(CONFIG_NAMSTO_DEBUG)         += namsto_debug.o

ifeq ($(CONFIG_NAMSTO_DEBUG),y)
EXTRA_CFLAGS += -Idrivers/staging/android/ion
endif
