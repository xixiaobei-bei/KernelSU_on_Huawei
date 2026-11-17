KERNEL_DIR :=$(srctree)

EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/core
EXTRA_CFLAGS += -I$(KERNEL_DIR)/drivers/hisi/tzdriver/tlogger

obj-$(CONFIG_CLIENT_AUTH) += client_hash_auth.o
obj-$(CONFIG_AUTH_ENHANCE) += security_auth_enhance.o

ifeq ($(findstring y, $(CONFIG_TEECD_AUTH) $(CONFIG_CLIENT_AUTH)), y)
	obj-y += auth_base_impl.o
endif