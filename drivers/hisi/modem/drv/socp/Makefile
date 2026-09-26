
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
#KBUILD_CPPFLAGS += -Idrivers/hisi/modem/drv/socp
KBUILD_CPPFLAGS += -I$(srctree)/drivers/hisi/modem/drv/socp/socp_core/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/hisi/modem/drv/socp/socp_debug/
KBUILD_CPPFLAGS += -I$(srctree)/drivers/hisi/modem/drv/socp/socp_hal/socp_common/
ifeq ($(strip $(CFG_SOCP_V300)),YES)
KBUILD_CPPFLAGS += -I$(srctree)/drivers/hisi/modem/drv/socp/socp_hal/socp_v300/
else
KBUILD_CPPFLAGS += -I$(srctree)/drivers/hisi/modem/drv/socp/socp_hal/socp_v200/
endif

ifeq ($(strip $(CFG_SOCP_V300)),YES)
obj-y += socp_hal/socp_v300/socp_hal_v300.o
else
obj-y += socp_hal/socp_v200/socp_hal_v200.o
endif
obj-y += socp_hal/socp_common/socp_hal_common.o
obj-y += socp_core/socp_enc_src.o
obj-y += socp_core/socp_enc_dst.o
obj-y += socp_core/socp_init.o
obj-y += socp_core/socp_int_manager.o
obj-y += socp_core/socp_mode_manager.o
obj-y += socp_debug/socp_debug.o

ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
obj-y				+= soft_decode/
endif


subdir-ccflags-y := -Werror
