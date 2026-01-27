# 如何构建

感谢大佬们的辛苦奉献，我只不过是站在了巨人的肩膀上而已

本教程虽然有别的型号的教程，但是我没有设备可以试一试，不保证能成功，主要面向华为

deconfig配置文件在 内核源码目录/arch/arm64/configs 文件夹下的deconfig结尾的文件，可能有多个，根据你的手机型号来

## 1.安装依赖
:::warning
由于Ubuntu/Debian高版本全局定义yylloc问题，所以只推荐使用Ubuntu20.04或Ubuntu18.04（Debian13以下，推荐这两个版本）
:::
#### 1.安装编译所需要的软件包

运行：

```bash
sudo apt-get install libncurses5-dev libncurses-dev libssl-dev device-tree-compiler bc cpio lib32ncurses5-dev lib32z1 build-essential binutils bc bison build-essential ccache curl flex g++-multilib gcc-multilib git gnupg gperf imagemagick lib32ncurses5-dev lib32readline-dev lib32z1-dev liblz4-tool libncurses5 libncurses5-dev libsdl1.2-dev libssl-dev libxml2 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev git python2
```

#### 2.获取GCC

```bash
git clone --depth=1 https://kkgithub.com/LineageOS/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9 aarch64-linux-android-4.9
```

注意：华为只能用Aarch64版本

## 2.获取源码

[华为的官方源码](https://consumer.huawei.com/en/opensource/)

在其中输入设备型号代码，下载后打开```Code_sources\kernel```文件夹，这为你的内核源码，解压出来即可

:::warning
由于编码原因，只能在Linux环境下解压到非ntfs/fat分区，否则会无法编译
:::

#### 其他设备可寻找第三方ROM的内核源码或在互联网搜索

## 3.更改内核

#### 需要关闭deconfig配置文件的以下选项：

```text
CONFIG_HISI_PMALLOC=y
CONFIG_HIVIEW_SELINUX=y
CONFIG_HISI_SELINUX_EBITMAP_RO=y
CONFIG_HISI_SELINUX_PROT=y
CONFIG_HISI_RO_LSM_HOOKS=y
CONFIG_INTEGRITY=y
CONFIG_INTEGRITY_AUDIT=y
CONFIG_HUAWEI_CRYPTO_TEST_MDPP=y
CONFIG_HUAWEI_SELINUX_DSM=y
CONFIG_HUAWEI_HIDESYMS=y
CONFIG_HW_SLUB_SANITIZE=y
CONFIG_HUAWEI_PROC_CHECK_ROOT=y
CONFIG_HW_ROOT_SCAN=y
CONFIG_HUAWEI_EIMA=y
CONFIG_HUAWEI_EIMA_ACCESS_CONTROL=y
CONFIG_HW_DOUBLE_FREE_DYNAMIC_CHECK=y
CONFIG_HKIP_ATKINFO=y
CONFIG_HW_KERNEL_STP=y
CONFIG_HISI_HHEE=y
CONFIG_HISI_HHEE_TOKEN=y
CONFIG_HISI_DIEID=y
CONFIG_HISI_SUBPMU=y
CONFIG_TEE_ANTIROOT_CLIENT=y
CONFIG_HWAA=y
```

这些内容需要改成如下格式：

```text
## CONFIG_XXXXXX is not set
```

一键替换命令：

```bash
sed -i \
  -e 's/^CONFIG_HISI_PMALLOC=y/## CONFIG_HISI_PMALLOC is not set/' \
  -e 's/^CONFIG_HIVIEW_SELINUX=y/## CONFIG_HIVIEW_SELINUX is not set/' \
  -e 's/^CONFIG_HISI_SELINUX_EBITMAP_RO=y/## CONFIG_HISI_SELINUX_EBITMAP_RO is not set/' \
  -e 's/^CONFIG_HISI_SELINUX_PROT=y/## CONFIG_HISI_SELINUX_PROT is not set/' \
  -e 's/^CONFIG_HISI_RO_LSM_HOOKS=y/## CONFIG_HISI_RO_LSM_HOOKS is not set/' \
  -e 's/^CONFIG_INTEGRITY=y/## CONFIG_INTEGRITY is not set/' \
  -e 's/^CONFIG_INTEGRITY_AUDIT=y/## CONFIG_INTEGRITY_AUDIT is not set/' \
  -e 's/^CONFIG_HUAWEI_CRYPTO_TEST_MDPP=y/## CONFIG_HUAWEI_CRYPTO_TEST_MDPP is not set/' \
  -e 's/^CONFIG_HUAWEI_SELINUX_DSM=y/## CONFIG_HUAWEI_SELINUX_DSM is not set/' \
  -e 's/^CONFIG_HUAWEI_HIDESYMS=y/## CONFIG_HUAWEI_HIDESYMS is not set/' \
  -e 's/^CONFIG_HW_SLUB_SANITIZE=y/## CONFIG_HW_SLUB_SANITIZE is not set/' \
  -e 's/^CONFIG_HUAWEI_PROC_CHECK_ROOT=y/## CONFIG_HUAWEI_PROC_CHECK_ROOT is not set/' \
  -e 's/^CONFIG_HW_ROOT_SCAN=y/## CONFIG_HW_ROOT_SCAN is not set/' \
  -e 's/^CONFIG_HUAWEI_EIMA=y/## CONFIG_HUAWEI_EIMA is not set/' \
  -e 's/^CONFIG_HUAWEI_EIMA_ACCESS_CONTROL=y/## CONFIG_HUAWEI_EIMA_ACCESS_CONTROL is not set/' \
  -e 's/^CONFIG_HW_DOUBLE_FREE_DYNAMIC_CHECK=y/## CONFIG_HW_DOUBLE_FREE_DYNAMIC_CHECK is not set/' \
  -e 's/^CONFIG_HKIP_ATKINFO=y/## CONFIG_HKIP_ATKINFO is not set/' \
  -e 's/^CONFIG_HW_KERNEL_STP=y/## CONFIG_HW_KERNEL_STP is not set/' \
  -e 's/^CONFIG_HISI_HHEE=y/## CONFIG_HISI_HHEE is not set/' \
  -e 's/^CONFIG_HISI_HHEE_TOKEN=y/## CONFIG_HISI_HHEE_TOKEN is not set/' \
  -e 's/^CONFIG_HISI_DIEID=y/## CONFIG_HISI_DIEID is not set/' \
  -e 's/^CONFIG_HISI_SUBPMU=y/## CONFIG_HISI_SUBPMU is not set/' \
  -e 's/^CONFIG_TEE_ANTIROOT_CLIENT=y/## CONFIG_TEE_ANTIROOT_CLIENT is not set/' \
  -e 's/^CONFIG_HWAA=y/## CONFIG_HWAA is not set/' \
  merge_hi3660_defconfig
```

把最后一行的merge_hi3660_defconfig换成你的deconfig配置文件路径

#### 可选部分： 

把

```text
## CONFIG_SECURITY_SELINUX_DEVELOP is not set
```

改为

```
CONFIG_SECURITY_SELINUX_DEVELOP=y
```
可以让开机的时候手机SELinux为Permissive状态。

#### 关闭AVB验证：

```text
CONFIG_DM_VERITY=y  
CONFIG_DM_VERITY_AVB=y
```

改为

```text
## CONFIG_DM_VERITY=y is not set  
## CONFIG_DM_VERITY_AVB=y is not set 
```

## 4.集成KernelSU

#### 1.拉取源码

:::code-group

```KernelSU
curl -LSs "https://raw.githubusercontent.com/tiann/KernelSU/main/kernel/setup.sh" | bash -s v0.9.5
```

```KernelSU-Next
curl -LSs "https://raw.githubusercontent.com/KernelSU-Next/KernelSU-Next/next/kernel/setup.sh" | bash -
```

:::

#### 2.启用KernelSU

在你的设备deconfig配置文件最后加入以下几行：

```text
## KernelSU
CONFIG_KSU=y
```

若要开启KernelSU的调试模式，还需要加入：

```text
CONFIG_KSU_DEBUG=y
```

#### 3.集成KernelSU到内核源码

参考[KernelSU官网](https://kernelsu.org/zh_CN/guide/how-to-integrate-for-non-gki.html##modify-kernel-source-code)修改，要该的c文件在fs目录

注意，若你的内核没有vfs_statx和do_faccessat，不要抄写上面的通用代码，要用下面给的，不要忽略！

#### 4.修改hooks.c以启用模块
参考[此GithubCommit](https://github.com/sticpaper/android_kernel_xiaomi_msm8998-ksu/commit/09a4672c0f521bf6b05daf24b207b125830a6fc5)

####  5.针对EMUI9/9.1.0 SELinux强制状态导致KernelSU不工作（已废弃，KernelSU v0.6版本已修复次问题）

参考[此GithubCommit](https://github.com/Coconutat/android_kernel_huawei_ravel_KernelSU/commit/f67307c967280d9b863058e47bae7611c8bc3db9)的第166行

## 5.编译

#### 1.设定默认Python2

```bash
sudo ln -sf /usr/bin/python2.7 /usr/bin/python
```

#### 2.设置环境变量

```bash
export ARCH=arm64  
export PATH=$PATH:GCC编译工具下bin文件夹的绝对路径  
export CROSS_COMPILE=aarch64-linux-android-
```

#### 3.编译

在内核源码目录输入命令：

```bash
make ARCH=arm64 O=out deconfig配置文件名字(只要名字，不要路径)  
make ARCH=arm64 O=out -j8
```

这里的-j8指定编译的CPU核心数，要你的CPU核心数X2

#### 4.编译完后打包

编译完后在out/arch/arm64/boot/路径下会有一个Image.gz文件，复制到内核根目录的tools文件夹

更改pack_kernerimage_cmd.sh文件内容为：

Selinux的Enforcing状态版本：

```sh
##!/bin/bash 
./mkbootimg --kernel kernel --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07C00000 --header_version 1 --os_version 9 --os_patch_level 2020-01-01  --output kernel.img
```

Selinux的Permissive状态版本：

```sh
##!/bin/bash 
./mkbootimg --kernel kernel --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07C00000 --header_version 1 --os_version 9 --os_patch_level 2020-01-01  --output kernel.img
```

###### 里面的内核编译时间需要更改

随后运行：

```bash
bash pack_kernerimage_cmd.sh
```

然后本目录会多出一个kernel.img文件，fastboot刷入kernel分区即可