ifeq ($(CONFIG_LZ4_SPEEDUP),y)
ifeq ($(strip $(cfi_check)),true)
ccflags-y += -O3
else
ccflags-y += -O3 -mllvm -custom-pm-sequence=$(CLANG_PREBUILTS_PATH)/../../linux-x86-huawei/clang-r353983c-huawei/share/clang/config/lz4_compress.json
endif
else
ccflags-y += -O3
endif

obj-$(CONFIG_LZ4_COMPRESS) += lz4_compress.o
