### KernelSU on Huawei Mate9
###[简体中文](README.md) | [English](README_en.md)

适用于华为mate9的KernelSU内核！

选择对应版本刷入即可为设备集成KernelSU

# 命名规则

名字中带9.0的内核适用于EMUI5.x底包的设备

名字中带9.0的内核适用于EMUI9.0.x底包的设备

名字中带2.0的内核适用于HarmonyOS底包的设备

名字中带PM后缀的内核SELinux为宽容状态，可用于启动SGSI

# 注意事项

1.若选择PM内核刷入，则需要先解密DATA分区才可开机

2.请选择对应底包的内核再刷入，否则会无法开机！

# 刷入

1.确保设备已解锁Bootloader

2.下载对应内核

3.手机进入fastboot模式，连接电脑

4.电脑上在ADB工具目录下输入以下命令

```bash
fastboot flash kernel <内核文件>
```

若是PM内核，请解密DATA后格式化DATA分区才可开机

# 编译

1.下载对应内核源码

2.修改你要修改的内容

3.在终端输入以下命令安装依赖
```bash
sudo apt-get install libncurses5-dev libncurses-dev libssl-dev device-tree-compiler bc cpio lib32ncurses5-dev lib32z1 build-essential binutils bc bison build-essential ccache curl flex g++-multilib gcc-multilib git gnupg gperf imagemagick lib32ncurses5-dev lib32readline-dev lib32z1-dev liblz4-tool libncurses5 libncurses5-dev libsdl1.2-dev libssl-dev libxml2 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev git python2
```

4.运行以下命令使Pyrhon2.7作为默认值
```bash
sudo ln -sf /usr/bin/python2.7 /usr/bin/python
```

5.x86电脑运行以下命令
```bash
git clone --depth=1 https://github.com/LineageOS/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9 aarch64-linux-android-4.9
```
ARM电脑运行以下命令
```bash
git clone --depth=1 https://github.com/LineageOS/android_prebuilts_gcc_linux-x86_arm_arm-linux-androideabi-4.9 arm-linux-androideabi-4.9
```
以下载交叉编译器

6.在内核源码目录下打开终端，运行以下命令（注意将命令中的路径换成刚刚下载的交叉编译器文件夹下的bin文件夹的路径）
```bash
export ARCH=arm64
export PATH=$PATH:/media/coconutat/Files/Downloads/Github/android_kernel_huawei_ravel_KernelSU——android_kernel_huawei_ravel_KernelSU/aarch64-linux-android-4.9/bin
export CROSS_COMPILE=aarch64-linux-android-
``````

7.保持终端窗口开启，在其中输入以下命令以编译（注意将数字8改成你的CPU核心数X2）
```bash
make ARCH=arm64 O=out merge_hi3660_defconfig
make ARCH=arm64 O=out -j8
```

8.等待编译完成

9.编译完成后在out/arch/ram64/boot目录下会生成Image.gz文件，将其移动到内核目录下的tools文件夹

10.修改pack_kernerimage_cmd.sh文件中的日期为当前时间

11.在tools目录下运行以下命令打包内核
```bash
bash pack_kernerimage_cmd.sh
```

12.在tools目录下会生成kernel.img文件，刷入测试

# 特别鸣谢
[dabao1955](https://github.com/dabao1955) / [KernelSU_General_porting_guide](https://github.com/dabao1955/KernelSU_General_porting_guide)——提供修改内核方法

[tiann](https://github.com/tiann) / [KernelSU](https://github.com/tiann/KernelSU/)——KernelSU作者



