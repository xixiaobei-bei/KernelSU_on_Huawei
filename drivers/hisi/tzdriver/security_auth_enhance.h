

#ifndef _SECURITY_AUTH_ENHANCE_H_
#define _SECURITY_AUTH_ENHANCE_H_
#include <linux/types.h>
#include "teek_ns_client.h"

#define INC     0x01
#define DEC     0x00
#define UN_SYNCED     0x55
#define IS_SYNCED     0xaa


TEEC_Result update_timestamp(const tc_ns_smc_cmd *cmd);
TEEC_Result update_chksum(tc_ns_smc_cmd *cmd);
TEEC_Result verify_chksum(const tc_ns_smc_cmd *cmd);
TEEC_Result sync_timestamp(const tc_ns_smc_cmd *cmd, uint8_t *token,
	uint32_t token_len, bool global);
#endif
