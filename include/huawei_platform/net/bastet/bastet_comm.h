/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014. All rights reserved.
 * Description: Provide kernel device information for bastet.
 *
 * Author: pengyu@huawei.com
 * Create: 2014-06-21
 */

#ifndef _BASTET_COMM_H
#define _BASTET_COMM_H

#include "bastet_dev.h"

#ifdef CONFIG_HW_CROSSLAYER_OPT
#include <net/tcp_crosslayer.h>
#endif


#define BST_MODEM_IOC_MAGIC					'j'

#define BST_MODEM_IOC_GET_MODEM_RAB_ID	_IOWR(BST_MODEM_IOC_MAGIC, 1, \
	struct bastet_modem_rab_id)
#define BST_MODEM_IOC_GET_MODEM_RESET	_IOWR(BST_MODEM_IOC_MAGIC, 2, int32_t)
#define BST_MODEM_IOC_GET_IPV6_MODEM_RAB_ID	_IOWR(BST_MODEM_IOC_MAGIC, 3, \
	struct bastet_modem_rab_id)

#ifdef CONFIG_HW_CROSSLAYER_OPT
#define BST_ASPEN_INFO_PKT_DROP 0xc0000001
#define BST_ASPEN_PKT_DROP_SIZE 128

/**
 * struct bst_aspen_pkt_drop - bastet aspen packet.
 * @ul_aspen_info_type: aspen info type.
 * @ul_pkt_num: aspen packet number.
 * @st_pkt[BST_ASPEN_PKT_DROP_SIZE]: aspen packet drop size.
 *
 * used to record the bastet aspen droped packet information.
 */
struct bst_aspen_pkt_drop {
	uint32_t ul_aspen_info_type;
	uint32_t ul_pkt_num;
	struct aspen_cdn_info st_pkt[BST_ASPEN_PKT_DROP_SIZE];
};
#endif

/**
 * struct bastet_modem_rab_id - bastet modem releated info.
 * @modem_id: modem id
 * @ul_pkt_num: aspen packet number.
 * @rab_id: rab id.
 *
 * used to record the bastet modem releated info such as modem id and rab id.
 */
struct bastet_modem_rab_id {
	uint16_t modem_id;
	uint16_t rab_id;
};

enum BST_ACORE_CORE_MSG_TYPE_ENUM {
	BST_ACORE_CORE_MSG_TYPE_DSPP,
	BST_ACORE_CORE_MSG_TYPE_ASPEN,
	BST_ACORE_CORE_MSG_TYPE_EMCOM_SUPPORT,
	BST_ACORE_CORE_MSG_TYPE_EMCOM_KEY_PSINFO,
	BST_ACORE_CORE_MSG_TYPE_RESET_INFO,
	BST_ACORE_CORE_MSG_TYPE_BUTT
};

typedef uint32_t BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32;

/**
 * struct bst_common_msg - bastet message.
 * @en_msg_type:message type.
 * @auc_value[0]:message content.
 *
 * used to record the bastet message information.
 */
struct bst_common_msg {
	BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32 en_msg_type;
	uint8_t                             auc_value[0];
};

/**
 * struct bst_acom_msg - bastet acom message.
 * @en_msg_type:message type of acom
 * @ul_len:message length.
 * @auc_value[4]:message content.
 *
 * used to record the bastet acom message information.
 */
struct bst_acom_msg {
	BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32 en_msg_type;
	uint32_t                            ul_len;
	uint8_t                             auc_value[4];
};

/**
 * struct bst_emcom_support_msg - bastet support message.
 * @en_msg_type:message type of acom
 * @en_state:to identify the message states.
 * @auc_reverse[3]:reserver 3 uint8_t.
 *
 * used to record the bastet support message information.
 */
struct bst_emcom_support_msg {
	BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32 en_msg_type;
	uint8_t                             en_state;
	uint8_t                             auc_reverse[3];
};

/**
 * struct bst_key_psinfo - bastet key info.
 * @en_msg_type:message type of acom
 * @en_state:to identify the key states.
 *
 * used to record the bastet key information.
 */
struct bst_key_psinfo {
	BST_ACORE_CORE_MSG_TYPE_ENUM_UINT32 en_msg_type;
	uint32_t                            en_state;
};

#ifdef CONFIG_HUAWEI_BASTET_COMM
union bst_rab_id_ioc_arg {
	char in[IFNAMSIZ];
	struct bst_modem_rab_id out;
};
#endif

void reg_ccore_reset_notify(void);
void unreg_ccore_reset_notify(void);

#endif /* _BASTET_COMM_H */
