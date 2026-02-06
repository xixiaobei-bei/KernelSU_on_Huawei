ifeq ($(TARGET_BOARD_PLATFORM), denver)
ifeq ($(product_type), lite)
NPU_CUT_MAKEFILE := yes
endif
endif
obj-$(CONFIG_LPM_HKADC) += adc/
obj-$(CONFIG_LPCPU_CCI_EXTRA) += cci_extra/
obj-$(CONFIG_HISI_CMDLINE_PARSE)	+= cmdline/
obj-$(CONFIG_HISI_IRQ_AFFINITY) += irq_affinity/
obj-$(CONFIG_HISILICON_PLATFORM_MAILBOX)	+= mailbox/
obj-$(CONFIG_HISI_USB_PHY_CHIP)			+= usb_phy_chip/
obj-$(CONFIG_SND_SOC_DA_SEPARATE_MBHC)		+= da_separate/
obj-$(CONFIG_DA_COMBINE_V3_CODEC)			+= da_combine/
obj-$(CONFIG_HIFI_DSP_ONE_TRACK)		+= hifi_dsp/
obj-$(CONFIG_SOUND_TRIGGER)			+= soundtrigger/
obj-$(CONFIG_HIFI_MAILBOX)			+= hifi_mailbox/
obj-$(CONFIG_HISILICON_PLATFORM_MAINTAIN) += mntn/
obj-$(CONFIG_FACTORY_SVFD) += factory_svfd/
obj-$(CONFIG_HISILICON_PLATFORM_KERNELDUMP) += memory_dump/
obj-$(CONFIG_HISILICON_PLATFORM_DDRC)   += ddrc/
obj-$(CONFIG_HISI_NOC)	+= noc/
obj-$(CONFIG_BSP_SYSCOUNTER) += syscounter/
obj-$(CONFIG_PLATFORM_SLIMBUS)+= slimbus/
obj-$(CONFIG_HSDTTRACE) += hsdt_trace/
obj-y += hhee/
obj-$(CONFIG_ATF_KERNEL_BL2_LOG) += bl2_log/
obj-$(CONFIG_FILE_BASED_ENCRYPTO) += file_encry/
obj-$(CONFIG_HISI_KERNEL_HARDEN_DEBUG) += kernel_harden_test/
obj-y += harden/
obj-$(CONFIG_LPCPU_LPNV_SWITCH) += lpnv/

obj-$(CONFIG_CODEC_DSP_MISC) += da_combine_dsp/
obj-y += dpm/
obj-y += l3cache/
obj-$(CONFIG_HISEE) += hisee/
obj-$(CONFIG_MSPC)  += mspc/
obj-$(CONFIG_HIEPS) += hieps/
ifneq ($(strip $(CFG_HISI_OPENSOURCE)), true)
endif
obj-$(CONFIG_DEV_IVP) += ivp/
obj-$(CONFIG_HISTAR_ISP) += isp/
obj-$(CONFIG_DRMDRIVER)                         += drmdriver/
obj-$(CONFIG_HI3XXX_EFUSE)                      += efuse/
obj-y                                           += hisi_cma/
obj-y                                           += hitest/
obj-y                                           += tzdriver/
obj-$(CONFIG_HISI_LB)                           += hisi_lb/
obj-$(CONFIG_LOAD_IMAGE) += load_image/
obj-$(CONFIG_HISI_SECS_CTRL) += secs_power_ctrl/
obj-$(CONFIG_SR)                      += pm/
obj-$(CONFIG_HW_VOTE)                 += hw_vote/
obj-$(CONFIG_GPUFREQ)                 += gpufreq/
obj-$(CONFIG_ONSYS_AVS)                 += onsys_avs/
obj-$(CONFIG_DRG)                     += drg/
obj-$(CONFIG_HISI_AP_CP_DECOUPL)           += modem_decoupl/
ifneq ($(link_kernel),false)
ifneq (,$(findstring $(CONFIG_HISI_BALONG_MODEM),y m))
obj-y += modem/
endif
endif
obj-y += max_power/
obj-y += hisi_lmk/
obj-$(CONFIG_HISI_PAGE_TRACKER) += hisi_page_tracker/
obj-$(CONFIG_PERFHUB) += perfhub/
obj-$(CONFIG_JPU_SUPPORT) += jpu/
obj-$(CONFIG_HISI_TIMEKEEPER) += timekeeper/
obj-$(CONFIG_TLB_CONFLICT_WORKAROUND) += tlb_conflict/
obj-$(CONFIG_LPCPU_PMU_DUMP) += lpcpu_pmu_dump/
ifeq ($(TARGET_VERSION_MODE),factory)
obj-$(CONFIG_HISI_DIEID) += dieid/
endif
obj-$(CONFIG_HISPIPP) += ipp/
obj-$(CONFIG_ACCURATE_DELAY) += accurate_delay/
obj-$(CONFIG_PERF_CTRL) += perf_ctrl/
obj-$(CONFIG_HISI_DUMMY_KO) += hisi_dummy_ko/
obj-$(CONFIG_VENDOR_RPMB) += rpmb/
obj-$(CONFIG_PERFHUB) += perfhub/
obj-$(CONFIG_HISI_ICS_IPU_PLATFORM) += ics/
ifneq ($(NPU_CUT_MAKEFILE), yes)
endif
obj-$(CONFIG_ATF_KERNEL_BL31_LOG) += bl31_log/
obj-y += bus_monitor/
obj-$(CONFIG_SOC_MAD) += soc_mad/
obj-$(CONFIG_HISI_AP_READY)	+= hisi_ap_ready/
obj-$(CONFIG_HISI_BL31_HIBERNATE) += bl31_hibernate/
obj-$(CONFIG_HISI1103_EEPROM) += hi1103_eeprom/
obj-$(CONFIG_THEED) += thee/
obj-$(CONFIG_AB_PARTITION_TABLE) += bootctrl/
