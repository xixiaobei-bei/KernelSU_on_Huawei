# hisilicon ivp ivpv310
ifeq ($(es_low_freq),true)
ccflags-$(CONFIG_IVP_IVPV310) += -DCONFIG_ES_LOW_FREQ
endif

ccflags-$(CONFIG_IVP_IVPV310) += -Idrivers/hisi/ivp/

obj-$(CONFIG_IVP_IVPV310) += ivp_platform.o
obj-$(CONFIG_IVP_IVPV310) += ivp_atf_subsys.o
