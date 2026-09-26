#
# Makefile for the memory technology device drivers.
#

# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk


ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y               += comm/
obj-y               += mdmap/
obj-y               += core/
obj-y               += mdmcp/
ifeq ($(strip $(CFG_ENABLE_BUILD_DUMP_MDM_LPM3)),YES)
obj-y               += mdmm3/
endif
ifeq ($(strip $(CFG_ENABLE_BUILD_NRRDR)),YES)
obj-y               += mdmnr/
endif
obj-y               += apr/
endif

KBUILD_CPPFLAGS += -I$(srctree)/drivers/hisi/modem/drv/dump/comm\
                   -I$(srctree)/drivers/hisi/modem/drv/dump/mdmap\
                   -I$(srctree)/drivers/hisi/modem/drv/dump/core\
                   -I$(srctree)/drivers/hisi/modem/drv/dump/mdmcp\
                   -I$(srctree)/drivers/hisi/modem/drv/dump/mdmnr\
                   -I$(srctree)/drivers/hisi/modem/drv/dump/apr\
                   -I$(srctree)/drivers/hisi/modem/drv/dump/mdmm3\
                   -I$(srctree)/drivers/hisi/modem/include/adrv \


