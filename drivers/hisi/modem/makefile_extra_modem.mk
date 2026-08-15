-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk
subdir-ccflags-y += -I$(srctree)/drivers/hisi/tzdriver/libhwsecurec/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/platform/ccore/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/platform/dsp/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/platform/acore/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/platform/acore/$(CFG_PLATFORM)/drv/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/platform/common/$(CFG_PLATFORM)/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/platform/common/$(CFG_PLATFORM)/soc/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/adrv/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/drv/acore/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/drv/common/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/drv/common/include/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config

ifeq ($(strip $(OBB_LLT_MDRV)),y)
subdir-ccflags-y += -DDRV_BUILD_LLT
endif
ifeq ($(LLT_BUILD_GCOV), y)
subdir-ccflags-y += -DDRV_BUILD_GCOV
endif

obj-$(CONFIG_HISI_BALONG_MODEM) +=drv/

ifneq ($(chip_type),es)
KBUILD_CFLAGS += -DCHIP_TYPE_CS=FEATURE_ON
endif
KBUILD_CFLAGS += -DMODULE_EXPORTED=




ifeq ($(strip $(CONFIG_HISI_BALONG_MODEM)),m)

BALONG_MODEM_MODULE := balong_modem
$(BALONG_MODEM_MODULE)-y :=

obj-y	+= modem_symbol.o

# overwrite obj-m
obj-m	:= $(BALONG_MODEM_MODULE).o

subdir-ccflags-y += -DCONFIG_HISI_BALONG_MODEM -Wno-unused-function -Wno-undef

# drv
include $(srctree)/drivers/hisi/modem/drv/Makefile
$(BALONG_MODEM_MODULE)-y += $(addprefix drv/, $(drv-y))
obj-y	+= $(addprefix drv/, $(drv-builtin))

ifneq ($(OBB_SEPARATE),true)
# taf
-include $(srctree)/drivers/hisi/modem/tafnew/Kbuild
$(BALONG_MODEM_MODULE)-y += $(addprefix tafnew/, $(taf-objs))
endif

endif
