ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

KBUILD_CPPFLAGS +=	-Idrivers/hisi/modem/drv/om/common \
                    -Idrivers/hisi/modem/drv/om/dump \
                    -Idrivers/hisi/modem/drv/om/oms\
                    -Idrivers/hisi/modem/drv/om/log \
                    -Idrivers/hisi/modem/drv/om/sys_view \
		    -Idrivers/modem/drv/om/usbtrace

obj-$(CONFIG_HW_IP_BASE_ADDR)	+= hwadp_balong.o hwadp_core.o hwadp_debug.o

ifeq ($(strip $(CFG_CONFIG_MALLOC_UNIFIED)),YES)
obj-y += malloc_interface.o
endif

subdir-ccflags-y := -Werror
