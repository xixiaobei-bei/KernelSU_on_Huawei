/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
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

/**
 *  @brief   msg模块在acore上的对外头文件
 *  @file    mdrv_msg.h
 *  @author  wangzaiwei
 *  @version v1.0
 *  @date    2019.11.15
 *  @note
 *  -v1.0|2019.11.15|创建文件
 *  @since   DRV2.0
 */

#ifndef __MDRV_MSG_H__
#define __MDRV_MSG_H__
#ifdef __cplusplus
extern "C" {
#endif

/** @brief 使用mdrv_msg_alloc申请内存，无特殊要求 */
#define MSG_DEFAULT (0x0)
/** @brief 使用mdrv_msg_alloc申请内存，优先使用低速内存 */
#define MSG_LF (0x04u)
/** @brief 使用mdrv_msg_alloc申请内存，优先使用中速内存 */
#define MSG_MF (0x08u)
/** @brief 使用mdrv_msg_alloc申请内存，优先使用高速内存 */
#define MSG_HF (0x10u)
/** @brief 使用mdrv_msg_alloc申请内存，长度参数已经包含了头长度 */
#define MSG_FULL_LEN (0x20u)

/** @brief 进行消息发送时，使用此标记，该消息会插队 */
#define MSG_UG (0x100u)

/** @brief 消息头定义(宏格式) */
/*lint -emacro({760,547}, MDRV_MSG_HEAD)*/
#define MDRV_MSG_HEAD                                                                                                  \
    unsigned short flag;                                                                                               \
    unsigned short timestamp;                                                                                          \
    unsigned int data_len;                                                                                             \
    unsigned int src_id;                                                                                               \
    unsigned int dst_id;

/** @brief 消息头定义(结构体格式) */
struct msg_head {
    MDRV_MSG_HEAD
};

/** @brief 消息内存状态(协议栈组件使用)，请使用枚举名进行判断，不要依赖枚举值 */
enum msg_memstatus {
    MSG_MEM_UNRESERVED = 0, /**< 消息内存未被RESERVE，调用free时会释放 */
    MSG_MEM_RESERVED,       /**< 消息内存被RESERVE，调用free时不会释放 */
};

/** @brief MSG消息头长度 */
#define MDRV_MSG_HEAD_LEN (sizeof(struct msg_head))

/**
 * @brief 用于填写消息头中的目的id，源id和长度
 */
#define MDRV_MSG_HEAD_FILL(a, sid, did, len)                                                                           \
    do {                                                                                                               \
        ((struct msg_head *)(a))->data_len = (unsigned int)len;                                                        \
        ((struct msg_head *)(a))->src_id = (unsigned int)sid;                                                          \
        ((struct msg_head *)(a))->dst_id = (unsigned int)did;                                                          \
    } while (0)

/**
 * @brief 用于填写消息长度
 */
#define MDRV_MSG_HEAD_SET_LEN(a, len)                                                                                  \
    do {                                                                                                               \
        ((struct msg_head *)(a))->data_len = (unsigned int)len;                                                        \
    } while (0)

/**
 * @brief 用于获取消息长度
 */
#define MDRV_MSG_HEAD_GET_LEN(a) ((struct msg_head *)(a))->data_len

/**
 * @brief 用于填写源id
 */
#define MDRV_MSG_HEAD_SET_SRCID(a, sid)                                                                                \
    do {                                                                                                               \
        ((struct msg_head *)(a))->src_id = (unsigned int)sid;                                                          \
    } while (0)

/**
 * @brief 用于获取消息源id
 */
#define MDRV_MSG_HEAD_GET_SRCID(a) ((struct msg_head *)(a))->src_id

/**
 * @brief 用于填写目的id
 */
#define MDRV_MSG_HEAD_SET_DSTID(a, did)                                                                                \
    do {                                                                                                               \
        ((struct msg_head *)(a))->dst_id = (unsigned int)did;                                                          \
    } while (0)

/**
 * @brief 用于获取消息目的id
 */
#define MDRV_MSG_HEAD_GET_DSTID(a) ((struct msg_head *)(a))->dst_id

/**
 * @brief 用于填写时间戳
 */
#define MDRV_MSG_HEAD_SET_TS(a, ts)                                                                                    \
    do {                                                                                                               \
        ((struct msg_head *)(a))->timestamp = (unsigned short)ts;                                                      \
    } while (0)
/**
 * @brief 用于获取时间戳
 */
#define MDRV_MSG_HEAD_GET_TS(a) ((struct msg_head *)(a))->timestamp

/**
 * @brief 申请消息内存的接口
 *
 * @par 描述:
 * 申请消息内存,使用消息接口进行内存发送时，必须使用该接口申请得到的内存
 *
 * @attention
 * <ul><li>申请高速内存时，若无内存可用，会依次尝试往低速方向分配内存</li></ul>
 *
 * @param[in]  mid, 申请消息内存的模块id
 * @param[in]  len, 申请消息内存的长度
 * @param[in]  flag, 申请消息内存时的特殊要求，参考MSG_LF等定义，无特殊要求填MSG_DEFAULT
 *
 * @retval NULL,表示消息申请失败。
 * @retval !=NULL,表示消息申请成功。

 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_free
 */
MODULE_EXPORTED void *mdrv_msg_alloc(unsigned mid, unsigned len, unsigned flag);

/**
 * @brief 释放消息内存的接口
 *
 * @par 描述:
 * 释放mdrv_msg_alloc申请的消息内存
 *
 * @param[in]  mid, 释放消息内存的模块id
 * @param[in]  addr, 释放消息内存的地址, 只能释放消息内存，不能释放普通内存
 *
 * @retval void
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_alloc
 */
MODULE_EXPORTED void mdrv_msg_free(unsigned mid, void *addr);

/**
 * @brief 标记消息内存的接口
 *
 * @par 描述:
 * 将消息内存标记为reserved
 *
 * @attention
 * <ul><li>多次mdrv_msg_reserve和mdrv_msg_unreserve以最后一次操作为准，无计数</li></ul>
 * <ul><li>当内存标记为reserved时，mdrv_msg_free不会释放内存</li></ul>
 *
 * @param[in]  mid, 标记消息内存的模块id
 * @param[in]  addr, 标记消息内存的地址，由mdrv_msg_alloc得来
 *
 * @retval 0, 标记成功
 * @retval 非0，标记失败
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_alloc mdrv_msg_unreserve MSG_MEM_RESERVED
 */
MODULE_EXPORTED int mdrv_msg_reserve(unsigned mid, void *addr);

/**
 * @brief 标记消息内存的接口
 *
 * @par 描述:
 * 将消息内存标记为unreserved
 *
 * @attention
 * <ul><li>多次mdrv_msg_reserve和mdrv_msg_unreserve以最后一次操作为准，无计数</li></ul>
 * <ul><li>当内存标记为unreserved时，mdrv_msg_free会释放内存</li></ul>
 *
 * @param[in]  mid, 标记消息内存的模块id
 * @param[in]  addr, 标记消息内存的地址，由mdrv_msg_alloc得来
 *
 * @retval 0, 标记成功
 * @retval 非0，标记失败
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_alloc mdrv_msg_reserve MSG_MEM_UNRESERVED
 */
MODULE_EXPORTED int mdrv_msg_unreserve(unsigned mid, void *addr);

/**
 * @brief 查询内存标记状态的接口
 *
 * @par 描述:
 * 查询内存标记状态
 *
 * @attention
 * <ul><li>当内存标记为unreserved时，mdrv_msg_free会释放内存</li></ul>
 *
 * @param[in]  mid, 标记消息内存的模块id
 * @param[in]  addr, 标记消息内存的地址，由mdrv_msg_alloc得来
 * @param[out]  status,返回内存标记状态
 *
 * @retval 0, 查询标记成功
 * @retval 非0，查询标记失败
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_reserve mdrv_msg_unreserve
 */
MODULE_EXPORTED int mdrv_msg_reserve_status(unsigned mid, void *addr, unsigned *status);

/**
 * @brief 宏，用于mdrv_msg_recv函数，表示从当前任务任意队列接收消息
 * @attention
 * <ul><li>只能用在mdrv_msg_recv第一个参数</li></ul>
 * @see mdrv_msg_recv
 */
#define MDRV_MSG_QID_ANY 0xFFFFFFFFU
/**
 * @brief 宏，用于mdrv_msg_recv函数，表示阻塞式消息接收，除非错误或者消息到达，否则永不超时
 * @attention
 * <ul><li>不要使用该宏在任何mdrv_msg_recv第三个参数以外的地方</li></ul>
 * @see mdrv_msg_recv
 */
#define MDRV_MSG_WAIT_FOREVER 0xFFFFFFFFU

/** @brief 错误码(使用要使用名字，不要依赖值) 消息接收接口返回，但未收到消息(超时或者使用非阻塞式调用) */
#define MDRV_MSG_ERR_QUEUE_EMPTY 0x1

/** @brief 注册mid的特性标志 */
#define MSG_MIDREG_FLAGS_FORCEBIND 0x1

/** @brief 注册mid使用的参数结构 */
struct mid_attr {
    unsigned magic;
    unsigned task_id;
    unsigned task_qid;
    unsigned flags;
};

/**
 * @brief 创建任务消息队列和事件队列接口
 *
 * @par 描述:
 * 创建任务消息队列和事件队列
 *
 * @attention
 * <ul><li>一个任务必须调用该接口，才能接收消息和事件</li></ul>
 * <ul><li>事件队列固定创建一个，消息队列创建个数由参数决定</li></ul>
 * <ul><li>消息队列编号为[0,q_num-1],多个队列时，编号越大优先级越高</li></ul>
 *
 * @param[in]  task_id, 任务id，在kernel下即pid，可以从task_struct中的pid得到
 * @param[in]  q_num, 创建的消息队列数，目前最大支持16个消息队列
 *
 * @retval 0,创建成功
 * @retval 非0,创建失败。
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_register_mid mdrv_msg_recv
 */
MODULE_EXPORTED int mdrv_msg_qcreate(unsigned task_id, unsigned q_num);

/**
 * @brief 初始化注册mid使用的参数结构
 *
 * @par 描述:
 * struct mid_attr在使用前，需要先使用该函数进行初始化，将某些参数赋值为默认值
 *
 * @param[in]  attr, 注册mid使用的参数结构
 *
 * @retval 0,成功
 * @retval 非0,失败。
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_mid_reg
 */
MODULE_EXPORTED int mdrv_msg_attr_init(struct mid_attr *attr);

/**
 * @brief 绑定并注册mid的接口
 *
 * @par 描述:
 * 将任务id和消息队列id绑定注册到某个mid上
 *
 * @attention
 * <ul><li>一个任务必须调用该接口，才能接收使用mid发过来的消息</li></ul>
 *
 * @param[in]  mid, 注册绑定的mid, mid用于唯一标识模块，在modemconfig下添加
 * @param[in]  attr, 注册mid使用的参数结构
 *
 * @retval 0,成功
 * @retval 非0,失败。
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_attr_init
 */
MODULE_EXPORTED int mdrv_msg_mid_reg(unsigned mid, struct mid_attr *attr);

/**
 * @brief 绑定并注册mid的接口
 *
 * @par 描述:
 * 将任务id和消息队列id绑定注册到某个mid上
 *
 * @attention
 * <ul><li>一个任务必须调用该接口，才能接收使用mid发过来的消息</li></ul>
 *
 * @param[in]  task_id, 任务id
 * @param[in]  qid, 创建任务时创建的消息队列的id，从0开始
 *
 * @retval 0,绑定成功
 * @retval 非0,绑定失败。
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_qcreate
 */
MODULE_EXPORTED int mdrv_msg_register_mid(unsigned task_id, unsigned mid, unsigned qid);

/**
 * @brief 消息发送接口
 *
 * @par 描述:
 * 根据msg中填写好的字段，将消息发送出去，关键字段包括源id，目的id和长度
 *
 * @attention
 * <ul><li>发送成功之后，发送方不应该再访问msg消息，也应该不需要释放msg消息</li></ul>
 *
 * @param[in]  msg, 通过mdrv_msg_alloc得到，且经过了MDRV_MSG_HEAD_FILL。
 * @param[in]  flag, 默认填0，发送紧急消息时填MSG_UG。
 *
 * @retval 0,发送成功
 * @retval 非0,发送失败。
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_alloc MDRV_MSG_HEAD_FILL
 */
MODULE_EXPORTED int mdrv_msg_send(void *msg, unsigned flag);

/**
 * @brief 消息接收接口
 *
 * @par 描述:
 * 从当前任务的队列中接收消息
 *
 * @attention
 * <ul><li>调用前需要保证队列已经创建ok</li></ul>
 * <ul><li>只能在任务上下文调用，在禁止调度的任务上下文中不能阻塞</li></ul> 
 *
 * @param[in]  qid,  创建任务时的消息队列的id，从0开始，也可以使用MDRV_MSG_QID_ANY
 * @param[out]  msg, 接收到的消息，该消息可以认为是用mdrv_msg_alloc申请
 * @param[in]  time_out,  超时时间，以当前系统tick为单位，内核上为HZ
 * 当超时时间为0时，代表非阻塞RECV,可以在禁止调度的任务上下文中使用
 * 当超时时间为MDRV_MSG_WAIT_FOREVER时，代表永不超时
 * 当超时时间为其他值时，按照tick计数，若计时到达前事件满足，则正常返回，否则超时返回
 * @retval 0,接收成功
 * @retval 非0,接收失败。
 * @retval MDRV_MSG_ERR_QUEUE_EMPTY 没有消息，接收超时
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_send
 */
MODULE_EXPORTED int mdrv_msg_recv(unsigned qid, void **msg, unsigned time_out);

/**
 * @brief V消息发送接口
 *
 * @par 描述:
 * 根据msg中填写好的字段，将消息发送出去，关键字段包括源id，目的id和长度
 *
 * @attention
 * <ul><li>与mdrv_msg_send区别在于该接口可以唤醒mdrv_event_vrecv</li></ul>
 *
 * @param[in]  msg, 通过mdrv_msg_alloc得到，且经过了MDRV_MSG_HEAD_FILL
 * @param[in]  flag, 默认填0，发送紧急消息时填MSG_UG
 *
 * @retval 0,发送成功
 * @retval 非0,发送失败。
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_alloc MDRV_MSG_HEAD_FILL mdrv_msg_recv mdrv_event_vrecv
 */
MODULE_EXPORTED int mdrv_msg_vsend(void *msg, unsigned flag);

/**
 * @brief 获取消息数目接口
 *
 * @par 描述:
 * 查询某个任务某个消息队列中有多少条消息
 *
 * @param[in]  task_id, 任务id
 * @param[in]  qid,  创建的消息队列的id，从0开始
 *
 * @retval >=0, 查询成功，返回消息数量
 * @retval <0, 查询失败。
 * @par 依赖:
 * <ul><li>mdrv_msg.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_msg_send mdrv_msg_recv
 */
MODULE_EXPORTED int mdrv_msg_num_get(unsigned task_id, unsigned qid);

/** @brief 非公开接口 */
struct msg_fixed_filed {
    unsigned int src_id;
    unsigned int dst_id;
    unsigned int payload[0x2];
};

/** @brief 非公开接口 */
MODULE_EXPORTED int mdrv_msg_fixed_send(struct msg_fixed_filed *fixed_msg, unsigned urgent);
/** @brief 非公开接口 */
MODULE_EXPORTED int mdrv_msg_fixed_recv(unsigned qid, struct msg_fixed_filed *fixed_msg, unsigned time_out);

/**
 * @brief msg通道打开后返回句柄类型
 * @attention
 * <ul><li>用户不需要使用该结构类型, 只需要使用该结构类型的指针</li></ul>
 */
struct msg_chn_hdl;

/**
 * @brief 描述通道位于哪个核
 * @attention
 * <ul><li>次枚举只能用于相等判断，用户不应该依赖此枚举的排列顺序，枚举号大小</li></ul>
 */
enum {
    MSG_CORE_STUB = 0, /* 编号为0的空着不用 */
    MSG_CORE_APP = 1,
    MSG_CORE_TSP = 2,
    MSG_CORE_LPM = 4,
    MSG_CORE_TEE = 6,
    MSG_CORE_SENSOR = 7,
    MSG_CORE_CNTMAX,
};

/** @brief 用于msgchn_attr的coremask生成宏 */
#define MSG_CORE_MASK(x) (1U << (x))

/** @brief 描述msg通道的地址结构 */
struct msg_addr {
    unsigned core;
    unsigned chnid;
};

/**
 * @brief 用户打开通道时的输入参数
 * @attention
 * <ul><li>该结构体在用于open前,需要使用mdrv_msgchn_attr_init初始化</li></ul>
 * <ul><li>该结构仅用于打开通道，用户不应该存储，也不应该依赖本结构体的成员排序，字段大小，结构大小等</li></ul>
 */
struct msgchn_attr {
    unsigned magic;     /**< 由mdrv_msgchn_attr_init填写，用户不要访问 */
    unsigned chnid;     /**< 通道的id定义, 在mdrv_msgchn_attr_init后用户填写 */
    unsigned coremask;  /**< 允许从哪几个核收消息 */
    int (*lite_notify)(const struct msg_addr *src, void *msg, unsigned len);
};

/**
 * @brief 初始化属性的接口
 *
 * @par 描述:
 * 用于描述打开通道的各种参数，该函数用于实现将pattr设置为默认值
 *
 * @param[in]  pattr, 通道属性描述
 */
void mdrv_msgchn_attr_init(struct msgchn_attr *pattr);

/**
 * @brief 通道打开接口
 *
 * @par 描述:
 * 打开通道的接口，存在约束，需mdrv_msgchn_attr_init初始化pattr
 *
 * @param[in]  pattr, 通道属性描述
 *
 * @retval 非NULL, 执行成功
 * @retval NULL, 执行失败
 */
struct msg_chn_hdl *mdrv_msgchn_lite_open(struct msgchn_attr *pattr);

/**
 * @brief 数据发送接口
 *
 * @par 描述:
 * 将消息数据发送到目的端
 *
 * @param[in]  chn_hdl, 通道成功打开后返回的句柄
 * @param[in]  dst, 该消息发往的目的地描述
 * @param[in]  msg, 需要发送的数据buff指针
 * @param[in]  len, 需要发送的数据buff的长度
 *
 * @retval 0, 执行成功
 * @retval 非0, 执行失败
 */
int mdrv_msgchn_lite_sendto(struct msg_chn_hdl *chn_hdl, const struct msg_addr *dst, void *msg, unsigned len);

#ifdef __cplusplus
}
#endif
#endif
