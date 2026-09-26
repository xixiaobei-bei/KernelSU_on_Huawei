-include $(BALONG_TOPDIR)/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
ifneq ($(CFG_MODEM_FUSION_VERSION),true)
obj-y += gu/ tl/
else
obj-y += gu/ tl/ gu/psp/
endif
