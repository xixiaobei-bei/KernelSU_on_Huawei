# 如何构建

感谢大佬们的辛苦奉献，我只不过是站在了巨人的肩膀上而已

defconfig配置文件在 内核源码目录/arch/arm64/configs 文件夹下的defconfig结尾的文件，可能有多个，根据你的手机型号来

## 安装依赖

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

:::tip
华为只能用Aarch64版本
:::

## 获取源码

[华为的官方源码](https://consumer.huawei.com/en/opensource/)

在其中输入设备型号代码，下载后打开```Code_opensource\kernel```文件夹，这为你的内核源码，解压出来即可

:::warning
由于编码原因，只能在Linux环境下解压到非ntfs/fat分区，否则会无法编译
:::

#### 其他设备可寻找第三方ROM的内核源码或在互联网搜索

## 更改内核

#### 需要关闭defconfig配置文件的以下选项：

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
# CONFIG_XXXXXX is not set
```

自动替换命令：
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

将最后的merge_hi3660_defconfig替换为你的配置文件

#### 可选部分： 

把

```text
# CONFIG_SECURITY_SELINUX_DEVELOP is not set
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
# CONFIG_DM_VERITY=y is not set  
# CONFIG_DM_VERITY_AVB=y is not set 
```

## 集成KernelSU/KernelSU-Next

#### 1.拉取源码

:::code-group

```KernelSU
curl -LSs "https://raw.githubusercontent.com/tiann/KernelSU/main/kernel/setup.sh" | bash -s v0.9.2
```

```KernelSU-Next
curl -LSs "https://raw.githubusercontent.com/KernelSU-Next/KernelSU-Next/next/kernel/setup.sh" | bash -
```

:::

:::tip
KernelSU官方的v0.9.5的源码与内核代码冲突，拉取源码要用v0.9.2版本
:::

#### 2.启用KernelSU

在你的设备defconfig配置文件最后加入以下几行：

```text
# KernelSU
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

## 集成SukiSU-Ultra

### 1.拉取源码

```bash
curl -LSs "https://raw.githubusercontent.com/SukiSU-Ultra/SukiSU-Ultra/main/kernel/setup.sh" | bash -s builtin
```

### 2.应用补丁

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

```diff[hooks.c]
diff -ruN a/security/selinux/hooks.c b/security/selinux/hooks.c
--- a/security/selinux/hooks.c	2019-02-27 18:31:35.000000000 +0800
+++ b/security/selinux/hooks.c	2026-01-28 17:58:56.593108000 +0800
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

### 3.修改内核以启用SukiSU-Ultra

在你的设备defconfig配置文件最后加入以下几行：

```text
# KernelSU
CONFIG_KSU=y
CONFIG_KSU_MANUAL_HOOK=y
CONFIG_KSU_MANUAL_SU=y
# CONFIG_KSU_SUSFS is not set
```

若要开启SukiSU-Ultra的调试模式，还需要加入：

```text
CONFIG_KSU_DEBUG=y
```

## 编译

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
make ARCH=arm64 O=out defconfig配置文件名字(只要名字，不要路径)  
make ARCH=arm64 O=out -j8
```

这里的-j8指定编译的CPU核心数，要你的CPU核心数X2

#### 4.编译完后打包

编译完后在out/arch/arm64/boot/路径下会有一个Image.gz文件，复制到内核根目录的tools文件夹

更改pack_kernerimage_cmd.sh文件内容为：

Selinux的Enforcing状态版本：

```sh
##!/bin/bash 
./mkbootimg --kernel Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07C00000 --header_version 1 --os_version 9 --os_patch_level 2020-01-01  --output kernel.img
```

Selinux的Permissive状态版本：

```sh
##!/bin/bash 
./mkbootimg --kernel Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07C00000 --header_version 1 --os_version 9 --os_patch_level 2020-01-01  --output kernel.img
```

###### 里面的内核编译时间需要更改

随后运行：

```bash
bash pack_kernerimage_cmd.sh
```

然后本目录会多出一个kernel.img文件，fastboot刷入kernel分区即可