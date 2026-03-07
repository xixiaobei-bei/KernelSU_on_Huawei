# How to Use?

:::danger

Root your device may void your warranty and cause permanent damage if done improperly. Please create a full backup before proceeding, read the documentation to ensure compatibility with your device, follow the documentation's references, and prepare a recovery plan. The project author (xixiaobei) and anyone else are not responsible for any consequences resulting from improper operation!

:::

First, you should read the list of supported devices:

|Model Name|Model Code|Supported Systems|Support Status|
|:---:|:---:|:---:|:---:|
|Mate9 Series|MHA|HarmonyOS 2.0, EMUI 9.0|KernelSU, RKSU, KernelSU-Next, SukiSU-Ultra, ReSukiSU|
|Mate10 Pro (Under Testing)|BLA|EMUI 9.1|KernelSU|
|Mate10 Series (Under Testing)|ALP|EMUI 9.0|KernelSU|
|P10 Series|VTR|EMUI 9.0, HarmonyOS 2.0|KernelSU|
|Honor V9 Series|DUK|HarmonyOS 2.0|KernelSU|
|Honor 9 Series|STF|EMUI 9.0|KernelSU|
|Nova4 Series | VCE | EMUI 9.0 | KernelSU, SukiSU-Ultra |
| Enjoy 9 Plus Series | JKM | EMUI 9.1, HarmonyOS 2.0 | KernelSU |

## Download Kernel Image File

### Download via Github Releases

You can download it from [this page](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/releases). Select the latest Release, find the corresponding file, and click download.

### Download Development Version via Github Actions

:::warning

The development version is not confirmed to be fully usable and may contain unknown issues!

:::

:::tip

Downloading via Github Actions requires logging into your Github account to display the download button.

:::

Open the [Github Actions page for this project](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/actions), select the Build Kernel section in the Actions list on the left, select the latest build task, and then locate and download the file corresponding to your device.

The naming convention for image files is as follows:

``Device Code_Baseband Version_KernelSU Type_Selinux Status```

For example:

```MHA_EMUI9.0_KernelSU-Next_PM.img```

This indicates that the KernelSU-Next kernel with a forgiving Selinux status is supported for MHA-model EMUI9.0 baseband devices.

## Flashing the Kernel Image File

Download [Platform-tools](https://developer.android.google.cn/tools/releases/platform-tools?hl=zh-cn) on your computer.

Then reboot your phone into Fastboot mode and ensure the bootloader is unlocked.

In the terminal within the Platform-tools directory, type:

```bash
fastboot flash kernel <kernel image file path>

```
Finally, type:

```bash
fastboot reboot
```

To reboot.

## Install Manager

After restarting the device, install the corresponding manager to obtain root privileges. The managers corresponding to the KernelSU branch are shown in the table below:

|KernelSU Branch|Corresponding Manager Version|Download Link|
|:---:|:---:|:---:|
|KernelSU|v0.9.2|[GithubRelease](https://github.com/tiann/KernelSU/releases/download/v0.9.2/KernelSU_v0.9.2_11682-release.apk)|
|RKSU|Latest Version|[GithubReleases](https://github.com/rsuntk/KernelSU/releases)
|KernelSU-Next|Latest Version| [GithubReleases](https://github.com/KernelSU-Next/KernelSU-Next/releases)|
|SukiSU-Ultra|Latest Version|[GithubReleases](https://github.com/SukiSU-Ultra/SukiSU-Ultra/releases)|
|ReSukiSU|Latest Version|[GithubActions](https://github.com/ReSukiSU/ReSukiSU/actions/workflows/build-manager.yml)

:::tip

Downloading via GithubActions requires logging into a Github account to display the download button.

:::