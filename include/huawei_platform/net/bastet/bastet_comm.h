/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Provide kernel device information for bastet.
 * Author: pengyu@huawei.com
 * Create: 2014-06-21
 */

#ifndef _BASTET_COMM_H
#define _BASTET_COMM_H

#include "bastet_dev.h"

#define BST_MODEM_IOC_MAGIC 'j'

#define BST_MODEM_IOC_GET_MODEM_RAB_ID _IOWR(BST_MODEM_IOC_MAGIC, 1, \
	struct bastet_modem_rab_id)
#define BST_MODEM_IOC_GET_MODEM_RESET _IOWR(BST_MODEM_IOC_MAGIC, 2, int32_t)
#define BST_MODEM_IOC_GET_IPV6_MODEM_RAB_ID _IOWR(BST_MODEM_IOC_MAGIC, 3, \
	struct bastet_modem_rab_id)

#define BST_HEX_DUMP_LEN 16
#define BST_MSG_CONT_LEN 4
#define BST_REV_3 3

/*
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

enum bst_acore_core_msg_type_enum {
	BST_ACORE_CORE_MSG_TYPE_DSPP,
	BST_ACORE_CORE_MSG_TYPE_ASPEN,
	BST_ACORE_CORE_MSG_TYPE_EMCOM_SUPPORT,
	BST_ACORE_CORE_MSG_TYPE_EMCOM_KEY_PSINFO,
	BST_ACORE_CORE_MSG_TYPE_RESET_INFO,

	BST_ACORE_CORE_MSG_TYPE_BUTT
};

enum bastet_sleep_value_enum {
	BST_SLEEP_VAL_1000 = 1000,
	BST_SLEEP_VAL_5000 = 5000,

	BST_SLEEP_VAL_BUTT
};

/*
 * struct bst_common_msg - bastet message.
 * @en_msg_type:message type.
 * @auc_value[0]:message content.
 *
 * used to record the bastet message information.
 */
struct bst_common_msg {
	uint32_t en_msg_type;
	uint8_t auc_value[0];
};

/*
 * struct bst_acom_msg - bastet acom message.
 * @en_msg_type:message type of acom
 * @ul_len:message length.
 * @auc_value[BST_MSG_CONT_LEN]:message content.
 *
 * used to record the bastet acom message information.
 */
struct bst_acom_msg {
	uint32_t en_msg_type;
	uint32_t ul_len;
	uint8_t auc_value[BST_MSG_CONT_LEN];
};

/*
 * struct bst_emcom_support_msg - bastet support message.
 * @en_msg_type:message type of acom
 * @en_state:to identify the message states.
 * @auc_reverse[BST_REV_3]:reserver 3 uint8_t.
 *
 * used to record the bastet support message information.
 */
struct bst_emcom_support_msg {
	uint32_t en_msg_type;
	uint8_t en_state;
	uint8_t auc_reverse[BST_REV_3];
};

/*
 * struct bst_key_psinfo - bastet key info.
 * @en_msg_type:message type of acom
 * @en_state:to identify the key states.
 *
 * used to record the bastet key information.
 */
struct bst_key_psinfo {
	uint32_t en_msg_type;
	uint32_t en_state;
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
