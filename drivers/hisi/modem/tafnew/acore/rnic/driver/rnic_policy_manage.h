/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __RNIC_POLICY_MANAGE_H__
#define __RNIC_POLICY_MANAGE_H__

#include <linux/types.h>
#include <linux/version.h>
#include "rnic_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*lint -esym(752,rnic_cpumasks_deinit)*/
void rnic_cpumasks_deinit(struct rnic_lb_policy_s *policy);
/*lint -esym(752,rnic_cpumasks_init)*/
int rnic_cpumasks_init(struct rnic_lb_policy_s *policy);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0))
/*lint -esym(752,rnic_cpu_hotplug_init)*/
void rnic_cpu_hotplug_init(void);
/*lint -esym(752,rnic_cpu_hotplug_deinit)*/
void rnic_cpu_hotplug_deinit(void);
#else
/*lint -esym(752,rnic_cpuhp_init)*/
void rnic_cpuhp_init(void);
/*lint -esym(752,rnic_cpuhp_deinit)*/
void rnic_cpuhp_deinit(void);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0) */
void rnic_pm_init(void);

/*lint -esym(752,rnic_set_lb_config)*/
int rnic_set_lb_config(const struct rnic_lb_config_s *lb_config);
/*lint -esym(752,rnic_set_lb_level)*/
int rnic_set_lb_level(uint8_t devid, uint8_t level);
/*lint -esym(752,rnic_select_cpu_candidacy)*/
void rnic_select_cpu_candidacy(struct rnic_lb_policy_s *policy);

void rinc_queue_cong_ctrl(struct rnic_dev_priv_s *priv);
void rnic_qos_rst_work_fn(struct work_struct *work);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* __RNIC_POLICY_MANAGE_H__ */
