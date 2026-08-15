

#ifndef _TEE_CLIENT_TYPE_H_
#define _TEE_CLIENT_TYPE_H_
#define SECURITY_AUTH_ENHANCE
#include "teek_client_list.h"
#include "teek_client_constants.h"
#define TOKEN_SAVE_LEN	24

/*
 * @ingroup teec_common_data
 * null值的定义
 */
#ifndef NULL
#define NULL 0
#endif

/*
 * @ingroup teec_common_data
 * 函数返回值类型定义
 *
 * 用于表示函数返回结果
 */
typedef uint32_t teec_result;

/*
 * @ingroup teec_common_data
 * uuid类型定义
 *
 * uuid类型遵循rfc4122 [2]，用于标识安全服务
 */
typedef struct {
	uint32_t time_low;
	/* < 时间戳的低4字节  */
	uint16_t time_mid;
	/* < 时间戳的中2字节  */
	uint16_t timehi_and_version;
	/* < 时间戳的高2字节与版本号的组合  */
	uint8_t clockseq_and_node[8];
	/* < 时钟序列与节点标识符的组合  */
} teec_uuid;

/*
 * @ingroup teec_common_data
 * teec_context结构体类型定义
 *
 * 描述客户端应用与安全世界之间建立的链接环境
 */
typedef struct {
	void *dev;
	uint8_t *ta_path;
	struct list_node session_list;
	/* < 会话链表  */
	struct list_node shrd_mem_list;
	/* < 共享内存链表  */
} teec_context;

/*
 * @ingroup teec_common_data
 * teec_session结构体类型定义
 * 描述客户端应用与安全世界之间建立的会话
 */
typedef struct {
	uint32_t session_id;
	/* < 会话id，由安全世界返回  */
	teec_uuid service_id;
	/* < 安全服务的uuid，每个安全服务拥有唯一的uuid  */
	uint32_t ops_cnt;
	/* < 在会话内的操作数  */
	struct list_node head;
	/* < 会话链表头  */
	teec_context *context;
	/* < 指向会话所属的tee环境  */
#ifdef SECURITY_AUTH_ENHANCE
	/* token_save_len_24byte = token_16byte + timestamp_8byte */
	uint8_t teec_token[TOKEN_SAVE_LEN];
#endif
} teec_session;

/*
 * @ingroup teec_common_data
 * teec_sharedmemory结构体类型定义
 * 描述一块共享内存，可以注册，也可以分配
 */
typedef struct {
	void *buffer;
	/* < 内存指针  */
	size_t size;
	/* < 内存大小  */
	uint32_t flags;
	/* < 内存标识符，用于区别输入或输出，取值范围为#teec_sharedmemctl  */
	uint32_t ops_cnt;
	/* < 内存操作数  */
	bool is_allocated;
	/* < 内存分配标示符，用于区别是注册的，还是分配的  */
	struct list_node head;
	/* < 共享内存链表头  */
	teec_context *context;
	/* < 指向所属的tee环境 */
} teec_sharedmemory;

/*
 * @ingroup teec_common_data
 * teec_tempmemory_reference结构体类型定义
 * 描述一块临时缓冲区指针\n
 * 可以用于#teec_parameter的类型，与其相对应的类型可以是
 * #teec_memref_temp_input， #teec_memref_temp_output，或#teec_memref_temp_inout
 */
typedef struct {
	void *buffer;
	/* < 临时缓冲区指针  */
	size_t size;
	/* < 临时缓冲区大小  */
} teec_tempmemory_reference;

/*
 * @ingroup teec_common_data
 * teec_registeredmemory_reference结构体类型定义
 * 描述共享内存指针，指向事先注册或分配好的共享内存\n
 * 可以用于#teec_parameter的类型，与其相对应的类型可以是
 * #teec_memref_whole， #teec_memref_partial_input，
 * #teec_memref_partial_output，或 #teec_memref_partial_inout
 */
typedef struct {
	teec_sharedmemory *parent;
	/* < 共享内存指针  */
	size_t size;
	/* < 共享内存的使用大小  */
	size_t offset;
	/* < 共享内存的使用偏移  */
} teec_registeredmemory_reference;

/*
 * @ingroup teec_common_data
 * teec_value结构体类型定义
 * 描述少量的数据\n
 * 可以用于#teec_parameter的类型，与其相对应的类型可以是
 * #teec_value_input， #teec_value_output，或#teec_value_inout
 */
typedef struct {
	uint32_t a;
	/* < 整型数据a  */
	uint32_t b;
	/* < 整型数据b  */
} teec_value;

typedef struct {
	int ion_share_fd;  /* < 整型数据a  */
	size_t ion_size;  /* < 整型数据b  */
} teec_ion_reference;

/*
 * @ingroup teec_common_data
 * teec_parameter联合体类型定义
 * 描述#teec_operation所对应的参数类型
 */
typedef union {
	teec_tempmemory_reference tmpref;
	/* < 描述#teec_tempmemory_reference类型  */
	teec_registeredmemory_reference memref;
	/* < 描述#teec_registeredmemory_reference类型  */
	teec_value value;
	/* < 描述#teec_value类型  */
	teec_ion_reference ionref;
} teec_parameter;

typedef struct {
	uint32_t event_type;
	/* tui event type */
	uint32_t value;
	/* return value, is keycode if tui event is getkeycode */
	uint32_t notch; /* notch size of phone */
	uint32_t width; /* width of foldable screen */
	uint32_t height;/* height of foldable screen */
	uint32_t fold_state;/* state of foldable screen */
	uint32_t display_state;/* one state of folded state */
	uint32_t phy_width;/* real width of the mobile */
	uint32_t phy_height;/* real height of the mobile */
} teec_tui_parameter;

/*
 * @ingroup teec_common_data
 * teec_operation结构体类型定义
 *
 * 打开会话或发送命令时的参数可以通过此类型描述，
 * 取消操作也可以使用此参数
 */
typedef struct {
	uint32_t started;
	/* < 标识是否是取消操作，0表示取消操作  */
	uint32_t paramtypes;
	/* 描述params的参数类型#teec_paramtype,
	 * 需要使用宏#teec_param_types组合参数类?	 */
	teec_parameter params[4];
	/* < 参数内容，类型为#teec_parameter  */
	teec_session *session;
	bool cancel_flag;
} teec_operation;

/* begin: for KERNEL-HAL out interface */
/*
 * @ingroup TEEC_COMMON_DATA
 * 函数返回值类型定义
 *
 * 用于表示函数返回结果
 */
typedef uint32_t TEEC_Result;

/*
 * @ingroup TEEC_COMMON_DATA
 * UUID类型定义
 *
 * UUID类型遵循RFC4122 [2]，用于标识安全服务
 */
typedef struct {
	uint32_t timeLow;
	/* < 时间戳的低4字节  */
	uint16_t timeMid;
	/* < 时间戳的中2字节  */
	uint16_t timeHiAndVersion;
	/* < 时间戳的高2字节与版本号的组合  */
	uint8_t clockSeqAndNode[8];
	/* < 时钟序列与节点标识符的组合  */
} TEEC_UUID;

/*
 * @ingroup TEEC_COMMON_DATA
 * TEEC_Context结构体类型定义
 *
 * 描述客户端应用与安全世界之间建立的链接环境
 */
typedef struct {
	void *dev;
	uint8_t *ta_path;
	struct list_node session_list;
	/* < 会话链表  */
	struct list_node shrd_mem_list;
	/* < 共享内存链表  */
} TEEC_Context;

/*
 * @ingroup TEEC_COMMON_DATA
 * TEEC_Session结构体类型定义
 *
 * 描述客户端应用与安全世界之间建立的会话
 */
typedef struct {
	uint32_t session_id;
	/* < 会话ID，由安全世界返回  */
	TEEC_UUID service_id;
	/* < 安全服务的UUID，每个安全服务拥有唯一的UUID  */
	uint32_t ops_cnt;
	/* < 在会话内的操作数  */
	struct list_node head;
	/* < 会话链表头  */
	TEEC_Context *context;
	/* < 指向会话所属的TEE环境  */
#ifdef SECURITY_AUTH_ENHANCE
	/* TOKEN_SAVE_LEN_24byte = token_16byte + timestamp_8byte */
	uint8_t teec_token[TOKEN_SAVE_LEN];
#endif
} TEEC_Session;

/*
 * @ingroup TEEC_COMMON_DATA
 * TEEC_SharedMemory结构体类型定义
 *
 * 描述一块共享内存，可以注册，也可以分配
 */
typedef struct {
	void *buffer;
	/* < 内存指针  */
	size_t size;
	/* < 内存大小  */
	uint32_t flags;
	/* < 内存标识符，用于区别输入或输出，取值范围为#TEEC_SharedMemCtl  */
	uint32_t ops_cnt;
	/* < 内存操作数  */
	bool is_allocated;
	/* < 内存分配标示符，用于区别是注册的，还是分配的  */
	struct list_node head;
	/* < 共享内存链表头  */
	TEEC_Context *context;
	/* < 指向所属的TEE环境 */
} TEEC_SharedMemory;

/*
 * @ingroup TEEC_COMMON_DATA
 * TEEC_TempMemoryReference结构体类型定义
 *
 * 描述一块临时缓冲区指针\n
 * 可以用于#TEEC_Parameter的类型，与其相对应的类型可以是
 * #TEEC_MEMREF_TEMP_INPUT， #TEEC_MEMREF_TEMP_OUTPUT，或#TEEC_MEMREF_TEMP_INOUT
 */
typedef struct {
	void *buffer;
	/* < 临时缓冲区指针  */
	size_t size;
	/* < 临时缓冲区大小  */
} TEEC_TempMemoryReference;

/*
 * @ingroup TEEC_COMMON_DATA
 * TEEC_RegisteredMemoryReference结构体类型定义
 *
 * 描述共享内存指针，指向事先注册或分配好的共享内存\n
 * 可以用于#TEEC_Parameter的类型，与其相对应的类型可以是
 * #TEEC_MEMREF_WHOLE， #TEEC_MEMREF_PARTIAL_INPUT，
 * #TEEC_MEMREF_PARTIAL_OUTPUT，或 #TEEC_MEMREF_PARTIAL_INOUT
 */
typedef struct {
	TEEC_SharedMemory *parent;
	/* < 共享内存指针  */
	size_t size;
	/* < 共享内存的使用大小  */
	size_t offset;
	/* < 共享内存的使用偏移  */
} TEEC_RegisteredMemoryReference;

/*
 * @ingroup TEEC_COMMON_DATA
 * TEEC_Value结构体类型定义
 *
 * 描述少量的数据\n
 * 可以用于#TEEC_Parameter的类型，与其相对应的类型可以是
 * #TEEC_VALUE_INPUT， #TEEC_VALUE_OUTPUT，或#TEEC_VALUE_INOUT
 */
typedef struct {
	uint32_t a;
	/* < 整型数据a  */
	uint32_t b;
	/* < 整型数据b  */
} TEEC_Value;

typedef struct {
	int ion_share_fd;  /* < 整型数据a  */
	size_t ion_size;  /* < 整型数据b  */
} TEEC_IonReference;

/*
 * @ingroup TEEC_COMMON_DATA
 * TEEC_Parameter联合体类型定义
 *
 * 描述#TEEC_Operation所对应的参数类型
 */
typedef union {
	TEEC_TempMemoryReference tmpref;
	/* < 描述#TEEC_TempMemoryReference类型  */
	TEEC_RegisteredMemoryReference memref;
	/* < 描述#TEEC_RegisteredMemoryReference类型  */
	TEEC_Value value;
	/* < 描述#TEEC_Value类型  */
	TEEC_IonReference ionref;
} TEEC_Parameter;

typedef struct {
	uint32_t event_type;
	/* Tui event type */
	uint32_t value;
	/* return value, is keycode if tui event is getKeycode */
	uint32_t notch; /* notch size of phone */
	uint32_t width; /* width of foldable screen */
	uint32_t height;/* height of foldable screen */
	uint32_t fold_state;/* state of foldable screen */
	uint32_t display_state;/* one state of folded state */
	uint32_t phy_width;/* real width of the mobile */
	uint32_t phy_height;/* real height of the mobile */
} TEEC_TUI_Parameter;

/*
 * @ingroup TEEC_COMMON_DATA
 * TEEC_Operation结构体类型定义
 *
 * 打开会话或发送命令时的参数可以通过此类型描述，
 * 取消操作也可以使用此参数
 */
typedef struct {
	uint32_t started;
	/*  标识是否是取消操作，0表示取消操作*/
	uint32_t paramTypes;
	/* 描述params的参数类型#TEEC_ParamType,
	 * 需要使用宏#TEEC_PARAM_TYPES组合参数类型
	 */
	TEEC_Parameter params[4];
	/*  参数内容，类型为#TEEC_Parameter  */
	TEEC_Session *session;
	bool cancel_flag;
} TEEC_Operation;

/* end: for KERNEL-HAL out interface */
#endif
