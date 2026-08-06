#!/bin/bash
./mkbootimg --kernel kernel --base 0x0 --cmdline "loglevel=4 page_tracker=on unmovable_isolate1=2:192M,3:224M,4:256M printktimer=0xfff0a000,0x534,0x538 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0x37E00000 --kernel_offset 0x00080000 --ramdisk_offset 0x37600000 --header_version 1 --os_version 10 --os_patch_level 2020-11-01  --output kernel.img
