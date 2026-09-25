/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015. All rights reserved.
 * Description: process the event for hongbao fastgrab in the kernel
 *
 * Author: zhuweichen@huawei.com
 * Create: 2015-09-30
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/file.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/fdtable.h>
#include <linux/tcp.h>
#include <net/sock.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/inet_sock.h>

#include <huawei_platform/net/bastet/bastet_utils.h>
#if (KERNEL_VERSION(4, 14, 0) <= LINUX_VERSION_CODE)
#include <linux/sched/task.h>
#endif
#ifdef CONFIG_CHR_NETLINK_MODULE
#include <hwnet/booster/chr_manager.h>
#endif

#ifdef CONFIG_CHR_NETLINK_MODULE
#define MSG_LEN 4
#endif

struct bst_fg_app_info_stru g_fast_grab_app_info[BST_FG_MAX_APP_NUMBER];
uid_t g_currnt_acc_uid;
struct bst_fg_custom_stru g_st_bastet_fg_custom;
struct bst_fg_hongbao_stru g_st_bastet_fg_hongbao;

#ifdef CONFIG_CHR_NETLINK_MODULE
static notify_event *g_notifier = NULL;
#endif

/**
 * bst_fg_init() - initialize the structure of the fast
 *                 preemption technology
 *
 * initialize the global struct.
 * Return: None
 */
void bst_fg_init(void)
{
	uint16_t i;
	uint16_t j;

	for (i = 0; i < BST_FG_MAX_APP_NUMBER; i++) {
		g_fast_grab_app_info[i].l_uid = BST_FG_INVALID_UID;
		g_fast_grab_app_info[i].us_index = i;
		spin_lock_init(&g_fast_grab_app_info[i].st_locker);
		for (j = 0; j < BST_FG_MAX_KW_NUMBER; j++)
			g_fast_grab_app_info[i].pst_kws[j] = NULL;
	}
	g_currnt_acc_uid = BST_FG_INVALID_UID;

	g_st_bastet_fg_custom.uc_app_num = 0;
	for (i = 0; i < BST_FG_MAX_CUSTOM_APP; i++) {
		g_st_bastet_fg_custom.ast_custom_info[i].l_uid = 0;
		g_st_bastet_fg_custom
			.ast_custom_info[i].uc_protocol_bitmap = 0;
		g_st_bastet_fg_custom.ast_custom_info[i].uc_discard_timer = 0;
		g_st_bastet_fg_custom
			.ast_custom_info[i].uc_tcp_retran_discard = 0;
		g_st_bastet_fg_custom.ast_custom_info[i].uc_app_type = 0;
	}

	g_st_bastet_fg_hongbao.uc_congestion_proc_flag = false;
	g_st_bastet_fg_hongbao.uc_duration_time = 0;
}

/**
 * bst_fg_proc_wx_sock() - Processing WeChat Socket Status Changes
 * @pstSock: the object of sock
 * @state: the state of new socket
 *
 * process the wechat socket connection states.
 *
 * Return: None
 */
static void bst_fg_proc_wx_sock(struct sock *pst_sock, int state)
{
	/* If new socket is built, we think it to "waiting" state */
	if (state == TCP_ESTABLISHED) {
		pst_sock->fg_Spec = BST_FG_WECHAT_WAIT_HONGBAO;
		bastet_logi("%s", __func__, " Set To WAIT_HONGBAO, PTR=%p",
			pst_sock);
	}
	/*
	 * Only out put a log. But now wx socket is using "RST"
	 * but "Close", so this
	 * can't be printed;
	 */
	else if (state == TCP_CLOSING) {
		if (pst_sock->fg_Spec == BST_FG_WECHAT_HONGBAO)
			bastet_logi("%s", __func__,
			" Hongbao_socket is Removed");
	} else {
		return;
	}
}

/**
 * bst_fg_proc_wx_match_keyword_dl() - process the matched dl keyword
 * @pstKwdIns: the key words that needed to matched.
 * @pData: the received Downlink data(from skbuff)
 * @ulLength: Maximum valid data length
 *
 * This function is used to process WeChat socket downstream hook
 * packets. This parameter is valid only for push channels.
 *
 * Return: true - matche.
 *         false - not match.
 */
static bool bst_fg_proc_wx_match_keyword_dl(
	struct bst_fg_keyword_stru *pst_kwd_ins,
	uint8_t *p_data,
	uint32_t ul_length)
{
	uint64_t us_tot_len;

	if (pst_kwd_ins == NULL)
		return false;

	/*
	 * probe log, get the packet head
	 * match the length of XML to dl packet
	 */
	if ((ul_length > pst_kwd_ins->st_len_porp.us_max)
		|| (ul_length < pst_kwd_ins->st_len_porp.us_min))
		return false;

	/* match keywords */
	us_tot_len = pst_kwd_ins->st_key_word.us_tot_len;
	if (memcmp(&pst_kwd_ins->st_key_word.auc_data[0],
		p_data, us_tot_len) == 0)
		return true;
	return false;
}

/**
 * bst_fg_proc_wx_packet_dl() - handle dl wx packet.
 * @pstSock: the object of sock
 * @pData: the received Downlink data(from skbuff)
 * @ulLength: Maximum valid data length
 *
 * This function is used to process WeChat socket
 * downstream hook packets. This parameter is valid
 * only for push channels.
 *
 * Return: None
 */
static uint8_t bst_fg_proc_wx_packet_dl(
	struct sock *pst_sock,
	uint8_t *p_data,
	uint32_t ul_length)
{
	struct bst_fg_keyword_stru *pst_kwd_ins = NULL;
	struct bst_fg_keyword_stru *pst_kwd_ins_new = NULL;
	bool b_found = false;

	if (pst_sock->fg_Spec != BST_FG_WECHAT_PUSH)
		return 0;
	/* Set the "PUSH"( 0 0 4 ) SIP-Command to be compared object. */
	pst_kwd_ins = bst_fg_get_app_ins(BST_FG_IDX_WECHAT)
		->pst_kws[BST_FG_FLAG_WECHAT_PUSH];
	b_found = bst_fg_proc_wx_match_keyword_dl(pst_kwd_ins, p_data,
		ul_length);
	if (b_found)
		return 1;
	pst_kwd_ins_new = bst_fg_get_app_ins(BST_FG_IDX_WECHAT)
		->pst_kws[BST_FG_FLAG_WECHAT_PUSH_NEW];
	b_found = bst_fg_proc_wx_match_keyword_dl(pst_kwd_ins_new,
		p_data, ul_length);
	if (b_found)
		return 1;
	return 0;
}

/*
 * bst_fg_proc_send_rpacket_priority() - Determine whether WeChat
 *     data is processed based on WeChat data.
 * @pstSock: the object of sock
 * @pData: the received Downlink data(forced conversion msg,
 *          iov user space data)
 * @ulLength: Maximum valid data length
 *
 * this function was invoked in tcp_output.c/tcp_write_xmit.
 *
 * Return: 0 - No key data information
 *         1 - has key data information
 *
 */
uint8_t bst_fg_proc_send_rpacket_priority(struct sock *pst_sock)
{
	if (!pst_sock)
		return 0;
	if (pst_sock->fg_Spec == BST_FG_WECHAT_HONGBAO) {
		bastet_logi("%s", __func__, " Hongbao_packet");
		return 1;
	}
	return 0;
}

static bool judge_the_data_length(uint32_t ul_length,
	struct bst_fg_keyword_stru *pst_kwd_ins,
	struct sock *pst_sock)
{
	if ((ul_length < pst_kwd_ins->st_len_porp.us_copy) ||
		(pst_kwd_ins->st_len_porp.us_copy >
			BST_FG_MAX_US_COPY_NUM) ||
		(pst_kwd_ins->st_len_porp.us_copy == 0)) {
		bastet_logi("%s", __func__,
			" pstKwdIns->stLenPorp.usCopy is invalid");
		bst_fg_set_sock_special(pst_sock, BST_FG_NO_SPEC);
		return false;
	}
	return true;
}

/**
 * bst_fg_proc_wx_packet_ul() - Processing WeChat Upload hook Packets
 * @pstSock: the object of sock
 * @pData: the received Downlink data(forced conversion msg,
 *         iov user space data)
 * @ulLength: Maximum valid data length
 *
 * copy the date from user space to kernel,process the wx ul packet.
 *
 * Return: 0 - No key data information
 *         1 - has key data information
 *
 */
static uint8_t bst_fg_proc_wx_packet_ul(
	struct sock *pst_sock,
	uint8_t *p_data,
	uint32_t ul_length)
{
	struct bst_fg_keyword_stru *pst_kwd_ins = NULL;
	char *pc_found = NULL;
	char *p_usr_data = NULL;
	struct msghdr *p_usr_msg_hdr = NULL;

	/*
	 * If this sock has been matched tob HONGBAO-Server connection,
	 * return 1 direction to let HP data sending.
	 */
	if (pst_sock->fg_Spec == BST_FG_WECHAT_HONGBAO) {
		bastet_logi("%s", __func__, " Hongbao_socket is sending");
		return 1;
	}
	/*
	 * If this sock is "WAITING" to be matched state,
	 * here will Match the first sending packet
	 * of this sock to find "hongbao"-URL message.
	 * ATENTION: This function only execute matching one time per sock.
	 */
	if (pst_sock->fg_Spec == BST_FG_WECHAT_WAIT_HONGBAO) {
		/* Set the "hongbao"-URL to be compared object. */
		pst_kwd_ins = bst_fg_get_app_ins(BST_FG_IDX_WECHAT)
			->pst_kws[BST_FG_FLAG_WECHAT_GET];
		if (pst_kwd_ins == NULL)
			return 0;

		/* too short. */
		if (!judge_the_data_length(ul_length, pst_kwd_ins, pst_sock))
			return 0;
		/* Think it to be a common sock firstly. */
		pst_sock->fg_Spec = BST_FG_NO_SPEC;
		p_usr_data = kmalloc(pst_kwd_ins->st_len_porp.us_copy,
			GFP_ATOMIC);
		if (p_usr_data == NULL)
			return 0;
		/*
		 * Copy data from usr space.
		 * Set the last byte to '0' for strstr input.
		 */
		p_usr_msg_hdr = (struct msghdr *)p_data;
		if (copy_from_user(p_usr_data,
			p_usr_msg_hdr->msg_iter.iov->iov_base,
			pst_kwd_ins->st_len_porp.us_copy)) {
			kfree(p_usr_data);
			return 0;
		}
		p_usr_data[pst_kwd_ins->st_len_porp.us_copy - 1] = 0;
		pc_found = strstr((const char *)p_usr_data,
			(const char *)&pst_kwd_ins->st_key_word.auc_data[0]);
		kfree(p_usr_data);

		if (pc_found == NULL) {
			pst_sock->fg_Spec = BST_FG_NO_SPEC;
			return 0;
		}
		bst_fg_set_sock_special(pst_sock, BST_FG_WECHAT_HONGBAO);
		bastet_logi("%s", __func__, " Find New Hongbao_socket");
		return 1;
	}
	return 0;
}

/**
 * bst_fg_proc_wx_packet() - Processing WeChat hook Packages
 * @pstSock: the object of sock
 * @pData: the received data.
 * @ulLength: Maximum valid data length
 * @ulRole: TX/RX direction
 *
 * invoke the dl or ul process method by the ul_role.
 *
 * Return: 0 - No key data information
 *         1 - has key data information
 */
static uint8_t bst_fg_proc_wx_packet(
	struct sock *pst_sock,
	uint8_t *p_data,
	uint32_t ul_length,
	uint32_t ul_role)
{
	uint8_t uc_rtn = 0;
	/* Call UL/DL packet proc, according to ROLE */
	if (ul_role == BST_FG_ROLE_RCVER)
		uc_rtn = bst_fg_proc_wx_packet_dl(pst_sock, p_data, ul_length);

	else if (ul_role == BST_FG_ROLE_SNDER)
		uc_rtn = bst_fg_proc_wx_packet_ul(pst_sock, p_data, ul_length);

	return uc_rtn;
}

#ifdef CONFIG_CHR_NETLINK_MODULE
void regist_msg_fun(notify_event *notify)
{
	if (notify == NULL) {
		bastet_loge("notify null");
		return;
	}
	g_notifier = notify;
}

static void notify_fastgrab_chr()
{
	struct res_msg_head msg;

	if (g_notifier == NULL) {
		bastet_loge("g_notifier null");
		return;
	}
	msg.type = FASTGRAB_CHR_RPT;
	msg.len = MSG_LEN;
	g_notifier(&msg);
}
#endif

/**
 * bst_fg_hook_packet() - external interface,
 * Processing the packets sent and received
 *                       by the external socket
 * @pstSock: the object of sock
 * @pData: the received data.
 * @ulLength: Maximum valid data length
 * @ulRole: TX/RX direction
 *
 * hook the wx packet by us_idx,if the scenario is grab
 * hongbao, will start rrc alive timer.
 *
 * Return: 0 - No key data information
 *         1 - has key data information
 */
uint8_t bst_fg_hook_packet(
	struct sock *pst_sock,
	uint8_t *p_data,
	uint32_t ul_length,
	uint32_t ul_role)
{
	uid_t l_sock_uid = 0;
	uint8_t uc_rtn = 0;
	uint16_t us_idx = BST_FG_IDX_INVALID_APP;

	if (IS_ERR_OR_NULL(pst_sock)) {
		bastet_loge("invalid parameter");
		return 0;
	}
	/* Get and find the uid in fast-grab message information list. */
	l_sock_uid = sock_i_uid(pst_sock).val;
	for (us_idx = 0; us_idx < BST_FG_MAX_APP_NUMBER; us_idx++) {
		if (l_sock_uid == g_fast_grab_app_info[us_idx].l_uid)
			break;
	}
	if (!bst_fg_is_app_idx_valid(us_idx))
		return 0;

	bastet_logi("BST_FG_Hook1  Length=%u, Role=%u, sk_state=%d",
				 ul_length, ul_role, pst_sock->fg_Spec);
	/* Call different packet proc according the Application Name(Index). */
	switch (us_idx) {
	case BST_FG_IDX_WECHAT:
		uc_rtn = bst_fg_proc_wx_packet(pst_sock,
			p_data, ul_length, ul_role);
		if (uc_rtn) {
			post_indicate_packet(BST_IND_FG_KEY_MSG,
				&l_sock_uid, sizeof(uid_t));
#ifdef CONFIG_CHR_NETLINK_MODULE
			notify_fastgrab_chr();
#endif
		}
		break;

	default:
		break;
	}
	return uc_rtn;
}


static struct sock *bst_fg_get_sock_by_fd_pid(int fd, int32_t pid)
{
	struct inet_sock *isk = NULL;
	struct sock *sk = NULL;

	sk = get_sock_by_fd_pid(fd, pid);
	if (sk == NULL)
		return NULL;

	if (sk->sk_state != TCP_ESTABLISHED) {
		sock_put(sk);
		return NULL;
	}

	if (sk->sk_socket && sk->sk_socket->type !=
		SOCK_STREAM) {
		sock_put(sk);
		return NULL;
	}

	isk = inet_sk(sk);
	if (isk == NULL) {
		sock_put(sk);
		return NULL;
	}

	if ((isk->inet_daddr == BST_FG_INVALID_INET_ADDR)
		&& (isk->inet_dport ==
			BST_FG_INVALID_INET_ADDR)) {
		sock_put(sk);
		return NULL;
	}
	return sk;
}

static bool process_sock_find_result(struct sock **pst_sock_reg,
	uint16_t *us_found_port, int *l_found_fd, int fd,
	struct sock **pst_sock, struct inet_sock *pst_inet)
{
	/* If there no sock be found, set the ptr to tmp-ptr. */
	if (*pst_sock_reg == NULL) {
		*l_found_fd = fd;
		*us_found_port = pst_inet->inet_sport;
		*pst_sock_reg = *pst_sock;
	} else {
		/*
		 * If there is a sock has been recorded,we will
		 * check fd/port to judge if it is the same one.
		 * If it's another one, it means that there is
		 * not only one sock in this uid, unsuccessful.
		 */
		if ((fd == *l_found_fd) && (*us_found_port ==
			pst_inet->inet_sport)) {
			sock_put(*pst_sock);
		} else {
			sock_put(*pst_sock);
			sock_put(*pst_sock_reg);
			bastet_logi("%s", __func__, " More than One Push Socket Found");
			return false;
		}
	}
	return true;
}

static struct files_struct *find_file_sys_head_according_to_task(
	struct task_struct *pst_task)
{
	struct files_struct *pst_files = NULL;

	if (pst_task == NULL) {
		bastet_loge("%s", __func__, " pstTask error");
		rcu_read_unlock();
		return NULL;
	}
	get_task_struct(pst_task);
	rcu_read_unlock();
	/* Find File sys head according to task */
	pst_files = pst_task->files;
	if (pst_files == NULL) {
		put_task_struct(pst_task);
		bastet_loge("%s", __func__, " pstFiles error");
		return NULL;
	}
	return pst_files;
}

static int bst_fg_set_hongbao_push_sock(int32_t tid_num, int32_t *tids)
{
	struct task_struct *pst_task = NULL;
	struct files_struct *pst_files = NULL;
	struct fdtable *pst_fdt = NULL;
	struct sock *pst_sock = NULL;
	struct sock *pst_sock_reg = NULL;
	struct inet_sock *pst_inet = NULL;
	uint16_t us_looper = 0;
	int fd = -1;
	uint16_t us_found_port = 0;
	int l_found_fd = -1;

	/* use pid to proc. */
	for (us_looper = 0; us_looper < tid_num; us_looper++) {
		/* Find task message head */
		rcu_read_lock();
		pst_task = find_task_by_vpid(tids[us_looper]);
		pst_files = find_file_sys_head_according_to_task(pst_task);
		if (pst_files == NULL)
			return -EFAULT;
		put_task_struct(pst_task);
		/* list the fd table */
		pst_fdt = files_fdtable(pst_files);
		for (fd = 0; fd < pst_fdt->max_fds; fd++) {
			/* check the state, ip-addr, port-num of this sock */
			pst_sock = bst_fg_get_sock_by_fd_pid(fd,
				tids[us_looper]);
			if (!pst_sock)
				continue;
			pst_inet = inet_sk(pst_sock);
			if (!process_sock_find_result(&pst_sock_reg,
				&us_found_port, &l_found_fd, fd, &pst_sock,
				pst_inet))
				return 0;
		}
	}
	if (pst_sock_reg != NULL) {
		/* record the PUSH special flag to this sock */
		bst_fg_set_sock_special(pst_sock_reg, BST_FG_WECHAT_PUSH);
		sock_put(pst_sock_reg);
		bastet_logi("%s", __func__, " Found Success!");
	}
	return 0;
}

static bool get_keyword_data_block_from_user_space(
	struct bst_fg_keyword_stru **pst_kwd_src,
	struct bst_fg_keyword_stru **pst_kwd_dst,
	void __user *argp,
	uint16_t us_copyed,
	struct bst_fg_kws_comm_stru st_kws_comm)
{
	/* Get keyword data block from usr space */
	if ((*pst_kwd_src)->st_key_word.us_tot_len > BST_FG_MAX_KW_LEN) {
		kfree(*pst_kwd_dst);
		return false;
	}
	if (copy_from_user(&((*pst_kwd_dst)->st_key_word.auc_data[0]),
		(uint8_t *)argp + us_copyed + sizeof(st_kws_comm),
		(*pst_kwd_src)->st_key_word.us_tot_len)) {
		bastet_loge("%s", __func__, " copy_from_user aucData error");
		kfree(*pst_kwd_dst);
		return false;
	}

	return true;
}

static bool get_keyword_data_from_user_space(
	struct bst_fg_keyword_stru **pst_kwd_src,
	struct bst_fg_keyword_stru **pst_kwd_dst,
	void __user *argp,
	uint16_t us_copyed,
	struct bst_fg_kws_comm_stru st_kws_comm)
{
	/* Get keyword data structure from usr space */
	if (copy_from_user(*pst_kwd_src,
		(uint8_t *)argp + us_copyed + sizeof(st_kws_comm),
		sizeof(**pst_kwd_src))) {
		bastet_loge("%s...", __func__,
			" copy_from_user pstKwdSrc error");
		return false;
	}
	if ((*pst_kwd_src)->st_key_word.us_tot_len > BST_FG_MAX_KW_LEN)
		return false;
	*pst_kwd_dst = kmalloc(sizeof(**pst_kwd_dst) +
		(*pst_kwd_src)->st_key_word.us_tot_len,
		GFP_ATOMIC);
	if (*pst_kwd_dst == NULL)
		return false;

	memcpy(*pst_kwd_dst, *pst_kwd_src, sizeof(**pst_kwd_src));
	return true;
}

/**
 * bst_fg_save_keyword() - save the keyword.
 * @arg: the iov pointer of user space.
 *
 * Save Application Matching Keyword Information.
 *
 * Return: None
 */
static void bst_fg_save_keyword(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct bst_fg_app_info_stru *pst_app_ins = NULL;
	struct bst_fg_keyword_stru *pst_kwd_src = NULL;
	struct bst_fg_keyword_stru *pst_kwd_dst = NULL;
	struct bst_fg_kws_comm_stru st_kws_comm = {0};
	uint16_t us_copyed = 0;
	uint16_t us_looper = 0;

	/* Get keyword head information from usr space */
	if (copy_from_user(&st_kws_comm, argp, sizeof(st_kws_comm))) {
		bastet_loge("%s", __func__, " copy_from_user stKwsComm error");
		return;
	}
	if (!bst_fg_is_app_idx_valid(st_kws_comm.us_index))
		return;

	bastet_logi("%s", __func__, " stKwsComm.usIndex=%d",
		st_kws_comm.us_index);
	pst_kwd_src = kmalloc(sizeof(*pst_kwd_src), GFP_ATOMIC);
	if (pst_kwd_src == NULL)
		return;

	pst_app_ins = bst_fg_get_app_ins(st_kws_comm.us_index);
	us_looper = 0;
	spin_lock_bh(&pst_app_ins->st_locker);
	while (1) {
		if (!get_keyword_data_from_user_space(&pst_kwd_src,
			&pst_kwd_dst, argp, us_copyed, st_kws_comm))
			break;
		us_copyed += sizeof(*pst_kwd_dst);
		if (pst_kwd_src->st_key_word.us_tot_len > 0) {
			/* Get keyword data block from usr space */
			if (!get_keyword_data_block_from_user_space(
				&pst_kwd_src, &pst_kwd_dst, argp, us_copyed,
				st_kws_comm))
				break;
			us_copyed += pst_kwd_src->st_key_word.us_tot_len;
		}
		if (pst_app_ins->pst_kws[us_looper] != NULL)
			kfree(pst_app_ins->pst_kws[us_looper]);

		pst_app_ins->pst_kws[us_looper] = pst_kwd_dst;


		if (us_copyed > st_kws_comm.us_key_length) {
			kfree(pst_kwd_dst);
			pst_app_ins->pst_kws[us_looper] = NULL;
			break;
		}
		us_looper += 1;
		if (us_looper >= (uint16_t)BST_FG_MAX_KW_NUMBER)
			break;
	}
	kfree(pst_kwd_src);
	spin_unlock_bh(&pst_app_ins->st_locker);
}

/**
 * bst_fg_save_uid_info() - Save the uid info.
 * @arg: the iov pointer of user space.
 *
 * Save the UID update information of the application.
 *
 * Return: None
 */
static void bst_fg_save_uid_info(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct bst_fg_app_info_stru *pst_app_ins = NULL;
	struct bst_fg_uid_comm_stru st_uid_comm = {0};

	/* Get uid message structure from usr space */
	if (copy_from_user(&st_uid_comm, argp, sizeof(st_uid_comm))) {
		bastet_loge("%s", __func__, " copy_from_user error");
		return;
	}
	if (!bst_fg_is_app_idx_valid(st_uid_comm.us_index))
		return;

	pst_app_ins = bst_fg_get_app_ins(st_uid_comm.us_index);
	spin_lock_bh(&pst_app_ins->st_locker);
	pst_app_ins->l_uid = st_uid_comm.l_uid;
	bastet_logi("%s", __func__, " app_index=%d, uid=%d",
		pst_app_ins->us_index, pst_app_ins->l_uid);
	spin_unlock_bh(&pst_app_ins->st_locker);
}

/**
 * bst_fg_save_tid_info() - save tid info.
 * @arg: the iov pointer of user space.
 *
 * Obtaining the Key PID Information List in the Bearer UID
 *
 * Return: None
 */
static void bst_fg_save_tid_info(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct bst_fg_app_info_stru *pst_app_ins = NULL;
	int32_t *pl_tids = NULL;
	struct bst_fg_tid_comm_stru st_tid_comm = {0};
	int32_t l_tot_len = 0;

	/* Get tid message structure from usr space */
	if (copy_from_user(&st_tid_comm, argp, sizeof(st_tid_comm))) {
		bastet_loge("%s", __func__, " copy_from_user stTidComm error");
		return;
	}
	if (!bst_fg_is_app_idx_valid(st_tid_comm.us_index))
		return;
	if (((uint32_t)st_tid_comm.us_totle > BST_FG_MAX_PID_NUMBER) ||
		(st_tid_comm.us_totle == 0)) { /*lint !e571*/
		return;
	}

	l_tot_len = st_tid_comm.us_totle * sizeof(int32_t);
	pl_tids = kmalloc(l_tot_len, GFP_ATOMIC);
	if (pl_tids == NULL)
		return;

	/* Get tid list from usr space */
	if (copy_from_user(pl_tids, (uint8_t *)argp
		+ sizeof(st_tid_comm), l_tot_len)) {
		bastet_loge("%s", __func__, " copy_from_user plTids error");
		kfree(pl_tids);
		return;
	}
	pst_app_ins = bst_fg_get_app_ins(st_tid_comm.us_index);
	spin_lock_bh(&pst_app_ins->st_locker);

	if (!bst_fg_is_uid_valid(pst_app_ins->l_uid)) {
		spin_unlock_bh(&pst_app_ins->st_locker);
		kfree(pl_tids);
		return;
	}
	switch (st_tid_comm.us_index) {
	case BST_FG_IDX_WECHAT:
		/* If wechat, find the push socket according the pid message */
		bst_fg_set_hongbao_push_sock(st_tid_comm.us_totle, pl_tids);
		break;

	default:
		break;
	}
	kfree(pl_tids);
	spin_unlock_bh(&pst_app_ins->st_locker);
}

/**
 * bst_fg_save_dscp_info() - save dscp info.
 * @arg: the iov pointer of user space.
 *
 * Obtains the current dscp value.
 *
 * Return: None
 */
extern int g_FastGrabDscp;
static void bst_fg_save_dscp_info(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	int l_dscp = 0;

	/* Get dscp message from usr space */
	if (copy_from_user(&l_dscp, argp, sizeof(l_dscp))) {
		bastet_loge("%s", __func__, " copy_from_user for dscp error");
		return;
	}

	g_FastGrabDscp = l_dscp;
}

/**
 * bst_fg_update_acc() - update acc.
 * @uid: uid of the application
 *
 * Upper-layer notification acceleration
 *
 * Return: None
 */
static void bst_fg_update_acc(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	uid_t l_uid = 0;
	struct bst_fg_app_info_stru *pst_app_ins = NULL;

	/* Get dscp message from usr space */
	if (copy_from_user(&l_uid, argp, sizeof(l_uid))) {
		bastet_loge("BST_FG_Add_AccUId copy_from_user for uid error\n");
		return;
	}

	g_currnt_acc_uid = l_uid;

	pst_app_ins = bst_fg_get_app_ins(BST_FG_IDX_WECHAT);
	if ((l_uid == pst_app_ins->l_uid) &&
		(g_st_bastet_fg_hongbao.uc_duration_time != 0)) {
		g_st_bastet_fg_hongbao.uc_congestion_proc_flag = true;
		g_st_bastet_fg_hongbao.ul_start_time = jiffies;
		bastet_logi(
			"BST_FG_CUSTOM:hongbao congestion start,duration time is %d",
			g_st_bastet_fg_hongbao.uc_duration_time);
	}
}


/**
 * bst_fg_check_acc_uid() - check acc uid.
 * @lUid: uid of the application
 *
 * Check whether the uplink data packets need to be accelerated.
 *
 * Return: None
 */
static bool bst_fg_check_acc_uid(uid_t l_uid)
{
	if (!bst_fg_is_uid_valid(l_uid))
		return false;

	if (g_currnt_acc_uid == l_uid)
		return true;
	return false;
}

#ifndef CONFIG_HUAWEI_XENGINE
/**
 * bst_fg_hook_ul_stub() - hook the uplink data packets.
 * @pstSock: the object of socket
 * @msg: struct msghdr, the struct of message that was sent.
 *
 * if the socket has matched keyword, or uid equals current acc uid
 * hook the uplink data packets.
 *
 * Return: None
 */
void bst_fg_hook_ul_stub(struct sock *pst_sock, struct msghdr *msg)
{
	uid_t l_sock_uid;
	bool b_found = false;

	if ((pst_sock == NULL) || (msg == NULL))
		return;

	/*
	 * if the socket has matched keyword,
	 * keep acc always until socket destruct
	 */
	if (pst_sock->fg_Spec == BST_FG_WECHAT_HONGBAO) {
		bst_fg_set_acc_state(pst_sock, BST_FG_ACC_HIGH);
		return;
	}

	/* if matched keyword, accelerate socket */
	if (pst_sock->fg_Spec != BST_FG_NO_SPEC) {
		if (bst_fg_hook_packet(pst_sock, (uint8_t *)msg,
			(uint32_t)(msg->msg_iter.iov->iov_len),
				BST_FG_ROLE_SNDER)) {
			bst_fg_set_acc_state(pst_sock, BST_FG_ACC_HIGH);
			return;
		}
	}

	/* if uid equals current acc uid, accelerate it,else stop it */
	l_sock_uid = sock_i_uid(pst_sock).val;
	b_found = bst_fg_check_acc_uid(l_sock_uid);
	if (b_found)
		bst_fg_set_acc_state(pst_sock, BST_FG_ACC_HIGH);
	else
		bst_fg_set_acc_state(pst_sock, BST_FG_ACC_NORMAL);
}
#else
/**
 * bst_fg_hook_ul_stub() - hook the uplink data packets.
 * @*pstSock: the object of socket
 * @*msg: struct msghdr, the struct of message that was sent.
 *
 * if socket has matched keyword and if uid equals current acc uid
 * hook the uplink data packets.
 *
 * Return: true - hook the ul stub
 *         false - not matched.
 */
bool bst_fg_hook_ul_stub(struct sock *pst_sock, struct msghdr *msg)
{
	uid_t l_sock_uid;
	bool b_found = false;

	if ((pst_sock == NULL) || (msg == NULL))
		return false;

	/*
	 * if the socket has matched keyword, keep acc always
	 * until socket destruct
	 */
	if (pst_sock->fg_Spec == BST_FG_WECHAT_HONGBAO)
		return true;

	/* if matched keyword, accelerate socket */
	if (pst_sock->fg_Spec != BST_FG_NO_SPEC) {
		if (bst_fg_hook_packet(pst_sock,
			(uint8_t *)msg,
			(uint32_t)(msg->msg_iter.iov->iov_len),
			BST_FG_ROLE_SNDER))
			return true;
	}

	/* if uid equals current acc uid, accelerate it,else stop it */
	l_sock_uid = sock_i_uid(pst_sock).val;
	b_found = bst_fg_check_acc_uid(l_sock_uid);
	return b_found;
}
#endif

/**
 * bst_fg_save_custom_info() - save custom info.
 * @arg:  the iov pointer of user space.
 *
 * Saves the information of application customization.
 *
 * Return: None
 */
static void bst_fg_save_custom_info(unsigned long arg)
{
	uint8_t idx;
	void __user *argp = (void __user *)arg;
	struct bst_fg_custom_stru *p_bst_fg_custom_stru = NULL;
	/* Get policy message from usr space */
	if (copy_from_user(&g_st_bastet_fg_custom, argp,
		sizeof(*p_bst_fg_custom_stru))) {
		bastet_loge("BST_FG_CUSTOM:save custom info error");
		return;
	}

	g_st_bastet_fg_custom.uc_app_num =
		bst_min(g_st_bastet_fg_custom.uc_app_num,
			BST_FG_MAX_CUSTOM_APP);

	g_st_bastet_fg_hongbao.uc_duration_time =
		g_st_bastet_fg_custom.uc_hongbao_duration_time;

	for (idx = 0; idx < g_st_bastet_fg_custom.uc_app_num; idx++) {
		bastet_logi("BST_FG_CUSTOM:Save Custom Info UID is %d",
			g_st_bastet_fg_custom.ast_custom_info[idx].l_uid);
		bastet_logi("discard_timer is %d",
			g_st_bastet_fg_custom
				.ast_custom_info[idx].uc_discard_timer);
		bastet_logi("app_type is %d,",
			g_st_bastet_fg_custom
				.ast_custom_info[idx].uc_app_type);
		bastet_logi("ProtocolBitMap is %d",
			g_st_bastet_fg_custom
				.ast_custom_info[idx].uc_protocol_bitmap);
		bastet_logi("RetranDiscardFlag is %d",
			g_st_bastet_fg_custom
				.ast_custom_info[idx].uc_tcp_retran_discard);
	}
}

/**
 * bst_fg_encode_discard_timer() - To Code the discard_timer field
 * @usTimer: the time value(input parameter)
 *
 * code the timer by different area.
 *
 * Return: None
 */
uint8_t bst_fg_encode_discard_timer(unsigned long ul_timer)
{
	uint8_t discard_timer = 0;

	if ((ul_timer >= 0) && (ul_timer <= 630))
		discard_timer = 0x00 | ((ul_timer + 10 - 1) / 10);
	else if ((ul_timer > 630) && (ul_timer <= 3160))
		discard_timer = 0x40 | (((ul_timer - 640) + 40 - 1) / 40);
	else if ((ul_timer > 3160) && (ul_timer <= 9500))
		discard_timer = 0x80 | (((ul_timer - 3200) + 100 - 1) / 100);
	else if ((ul_timer > 9500) && (ul_timer <= 34800))
		discard_timer = 0xC0 | (((ul_timer - 9600) + 400 - 1) / 400);
	else
		discard_timer = 0;

	return discard_timer;
}

/**
 * bst_fg_hongbao_process() - process hongbao in fastgrab scenario.
 * @*pstSock: the object of socket
 * @lSockUid: the Uid of Sock
 * @ucTcpOrUdpBitMa: bitmap of TCP or UDP
 *
 * The uplink congestion scheduling flag is added to the data gift(hongbao).
 *
 * Return: None
 */
void bst_fg_hongbao_process(struct sock *pst_sock,
	uid_t l_sock_uid,
	uint8_t uc_protocol_bitmap)
{
	struct bst_fg_app_info_stru *pst_app_ins = NULL;

	if (uc_protocol_bitmap != BST_FG_TCP_BITMAP)
		return;

	pst_app_ins = bst_fg_get_app_ins(BST_FG_IDX_WECHAT);
	if ((g_st_bastet_fg_hongbao.uc_congestion_proc_flag) &&
		(l_sock_uid == pst_app_ins->l_uid)) {
		if (before((g_st_bastet_fg_hongbao.ul_start_time +
			g_st_bastet_fg_hongbao.uc_duration_time*HZ),
			jiffies)) {
			g_st_bastet_fg_hongbao.uc_congestion_proc_flag = false;

			bastet_logi(
				"BST_FG_CUSTOM:hongbao congestion stop,HZ is %u",
				HZ);
			return;
		}
#ifdef CONFIG_HW_DPIMARK_MODULE
		bst_fg_set_app_type(pst_sock,
			(BST_FG_ACC_BITMAP | BST_FG_CONGESTION_BITMAP));
#endif
	}
}

#ifdef CONFIG_HUAWEI_BASTET
static bool detect_sock_state(struct sock *pst_sock)
{
	if ((!sk_fullsock(pst_sock)) || sock_flag(pst_sock, SOCK_DEAD) ||
		(pst_sock->sk_state == TCP_TIME_WAIT)) {
		bastet_logd("%s", __func__, " fail,wrong sk_state");
		return false;
	}
	return true;
}

static void print_bst_fg_custom_info(uid_t l_sock_uid,
	uint8_t uc_discard_timer,
	uint8_t uc_protocol_bitmap,
	struct bst_fg_custom_info *past_custom_info)
{
	bastet_logd("BST_FG_CUSTOM:Find sock UID is %d", l_sock_uid);
	bastet_logd("discard_timer is %d", uc_discard_timer);
	bastet_logd("app_type is %d", past_custom_info->uc_app_type);
	bastet_logd("ProtocolBitMap is %d", uc_protocol_bitmap);
	bastet_logd("RetranDiscardFlag is %d",
		past_custom_info->uc_tcp_retran_discard);
}

static void handle_discard_timer(struct bst_fg_custom_info *past_custom_info,
	uint8_t *uc_discard_timer, struct sock *pst_sock,
	struct inet_connection_sock *p_inet_sock)
{
	unsigned long timeout;

	if (p_inet_sock->icsk_timeout > p_inet_sock->icsk_timeout)
		timeout = p_inet_sock->icsk_timeout - jiffies;
	else
		timeout = 0;
	bastet_logd("BST_FG_CUSTOM:TCP Timeout is 0x%lx", timeout);
	if ((0 != timeout) && (0 != HZ)) {
		timeout = ((timeout * 1000) / HZ) +
			(20 *
			(past_custom_info->uc_tcp_retran_discard & 0x7F));
		*uc_discard_timer = bst_fg_encode_discard_timer(timeout);
		bst_fg_set_discard_timer(pst_sock, *uc_discard_timer);
	}
}

/**
 * bst_fg_custom_process() - Application Customization
 * @*pstSock: the object of socket
 * @*msg: struct msghdr, the struct of message that was sent.
 * @ucTcpOrUdpBitMa: bitmap of TCP or UDP.
 *
 * start to process the custom request.
 *
 * Return: None
 */
void bst_fg_custom_process(struct sock *pst_sock,
	struct msghdr *msg,
	uint8_t uc_protocol_bitmap)
{
	uid_t l_sock_uid;
	uint8_t idx;
	uint8_t uc_app_num;
	uint8_t uc_discard_timer;
	struct bst_fg_custom_info *past_custom_info = NULL;
	struct inet_connection_sock *p_inet_sock = NULL;

	if (!detect_sock_state(pst_sock))
		return;
	l_sock_uid = sock_i_uid(pst_sock).val;
	bst_fg_set_discard_timer(pst_sock, 0);
#ifdef CONFIG_HW_DPIMARK_MODULE
	bst_fg_init_app_type(pst_sock);
#endif

	if (!bst_fg_is_uid_valid(l_sock_uid))
		return;

	bst_fg_hongbao_process(pst_sock, l_sock_uid, uc_protocol_bitmap);

	uc_app_num = bst_min(g_st_bastet_fg_custom.uc_app_num,
		BST_FG_MAX_CUSTOM_APP);
	for (idx = 0; idx < uc_app_num; idx++) {
		if (l_sock_uid ==
			g_st_bastet_fg_custom.ast_custom_info[idx].l_uid) {
			past_custom_info =
				&(g_st_bastet_fg_custom.ast_custom_info[idx]);
			break;
		}
	}

	if (past_custom_info == NULL)
		return;

	if (!(uc_protocol_bitmap & past_custom_info->uc_protocol_bitmap))
		return;

	if ((past_custom_info->uc_tcp_retran_discard & 0x80) &&
		(uc_protocol_bitmap == BST_FG_TCP_BITMAP)) {
		p_inet_sock = inet_csk(pst_sock);
		handle_discard_timer(past_custom_info, &uc_discard_timer,
			pst_sock, p_inet_sock);
	} else {
		uc_discard_timer = past_custom_info->uc_discard_timer;
		bst_fg_set_discard_timer(pst_sock, uc_discard_timer);
	}
#ifdef CONFIG_HW_DPIMARK_MODULE
	bst_fg_set_app_type(pst_sock, past_custom_info->uc_app_type);
#endif
#ifdef CONFIG_HW_DPIMARK_MODULE
	bastet_logd("BST_FG_CUSTOM:DPI_MARK is 0x%x",
		pst_sock->__sk_common.skc_hwdpi_mark);
#endif
	print_bst_fg_custom_info(l_sock_uid, uc_discard_timer,
		uc_protocol_bitmap, past_custom_info);
}
#endif

/**
 * bst_fg_io_ctrl() - ioctrl of fastgrab.
 * @arg: iov pointer of User space
 *
 * External IoCtrl interfaces of the Fast-grab
 *
 * Return: None
 */
void bst_fg_io_ctrl(unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct fastgrab_info st_io_cmd;

	if (argp == NULL)
		return;

	/* Copy io ctrl message of fast-grab from usr space. */
	if (copy_from_user(&st_io_cmd, argp, sizeof(st_io_cmd))) {
		bastet_loge("%s", __func__, " copy_from_user error");
		return;
	}
	bastet_logi("%s", __func__, " command=%d, msg_len=%d",
		st_io_cmd.cmd, st_io_cmd.len);

	/* Call branched function according to the fast-grab command. */
	switch (st_io_cmd.cmd) {
	case CMD_LOAD_KEYWORDS:
		bst_fg_save_keyword(arg + sizeof(st_io_cmd));
		break;
	case CMD_UPDATE_UID:
		bst_fg_save_uid_info(arg + sizeof(st_io_cmd));
		break;
	case CMD_UPDATE_TID:
		bst_fg_save_tid_info(arg + sizeof(st_io_cmd));
		break;
	case CMD_UPDATE_DSCP:
		bst_fg_save_dscp_info(arg + sizeof(st_io_cmd));
		break;
	case CMD_UPDATE_ACC_UID:
		bst_fg_update_acc(arg + sizeof(st_io_cmd));
		break;
	case CMD_UPDATE_CUSTOM:
		bst_fg_save_custom_info(arg + sizeof(st_io_cmd));
		break;
	default:
		break;
	}
}

/**
 * bst_fg_check_sock_uid() - check sock uid.
 * @pstSock: sock struct
 * @state: the state of socket
 *
 * Processes the socket status change information related to uid.
 *
 * Return: None.
 */
void bst_fg_check_sock_uid(struct sock *pst_sock, int state)
{
	uid_t l_uid = BST_FG_INVALID_UID;
	uint16_t us_idx = BST_FG_IDX_INVALID_APP;

	if (IS_ERR_OR_NULL(pst_sock)) {
		bastet_loge("invalid parameter");
		return;
	}

	/* Judge the state if it's should be cared. */
	if (!bst_fg_is_cared_sk_state(state))
		return;

	/* Reset the sock fg special flag to inited state. */
	bst_fg_init_sock_spec(pst_sock);
	l_uid = sock_i_uid(pst_sock).val;
	if (!bst_fg_is_uid_valid(l_uid))
		return;

	/* Find the application name(index) according to uid. */
	l_uid = sock_i_uid(pst_sock).val;
	for (us_idx = 0; us_idx < BST_FG_MAX_APP_NUMBER; us_idx++) {
		if (l_uid == g_fast_grab_app_info[us_idx].l_uid) {
			bastet_logi("%s", __func__,
				"Cared Uid Found, uid=%u", l_uid);
			post_indicate_packet(BST_IND_FG_UID_SOCK_CHG,
				&l_uid, sizeof(uid_t));
			break;
		}
	}
	if (!bst_fg_is_app_idx_valid(us_idx))
		return;

	bastet_logi("%s", __func__, " AppIdx=%d ", us_idx);
	/* Call branched function of different application. */
	switch (us_idx) {
	case BST_FG_IDX_WECHAT:
		bst_fg_proc_wx_sock(pst_sock, state);
		break;

	default:
		break;
	}
}
