

#ifndef _MPTCP_HEARTBEAT_H_
#define _MPTCP_HEARTBEAT_H_

/* include headers */
#include <linux/in.h>
#include <linux/net.h>

/* macro */
#define LINK_NUM 2 // the total links availiable, currently 2, wifi and cellular
#define IF_NAME_LIST {"wlan0", "rmnet0"}

#define MAX_PAYLOAD_SIZE 128
#define MAX_RETRY_COUNT 10 // a maximum retry count is necessary

/* callback definition */
typedef void (*heartbeat_detection_callback)(int type, int result);

/* struct definition */
typedef struct _detection_para {
	int retry_count;
	int timeout;    // for each retry
	char* payload;
	int payload_len;    // do NOT exceed MAX_PAYLOAD_SIZE
	heartbeat_detection_callback cb_fun;
} detection_para;

typedef enum _link_status {
	LINK_OK,
	LINK_BROKEN,
} link_status;

/* argment of function "heartbeat_trigger" may be a combination of the below */
/* eg. 3 means WIFI & LTE */
typedef enum _detection_type {
	WIFI_DETECTION = (1 << 0),
	LTE_DETECTION = (1 << 1),
} detection_type;

/* function declaration */
void heartbeat_init(struct sockaddr_in* link_addrs);
void heartbeat_deinit(void);
void heartbeat_trigger(unsigned int type, detection_para* para);

#endif /* _MPTCP_HEARTBEAT_H_ */
