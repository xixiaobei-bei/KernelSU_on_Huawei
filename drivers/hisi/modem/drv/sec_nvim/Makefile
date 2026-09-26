
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk

KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/om/dump \
                   -Idrivers/hisi/modem/include/adrv

subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/product/

ifeq ($(strip $(CFG_CONFIG_NV_FUSION)), YES)
obj-y += fusion/nv_base.o \
         fusion/nv_debug.o \
         fusion/nv_msg.o \
         fusion/nv_msg_req.o \
         fusion/nv_msg_eicc.o \
         fusion/nv_partition.o \
         fusion/nv_partition_image.o \
         fusion/nv_partition_backup.o \
         fusion/nv_partition_factory.o \
         fusion/nv_upgrade.o
else
obj-y += nv_ctrl.o \
         nv_comm.o \
         nv_base.o \
         nv_emmc.o \
         nv_debug.o \
         nv_index.o \
         nv_partition_upgrade.o \
         nv_partition_img.o \
         nv_partition_bakup.o \
         nv_partition_factory.o \
         nv_factory_check.o nv_msg.o \
         nv_proc.o \
         nv_verify.o

ifeq ($(strip $(CFG_CONFIG_NV_FUSION_MSG)),YES)
obj-y += nv_msg_task.o
else
obj-y += nv_msg_icc.o
endif

endif

subdir-ccflags-y := -Werror

