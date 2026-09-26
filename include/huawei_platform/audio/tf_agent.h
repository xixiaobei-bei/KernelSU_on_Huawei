/*
 * tf_agent.h
 *
 * tfagent driver header file
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __TF_AGENT_H__
#define __TF_AGENT_H__

#define TF_IOC_MAGIC 'x'
#define CLOSE_READ              _IO(TF_IOC_MAGIC, 0x01)
#define SEC_KEY_NEG_START       _IO(TF_IOC_MAGIC, 0x02)
#define SEC_KEY_NEG_END_SUCCESS _IO(TF_IOC_MAGIC, 0x03)
#define SEC_KEY_NEG_END_FAIL    _IO(TF_IOC_MAGIC, 0x04)

#endif // __TF_AGENT_H__

