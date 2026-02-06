# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_HMDFS_FS) += hmdfs.o
hmdfs-y := \
	main.o super.o inode.o dentry.o client_writeback.o \
	inode_root.o inode_local.o inode_remote.o inode_merge.o \
	file_root.o file_local.o file_remote.o file_merge.o \
	hmdfs_client.o hmdfs_server.o hmdfs_dentryfile.o \
	comm/device_node.o comm/connection.o comm/transport.o \
	comm/socket_adapter.o comm/message_verify.o DFS_1_0/adapter_crypto.o
hmdfs-y += server_writeback.o
hmdfs-y += comm/node_cb.o
hmdfs-y += stash.o

# For backward compatibility
hmdfs-$(CONFIG_HMDFS_1_0) += \
	DFS_1_0/adapter_client.o \
	DFS_1_0/adapter_dentry_limitation.o \
	DFS_1_0/adapter_dentry.o \
	DFS_1_0/adapter_file_id_generator.o \
	DFS_1_0/adapter_file.o \
	DFS_1_0/adapter_inode.o \
	DFS_1_0/adapter_lookup.o \
	DFS_1_0/adapter_server.o \
	DFS_1_0/adapter_socket.o \
	DFS_1_0/dentry_syncer.o

# For 11.0 crypto after handshake
hmdfs-$(CONFIG_HMDFS_CRYPTO) += \
	comm/crypto.o

# For acl permission check.
hmdfs-$(CONFIG_HMDFS_ANDROID) += \
	authority/authentication.o \
	authority/packagelist.o

# For fault_inject
hmdfs-$(CONFIG_FAULT_INJECTION_DEBUG_FS) += comm/fault_inject.o

ccflags-y += -I$(src) -Werror -Wall
ccflags-$(CONFIG_HMDFS_1_0) += -DCONFIG_HMDFS_1_0
ccflags-$(CONFIG_HMDFS_CRYPTO) += -DCONFIG_HMDFS_CRYPTO
ccflags-$(CONFIG_HMDFS_ANDROID) += -DCONFIG_HMDFS_ANDROID
ccflags-$(ENABLE_SYMBOL) += -g
# For external module build
ifeq ($(KERNELRELEASE),)

KVER := $(shell uname -r)
KDIR := /lib/modules/$(KVER)/build
PWD := $(shell pwd)

export CONFIG_HMDFS_FS := m
export CONFIG_HMDFS_ANDROID := y
export CONFIG_HMDFS_1_0 := y
export CONFIG_HMDFS_CRYPTO := y
export ENABLE_SYMBOL := y

all: hmdfs

hmdfs: hmdfs-sym
	mkdir -p out
	objcopy -g hmdfs.ko out/hmdfs.ko
hmdfs-sym:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	@git clean -dfx
rebuild:
	@make clean && make
endif

.PHONY : all clean rebuild
