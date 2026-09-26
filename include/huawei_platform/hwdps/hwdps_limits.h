/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Define about limits.
 * Create: 2020-06-16
 */

#ifndef _HWDPS_LIMITS_H
#define _HWDPS_LIMITS_H

#ifdef __KERNEL__
#include <linux/limits.h>
#else
#include <sys/limits.h>
#endif

#ifndef HWDPS_USER_KEY_DESC_MAX
#define HWDPS_USER_KEY_DESC_MAX 8
#endif

#ifndef HWDPS_POLICY_RULESET_MIN
#define HWDPS_POLICY_RULESET_MIN 10
#endif

#ifndef HWDPS_POLICY_RULESET_MAX
#define HWDPS_POLICY_RULESET_MAX 256
#endif

#ifndef HWDPS_POLICY_PACKAGE_MAX
#define HWDPS_POLICY_PACKAGE_MAX 8192 /* to avoid app_policy_len reverse */
#endif

#ifndef HWDPS_ENCODE_WFEK_SIZE_OLD
#define HWDPS_ENCODE_WFEK_SIZE_OLD 251
#endif

#ifndef HWDPS_ENCODED_WFEK_SIZE
#define HWDPS_ENCODED_WFEK_SIZE (81 + 16)
#endif

#ifndef HWDPS_FEK_SIZE_MAX
#define HWDPS_FEK_SIZE_MAX 64
#define HWDPS_FEK_SIZE 64
#endif

#endif
