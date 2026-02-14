export ARCH=arm64
export PATH=$PATH:/home/xfy/kernel/aarch64-linux-android-4.9/bin
export CROSS_COMPILE=aarch64-linux-android-
make clean
make ARCH=arm64 O=out merge_kirin970_defconfig
make ARCH=arm64 O=out -j8

