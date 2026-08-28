# What is the KernelSU_on_Huawei project?  

## Project Introduction  

This project aims to integrate the KernelSU kernel root solution into Huawei devices. You can find the project on GitHub; all project files are open source.  

## Why use KernelSU?  

Huawei devices have both the recovery and boot ramdisk located in the recovery_ramdisk partition. If you choose Magisk for patching, Recovery cannot be used, even with TWRP. KernelSU only modifies the kernel partition, so choosing KernelSU allows you to use TWRP + Root simultaneously.  

## How to use it?  

[How to use](/en/guide/how-to-use.md)  

## How to adapt?  

[How to adapt](/en/guide/how-to-adapt.md)  

## Interate KernelSU without SuSFS

[Interate KernelSU without SuSFS](/en/guide/how-to-intergrate-kernelsu.md)  

## Interate KernelSU with SuSFS and KPM

[Interate kernel with SuSFS and KPM](/en/guide/how-to-intergrate-susfs-and-kpm.md)  

## Build

[Build 4.9 kernel](/en/guide/how-to-build-4.9.md)