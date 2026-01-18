# KernelSU on Huawei

[[简体中文](README.md)]  [[English](README_en.md)]

- 适用于华为的KernelSU内核！

- 现支持KernelSU和KernelSU-Next（SukiSU在研究了）

- 感谢各位贡献作者，我只是站在了巨人的肩膀上

## 命名规则

- 名字中带EMUI-xxx的内核适用于EMUI xxx版本底包的设备

- 名字中带HarmonyOS-xxx的内核适用于HarmonyOS xxx版本底包的设备

- 名字中带PM后缀的内核SELinux为宽容状态，可用于启动SGSI

## 注意事项

- 若选择PM内核刷入，则需要先解密DATA分区才可开机

- 请选择对应底包的内核再刷入，否则会无法开机！

## 刷入

1.解锁Bootloader

2.下载对应内核

3.手机进入fastboot模式，连接电脑

4.电脑上在ADB工具目录下输入以下命令

```bash
fastboot flash kernel <内核文件>
```

若是PM内核，请解密DATA后格式化DATA分区才可开机

# 支持设备

- [x] Mate 9 系列（MHA）
 
### 持续更新中...

# 贡献项目

## 方法1

按照wiki的教程编译出自己的内核后提交新分支pr或打包发给我的邮箱

jiaxi120516@outlook.com

有Github账号的可以把名字一并发给我，我会把你加入项目贡献者

## 方法2

将自己的 设备型号和系统版本（内核至少得是4.9版本） 一起发给我的邮箱

jiaxi120516@outlook.com

我在周末将会试图制作

## 若你知道如何将SukiSU移植到4.9内核，请指导我，非常感谢！

# 修改内容

KernelSU本体有关：
- `fs/open.c` 中的 `faccessat` 方法
- `fs/exec.c` 中的 `do_execveat_common` 方法
- `fs/read_write.c` 中的 `vfs_read` 方法
- `stat.c` 中的 `vfs_fstatat` 方法

KernelSU安全模式有关：
- `drivers/input/input.c` 中的 `input_handle_event` 方法

KernelSU模块功能：
- `security/selinux/hooks.c` 参考 [此commit](https://github.com/sticpaper/android_kernel_xiaomi_msm8998-ksu/commit/09a4672c0f521bf6b05daf24b207b125830a6fc5)

EMUI强制内核宽容导致KernelSU不工作解决：
- `security/selinux/selinuxfs.c` 参考 [此commit](https://github.com/Coconutat/android_kernel_huawei_kirin970_EMUI9.1.0_KernelSU/commit/f67307c967280d9b863058e47bae7611c8bc3db9)第166行

## 特别鸣谢
- [dabao1955](https://github.com/dabao1955) / [KernelSU_General_porting_guide](https://github.com/dabao1955/KernelSU_General_porting_guide)——提供修改内核集成KernelSU的方法

- [tiann](https://github.com/tiann) / [KernelSU](https://github.com/tiann/KernelSU/)——KernelSU作者

- [KernelSU-Next](https://github.com/KernelSU-Next/) / [KernelSU-Next](https://github.com/KernelSU-Next/KernelSU-Next)——KernelSU-Next作者（好像是的）