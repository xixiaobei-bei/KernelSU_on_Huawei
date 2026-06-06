# balong spe module
subdir-ccflags-y := -Werror
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

obj-y						:= espe_sysfs.o
obj-y						+= espe_sysfs_main.o

ifeq ($(strip $(CFG_CONFIG_ESPE_DIRECT_FW)),YES)
obj-y						+= espe_direct_fw.o
endif


ifeq ($(strip $(CFG_CONFIG_BALONG_ESPE_XFRM)),YES)
obj-y						+= espe_xfrm.o
endif


ifeq ($(strip $(ESPE_LLT_MDRV)),y)
obj-y						+= espe_nic.o
obj-y						+= espe_nic_pkt.o
obj-y						+= espe_ncm.o
obj-y						+= espe_nic_ncm.o
obj-y						+= espe_nic_table.o
endif


