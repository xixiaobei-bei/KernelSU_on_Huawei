# 如何使用？

首先，您应该阅读支持设备列表：

|机型名称|机型代码|支持情况|
|:---:|:---:|:---:|
|Mate9全系列|MHA-XXX|KernelSU、KernelSU-Next、SukiSU-Ultra|
|P9全系列(testing)|EVA-XXX|KernelSU|

## 下载内核映像文件

访问项目[GithubRelease](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/releases)，随后下载对应内核映像文件，映像文件的命名规则如下：

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