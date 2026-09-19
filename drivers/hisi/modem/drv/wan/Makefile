ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
obj-y	+= ipf_ap.o  ipf_ap_test.o
obj-y	+= wan.o wan_test.o
