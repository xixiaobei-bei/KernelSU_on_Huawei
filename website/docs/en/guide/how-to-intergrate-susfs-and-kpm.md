# Building a Kernel with SuSFS and KPM

## Modifying the Kernel

:::tip

The defconfig configuration file is located in the `kernel_source_dir/arch/arm64/configs` directory. There may be multiple files ending with `defconfig`. Choose the one that matches your phone model.

:::

### The following options need to be disabled in the defconfig file:

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

These entries need to be changed to the following format:

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

Replace the final `merge_hi3660_defconfig` with your configuration file name.

### Optional:

Change

```text
# CONFIG_SECURITY_SELINUX_DEVELOP is not set
```

to

```
CONFIG_SECURITY_SELINUX_DEVELOP=y
```

This will make the phone's SELinux status Permissive at boot.

### Disable AVB Verification:

```text
CONFIG_DM_VERITY=y  
CONFIG_DM_VERITY_AVB=y
```

Change to

```text
# CONFIG_DM_VERITY is not set  
# CONFIG_DM_VERITY_AVB is not set 
```

### Modifying hook.c to Fix Root Access Issues

```diff
diff -uNr a/security/selinux/hooks.c b/security/selinux/hooks.c
--- a/security/selinux/hooks.c	2019-02-27 18:31:35.000000000 +0800
+++ b/security/selinux/hooks.c	2026-08-28 13:30:21.276759515 +0800
@@ -2291,16 +2291,37 @@
 			    const struct task_security_struct *old_tsec,
 			    const struct task_security_struct *new_tsec)
 {
+#ifdef CONFIG_KSU
+    static u32 ksu_sid;
+    char *secdata;
+#endif
 	int nnp = (bprm->unsafe & LSM_UNSAFE_NO_NEW_PRIVS);
 	int nosuid = !mnt_may_suid(bprm->file->f_path.mnt);
 	int rc;
 
+#ifdef CONFIG_KSU
+    int error;
+    u32 seclen;
+#endif
 	if (!nnp && !nosuid)
 		return 0; /* neither NNP nor nosuid */
 
 	if (new_tsec->sid == old_tsec->sid)
 		return 0; /* No change in credentials */
 
+#ifdef CONFIG_KSU
+    if (!ksu_sid)
+        security_secctx_to_secid("u:r:su:s0", strlen("u:r:su:s0"), &ksu_sid);
+
+    error = security_secid_to_secctx(old_tsec->sid, &secdata, &seclen);
+    if (!error) {
+        rc = strcmp("u:r:init:s0", secdata);
+        security_release_secctx(secdata, seclen);
+        if (rc == 0 && new_tsec->sid == ksu_sid)
+            return 0;
+    }
+#endif
+
 	/*
 	 * The only transitions we permit under NNP or nosuid
 	 * are transitions to bounded SIDs, i.e. SIDs that are
```

## SuSFS

:::tip

Currently:
SuSFS v2.0.0 supports kernel v4.14.xxx (EMUI 10+) and v4.9.xxx (EMUI 9+).
SuSFS v2.2.0 only supports kernel v4.9.xxx (EMUI 9+).  
Compatible with ReSukiSU and SukiSU-Ultra (other KernelSU branches should also support it theoretically, please test on your own).

If you are using the SuSFS v2.0.0 patch for the 4.9 kernel, KPM is already integrated. You do not need to apply the patch again, but you need to add `CONFIG_KPM=y` to the defconfig file and patch the kernel to enable KPM.

:::

:::warning

The new version of ReSukiSU has removed KPM support. If you still need to integrate KPM, please use an older version of ReSukiSU + SuSFS v2.0.0 patch, or use SukiSU-Ultra.

:::

::: details Patching for 4.9 Kernel

To use ReSukiSU, run the following command in the kernel source directory:

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
```

To use SukiSU-Ultra, run the following command in the kernel source directory:

```shell
curl -LSs "https://raw.githubusercontent.com/SukiSU-Ultra/SukiSU-Ultra/main/kernel/setup.sh" | bash -s builtin
```

Navigate to the `SuSFS_Patches` folder in this repository, then enter the `v4.9.xxx` folder.  
Copy the patch to the root of the source code, then enter the following in the terminal:

```shell
patch -p1 < SuSFS_inline_hook_2.2.0.patch
```

The patch will be applied. If patching fails, you will see generated error files ending with `.rej`.  
You can manually apply the patch based on the error locations indicated in the `.rej` files.

:::

::: details Legacy - Applying SuSFS v2.0.0 Patch for 4.9 Kernel

ReSukiSU needs to use an older version. Run the following commands in the kernel source directory:

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
cd KernelSU
git reset --hard a13d71f
```

Navigate to the `SuSFS_Patches` folder in this repository, then enter the `v4.9.xxx/Legacy` folder.  
Copy the patch to the root of the source code, then enter the following in the terminal:

```shell
patch -p1 < SuSFS_KPM_inline_hook_2.0.0.patch
```

The patch will be applied. If patching fails, you will see generated error files ending with `.rej`.  
You can manually apply the patch based on the error locations indicated in the `.rej` files.

:::

::: details Patching for 4.14 Kernel

Since the 4.14 kernel only has SuSFS v2.0.0, ReSukiSU also needs to use an older version. Run the following commands in the kernel source directory:

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
cd KernelSU
git reset --hard a13d71f
```

Then navigate to the `SuSFS_Patches` folder in this repository, and enter the `v4.14.xxx` folder.  
Copy the patch to the root of the source code, then enter the following in the terminal:

```shell
patch -p1 < SuSFS_KPM_inline_hook_2.0.0.patch
```

The patch will be applied. If patching fails, you will see generated error files ending with `.rej`.  
You can manually apply the patch based on the error locations indicated in the `.rej` files.

:::

Then add the following to the defconfig:

```text
# ReSukiSU - SuSFS
CONFIG_KSU=y
CONFIG_KSU_SUSFS=y
CONFIG_KSU_SUSFS_SUS_PATH=y
CONFIG_KSU_SUSFS_SUS_MOUNT=y
CONFIG_KSU_SUSFS_SUS_KSTAT=y
CONFIG_KSU_SUSFS_SPOOF_UNAME=y
CONFIG_KSU_SUSFS_ENABLE_LOG=y
CONFIG_KSU_SUSFS_HIDE_KSU_SUSFS_SYMBOLS=y
CONFIG_KSU_SUSFS_SPOOF_CMDLINE_OR_BOOTCONFIG=y
CONFIG_KSU_SUSFS_OPEN_REDIRECT=y
CONFIG_KSU_SUSFS_SUS_MAP=y
```

## KPM

### Pull Source Code

:::tip

If you have already integrated SuSFS and pulled the source code, you can skip this step.

:::

To use ReSukiSU, run the following commands in the kernel source directory:

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
cd KernelSU
git reset --hard a13d71f
```

To use SukiSU-Ultra, run the following command in the kernel source directory:

```shell
curl -LSs "https://raw.githubusercontent.com/SukiSU-Ultra/SukiSU-Ultra/main/kernel/setup.sh" | bash -s builtin
```

### Applying KernelSU Patch

:::tip

If you are using SuSFS, you can skip this step.

:::

:::details Patching for SukiSU-Ultra

You can revert `path_umount` via the [KernelSU official website](https://kernelsu.org/guide/how-to-integrate-for-non-gki.html#manually-modify-the-kernel-source) to obtain the module unload capability.

Add the following lines at the end of your device's defconfig file:

```text
# KernelSU
CONFIG_KSU=y
CONFIG_KSU_MANUAL_HOOK=y
CONFIG_KSU_MANUAL_SU=y
```

To enable debug mode for SukiSU-Ultra, you also need to add:

```text
CONFIG_KSU_DEBUG=y
```

Then apply the following patches:

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

:::details Patching for ReSukiSU

Add the following lines at the end of your device's defconfig file:

```text
# ReSukiSU
CONFIG_KSU=y
CONFIG_KSU_MANUAL_HOOK=y
```

To enable debug mode for KernelSU, you also need to add:

```text
CONFIG_KSU_DEBUG=y
```

Then refer to the [ReSukiSU official website](https://resukisu.github.io/guide/manual-integrate.html) for modifications.

:::

### Applying KPM Patch

:::warning

KPM is currently only compatible with v4.9.xxx (EMUI 9+). Applying subsequent kernel patches may cause devices with v4.14.xxx (EMUI 10+) kernels to freeze.  
This is presumably caused by EMUI 10 and above systems shielding USER LOCK.

:::

:::tip

If you are using the SuSFS v2.0.0 patch for the 4.9 kernel, KPM is already integrated and you do not need to apply the patch again.

:::

:::warning

The new version of ReSukiSU has removed KPM support. If you need to integrate KPM, please use an older version of ReSukiSU or SukiSU-Ultra.

:::

Then navigate to the `SuSFS_Patches` folder in this repository, and enter the `v4.9.xxx` folder.  
Copy the `KPM.patch` to the root of the source code, then enter the following in the terminal:

```shell
patch -p1 < KPM.patch
```

The patch will be applied. If patching fails, you will see generated error files ending with `.rej`.  
You can manually apply the patch based on the error locations indicated in the `.rej` files.

Then, you need to add the following to the kernel defconfig:

```text
CONFIG_KPM=y
```

Then compile the kernel, but do not package it. Follow the steps below to patch the kernel.

### Patching the Kernel

Compile the kernel. At this point the kernel does not yet support KPM, so you need to patch the kernel to enable KPM.

Go to the [Releases](https://github.com/SukiSU-Ultra/SukiSU_KernelPatch_patch/releases) page of the SukiSU-Ultra/SukiSU's KernelPatch patch repository, find the latest release, download the `patch_linux` file, and place it in the `kernel_source_dir/out/arch/arm64/boot` directory. Then open a terminal in that directory and enter:

```shell
chmod +x patch_linux
patch_linux
rm -rf Image.gz
mv -f oImage Image
gzip -k Image
```

At this point, a new `Image.gz` file will appear in this directory. Use this file for packaging.