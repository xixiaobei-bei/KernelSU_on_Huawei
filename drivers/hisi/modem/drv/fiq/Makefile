-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk


ifeq ($(strip $(CFG_CONFIG_CCPU_FIQ_SMP)),YES)
obj-y			+= fiq_smp.o
else
obj-y			+= fiq.o
endif

ifeq ($(strip $(CFG_CONFIG_HAS_NRCCPU)),YES)
obj-y			+= fiq_smp_5g.o
endif
subdir-ccflags-y := -Werror

