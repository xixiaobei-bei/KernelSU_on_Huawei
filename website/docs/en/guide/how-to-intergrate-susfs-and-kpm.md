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

## Disable AVB Verification:

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
Compatible with ReSukiSU (other KernelSU branches should also support it theoretically, please test on your own).

If you are using the SuSFS v2.0.0 patch for the 4.9 kernel, KPM is already integrated. You do not need to apply the patch again, but you need to add `CONFIG_KPM=y` to the defconfig file and patch the kernel to enable KPM.

:::

::: details Integrating for 4.9 Kernel

Clone ReSukiSU and run the following command in the kernel source directory:

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
```

Navigate to the `SuSFS_Patches` folder in this repository, then enter the `v4.9.xxx` folder.  
Copy the patch to the root of the source code, then enter the following in the terminal:

```shell
patch -p1 < SuSFS_inline_hook_2.2.0.patch
```

The patch will be applied. If patching fails, you will see generated error files ending with `.rej`.  
You can manually apply the patch based on the error locations indicated in the `.rej` files.

:::

::: details Legacy - Integrating SuSFS v2.0.0 for 4.9 Kernel

ReSukiSU needs to use an older version. Run the following commands in the kernel source directory:

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
cd KernelSU
git reset --hard 0d096be
```

Navigate to the `SuSFS_Patches` folder in this repository, then enter the `v4.9.xxx/Legacy` folder.  
Copy the patch to the root of the source code, then enter the following in the terminal:

```shell
patch -p1 < SuSFS_KPM_inline_hook_2.0.0.patch
```

The patch will be applied. If patching fails, you will see generated error files ending with `.rej`.  
You can manually apply the patch based on the error locations indicated in the `.rej` files.

:::

::: details Integrating for 4.14 Kernel

Since the 4.14 kernel only has SuSFS v2.0.0, ReSukiSU also needs to use an older version. Run the following commands in the kernel source directory:

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
cd KernelSU
git reset --hard 0d096be
```

Then navigate to the `SuSFS_Patches` folder in this repository, and enter the `v4.14.xxx` folder.  
Copy the patch to the root of the source code, then enter the following in the terminal:

```shell
patch -p1 < SuSFS_KPM_inline_hook_2.0.0.patch
```

The patch will be applied. If patching fails, you will see generated error files ending with `.rej`.  
You can manually apply the patch based on the error locations indicated in the `.rej` files.

:::

Then add the following to the kernel defconfig:

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

### Applying Kernel Patches

:::warning

KPM is currently only compatible with v4.9.xxx (EMUI 9+). Applying subsequent kernel patches may cause devices with v4.14.xxx (EMUI 10+) kernels to freeze.  
This is presumably caused by EMUI 10 and above systems shielding USER LOCK.

:::

:::tip

If you are using the SuSFS v2.0.0 patch for the 4.9 kernel, KPM is already integrated and you do not need to apply the patch again.

:::

Navigate to the `SuSFS_Patches` folder in this repository, then enter the `v4.9.xxx` folder.  
Copy the `KPM.patch` to the root of the source code, then enter the following in the terminal:

```shell
git apply --reject 0001-susfs-4.9.xxx-KPM-backport-and-Susfs-v2.0.0-inline-h.patch
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