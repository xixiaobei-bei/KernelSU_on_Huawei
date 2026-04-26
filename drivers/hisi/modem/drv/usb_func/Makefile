subdir-ccflags-y := -Werror

obj-y += u_ppp_ndis_ctrl.o
ifeq ($(strip $(CONFIG_USB_GADGET)),y)
obj-y += u_ndis_ctrl.o
endif