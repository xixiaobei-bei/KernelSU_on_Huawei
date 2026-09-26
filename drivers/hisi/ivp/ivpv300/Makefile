# hisilicon ivp ivpv300
ifeq ($(es_low_freq),true)
ccflags-$(CONFIG_IVP_IVPV300) += -DCONFIG_ES_LOW_FREQ
endif

ccflags-$(CONFIG_IVP_IVPV300) += -Idrivers/hisi/ivp/

obj-$(CONFIG_IVP_IVPV300) += ivp_platform.o
