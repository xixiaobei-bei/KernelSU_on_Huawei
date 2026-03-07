# How to Build

Thanks to the experts for their hard work; I'm just standing on the shoulders of giants.

The defconfig configuration file is located in the kernel source directory /arch/arm64/configs folder, and there may be multiple files ending in defconfig, depending on your phone model.

## Install Dependencies

:::warning
Due to the global definition of yylloc in higher versions of Ubuntu/Debian, it is only recommended to use Ubuntu 20.04 or Ubuntu 18.04 (for Debian 13 and below, these two versions are recommended).

:::

#### 1. Install the packages required for compilation

Run:

```bash
sudo apt-get install libncurses5-dev libncurses-dev libssl-dev device-tree-compiler bc cpio lib32ncurses5-dev lib32z1 build-essential binutils bc bison build-essential ccache curl flex g++-multilib gcc-multilib git gnupg gperf imagemagick lib32ncurses5-dev lib32readline-dev lib32z1-dev liblz4-tool libncurses5 libncurses5-dev libsdl1.2-dev libssl-dev libxml2 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev git python2

```

#### 2. Obtain GCC

```bash
git clone --depth=1 https://kkgithub.com/LineageOS/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9 aarch64-linux-android-4.9

```

:::tip Huawei can only use the Aarch64 version

:::
## Obtaining the Source Code

[Huawei's Official Source Code](https://consumer.huawei.com/en/opensource/)

Enter your device model code, download, and open the ```Code_opensource\kernel``` folder. This is your kernel source code; extract it.

:::warning Due to encoding limitations, it can only be extracted to a non-NTFS/FAT partition in a Linux environment; otherwise, compilation will fail.

:::

#### For other devices, you can find the kernel source code of third-party ROMs or search on the internet.

## Modifying the Kernel

#### The following options of the defconfig configuration file need to be turned off:

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
CONFIG_HUAWEI_ PROC_CHECK_ROOT=y
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

These contents need to be changed to the following format:

```text
# CONFIG_XXXXXX is not set
```

Automatic replacement command:
```bash
sed -i '/^CONFIG_HISI_PMALLOC=y$/c\# CONFIG_HISI_PMALLOC is not set
/^CONFIG_HIVIEW_SELINUX=y$/c\# CONFIG_HIVIEW_SELINUX is not set
/^CONFIG_HISI_SELINUX_EBITMAP_RO=y$/c\# CONFIG_HISI_SELINUX_EBITMAP_RO is not set
/^CONFIG_HISI_SELINUX_PROT=y$/c\# CONFIG_HISI_SELINUX_PROT is not set
/^CONFIG_HISI_RO_LSM_HOOKS=y$/c\# CONFIG_HISI_RO_LSM_HOOKS is not set
/^CONFIG_INTEGRITY=y$/c\# CONFIG_INTEGRITY is not set
/^CONFIG_INTEGRITY_AUDIT=y$/c\# CONFIG_INTEGRITY_AUDIT is not set
/^CONFIG_HUAWEI_CRYPTO_TEST_MDPP=y$/c\# CONFIG_HUAWEI_CRYPTO_TEST_MDPP is not set
/^CONFIG_HUAWEI_SELINUX_DSM=y$/c\# CONFIG_HUAWEI_SELINUX_DSM is not set
/^CONFIG_HUAWEI_HIDESYMS=y$/c\# CONFIG_HUAWEI_HIDESYMS is not set
/^CONFIG_HW_SLUB_SANITIZE=y$/c\# CONFIG_HW_SLUB_SANITIZE is not set
/^CONFIG_HUAWEI_PROC_CHECK_ROOT=y$/c\# CONFIG_HUAWEI_PROC_CHECK_ROOT is not set
/^CONFIG_HW_ROOT_SCAN=y$/c\# CONFIG_HW_ROOT_SCAN is not set
/^CONFIG_HUAWEI_EIMA=y$/c\# CONFIG_HUAWEI_EIMA is not set
/^CONFIG_HUAWEI_EIMA_ACCESS_CONTROL=y$/c\# CONFIG_HUAWEI_EIMA_ACCESS_CONTROL is not set
/^CONFIG_HW_DOUBLE_FREE_DYNAMIC_CHECK=y$/c\# CONFIG_HW_DOUBLE_FREE_DYNAMIC_CHECK is not set
/^CONFIG_HKIP_ATKINFO=y$/c\# CONFIG_HKIP_ATKINFO is not set
/^CONFIG_HW_KERNEL_STP=y$/c\# CONFIG_HW_KERNEL_STP is not set
/^CONFIG_HISI_HHEE=y$/c\# CONFIG_HISI_HHEE is not set
/^CONFIG_HISI_HHEE_TOKEN=y$/c\# CONFIG_HISI_HHEE_TOKEN is not set
/^CONFIG_HISI_DIEID=y$/c\# CONFIG_HISI_DIEID is not set
/^CONFIG_HISI_SUBPMU=y$/c\# CONFIG_HISI_SUBPMU is not set
/^CONFIG_TEE_ANTIROOT_CLIENT=y$/c\# CONFIG_TEE_ANTIROOT_CLIENT is not set

/^CONFIG_HWAA=y$/c\# CONFIG_HWAA is not set' merge_hi3660_defconfig

```

Replace the last `merge_hi3660_defconfig` with your configuration file

#### Optional Part:

Change

```text

# CONFIG_SECURITY_SELINUX_DEVELOP is not set

```

to

```CONFIG_SECURITY_SELINUX_DEVELOP=y

``` This will enable SELinux on the phone to be in Permissive state upon boot.

#### Disable AVB Verification:

```text
CONFIG_DM_VERITY=y
CONFIG_DM_VERITY_AVB=y

```

Change to
```text

# CONFIG_DM_VERITY=y is not set

# CONFIG_DM_VERITY_AVB=y is not set

```

## Integrating KernelSU

#### 1. Pull Source Code

```bash
curl -LSs "https://raw.githubusercontent.com/tiann/KernelSU/main/kernel/setup.sh" | bash -s v0.9.2

```

:::tip The official KernelSU v0.9.5 source code conflicts with the kernel code; you need to use version v0.9.2 to pull the source code.

:::

#### 2. Enable KernelSU

Add the following lines to the end of your device's defconfig configuration file:

``text
# KernelSU
CONFIG_KSU=y

```

To enable KernelSU's debug mode, you also need to add:

```text
CONFIG_KSU_DEBUG=y

```

#### 3. Apply the Patch

Refer to the [KernelSU official website](https://kernelsu.org/zh_CN/guide/how-to-integrate-for-non-gki.html##modify-kernel-source-code) for modification.

Note: If your kernel does not have vfs_statx and do_faccessat, do not copy the general code above; use the code provided below. Do not ignore it!

#### 4. Modify hooks.c to enable the module

Refer to [this Github Commit](https://github.com/sticpaper/android_kernel_xiaomi_msm8998-ksu/commit/09a4672c0f521bf6b05daf24b207b125830a6fc5)

## Integrating RKSU/KernelSU-Next

### 1. Pull the source code

:::code-group

```RKSU
curl -LSs "https://raw.githubusercontent.com/rsuntk/KernelSU/main/kernel/setup.sh" | bash -s main

```
```KernelSU-Next
curl -LSs "https://raw.githubusercontent.com/KernelSU-Next/KernelSU-Next/next/kernel/setup.sh" | bash -s legacy

```

:::

### 2. Enable RKSU/KernelSU-Next

Add the following lines to the end of your device's defconfig configuration file:

```text

# KernelSU

CONFIG_KSU=y

CONFIG_KSU_MANUAL_HOOK=y

```

To enable KernelSU's debug mode, you also need to add:

```text

CONFIG_KSU_D