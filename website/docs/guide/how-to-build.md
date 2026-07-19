# 如何编译？

:::warning

网站施工中，不建议现在根据网站集成

:::

## 安装依赖

:::warning

由于Ubuntu/Debian高版本全局定义`yylloc`问题，所以只推荐使用Ubuntu18.04到Ubuntu22.04（Debian13以下，推荐这两个版本）

如果你的系统版本大于20.04,需要安装GCC9并设为默认：

```shell
sudo apt install gcc-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 100
```

:::
### 安装软件包

运行：

```shell
sudo apt-get install libncurses5-dev libncurses-dev libssl-dev device-tree-compiler bc cpio lib32ncurses5-dev lib32z1 build-essential binutils bc bison build-essential ccache curl flex g++-multilib gcc-multilib git gnupg gperf imagemagick lib32ncurses5-dev lib32readline-dev lib32z1-dev liblz4-tool libncurses5 libncurses5-dev libsdl1.2-dev libssl-dev libxml2 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev git python2
```

### 获取GCC

```shell
git clone --depth=1 https://kkgithub.com/LineageOS/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9 aarch64-linux-android-4.9
```

:::tip

华为只能用Aarch64版本

:::

## 编译

### 设定默认Python2

```shell
sudo ln -sf /usr/bin/python2.7 /usr/bin/python
```

### 设置环境变量

```shell
export ARCH=arm64  
export PATH=$PATH:GCC编译工具下bin文件夹的绝对路径  
export CROSS_COMPILE=aarch64-linux-android-
```

### 编译

在内核源码目录输入命令：

```shell
make ARCH=arm64 O=out defconfig配置文件名字(只要名字，不要路径)  
make ARCH=arm64 O=out -j8
```

这里的-j8指定编译的CPU核心数，要你的CPU核心数X2

### 打包

编译完后在out/arch/arm64/boot/路径下会有一个Image.gz文件，复制到内核根目录的tools文件夹

#### 更改pack_kernerimage_cmd.sh文件

更改`--kernel kernel`为`--kernel Image.gz`以修复文件名问题

如果你的SELinux为宽容模式，请将`androidboot.selinux=enforcing`改为`androidboot.selinux=permissive`

修改`--os_patch_level 2020-01-01`以更改内核编译时间

#### 打包

运行：

```shell
sh pack_kernerimage_cmd.sh
```

然后本目录会多出一个kernel.img文件，fastboot刷入kernel分区即可