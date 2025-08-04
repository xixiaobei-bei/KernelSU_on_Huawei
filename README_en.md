### KernelSU on Huawei Mate9[简体中文](README.md) | [English](README_en.md)

A KernelSU kernel for Huawei Mate9!

Flash the corresponding version to integrate KernelSU into your device.

# Naming Convention

Kernels with 9.0 in the name are for devices with an EMUI 5.x base.

Kernels with 9.0 in the name are for devices with an EMUI 9.0.x base.

Kernels with 2.0 in the name are for devices with a HarmonyOS base.

Kernels with the PM suffix have permissive SELinux and can be used to boot SGSI.

# Notes

1.If flashing a PM kernel, you must decrypt the DATA partition before booting.

2.Ensure you flash the kernel matching your device's base version, otherwise the device will not boot!

# Flashing

1.Ensure the device's Bootloader is unlocked.

2.Download the corresponding kernel.

3.Boot the phone into fastboot mode and connect it to a computer.

4.On the computer, navigate to the ADB tools directory and run the following command:
```bash
fastboot flash kernel <Kernel File>
```

If using a PM kernel, decrypt the DATA partition and format it before booting.

# Build

1.Download the corresponding kernel source code.

2.Modify the desired content.

3.Install dependencies by running the following command in the terminal:
```bash
sudo apt-get install libncurses5-dev libncurses-dev libssl-dev device-tree-compiler bc cpio lib32ncurses5-dev lib32z1 build-essential binutils bc bison build-essential ccache curl flex g++-multilib gcc-multilib git gnupg gperf imagemagick lib32ncurses5-dev lib32readline-dev lib32z1-dev liblz4-tool libncurses5 libncurses5-dev libsdl1.2-dev libssl-dev libxml2 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev git python2
```

4.Set Python 2.7 as the default by running:
```bash
sudo ln -sf /usr/bin/python2.7 /usr/bin/python
```

5.For x86 computers, run:
```bash
git clone --depth=1 https://github.com/LineageOS/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9 aarch64-linux-android-4.9
```
For ARM computers, run:
```bash
git clone --depth=1 https://github.com/LineageOS/android_prebuilts_gcc_linux-x86_arm_arm-linux-androideabi-4.9 arm-linux-androideabi-4.9
```
This downloads the cross-compiler.

6.Open a terminal in the kernel source directory and run (replace the path with the bin folder path of the downloaded cross-compiler):
```bash
export ARCH=arm64
export PATH=$PATH:/media/coconutat/Files/Downloads/Github/android_kernel_huawei_ravel_KernelSU——android_kernel_huawei_ravel_KernelSU/aarch64-linux-android-4.9/bin
export CROSS_COMPILE=aarch64-linux-android-
``````

7.Keep the terminal open and run the following commands to compile (replace 8 with 2x your CPU core count):
```bash
make ARCH=arm64 O=out merge_hi3660_defconfig
make ARCH=arm64 O=out -j8
```

8.Wait for compilation to complete.

9.After compilation, the Image.gz file will be generated in out/arch/arm64/boot. Move it to the tools folder in the kernel directory.

10.Modify the date in pack_kernerimage_cmd.sh to the current time.

11.Run the following command in the tools directory to package the kernel:
```bash
bash pack_kernerimage_cmd.sh
```

12.The kernel.img file will be generated in the tools directory. Flash and test it.

# Special Thanks
[dabao1955](https://github.com/dabao1955) / [KernelSU_General_porting_guide](https://github.com/dabao1955/KernelSU_General_porting_guide)——For providing kernel modification methods.

[tiann](https://github.com/tiann) / [KernelSU](https://github.com/tiann/KernelSU/)——The creator of KernelSU.



