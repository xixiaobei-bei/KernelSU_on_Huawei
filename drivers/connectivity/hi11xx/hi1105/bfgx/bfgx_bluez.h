/*
 * 版权所有 (c) 华为技术有限公司 2016-2020
 * 功能说明   : 资源获取
 * 作者       : bluez 对外头文件
 * 创建日期   : 2020年12月17日
 */

#ifndef __BFGX_BLUEZ_H_
#define __BFGX_BLUEZ_H_

#include "hw_bfg_ps.h"

int32_t hw_bt_release(struct inode *inode, struct file *filp);
ssize_t hw_bt_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
uint32_t hw_bt_poll(struct file *filp, poll_table *wait);
ssize_t hw_bt_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
int32_t hw_bt_open(struct inode *inode, struct file *filp);

#endif
