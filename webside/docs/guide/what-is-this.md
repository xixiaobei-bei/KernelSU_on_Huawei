# 什么是KernelSU_on_Huawei项目？

## 项目介绍

本项目计划为华为设备集成KernelSU内核Root方案，你可以在Github找到该项目，项目所有文件都已开源。

## 为什么用KernelSU？

华为设备recovery与boot的ramdisk都位于recovery_ramdisk分区，若选择Magisk修补，则无法使用Recovery，即使是TWRP。而KernelSU只修改Kernel分区，所以选择KernelSU就可以实现同时使用TWRP+Root

## 如何使用？

[如何使用](/guide/how-to-use.md)

## 如何适配？

[如何适配](/guide/how-to-adapt.md)

## 如何构建？

[如何构建](/guide/how-to-build.md)

## 如何添加Susfs 

[如何添加Susfs](/guide/how-to-add-susfs-and-kpm.md)