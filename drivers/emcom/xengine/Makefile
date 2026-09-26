EXTRA_CFLAGS += -Inet/netfilter

obj-$(CONFIG_HUAWEI_XENGINE) += emcom_xengine.o emcom_mpdns.o mpflow.o mpflow_ai.o

ifdef CONFIG_HUAWEI_EMCOM_CHANNEL
ifneq ($(OBB_SEPARATE),true)
ifdef CONFIG_HISI_BALONG_MODEM
EXTRA_CFLAGS += -DCONFIG_HUAWEI_BASTET_COMM
endif
endif
endif
