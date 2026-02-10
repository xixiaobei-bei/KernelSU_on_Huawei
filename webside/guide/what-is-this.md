# 什么是KernelSU_on_Huawei项目？

## 项目介绍

本项目计划为华为设备集成KernelSU内核Root方案，你可以在Github找到该项目，项目所有文件都已开源。

## 为什么用KernelSU？

华为设备recovery与boot的ramdisk都位于recovery_ramdisk分区，若选择Magisk修补，则无法使用Recovery，即使是TWRP。而KernelSU只修改Kernel分区，所以选择KernelSU就可以实现同时使用TWRP+Root

## 如何使用？

[如何使用](/guide/how-to-use.md)

## 如何适配？

[如何适配](/guide/how-to-make.md)

## 如何构建？

[如何构建](/guide/how-to-build.md)

## 未来计划

- [x] 添加官方网站
- [x] 添加GithubAction自动编译功能
- [x] 添加对SukiSU-Ultra的支持
- [ ] 添加ManualHooks自动集成的支持
- [x] 添加对RKSU的支持
- [x] 添加对ReSukiSU的支持
- [ ] 添加对SUSFS的支持
- [ ] 添加对KPM的支持