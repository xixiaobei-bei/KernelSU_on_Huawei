#
# Makefile for the memory technology device drivers.
#

# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

obj-y				+= debug/
obj-y				+= ppm/
obj-y				+= cpm/
obj-y				+= scm/

KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/diag/scm \
                   -Idrivers/hisi/modem/drv/diag/cpm \
                   -Idrivers/hisi/modem/drv/diag/ppm \
				   -Idrivers/hisi/modem/drv/diag/debug \



