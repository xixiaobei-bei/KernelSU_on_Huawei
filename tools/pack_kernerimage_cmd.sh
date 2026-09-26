#!/bin/bash
./mkbootimg --kernel kernel --base 0x0 --cmdline "loglevel=4 page_tracker=on printktimer=0xfa89b000,0x534,0x538 rcupdate.rcu_expedited=1 androidboot.selinux=enforcing buildvariant=user" --tags_offset 0xBCD00000 --kernel_offset 0x00080000 --ramdisk_offset 0xBCF00000 --header_version 2 --os_version 10 --os_patch_level 2020-11-01  --output kernel.img
