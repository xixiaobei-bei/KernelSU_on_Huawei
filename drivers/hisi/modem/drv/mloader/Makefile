ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
EXTRA_CFLAGS += -Idrivers/tzdriver_hm \
				-Idrivers/hisi/tzdriver_hm \
				-Iscripts/dtc/libfdt

obj-y		    += mloader_comm.o
obj-y			+= mloader_load_image.o

obj-y		    += mloader_load_image_unsec.o

obj-y           += modem_cold_patch_stub.o

subdir-ccflags-y := -Werror
