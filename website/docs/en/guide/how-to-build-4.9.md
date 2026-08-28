# How to build 4.9 kernel?

## Install Dependencies

:::warning

Due to the global definition issue of `yylloc` in higher versions of Ubuntu/Debian, it is recommended to use only Ubuntu 18.04 to Ubuntu 22.04 (below Debian 13, these two versions are recommended).

If your system version is greater than 20.04, you need to install GCC9 and set it as the default:

```shell
sudo apt install gcc-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 100
```

:::

### Install the packages required for compilation

Run:

```shell
sudo apt-get install libncurses5-dev libncurses-dev libssl-dev device-tree-compiler bc cpio lib32ncurses5-dev lib32z1 build-essential binutils bc bison build-essential ccache curl flex g++-multilib gcc-multilib git gnupg gperf imagemagick lib32ncurses5-dev lib32readline-dev lib32z1-dev liblz4-tool libncurses5 libncurses5-dev libsdl1.2-dev libssl-dev libxml2 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev git python2

```

### Obtain GCC

```shell
git clone --depth=1 https://kkgithub.com/LineageOS/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9 aarch64-linux-android-4.9

```

:::tip

Huawei can only use the Aarch64 version

:::


## Build

### Set up default Python2

```shell
sudo ln -sf /usr/bin/python2.7 /usr/bin/python
```

### Set environment variables.

```shell
export ARCH=arm64  
export PATH=$PATH:<Path to your GCC>
export CROSS_COMPILE=aarch64-linux-android-
```

### Build

Run this command on your kernel root：

```shell
make ARCH=arm64 O=out <defconfig>(Only name,not path)  
make ARCH=arm64 O=out -j8
```

Here, `-j8` specifies the number of CPU cores to use for compilation; it should be set to your CPU core count × 2.

### Package after compilation.

After compilation, there will be an `Image.gz` file in the `out/arch/arm64/boot/` path. Copy it to the `tools` folder in the root directory of the kernel.

#### Modify `pack_kernerimage_cmd.sh` file

Change `--kernel kernel` to `--kernel Image.gz` to fix file name problem.

If your SELinux is in permissive mode, change `androidboot.selinux=enforcing` to `androidboot.selinux=permissive`

Change`--os_patch_level 2020-01-01` to update the kernel build timestamp.

#### Pack up

Run：

```shell
sh pack_kernerimage_cmd.sh
```

Then a kernel.img file will appear in the current directory. Flash it to the `kernel` partition using fastboot.