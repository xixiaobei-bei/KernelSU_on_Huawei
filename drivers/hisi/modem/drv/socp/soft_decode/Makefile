#
# Makefile for the memory technology device drivers.
#
# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/tools
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/drv/diag/
KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/diag/scm \
                   -Idrivers/hisi/modem/drv/diag/cpm \
                   -Idrivers/hisi/modem/drv/diag/ppm \
                   -Idrivers/hisi/modem/drv/diag/debug \
                   -Idrivers/hisi/modem/drv/diag/adp \
                   -Idrivers/hisi/modem/drv/diag/comm \
                   -Idrivers/hisi/modem/drv/diag/report \
                   -Idrivers/hisi/modem/drv/diag/serivce \
				   -Idrivers/hisi/modem/drv/hds
ifeq ($(strip $(CFG_DIAG_SYSTEM_FUSION)),YES)
KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/diag/message
endif


obj-y				+= hdlc.o 
obj-y				+= ring_buffer.o 
obj-y				+= soft_decode.o






