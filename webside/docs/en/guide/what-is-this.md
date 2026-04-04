# What is the KernelSU_on_Huawei project?  

## Project Introduction  

This project aims to integrate the KernelSU kernel root solution into Huawei devices. You can find the project on GitHub; all project files are open source.  

## Why use KernelSU?  

Huawei devices have both the recovery and boot ramdisk located in the recovery_ramdisk partition. If you choose Magisk for patching, Recovery cannot be used, even with TWRP. KernelSU only modifies the kernel partition, so choosing KernelSU allows you to use TWRP + Root simultaneously.  

## How to use it?  

[How to use](/guide/how-to-use.md)  

## How to adapt?  

[How to adapt](/guide/how-to-adapt.md)  

## Build kernel without SuSFS

[Build kernel without SuSFS](/guide/how-to-build.md)  

## Build kernel with SuSFS and KPM

[Build kernel with SuSFS and KPM](/guide/how-to-build-susfs-and-kpm.md)  