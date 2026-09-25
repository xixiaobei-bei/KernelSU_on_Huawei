obj-$(CONFIG_HISI_NPUFREQ)      += hisi_npu_devfreq.o
ifeq ($(CONFIG_HISI_NPU_PM_SMC), y)
obj-$(CONFIG_HISI_NPU_PM)       += smc/hisi_npu_pm_smc.o
obj-$(CONFIG_HISI_NPU_PM)       += smc/hisi_npu_dvfs_smc.o
obj-$(CONFIG_HISI_NPU_PM_DEBUG) += smc/hisi_npu_pm_debug_smc.o
else
obj-$(CONFIG_HISI_NPU_PM)       += hisi_npu_pm.o
obj-$(CONFIG_HISI_NPU_PM)      += hisi_npu_dvfs.o
obj-$(CONFIG_HISI_NPU_PM_DEBUG) += hisi_npu_pm_debug.o
obj-$(CONFIG_HISI_NPU_AVS_VDM)  += hisi_npu_avs.o
endif

subdir-ccflags-$(CONFIG_HISI_NPU_SHARE_PERI_BUCK)  += -Idrivers/clk/hisi/dvfs/
ccflags-$(CONFIG_HISI_NPU_PM)  +=
