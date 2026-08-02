# How to Use?

:::danger

Root your device may void your warranty and cause permanent damage if done improperly. Please create a full backup before proceeding, read the documentation to ensure compatibility with your device, follow the documentation's references, and prepare a recovery plan. The project author (xixiaobei) and anyone else are not responsible for any consequences resulting from improper operation!

:::

First, you should read the list of supported devices:

|Model Name|Model Code|Supported Systems|Support Status|Requirements|
|:---:|:---:|:---:|:---:|:---:|
|Mate9 Series|MHA|HarmonyOS 2.0, EMUI 9.0|KernelSU, RKSU, KernelSU-Next, SukiSU-Ultra, ReSukiSU|None|
|Mate10 Pro (Under Testing)|BLA|EMUI 9.1|KernelSU|None|
|Mate10 Series|ALP|EMUI 9.1|RKSU|None|
|Mate20 Pro Series|LYA|EMUI 9.0|RKSU|None|
|P10 Series|VTR|EMUI 9.0, HarmonyOS 2.0|KernelSU|None|
|P20 Series|EML|EMUI9.1|KernelSU|None|
|Honor V9 Series|DUK|HarmonyOS 2.0|KernelSU|None|
|Honor 9 Series|STF|EMUI 9.0|KernelSU|None|
|Nova 4 Series | VCE | EMUI 9.0 | KernelSU, SukiSU-Ultra |None|
|Nova 3i Series | INE | EMUI 9.1 | RKSU |None|
|Nova 5i Series|GLK|EMUI9.0|RKSU|None|
|Nova 5 Series|SEA-AL00、SEA-TL00|EMUI9.1|RKSU|None|
|Nova 5 Pro Series|SEA-AL10、SEA-TL10、SEA-TL10HKA|EMUI9.1|RKSU|None|
|Nova Y 9 Plus Series | JKM | EMUI 9.1, HarmonyOS 2.0 | KernelSU |None|
|Nova Y 10 Plus Series | STK | EMUI 9.1 | RKSU |None|
|Huawei PadM3 Lite Series|CPN|LineageOS17.1|KernelSU-Next|[Require System](https://www.coolapk.com/feed/62006426?s=MTU5NjM4YzUxYmZkYjY0ZzZhMTZkYjM2ega1621)|

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

`Device Code_Baseband Version_KernelSU Type_Selinux Status`

For example:

`MHA_EMUI9.0_KernelSU-Next_PM.img`

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
|RKSU|v3.0.0-30-legacy|[GithubReleases](https://github.com/rsuntk/KernelSU/releases/download/v3.0.0-30-legacy/KernelSU_v3.0.0-30-legacy_32334-release.apk)
|KernelSU-Next|Latest Version| [GithubReleases](https://github.com/KernelSU-Next/KernelSU-Next/releases)|
|SukiSU-Ultra|4.1.2|[GithubReleases](https://github.com/SukiSU-Ultra/SukiSU-Ultra/releases/download/v4.1.2/SukiSU_v4.1.2_40545-release.apk)|
|ReSukiSU|Latest Version|[GithubActions](https://github.com/ReSukiSU/ReSukiSU/actions/workflows/build-manager.yml)

:::tip

Downloading via GithubActions requires logging into a Github account to display the download button.

:::
