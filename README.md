# KernelSU on Huawei

[[简体中文](README.md)]  [[English](README_en.md)]

- 适用于华为的KernelSU内核！

- 现支持KernelSU和KernelSU-Next（SukiSU在研究了）

- 感谢各位贡献作者，我只是站在了巨人的肩膀上

## 命名规则

```机型代码_底包版本_KernelSU类型_Selinux状态```

例如：

```MHA_EMUI9.0_KernelSU-Next_PM.img```

代表支持MHA型号EMUI9.0底包设备的KernelSU-Next的Selinux状态为宽容的内核

## 注意事项

- 若选择PM内核刷入，则需要先解密DATA分区才可开机

- 请选择对应底包的内核再刷入，否则会无法开机！

- KernelSU官方在1.0版本已经舍弃非GKI内核，所以需要使用 [v0.9.5的管理器](https://github.com/tiann/KernelSU/releases/v0.9.5/) ，其他KernelSU分支不受影响

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

## 未来计划

- [x] 添加官方网站
- [x] 添加GithubAction自动编译功能
- [x] 添加对SukiSU-Ultra的支持
- [ ] 添加ManualHooks自动集成的支持
- [ ] 添加对RKSU的支持
- [ ] 添加对ReSukiSU的支持
- [ ] 添加对SusFS的支持
- [ ] 添加对KPM的支持

# 贡献项目

## 方法1

按照[wiki](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/wiki/%E7%AE%80%E4%BD%93%E4%B8%AD%E6%96%87)的教程编译出自己的内核后打包发给我的邮箱

jiaxi120516@outlook.com

有Github账号的可以把名字一并发给我，我会把你加入项目贡献者

## 方法2

将自己的 设备型号，安卓版本，系统版本和内核版本（内核至少得是4.9版本） 一起发给我的邮箱

jiaxi120516@outlook.com

我在周末将会试图制作

# 修改内容

KernelSU有关：
- `fs/open.c` 中的 `faccessat` 方法
- `fs/exec.c` 中的 `do_execveat_common` 方法
- `fs/read_write.c` 中的 `vfs_read` 方法
- `stat.c` 中的 `vfs_fstatat` 方法

SukiSU-Ultra的ManualHooks有关：
- `fs/open.c` 中的 `faccessat` 方法
- `fs/exec.c` 中的 `do_execve` 方法
- `fs/read_write.c` 中的 `SYSCALL_DEFINE3` 方法
- `stat.c` 中的 `vfs_fstatat` 方法
- `kernel/reboot.c` 中的 `SYSCALL_DEFINE4` 方法
- `kernel/sys.c` 中的 `setuid` 方法
- `security/selinux/hooks.c` 中的 `int check_nnp_nosuid` 方法

KernelSU安全模式有关：
- `drivers/input/input.c` 中的 `input_handle_event` 方法

KernelSU模块有关：

- `security/selinux/hooks.c` 参考 [此commit](https://github.com/sticpaper/android_kernel_xiaomi_msm8998-ksu/commit/09a4672c0f521bf6b05daf24b207b125830a6fc5)

- `fs/namespace.c` 中的path_umount方法

## 特别鸣谢（排名不分前后）

- [tiann](https://github.com/tiann) / [KernelSU](https://github.com/tiann/KernelSU/)——KernelSU作者

- [KernelSU-Next](https://github.com/KernelSU-Next/) / [KernelSU-Next](https://github.com/KernelSU-Next/KernelSU-Next)——KernelSU-Next作者（好像是的）

- [SukiSU-Ultra](https://github.com/SukiSU-Ultra/SukiSU-Ultra)——SukiSU-Ultra作者

- [ReSukiSU](https://github.com/ReSukiSU/ReSukiSU)——添加SukiSU-Ultra在5系内核以下的ManualHook支持

- [Coconutat](https://github.com/Coconutat) 以及他提交的[pr#1](https://github.com/xixiaobei-bei/KernelSU_on_Huawei/pull/1/)——提供SukiSU-Ultra的ManualHooks集成思路

- [dabao1955](https://github.com/dabao1955) / [KernelSU_General_porting_guide](https://github.com/dabao1955/KernelSU_General_porting_guide)——编译KernelSU内核的基础教程