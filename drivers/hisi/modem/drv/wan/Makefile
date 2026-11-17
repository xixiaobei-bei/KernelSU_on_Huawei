ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
obj-y	+= ipf_ap.o
obj-y	+= wan.o
ifneq ($(strip $(OBB_LLT_MDRV)),y)
#obj-y   += ipf_ap_test.o wan_test.o
endif
