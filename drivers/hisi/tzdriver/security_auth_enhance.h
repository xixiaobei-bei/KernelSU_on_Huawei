

#ifndef _SECURITY_AUTH_ENHANCE_H_
#define _SECURITY_AUTH_ENHANCE_H_

#include "teek_ns_client.h"

#define INC	0x01
#define DEC	0x00
#define UN_SYNCED	0x55
#define IS_SYNCED	0xaa

int32_t update_timestamp(tc_ns_smc_cmd *cmd);
int32_t update_chksum(tc_ns_smc_cmd *cmd);
int32_t verify_chksum(tc_ns_smc_cmd *cmd);
int32_t sync_timestamp(tc_ns_smc_cmd *cmd, uint8_t *token, unsigned char *uuid, bool global);

#endif
