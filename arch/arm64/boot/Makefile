#
# arch/arm64/boot/Makefile
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies.
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 2012, ARM Ltd.
# Author: Will Deacon <will.deacon@arm.com>
#
# Based on the ia64 boot/Makefile.
#

include $(srctree)/arch/arm64/boot/dts/Makefile

ifeq ($(CONFIG_ARM64_HKRR),y)
OBJCOPYFLAGS_Image.in :=-O binary -R .note -R .note.gnu.build-id -R .comment -S
HKRRFLAGS :=
GENFLAGS :=

ifeq ($(CONFIG_ARM64_HKRR_DEBUG),y)
HKRRFLAGS += --debug
endif

ifneq ($(CONFIG_LTO_CLANG),y)
GENFLAGS += --align
endif

else
OBJCOPYFLAGS_Image :=-O binary -R .note -R .note.gnu.build-id -R .comment -S
endif

targets := Image Image.gz

DTB_NAMES := $(subst $\",,$(CONFIG_BUILD_ARM64_APPENDED_DTB_IMAGE_NAMES))
ifneq ($(DTB_NAMES),)
DTB_LIST := $(addsuffix .dtb,$(DTB_NAMES))
else
DTB_LIST := $(dtb-y)
endif
DTB_OBJS := $(addprefix $(obj)/dts/,$(DTB_LIST))

ifeq ($(CONFIG_ARM64_HKRR),y)

$(obj)/Image: $(obj)/Image.in hkrr.img
	$(srctree)/scripts/hkrr.py $(HKRRFLAGS) insert hkrr.img $(obj)/Image.in $(obj)/Image

hkrr.img: vmlinux $(srctree)/scripts/hkrr.py
	rm -f hkrr.log
	$(srctree)/scripts/hkrr.py $(HKRRFLAGS) generate $(GENFLAGS) vmlinux vmlinux.map hkrr.img

$(obj)/Image.in: vmlinux FORCE
	$(call if_changed,objcopy)

else

$(obj)/Image: vmlinux FORCE
	$(call if_changed,objcopy)

endif

$(obj)/Image.bz2: $(obj)/Image FORCE
	$(call if_changed,bzip2)

$(obj)/Image-dtb: $(obj)/Image $(DTB_OBJS) FORCE
	$(call if_changed,cat)

$(obj)/Image.gz: $(obj)/Image FORCE
	$(call if_changed,minigzip)

$(obj)/Image.lz4: $(obj)/Image FORCE
	$(call if_changed,lz4)

$(obj)/Image.lzma: $(obj)/Image FORCE
	$(call if_changed,lzma)

$(obj)/Image.lzo: $(obj)/Image FORCE
	$(call if_changed,lzo)

$(obj)/Image.gz-dtb: $(obj)/Image.gz $(DTB_OBJS) FORCE
	$(call if_changed,cat)

install:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh $(KERNELRELEASE) \
	$(obj)/Image System.map "$(INSTALL_PATH)"

zinstall:
	$(CONFIG_SHELL) $(srctree)/$(src)/install.sh $(KERNELRELEASE) \
	$(obj)/Image.gz System.map "$(INSTALL_PATH)"
