#gcov
ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
GCOV_PROFILE_socp_balong_test.o := n
GCOV_PROFILE_socp_api_test.o := n
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk
ifeq ($(strip $(CFG_DIAG_SYSTEM_5G)),YES)
obj-y       += socp_enc.o
else
obj-y       += socp_balong.o
endif
obj-y       += socp_ind_delay.o socp_debug.o
#socp_balong_test.o socp_balong_test_llt.o 

ifeq ($(strip $(CFG_CONFIG_DEFLATE)),YES)
obj-y      += deflate.o  deflate_test.o
endif

subdir-ccflags-y := -Werror
