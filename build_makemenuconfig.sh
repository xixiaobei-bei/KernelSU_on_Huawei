#!/bin/bash
#设置环境

# 交叉编译器路径
export PATH=$PATH:$(pwd)/../Compiler/aarch64-linux-android-4.9-pie/bin
export CROSS_COMPILE=aarch64-linux-android-

export ARCH=arm64
# export DTC_EXT=dtc

make ARCH=arm64 O=out merge_hi3660_defconfig
make ARCH=arm64 O=out menuconfig