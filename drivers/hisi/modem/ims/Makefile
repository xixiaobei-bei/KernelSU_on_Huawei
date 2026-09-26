ims-objs	:=

ims-objs	+= src/act/act_acore.o

ifeq ($(strip $(CONFIG_MODEM_PS_FUSION_VERSION)),y)
EXTRA_CFLAGS += -DIMS_FEATURE_LTE_5010
endif

ifneq ($(strip $(CONFIG_HISI_BALONG_MODEM)),m)
obj-$(CONFIG_HISI_BALONG_MODEM_HI3650)   += ims.o
endif
