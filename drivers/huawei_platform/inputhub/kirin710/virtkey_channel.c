/*
 *  drivers/misc/inputhub/virtkeyhub_channel.c
 *  FHB Channel driver
 *
 *  Copyright (C) 2012 Huawei, Inc.
 *  Author: @
 *  Date:   2016.3.10
 *
 */
#include <linux/input.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/of.h>
int tell_codec_key_route_status_to_mcu(int key_st, int lock_type)
{
    (void)(key_st);
    (void)(lock_type);
    return 0;
}
EXPORT_SYMBOL_GPL(tell_codec_key_route_status_to_mcu);
