
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

subdir-ccflags-y := -Werror

ifeq ($(strip $(CFG_CONFIG_TRUSTZONE_HM)),YES)
subdir-ccflags-y += 	-Idrivers/tzdriver_hm \
			-Idrivers/hisi/tzdriver_hm
else
subdir-ccflags-y +=	-Idrivers/tzdriver \
			-Idrivers/hisi/tzdriver
endif

ifeq ($(strip $(CFG_CONFIG_MLOADER)),YES)
obj-y	+= mloader_comm.o
obj-y	+= mloader_load_ccore_imgs.o
obj-y	+= mloader_load_dts.o
obj-y	+= mloader_load_image.o
obj-y	+= mloader_load_lr.o
obj-y   += mloader_load_lpmcu.o
obj-y	+= mloader_load_patch.o
obj-y	+= mloader_main.o
obj-y	+= mloader_sec_call.o
obj-y	+= mloader_debug.o
obj-y	+= mloader_msg.o
obj-y	+= mloader_msg_icc.o
endif
obj-y	+= mloader_load_patch.o
