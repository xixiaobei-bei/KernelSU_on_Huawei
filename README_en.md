# KernelSU for Huawei

- KernelSU kernels for Huawei devices!

- Currently supports KernelSU,RKSU,KernelSU-Next,SukiSU-Ultra and ReSukiSU.

- Thanks to all contributors — I'm standing on the shoulders of giants.

> [!CAUTION]
>
> Rooting your device may void its warranty and could cause permanent damage if not done correctly. Please ensure you create a full backup before proceeding, read the documentation to confirm compatibility with your device, follow all reference guides, and have a recovery plan ready. The author of this project (xixiaobei) and any other individuals assume no responsibility for any consequences resulting from improper operation!
>

## Download

### Download via GitHub Releases

You can download it from [this page](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/releases).Select the latest Release, find the corresponding file, and click to download.

### Download Development Versions via GitHub Actions

> [!WARNING]
>
> The development version has not been confirmed to be fully usable and may contain unknown issues!
>

> [!NOTE]
>
> Downloading via GitHub Actions requires logging into your GitHub account for the download button to appear.
>

Open the [GitHub Actions page for this project](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/actions),select the "Build Kernel" section from the left Action list, choose the latest compilation task, and then download the file corresponding to your device.

## Naming Convention

`device_code_base_firmware_KernelSU_type_SELinux_state`

Example:

`MHA_EMUI9.0_KernelSU-Next_PM.img`

This indicates a KernelSU-Next kernel for the MHA model running EMUI 9.0, with SELinux in permissive mode (PM)

## Important Notes

- If you flash a PM kernel, you must decrypt the DATA partition before the device can boot.

- Always flash a kernel that matches the device's base firmware; flashing a mismatched kernel may prevent the device from booting!

- Official KernelSU dropped non-GKI kernels in v1.0. If your device requires a non-GKI kernel, use the [v0.9.5 manager](https://github.com/tiann/KernelSU/releases/v0.9.5/). Other KernelSU branches are unaffected.

## Flashing (Installation)

1. Unlock the bootloader.
2. Download the corresponding kernel for your device and firmware.
3. Boot the phone into fastboot mode and connect it to your computer.
4. In the fastboot/ADB tools directory on your computer, run:

```bash
fastboot flash kernel <kernel-file>
```

If you are flashing a PM kernel, decrypt and (if necessary) reformat the DATA partition before booting.

## Install the manager.

After the device reboots, install the corresponding manager to obtain Root access. The manager corresponding to each KernelSU branch is listed in the table below:
|KernelSU branches name|Manager version|Download links|
|:---:|:---:|:---:|
|KernelSU|v0.9.2|[GithubRelease](https://github.com/tiann/KernelSU/releases/download/v0.9.2/KernelSU_v0.9.2_11682-release.apk)|
|RKSU|Lastest|[GithubReleases](https://github.com/rsuntk/KernelSU/releases)
|KernelSU-Next|Lastest|[GithubReleases](https://github.com/KernelSU-Next/KernelSU-Next/releases)|
|SukiSU-Ultra|Lastest|[GithubReleases](https://github.com/SukiSU-Ultra/SukiSU-Ultra/releases)|
|ReSukiSU|Lastest|[GithubActions](https://github.com/ReSukiSU/ReSukiSU/actions/workflows/build-manager.yml)

> [!NOTE]
>
> Downloading via GitHub Actions requires logging into your GitHub account for the download button to appear.
>

# Supported Devices

- [x] Mate 9 series(MHA)
- [ ] Mate 10 Pro series(BLA)(testing)
- [ ] Mate 10 series(ALP)(testing)
- [x] P 10 series(VTR)
- [x] HONOR V9 series(DUK)
- [x] HONOR 9 series(STF)
- [x] Nova 4 series(VCE)
- [x] Nova Y 9 series(JKM)

### More devices coming...

## Roadmap / Future Plans

- [x] Add an official website
- [x] Add GitHub Actions for automatic builds
- [x] Add support for SukiSU-Ultra
- [ ] Add automatic ManualHooks integration
- [x] Add support for RKSU
- [x] Add support for ReSukiSU
- [ ] Add support for SUSFS
- [ ] Add support for KPM

# Contributing

If you'd like to contribute, there are two main ways:

## Method 1

Follow the tutorial on the [wiki](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/wiki/%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87) to build your kernel, package it, and send it to my email:

jiaxi120516@outlook.com

If you have a GitHub account, include your username and I'll add you as a project contributor.

## Method 2

Send your device model, Android version, system version, and kernel version (kernel must be at least 4.9) to:

jiaxi120516@outlook.com

I'll try to build and test it on the weekend.

# Changes (What was modified)

KernelSU-related:
- `fs/open.c` — `faccessat`
- `fs/exec.c` — `do_execveat_common`
- `fs/read_write.c` — `vfs_read`
- `stat.c` — `vfs_fstatat`

ManualHooks related:
- `fs/open.c` — `faccessat`
- `fs/exec.c` — `do_execve`
- `fs/read_write.c` — `SYSCALL_DEFINE3`
- `stat.c` — `vfs_fstatat`
- `kernel/reboot.c` — `SYSCALL_DEFINE4`
- `kernel/sys.c` — `setuid`
- `security/selinux/hooks.c` — `int check_nnp_nosuid`

KernelSU Safe Mode-related:
- `drivers/input/input.c` — `input_handle_event`

KernelSU modules:

- `security/selinux/hooks.c` — see [this commit](https://github.com/sticpaper/android_kernel_xiaomi_msm8998-ksu/commit/09a4672c0f521bf6b05daf24b207b125830a6fc5)

- `fs/namespace.c` — `path_umount` method

## Special Thanks (in no particular order)

- [tiann](https://github.com/tiann) / [KernelSU](https://github.com/tiann/KernelSU/) — KernelSU author

- [rsuntk](https://github.com/rsuntk) / [KernelSU](https://github.com/rsuntk/KernelSU)——added support for KernelSU on kernels below the 5.x version

- [KernelSU-Next](https://github.com/KernelSU-Next/) / [KernelSU-Next](https://github.com/KernelSU-Next/KernelSU-Next) — KernelSU-Next authors

- [SukiSU-Ultra](https://github.com/SukiSU-Ultra/SukiSU-Ultra) — SukiSU-Ultra author

- [ReSukiSU](https://github.com/ReSukiSU/ReSukiSU) — added support for SukiSU-Ultra on kernels below the 5.x version

- [Coconutat](https://github.com/Coconutat) and their [PR #1](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/pull/1/) — provided ideas for integrating SukiSU-Ultra ManualHooks

- [dabao1955](https://github.com/dabao1955) / [KernelSU_General_porting_guide](https://github.com/dabao1955/KernelSU_General_porting_guide) — basic guide for building KernelSU kernels

- [xfy2412](https://github.com/xfy2412)——Added KernelSU support for Huawei Nova4