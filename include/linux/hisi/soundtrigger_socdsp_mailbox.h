#ifndef __SOUNDTRIGGER_SOCDSP_MAILBOX_H
#define __SOUNDTRIGGER_SOCDSP_MAILBOX_H

enum IRQ_RT
{
	/* IRQ Not Handled as Other problem */
	IRQ_NH_OTHERS    = -5,
	/* IRQ Not Handled as Mailbox problem */
	IRQ_NH_MB     = -4,
	/* IRQ Not Handled as pcm MODE problem */
	IRQ_NH_MODE     = -3,
	/* IRQ Not Handled as TYPE problem */
	IRQ_NH_TYPE     = -2,
	/* IRQ Not Handled */
	IRQ_NH          = -1,
	/* IRQ HanDleD */
	IRQ_HDD         = 0,
	/* IRQ HanDleD related to PoinTeR */
	IRQ_HDD_PTR     = 1,
	/* IRQ HanDleD related to STATUS */
	IRQ_HDD_STATUS,
	/* IRQ HanDleD related to SIZE */
	IRQ_HDD_SIZE,
	/* IRQ HanDleD related to PoinTeR of Substream */
	IRQ_HDD_PTRS,
	/* IRQ HanDleD Error */
	IRQ_HDD_ERROR,
};
typedef enum IRQ_RT irq_rt_t;

typedef irq_rt_t (*irq_hdl_t)(void *, unsigned int);

enum WAKEUP_CHN_MSG_TYPE {
	WAKEUP_CHN_MSG_START = 0xFD01,
	WAKEUP_CHN_MSG_START_ACK = 0xFD08, //Todo:
	WAKEUP_CHN_MSG_STOP = 0xFD02,
	WAKEUP_CHN_MSG_STOP_ACK = 0xFD09,  //Todo:
	WAKEUP_CHN_MSG_PARAMETER_SET = 0xFD03,
	WAKEUP_CHN_MSG_PARAMETER_SET_ACK = 0xFD0A,  //Todo:
	WAKEUP_CHN_MSG_PARAMETER_GET = 0xFD04,
	WAKEUP_CHN_MSG_PARAMETER_GET_ACK = 0xFD0B,  //Todo:
	WAKEUP_CHN_MSG_HOTWORD_DETECT_RCV = 0xFD05,
	WAKEUP_CHN_MSG_ELAPSED_RCV = 0xFD06,
};

enum LP_WAKEUP_CHN_MSG_TYPE {
	/* AP <--> HIFI */
	LP_WAKEUP_CHN_MSG_START = 0xFD11,
	LP_WAKEUP_CHN_MSG_STOP = 0xFD12,
	LP_WAKEUP_CHN_MSG_PARAMETER_SET = 0xFD13,
	LP_WAKEUP_CHN_MSG_PARAMETER_GET = 0xFD14,

	/* HIFI <--> AP */
	LP_WAKEUP_CHN_MSG_HOTWORD_DETECT_RCV = 0xFD15,
	LP_WAKEUP_CHN_MSG_ELAPSED_RCV = 0xFD16,
	LP_WAKEUP_CHN_MSG_START_ACK = 0xFD17,
	LP_WAKEUP_CHN_MSG_STOP_ACK = 0xFD18,
	LP_WAKEUP_CHN_MSG_PARAMETER_SET_ACK = 0xFD19,
	LP_WAKEUP_CHN_MSG_PARAMETER_GET_ACK = 0xFD1A,
};

struct parameter_set {
	int key;
	union {
		int value;
		struct {
			unsigned int index;
			unsigned int length;
			char piece[0];
		} model;
	};
};

struct soundtrigger_sync_msg {
	unsigned short msg_type;
	unsigned short reserved;
	int module_id;
	struct parameter_set set_param;
};

#define WAKEUP_CHN_COMMON   \
	unsigned short msg_type;   \
	unsigned short reserved;

struct parameter_set_msg {
	WAKEUP_CHN_COMMON
	unsigned int module_id;
	struct parameter_set para;
};

struct parameter_get_msg {
	WAKEUP_CHN_COMMON
};

struct wakeup_start_msg {
	WAKEUP_CHN_COMMON
	unsigned int module_id;
};

struct wakeup_stop_msg {
	WAKEUP_CHN_COMMON
	unsigned int module_id;
};

struct wakeup_period_elapsed {
	unsigned int seq;
	unsigned int start;
	unsigned int len;
};

struct soundtrigger_rcv_msg {
	WAKEUP_CHN_COMMON
	union {
		int socdsp_handle;
		unsigned int fast_len;
		struct wakeup_period_elapsed elapsed;
	};
	char *elapsed_buffer;
};

struct msg_node {
	struct completion *msg_complete;
	unsigned short msg_type;
	unsigned short recv_msg_type;
	void (*recv_msg_proc)(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete);
	char msg_name[40];
};

int start_recognition_msg(unsigned short msg_type, int module_id, bool is_lp);
int stop_recognition_msg(unsigned short msg_type, int module_id, bool is_lp);
int get_handle_msg(unsigned short msg_type, int *socdsp_handle);
int parameter_set_msg(unsigned short msg_type, int module_id, struct parameter_set *set_val);
int soundtrigger_mailbox_init(void);
void soundtrigger_mailbox_deinit(void);
#endif

