/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
 *  @brief   mdrv_socket在acore上的对外头文件
 *  @file    mdrv_socket.h
 *  @author  liuyunfei
 *  @version v1.0
 *  @date    2018.02.24
 *  @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2018.02.24|创建文件</li></ul>
 *  @since
 */
#ifndef _HI_SOCKET_H
#define _HI_SOCKET_H

#include <linux/socket.h>
#include <linux/net.h>
#include <linux/time.h>
#include "product_config.h"
#include "securec.h"

/**
* @brief socket fd bit 指示
*/
typedef struct {
    unsigned int fds_bits;
} hi_fd_set;

/**
* @brief socket send/recv info
*/
struct hisk_info {
    int fd;
    void *buff;
    size_t len;
};

/**
* @brief socket fd所占bit个数
*/
#define __HI_NFDBITS	(8 * sizeof(unsigned int))

/**
* @brief 求socket fd第d bit掩码，
* @note d=1时 MASK=0x2
*/
#define __HI_FDMASK(d)	(1UL << ((d) % __HI_NFDBITS))

/**
* @brief 判断set所指示fd 第d bit是否被置位
*/
#define HI_FD_ISSET(d, set) ((set)->fds_bits & __HI_FDMASK(d))

/**
* @brief 将set所指示fd 第d bit置位
*/
#define HI_FD_SET(d, set) ((set)->fds_bits |= __HI_FDMASK(d))

/**
* @brief 将set所指示fd 第d bit复位
*/
#define HI_FD_CLR(d, set)  ((set)->fds_bits &= ~(__HI_FDMASK(d)))

/**
* @brief 指示关闭socket维测
*/
#define DISABLE_SOCKET_DEBUG 0

/**
* @brief 指示开启socket维测
*/
#define ENABLE_SOCKET_DEBUG 1

/**
* @brief 将set指示socket fd全体bit置0
*/
void mdrv_sock_fd_zero(hi_fd_set* set);
#if (defined(FEATURE_SVLSOCKET) || ((defined(FEATURE_HISOCKET)) && (FEATURE_ON == FEATURE_HISOCKET)))

/**
 * @brief 创建socket
 * @param[in]  family , 地址族 IP 地址类型，常用 AF_INET，IPv4 地址 和 AF_INET6， IPv6 地址
 * @param[in]  type, 物理通道号，常用SOCK_STREAM面向连接的数据传输方式 和 SOCK_DGRAM无连接的数据传输方式
 * @param[in]  protocol，协议类型，0 是指不指定协议类型，系统自动根据情况指定
 * @retval 非负数, 表示函数执行成功，返回socketfd值
 * @retval -1,表示函数执行失败
 */
int mdrv_socket(int family, int type, int protocol);

/**
 * @brief 绑定socket
 * @param[in]  fd , socket fd，需要创建socket返回fd值，其他值非法
 * @param[in]  addr, 要绑定的端口
 * @param[in]  addrlen, addr结构长度
 * @retval 0,表示函数执行成功
 * @retval ！=0, 表示函数执行失败
 */
int mdrv_bind(int fd, struct sockaddr *addr, int addrlen);

/**
 * @brief 监听socket
 * @param[in]  fd , socket fd，需要创建socket返回fd值，其他值非法
 * @param[in]  backlog, 控制accept queue大小要素之一，推荐配置1
 * @retval 0,表示函数执行成功
 * @retval ！=0, 表示函数执行失败
 */
int mdrv_listen(int fd, int backlog);

/**
 * @brief 获取客户端连接
 * @param[in]  fd , sever端创建服务用socket fd
 * @param[out]  upeer_sockaddr, 客户端socket地址信息
 * @param[out]  upeer_addrlen, upeer_sockaddr长度
 * @retval 非负数, 表示函数执行成功，返回与客户端连接的socketfd值
 * @retval -1,表示函数执行失败
 */
int mdrv_accept(int fd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen);

/**
 * @brief 客户端连接服务端
 * @param[in]  fd , 客户端创建连接用socket fd
 * @param[out]  addr, 服务端socket地址信息
 * @param[out]  addrlen, addr长度
 * @retval 非负数, 表示函数执行成功，返回与服务端连接的socketfd值
 * @retval -1,表示函数执行失败
 */
int mdrv_connect(int fd, struct sockaddr *addr, int addrlen);

/**
 * @brief 监听接收、发送、异常
 * @param[in]  n, 待监听的socketfd最大值+1
 * @param[in,out]  inp, 设置关注接收消息的socketfd bitmap
 * @param[in,out]  outp, 设置关注发送消息的socketfd bitmap
 * @param[in,out]  exp, 设置关注异常消息的socketfd bitmap
 * @param[in]  tvp, 设置超时时间
 * @retval 非负数, 表示函数执行成功
 * @retval -1,错误
 */
int mdrv_select(int n, hi_fd_set *inp, hi_fd_set *outp, hi_fd_set *exp, struct timeval *tvp);

/**
 * @brief 关闭socket
 * @param[in]  fd, 要关闭的socket fd
 * @retval 0, 表示函数执行成功
 * @retval -1,错误
 */
int mdrv_close(int fd);

/**
 * @brief socket 发送
 * @param[in]  fd, 发送使用socket fd
 * @param[in]  buff, 发送buff
 * @param[in]  len, 发送buff长度
 * @param[in]  flags, 发送推荐值0
 * @retval 非负数, 表示函数执行成功，返回发送字节数
 * @retval -1,错误
 */
int mdrv_send(int fd, void *buff, size_t len, int flags);

/**
 * @brief socket 接收
 * @param[in]  fd, 接收使用socket fd
 * @param[out]  buff, 接收buff
 * @param[in]  len, 接收buff长度
 * @param[in]  flags, 接收推荐值0
 * @retval 非负数, 表示函数执行成功，返回接收字节数
 * @retval -1,错误
 */
int mdrv_recv(int fd, void *ubuf, size_t size, int flags);

/**
 * @brief socket UDP发送
 * @param[in]  fd, 发送使用socket fd
 * @param[in]  buff, 发送buff
 * @param[in]  len, 发送buff长度
 * @param[in]  flags, 发送推荐值0
 * @param[in]  addr, 指示发送目标地址
 * @param[in]  addr_len, addr 长度
 * @retval 非负数, 表示函数执行成功，返回发送字节数
 * @retval -1,错误
 */
int mdrv_sendto(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int addr_len);

/**
 * @brief socket UDP接收
 * @param[in]  fd, 接收使用socket fd
 * @param[out]  buff, 接收buff
 * @param[in]  len, 接收buff长度
 * @param[in]  addr, 指示接收目标地址
 * @param[in]  addr_len, addr 长度
 * @param[in]  flags, 接收推荐值0
 * @retval 非负数, 表示函数执行成功，返回接收字节数
 * @retval -1,错误
 */
int mdrv_recvfrom(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int *addr_len);

/**
 * @brief 获取与某个套接字关联的本地协议地址
 * @param[in]  fd, socket fd
 * @param[out]  addr, 返回协议地址
 * @param[in]  addrlen, addr 长度
 * @retval 0, 表示函数执行成功
 * @retval -1,错误
 */
int mdrv_getsockname(int fd, struct sockaddr *addr, int *addrlen);

/**
 * @brief 获取与某个套接字关联的外地协议地址
 * @param[in]  fd, socket fd
 * @param[out]  addr, 返回协议地址
 * @param[in]  addrlen, addr 长度
 * @retval 0, 表示函数执行成功
 * @retval -1,错误
 */
int mdrv_getpeername(int fd, struct sockaddr *addr, int *addrlen);

/**
 * @brief 获取与某个套接字关联的选项
 * @param[in]  fd, socket fd
 * @param[in]  level, 选项所在的协议层
 * @param[in]  optname, 需要访问的选项名
 * @param[out]  optval, 指向返回选项值的缓冲
 * @param[in,out]  optlen, 入参时选项值的最大长度,返回实际长度
 * @retval 0, 表示函数执行成功
 * @retval -1,错误
 */
int mdrv_getsockopt(int fd, int level, int optname, char *optval, int *optlen);

/**
 * @brief 设置与某个套接字关联的选项
 * @param[in]  fd, socket fd
 * @param[in]  level, 选项所在的协议层
 * @param[in]  optname, 需要访问的选项名
 * @param[in]  optval, 指向包含新选项值的缓冲
 * @param[in,out]  optlen, 现选项的长度
 * @retval 0, 表示函数执行成功
 * @retval -1,错误
 */
int mdrv_setsockopt(int fd, int level, int optname, char *optval, unsigned int optlen);

/**
 * @brief socket ioctl
 * @param[in]  fd, socket fd
 * @param[in]  cmd, 发送指令
 * @param[in]  arg, 参数
 * @retval 0, 表示函数执行成功
 * @retval -1,错误
 */
int mdrv_ioctl(int fd, int cmd, unsigned int arg);

/**
 * @brief 关闭socket端口
 * @param[in]  fd, socket fd
 * @param[in]  how, 关闭收方向、发方向或双向
 * @retval 0, 表示函数执行成功
 * @retval -1,错误
 */
int mdrv_shutdown(int fd, enum sock_shutdown_cmd how);

/**
 * @brief 字符串转换为ipv4地址
 * @param[in]  str, 字符串表示的ipv4地址
 * @retval 非负值, 表示函数执行成功，转换后的ipv4地址
 * @retval -1,错误
 */
unsigned int mdrv_inet_addr(const char * str);

#else
static inline int mdrv_socket(int family, int type, int protocol)
{
    return 0;
}
static inline int mdrv_bind(int fd, struct sockaddr *addr, int addrlen)
{
    return 0;
}
static inline  int mdrv_listen(int fd, int backlog)
{
    return 0;
}
static inline  int mdrv_accept(int fd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen)
{
    return 0;
}
static inline  int mdrv_connect(int fd, struct sockaddr *addr, int addrlen)
{
    return 0;
}
static inline  int mdrv_select(int n, hi_fd_set *inp, hi_fd_set *outp, hi_fd_set *exp, struct timeval *tvp)
{
    return 0;
}
static inline  int mdrv_close(int fd)
{
    return 0;
}
static inline  int mdrv_send(int fd, void *buff, size_t len, int flags)
{
    return 0;
}
static inline  int mdrv_recv(int fd, void *ubuf, size_t size, int flags)
{
    return 0;
}
static inline  int mdrv_sendto(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int addr_len)
{
    return 0;
}
static inline  int mdrv_recvfrom(struct hisk_info *hiskinfo, unsigned flags, struct sockaddr *addr, int *addr_len)
{
    return 0;
}
static inline  int mdrv_getsockname(int fd, struct sockaddr *addr, int *addrlen)
{
    return 0;
}
static inline  int mdrv_getpeername(int fd, struct sockaddr *addr, int *addrlen)
{
    return 0;
}
static inline  int mdrv_getsockopt(int fd, int level, int optname, char *optval, int *optlen)
{
    return 0;
}
static inline  int mdrv_setsockopt(int fd, int level, int optname, char *optval, unsigned int optlen)
{
    return 0;
}
static inline  int mdrv_ioctl(int fd, int cmd, unsigned int arg)
{
    return 0;
}
static inline  int mdrv_shutdown(int fd, enum sock_shutdown_cmd how)
{
    return 0;
}
static inline  unsigned int mdrv_inet_addr(const char * str)
{
    return 0;
}
#endif

#endif

