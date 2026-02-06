

#ifndef __OAL_UTIL_H__
#define __OAL_UTIL_H__

/* 其他头文件包含 */
#include "oal_types.h"
#include "oal_mm.h"
#include "arch/oal_util.h"
#include "securec.h"
#include "platform_oneimage_define.h"

/* 宏定义 */
#define OAL_VA_START va_start
#define OAL_VA_END   va_end

#define OAL_VA_LIST va_list

#define OAL_MAC_ADDRESS_LAN 6
#define OAL_SEC_PER_MIN 60
#define OAL_COUNTRY_CODE_LAN 3
/* 拼接为16 bit或者 32bit */
#define oal_make_word16(lsb, msb)           ((((uint16_t)(msb) << 8) & 0xFF00) | (lsb))
#define oal_make_word32(lsw, msw)           ((((uint32_t)(msw) << 16) & 0xFFFF0000) | (lsw))
#define oal_join_word32(lsb, ssb, asb, msb) (((uint32_t)(msb) << 24) | ((uint32_t)(asb) << 16) | \
                                             ((uint32_t)(ssb) << 8) | (lsb))
#define oal_join_word20(lsw, msw)           ((((uint32_t)(msw) << 10) & 0xFFC00) | ((lsw) & 0x3FF))

/* 计算为字节对齐后填充后的长度 */
#define padding_m(x, size) (((x) + (size)-1) & (~((size)-1)))

/* increment with wrap-around */
#define oal_incr(_l, _sz)  \
    do {                   \
        (_l)++;            \
        (_l) &= ((_sz) - 1); \
    } while (0)

#define oal_decr(_l, _sz)  \
    do {                   \
        (_l)--;            \
        (_l) &= ((_sz) - 1); \
    } while (0)

/* 获取数组大小 */
#define oal_array_size(_ast_array) (sizeof(_ast_array) / sizeof((_ast_array)[0]))

/* 四字节对齐 */
#define oal_get_4byte_align_value(_ul_size) (((_ul_size) + 0x03) & (~0x03))

/* 获取当前线程信息 */
#define OAL_CURRENT_TASK (current_thread_info()->task)

#define oal_swap_byteorder_16(_val) ((((_val)&0x00FF) << 8) + (((_val)&0xFF00) >> 8))

#if (_PRE_BIG_CPU_ENDIAN == _PRE_CPU_ENDIAN) /* BIG_ENDIAN */
#define oal_byteorder_to_le32(_val)     oal_swap_byteorder_32(_val)
#define oal_byteorder_to_le16(_val)     oal_swap_byteorder_16(_val)
#define oal_mask_inverse(_len, _offset) ((uint32_t)(oal_swap_byteorder_32(~(((1 << (_len)) - 1) << (_offset)))))
#define oal_mask(_len, _offset)         ((uint32_t)(oal_swap_byteorder_32(((1 << (_len)) - 1) << (_offset))))
#define oal_ntoh_16(_val)               _val
#define oal_ntoh_32(_val)               _val
#define oal_hton_16(_val)               _val
#define oal_hton_32(_val)               _val

#elif (_PRE_LITTLE_CPU_ENDIAN == _PRE_CPU_ENDIAN) /* LITTLE_ENDIAN */
#define oal_byteorder_to_le32(_val)     _val
#define oal_byteorder_to_le16(_val)     _val
#define oal_mask_inverse(_len, _offset) ((uint32_t)(~(((1UL << (_len)) - 1) << (_offset))))
#define oal_mask(_len, _offset)         ((uint32_t)(((1UL << (_len)) - 1) << (_offset)))
#define oal_ntoh_16(_val)               oal_swap_byteorder_16(_val)
#define oal_ntoh_32(_val)               oal_swap_byteorder_32(_val)
#define oal_hton_16(_val)               oal_swap_byteorder_16(_val)
#define oal_hton_32(_val)               oal_swap_byteorder_32(_val)
#endif

#define oal_value_eq_any2(_value, _val0, _val1) \
    (((_val0) == (_value)) || ((_val1) == (_value)))
#define oal_value_eq_any3(_value, _val0, _val1, _val2) \
    (((_val0) == (_value)) || ((_val1) == (_value)) || ((_val2) == (_value)))

#define oal_value_eq_any4(_value, _val0, _val1, _val2, _val3) \
    (((_val0) == (_value)) || ((_val1) == (_value)) || ((_val2) == (_value)) || ((_val3) == (_value)))

#define oal_value_eq_all2(_value, _val0, _val1) \
    (((_val0) == (_value)) && ((_val1) == (_value)))
#define oal_value_nq_and_eq(_nqvalue, _val0, _eqvalue, _val1) \
    (((_val0) != (_nqvalue)) && ((_val1) == (_eqvalue)))

#define oal_value_in_valid_range(_value, _start, _end)       (((_value) >= (_start)) && ((_value) <=(_end)))
#define oal_value_not_in_valid_range(_value, _start, _end)   (((_value) < (_start)) || ((_value) > (_end)))
#define oal_any_true_value2(_val0, _val1)                    (((_val0) == OAL_TRUE) || ((_val1) == OAL_TRUE))
#define oal_value_ne_all2(value, _val0, _val1)               (((_val0) != (value)) && ((_val1) != (value)))
#define oal_any_zero_value2(_val0, _val1)                    (((_val0) == 0) || ((_val1) == 0))

#define oal_any_null_ptr1(_ptr1)                             (((_ptr1) == NULL))
#define oal_any_null_ptr2(_ptr1, _ptr2)                      (((_ptr1) == NULL) || ((_ptr2) == NULL))
#define oal_any_null_ptr3(_ptr1, _ptr2, _ptr3)               (((_ptr1) == NULL) || ((_ptr2) == NULL) || \
                                                              ((_ptr3) == NULL))
#define oal_any_null_ptr4(_ptr1, _ptr2, _ptr3, _ptr4)        (((_ptr1) == NULL) || ((_ptr2) == NULL) || \
                                                              ((_ptr3) == NULL) || ((_ptr4) == NULL))
#define oal_any_null_ptr5(_ptr1, _ptr2, _ptr3, _ptr4, _ptr5) (((_ptr1) == NULL) || ((_ptr2) == NULL) || \
                                                              ((_ptr3) == NULL) || ((_ptr4) == NULL) || \
                                                              ((_ptr5) == NULL))
#define oal_all_not_null_ptr2(_ptr1, _ptr2)                   (((_ptr1) != NULL) && ((_ptr2) != NULL))
#define oal_all_true_value2(_val0, _val1)                     (((_val0) == OAL_TRUE) && ((_val1) == OAL_TRUE))

#define oal_all_zero_value2(_val0, _val1)                     (((_val0) == 0) && ((_val1) == 0))

#if (!defined(_PRE_PC_LINT) && !defined(WIN32))
#ifdef __GNUC__
#define oal_build_bug_on(_con) ((void)sizeof(char[1 - 2 * !!(_con)]))
#else
#define oal_build_bug_on(_con)
#endif
#else
#define oal_bug_on(_con)
#define oal_build_bug_on(_con)
#endif

#ifndef atomic_inc_return
#define oal_atomic_inc_return(a) 0
#else
#define oal_atomic_inc_return atomic_inc_return
#endif

// dead loop
#define forever_loop()  for (;;)

/* 比较宏 */
#define oal_min(_A, _B) (((_A) < (_B)) ? (_A) : (_B))

/* 比较宏 */
#define oal_max(_A, _B) (((_A) > (_B)) ? (_A) : (_B))

#define oal_sub(_A, _B) (((_A) > (_B)) ? ((_A) - (_B)) : (0))

#define oal_absolute_sub(_A, _B) (((_A) > (_B)) ? ((_A) - (_B)) : ((_B) - (_A)))

/* 从某个设备读取某个寄存器地址的32-bit寄存器的值。 */
#define oal_reg_read32(_addr) \
    *((OAL_VOLATILE uint32_t *)(_addr))

#define oal_reg_read16(_addr) \
    *((OAL_VOLATILE uint16_t *)(_addr))

/* 往某个设备某个32-bit寄存器地址写入某个值 */
#define oal_reg_write32(_addr, _val) \
    (*((OAL_VOLATILE uint32_t *)(_addr)) = (_val))
#define oal_reg_write16(_addr, _val) \
    (*((OAL_VOLATILE uint16_t *)(_addr)) = (_val))

/* Is val aligned to "align" ("align" must be power of 2) */
#ifndef IS_ALIGNED
#define oal_is_aligned(val, align) \
    (((uint32_t)(val) & ((align) - 1)) == 0)
#else
#define oal_is_aligned IS_ALIGNED
#endif

/* Bit Values */
#define BIT31 ((uint32_t)(1UL << 31))
#define BIT30 ((uint32_t)(1 << 30))
#define BIT29 ((uint32_t)(1 << 29))
#define BIT28 ((uint32_t)(1 << 28))
#define BIT27 ((uint32_t)(1 << 27))
#define BIT26 ((uint32_t)(1 << 26))
#define BIT25 ((uint32_t)(1 << 25))
#define BIT24 ((uint32_t)(1 << 24))
#define BIT23 ((uint32_t)(1 << 23))
#define BIT22 ((uint32_t)(1 << 22))
#define BIT21 ((uint32_t)(1 << 21))
#define BIT20 ((uint32_t)(1 << 20))
#define BIT19 ((uint32_t)(1 << 19))
#define BIT18 ((uint32_t)(1 << 18))
#define BIT17 ((uint32_t)(1 << 17))
#define BIT16 ((uint32_t)(1 << 16))
#define BIT15 ((uint32_t)(1 << 15))
#define BIT14 ((uint32_t)(1 << 14))
#define BIT13 ((uint32_t)(1 << 13))
#define BIT12 ((uint32_t)(1 << 12))
#define BIT11 ((uint32_t)(1 << 11))
#define BIT10 ((uint32_t)(1 << 10))
#define BIT9  ((uint32_t)(1 << 9))
#define BIT8  ((uint32_t)(1 << 8))
#define BIT7  ((uint32_t)(1 << 7))
#define BIT6  ((uint32_t)(1 << 6))
#define BIT5  ((uint32_t)(1 << 5))
#define BIT4  ((uint32_t)(1 << 4))
#define BIT3  ((uint32_t)(1 << 3))
#define BIT2  ((uint32_t)(1 << 2))
#define BIT1  ((uint32_t)(1 << 1))
#define BIT0  ((uint32_t)(1 << 0))
#define ALL   0xFFFF

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif

#define OAL_BITS_PER_BYTE 8 /* 一个字节中包含的bit数目 */

/* 位操作 */
#define oal_set_bit(_val)                        (1 << (_val))
#define oal_left_shift(_data, _num)              ((_data) << (_num))
#define oal_rght_shift(_data, _num)              ((_data) >> (_num))
#define oal_write_bits(_data, _val, _bits, _pos) \
    do {                                                                    \
        (_data) &= ~((((uint32_t)1 << (_bits)) - 1) << (_pos));           \
        (_data) |= (((_val) & (((uint32_t)1 << (_bits)) - 1)) << (_pos)); \
    } while (0)
#define oal_get_bits(_data, _bits, _pos) (((_data) >> (_pos)) & (((uint32_t)1 << (_bits)) - 1))

/* 位数定义 */
#define NUM_1_BITS  1
#define NUM_2_BITS  2
#define NUM_3_BITS  3
#define NUM_4_BITS  4
#define NUM_5_BITS  5
#define NUM_6_BITS  6
#define NUM_7_BITS  7
#define NUM_8_BITS  8
#define NUM_9_BITS  9
#define NUM_10_BITS 10
#define NUM_11_BITS 11
#define NUM_12_BITS 12
#define NUM_13_BITS 13
#define NUM_14_BITS 14
#define NUM_15_BITS 15
#define NUM_16_BITS 16
#define NUM_17_BITS 17
#define NUM_18_BITS 18
#define NUM_19_BITS 19
#define NUM_20_BITS 20
#define NUM_21_BITS 21
#define NUM_22_BITS 22
#define NUM_23_BITS 23
#define NUM_24_BITS 24
#define NUM_25_BITS 25
#define NUM_26_BITS 26
#define NUM_27_BITS 27
#define NUM_28_BITS 28
#define NUM_29_BITS 29
#define NUM_30_BITS 30
#define NUM_31_BITS 31
#define NUM_32_BITS 32

/* 字节数定义 */
#define NUM_1_BYTES  1
#define NUM_2_BYTES  2
#define NUM_3_BYTES  3
#define NUM_4_BYTES  4
#define NUM_5_BYTES  5
#define NUM_6_BYTES  6
#define NUM_7_BYTES  7
#define NUM_8_BYTES  8
#define NUM_9_BYTES  9
#define NUM_10_BYTES 10
#define NUM_11_BYTES 11
#define NUM_12_BYTES 12
#define NUM_13_BYTES 13
#define NUM_14_BYTES 14
#define NUM_15_BYTES 15
#define NUM_16_BYTES 16
#define NUM_17_BYTES 17
#define NUM_18_BYTES 18
#define NUM_19_BYTES 19
#define NUM_20_BYTES 20
#define NUM_21_BYTES 21
#define NUM_22_BYTES 22
#define NUM_23_BYTES 23
#define NUM_24_BYTES 24
#define NUM_25_BYTES 25
#define NUM_26_BYTES 26
#define NUM_27_BYTES 27
#define NUM_28_BYTES 28
#define NUM_29_BYTES 29
#define NUM_30_BYTES 30
#define NUM_31_BYTES 31
#define NUM_32_BYTES 32
#define NUM_255_BYTES 255


/* 定点数四舍五入，并取整数部分, fract_bits为小数位数 */
#define _round_pos(fix_num, fract_bits) ((uint16_t)((fix_num) + (1u << ((uint16_t)(fract_bits)-1))) >> \
                                         ((uint16_t)(fract_bits)))
#define _round_neg(fix_num, fract_bits) (-_round_pos(-(fix_num), (fract_bits)))
#define oal_round(fix_num, fract_bits)  ((fix_num) > 0 ? _round_pos(fix_num, fract_bits) : \
                                                         _round_neg(fix_num, fract_bits))

#define OAL_RSSI_INIT_MARKER   0x320 /* RSSI平滑值初始值 */
#define OAL_RSSI_MAX_DELTA     24    /* 最大步长 24/8 = 3 */
#define OAL_RSSI_FRACTION_BITS 3
#define OAL_RSSI_SIGNAL_MIN    (-103)   /* 上报RSSI下边界 */
#define OAL_RSSI_SIGNAL_MAX    5      /* 上报RSSI上边界 */
#define OAL_SNR_INIT_VALUE     0x7F   /* SNR上报的初始值 */
#define OAL_RSSI_INIT_VALUE    (-128) /* RSSI的初始值 */

#define OAL_IPV6_ADDR_LEN 16
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define OAL_TRINITY_NAME_LEN 50
#endif

/* STRUCT定义 */
#define HI11XX_LOG_ERR     0
#define HI11XX_LOG_WARN    1
#define HI11XX_LOG_INFO    2
#define HI11XX_LOG_DBG     3
#define HI11XX_LOG_VERBOSE 4

#ifdef CONFIG_PRINTK
#include <linux/module.h>

#include "platform_oneimage_define.h"

#ifndef HI11XX_LOG_MODULE_NAME
#define HI11XX_LOG_MODULE_NAME "[HI11XX]"
extern int32_t g_hi11xx_loglevel;
#endif

#ifndef HI11XX_LOG_MODULE_NAME_VAR
#define HI11XX_LOG_MODULE_NAME_VAR g_hi11xx_loglevel
extern int32_t g_hi11xx_loglevel;
#else
static int32_t HI11XX_LOG_MODULE_NAME_VAR = HI11XX_LOG_INFO;
#if defined(PLATFORM_DEBUG_ENABLE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
module_param(HI11XX_LOG_MODULE_NAME_VAR, int, S_IRUGO | S_IWUSR);
#endif
#endif

#define hi11xx_log_module_name_nums_str(num) #num

extern char *g_hi11xx_loglevel_format[];

#define oal_print_hi11xx_log(loglevel, fmt, arg...)                         \
    do {                                                                    \
        if (oal_unlikely(HI11XX_LOG_MODULE_NAME_VAR >= loglevel)) {         \
            printk("%s%s" fmt "[%s:%d]\n",                                  \
                g_hi11xx_loglevel_format[loglevel], HI11XX_LOG_MODULE_NAME, \
                ##arg, __FUNCTION__, __LINE__);                             \
        }                                                                   \
    } while (0)
#else
//lint -emacro( {522}, oal_print_hi11xx_log)
#define oal_print_hi11xx_log
#endif

#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
typedef struct _hwifi_panic_log_ hwifi_panic_log;
typedef int32_t (*hwifi_panic_log_cb)(void *data, char *pst_buf, int32_t buf_len);
struct _hwifi_panic_log_ {
    struct list_head list;
    /* the log module name */
    char *name;
    hwifi_panic_log_cb cb;
    void *data;
};
#define declare_wifi_panic_stru(module_name, func) \
    hwifi_panic_log module_name = {                \
        .name = #module_name,                      \
        .cb = (hwifi_panic_log_cb)func,            \
    }
#endif

/* 函数声明 */
#ifdef _PRE_CONFIG_HISI_PANIC_DUMP_SUPPORT
/*
 * 函 数 名  : hwifi_panic_log_register
 * 功能描述  : Kernl Panic 的回调函数注册
 */
extern void hwifi_panic_log_register(hwifi_panic_log *log, void *data);
extern void hwifi_panic_log_dump(char *print_level);
#else
OAL_STATIC OAL_INLINE void hwifi_panic_log_dump(char *print_level)
{
}
#endif

OAL_STATIC OAL_INLINE void oal_print_inject_check_stack(void)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    char trinity_name[OAL_TRINITY_NAME_LEN];

    memset_s(trinity_name, sizeof(trinity_name), 0, sizeof(trinity_name));
    if (memcpy_s(trinity_name, sizeof(trinity_name), current->comm, OAL_STRLEN("trinity")) != EOK) {
        oal_io_print("memcpy_s error, destlen=%u, srclen=%u\n ",
                     (uint32_t)sizeof(trinity_name), (uint32_t)OAL_STRLEN("trinity"));
        return;
    }

    if (unlikely(!memcmp((void *)"trinity", (void *)trinity_name, OAL_STRLEN("trinity")))) {
        /* Debug */
        WARN_ON(1);
    }
#endif
}

OAL_STATIC OAL_INLINE uint8_t oal_strtohex(const char *c_string)
{
    uint8_t uc_ret = 0;
    if (oal_unlikely(c_string == NULL)) {
        oal_warn_on(1);
        return 0;
    }

    if (*c_string >= '0' && *c_string <= '9') {
        uc_ret = (uint8_t)(*c_string - '0');
    } else if (*c_string >= 'A' && *c_string <= 'F') {
        uc_ret = (uint8_t)(*c_string - 'A' + 10); /* 加10为了保证取出的'A'~'F'等于10~15 */
    } else if (*c_string >= 'a' && *c_string <= 'f') {
        uc_ret = (uint8_t)(*c_string - 'a' + 10); /* 加10为了保证取出的'a'~'f'等于10~15 */
    }

    return uc_ret;
}

/*
 * 函 数 名  : oal_strtoaddr
 * 功能描述  : 字符串转MAC地址
 * 输入参数  : pc_param: MAC地址字符串, 格式 xx:xx:xx:xx:xx:xx  分隔符支持':'与'-'
 * 输出参数  : puc_mac_addr: 转换成16进制后的MAC地址
 */
OAL_STATIC OAL_INLINE void oal_strtoaddr(const char *pc_param,
    uint8_t uc_param_len, uint8_t *puc_mac_addr, uint8_t uc_mac_addr_len)
{
    uint8_t uc_char_index;
    const uint32_t uc_mac_max_num = 12; /* 单字节遍历是不是正确的mac地址:xx:xx:xx:xx:xx:xx，跳过数字间的字符 */
    const char *pc_param_end = NULL;

    if (oal_unlikely((pc_param == NULL) || (puc_mac_addr == NULL))) {
        oal_warn_on(1);
        return;
    }

    pc_param_end = pc_param + uc_param_len;

    /* 获取mac地址,16进制转换 */
    for (uc_char_index = 0; uc_char_index < uc_mac_max_num; uc_char_index++) {
        if (pc_param >= pc_param_end) {
            oal_warn_on(1);
            return;
        }

        if ((*pc_param == ':') || (*pc_param == '-')) {
            pc_param++;
            if (uc_char_index != 0) {
                uc_char_index--;
            }

            continue;
        }
        if ((uc_char_index / 2) >= uc_mac_addr_len) {
            oal_warn_on(1);
            return;
        }

        /* 下列计算是把字符串转换成的mac地址写到buff里，xx:xx 只对应两个mac地址 */
        puc_mac_addr[uc_char_index / 2] =
            (uint8_t)(puc_mac_addr[uc_char_index / 2] * 16 * (uc_char_index % 2) +
                        oal_strtohex(pc_param));
        pc_param++;
    }
}

/*
 * 函 数 名  : oal_strtoipv6
 * 功能描述  : 字符串转ipv6地址
 * 输入参数  : pc_param: ipv6地址字符串, 格式 xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx
 * 输出参数  : puc_mac_addr: 转换成16进制后的ipv6地址
 */
OAL_STATIC OAL_INLINE void oal_strtoipv6(const char *pc_param, unsigned char *puc_ipv6_addr)
{
    uint8_t uc_char_index;
    const uint32_t uc_ipv6_max_num = OAL_IPV6_ADDR_LEN * 2; /* 单字节遍历是不是正确的ipv6地址 */

    if (oal_unlikely((pc_param == NULL) || (puc_ipv6_addr == NULL))) {
        oal_warn_on(1);
        return;
    }

    /* 获取ipv6地址,16进制转换 */
    for (uc_char_index = 0; uc_char_index < uc_ipv6_max_num; uc_char_index++) {
        if ((*pc_param == ':')) {
            pc_param++;
            if (uc_char_index != 0) {
                uc_char_index--;
            }

            continue;
        }
        /* 将ipv6字符串转换为16进制数组 */
        puc_ipv6_addr[uc_char_index >> 1] =
            (uint8_t)(((puc_ipv6_addr[uc_char_index >> 1]) << 4) * (uc_char_index % 2) +
                        oal_strtohex(pc_param));
        pc_param++;
    }
}

OAL_STATIC OAL_INLINE int oal_memcmp(const void *p_buf1, const void *p_buf2, uint32_t ul_count)
{
    return OAL_MEMCMP(p_buf1, p_buf2, ul_count);
}

OAL_STATIC OAL_INLINE int oal_strncmp(const char *p_buf1, const char *p_buf2, uint32_t ul_count)
{
    return OAL_STRNCMP(p_buf1, p_buf2, ul_count);
}

OAL_STATIC OAL_INLINE int oal_strncasecmp(const char *p_buf1,
                                              const char *p_buf2,
                                              uint32_t ul_count)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    return OAL_STRNCASECMP(p_buf1, p_buf2, ul_count);
#else
    return OAL_STRNCMP(p_buf1, p_buf2, ul_count); /* windows still use strncmp */
#endif
}

/*
 * 函 数 名  : oal_get_random_bytes
 * 功能描述  : 从urandom文件获取随机值
 * 输入参数  : pc_random_buf :从文件读出数据后存放的buf
 *             ul_random_len :读取数据大小(字节数)
 */
OAL_STATIC OAL_INLINE void oal_get_random_bytes(char *pc_random_buf, uint32_t ul_random_len)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (oal_unlikely(pc_random_buf == NULL)) {
        oal_warn_on(1);
        return;
    }
    get_random_bytes(pc_random_buf, ul_random_len);
#endif
}

/*
 * 函 数 名  : oal_gen_random
 * 功能描述  : 产生随机数
 * 输入参数  : ul_val:随机种子   us_rst_flag:0:不更新随机种子，非0:更新随机种子
 */
OAL_STATIC OAL_INLINE uint8_t oal_gen_random(uint32_t ul_val, uint8_t us_rst_flag)
{
    OAL_STATIC uint32_t ul_rand = 0;
    if (us_rst_flag != 0) {
        ul_rand = ul_val;
    }
    ul_rand = ul_rand * 1664525L + 1013904223L;
    return (uint8_t)(ul_rand >> 24); // 左移24位，取最高字节
}

/*
 * 函 数 名  : oal_bit_get_num_one_byte
 * 功能描述  : 获取单字节中的bit1的个数
 * 输入参数  : uc_byte:需要操作的字节
 * 返 回 值  : bit的个数
 */
OAL_STATIC OAL_INLINE uint8_t oal_bit_get_num_one_byte(uint8_t uc_byte)
{
    uc_byte = (uc_byte & 0x55) + ((uc_byte >> 1) & 0x55);
    uc_byte = (uc_byte & 0x33) + ((uc_byte >> 2) & 0x33);
    uc_byte = (uc_byte & 0x0F) + ((uc_byte >> 4) & 0x0F);

    return uc_byte;
}

/*
 * 函 数 名  : oal_bit_get_num_four_byte
 * 功能描述  : 获取4字节中bit1的个数
 * 输入参数  : uc_byte:需要操作的字节
 * 返 回 值  : bit的个数
 */
OAL_STATIC OAL_INLINE uint32_t oal_bit_get_num_four_byte(uint32_t ul_byte)
{
    ul_byte = (ul_byte & 0x55555555) + ((ul_byte >> 1) & 0x55555555);
    ul_byte = (ul_byte & 0x33333333) + ((ul_byte >> 2) & 0x33333333);
    ul_byte = (ul_byte & 0x0F0F0F0F) + ((ul_byte >> 4) & 0x0F0F0F0F);
    ul_byte = (ul_byte & 0x00FF00FF) + ((ul_byte >> 8) & 0x00FF00FF);
    ul_byte = (ul_byte & 0x0000FFFF) + ((ul_byte >> 16) & 0x0000FFFF);

    return ul_byte;
}

/*
 * 函 数 名  : oal_bit_set_bit_one_byte
 * 功能描述  : 对1字节的指定位置一
 * 输入参数  : puc_byte: 要进行位操作的变量地址
 *             nr: 置位的位数
 */
OAL_STATIC OAL_INLINE void oal_bit_set_bit_one_byte(uint8_t *puc_byte, oal_bitops nr)
{
    if (oal_unlikely(puc_byte == NULL)) {
        oal_warn_on(1);
        return;
    }
    *puc_byte |= ((uint8_t)(1 << nr));
}

/*
 * 函 数 名  : oal_bit_clear_bit_one_byte
 * 功能描述  : 对1字节的指定位置零
 * 输入参数  : puc_byte: 要进行位操作的变量地址
 *             nr: 置零的位数
 */
OAL_STATIC OAL_INLINE void oal_bit_clear_bit_one_byte(uint8_t *puc_byte, oal_bitops nr)
{
    if (oal_unlikely(puc_byte == NULL)) {
        oal_warn_on(1);
        return;
    }
    *puc_byte &= (~((uint8_t)(1 << nr)));
}

OAL_STATIC OAL_INLINE uint8_t oal_bit_get_bit_one_byte(uint8_t uc_byte, oal_bitops nr)
{
    return ((uc_byte >> nr) & 0x1);
}

/*
 * 函 数 名  : oal_bit_set_bit_four_byte
 * 功能描述  : 对4字节的指定位置一
 * 输入参数  : pul_byte: 要进行位操作的变量地址
 *            nr: 置位的位数
 */
OAL_STATIC OAL_INLINE void oal_bit_set_bit_four_byte(uint32_t *pul_byte, oal_bitops nr)
{
    if (oal_unlikely(pul_byte == NULL)) {
        oal_warn_on(1);
        return;
    }
    *pul_byte |= ((uint32_t)(1 << nr));
}

/*
 * 函 数 名  : oal_bit_clear_bit_four_byte
 * 功能描述  : 对4字节的指定位置零
 * 输入参数  : pul_byte: 要进行位操作的变量地址
 *             nr: 置零的位数
 */
OAL_STATIC OAL_INLINE void oal_bit_clear_bit_four_byte(uint32_t *pul_byte, oal_bitops nr)
{
    if (oal_unlikely(pul_byte == NULL)) {
        oal_warn_on(1);
        return;
    }
    *pul_byte &= ~((uint32_t)(1 << nr));
}

/*
 * 函 数 名  : oal_bit_set_bit_eight_byte
 * 功能描述  : 对8字节的指定位置一
 */
OAL_STATIC OAL_INLINE void oal_bit_set_bit_eight_byte(uint64_t *pull_byte, oal_bitops nr)
{
    if (oal_unlikely(pull_byte == NULL)) {
        oal_warn_on(1);
        return;
    }
    *pull_byte |= ((uint64_t)1 << nr);
}

/*
 * 函 数 名  : oal_bit_clear_bit_eight_byte
 * 功能描述  : 对8字节的指定位置0
 */
OAL_STATIC OAL_INLINE void oal_bit_clear_bit_eight_byte(uint64_t *pull_byte, oal_bitops nr)
{
    if (oal_unlikely(pull_byte == NULL)) {
        oal_warn_on(1);
        return;
    }
    *pull_byte &= ~((uint64_t)1 << nr);
}

/*
 * 函 数 名  : oal_bit_find_first_bit_one_byte
 * 功能描述  : 找到1字节右数第一个是1的位数
 * 输入参数  : uc_byte: 要查找的字节
 * 返 回 值  : 右数第一个是1的位数
 */
OAL_STATIC OAL_INLINE uint8_t oal_bit_find_first_bit_one_byte(uint8_t uc_byte)
{
    uint8_t uc_ret = 0;

    uc_byte = uc_byte & (uint8_t)(-uc_byte);

    while (uc_byte != 1) {
        uc_ret++;
        uc_byte = (uc_byte >> 1);

        if (uc_ret > 7) { /* 大于7， 这时候说明已经到了最高bit了 */
            return uc_ret;
        }
    }

    return uc_ret;
}

/*
 * 函 数 名  : oal_bit_find_first_zero_one_byte
 * 功能描述  : 找到1字节右数第一个是0的位数
 * 输入参数  : uc_byte: 要查找的字节
 * 返 回 值  : 右数第一个是0的位数
 */
OAL_STATIC OAL_INLINE uint8_t oal_bit_find_first_zero_one_byte(uint8_t uc_byte)
{
    uint8_t uc_ret = 0;

    uc_byte = ~uc_byte;
    uc_byte = uc_byte & (uint8_t)(-uc_byte);

    while (uc_byte != 1) {
        uc_ret++;
        uc_byte = (uc_byte >> 1);

        if (uc_ret > 7) { /* 大于7，这时候说明已经到了最高bit了 */
            return uc_ret;
        }
    }

    return uc_ret;
}

/*
 * 函 数 名  : oal_bit_find_first_bit_four_byte
 * 功能描述  : 找到1字节右数第一个是1的位数
 * 输入参数  : puc_byte: 要查找的字节
 * 返 回 值  : 右数第一个是1的位数
 */
OAL_STATIC OAL_INLINE uint8_t oal_bit_find_first_bit_four_byte(uint32_t ul_byte)
{
    uint8_t uc_ret = 0;

    if (ul_byte == 0) {
        return uc_ret;
    }

    if (!(ul_byte & 0xffff)) {
        ul_byte >>= 16; /* 说明32bit数据的低16bit全为0，所以可以直接从高16bit开始找为1的位数 */
        uc_ret += 16;
    }

    if (!(ul_byte & 0xff)) {
        ul_byte >>= 8; /* 说明16bit数据的低8bit全为0，所以可以直接从高8bit开始找为1的位数 */
        uc_ret += 8;
    }

    if (!(ul_byte & 0xf)) {
        ul_byte >>= 4; /* 说明8bit数据的低4bit全为0，所以可以直接从高4bit开始找为1的位数 */
        uc_ret += 4;
    }

    if (!(ul_byte & 3)) {
        ul_byte >>= 2; /* 说明4bit数据的低2bit全为0，所以可以直接从高2bit开始找为1的位数 */
        uc_ret += 2;
    }

    if (!(ul_byte & 1)) {
        uc_ret += 1;
    }

    return uc_ret;
}

/*
 * 函 数 名  : oal_bit_find_first_zero_four_byte
 * 功能描述  : 找到1字节右数第一个是0的位数
 * 输入参数  : puc_byte: 要查找的字节
 * 返 回 值  : 右数第一个是0的位数
 */
OAL_STATIC OAL_INLINE uint8_t oal_bit_find_first_zero_four_byte(uint32_t byte)
{
    uint8_t uc_ret = 0;

    byte = ~byte;

    if (!(byte & 0xffff)) {
        byte >>= 16; /* 说明32bit数据的低16bit全为1，所以可以直接从高16bit开始找为0的位数 */
        uc_ret += 16;
    }

    if (!(byte & 0xff)) {
        byte >>= 8; /* 说明16bit数据的低8bit全为1，所以可以直接从高8bit开始找为0的位数 */
        uc_ret += 8;
    }

    if (!(byte & 0xf)) {
        byte >>= 4; /* 说明8bit数据的低4bit全为1，所以可以直接从高4bit开始找为0的位数 */
        uc_ret += 4;
    }

    if (!(byte & 3)) {
        byte >>= 2; /* 说明4bit数据的低2bit全为1，所以可以直接从高2bit开始找为0的位数 */
        uc_ret += 2;
    }

    if (!(byte & 1)) {
        uc_ret += 1;
    }

    return uc_ret;
}

#define oal_compare_mac_addr_m(puc_mac_addr1, puc_mac_addr2)  \
    (uint32_t)oal_memcmp((void *)(puc_mac_addr1), (void *)(puc_mac_addr2), OAL_MAC_ADDRESS_LAN)

/*
 * 函 数 名  : oal_set_mac_addr
 * 功能描述  : 将第二个mac地址 赋值给第一个mac地址
 * 输入参数  : puc_mac_addr1: 第一个mac地址
 *             puc_mac_addr2: 第二个mac地址
 */
OAL_STATIC OAL_INLINE void oal_set_mac_addr(unsigned char *puc_mac_addr1, const unsigned char *puc_mac_addr2)
{
    if (memcpy_s(puc_mac_addr1, OAL_MAC_ADDRESS_LAN, puc_mac_addr2, OAL_MAC_ADDRESS_LAN) != EOK) {
        oal_io_print("oal_set_mac_addr: memcpy_s failed.\n");
    }
}

OAL_STATIC OAL_INLINE void oal_set_mac_addr_zero(unsigned char *puc_mac_addr)
{
    memset_s(puc_mac_addr, OAL_MAC_ADDRESS_LAN, 0, OAL_MAC_ADDRESS_LAN);
}

OAL_STATIC OAL_INLINE uint32_t oal_compare_mac_addr(const unsigned char *puc_mac_addr1,
                                                    const unsigned char *puc_mac_addr2)
{
    return (uint32_t)oal_memcmp((void *)puc_mac_addr1, (void *)puc_mac_addr2, OAL_MAC_ADDRESS_LAN);
}
OAL_STATIC OAL_INLINE uint32_t oal_compare_country_code(const unsigned char *puc_country_code1,
                                                        const unsigned char *puc_country_code2)
{
    return (uint32_t)oal_memcmp((void *)puc_country_code1, (void *)puc_country_code2, OAL_COUNTRY_CODE_LAN);
}
/*
 * 函 数 名  : oal_cmp_seq_num
 * 功能描述  : 比较两个接收描述符的中断顺序号的大小，seq_num1大于seq_num2返回真
 */
OAL_STATIC OAL_INLINE oal_bool_enum_uint8 oal_cmp_seq_num(uint32_t ul_seq_num1,
                                                          uint32_t ul_seq_num2,
                                                          uint32_t ul_diff_value)
{
    if (((ul_seq_num1 < ul_seq_num2) && ((ul_seq_num2 - ul_seq_num1) < ul_diff_value)) ||
        ((ul_seq_num1 > ul_seq_num2) && ((ul_seq_num1 - ul_seq_num2) > ul_diff_value))) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

OAL_STATIC OAL_INLINE int32_t oal_strcmp(const char *pc_src, const char *pc_dst)
{
    char c_c1;
    char c_c2;
    int32_t l_ret = 0;

    if (oal_unlikely((pc_src == NULL) || (pc_dst == NULL))) {
        oal_warn_on(1);
        return 1;
    }

    do {
        c_c1 = *pc_src++;
        c_c2 = *pc_dst++;
        l_ret = c_c1 - c_c2;
        if (l_ret) {
            break;
        }
    } while (c_c1);

    return l_ret;
}

/*
 * 函 数 名  : oal_strim
 * 功能描述  : 去掉字符串开始与结尾的空格
 */
OAL_STATIC OAL_INLINE char *oal_strim(char *pc_s)
{
    uint32_t size;
    char *pc_end = NULL;

    if (oal_unlikely(pc_s == NULL)) {
        oal_warn_on(1);
        return NULL;
    }

    while (*pc_s == ' ') {
        ++pc_s;
    }

    size = OAL_STRLEN(pc_s);
    if (!size) {
        return pc_s;
    }

    pc_end = pc_s + size - 1;
    while (pc_end >= pc_s && *pc_end == ' ') {
        pc_end--;
    }

    *(pc_end + 1) = '\0';

    return pc_s;
}

OAL_STATIC OAL_INLINE char *oal_strstr(char *pc_s1, char *pc_s2)
{
    return OAL_STRSTR(pc_s1, pc_s2);
}

/*
 * 函 数 名  : oal_init_lut
 * 功能描述  : 初始化传入的LUT BITMAP表
 */
OAL_STATIC OAL_INLINE uint32_t oal_init_lut(uint8_t *puc_lut_index_table, uint8_t uc_bmap_len)
{
    uint8_t uc_lut_idx;

    if (oal_unlikely(puc_lut_index_table == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

    for (uc_lut_idx = 0; uc_lut_idx < uc_bmap_len; uc_lut_idx++) {
        puc_lut_index_table[uc_lut_idx] = 0;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_get_lut_index
 * 功能描述  : 从LUT index bitmap表中，获取一个没有被使用的索引，没有找到的话，
 *             返回不可用的索引标识(非关键路径，未进行优化，有时间可以优化)
 */
OAL_STATIC OAL_INLINE uint8_t oal_get_lut_index(uint8_t *puc_lut_index_table,
                                                uint8_t uc_bmap_len,
                                                uint16_t us_max_lut_size,
                                                uint16_t us_start,
                                                uint16_t us_stop)
{
    uint8_t uc_byte = 0;
    uint8_t uc_bit = 0;
    uint8_t uc_temp = 0;
    uint16_t us_index = 0;

    if (oal_unlikely(puc_lut_index_table == NULL)) {
        oal_warn_on(1);
        return 0;
    }

    for (uc_byte = 0; uc_byte < uc_bmap_len; uc_byte++) {
        uc_temp = puc_lut_index_table[uc_byte];

        for (uc_bit = 0; uc_bit < 8; uc_bit++) { /* 以一个bit为单位遍历某一位为0 */
            if ((uc_temp & (1 << uc_bit)) == 0x0) {
                us_index = (uc_byte * 8 + uc_bit); /* 算出为0的bit位的位数 */
                if ((us_index < us_start) || (us_index >= us_stop)) {
                    continue;
                }
                if (us_index < us_max_lut_size) {
                    puc_lut_index_table[uc_byte] |= (uint8_t)(1 << uc_bit);

                    return (uint8_t)us_index;
                } else {
                    return (uint8_t)us_max_lut_size;
                }
            }
        }
    }

    return (uint8_t)us_max_lut_size;
}

/*
 * 函 数 名  : oal_del_lut_index
 * 功能描述  : 在LUT index bitmap表中，删除一个LUT index (注:%可以作为优化项)
 */
OAL_STATIC OAL_INLINE void oal_del_lut_index(uint8_t *puc_lut_index_table, uint8_t uc_idx)
{
    uint8_t uc_byte = uc_idx >> 3;
    uint8_t uc_bit = uc_idx & 0x07;

    if (oal_unlikely(puc_lut_index_table == NULL)) {
        oal_warn_on(1);
        return;
    }

    puc_lut_index_table[uc_byte] &= ~(uint8_t)(1 << uc_bit);
}

OAL_STATIC OAL_INLINE oal_bool_enum oal_is_active_lut_index(uint8_t *puc_lut_idx_status_table,
                                                            uint16_t us_max_lut_size, uint8_t uc_idx)
{
    uint8_t uc_byte = uc_idx >> 3;
    uint8_t uc_bit = uc_idx & 0x07;

    if (oal_unlikely(puc_lut_idx_status_table == NULL)) {
        oal_warn_on(1);
        return OAL_FALSE;
    }

    if (uc_idx >= us_max_lut_size) {
        return OAL_FALSE;
    }

    return puc_lut_idx_status_table[uc_byte] & ((uint8_t)(1 << uc_bit)) ? OAL_TRUE : OAL_FALSE;
}

OAL_STATIC OAL_INLINE void oal_set_lut_index_status(uint8_t *puc_lut_idx_status_table,
                                                    uint16_t us_max_lut_size, uint8_t uc_idx)
{
    uint8_t uc_byte = uc_idx >> 3;
    uint8_t uc_bit = uc_idx & 0x07;

    if (oal_unlikely(puc_lut_idx_status_table == NULL)) {
        oal_warn_on(1);
        return;
    }

    if (uc_idx >= us_max_lut_size) {
        return;
    }

    puc_lut_idx_status_table[uc_byte] |= (uint8_t)(1 << uc_bit);
}

OAL_STATIC OAL_INLINE void oal_reset_lut_index_status(uint8_t *puc_lut_idx_status_table,
                                                      uint16_t us_max_lut_size, uint8_t uc_idx)
{
    uint8_t uc_byte = uc_idx >> 3;
    uint8_t uc_bit = uc_idx & 0x07;

    if (oal_unlikely(puc_lut_idx_status_table == NULL)) {
        oal_warn_on(1);
        return;
    }

    if (uc_idx >= us_max_lut_size) {
        return;
    }

    puc_lut_idx_status_table[uc_byte] &= ~(uint8_t)(1 << uc_bit);
}

/*
 * 函 数 名  : oal_get_virt_addr
 * 功能描述  : 物理地址转化为虚拟地址
 */
OAL_STATIC OAL_INLINE uint32_t *oal_get_virt_addr(uint32_t *pul_phy_addr)
{
    /* 空指针无需转化 */
    if (pul_phy_addr == NULL) {
        return pul_phy_addr;
    }

    return (uint32_t *)oal_phy_to_virt_addr((uintptr_t)pul_phy_addr);
}

extern int32_t oal_dump_stack_str(uint8_t *puc_str, uint32_t ul_max_size);

OAL_STATIC OAL_INLINE int8_t oal_get_real_rssi(int16_t s_scaled_rssi)
{
    /* 四舍五入 */
    return oal_round(s_scaled_rssi, OAL_RSSI_FRACTION_BITS);
}

OAL_STATIC OAL_INLINE void oal_rssi_smooth(int16_t *ps_old_rssi, int8_t c_new_rssi)
{
    int16_t s_delta;

    if (oal_unlikely(ps_old_rssi == NULL)) {
        oal_warn_on(1);
        return;
    }

    /* 若上报的值超过了合法范围，则不进行平滑操作，函数直接返回 */
    if (c_new_rssi < OAL_RSSI_SIGNAL_MIN || c_new_rssi > OAL_RSSI_SIGNAL_MAX) {
        return;
    }

    /* 若上报的值为0，则是描述符未填rssi值,不进行平滑 */
    if (c_new_rssi == 0) {
        return;
    }

    /* 判断为初始值,芯片上报的rssi作为平滑结果 */
    if (*ps_old_rssi == OAL_RSSI_INIT_MARKER) {
        *ps_old_rssi = (int16_t)c_new_rssi << OAL_RSSI_FRACTION_BITS;
    }

    /* old_rssi四舍五入后计算delta */
    s_delta = (int16_t)c_new_rssi - oal_get_real_rssi(*ps_old_rssi);
    if (s_delta > OAL_RSSI_MAX_DELTA) {
        s_delta = OAL_RSSI_MAX_DELTA;
    }
    if (s_delta < -OAL_RSSI_MAX_DELTA) {
        s_delta = -OAL_RSSI_MAX_DELTA;
    }
    *ps_old_rssi += s_delta;
}

#endif /* end of oal_util.h */
