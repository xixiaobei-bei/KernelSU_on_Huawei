/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: the selinux_avc_trace.h for selinux call trace print
 * Author: linxuanyu
 * Create: 2019-05-09
 */
#ifndef SELINUX_AVC_TRACE_H
#define SELINUX_AVC_TRACE_H
#ifdef CONFIG_SECURITY_SELINUX_TRACE_LOG

#define CONTEXT_LEN  100

extern char avc_scontext[CONTEXT_LEN];
extern char avc_tcontext[CONTEXT_LEN];
extern char avc_tclass[CONTEXT_LEN];

int avc_getcontext(void);
#endif
#endif