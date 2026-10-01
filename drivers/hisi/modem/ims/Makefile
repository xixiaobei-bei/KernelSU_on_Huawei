ims-objs	:=

ims-objs	+= src/act/act_acore.o

ifneq ($(strip $(CONFIG_HISI_BALONG_MODEM)),m)
obj-$(CONFIG_HISI_BALONG_MODEM_HI3650)   += ims.o
endif
