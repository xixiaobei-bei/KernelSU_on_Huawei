#ifndef __UAPI_LINUX_HISI_MDFX_H
#define __UAPI_LINUX_HISI_MDFX_H

#include <linux/types.h>

#ifdef _BIT_
#undef _BIT_
#endif

#define _BIT_(x)   (1 << (x))
#define BIT64(x)   (1ul << (x))

#define ENABLE_BITS_FIELD(value, bitsfield) ((value & bitsfield) == bitsfield)
#define DISABLE_BITS_FIELD(value, bitsfield) ((value) &= ~(bitsfield))

#define ENABLE_BIT(value, bit)   ((value) & _BIT_(bit))
#define ENABLE_BIT64(value, bit) ((value) & BIT64(bit))
#define DISABLE_BIT64(value, bit) ((value) &= ~(BIT64(bit)))

#define HISI_MDFX_IOCTL_MAGIC 'D'
/*
 * user mode service query the log files specify,
 * such as the max buf size, the max log files nums
 */
#define MDFX_QUERY_FILE_SPEC _IOW(HISI_MDFX_IOCTL_MAGIC, 0x01, struct mdfx_file_spec)
#define MDFX_DMD_REPORT      _IOW(HISI_MDFX_IOCTL_MAGIC, 0x05, unsigned int)

#define MDFX_DELIVER_EVENT   _IOW(HISI_MDFX_IOCTL_MAGIC, 0x09, struct mdfx_event_desc)
#define MDFX_DUMP_INFO       _IOW(HISI_MDFX_IOCTL_MAGIC, 0x0a, struct mdfx_dump_desc)
#define MDFX_TRACE_INFO    _IOW(HISI_MDFX_IOCTL_MAGIC, 0x0b, struct mdfx_tracing_desc)
#define MDFX_ADD_VISITOR  _IOW(HISI_MDFX_IOCTL_MAGIC, 0x0c, struct mdfx_visitor_desc)
#define MDFX_REMOVE_VISITOR _IOW(HISI_MDFX_IOCTL_MAGIC, 0x0d, int64_t)
#define MDFX_GET_CAPABILITY _IOW(HISI_MDFX_IOCTL_MAGIC, 0x0e, uint32_t)

/* visitor type */
#define VISITOR_GRAPHIC_DISPLAY     BIT64(0)
#define VISITOR_GRAPHIC_MEDIACOMM   BIT64(1)
#define VISITOR_GRAPHIC_GRALLOC     BIT64(2)
#define VISITOR_GRAPHIC_DRM         BIT64(3)
#define VISITOR_DISPLAY_MASK        0xFFFFul
#define VISITOR_GRAPHIC_MALI_DDK    BIT64(16)
#define VISITOR_GRAPHIC_MALI_KMS    BIT64(17)
#define VISITOR_GRAPHIC_MASK         0xFFFFFFFFul

/* saver detail */
#define SAVER_DETAIL_IMAGE  BIT64(0)
#define SAVER_DETAIL_STRING BIT64(1)

/* logger detail */
#define LOGGER_PRINT_USER_LOG BIT64(0)
#define LOGGER_PRINT_DRIVER_LOG BIT64(1)

#define MDFX_EVENT_NAME_MAX  64

/**
 * dump platform freq and voltage,
 * include ddr freq, ip's freq and voltage
 */
#define DUMP_FREQ_AND_VOLTAGE  BIT64(0)
#define DUMP_CPU_RUNNABLE      BIT64(1) /** dump runnable cpu backtrace */
#define DUMP_CMDLIST           BIT64(2) /** dump ip's cmdlist */
#define DUMP_CLIENT_CALLSTACK  BIT64(3) /** dump callstack for client */
#define DUMP_ACTION_MASK       0xffffffful

#define DUMP_IP_DISPLAY        BIT64(48)
#define DUMP_IP_MEDIACOMM      BIT64(49)
#define DUMP_IP_GPU            BIT64(50)
/** reserved for other IPs */
#define DUMP_IP_MASK           0xfffffff0000000ul

/** trace vote frequency flow */
enum tracing_type {
    TRACING_VOTE = 0,
    TRACING_HOTPLUG,     /** trace dp hotplug flow */
    TRACING_DISPLAY_FLOW,   /** trace present display flow */
    TRACING_TYPE_MAX
};

#define TRACING_TYPE_VOTE          BIT64(TRACING_VOTE)
#define TRACING_TYPE_HOTPLUG       BIT64(TRACING_HOTPLUG)
#define TRACING_TYPE_DISPLAY_FLOW  BIT64(TRACING_DISPLAY_FLOW)
#define TRACING_TYPE_ALL_MASK      0xffffffffffffffff

/* this define must be equal to enum::EventActor */
enum mdfx_actor_type {
	ACTOR_DUMPER = 0,
	ACTOR_TRACING,
	ACTOR_SAVER,
	ACTOR_LOGGER,
	ACTOR_MAX,
};

enum mdfx_capability {
	MDFX_CAP_NULL = 0,
	MDFX_CAP_LOGGER = 1 << 0,
	MDFX_CAP_DUMPER = 1 << 1,
	MDFX_CAP_SAVER =  1 << 2,
	MDFX_CAP_TRACING = 1 << 3,
	MDFX_CAP_EVENT = 1 << 4,
};

#define MDFX_HAS_CAPABILITY(caps, needCaps) (!!((caps) & (needCaps)))

/* action type */
#define ACTION_DUMP_INFO    BIT64(ACTOR_DUMPER)
#define ACTION_TRACE_RECODE BIT64(ACTOR_TRACING)
#define ACTION_SAVE_INFO    BIT64(ACTOR_SAVER)
#define ACTION_LOG_PRINT    BIT64(ACTOR_LOGGER)

struct mdfx_event_desc {
	int64_t id;
	uint64_t relevance_visitor_types;
	uint64_t actions;
	uint32_t detail_count;
	uint64_t *detail;
	char event_name[MDFX_EVENT_NAME_MAX];
};

struct mdfx_dump_desc {
	int64_t id;
	uint64_t dump_infos;
};

struct mdfx_tracing_desc {
	int64_t id;
	uint64_t tracing_type;
	uint32_t msg_len;
	char* msg;
};

/*
 * id is a num, which it was init by mdfx service at user mode,
 * if id is -1, mdfx driver will initial it.
 * type is a enmu value which can be initial by enum VisitorType
 * pid is client's pid
 */
struct mdfx_visitor_desc {
	int64_t id;
	uint64_t type;
	int64_t pid;
};

// file generate
struct mdfx_file_spec {
	uint32_t file_max_size;
	uint32_t file_max_num;
};

#endif /* __UAPI_LINUX_HISI_MDFX_H */
