ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif
subdir-ccflags-y := -Werror
 -include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

obj-y += hmi_core.o hmi_chan_adp.o

ifeq ($(strip $(CFG_CONFIG_HMI_ICC)),YES)
obj-y += hmi_icc.o
endif

ifeq ($(strip $(CFG_CONFIG_HMI_EICC)),YES)
obj-y += hmi_icc.o
endif

ifeq ($(strip $(CFG_CONFIG_HMI_DEBUG)),YES)
obj-y += hmi_debug.o
endif
