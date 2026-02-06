#
# Makefile for the memory technology device drivers.
#
# Core functionality.
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

#A+B CP
ifeq ($(strip $(CFG_DIAG_SYSTEM_A_PLUS_B_CP)),YES)
obj-y				+= OmPortSwitch.o
obj-y				+= OmCpPcdevPpm.o
obj-y				+= OmCommonPpm.o 
else
#A+B AP
ifeq ($(strip $(CFG_DIAG_SYSTEM_A_PLUS_B_AP)),YES)
obj-y				+= OmPortSwitch.o
obj-y				+= OmCpAgentPpm.o
obj-y				+= OmCommonPpm.o 
obj-y				+= OmSocketPpm.o 
obj-y				+= OmUsbPpm.o 
obj-y				+= OmVcomPpm.o 
else
#SOC
obj-y				+= OmPortSwitch.o
obj-y				+= OmCommonPpm.o 
obj-y				+= OmSocketPpm.o 
obj-y				+= OmUsbPpm.o 
obj-y				+= OmVcomPpm.o 
endif
endif


