ifeq ($(CONFIG_LZ4_SPEEDUP),y)
ifeq ($(strip $(cfi_check)),true)
ccflags-y += -O3
else
ccflags-y += -O3 -mllvm -custom-pm-sequence=$(CLANG_PREBUILTS_PATH)/../../linux-x86-huawei/clang-r353983c-huawei/share/clang/config/lz4_decompress.json -mcpu=cortex-a55 -tunertest-microopbuffersize=256 -tunertest-mispredictpenalty=30 -tunertest-loopmicroppbuffersize=14 -tunertest-loadlatency=6 -tunertest-issuewidth=7
endif
else
ccflags-y += -O3
endif

obj-$(CONFIG_LZ4_DECOMPRESS) += lz4_decompress.o
