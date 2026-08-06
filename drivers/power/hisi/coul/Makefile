obj-$(CONFIG_COUL_PMIC6X21V500)	+= pmic6x21v500/
obj-$(CONFIG_COUL_PMIC6X55V100)	+= pmic6x55v100/
obj-$(CONFIG_COUL_PMIC6X55V200)      += pmic6xxx/
obj-$(CONFIG_COUL_PMIC6X55V300)      += pmic6xxx/
obj-$(CONFIG_COUL_PMIC6X55V500)      += pmic6xxx/
obj-$(CONFIG_COUL_PMIC6X21V600)      += pmic6xxx/
obj-$(CONFIG_COUL_PMIC6X21V700)      += pmic6xxx/
obj-$(CONFIG_COUL_PMIC6X21V800)      += pmic6xxx/
obj-$(CONFIG_COUL_PMIC6X21V900)      += pmic6xxx/

ifeq ($(wildcard $(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)),)
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/default/
else
ccflags-y  += -I$(srctree)/drivers/huawei_platform/inputhub/$(TARGET_BOARD_PLATFORM)/
endif

obj-$(CONFIG_COUL_DRV)         += coul.o
coul-y                         := coul_interface.o coul_private_interface.o coul_temp.o coul_dts.o coul_nv.o
coul-y                         += coul_sysfs.o coul_core_test.o coul_ocv_ops.o
coul-y                         += coul_fault_evt_ops.o coul_core.o coul_drv.o coul_event.o
obj-$(CONFIG_COUL_POLAR)       += coul_polar.o coul_update_polar_info.o
obj-$(CONFIG_COUL_TEST)        += coul_drv_test.o
