#
# Makefile for the modem drivers.
#
-include $(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config/balong_product_config.mk


ifeq ($(strip $(CONFIG_HISI_BALONG_MODEM)),m)
drv-y :=
drv-builtin :=

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
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/tl/drv/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/product/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/tl/oam/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/tl/lps/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/acore/sys/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/acore/drv/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/acore/msp/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/acore/pam/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/acore/guc_as/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/acore/guc_nas/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/common/sys/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/common/drv/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/common/msp/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/common/pam/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/common/guc_as/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/common/guc_nas/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/nv/common/tl_as
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/phy/lphy/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/config/nvim/include/gu/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/taf/common/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/include/taf/acore/
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/config/product/$(OBB_PRODUCT_NAME)/$(OBB_MODEM_CUST_CONFIG_DIR)/config
subdir-ccflags-y += -I$(srctree)/drivers/hisi/modem/platform/common/

subdir-ccflags-y+= -Idrivers/hisi/modem/drv/icc
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/rtc
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/nvim
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/mem
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/om \
                   -Idrivers/hisi/modem/drv/om/common \
                   -Idrivers/hisi/modem/drv/om/dump \
                   -Idrivers/hisi/modem/drv/om/log
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/udi
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/balong_timer
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/hds
subdir-ccflags-y+= -Idrivers/usb/gadget
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/diag/scm \
                   -Idrivers/hisi/modem/drv/diag/cpm \
                   -Idrivers/hisi/modem/drv/diag/debug \
                   -Idrivers/hisi/modem/drv/diag/ppm \
                   -Idrivers/hisi/modem/drv/diag/soft_decode
subdir-ccflags-y+= -Idrivers/hisi/modem/include/tools
subdir-ccflags-y += -DMODULE_EXTRA_MODEM


drv-y += sysboot/sysboot_balong.o

ifeq ($(strip $(CFG_CONFIG_RFS_SERVER)),YES)
drv-y += rfs_server/rfs_server.o
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_DEV)),YES)
drv-y += pcie_balong_dev/pcie_balong_dev.o
drv-y += pcie_boot_dev/pcie_boot_dev.o
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_DEV_TEST)),YES)
drv-y += pcie_balong_dev/pcie_balong_dev_test.o
endif

ifeq ($(strip $(CFG_CONFIG_UART_BOOT)),YES)
drv-y += modem_boot/uart_boot_dev.o
endif

ifeq ($(strip $(CFG_CONFIG_MODEM_BOOT)),YES)
drv-y += modem_boot/modem_load.o
drv-y += modem_boot/modem_boot.o
endif

drv-y += s_memory/s_memory.o
drv-y += icc/icc_core.o icc/icc_linux.o icc/icc_debug.o

ifeq ($(strip $(CFG_CONFIG_VCOM_NODE)),YES)
drv-y += vcom/vcom.o
drv-y += vcom/vcom_balong_power.o
endif
ifeq ($(strip $(CFG_CONFIG_VCOM_AGENT)),YES)
drv-y += vcom/vcom_agent.o
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_CDEV_RC)),YES)
drv-y += pfunc/pcie_cdev.o
drv-y += pfunc/pcie_cdev_rc.o
drv-y += pfunc/pcie_cdev_desc.o
drv-y += pfunc/pcie_cdev_dbg.o
drv-y += pfunc/pcie_cdev_test.o
drv-y += pfunc/hisi_adp_pcdev_rc.o
drv-y += pfunc/pcie_usb_relay.o
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_ETH_RC)),YES)
drv-y += peth/pcie_nic.o
drv-y += peth/pcie_nic_rc.o
drv-y += peth/pcie_nic_desc.o
drv-y += peth/pcie_nic_dbg.o
	ifeq ($(CFG_EXTRA_MODEM_MODE),FEATURE_ON)
	drv-y      += peth/mdrv_remote_eth.o
	endif
endif

drv-y           += adp/adp_socp.o
drv-y           += adp/adp_usb.o


ifeq ($(strip $(CFG_CONFIG_BALONG_MODEM_GLOBAL_RESET)),YES)
drv-y += reset/reset_balong_global.o
endif
ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/diag/scm \
                   -Idrivers/hisi/modem/drv/diag/cpm \
                   -Idrivers/hisi/modem/drv/diag/ppm \
                   -Idrivers/hisi/modem/drv/diag/debug \
                   -Idrivers/hisi/modem/drv/diag/comm \
                   -Idrivers/hisi/modem/drv/diag/serivce \
                   -Idrivers/hisi/modem/drv/diag/report \
                   -Idrivers/hisi/modem/drv/socp/soft_decode
subdir-ccflags-y+= -Idrivers/hisi/modem/include/tools

drv-y           += diag/cpm/diag_port_manager.o
drv-y           += diag/debug/diag_system_debug.o
drv-y           += diag/ppm/OmCommonPpm.o
drv-y           += diag/ppm/OmSocketPpm.o
drv-y           += diag/ppm/OmUsbPpm.o
drv-y           += diag/ppm/OmVcomPpm.o
drv-y           += diag/comm/diag_comm.o
drv-y           += diag/ppm/OmCpAgentPpm.o
drv-y           += diag/ppm/OmPortSwitch.o
drv-y           += socp/soft_decode/ring_buffer.o
endif

ifeq ($(strip $(CFG_CONFIG_DIAG_NETLINK)),YES)
drv-y           += diag_vcom/diag_vcom_main.o
drv-y           += diag_vcom/diag_vcom_handler.o
endif

drv-y += version/version_emdm.o

ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/dump/mdmext
drv-y+= dump/mdmext/dump_mdmext_area.o
drv-y+= dump/mdmext/dump_mdmext_baseinfo.o
drv-y+= dump/mdmext/dump_mdmext_comm.o
drv-y+= dump/mdmext/dump_mdmext_fastboot_agent.o
drv-y+= dump/mdmext/dump_mdmext_field.o
drv-y+= dump/mdmext/dump_mdmext_kernel_agent.o
drv-y+= dump/mdmext/dump_mdmext_pcie_comm.o
drv-y+= dump/mdmext/dump_mdmext_core.o
drv-y+= dump/mdmext/dump_mdmext_hook.o
drv-y+= dump/mdmext/dump_mdmext_ko_dump.o
drv-y+= log/bsp_om_log.o
drv-y+= adp/adp_print.o
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_ADC)),YES)
drv-y += adc/bsp_adc.o
endif
# module makefile end
else

obj-y           += adp/adp_socp.o
obj-y           += adp/adp_usb.o

ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/diag/scm \
                   -Idrivers/hisi/modem/drv/diag/cpm \
                   -Idrivers/hisi/modem/drv/diag/ppm \
                   -Idrivers/hisi/modem/drv/diag/debug \
                   -Idrivers/hisi/modem/drv/diag/comm \
                   -Idrivers/hisi/modem/drv/diag/serivce \
                   -Idrivers/hisi/modem/drv/diag/report \
                   -Idrivers/hisi/modem/drv/socp/soft_decode
subdir-ccflags-y+= -Idrivers/hisi/modem/include/tools

obj-y           += diag/cpm/diag_port_manager.o
obj-y           += diag/debug/diag_system_debug.o
obj-y           += diag/ppm/OmCommonPpm.o
obj-y           += diag/ppm/OmSocketPpm.o
obj-y           += diag/ppm/OmUsbPpm.o
obj-y           += diag/ppm/OmVcomPpm.o
obj-y           += diag/comm/diag_comm.o
obj-y           += diag/ppm/OmCpAgentPpm.o
obj-y           += diag/ppm/OmPortSwitch.o
obj-y           += socp/soft_decode/ring_buffer.o
endif

##ifeq ($(strip $(CFG_CONFIG_MODULE_IPC)),YES)
##obj-y               += ipc/
##endif

ifeq ($(strip $(CFG_CONFIG_BALONG_MODEM_GLOBAL_RESET)),YES)
obj-y += reset/reset_balong_global.o
endif

ifeq ($(strip $(CFG_CONFIG_VCOM_NODE)),YES)
obj-y += vcom/vcom.o
obj-y += vcom/vcom_balong_power.o
endif
ifeq ($(strip $(CFG_CONFIG_VCOM_AGENT)),YES)
obj-y += vcom/vcom_agent.o
endif
ifeq ($(strip $(CFG_CONFIG_UART_BOOT)),YES)
obj-y += modem_boot/uart_boot_dev.o
endif

ifeq ($(strip $(CFG_CONFIG_MODEM_BOOT)),YES)
obj-y += modem_boot/modem_load.o
obj-y += modem_boot/modem_boot.o
endif

obj-y += s_memory/s_memory.o
obj-y += icc/icc_core.o icc/icc_linux.o icc/icc_debug.o

ifeq ($(strip $(CFG_CONFIG_RFS_SERVER)),YES)
obj-y += rfs_server/
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_DEV)),YES)
obj-y += pcie_balong_dev/pcie_balong_dev.o
obj-y += pcie_boot_dev/pcie_boot_dev.o
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_DEV_TEST)),YES)
obj-y += pcie_balong_dev/pcie_balong_dev_test.o
endif

ifeq ($(strip $(CFG_CONFIG_BALONG_PCIE_CDEV_RC)),YES)
obj-y += pfunc/
endif

obj-$(CONFIG_BALONG_PCIE_ETH_RC) += peth/

obj-y += adp/adp_socp.o
obj-y += adp/adp_usb.o
ifeq ($(strip $(CFG_CONFIG_DIAG_SYSTEM)),YES)
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/diag/scm \
                   -Idrivers/hisi/modem/drv/diag/cpm \
                   -Idrivers/hisi/modem/drv/diag/ppm \
                   -Idrivers/hisi/modem/drv/diag/debug \
                   -Idrivers/hisi/modem/drv/diag/comm \
                   -Idrivers/hisi/modem/drv/diag/serivce \
                   -Idrivers/hisi/modem/drv/diag/report \
                   -Idrivers/hisi/modem/drv/socp/soft_decode
subdir-ccflags-y+= -Idrivers/hisi/modem/include/tools
subdir-ccflags-y+= -Idrivers/hisi/modem/include/nv/acore/drv
subdir-ccflags-y+= -Idrivers/hisi/modem/include/nv/common/drv
obj-y           += diag/cpm/diag_port_manager.o
obj-y           += diag/debug/diag_system_debug.o
obj-y           += diag/ppm/OmCommonPpm.o
obj-y           += diag/ppm/OmSocketPpm.o
obj-y           += diag/ppm/OmUsbPpm.o
obj-y           += diag/ppm/OmVcomPpm.o
obj-y           += diag/ppm/OmCpAgentPpm.o
obj-y           += diag/ppm/OmPortSwitch.o
obj-y           += diag/comm/diag_comm.o
obj-y           += socp/soft_decode/ring_buffer.o
endif

ifeq ($(strip $(CFG_CONFIG_DIAG_NETLINK)),YES)
obj-y           += diag_vcom/diag_vcom_main.o
obj-y           += diag_vcom/diag_vcom_handler.o
endif

obj-y += version/version_emdm.o

ifeq ($(strip $(CFG_ENABLE_BUILD_OM)),YES)
subdir-ccflags-y+= -Idrivers/hisi/modem/drv/dump/mdmext
obj-y+= dump/mdmext/dump_mdmext_area.o
obj-y+= dump/mdmext/dump_mdmext_baseinfo.o
obj-y+= dump/mdmext/dump_mdmext_comm.o
obj-y+= dump/mdmext/dump_mdmext_config.o
obj-y+= dump/mdmext/dump_mdmext_fastboot_agent.o
obj-y+= dump/mdmext/dump_mdmext_field.o
obj-y+= dump/mdmext/dump_mdmext_kernel_agent.o
obj-y+= dump/mdmext/dump_mdmext_pcie_comm.o
obj-y+= dump/mdmext/dump_mdmext_core.o
obj-y+= dump/mdmext/dump_mdmext_hook.o
obj-y+= log/bsp_om_log.o
obj-y+= adp/adp_print.o
endif
ifeq ($(strip $(CFG_CONFIG_BALONG_ADC)),YES)
obj-y += adc/bsp_adc.o
endif

endif
