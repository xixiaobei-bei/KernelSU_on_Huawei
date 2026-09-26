obj-$(CONFIG_NPUFREQ)      += npu_devfreq.o
ifeq ($(CONFIG_NPU_PM_SMC), y)
obj-$(CONFIG_NPU_PM)       += smc/npu_pm_smc.o
obj-$(CONFIG_NPU_PM)       += smc/npu_dvfs_smc.o
obj-$(CONFIG_NPU_PM_DEBUG) += smc/npu_pm_debug_smc.o
else
obj-$(CONFIG_NPU_PM)       += npu_pm.o
obj-$(CONFIG_NPU_PM)      += npu_dvfs.o
obj-$(CONFIG_NPU_PM_DEBUG) += npu_pm_debug.o
obj-$(CONFIG_NPU_PM_AVS_VDM)  += npu_avs.o
endif

subdir-ccflags-$(CONFIG_NPU_PM_SHARE_PERI_BUCK)  += -Idrivers/clk/hisi/dvfs/
ccflags-$(CONFIG_NPU_PM)  +=
