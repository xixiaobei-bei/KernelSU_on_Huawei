# 如何使用？

:::danger

Root 您的设备可能会使保修失效，如果操作不当可能会造成永久性损坏。 请务必在继续之前创建完整备份，阅读文档确保与您的设备兼容，遵循文档参考，准备好恢复计划。由操作不当导致的任何后果不由本项目作者（xixiaobei）及任何人负责！

:::

首先，您应该阅读支持设备列表：

|机型名称|机型代码|支持情况|
|:---:|:---:|:---:|
|Mate9全系列|MHA-XXX|KernelSU、KernelSU-Next、SukiSU-Ultra|
|P9全系列(testing)|EVA-XXX|KernelSU|

## 下载内核映像文件

### 通过GithubReleases下载

可以在[此页面](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/releases)下载，选择最新Release找到对应文件点击下载即可

### 通过GithubAction下载开发版

:::warning

开发版没有被确定完全可用，可能包含未知的问题！

:::

:::tip

通过GithubAction下载需要登陆Github账号才可以显示下载按钮

:::

打开[本项目GithubAction页面](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/actions)，选择左侧Action列表的Build Kernel板块，选择最新编译任务，然后找到设备对应的文件下载。

映像文件的命名规则如下：

```机型代码_底包版本_KernelSU类型_Selinux状态```

例如：

```MHA_EMUI9.0_KernelSU-Next_PM.img```

代表支持MHA型号EMUI9.0底包设备的KernelSU-Next的Selinux状态为宽容的内核

## 刷入内核映像文件

在电脑上下载[Platform-tools](https://developer.android.google.cn/tools/releases/platform-tools?hl=zh-cn)

随后将手机重启进入Fastboot模式，并确保设备已解锁BL

在Platform-tools工具目录下终端输入：

```bash
fastboot flash kernel <内核映像文件路径>
```

最后输入

```bash
fastboot reboot
```

以重启

## 安装管理器

设备重启后安装对应管理器即可获取Root权限，对应KernelSU分支对应的管理器见下表：
|KernelSU分支|对应管理器版本|下载链接|
|:---:|:---:|:---:|
|KernelSU|v0.9.5|[GithubRelease](https://github.com/tiann/KernelSU/releases/download/v0.9.5/KernelSU_v0.9.5_11872-release.apk)|
|KernelSU-Next|最新版|[GithubReleases](https://github.com/KernelSU-Next/KernelSU-Next/releases)|
|SukiSU-Ultra|最新版|[GithubReleases](https://github.com/SukiSU-Ultra/SukiSU-Ultra/releases)|