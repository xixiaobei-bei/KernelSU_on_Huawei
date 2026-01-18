# KernelSU for Huawei

[[简体中文](README.md)]  [[English](README_en.md)]

- KernelSU kernel specifically adapted for Huawei devices!

- Currently supports KernelSU and KernelSU-Next (SukiSU is under research)

- Thanks to all the contributors; I'm merely standing on the shoulders of giants

## Naming Convention

- Kernels with `EMUI-xxx` in their name are suitable for devices with EMUI xxx base firmware.

- Kernels with `HarmonyOS-xxx` in their name are suitable for devices with HarmonyOS xxx base firmware.

- Kernels with the `PM` suffix have SELinux set to permissive mode and can be used to boot SGSI.

## Important Notes

- If you choose to flash a PM kernel, you **must decrypt the DATA partition** before the device can boot.

- **Always select the kernel that matches your device's base firmware version.** Flashing an incompatible kernel will result in a failure to boot!

## Flashing Instructions

1. Unlock the Bootloader.
2. Download the corresponding kernel file.
3. Boot the phone into fastboot mode and connect it to your computer.
4. On your computer, navigate to your ADB tools directory and execute the following command:

```bash
fastboot flash kernel <kernel_filename>
```

If using a PM kernel, you must decrypt and then format the DATA partition before the device can boot.

# Supported Devices

- [ ] Mate 9 Series (MHA)

## Continuously updating...

# Contributing to the Project

## Method 1

Follow the tutorial in the wiki to compile your own kernel, then either submit a pull request with a new branch or package it and email it to me at:

jiaxi120516@outlook.com

If you have a GitHub account, please send your username as well so I can add you to the project's list of contributors.

## Method 2

Send your device model and system version (the kernel must be at least version 4.9) to my email:

jiaxi120516@outlook.com

I will attempt to work on it over the weekend.

If you know how to port SukiSU to kernel version 4.9, please guide me. Your help would be greatly appreciated!

# Modifications

Related to KernelSU core:
- `fs/open.c`: Modified the `faccessat` function.
- `fs/exec.c`: Modified the `do_execveat_common` function.
- `fs/read_write.c`: Modified the `vfs_read` function.
- `fs/stat.c`: Modified the `vfs_fstatat` function.

Related to KernelSU Safe Mode:
- `drivers/input/input.c`: Modified the `input_handle_event` function.

KernelSU Module Support:
- `security/selinux/hooks.c`: Refer to [this commit](https://github.com/sticpaper/android_kernel_xiaomi_msm8998-ksu/commit/09a4672c0f521bf6b05daf24b207b125830a6fc5).

Fix for EMUI forcing kernel permissive mode causing KernelSU to fail:
- `security/selinux/selinuxfs.c`: Refer to line 166 of [this commit](https://github.com/Coconutat/android_kernel_huawei_kirin970_EMUI9.1.0_KernelSU/commit/f67307c967280d9b863058e47bae7611c8bc3db9).

# Special Thanks
- [dabao1955](https://github.com/dabao1955) / [KernelSU_General_porting_guide](https://github.com/dabao1955/KernelSU_General_porting_guide)——For providing the general porting guide for integrating KernelSU into kernels.

- [tiann](https://github.com/tiann) / [KernelSU](https://github.com/tiann/KernelSU/)——The creator of KernelSU.

- [KernelSU-Next](https://github.com/KernelSU-Next/) / [KernelSU-Next](https://github.com/KernelSU-Next/KernelSU-Next)——The team behind KernelSU-Next (presumably).