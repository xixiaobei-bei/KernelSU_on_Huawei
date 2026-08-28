# 构建带SuSFS和KPM的内核

## 更改内核

:::tip

defconfig配置文件在 内核源码目录/arch/arm64/configs 文件夹下的defconfig结尾的文件，可能有多个，根据你的手机型号来

:::
### 需要关闭defconfig配置文件的以下选项：

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

将最后的`merge_hi3660_defconfig`替换为你的配置文件

### 可选部分： 

把

```text
# CONFIG_SECURITY_SELINUX_DEVELOP is not set
```

改为

```
CONFIG_SECURITY_SELINUX_DEVELOP=y
```
可以让开机的时候手机SELinux为Permissive状态。

## 关闭AVB验证：

```text
CONFIG_DM_VERITY=y  
CONFIG_DM_VERITY_AVB=y
```

改为

```text
# CONFIG_DM_VERITY is not set  
# CONFIG_DM_VERITY_AVB is not set 
```

### 修改hook.c以修复无法获取root

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

目前
SuSFS v2.0.0支持内核v4.14.xxx(EMUI 10+)和v4.9.xxx(EMUI9+)
SuSFS v2.2.0仅支持内核v4.9.xxx(EMUI9+)。  
兼容ReSukiSU（其他KernelSU分支理论也支持，请自行测试）

若你使用的4.9内核的SuSFS2.0.0版本补丁，则已经自带集成了KPM，你无需再次打补丁，但是需要在defconfig文件加入`CONFIG_KPM=y`，然后修补内核以启用KPM
:::

::: details 为4.9内核集成

拉取ReSukiSU，在内核源码目录执行：

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
```

进入此仓库的`SuSFS_Patches`文件夹，进入`v4.9.xxx`文件夹。  
复制补丁到源码根目录，在终端输入：

```shell
patch -p1 < SuSFS_inline_hook_2.2.0.patch
```

你就能打上补丁，如果打补丁失败，你能看到生成的错误文件，文件按照xxxx.rej结尾。  
你可以跟据.rej文件错误位置，手动打补丁。 

:::

::: details Legacy-为4.9内核集成v2.0.0版本SuSFS

ReSukiSU需要使用老版本。在内核源码目录执行：

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
cd KernelSU
git reset --hard 0d096be
```

进入此仓库的`SuSFS_Patches`文件夹，进入`v4.9.xxx/Legacy`文件夹。  
复制补丁到源码根目录，在终端输入：

```shell
patch -p1 < SuSFS_KPM_inline_hook_2.0.0.patch
```

你就能打上补丁，如果打补丁失败，你能看到生成的错误文件，文件按照xxxx.rej结尾。  
你可以跟据.rej文件错误位置，手动打补丁。 

:::

::: details 为4.14内核集成

由于4.14内核只有v2.0.0的SuSFS，所以ReSukiSU也需要使用老版本。在内核源码目录执行：

```shell
curl -LSs "https://raw.githubusercontent.com/ReSukiSU/ReSukiSU/main/kernel/setup.sh" | bash
cd KernelSU
git reset --hard 0d096be
```

然后进入此仓库的`SuSFS_Patches`文件夹，进入`v4.14.xxx`文件夹。  
复制补丁到源码根目录，在终端输入：

```shell
patch -p1 < SuSFS_KPM_inline_hook_2.0.0.patch
```

你就能打上补丁，如果打补丁失败，你能看到生成的错误文件，文件按照xxxx.rej结尾。  
你可以跟据.rej文件错误位置，手动打补丁。

:::

## KPM

### 打内核补丁

:::warning

KPM目前只能和v4.9.xxx(EMUI9+)兼容，因为后续给内核打补丁的操作会让v4.14.xxx(EMUI 10+)内核的设备死机。  
推测是EMUI10及其以上的系统屏蔽USER LOCK导致的。  

:::

:::tip

若你使用的4.9内核的SuSFS2.0.0版本补丁，则已经自带集成了KPM，你无需再次打补丁

:::

进入此仓库的`SuSFS_Patches`文件夹，进入`v4.9.xxx`文件夹。  
复制补丁`KPM.patch`到源码根目录，在终端输入：

```shell
git apply --reject 0001-susfs-4.9.xxx-KPM-backport-and-Susfs-v2.0.0-inline-h.patch
```

你就能打上补丁，如果打补丁失败，你能看到生成的错误文件，文件按照xxxx.rej结尾。  
你可以跟据.rej文件错误位置，手动打补丁。  

然后，你需要在内核defconfig中加入：

```text
CONFIG_KPM=y
```

随后编译内核，但不要打包，再按照下方的步骤修补内核

### 修补内核

编译出内核，此时的内核还不支持KPM，还需要修补内核才能启用KPM

进入SukiSU-Ultra/SukiSU's KernelPatch patch仓库的[Releases](https://github.com/SukiSU-Ultra/SukiSU_KernelPatch_patch/releases)页面，找到最新的Release，下载patch_linux文件，放入`内核源码/out/arch/arm64/boot`目录下。然后在这个目录打开终端并输入：

```shell
chmod +x patch_linux
patch_linux
rm -rf Image.gz
mv -f oImage Image
gzip -k Image
```

此时该目录就会出现一个新的Image.gz文件，使用这个文件打包即可。