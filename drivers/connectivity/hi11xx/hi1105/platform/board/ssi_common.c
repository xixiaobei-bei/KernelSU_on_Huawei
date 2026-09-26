

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG

/* 头文件包含 */
#include "ssi_common.h"
#include "plat_debug.h"
#include "plat_firmware.h"
#include "board.h"
#include "ssi_hi1103.h"
#include "ssi_hi1106.h"
#include "oal_kernel_file.h"
#include "chr_user.h"

#define HI110X_SSI_CLK_GPIO_NAME  "hi110x ssi clk"
#define HI110X_SSI_DATA_GPIO_NAME "hi110x ssi data"
#define INTERVAL_TIME             10
#define SSI_DATA_LEN              16
#define FILENAME_MAX_LEN          200
#define ssi_delay(x) ndelay(x)

typedef struct ht_test_s {
    int32_t add;
    int32_t data;
} ht_test_t;

static ht_test_t g_ht_cnt[] = {
    { 0x50000314, 0x0D00 },
    { 0x50002724, 0x0022 },
    { 0x50002720, 0x0033 },
};

int g_hi11xx_kernel_crash = 0;
int g_ssi_is_logfile = 0;
uint32_t g_halt_det_cnt = 0; /* 检测soc异常次数 */
char *g_ssi_cpu_st_str[] = {
    "OFF",              /* 0x0 */
    "BOOTING",          /* 0x1 */
    "SLEEPING",         /* 0x2 */
    "WORK",             /* 0x3 */
    "SAVING",           /* 0x4 */
    "PROTECT(ocp/scp)", /* 0x5 */
    "SLEEP",            /* 0x6 */
    "PROTECTING"        /* 0x7 */
};

#ifdef _PRE_WINDOWS_SUPPORT
oal_define_spinlock(g_ssi_lock);
#else
static oal_define_spinlock(g_ssi_lock);
#endif
static uint32_t g_ssi_lock_state = 0x0;
static char g_str_gpio_ssi_dump_path[100] = HISI_TOP_LOG_DIR "/wifi/memdump";
static int g_ssi_dfr_bypass = 0;
static int g_ssi_dump_enable = 1;


#ifdef _PRE_HI_DRV_GPIO
static uint32_t g_ssi_clk_gpio = SSI_CLK_HISI_GPIO;                /* 模拟ssi时钟的GPIO管脚号 */
static uint32_t g_ssi_data_gpio = SSI_DATA_HISI_GPIO;              /* 模拟ssi数据线的GPIO管脚号 */
#else
static uint32_t g_ssi_clk_gpio = 0;              /* 模拟ssi时钟的GPIO管脚号 */
static uint32_t g_ssi_data_gpio = 0;             /* 模拟ssi数据线的GPIO管脚号 */
#endif
static uint16_t g_ssi_base_addr = 0x8000;        /* ssi基址 */
static uint32_t g_interval_time = INTERVAL_TIME; /* GPIO拉出来的波形保持时间，单位us */
static uint32_t g_ssi_delay = 5;

static ssi_reg_info g_ssi_master_reg_full = { 0x38, 0x28, SSI_RW_SSI_MOD };
oal_spin_lock_stru* get_ssi_lock_glb_addr(void)
{
    return &g_ssi_lock;
}

void ssi_excetpion_dump_disable(void)
{
    g_ssi_dump_enable = 0;
    ps_print_err("ssi excetpion dump disable\n");
}

void ssi_excetpion_dump_enable(void)
{
    g_ssi_dump_enable = 1;
    ps_print_err("ssi excetpion dump enable\n");
}

/* ssi 工作时必须切换ssi clock, 此时aon会受到影响，BCPU/WCPU 有可能异常，慎用! */
static int32_t ssi_try_lock(void)
{
    unsigned long flags;
    oal_spin_lock_irq_save(&g_ssi_lock, &flags);
    if (g_ssi_lock_state) {
        /* lock failed */
        oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
        return 1;
    }
    g_ssi_lock_state = 1;
    oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
    return 0;
}

static int32_t ssi_unlock(void)
{
    unsigned long flags;
    oal_spin_lock_irq_save(&g_ssi_lock, &flags);
    g_ssi_lock_state = 0;
    oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
    return 0;
}

int32_t wait_for_ssi_idle_timeout(int32_t mstimeout)
{
    int32_t can_sleep = 0;
    int32_t timeout = mstimeout;
    if (oal_in_interrupt() || oal_in_atomic() || irqs_disabled()) {
        can_sleep = 0;
    } else {
        can_sleep = 1;
    }
    /* 考虑效率，这里需要判断是否可以睡眠 */
    while (ssi_try_lock()) {
        if (can_sleep) {
            msleep(1);
        } else {
            mdelay(1);
        }
        if (!(--timeout)) {
            ps_print_err("wait for ssi timeout:%dms\n", mstimeout);
            return 0;
        }
    }
    ssi_unlock();
    return timeout;
}

STATIC OAL_INLINE int32_t ssi_gpio_valid(void)
{
    if ((g_st_board_info.ssi_gpio_clk != 0) &&
        (g_st_board_info.ssi_gpio_data != 0)) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

int ssi_set_gpio_pins(int32_t clk, int32_t data)
{
    g_st_board_info.ssi_gpio_clk = clk;
    g_st_board_info.ssi_gpio_data = data;

    ssi_request_gpio(g_st_board_info.ssi_gpio_clk, g_st_board_info.ssi_gpio_data);
    ps_print_info("set ssi gpio clk:%d , gpio data:%d\n", clk, data);
    return 0;
}

int32_t ssi_show_setup(void)
{
    ps_print_info("clk=%d, data=%d, interval=%d us, ssi base=0x%x, r/w delay=%d cycle\n",
                  g_ssi_clk_gpio, g_ssi_data_gpio, g_interval_time, g_ssi_base_addr, g_ssi_delay);
    return BOARD_SUCC;
}

int32_t ssi_setup(uint32_t interval, uint32_t delay, uint16_t ssi_base)
{
    g_interval_time = interval;
    g_ssi_delay = delay;
    g_ssi_base_addr = ssi_base;

    return BOARD_SUCC;
}

#ifdef _PRE_HI_DRV_GPIO
static int32_t hitv_ssi_request_gpio(uint32_t clk, uint32_t data)
{
    /* Oscar的v811主芯片不支持Linux的DTS机制，目前还在适配标准的内核GPIO，等小海思提供接口后该函数再另行整改 */
    /*
    {0xF8A21218, 0x00000000}, //RegName:ctrl57; PadName:SD_D2; RegValue: 0x0--SD_D2,0x1--GPIOPWM6_7,0x2--TSI2_D7,
                                //0x3--NA,0x4--NA,0x5--NA,0x6--NA,0x7--NA
    {0xF8A2121C, 0x00000000}, //RegName:ctrl58; PadName:SD_D3; RegValue: 0x0--SD_D3,0x1--GPIOPWM7_0,0x2--TSI2_D2,
    */
    (void __iomem *) gpio_mux_addr = ioremap_nocache(GPIO_MUX_REG_ADDR, GPIO_MUX_REG_VALUE);
    if (gpio_mux_addr != NULL) {
        writel(1, gpio_mux_addr);
        writel(1, (void __iomem *)(uintptr_t)((unsigned long)(uintptr_t)gpio_mux_addr + GPIO_MUX_REG_OFFSET));
        iounmap(gpio_mux_addr);
    }

    g_ssi_clk_gpio = SSI_CLK_HISI_GPIO;
    g_ssi_data_gpio = SSI_DATA_HISI_GPIO;
    hitv_gpio_request(g_ssi_clk_gpio, 0);
    hitv_gpio_direction_output(g_ssi_clk_gpio, GPIO_LOWLEVEL);
    hitv_gpio_request(g_ssi_data_gpio, 0);
    hitv_gpio_direction_output(g_ssi_data_gpio, GPIO_LOWLEVEL);

    return BOARD_SUCC;
}
#endif

int32_t ssi_request_gpio(uint32_t clk, uint32_t data)
{
#ifdef _PRE_HI_DRV_GPIO
    return hitv_ssi_request_gpio(clk, data);
#else
    int32_t ret;

    ps_print_dbg("request hi110x ssi GPIO\n");

    ret = gpio_request_one(clk, GPIOF_OUT_INIT_LOW, HI110X_SSI_CLK_GPIO_NAME);
    if (ret) {
        ps_print_err("%s gpio_request_one failed ret=%d\n", HI110X_SSI_CLK_GPIO_NAME, ret);
        goto err_get_ssi_clk_gpio;
    }

    g_ssi_clk_gpio = clk;

    ret = gpio_request_one(data, GPIOF_OUT_INIT_LOW, HI110X_SSI_DATA_GPIO_NAME);
    if (ret) {
        ps_print_err("%s gpio_request_one failed ret=%d\n", HI110X_SSI_DATA_GPIO_NAME, ret);
        goto err_get_ssi_data_gpio;
    }

    g_ssi_data_gpio = data;

    return BOARD_SUCC;

err_get_ssi_data_gpio:
    gpio_free(clk);
    g_ssi_clk_gpio = 0;
err_get_ssi_clk_gpio:

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_GPIO, CHR_PLAT_DRV_ERROR_SSI_GPIO);

    return ret;
#endif
}

int32_t ssi_free_gpio(void)
{
    ps_print_dbg("free hi110x ssi GPIO\n");

    if (g_ssi_clk_gpio != 0) {
#ifndef _PRE_HI_DRV_GPIO
        gpio_free(g_ssi_clk_gpio);
#endif
        g_ssi_clk_gpio = 0;
    }

    if (g_ssi_data_gpio != 0) {
#ifndef _PRE_HI_DRV_GPIO
        gpio_free(g_ssi_data_gpio);
#endif
        g_ssi_data_gpio = 0;
    }

    return BOARD_SUCC;
}

STATIC void ssi_clk_output(void)
{
    oal_gpio_direction_output(g_ssi_clk_gpio, GPIO_LOWLEVEL);
    ssi_delay(g_interval_time);
    oal_gpio_direction_output(g_ssi_clk_gpio, GPIO_HIGHLEVEL);
}

STATIC void ssi_data_output(uint16_t data)
{
    ssi_delay(5);
    if (data) {
        oal_gpio_direction_output(g_ssi_data_gpio, GPIO_HIGHLEVEL);
    } else {
        oal_gpio_direction_output(g_ssi_data_gpio, GPIO_LOWLEVEL);
    }

    ssi_delay(g_interval_time);
}

STATIC int32_t ssi_write_data(uint16_t addr, uint16_t value)
{
    uint16_t tx;
    uint32_t i;

    for (i = 0; i < g_ssi_delay; i++) {
        ssi_clk_output();
        ssi_data_output(0);
    }

    /* 发送SYNC位 */
    ps_print_dbg("tx sync bit\n");
    ssi_clk_output();
    ssi_data_output(1);

    /* 指示本次操作为写，高读低写 */
    ps_print_dbg("tx r/w->w\n");
    ssi_clk_output();
    ssi_data_output(0);

    /* 发送地址 */
    ps_print_dbg("write addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        ps_print_dbg("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /* 发送数据 */
    ps_print_dbg("write value:0x%x\n", value);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (value >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        ps_print_dbg("tx data bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /* 数据发送完成以后，保持delay个周期的0 */
    ps_print_dbg("ssi write:finish, delay %d cycle\n", g_ssi_delay);
    for (i = 0; i < g_ssi_delay; i++) {
        ssi_clk_output();
        ssi_data_output(0);
    }

    return BOARD_SUCC;
}

STATIC uint16_t ssi_read_data(uint16_t addr)
{
#define SSI_READ_RETTY 1000
    uint16_t tx;
    uint32_t i;
    uint32_t retry = 0;
    uint16_t rx;
    uint16_t data = 0;

    for (i = 0; i < g_ssi_delay; i++) {
        ssi_clk_output();
        ssi_data_output(0);
    }

    /* 发送SYNC位 */
    ps_print_dbg("tx sync bit\n");
    ssi_clk_output();
    ssi_data_output(1);

    /* 指示本次操作为读，高读低写 */
    ps_print_dbg("tx r/w->r\n");
    ssi_clk_output();
    ssi_data_output(1);

    /* 发送地址 */
    ps_print_dbg("read addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        ps_print_dbg("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /* 延迟一个clk，否则上一个数据只保持了半个时钟周期 */
    ssi_clk_output();

    /* 设置data线GPIO为输入，准备读取数据 */
    oal_gpio_direction_input(g_ssi_data_gpio);

    ps_print_dbg("data in mod, current gpio level is %d\n", gpio_get_value(g_ssi_data_gpio));

    /* 读取SYNC同步位 */
    do {
        ssi_clk_output();
        ssi_delay(g_interval_time);
        if (oal_gpio_get_value(g_ssi_data_gpio)) {
            ps_print_dbg("read data sync bit ok, retry=%d\n", retry);
            break;
        }
        retry++;
    } while (retry != SSI_READ_RETTY);

    if (retry == SSI_READ_RETTY) {
        ps_print_err("ssi read sync bit timeout\n");
        ssi_data_output(0);
        return data;
    }

    for (i = 0; i < SSI_DATA_LEN; i++) {
        ssi_clk_output();
        ssi_delay(g_interval_time);
        rx = oal_gpio_get_value(g_ssi_data_gpio);
        ps_print_dbg("rx data bit %d:%d\n", SSI_DATA_LEN - i - 1, rx);
        data = data | (rx << (SSI_DATA_LEN - i - 1));
    }

    /* 恢复data线GPIO为输出，并输出0 */
    ssi_data_output(0);

    return data;
}

int32_t ssi_write16(uint16_t addr, uint16_t value)
{
#define WRITE_RETRY 3
    uint32_t retry = 0;
    uint16_t read_v;

    do {
        ssi_write_data(addr, value);
        read_v = ssi_read_data(addr);
        if (value == read_v) {
            ps_print_dbg("ssi write: 0x%x=0x%x succ\n", addr, value);
            return BOARD_SUCC;
        }
        retry++;
    } while (retry < WRITE_RETRY);

    ps_print_err("ssi write: 0x%x=0x%x ,read=0x%x fail\n", addr, value, read_v);

    return BOARD_FAIL;
}

uint16_t ssi_read16(uint16_t addr)
{
    uint16_t data;

    data = ssi_read_data(addr);

    ps_print_suc("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

int32_t ssi_write32(uint32_t addr, uint16_t value)
{
    uint16_t addr_half_word_high;
    uint16_t addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff; /* Shifts rightwards by 16 bits and 0xFFFF. */
    addr_half_word_low = (addr & 0xffff) >> 1;

    /* 往基地址写地址的高16位 */
    if (ssi_write16(g_ssi_base_addr, addr_half_word_high) < 0) {
        ps_print_err("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    /* 低地址写实际要写入的value */
    if (ssi_write16(addr_half_word_low, value) < 0) {
        ps_print_err("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    ps_print_dbg("ssi write: 0x%x=0x%x succ\n", addr, value);

    return BOARD_SUCC;
}

int32_t ssi_read32(uint32_t addr)
{
    uint16_t data;
    uint16_t addr_half_word_high;
    uint16_t addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff; /* Shifts rightwards by 16 bits and 0xFFFF. */
    addr_half_word_low = (addr & 0xffff) >> 1;

    if (ssi_write16(g_ssi_base_addr, addr_half_word_high) < 0) {
        ps_print_err("ssi read 0x%x fail\n", addr);
        return BOARD_FAIL;
    }

    data = ssi_read_data(addr_half_word_low);

    ps_print_dbg("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

STATIC int32_t ssi_read_data16(uint16_t addr, uint16_t *value)
{
#define SSI_READ_RETTY 1000
    uint16_t tx;
    uint32_t i;
    uint32_t retry = 0;
    uint16_t rx;
    uint16_t data = 0;

    for (i = 0; i < g_ssi_delay; i++) {
        ssi_clk_output();
        ssi_data_output(0);
    }

    /* 发送SYNC位 */
    ps_print_dbg("tx sync bit\n");
    ssi_clk_output();
    ssi_data_output(1);

    /* 指示本次操作为读，高读低写 */
    ps_print_dbg("tx r/w->r\n");
    ssi_clk_output();
    ssi_data_output(1);

    /* 发送地址 */
    ps_print_dbg("read addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        ps_print_dbg("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output();
        ssi_data_output(tx);
    }

    /* 延迟一个clk，否则上一个数据只保持了半个时钟周期 */
    ssi_clk_output();

    /* 设置data线GPIO为输入，准备读取数据 */
    oal_gpio_direction_input(g_ssi_data_gpio);

    ps_print_dbg("data in mod, current gpio level is %d\n", gpio_get_value(g_ssi_data_gpio));

    /* 读取SYNC同步位 */
    do {
        ssi_clk_output();
        ssi_delay(g_interval_time);
        if (oal_gpio_get_value(g_ssi_data_gpio)) {
            ps_print_dbg("read data sync bit ok, retry=%d\n", retry);
            break;
        }
        retry++;
    } while (retry != SSI_READ_RETTY);

    if (retry == SSI_READ_RETTY) {
        ps_print_err("ssi read sync bit timeout\n");
        ssi_data_output(0);
        return -OAL_EFAIL;
    }

    for (i = 0; i < SSI_DATA_LEN; i++) {
        ssi_clk_output();
        ssi_delay(g_interval_time);
        rx = oal_gpio_get_value(g_ssi_data_gpio);
        ps_print_dbg("rx data bit %d:%d\n", SSI_DATA_LEN - i - 1, rx);
        data = data | (rx << (SSI_DATA_LEN - i - 1));
    }

    /* 恢复data线GPIO为输出，并输出0 */
    ssi_data_output(0);

    *value = data;

    return OAL_SUCC;
}

/* 32bits address, 32bits value */
int32_t ssi_read_value16(uint32_t addr, uint16_t *value, int16_t last_high_addr)
{
    int32_t ret;
    uint16_t addr_half_word_high;
    uint16_t addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff; /* Shifts rightwards by 16 bits and 0xFFFF. */
    addr_half_word_low = (addr & 0xffff) >> 1;

    if (last_high_addr != addr_half_word_high) {
        if (ssi_write16(g_ssi_base_addr, addr_half_word_high) < 0) {
            ps_print_err("ssi read 0x%x fail\n", addr);
            return BOARD_FAIL;
        }
    }

    ret = ssi_read_data16(addr_half_word_low, value);

    ps_print_dbg("ssi read: 0x%x=0x%x\n", addr, *value);

    return ret;
}

/*
 * 函 数 名  : ssi_read_value32
 * 功能描述  : gpio模拟SSI 读32BIT value
 *             1.配置SSI 为32BIT模式
 *             2.第一次读16BIT操作，SOC发起32BIT操作，返回低16BIT给HOST
 *             3.第二次读同一地址16BIT操作，SOC不发起总线操作，返回高16BIT给HOST
 *             4.如果跳过步骤3 读其他地址，SOC侧高16BIT 会被丢弃
 */
int32_t ssi_read_value32(uint32_t addr, uint32_t *value, int16_t last_high_addr)
{
    int32_t ret;
    uint16_t reg;

    ret = ssi_read_value16(addr, &reg, last_high_addr);
    if (ret) {
        ps_print_err("read addr 0x%x low 16 bit failed, ret=%d\n", addr, ret);
        return ret;
    }
    *value = (uint32_t)reg;

    /* 读32位地址的高16位 */
    ret = ssi_read_value16(addr + 0x2, &reg, (addr >> 16));
    if (ret) {
        ps_print_err("read addr 0x%x high 16 bit failed, ret=%d\n", addr, ret);
        return ret;
    }

    *value = ((reg << 16) | *value); /* Shift leftwards by 16 bits. */

    return OAL_SUCC;
}

int32_t ssi_write_value32(uint32_t addr, uint32_t value)
{
    uint16_t addr_half_word_high;
    uint16_t addr_half_word_low;
    uint16_t addr_half_word_low_incr;

    addr_half_word_high = (addr >> 16) & 0xffff; /* Shifts rightwards by 16 bits and 0xFFFF. */
    addr_half_word_low = (addr & 0xffff) >> 1;
    addr_half_word_low_incr = ((addr + 2) & 0xffff) >> 1; /* Address offset 2  */

    /* 往基地址写地址的高16位 */
    if (ssi_write_data(g_ssi_base_addr, addr_half_word_high) < 0) {
        ps_print_err("ssi write high addr: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    /* 低地址写实际要写入的value */
    if (ssi_write_data(addr_half_word_low, value & 0xffff) < 0) {
        ps_print_err("ssi write low value: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    /* Shifts rightwards by 16 bits and 0xFFFF. */
    if (ssi_write_data(addr_half_word_low_incr, (value >> 16) & 0xffff) < 0) {
        ps_print_err("ssi write high value: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    ps_print_dbg("ssi write: 0x%x=0x%x succ\n", addr, value);

    return BOARD_SUCC;
}

/* 16bits/32bits switch mode */
static int32_t ssi_switch_ahb_mode(int32_t is_32bit_mode)
{
    return ssi_write16(0x8001, !!is_32bit_mode);
}

static int32_t ssi_clear_ahb_highaddr(void)
{
    return ssi_write16(g_ssi_base_addr, 0x0);
}

static int ssi_switch_to_tcxo_clk(void)
{
    int ret;

    ret = ssi_write16(gpio_ssi_reg(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO);
    ps_print_info("switch tcxo clk %s\n", (ret == BOARD_SUCC) ? "ok" : "failed");

    return ret;
}

static int ssi_switch_to_ssi_clk(void)
{
    int ret;

    ret = ssi_write16(gpio_ssi_reg(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI);
    ps_print_info("switch ssi clk %s\n", (ret == BOARD_SUCC) ? "ok" : "failed");

    return ret;
}

static int ssi_setup_clk(uint32_t clk_type)
{
    if (clk_type == SSI_AON_CLKSEL_TCXO) {
        return ssi_switch_to_tcxo_clk();
    } else if (clk_type == SSI_AON_CLKSEL_SSI) {
        return ssi_switch_to_ssi_clk();
    } else {
        ps_print_err("ssi clk_type %d is error\n", clk_type);
        return BOARD_FAIL;
    }
}

static int ssi_is_alive(void)
{
    uint16_t value;

    value = ssi_read16(gpio_ssi_reg(SSI_SYS_CTL_ID));
    if (value == SSI_ALIVE) {
        return 1;
    } else {
        return 0;
    }
}

static int ssi_read_single_seg_reg(ssi_reg_info *pst_reg_info, void *buf, int32_t buf_len,
                                   int seg_index, int32_t seg_size)
{
    int32_t ret, j, k;
    const int32_t step = 4;
    uint32_t reg, ssi_address;
    uint16_t reg16, ssi_master_address;
    uint16_t last_high_addr = 0x0;
    const uint32_t ul_max_ssi_read_retry_times = 3;

    for (j = 0; j < seg_size; j += step) {
        ssi_address = pst_reg_info->base_addr + seg_index * buf_len + j;

        for (k = 0; k < ul_max_ssi_read_retry_times; k++) {
            reg = 0x0;
            reg16 = 0x0;
            if (pst_reg_info->rw_mod == SSI_RW_SSI_MOD) {
                ssi_master_address = (uint16_t)gpio_ssi_reg(ssi_address);
                ret = ssi_read_data16(ssi_master_address, &reg16);
                reg = reg16;
            } else if (pst_reg_info->rw_mod == SSI_RW_DWORD_MOD) {
                ret = ssi_read_value32(ssi_address, &reg, last_high_addr);
            } else {
                ret = ssi_read_value16(ssi_address, &reg16, last_high_addr);
                reg = reg16;
            }

            if (ret == 0) {
                break;
            }
        }

        if (k == ul_max_ssi_read_retry_times) {
            ps_print_err("ssi read address 0x%x failed, retry %d times", ssi_address, k);
            goto fail_read_reg;
        }

        last_high_addr = (ssi_address >> 16); /* Shift rightwards by 16 bits */
        oal_writel(reg, (uintptr_t)buf + j);
    }

    return OAL_SUCC;

fail_read_reg:
    if (ssi_address > pst_reg_info->base_addr) {
#ifdef CONFIG_PRINTK
        /* print the read buf before errors, rowsize = 32,groupsize = 4 */
        print_hex_dump(KERN_INFO, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
                       buf, oal_min(buf_len, ssi_address - pst_reg_info->base_addr), false); /* 内核函数固定传参 */
#endif
    }

    return -OAL_EFAIL;
}

static void ssi_ahb_mode_set(uint32_t mode, int32_t flag)
{
    if (mode == SSI_RW_DWORD_MOD) {
        if (flag == SSI_AHB_MODE_SET_START) {
            /* switch 32bits mode */
            ssi_switch_ahb_mode(1);
        } else {
            /* switch 16bits mode */
            ssi_switch_ahb_mode(0);
        }
    } else {
        if (flag == SSI_AHB_MODE_SET_START) {
            ssi_switch_ahb_mode(0);
        }
    }
}

static int ssi_read_segs_reg(char *filename, int32_t is_file, ssi_reg_info *pst_reg_info,
                             void *buf, int32_t buf_len)
{
    int i, seg_size, seg_nums, left_size;
    int ret = -OAL_EFAIL;
    mm_segment_t fs;
    int32_t is_logfile;
    os_kernel_file *fp = NULL;

    is_logfile = is_file;

    if (buf_len == 0) {
        ps_print_err("buf_len is zero\n");
        return -OAL_EFAIL;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);

    if (is_logfile) {
        fp = filp_open(filename, O_RDWR | O_CREAT, 0644); /* 0644 is File permission */
        if (oal_is_err_or_null(fp)) {
            ps_print_info("open file %s failed ret=%ld\n", filename, PTR_ERR(fp));
            is_logfile = 0;
        } else {
            ps_print_info("open file %s succ\n", filename);
            vfs_llseek(fp, 0, SEEK_SET);
        }
    }

    ssi_clear_ahb_highaddr();

    ssi_ahb_mode_set(pst_reg_info->rw_mod, SSI_AHB_MODE_SET_START);

retry:
    seg_nums = (pst_reg_info->len - 1 / buf_len) + 1;
    left_size = pst_reg_info->len;

    for (i = 0; i < seg_nums; i++) {
        seg_size = oal_min(left_size, buf_len);
        ret = ssi_read_single_seg_reg(pst_reg_info, buf, buf_len, i, seg_size);
        if (ret < 0) {
            break;
        }
        left_size -= seg_size;

        if (is_logfile) {
            ret = oal_kernel_file_write(fp, buf, seg_size);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
            vfs_fsync(fp, 0);
#else
            vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
            if (ret != seg_size) {
                ps_print_err("ssi print file failed, request %d, write %d actual\n", seg_size, ret);
                is_logfile = 0;
                filp_close(fp, NULL);
                goto retry;
            }
        } else {
#ifdef CONFIG_PRINTK
            /* print to kenrel msg. rowsize = 32,groupsize = 4 */
            print_hex_dump(KERN_INFO, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
                           buf, seg_size, false); /* 内核函数固定传参 */
#endif
        }
    }

    ssi_ahb_mode_set(pst_reg_info->rw_mod, SSI_AHB_MODE_SET_END);

    if (is_logfile) {
        filp_close(fp, NULL);
    }
    set_fs(fs);

    return ret;
}

static void *ssi_malloc_memory(uint32_t size, int32_t is_atomic)
{
    void *buf = NULL;

    if (size == 0) { /* 0 is invalid */
        return NULL;
    }

    if (is_atomic) {
        buf = kmalloc(size, GFP_ATOMIC);
    } else {
        buf = os_vmalloc_gfp(size);
    }

    return buf;
}

static void ssi_free_memory(const void *buf, int32_t is_atomic)
{
    if (buf == NULL) {
        return;
    }

    if (is_atomic) {
        kfree(buf);
    } else {
        os_mem_vfree(buf);
    }
}

static int ssi_read_reg_prep(ssi_reg_info *pst_reg_info, int32_t *is_logfile, int32_t *is_atomic,
                             char *filename, uint32_t len)
{
    struct timeval tv;
    struct rtc_time tm = {0};
    int ret = OAL_SUCC;

    if (oal_in_interrupt() || oal_in_atomic() || irqs_disabled()) {
        *is_logfile = 0;
        *is_atomic = 1;
    }

    if (*is_logfile) {
        oal_do_gettimeofday(&tv);
        rtc_time_to_tm(tv.tv_sec, &tm);
        ret = snprintf_s(filename, len, len - 1, "%s/gpio_ssi_%08x_%08x_%04d%02d%02d%02d%02d%02d.bin",
                         g_str_gpio_ssi_dump_path,
                         pst_reg_info->base_addr,
                         pst_reg_info->base_addr + pst_reg_info->len - 1,
                         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, /* Based on 1900 */
                         tm.tm_hour, tm.tm_min, tm.tm_sec); /* 转换成当前时间 */
        if (ret < 0) {
            ps_print_err("log str format err line[%d]\n", __LINE__);
            return -OAL_EFAIL;
        }
    }
    return ret;
}

static int ssi_read_master_regs(ssi_reg_info *pst_reg_info, void *buf, int32_t size, int32_t is_file)
{
    int32_t is_atomic = 0;
    uint32_t realloc = 0;
    int32_t buf_len;
    int32_t is_logfile;
    char filename[FILENAME_MAX_LEN];

    is_logfile = is_file;
    buf_len = size;
    memset_s(filename, sizeof(filename), 0, sizeof(filename));

    if (ssi_read_reg_prep(pst_reg_info, &is_logfile, &is_atomic, filename, sizeof(filename)) < 0) {
        return -OAL_EFAIL;
    }

    if (buf == NULL) {
        buf = ssi_malloc_memory(pst_reg_info->len, is_atomic);
        if (buf == NULL) {
            ps_print_info("alloc mem failed before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
            return -OAL_ENOMEM;
        }
        buf_len = pst_reg_info->len;
        realloc = 1;
    }

    ps_print_info("dump reg info 0x%x:%u, buf len:%u \n", pst_reg_info->base_addr, pst_reg_info->len, buf_len);

    if (ssi_read_segs_reg(filename, is_logfile, pst_reg_info, buf, buf_len) < 0) {
        goto read_reg_fail;
    }

    if (realloc) {
        ssi_free_memory(buf, is_atomic);
    }

    return 0;

read_reg_fail:
    if (realloc) {
        ssi_free_memory(buf, is_atomic);
    }

    return -OAL_EFAIL;
}

static int ssi_setup_clk_enhance(uint32_t clk_type)
{
    uint16_t value;

    if (!ssi_is_alive()) {
        ps_print_err("ssi is not alive, clk type %d\n", clk_type);
        return BOARD_FAIL;
    }

    if (clk_type == SSI_AON_CLKSEL_TCXO) {
        return ssi_setup_clk(clk_type);
    }

    /* switch clk to ssi clk */
    ssi_read_master_regs(&g_ssi_master_reg_full, NULL, 0, g_ssi_is_logfile);
    /* 判断TCXO是否在位,TCXO不在位需要切换到ssi clk */
    /* bit[5:3] b_cur_sts,bit[2:0] w_cur_sts,中有一个为0x3，表示TCXO在位 */
    value = ssi_read16(gpio_ssi_reg(SSI_RPT_STS_L));
    if (((value & 0x3) != 0x3) && ((value & 0x18) != 0x18)) {
        ps_print_info("ssi tcxo not available, switch clk to ssi\n");
        return ssi_setup_clk(clk_type);
    }

    /* TCXO 在位，读取aon寄存器，读不出来需要切换到ssi clk */
    if (ssi_check_device_isalive()) {
        ps_print_info("ssi read aon fail, switch clk to ssi\n");
        return ssi_setup_clk(clk_type);
    }

    return BOARD_SUCC;
}

int ssi_clk_auto_switch_is_support(void)
{
    /* after hi1105, we use tcxo to replase ssi clk,
       we switch to ssi clk just when tcxo gating */
    int32_t chip_type = get_hi110x_subchip_type();

    switch (chip_type) {
        case BOARD_VERSION_HI1105:
        case BOARD_VERSION_HI1106:
            return OAL_TRUE;
        default:
            return OAL_FALSE;
    }
}

int ssi_switch_clk(uint32_t clk_type)
{
    if (ssi_clk_auto_switch_is_support() == OAL_TRUE) {
        return ssi_setup_clk_enhance(clk_type);
    } else {
        return ssi_setup_clk(clk_type);
    }
}

void ssi_force_dereset_reg(void)
{
    /* 解复位AON，注意寄存器配置顺序 */
    ssi_write16(gpio_ssi_reg(SSI_SSI_CTRL), 0x60);
    ssi_write16(gpio_ssi_reg(SSI_SEL_CTRL), 0x60);
}

void ssi_force_reset_reg(void)
{
    /* 先复位再解复位AON，注意寄存器配置顺序 */
    ssi_write16(gpio_ssi_reg(SSI_SEL_CTRL), 0x60);
    ssi_write16(gpio_ssi_reg(SSI_SSI_CTRL), 0x60);
    ps_print_info("ssi_ctrl:0x%x sel_ctrl:0x%x\n", ssi_read16(gpio_ssi_reg(SSI_SSI_CTRL)),
                  ssi_read16(gpio_ssi_reg(SSI_SEL_CTRL)));
}

int ssi_force_reset_aon(void)
{
    if ((ssi_gpio_valid() == OAL_FALSE)) {
        ps_print_info("reset aon, gpio ssi don't support\n");
        return -EINVAL;
    }

    ps_print_info("ssi_force_reset_aon");
    ssi_force_dereset_reg();

    return 0;
}

int32_t test_hd_ssi_write(void)
{
    int32_t i;

    if (ssi_gpio_valid() == OAL_FALSE) {
        ps_print_err("gpio ssi don't support\n");
        return BOARD_FAIL;
    }

    if (ssi_switch_clk(SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        ps_print_err("set ssi clk fail\n");
        return BOARD_FAIL;
    }

    for (i = 0; i < sizeof(g_ht_cnt) / sizeof(ht_test_t); i++) {
        if (ssi_write32(g_ht_cnt[i].add, g_ht_cnt[i].data) != 0) {
            ps_print_err("error: ssi write fail s_addr:0x%x s_data:0x%x\n", g_ht_cnt[i].add, g_ht_cnt[i].data);
        }
    }

    /* reset clk */
    if (ssi_switch_clk(SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        ps_print_err("set ssi clk fail\n");
        return BOARD_FAIL;
    }

    ps_print_err("ALL reg finish---------------------");
    return 0;
}

int32_t ssi_single_write(int32_t addr, int16_t data)
{
    if (ssi_gpio_valid() == OAL_FALSE) {
        ps_print_err("gpio ssi don't support\n");
        return BOARD_FAIL;
    }

    if (ssi_switch_clk(SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        ps_print_err("set ssi clk fail\n");
        return BOARD_FAIL;
    }

    /* set wcpu wait */
    if (ssi_write32(addr, data) != BOARD_SUCC) {
        return BOARD_FAIL;
    }

    /* reset clk */
    if (ssi_switch_clk(SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        ps_print_err("set ssi clk fail\n");
        return BOARD_FAIL;
    }

    return 0;
}

int32_t ssi_single_read(int32_t addr)
{
    int32_t ret;

    if (ssi_gpio_valid() == OAL_FALSE) {
        ps_print_err("gpio ssi don't support\n");
        return BOARD_FAIL;
    }

    if (ssi_switch_clk(SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        ps_print_err("set ssi clk fail\n");
        return BOARD_FAIL;
    }

    ret = ssi_read32(addr);
    /* reset clk */
    if (ssi_switch_clk(SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        ps_print_err("set ssi clk fail\n");
        return BOARD_FAIL;
    }

    return ret;
}

int ssi_check_device_isalive(void)
{
    int i;
    uint32_t reg;
    const uint32_t check_times = 2;

    for (i = 0; i < check_times; i++) {
        if (get_hi110x_subchip_type() == BOARD_VERSION_HI1106) {
            reg = (uint32_t)ssi_read32(0x40000000);
        } else {
            reg = (uint32_t)ssi_read32(0x50000000);
        }

        if (reg == 0x101) {
            ps_print_info("reg is 0x%x\n", reg);
            break;
        }
    }

    if (i == check_times) {
        ps_print_info("ssi is fail, gpio-ssi did't support, reg=0x%x\n", reg);
        return -1;
    }
    return 0;
}

int ssi_read_reg_info(ssi_reg_info *pst_reg_info, void *buf, int32_t size, int32_t is_file)
{
    int32_t is_atomic = 0;
    uint32_t realloc = 0;
    int32_t buf_len;
    int32_t is_logfile;
    char filename[FILENAME_MAX_LEN];

    is_logfile = is_file;
    buf_len = size;
    memset_s(filename, sizeof(filename), 0, sizeof(filename));

    if (ssi_read_reg_prep(pst_reg_info, &is_logfile, &is_atomic, filename, sizeof(filename)) < 0) {
        return -OAL_EFAIL;
    }

    if (ssi_check_device_isalive() < 0) {
        ps_print_info("gpio-ssi maybe dead before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
        return -OAL_EFAIL;
    }

    if (buf == NULL) {
        buf = ssi_malloc_memory(pst_reg_info->len, is_atomic);
        if (buf == NULL) {
            ps_print_info("alloc mem failed before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
            return -OAL_ENOMEM;
        }
        buf_len = pst_reg_info->len;
        realloc = 1;
    }

    ps_print_info("dump reg info 0x%x:%u, buf len:%u \n", pst_reg_info->base_addr, pst_reg_info->len, buf_len);

    if (ssi_read_segs_reg(filename, is_logfile, pst_reg_info, buf, buf_len) < 0) {
        goto read_reg_fail;
    }

    if (realloc) {
        ssi_free_memory(buf, is_atomic);
    }

    return 0;

read_reg_fail:
    if (realloc) {
        ssi_free_memory(buf, is_atomic);
    }

    return -OAL_EFAIL;
}

int ssi_read_reg_info_test(uint32_t base_addr, uint32_t len, uint32_t is_logfile, uint32_t rw_mode)
{
    int ret;
    ssi_reg_info reg_info;

    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -OAL_EBUSY;
    }
    if ((!g_ssi_dfr_bypass) &&
        (oal_work_is_busy(&pst_exception_data->wifi_excp_worker) ||
        oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker) ||
        (atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE))) {
        ps_print_err("dfr is doing ,not do ssi read\n");
        return -OAL_EBUSY;
    }

    memset_s(&reg_info, sizeof(reg_info), 0, sizeof(reg_info));

    reg_info.base_addr = base_addr;
    reg_info.len = len;
    reg_info.rw_mod = rw_mode;

    if (ssi_gpio_valid() == OAL_FALSE) {
        ps_print_info("gpio ssi don't support, check dts\n");
        return -1;
    }

    /* get ssi lock */
    if (ssi_try_lock()) {
        ps_print_info("ssi is locked, request return\n");
        return -OAL_EFAIL;
    }

    ssi_read16(gpio_ssi_reg(SSI_SSI_CTRL));
    ssi_read16(gpio_ssi_reg(SSI_SEL_CTRL));

    ssi_switch_clk(SSI_AON_CLKSEL_SSI);

    ret = ssi_read_device_arm_register(1);
    if (ret) {
        goto ssi_fail;
    }

    ps_print_info("ssi is ok, glb_ctrl is ready\n");

    ret = ssi_check_device_isalive();
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_read_reg_info(&reg_info, NULL, 0, is_logfile);
    if (ret) {
        goto ssi_fail;
    }

    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    ssi_unlock();

    return 0;

ssi_fail:
    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    ssi_unlock();
    return ret;
}

int ssi_read_reg_info_arry(ssi_reg_info **pst_reg_info, uint32_t reg_nums, int32_t is_logfile)
{
    int ret;
    int i;

    if (oal_unlikely(pst_reg_info == NULL)) {
        return -OAL_EFAIL;
    }

    for (i = 0; i < reg_nums; i++) {
        ret = ssi_read_reg_info(pst_reg_info[i], NULL, 0, is_logfile);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

OAL_STATIC int ssi_dump_device_regs_check_condition(unsigned long long module_set)
{
    hi110x_release_vtype vtype = hi110x_get_release_type();
    struct st_exception_info *pst_exception_data = NULL;

    if (ssi_gpio_valid() == OAL_FALSE) {
#ifndef _PRE_HI_DRV_GPIO
        ps_print_err("gpio ssi don't support, check dts\n");
        return OAL_FALSE;
#endif
    }

    if (g_ssi_dump_enable == 0) {
        ps_print_info("ssi dump is disabled\n");
        return OAL_FALSE;
    }

    /* 系统crash后强行dump,系统正常时user版本受控 */
    if ((vtype == HI110X_VTYPE_RELEASE) && (g_hi11xx_kernel_crash == 0)) {
        /* user build, limit the ssi dump */
        if (!oal_print_rate_limit(30 * PRINT_RATE_SECOND)) { /* 30s打印一次 */
            /* print limit */
            module_set = 0;
            ps_print_err("ssi dump print limit in release mode\n");
        }
    }

    if (module_set == 0) {
        ps_print_err("ssi dump regs bypass\n");
        return OAL_FALSE;
    }

    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return OAL_FALSE;
    }
    if ((!g_ssi_dfr_bypass) &&
        (oal_work_is_busy(&pst_exception_data->wifi_excp_worker) ||
        oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker) ||
        (atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE))) {
        ps_print_err("dfr is doing ,not do ssi read\n");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

static void ssi_module_set_parm_config(unsigned long long module_set)
{
    if (module_set & SSI_MODULE_MASK_AON_CUT) {
        module_set &= ~SSI_MODULE_MASK_AON;
    }

    if (module_set & SSI_MODULE_MASK_PCIE_CUT) {
        module_set &= ~(SSI_MODULE_MASK_PCIE_CFG | SSI_MODULE_MASK_PCIE_DBI);
    }
}

/* Try to dump all reg,
  ssi used to debug, we should */
int ssi_dump_device_regs(unsigned long long module_set)
{
    int ret;

    if (ssi_dump_device_regs_check_condition(module_set) != OAL_TRUE) {
        return -OAL_EBUSY;
    }

    /* get ssi lock */
    if (ssi_try_lock()) {
        ps_print_info("ssi is locked, request return\n");
        return -OAL_EBUSY;
    }

    if (gpio_get_value(g_st_board_info.power_on_enable) == 0) {
        ps_print_info("110x power off,ssi return,power_on=%d\n", g_st_board_info.power_on_enable);
        ssi_unlock();
        return -OAL_ENODEV;
    }

    ssi_module_set_parm_config(module_set);

    declare_dft_trace_key_info("ssi_dump_device_regs", OAL_DFT_TRACE_FAIL);

    ps_print_info("module_set=0x%llx\n", module_set);
    ret = ssi_device_regs_dump(module_set);

    ssi_unlock();

    return ret;
}

/* Try to dump all reg,
  dft for soc debug */
int ssi_dump_err_regs(ssi_err_type etype)
{
    return ssi_dump_err_reg(etype);
}
#endif /* #ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG */

