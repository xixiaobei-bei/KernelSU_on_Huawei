#
# Makefile for the memory technology device drivers.
#

# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

ifneq ($(strip $(OBB_SEPARATE)),true)
ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
ifeq ($(strip $(CFG_DIAG_SYSTEM_A_PLUS_B_AP)),YES)
obj-y				+= ppm/
obj-y				+= cpm/
obj-y				+= comm/
else
obj-y				+= debug/
obj-y				+= ppm/
obj-y				+= cpm/
obj-y				+= scm/
obj-y				+= comm/
obj-y				+= report/
obj-y				+= serivce/
endif
endif
endif
obj-y				+= adp/

KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/diag/scm \
                   -Idrivers/hisi/modem/drv/diag/cpm \
                   -Idrivers/hisi/modem/drv/diag/ppm \
				   -Idrivers/hisi/modem/drv/diag/debug \
                   -Idrivers/hisi/modem/drv/diag/adp \
                   -Idrivers/hisi/modem/drv/diag/comm \
				   -Idrivers/hisi/modem/drv/diag/report \
				   -Idrivers/hisi/modem/drv/diag/serivce



