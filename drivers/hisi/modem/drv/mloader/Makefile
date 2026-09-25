ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
ifeq ($(strip $(CFG_CONFIG_TRUSTZONE_HM)),YES)
EXTRA_CFLAGS += -Idrivers/tzdriver_hm \
				-Idrivers/hisi/tzdriver_hm
else
EXTRA_CFLAGS += -Idrivers/tzdriver \
				-Idrivers/hisi/tzdriver
endif
EXTRA_CFLAGS += -Iscripts/dtc/libfdt

obj-y		    += mloader_comm.o
obj-y			+= mloader_load_ccore_imgs.o
obj-y			+= mloader_load_dts.o
obj-y			+= mloader_load_image.o
obj-y			+= mloader_load_lr.o
obj-y			+= mloader_load_patch.o
obj-y		    += mloader_main.o
obj-y		    += mloader_sec_call.o
obj-y           += modem_cold_patch_stub.o

subdir-ccflags-y := -Werror
