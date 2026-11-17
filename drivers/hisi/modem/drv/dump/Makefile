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
ifeq ($(strip $(CFG_ENABLE_BUILD_NRRDR)),YES)
obj-y               += mdmnr/
endif
obj-y               += apr/
endif

KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/dump/comm\
                   -Idrivers/hisi/modem/drv/dump/mdmap\
                   -Idrivers/hisi/modem/drv/dump/core\
                   -Idrivers/hisi/modem/drv/dump/mdmcp\
                   -Idrivers/hisi/modem/drv/dump/mdmnr\
                   -Idrivers/hisi/modem/drv/dump/apr\


