
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

obj-$(CONFIG_HISI_SIM_HOTPLUG_SPMI)     += hisi_sim_hotplug.o
obj-$(CONFIG_HISI_SIM_HOTPLUG_SPMI)     += hisi_sim_hw_mgr.o
obj-$(CONFIG_HISI_SIM_HOTPLUG_SPMI)     += hisi_sim_hw_service.o
ifneq ($(strip $(CFG_CONFIG_SCI_DECP)),YES)
ifneq ($(strip $(CFG_CONFIG_BALONG_MSG)),YES)
obj-$(CONFIG_HISI_SIM_HOTPLUG_SPMI)     += hisi_sim_io_mgr.o
endif
endif

