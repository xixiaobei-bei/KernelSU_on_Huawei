ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
obj-$(CONFIG_BALONG_MODEM_RESET)        += reset_balong.o

ifeq ($(strip $(CFG_CONFIG_BALONG_MODEM_RESET_CTRL)),YES)
obj-y 		  += reset_balong_ctrl.o
endif

EXTRA_CFLAGS += -Idrivers/hisi/tzdriver \
                -Idrivers/tzdriver

subdir-ccflags-y := -Werror
