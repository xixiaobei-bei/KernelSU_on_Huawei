#!/bin/bash
#设置环境

# GCC交叉编译器路径
export PATH=$PATH:$(pwd)/../Compiler/aarch64-linux-android-4.9-pie/bin
export CROSS_COMPILE=aarch64-linux-android-
export ARCH=arm64
export SUBARCH=arm64

# GCC颜色输出
export CFLAGS="-fdiagnostics-color=always"
export CXXFLAGS="-fdiagnostics-color=always"

# export DTC_EXT=dtc

if [ ! -d "out" ]; then
    mkdir out
fi

start_time=$(date +%Y.%m.%d-%I_%M)

start_time_sum=$(date +%s)

make ARCH=arm64 O=out merge_hi3660_defconfig
# 定义编译线程数
make ARCH=arm64 O=out -j$(nproc --all) 2>&1 | tee kernel_log-${start_time}.log

end_time_sum=$(date +%s)

end_time=$(date +%Y.%m.%d-%I_%M)

# 计算运行时间（秒）
duration=$((end_time_sum - start_time_sum))

# 将秒数转化为 "小时:分钟:秒" 形式输出
hours=$((duration / 3600))
minutes=$(((duration % 3600) / 60))
seconds=$((duration % 60))

# 打印运行时间
echo "脚本运行时间为：${hours}小时 ${minutes}分钟 ${seconds}秒"

if [ -f out/arch/arm64/boot/Image.gz ]; then

    echo "***Sucessfully built kernel...***"
    
    # 创建输出目录
    mkdir -p Build_Kernel/Original
    mkdir -p Build_Kernel/KPM

    # =========================================================
    # PART 1: 处理原始未修补内核 (Original)
    # =========================================================
    echo "正在处理原始内核 (Original)..."
    
    # 1. 保存原始 Image.gz
    cp out/arch/arm64/boot/Image.gz Build_Kernel/Original/Image.gz 
    
    # 2. 打包原始 img
    ./tools/mkbootimg --kernel Build_Kernel/Original/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07C00000 --header_version 1 --os_version 9 --os_patch_level 2026-01-30 --output Build_Kernel/Original/Kirin960_9.0_Kernel.img
	./tools/mkbootimg --kernel Build_Kernel/Original/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07C00000 --header_version 1 --os_version 9 --os_patch_level 2026-01-30 --output Build_Kernel/Original/Kirin960_9.0_Kernel_PM.img
    
    echo "原始内核已保存至: Build_Kernel/Original/"

    # =========================================================
    # PART 2: KPM 自动检测与修补逻辑
    # =========================================================
    KPM_CONFIG="out/.config"
    PATCH_TOOL="./tools/kpm/patch_linux"

    # 检测是否开启了 CONFIG_KPM
    if grep -q "CONFIG_KPM=y" "$KPM_CONFIG"; then
        echo "==========================================="
        echo "检测到 CONFIG_KPM=y，准备进行内核修补..."
        
        if [ -f "$PATCH_TOOL" ]; then
            # 赋予工具执行权限
            chmod 777 "$PATCH_TOOL"

            echo "准备工作环境..."
            # 将原始 Image.gz 复制到当前目录准备解压，避免污染 Original 目录
            cp out/arch/arm64/boot/Image Build_Kernel/KPM/Image
            cd Build_Kernel/KPM/

            echo "正在运行 patch_linux 工具..."
            ../../$PATCH_TOOL

            # 检查是否生成了 oImage (KernelSU/APatch 常见行为)
            if [ -f "oImage" ]; then
                echo "检测到 oImage，替换原 Image..."
                mv -f oImage Image
            fi

            ls -l

            echo "正在重新压缩为 Image.gz 并保存到 KPM 目录..."
            # 压缩并输出到 KPM 目录
            gzip -k Image Image.gz
            ls -l
            cd ../..
            
            echo "正在打包修补后的内核 (KPM)..."
            # 使用修补后的内核进行打包
            ./tools/mkbootimg --kernel Build_Kernel/Original/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07C00000 --header_version 1 --os_version 9 --os_patch_level 2026-01-30 --output Build_Kernel/Original/Kirin960_9.0_Kernel.img
	        ./tools/mkbootimg --kernel Build_Kernel/Original/Image.gz --base 0x0 --cmdline "loglevel=4 initcall_debug=n page_tracker=on unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=permissive buildvariant=user" --tags_offset 0x07A00000 --kernel_offset 0x00080000 --ramdisk_offset 0x07C00000 --header_version 1 --os_version 9 --os_patch_level 2026-01-30 --output Build_Kernel/Original/Kirin960_9.0_Kernel_PM.img

            # 清理临时文件
            rm -f Image
            echo "KPM 修补及打包完成！文件位于: Build_Kernel/KPM/"
        else
            echo "警告: 未找到补丁工具 ($PATCH_TOOL)，跳过修补步骤！"
        fi
        echo "==========================================="
    else
        echo "未检测到 CONFIG_KPM 开启，跳过 KPM 修补流程。"
    fi

    exit 0
else
    echo " "
    echo "***Failed!***"
    exit 0
fi