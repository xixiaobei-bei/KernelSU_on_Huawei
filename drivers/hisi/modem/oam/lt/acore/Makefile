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
subdir-ccflags-y  += -I$(MODEM_DIR)/include/tools/
subdir-ccflags-y  += -I$(MODEM_DIR)/include/tag/apss/

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

#lt only INC
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/acore/diag/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/acore/chr/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/acore/service/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/comm/list/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/comm/diag/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/comm/debug/
subdir-ccflags-y  += -I$(MODEM_DIR)/oam/inc/lt/comm/frame/

$(warning fuxin start)
$(warning $(subdir-ccflags))
$(warning fuxin end)
#taf
ifeq ($(CFG_NAS_NEW_ARCH), YES)
subdir-ccflags-y  += -I$(MODEM_DIR)/tafnew/acore/at/inc/
else
subdir-ccflags-y  += -I$(MODEM_DIR)/taf/comm/src/acore/at/inc/
subdir-ccflags-y  += -I$(MODEM_DIR)/taf/lt/inc/acore/at/
endif

subdir-ccflags-y  +=  -DVOS_LINUX=4 -DOSA_CPU_ACPU=1 -DVOS_OS_VER=VOS_LINUX -DVOS_OSA_CPU=OSA_CPU_ACPU -D__LOG_RELEASE__ -DBSP_CORE_APP -DGU_MODE -DMSP_SUPPORT_USB_VCOM1 -DMSP_SUPPORT_USB_VCOM2 -DVOS_DOPRA_VER=2 -DNDIS_RELEASE -DBALONG_PHY_V200=2 -DBALONG_PHY_V300=3 -DBALONG_PHY_VER=BALONG_PHY_V300 -DBALONG_CHIP_V200=2 -DBALONG_CHIP_V300=3 -DBALONG_CHIP_V500=4 -DBALONG_CHIP_VER=BALONG_CHIP_V500 -DMSP_GUNAS_AT_UNITE

ifneq ($(TARGET_ARM_TYPE), arm64)
subdir-ccflags-y  += -D__LINUX_ARM_ARCH__=7 -march=armv7-a -Uarm
endif

ifneq ($(CFG_RAT_MODE),RAT_GU)
subdir-ccflags-y  += -DPRODUCT_CFG_MULTIMODE_LTE -DHAL_DIAG_SDM_USE
endif

subdir-ccflags-y  += -fsigned-char 

ifeq ($(strip $(CONFIG_HISI_BALONG_MODEM)),m)
tlmsp-y 			:=
tlmsp-y 			+= chr/chr_om.o
tlmsp-y 			+= chr/chr_cfg.o
tlmsp-y 			+= debug/msp_debug.o
tlmsp-y 			+= list/blist.o
tlmsp-y				+= diag/diag_acore_common.o
tlmsp-y				+= diag/diag_api.o
tlmsp-y				+= diag/diag_cfg.o
tlmsp-y				+= diag/diag_debug.o
tlmsp-y				+= diag/diag_message.o
tlmsp-y				+= diag/diag_msgapplog.o
tlmsp-y				+= diag/diag_msgbsp.o
tlmsp-y				+= diag/diag_msghifi.o
tlmsp-y				+= diag/diag_msgmsp.o
tlmsp-y				+= diag/diag_msg_easyrf.o
tlmsp-y				+= diag/diag_msgphy.o
tlmsp-y				+= diag/diag_msgps.o
tlmsp-y				+= diag/diag_service.o
tlmsp-y				+= diag/diag_connect.o
tlmsp-y				+= diag/diag_msglrm.o
ifeq ($(strip $(CFG_DIAG_SYSTEM_5G)),YES)
tlmsp-y				+= diag/diag_msgnrm.o
tlmsp-y				+= diag/diag_time_stamp.o
endif
ifeq ($(strip $(CFG_CONFIG_FILEBROSWER)),YES)
tlmsp-y				+= diag/diag_fs_proc.o
endif

ifeq ($(strip $(CFG_CONFIG_BBIT_SOCP_NRM)),YES)
tlmsp-y				+= diag/diag_msgbbp_nr_fpga.o
else
tlmsp-y				+= diag/diag_msgbbp.o
endif

tlmsp-y				+= service/msp_service.o

else

obj-y				+= chr/
obj-y				+= debug/
obj-y				+= list/
obj-y				+= diag/
obj-y				+= service/
endif

subdir-ccflags-y += -Werror
