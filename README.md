# KernelSU on Huawei

[[简体中文](README.md)]  [[English](README_en.md)]

- 适用于华为的KernelSU内核！

- 现支持KernelSU、RKSU、KernelSU-Next、SukiSU-Ultra和ReSukiSU

- 感谢各位贡献作者，我只是站在了巨人的肩膀上

> [!CAUTION]
>
> Root 您的设备可能会使保修失效，如果操作不当可能会造成永久性损坏。 请务必在继续之前创建完整备份，阅读文档确保与您的设备兼容，遵循文档参考，准备好恢复计划。由操作不当导致的任何后果不由本项目作者(xixiaobei)及任何人负责！
>

## 下载

### 通过GithubReleases下载

可以在[此页面](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/releases)下载，选择最新Release找到对应文件点击下载即可

### 通过GithubAction下载开发版

> [!WARNING]
>
> 开发版没有被确定完全可用，可能包含未知的问题！
>

> [!NOTE]
>
> 通过GithubAction下载需要登陆Github账号才可以显示下载按钮
>

打开[本项目GithubAction页面](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/actions)，选择左侧Action列表的Build Kernel Release板块，选择最新编译任务，然后找到设备对应的文件下载

## 命名规则

```机型代码_底包版本_KernelSU类型_Selinux状态```

例如：

```MHA_EMUI9.0_KernelSU-Next_PM.img```

代表支持MHA型号EMUI9.0底包设备的KernelSU-Next的Selinux状态为宽容的内核

## 注意事项

- 若选择PM内核刷入，则需要先解密DATA分区才可开机

- 请选择对应底包的内核再刷入，否则会无法开机！

- KernelSU官方在1.0版本已经舍弃非GKI内核，所以需要使用 [v0.9.2的管理器](https://github.com/tiann/KernelSU/releases/download/v0.9.2/KernelSU_v0.9.2_11682-release.apk) ，其他KernelSU分支不受影响

## 刷入

1.解锁Bootloader

2.下载对应内核

3.手机进入fastboot模式，连接电脑

4.电脑上在ADB工具目录下输入以下命令

```bash
fastboot flash kernel <内核文件>
```

若是PM内核，请解密DATA后格式化DATA分区才可开机

## 安装管理器

设备重启后安装对应管理器即可获取Root权限，对应KernelSU分支对应的管理器见下表：
|KernelSU分支|对应管理器版本|下载链接|
|:---:|:---:|:---:|
|KernelSU|v0.9.2|[GithubRelease](https://github.com/tiann/KernelSU/releases/download/v0.9.2/KernelSU_v0.9.2_11682-release.apk)|
|RKSU|v3.0.0-30-legacy|[GithubReleases](https://github.com/rsuntk/KernelSU/releases/download/v3.0.0-30-legacy/KernelSU_v3.0.0-30-legacy_32334-release.apk)
|KernelSU-Next|最新版|[GithubReleases](https://github.com/KernelSU-Next/KernelSU-Next/releases)|
|SukiSU-Ultra|4.1.2|[GithubReleases](https://github.com/SukiSU-Ultra/SukiSU-Ultra/releases/download/v4.1.2/SukiSU_v4.1.2_40545-release.apk)|
|ReSukiSU|最新版|[GithubActions](https://github.com/ReSukiSU/ReSukiSU/actions/workflows/build-manager.yml)

> [!NOTE]
>
> 通过GithubAction下载需要登陆Github账号才可以显示下载按钮
>

# 支持设备
|机型名称|机型代码|支持系统|支持情况|更多链接|
|:---:|:---:|:---:|:---:|:---:|
|Mate9 系列|MHA|HarmonyOS2.0、EMUI9.0|KernelSU、RKSU、KernelSU-Next、SukiSU-Ultra、ReSukiSU|无|
|Mate10 Pro(测试中)|BLA|EMUI9.1|KernelSU|[android_kernel_huawei_blanc](https://github.com/Coconutat/android_kernel_huawei_blanc)|
|Mate10 系列(测试中)|ALP|EMUI9.0|KernelSU|无|
|P10 系列|VTR|EMUI9.0、HarmonyOS2.0|KernelSU|[android_kernel_huawei_hi3660_HM2](https://github.com/Coconutat/android_kernel_huawei_hi3660_HM2)|
|荣耀V9 系列|DUK|HarmonyOS2.0|KernelSU|无|
|荣耀9 系列|STF|EMUI9.0|KernelSU|无|
|Nova4 系列|VCE|EMUI9.0|KernelSU、SukiSU-Ultra|无|
|Nova 3i 系列|INE|EMUI9.1|RKSU|无|
|畅享9 Plus 系列|JKM|EMUI9.1、HarmonyOS2.0|KernelSU|无|
|畅享10 Plus 系列|STK|EMUI9.1|RKSU|无|
  
### 持续更新中...

## 未来计划

- [x] 添加官方网站
- [x] 添加GithubAction自动编译功能
- [x] 添加对SukiSU-Ultra的支持
- [ ] 添加ManualHooks自动集成的支持
- [x] 添加对RKSU的支持
- [x] 添加对ReSukiSU的支持
- [x] 添加对SUSFS的支持
- [x] 添加对KPM的支持(仅兼容基于EMUI9设备的内核)
- [ ] 添加对ReSukiSU的多管理器支持
  
# 贡献项目

## 方法1

按照[官网](https://xixiaobei-bei.github.io/KernelSU_on_Huawei/guide/how-to-build.html)的教程编译出自己的内核后打包发给我的邮箱

jiaxi120516@outlook.com

有Github账号的可以把名字一并发给我，我会把你加入项目贡献者

## 方法2

将自己的 设备型号，安卓版本，系统版本和内核版本(内核至少得是4.9版本) 一起发给我的邮箱

jiaxi120516@outlook.com

我在周末将会试图制作

# 修改内容

KernelSU有关：
- `fs/open.c` 中的 `faccessat` 方法
- `fs/exec.c` 中的 `do_execveat_common` 方法
- `fs/read_write.c` 中的 `vfs_read` 方法
- `stat.c` 中的 `vfs_fstatat` 方法

ManualHooks有关：
- `fs/open.c` 中的 `faccessat` 方法
- `fs/exec.c` 中的 `do_execve` 方法
- `fs/read_write.c` 中的 `SYSCALL_DEFINE3` 方法
- `stat.c` 中的 `vfs_fstatat` 方法
- `kernel/reboot.c` 中的 `SYSCALL_DEFINE4` 方法
- `kernel/sys.c` 中的 `setuid` 方法
- `security/selinux/hooks.c` 中的 `int check_nnp_nosuid` 方法

KernelSU安全模式有关：
- `drivers/input/input.c` 中的 `input_handle_event` 方法

KernelSU模块有关：

- `security/selinux/hooks.c` 参考 [此commit](https://github.com/sticpaper/android_kernel_xiaomi_msm8998-ksu/commit/09a4672c0f521bf6b05daf24b207b125830a6fc5)

- `fs/namespace.c` 中的path_umount方法

## 特别鸣谢(排名不分前后)

- [tiann](https://github.com/tiann) / [KernelSU](https://github.com/tiann/KernelSU/)——KernelSU作者

- [rsuntk](https://github.com/rsuntk) / [KernelSU](https://github.com/rsuntk/KernelSU)——添加KernelSU在5系内核以下的支持

- [KernelSU-Next](https://github.com/KernelSU-Next/) / [KernelSU-Next](https://github.com/KernelSU-Next/KernelSU-Next)——KernelSU-Next作者

- [SukiSU-Ultra](https://github.com/SukiSU-Ultra) / [SukiSU-Ultra](https://github.com/SukiSU-Ultra/SukiSU-Ultra)——SukiSU-Ultra作者

- [ReSukiSU](https://github.com/ReSukiSU) / [ReSukiSU](https://github.com/ReSukiSU/ReSukiSU)——添加SukiSU-Ultra在5系内核以下的支持

- [Coconutat](https://github.com/Coconutat)——参与部分项目维护工作

- [dabao1955](https://github.com/dabao1955) / [KernelSU_General_porting_guide](https://github.com/dabao1955/KernelSU_General_porting_guide)——编译KernelSU内核的基础教程

- [xfy2412](https://github.com/xfy2412)——提供华为Nova4的KernelSU和SukiSU-Ultra支持

- [xqlpx](https://github.com/xqlpx)——提供华为Nova3i的RKSU支持