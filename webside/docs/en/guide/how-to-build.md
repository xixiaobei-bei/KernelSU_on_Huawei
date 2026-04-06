# Build kernel without SuSFS

:::warning

The website is under construction. It is not recommended to integrate on it at this time.

:::

## Install Dependencies

:::warning

Due to the global definition issue of `yylloc` in higher versions of Ubuntu/Debian, it is recommended to use only Ubuntu 18.04 to Ubuntu 22.04 (below Debian 13, these two versions are recommended).

If your system version is greater than 20.04, you need to install GCC9 and set it as the default:

```shell
sudo apt install gcc-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 100
```

:::

#### 1. Install the packages required for compilation

Run:

```shell
sudo apt-get install libncurses5-dev libncurses-dev libssl-dev device-tree-compiler bc cpio lib32ncurses5-dev lib32z1 build-essential binutils bc bison build-essential ccache curl flex g++-multilib gcc-multilib git gnupg gperf imagemagick lib32ncurses5-dev lib32readline-dev lib32z1-dev liblz4-tool libncurses5 libncurses5-dev libsdl1.2-dev libssl-dev libxml2 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev git python2

```

#### 2. Obtain GCC

```shell
git clone --depth=1 https://kkgithub.com/LineageOS/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9 aarch64-linux-android-4.9

```

:::tip

Huawei can only use the Aarch64 version

:::

## Obtaining the Source Code

[Huawei's Official Source Code](https://consumer.huawei.com/en/opensource/)

Enter your device model code, download, and open the ```Code_opensource\kernel``` folder. This is your kernel source code; extract it.

:::warning

Due to encoding limitations, it can only be extracted to a non-NTFS/FAT partition in a Linux environment; otherwise, compilation will fail.

:::

#### For other devices, you can find the kernel source code of third-party ROMs or search on the internet.

## Modifying the Kernel

:::tip

The defconfig configuration file is located in the kernel source directory /arch/arm64/configs folder, and there may be multiple files ending in defconfig, depending on your phone model.

:::
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

```shell
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

```text
CONFIG_SECURITY_SELINUX_DEVELOP=y
```

This will enable SELinux on the phone to be in Permissive state upon boot.

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

```shell
curl -LSs "https://raw.githubusercontent.com/tiann/KernelSU/main/kernel/setup.sh" | bash -s v0.9.2
```

:::tip

The official KernelSU v0.9.5 source code conflicts with the kernel code; you need to use version v0.9.2 to pull the source code.

:::

#### 2. Enable KernelSU

Add the following lines to the end of your device's defconfig configuration file:

```text
# KernelSU
CONFIG_KSU=y
```

To enable KernelSU's debug mode, you also need to add:

```text
CONFIG_KSU_DEBUG=y
```

#### 3. Apply the Patch

Refer to the [KernelSU official website](https://kernelsu.org/zh_CN/guide/how-to-integrate-for-non-gki.html##modify-kernel-source-code) for modification.

:::warning

If your kernel does not have vfs_statx and do_faccessat, do not copy the general code above; use the code provided below. Do not ignore it!

:::

#### 4. Modify hooks.c to enable the module

Refer to [this Github Commit](https://github.com/sticpaper/android_kernel_xiaomi_msm8998-ksu/commit/09a4672c0f521bf6b05daf24b207b125830a6fc5)

## Integrating KernelSU-Next

#### 1. Pull Source Code

```shell
curl -LSs "https://raw.githubusercontent.com/KernelSU-Next/KernelSU-Next/next/kernel/setup.sh" | bash -s legacy
```

#### 2. Enable KernelSU

Add the following lines to the end of your device's defconfig configuration file:

```text
# KernelSU
CONFIG_KSU=y
```

To enable KernelSU's debug mode, you also need to add:

```text
CONFIG_KSU_DEBUG=y
```

#### 3. Apply the Patch

Refer to the [KernelSU-Next official website](https://kernelsu-next.github.io/webpage/pages/how-to-integrate-for-non-gki.html#manually-modify-the-kernel-source) for modification.

In addition, you can also backport path_umount via the [KernelSU offical webside](https://kernelsu.org/guide/how-to-integrate-for-non-gki.html#manually-modify-the-kernel-source) to enable the module unmount feature.

## Integrating RKSU

### 1. Pull the source code

```shell
curl -LSs "https://raw.githubusercontent.com/rsuntk/KernelSU/main/kernel/setup.sh" | bash -s main
```

### 2.Enable RKSU

Add the following lines to the end of your device's defconfig file:

```text
# KernelSU
CONFIG_KSU=y
CONFIG_KSU_MANUAL_HOOK=y
```

To enable KernelSU debug mode, you also need to add:

```text
CONFIG_KSU_DEBUG=y
```

### 3.Apply patches

[4.4-4.9 kernel version](https://github.com/rksuorg/kernel_patches/blob/master/manual_hook/kernel-4.4_4.9.patch)

[4.14 kernel version](https://github.com/rksuorg/kernel_patches/blob/master/manual_hook/kernel-4.14.patch)

In addition, you can also backport path_umount via the [KernelSU offical webside](https://kernelsu.org/guide/how-to-integrate-for-non-gki.html#manually-modify-the-kernel-source) to enable the module unmount feature.

## Integrate SukiSU-Ultra

### 1.Pull the Source Code

```shell
curl -LSs "https://raw.githubusercontent.com/SukiSU-Ultra/SukiSU-Ultra/main/kernel/setup.sh" | bash -s builtin
```

### 2.Apply patches

:::code-group

```diff[exec.c]
diff -ruN a/fs/exec.c b/fs/exec.c
--- a/fs/exec.c	2019-02-27 18:31:35.000000000 +0800
+++ b/fs/exec.c	2026-01-28 17:58:55.317129000 +0800
@@ -1907,11 +1907,21 @@
 	} while (cmpxchg(&mm->flags, old, new) != old);
 }
 
+#ifdef CONFIG_KSU
+__attribute__((hot))
+extern int ksu_handle_execve_sucompat(int *fd, const char __user **filename_user,
+			       void *__never_use_argv, void *__never_use_envp,
+			       int *__never_use_flags);
+#endif
+
 SYSCALL_DEFINE3(execve,
 		const char __user *, filename,
 		const char __user *const __user *, argv,
 		const char __user *const __user *, envp)
 {
+#ifdef CONFIG_KSU
+	ksu_handle_execve_sucompat((int *)AT_FDCWD, &filename, NULL, NULL, NULL);
+#endif
 	return do_execve(getname(filename), argv, envp);
 }
 
@@ -1933,6 +1943,9 @@
 	const compat_uptr_t __user *, argv,
 	const compat_uptr_t __user *, envp)
 {
+#ifdef CONFIG_KSU
+	ksu_handle_execve_sucompat((int *)AT_FDCWD, &filename, NULL, NULL, NULL);
+#endif
 	return compat_do_execve(getname(filename), argv, envp);
 }
 
```

```diff[open.c]
diff -ruN a/fs/open.c b/fs/open.c
--- a/fs/open.c	2019-02-27 18:31:35.000000000 +0800
+++ b/fs/open.c	2026-01-28 17:58:55.473127000 +0800
@@ -360,6 +360,12 @@
  * We do this by temporarily clearing all FS-related capabilities and
  * switching the fsuid/fsgid around to the real ones.
  */
+#ifdef CONFIG_KSU
+__attribute__((hot))
+extern int ksu_handle_faccessat(int *dfd, const char __user **filename_user,
+				int *mode, int *flags);
+#endif
+
 SYSCALL_DEFINE3(faccessat, int, dfd, const char __user *, filename, int, mode)
 {
 	const struct cred *old_cred;
@@ -370,6 +376,9 @@
 	int res;
 	unsigned int lookup_flags = LOOKUP_FOLLOW;
 
+#ifdef CONFIG_KSU
+	ksu_handle_faccessat(&dfd, &filename, &mode, NULL);
+#endif
 	if (mode & ~S_IRWXO)	/* where's F_OK, X_OK, W_OK, R_OK? */
 		return -EINVAL;
```

```diff[read_write.c]
diff -ruN a/fs/read_write.c b/fs/read_write.c
--- a/fs/read_write.c	2019-02-27 18:31:35.000000000 +0800
+++ b/fs/read_write.c	2026-01-28 17:58:55.497126000 +0800
@@ -610,11 +610,22 @@
 	file->f_pos = pos;
 }
 
+#ifdef CONFIG_KSU
+// extern bool ksu_vfs_read_hook __read_mostly;
+bool ksu_vfs_read_hook __read_mostly = true;  // fix compiler ghost define
+extern __attribute__((cold)) int ksu_handle_sys_read(unsigned int fd,
+			char __user **buf_ptr, size_t *count_ptr);
+#endif
 SYSCALL_DEFINE3(read, unsigned int, fd, char __user *, buf, size_t, count)
 {
 	struct fd f = fdget_pos(fd);
 	ssize_t ret = -EBADF;
 
+	
+#ifdef CONFIG_KSU
+	if (unlikely(ksu_vfs_read_hook))
+		ksu_handle_sys_read(fd, &buf, &count);
+#endif
 	if (f.file) {
 		loff_t pos = file_pos_read(f.file);
 		ret = vfs_read(f.file, buf, count, &pos);
```

```diff[stat.c]
diff -ruN a/fs/stat.c b/fs/stat.c
--- a/fs/stat.c	2019-02-27 18:31:35.000000000 +0800
+++ b/fs/stat.c	2026-01-28 17:58:55.509126000 +0800
@@ -287,6 +287,12 @@
 	return cp_new_stat(&stat, statbuf);
 }
 
+#ifdef CONFIG_KSU
+__attribute__((hot))
+extern int ksu_handle_stat(int *dfd, const char __user **filename_user,
+				int *flags);
+#endif
+
 #if !defined(__ARCH_WANT_STAT64) || defined(__ARCH_WANT_SYS_NEWFSTATAT)
 SYSCALL_DEFINE4(newfstatat, int, dfd, const char __user *, filename,
 		struct stat __user *, statbuf, int, flag)
@@ -294,6 +300,9 @@
 	struct kstat stat;
 	int error;
 
+#ifdef CONFIG_KSU
+	ksu_handle_stat(&dfd, &filename, &flag);
+#endif
 	error = vfs_fstatat(dfd, filename, &stat, flag);
 	if (error)
 		return error;
@@ -436,6 +445,9 @@
 	struct kstat stat;
 	int error;
 
+#ifdef CONFIG_KSU
+	ksu_handle_stat(&dfd, &filename, &flag);
+#endif
 	error = vfs_fstatat(dfd, filename, &stat, flag);
 	if (error)
 		return error;
```

```diff[input.c]
diff -ruN a/drivers/input/input.c b/drivers/input/input.c
--- a/drivers/input/input.c	2019-02-27 18:31:32.000000000 +0800
+++ b/drivers/input/input.c	2026-01-28 17:58:52.537176000 +0800
@@ -425,11 +425,21 @@
  * to 'seed' initial state of a switch or initial position of absolute
  * axis, etc.
  */
+#ifdef CONFIG_KSU
+extern bool ksu_input_hook __read_mostly;
+extern __attribute__((cold)) int ksu_handle_input_handle_event(
+			unsigned int *type, unsigned int *code, int *value);
+#endif
 void input_event(struct input_dev *dev,
 		 unsigned int type, unsigned int code, int value)
 {
 	unsigned long flags;
 
+	
+#ifdef CONFIG_KSU
+	if (unlikely(ksu_input_hook))
+		ksu_handle_input_handle_event(&type, &code, &value);
+#endif
 	if (is_event_supported(type, dev->evbit, EV_MAX)) {
 
 		spin_lock_irqsave(&dev->event_lock, flags);
```

```diff[reboot.c]
diff -ruN a/kernel/reboot.c b/kernel/reboot.c
--- a/kernel/reboot.c	2019-02-27 18:31:34.000000000 +0800
+++ b/kernel/reboot.c	2026-01-28 17:58:56.149115000 +0800
@@ -277,12 +277,19 @@
  *
  * reboot doesn't sync: do that yourself before calling this.
  */
+#ifdef CONFIG_KSU
+extern int ksu_handle_sys_reboot(int magic1, int magic2, unsigned int cmd, void __user **arg);
+#endif
 SYSCALL_DEFINE4(reboot, int, magic1, int, magic2, unsigned int, cmd,
 		void __user *, arg)
 {
 	struct pid_namespace *pid_ns = task_active_pid_ns(current);
 	char buffer[256];
 	int ret = 0;
+#ifdef CONFIG_KSU
+	ksu_handle_sys_reboot(magic1, magic2, cmd, &arg);
+#endif
+
 
 	/* We only trust the superuser with rebooting the system. */
 	if (!ns_capable(pid_ns->user_ns, CAP_SYS_BOOT))
```

```diff[sys.c]
diff -ruN a/kernel/sys.c b/kernel/sys.c
--- a/kernel/sys.c	2019-02-27 18:31:34.000000000 +0800
+++ b/kernel/sys.c	2026-01-28 17:58:56.161115000 +0800
@@ -609,6 +609,10 @@
  * This function implements a generic ability to update ruid, euid,
  * and suid.  This allows you to implement the 4.4 compatible seteuid().
  */
+#ifdef CONFIG_KSU
+extern int ksu_handle_setresuid(uid_t ruid, uid_t euid, uid_t suid);
+#endif
+
 SYSCALL_DEFINE3(setresuid, uid_t, ruid, uid_t, euid, uid_t, suid)
 {
 	struct user_namespace *ns = current_user_ns();
@@ -621,6 +625,11 @@
 	keuid = make_kuid(ns, euid);
 	ksuid = make_kuid(ns, suid);
 
+#ifdef CONFIG_KSU_SUSFS
+	if (ksu_handle_setresuid(ruid, euid, suid)) {
+		pr_info("Something wrong with ksu_handle_setresuid()\\n");
+	}
+#endif
 	if ((ruid != (uid_t) -1) && !uid_valid(kruid))
 		return -EINVAL;
```

```diff[Kconfig]
diff -ruN a/security/Kconfig b/security/Kconfig
--- a/security/Kconfig	2019-02-27 18:31:35.000000000 +0800
+++ b/security/Kconfig	2026-01-28 17:58:56.577108000 +0800
@@ -237,13 +237,5 @@
         help
           Protects the security huulks from further modifications, after init.
 
-source security/mdpp_selftest/Kconfig
-source security/hwselinux/Kconfig
-source security/kernel_harden/Kconfig
-source security/check_root/Kconfig
-source security/hw_root_scan/Kconfig
-source security/check_double_free/Kconfig
-source security/hkip_atkinfo/Kconfig
-source security/kernel_stp/Kconfig
 endmenu
```

```diff[security/Makefile]
diff -ruN a/security/Makefile b/security/Makefile
--- a/security/Makefile	2019-02-27 18:31:35.000000000 +0800
+++ b/security/Makefile	2026-01-28 17:58:56.577108000 +0800
@@ -9,7 +9,6 @@
 subdir-$(CONFIG_SECURITY_APPARMOR)	+= apparmor
 subdir-$(CONFIG_SECURITY_YAMA)		+= yama
 subdir-$(CONFIG_SECURITY_LOADPIN)	+= loadpin
-subdir-$(CONFIG_HKIP_ATKINFO)		+= hkip_atkinfo
 
 # always enable default capabilities
 obj-y					+= commoncap.o
@@ -26,18 +25,10 @@
 obj-$(CONFIG_SECURITY_YAMA)		+= yama/
 obj-$(CONFIG_SECURITY_LOADPIN)		+= loadpin/
 obj-$(CONFIG_CGROUP_DEVICE)		+= device_cgroup.o
-obj-$(CONFIG_HKIP_ATKINFO)		+= hkip_atkinfo/
 
 # Object integrity file lists
 subdir-$(CONFIG_INTEGRITY)		+= integrity
 obj-$(CONFIG_INTEGRITY)			+= integrity/
 
 # HW Object
-subdir-$(CONFIG_HUAWEI_SELINUX_DSM)	+= hwselinux
-obj-$(CONFIG_HUAWEI_SELINUX_DSM)	+= hwselinux/
-obj-$(CONFIG_HUAWEI_PROC_CHECK_ROOT)    += check_root/
-obj-$(CONFIG_HUAWEI_CRYPTO_TEST_MDPP) += mdpp_selftest/
-obj-$(CONFIG_HW_ROOT_SCAN) += hw_root_scan/
-obj-$(CONFIG_HW_DOUBLE_FREE_DYNAMIC_CHECK) += check_double_free/
-obj-$(CONFIG_HW_KERNEL_STP) += kernel_stp/
 include security/kernel_harden/Makefile
```

```diff[drivers/Makefile]
--- a	2026-01-28 21:33:24.492017000 +0800
+++ b	2026-01-28 21:34:12.088802416 +0800
@@ -214,4 +214,4 @@
 obj-$(CONFIG_HW_MEMORY_MONITOR) += allocpages_delayacct/
 obj-y += cfi/
 obj-$(CONFIG_HUAWEI_DUBAI) += dubai/
-obj-$(CONFIG_KSU) += kernelsu/
+obj-y += kernelsu/
```

:::

In addition, you can also backport path_umount via the [KernelSU offical webside](https://kernelsu.org/guide/how-to-integrate-for-non-gki.html#manually-modify-the-kernel-source) to enable the module unmount feature.

### 3.Enable SukiSU-Ultra

Add the following lines to the end of your device's defconfig file:

```text
# KernelSU
CONFIG_KSU=y
CONFIG_KSU_MANUAL_HOOK=y
CONFIG_KSU_MANUAL_SU=y
# CONFIG_KSU_SUSFS is not set
```

To enable SukiSU-Ultra debug mode, you also need to add:

```text
CONFIG_KSU_DEBUG=y
```

```text
CONFIG_KSU_DEBUG=y
```

## Integrate ReSukiSU

### 1.Pull the Sources Code

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
```

### 2.Enable ReSukiSU

Add the following lines to the end of your device's defconfig file:

```text
# KernelSU
CONFIG_KSU=y
CONFIG_KSU_MANUAL_HOOK=y
```

To enable KernelSU debug mode, you also need to add:

```text
CONFIG_KSU_DEBUG=y
```

### 3.Apply Patches

Refer to the [ReSukiSU offical webside](https://resukisu.github.io/guide/manual-integrate.html) to modify.

Even if your kernel version is lower than 4.19, you still need to use the version of the sys_read hook intended for 4.19+.

## Build

### 1.Set up default Python2

```shell
sudo ln -sf /usr/bin/python2.7 /usr/bin/python
```

### 2.Set environment variables.

```shell
export ARCH=arm64  
export PATH=$PATH:<Path to your GCC>
export CROSS_COMPILE=aarch64-linux-android-
```

### 3.Build

Run this command on your kernel root：

```shell
make ARCH=arm64 O=out <defconfig>(Only name,not path)  
make ARCH=arm64 O=out -j8
```

Here, `-j8` specifies the number of CPU cores to use for compilation; it should be set to your CPU core count × 2.

### 4.Package after compilation.

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