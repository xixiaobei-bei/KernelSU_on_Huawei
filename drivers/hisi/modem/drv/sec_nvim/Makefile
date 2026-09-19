ifeq ($(strip $(llt_gcov)),y)
GCOV_PROFILE := y
endif

-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/om/dump

subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/product/

obj-$(CONFIG_NVIM)         += nv_ctrl.o nv_comm.o nv_base.o nv_emmc.o nv_debug.o nv_index.o nv_partition_upgrade.o nv_partition_img.o nv_partition_bakup.o nv_partition_factory.o nv_factory_check.o nv_msg.o nv_proc.o nv_verify.o

subdir-ccflags-y := -Werror

