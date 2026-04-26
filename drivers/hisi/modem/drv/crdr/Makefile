#
# Makefile for the memory technology device drivers.
#

# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk


ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
obj-y               += mdmap/
obj-y               += core/
obj-y               += mdmcp/
ifeq ($(strip $(CFG_ENABLE_BUILD_NRRDR)),YES)
obj-y               += mdmnr/
endif
endif

KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/crdr/comm\
                   -Idrivers/hisi/modem/drv/crdr/mdmap\
                   -Idrivers/hisi/modem/drv/crdr/core\
                   -Idrivers/hisi/modem/drv/crdr/mdmcp\
                   -Idrivers/hisi/modem/drv/crdr/mdmnr\
                   -Idrivers/hisi/modem/include/adrv\

subdir-ccflags-y += -Werror

