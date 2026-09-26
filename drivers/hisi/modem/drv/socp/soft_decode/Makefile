#
# Makefile for the memory technology device drivers.
#
# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/tools
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/drv/diag/
KBUILD_CPPFLAGS += -I$(srctree)drivers/hisi/modem/drv/diag/scm \
                   -I$(srctree)drivers/hisi/modem/drv/diag/cpm \
                   -I$(srctree)drivers/hisi/modem/drv/diag/ppm \
                   -I$(srctree)drivers/hisi/modem/drv/diag/debug \
                   -I$(srctree)drivers/hisi/modem/drv/diag/adp \
                   -I$(srctree)drivers/hisi/modem/drv/diag/comm \
                   -I$(srctree)drivers/hisi/modem/drv/diag/report \
                   -I$(srctree)drivers/hisi/modem/drv/diag/serivce \
				   -I$(srctree)drivers/hisi/modem/drv/hds
ifeq ($(strip $(CFG_DIAG_SYSTEM_FUSION)),YES)
KBUILD_CPPFLAGS += -I$(srctree)drivers/hisi/modem/drv/diag/message
endif


obj-y				+= hdlc.o 
obj-y				+= ring_buffer.o 
obj-y				+= soft_decode.o






