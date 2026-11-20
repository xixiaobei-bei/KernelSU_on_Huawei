#
# Makefile for the modem drivers.
#
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk


MODEM_DIR := $(srctree)/drivers/hisi/modem/
-include $(MODEM_DIR)/config/product/$(OBB_PRODUCT_NAME)/config/balong_product_config.mk

ifeq ($(TARGET_ARM_TYPE), arm64)
subdir-ccflags-y  += -Iarch/arm64/include/asm
else
subdir-ccflags-y  += -Iarch/arm/include/asm
endif

subdir-ccflags-y  += -Iinclude/linux
subdir-ccflags-y  += -Iinclude/generated
subdir-ccflags-y  += -I$(MODEM_DIR)/config/osa/
subdir-ccflags-y  += -I$(MODEM_DIR)/config/errlog/
subdir-ccflags-y  += -I$(MODEM_DIR)/config/product/$(OBB_PRODUCT_NAME)/config/
subdir-ccflags-y  += -I$(MODEM_DIR)/config/product/$(OBB_PRODUCT_NAME)/include/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/adrv
subdir-ccflags-y  += -I$(MODEM_DIR)/include/drv/common/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/drv/acore/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/med/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/comm/om/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/comm/errno/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/comm/nvim/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/gu/comm/osa/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/gu/comm/si/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/gu/comm/usimm/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/gu/log/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/gu/comm/om/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/gu/nvim/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/gu/hpa/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/lt/acore/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/oam/lt/common/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/phy/wphy/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/ps/gups/acore/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/ps/gups/ccore/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/ps/gups/common/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/ps/tlps/acore/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/ps/tlps/ccore/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/ps/tlps/common/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/ps/nas/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/taf/acore/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/taf/ccore/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/taf/common/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/nv/acore/guc_as/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/nv/acore/guc_nas/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/nv/common/sys/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/nv/common/pam/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/nv/common/guc_as/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/nv/common/guc_nas/

#inc comm
#subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/comm/comm/nvim/
#subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/comm/comm/nvim/
#subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/comm/ccore/xml/

#inc gu
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/comm/si/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/comm/cbtfun/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/comm/om/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/acore/om/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/acore/cbtcpm/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/acore/cbtppm/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/acore/cbtfun/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/ccore/usimm/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/ccore/hpa/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/ccore/om/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/ccore/pstool/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/ccore/sc/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/gu/ccore/sleep/

#lt only INC
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/acore/diag/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/acore/chr/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/comm/list/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/comm/diag/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/comm/type/


#taf
subdir-ccflags-y  += -I$(MODEM_DIR)/taf/comm/src/acore/at/inc/
subdir-ccflags-y  += -I$(MODEM_DIR)/taf/lt/inc/acore/at/

subdir-ccflags-y  +=  -DVOS_LINUX=4 -DOSA_CPU_ACPU=1 -DVOS_OS_VER=VOS_LINUX -DVOS_OSA_CPU=OSA_CPU_ACPU -D__LOG_RELEASE__ -DBSP_CORE_APP -DGU_MODE -DMSP_SUPPORT_USB_VCOM1 -DMSP_SUPPORT_USB_VCOM2 -DVOS_DOPRA_VER=2 -DNDIS_RELEASE -DBALONG_PHY_V200=2 -DBALONG_PHY_V300=3 -DBALONG_PHY_VER=BALONG_PHY_V300 -DBALONG_CHIP_V200=2 -DBALONG_CHIP_V300=3 -DBALONG_CHIP_V500=4 -DBALONG_CHIP_VER=BALONG_CHIP_V500 -DMSP_GUNAS_AT_UNITE

ifneq ($(TARGET_ARM_TYPE), arm64)
subdir-ccflags-y  += -D__LINUX_ARM_ARCH__=7 -march=armv7-a -Uarm
endif

ifneq ($(CFG_RAT_MODE),RAT_GU)
subdir-ccflags-y  += -DPRODUCT_CFG_MULTIMODE_LTE -DHAL_DIAG_SDM_USE
endif

subdir-ccflags-y  += -fsigned-char 

obj-y				+= chr/
obj-y				+= debug/
obj-y				+= list/
obj-y				+= diag/
obj-y				+= service/

subdir-ccflags-y += -Werror