#
# Makefile for the memory technology device drivers.
#
# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk


ifeq ($(strip $(CFG_DIAG_SYSTEM_FUSION)),YES)
obj-y				+= scm_fusion_chan_manager.o
obj-y				+= scm_fusion_common.o
else
obj-y				+= scm_ind_dst.o 
obj-y				+= scm_ind_src.o 
obj-y				+= scm_cnf_dst.o 
obj-y				+= scm_cnf_src.o 
obj-y				+= scm_common.o 
obj-y				+= scm_debug.o 
obj-y				+= scm_init.o 
endif
obj-y				+= scm_rate_ctrl.o 




