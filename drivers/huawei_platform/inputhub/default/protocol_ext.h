

#ifndef __PROTOCOL_EXT_H
#define __PROTOCOL_EXT_H

#ifdef CONFIG_INPUTHUB_30
#include <linux/hisi/contexthub/itf/ipc_ap_iomcu.h>
#endif

#define MAX_THERMO_CALIBRATE_DATA_LENGTH     30

#ifndef CONFIG_INPUTHUB_30
struct pkt_header {
	uint8_t tag;
	uint8_t cmd;
	/* value CMD_RESP means need resp, CMD_NO_RESP means need not resp */
	uint8_t resp;
	uint8_t partial_order;
	uint16_t tranid;
	uint16_t length;
} ;
#endif

typedef struct {
	struct pkt_header hd;
	uint32_t subcmd;
	int32_t return_data[MAX_THERMO_CALIBRATE_DATA_LENGTH];
} pkt_thermometer_data_req_t;

#endif /* end of include guard: __PROOCOL_EXT_H__ */