################################################################################

1. How to Build
- get Toolchain
From android git server, codesourcery and etc ..
- aarch64-linux-android-4.9
- clang-r353983c(git clone https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86)

- edit Makefile
edit CROSS_COMPILE to right toolchain path(You downloaded).
Ex)   export PATH :=${PATH}:$(android platform directory you download)/prebuilts/gcc/linux-x86/aarch64/aarch64-linux-android-4.9/bin
Ex)   export CROSS_COMPILE=aarch64-linux-android-

- put the clang in the right path.
put the clang-r353983c in the $(kernel directory)/../../prebuilts/clang/host/linux-x86 path

$ mkdir ../out
$ make ARCH=arm64 O=../out merge_kirin970_defconfig
$ make ARCH=arm64 O=../out -j8

2. Output files
- Kernel : out/arch/arm64/boot/Image.gz
- module : out/drivers/*/*.ko

3. How to Clean
$ make ARCH=arm64 distclean
$ rm -rf out
################################################################################
