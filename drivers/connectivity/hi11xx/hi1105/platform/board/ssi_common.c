

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG

/* 头文件包含 */
#include "ssi_common.h"

#include "plat_debug.h"
#include "plat_firmware.h"
#include "board.h"
#include "ssi_hi1103.h"
#include "oal_kernel_file.h"

#define HI110X_SSI_CLK_GPIO_NAME  "hi110x ssi clk"
#define HI110X_SSI_DATA_GPIO_NAME "hi110x ssi data"
#define INTERVAL_TIME             10
#define SSI_DATA_LEN              16

#define SSI_DELAY(x) ndelay(x)

typedef struct ht_test_s {
    int32 add;
    int32 data;
} ht_test_t;

ht_test_t ht_cnt[] = {
    { 0x50000314, 0x0D00 },
    { 0x50002724, 0x0022 },
    { 0x50002720, 0x0033 },
};

int hi11xx_kernel_crash = 0;
int ssi_is_logfile = 0;
ssi_trans_test_st ssi_test_st = {0};


static OAL_DEFINE_SPINLOCK(g_ssi_lock);
static oal_uint32 ssi_lock_state = 0x0;
static char str_gpio_ssi_dump_path[100] = HISI_TOP_LOG_DIR "/wifi/memdump";
static int ssi_dfr_bypass = 0;


#ifdef _PRE_HI_DRV_GPIO
static uint32 ssi_clk_etc = SSI_CLK_HISI_GPIO;                /*模拟ssi时钟的GPIO管脚号*/
static uint32 ssi_data_etc = SSI_DATA_HISI_GPIO;              /*模拟ssi数据线的GPIO管脚号*/
#else
static uint32 ssi_clk_etc = 0;              /* 模拟ssi时钟的GPIO管脚号 */
static uint32 ssi_data_etc = 0;             /* 模拟ssi数据线的GPIO管脚号 */
#endif
static uint16 ssi_base_etc = 0x8000;        /* ssi基址 */
static uint32 interval_etc = INTERVAL_TIME; /* GPIO拉出来的波形保持时间，单位us */
static uint32 delay_etc = 5;

static ssi_reg_info ssi_master_reg_full = { 0x38, 0x28, SSI_RW_SSI_MOD };

/* ssi 工作时必须切换ssi clock, 此时aon会受到影响，BCPU/WCPU 有可能异常，慎用! */
static int32 ssi_try_lock(void)
{
    oal_ulong flags;
    oal_spin_lock_irq_save(&g_ssi_lock, &flags);
    if (ssi_lock_state) {
        /* lock failed */
        oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
        return 1;
    }
    ssi_lock_state = 1;
    oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
    return 0;
}

static int32 ssi_unlock(void)
{
    oal_ulong flags;
    oal_spin_lock_irq_save(&g_ssi_lock, &flags);
    ssi_lock_state = 0;
    oal_spin_unlock_irq_restore(&g_ssi_lock, &flags);
    return 0;
}

int32 wait_for_ssi_idle_timeout(int32 mstimeout)
{
    int32 can_sleep = 0;
    int32 timeout = mstimeout;
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
            PS_PRINT_ERR("wait for ssi timeout:%dms\n", mstimeout);
            return 0;
        }
    }
    ssi_unlock();
    return timeout;
}

int ssi_set_gpio_pins(int32 clk, int32 data)
{
    board_info_etc.ssi_gpio_clk = clk;
    board_info_etc.ssi_gpio_data = data;
    PS_PRINT_INFO("set ssi gpio clk:%d , gpio data:%d\n", clk, data);
    return 0;
}

int32 ssi_show_setup_etc(void)
{
    PS_PRINT_INFO("clk=%d, data=%d, interval=%d us, ssi base=0x%x, r/w delay=%d cycle\n",
                  ssi_clk_etc, ssi_data_etc, interval_etc, ssi_base_etc, delay_etc);
    return BOARD_SUCC;
}

int32 ssi_setup_etc(uint32 interval, uint32 delay, uint16 ssi_base)
{
    interval_etc = interval;
    delay_etc = delay;
    ssi_base_etc = ssi_base;

    return BOARD_SUCC;
}

#ifdef _PRE_HI_DRV_GPIO
static int32 hitv_ssi_request_gpio_etc(uint32 clk, uint32 data)
{
    //Oscar的v811主芯片不支持Linux的DTS机制，目前还在适配标准的内核GPIO，等小海思提供接口后该函数再另行整改
    /*
    {0xF8A21218, 0x00000000}, //RegName:ctrl57; PadName:SD_D2; RegValue: 0x0--SD_D2,0x1--GPIOPWM6_7,0x2--TSI2_D7,
                                //0x3--NA,0x4--NA,0x5--NA,0x6--NA,0x7--NA
    {0xF8A2121C, 0x00000000}, //RegName:ctrl58; PadName:SD_D3; RegValue: 0x0--SD_D3,0x1--GPIOPWM7_0,0x2--TSI2_D2,
    */
    void __iomem * gpio_mux_addr = ioremap_nocache(GPIO_MUX_REG_ADDR, GPIO_MUX_REG_VALUE);
    if (gpio_mux_addr != NULL) {
        writel(1, gpio_mux_addr);
        writel(1, (void __iomem *)(uintptr_t)((unsigned long)(uintptr_t)gpio_mux_addr + GPIO_MUX_REG_OFFSET));
        iounmap(gpio_mux_addr);
    }

    ssi_clk_etc = SSI_CLK_HISI_GPIO;
    ssi_data_etc = SSI_DATA_HISI_GPIO;
    hitv_gpio_request(ssi_clk_etc, 0);
    hitv_gpio_direction_output(ssi_clk_etc, GPIO_LOWLEVEL);
    hitv_gpio_request(ssi_data_etc, 0);
    hitv_gpio_direction_output(ssi_data_etc, GPIO_LOWLEVEL);

    return BOARD_SUCC;
}
#endif

int32 ssi_request_gpio_etc(uint32 clk, uint32 data)
{
#ifdef _PRE_HI_DRV_GPIO
    return hitv_ssi_request_gpio_etc(clk, data);
#else
    int32 ret;

    PS_PRINT_DBG("request hi110x ssi GPIO\n");
#ifdef GPIOF_OUT_INIT_LOW
    ret = gpio_request_one(clk, GPIOF_OUT_INIT_LOW, HI110X_SSI_CLK_GPIO_NAME);
    if (ret) {
        PS_PRINT_ERR("%s gpio_request_one failed ret=%d\n", HI110X_SSI_CLK_GPIO_NAME, ret);
        goto err_get_ssi_clk_gpio;
    }

    ssi_clk_etc = clk;

    ret = gpio_request_one(data, GPIOF_OUT_INIT_LOW, HI110X_SSI_DATA_GPIO_NAME);
    if (ret) {
        PS_PRINT_ERR("%s gpio_request_one failed ret=%d\n", HI110X_SSI_DATA_GPIO_NAME, ret);
        goto err_get_ssi_data_gpio;
    }
#else
    ret = gpio_request(clk, HI110X_SSI_CLK_GPIO_NAME);
    if (ret) {
        PS_PRINT_ERR("%s gpio_request failed  ret=%d\n", HI110X_SSI_CLK_GPIO_NAME, ret);
        goto err_get_ssi_clk_gpio;
    }

    gpio_direction_output(clk, 0);

    ret = gpio_request(data, HI110X_SSI_DATA_GPIO_NAME);
    if (ret) {
        PS_PRINT_ERR("%s gpio_request failed  ret=%d\n", HI110X_SSI_DATA_GPIO_NAME, ret);
        goto err_get_ssi_data_gpio;
    }

    gpio_direction_output(data, 0);
#endif
    ssi_data_etc = data;

    return BOARD_SUCC;

err_get_ssi_data_gpio:
    gpio_free(clk);
    ssi_clk_etc = 0;
err_get_ssi_clk_gpio:

    CHR_EXCEPTION_REPORT(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_GPIO, CHR_PLAT_DRV_ERROR_SSI_GPIO);

    return ret;
#endif
}

int32 ssi_free_gpio_etc(void)
{
    PS_PRINT_DBG("free hi110x ssi GPIO\n");

    if (ssi_clk_etc != 0) {
#ifndef _PRE_HI_DRV_GPIO
        gpio_free(ssi_clk_etc);
#endif
        ssi_clk_etc = 0;
    }

    if (ssi_data_etc != 0) {
#ifndef _PRE_HI_DRV_GPIO
        gpio_free(ssi_data_etc);
#endif
        ssi_data_etc = 0;
    }

    return BOARD_SUCC;
}

void ssi_clk_output_etc(void)
{
    oal_gpio_direction_output(ssi_clk_etc, GPIO_LOWLEVEL);
    SSI_DELAY(interval_etc);
    oal_gpio_direction_output(ssi_clk_etc, GPIO_HIGHLEVEL);
}

void ssi_data_output_etc(uint16 data)
{
    SSI_DELAY(5);
    if (data) {
        oal_gpio_direction_output(ssi_data_etc, GPIO_HIGHLEVEL);
    } else {
        oal_gpio_direction_output(ssi_data_etc, GPIO_LOWLEVEL);
    }

    SSI_DELAY(interval_etc);
}

int32 ssi_write_data_etc(uint16 addr, uint16 value)
{
    uint16 tx;
    uint32 i;

    for (i = 0; i < delay_etc; i++) {
        ssi_clk_output_etc();
        ssi_data_output_etc(0);
    }

    /* 发送SYNC位 */
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 指示本次操作为写，高读低写 */
    PS_PRINT_DBG("tx r/w->w\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(0);

    /* 发送地址 */
    PS_PRINT_DBG("write addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output_etc();
        ssi_data_output_etc(tx);
    }

    /* 发送数据 */
    PS_PRINT_DBG("write value:0x%x\n", value);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (value >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx data bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output_etc();
        ssi_data_output_etc(tx);
    }

    /* 数据发送完成以后，保持delay个周期的0 */
    PS_PRINT_DBG("ssi write:finish, delay %d cycle\n", delay_etc);
    for (i = 0; i < delay_etc; i++) {
        ssi_clk_output_etc();
        ssi_data_output_etc(0);
    }

    return BOARD_SUCC;
}

uint16 ssi_read_data_etc(uint16 addr)
{
#define SSI_READ_RETTY 1000
    uint16 tx;
    uint32 i;
    uint32 retry = 0;
    uint16 rx;
    uint16 data = 0;

    for (i = 0; i < delay_etc; i++) {
        ssi_clk_output_etc();
        ssi_data_output_etc(0);
    }

    /* 发送SYNC位 */
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 指示本次操作为读，高读低写 */
    PS_PRINT_DBG("tx r/w->r\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 发送地址 */
    PS_PRINT_DBG("read addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output_etc();
        ssi_data_output_etc(tx);
    }

    /* 延迟一个clk，否则上一个数据只保持了半个时钟周期 */
    ssi_clk_output_etc();

    /* 设置data线GPIO为输入，准备读取数据 */
    oal_gpio_direction_input(ssi_data_etc);

    PS_PRINT_DBG("data in mod, current gpio level is %d\n", gpio_get_value(ssi_data_etc));

    /* 读取SYNC同步位 */
    do {
        ssi_clk_output_etc();
        SSI_DELAY(interval_etc);
        if (oal_gpio_get_value(ssi_data_etc)) {
            PS_PRINT_DBG("read data sync bit ok, retry=%d\n", retry);
            break;
        }
        retry++;
    } while (retry != SSI_READ_RETTY);

    if (retry == SSI_READ_RETTY) {
        PS_PRINT_ERR("ssi read sync bit timeout\n");
        ssi_data_output_etc(0);
        return data;
    }

    for (i = 0; i < SSI_DATA_LEN; i++) {
        ssi_clk_output_etc();
        SSI_DELAY(interval_etc);
        rx = oal_gpio_get_value(ssi_data_etc);
        PS_PRINT_DBG("rx data bit %d:%d\n", SSI_DATA_LEN - i - 1, rx);
        data = data | (rx << (SSI_DATA_LEN - i - 1));
    }

    /* 恢复data线GPIO为输出，并输出0 */
    ssi_data_output_etc(0);

    return data;
}

int32 ssi_write16_etc(uint16 addr, uint16 value)
{
#define write_retry 3
    uint32 retry = 0;
    uint16 read_v;

    do {
        ssi_write_data_etc(addr, value);
        read_v = ssi_read_data_etc(addr);
        if (value == read_v) {
            PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);
            return BOARD_SUCC;
        }
        retry++;
    } while (retry < write_retry);

    PS_PRINT_ERR("ssi write: 0x%x=0x%x ,read=0x%x fail\n", addr, value, read_v);

    return BOARD_FAIL;
}

uint16 ssi_read16_etc(uint16 addr)
{
    uint16 data;

    data = ssi_read_data_etc(addr);

    PS_PRINT_SUC("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

int32 ssi_write32_etc(uint32 addr, uint16 value)
{
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low = (addr & 0xffff) >> 1;

    /* 往基地址写地址的高16位 */
    if (ssi_write16_etc(ssi_base_etc, addr_half_word_high) < 0) {
        PS_PRINT_ERR("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    /* 低地址写实际要写入的value */
    if (ssi_write16_etc(addr_half_word_low, value) < 0) {
        PS_PRINT_ERR("ssi write: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);

    return BOARD_SUCC;
}

int32 ssi_read32_etc(uint32 addr)
{
    uint16 data;
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low = (addr & 0xffff) >> 1;

    if (ssi_write16_etc(ssi_base_etc, addr_half_word_high) < 0) {
        PS_PRINT_ERR("ssi read 0x%x fail\n", addr);
        return BOARD_FAIL;
    }

    data = ssi_read_data_etc(addr_half_word_low);

    PS_PRINT_DBG("ssi read: 0x%x=0x%x\n", addr, data);

    return data;
}

int32 ssi_read_data16(uint16 addr, uint16 *value)
{
#define SSI_READ_RETTY 1000
    uint16 tx;
    uint32 i;
    uint32 retry = 0;
    uint16 rx;
    uint16 data = 0;

    for (i = 0; i < delay_etc; i++) {
        ssi_clk_output_etc();
        ssi_data_output_etc(0);
    }

    /* 发送SYNC位 */
    PS_PRINT_DBG("tx sync bit\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 指示本次操作为读，高读低写 */
    PS_PRINT_DBG("tx r/w->r\n");
    ssi_clk_output_etc();
    ssi_data_output_etc(1);

    /* 发送地址 */
    PS_PRINT_DBG("read addr:0x%x\n", addr);
    for (i = 0; i < SSI_DATA_LEN; i++) {
        tx = (addr >> (SSI_DATA_LEN - i - 1)) & 0x0001;
        PS_PRINT_DBG("tx addr bit %d:%d\n", SSI_DATA_LEN - i - 1, tx);
        ssi_clk_output_etc();
        ssi_data_output_etc(tx);
    }

    /* 延迟一个clk，否则上一个数据只保持了半个时钟周期 */
    ssi_clk_output_etc();

    /* 设置data线GPIO为输入，准备读取数据 */
    oal_gpio_direction_input(ssi_data_etc);

    PS_PRINT_DBG("data in mod, current gpio level is %d\n", gpio_get_value(ssi_data_etc));

    /* 读取SYNC同步位 */
    do {
        ssi_clk_output_etc();
        SSI_DELAY(interval_etc);
        if (oal_gpio_get_value(ssi_data_etc)) {
            PS_PRINT_DBG("read data sync bit ok, retry=%d\n", retry);
            break;
        }
        retry++;
    } while (retry != SSI_READ_RETTY);

    if (retry == SSI_READ_RETTY) {
        PS_PRINT_ERR("ssi read sync bit timeout\n");
        ssi_data_output_etc(0);
        return -OAL_EFAIL;
    }

    for (i = 0; i < SSI_DATA_LEN; i++) {
        ssi_clk_output_etc();
        SSI_DELAY(interval_etc);
        rx = oal_gpio_get_value(ssi_data_etc);
        PS_PRINT_DBG("rx data bit %d:%d\n", SSI_DATA_LEN - i - 1, rx);
        data = data | (rx << (SSI_DATA_LEN - i - 1));
    }

    /* 恢复data线GPIO为输出，并输出0 */
    ssi_data_output_etc(0);

    *value = data;

    return OAL_SUCC;
}

/* 32bits address, 32bits value */
int32 ssi_read_value16(uint32 addr, uint16 *value, int16 last_high_addr)
{
    int32 ret;
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low = (addr & 0xffff) >> 1;

    if (last_high_addr != addr_half_word_high) {
        if (ssi_write16_etc(ssi_base_etc, addr_half_word_high) < 0) {
            PS_PRINT_ERR("ssi read 0x%x fail\n", addr);
            return BOARD_FAIL;
        }
    }

    ret = ssi_read_data16(addr_half_word_low, value);

    PS_PRINT_DBG("ssi read: 0x%x=0x%x\n", addr, *value);

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
int32 ssi_read_value32(uint32 addr, uint32 *value, int16 last_high_addr)
{
    int32 ret;
    uint16 reg;

    ret = ssi_read_value16(addr, &reg, last_high_addr);
    if (ret) {
        PS_PRINT_ERR("read addr 0x%x low 16 bit failed, ret=%d\n", addr, ret);
        return ret;
    }
    *value = (uint32)reg;

    /* 读32位地址的高16位 */
    ret = ssi_read_value16(addr + 0x2, &reg, (addr >> 16));
    if (ret) {
        PS_PRINT_ERR("read addr 0x%x high 16 bit failed, ret=%d\n", addr, ret);
        return ret;
    }

    *value = ((reg << 16) | *value);

    return OAL_SUCC;
}

int32 ssi_read_value32_test(uint32 addr)
{
    int32 ret;
    uint32 value = 0xffffffff;

    ret = ssi_read_value32(addr, &value, (((addr >> 16) & 0xffff) + 1));
    if (ret) {
        PS_PRINT_ERR("ssi_read_value32 ret=%d\n", ret);
        return 0xffffffff;
    }

    return value;
}

int32 ssi_write_value32(uint32 addr, uint32 value)
{
    uint16 addr_half_word_high;
    uint16 addr_half_word_low;
    uint16 addr_half_word_low_incr;

    addr_half_word_high = (addr >> 16) & 0xffff;
    addr_half_word_low = (addr & 0xffff) >> 1;
    addr_half_word_low_incr = ((addr + 2) & 0xffff) >> 1;

    /* 往基地址写地址的高16位 */
    if (ssi_write_data_etc(ssi_base_etc, addr_half_word_high) < 0) {
        PS_PRINT_ERR("ssi write high addr: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    /* 低地址写实际要写入的value */
    if (ssi_write_data_etc(addr_half_word_low, value & 0xffff) < 0) {
        PS_PRINT_ERR("ssi write low value: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    if (ssi_write_data_etc(addr_half_word_low_incr, (value >> 16) & 0xffff) < 0) {
        PS_PRINT_ERR("ssi write high value: 0x%x=0x%x fail\n", addr, value);
        return BOARD_FAIL;
    }

    PS_PRINT_DBG("ssi write: 0x%x=0x%x succ\n", addr, value);

    return BOARD_SUCC;
}

/* 16bits/32bits switch mode */
static int32 ssi_switch_ahb_mode(int32 is_32bit_mode)
{
    return ssi_write16_etc(0x8001, !!is_32bit_mode);
}

static int32 ssi_clear_ahb_highaddr(void)
{
    return ssi_write16_etc(ssi_base_etc, 0x0);
}

static int ssi_switch_to_tcxo_clk(void)
{
    int ret;

    ret = ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_TCXO);
    PS_PRINT_INFO("switch tcxo clk %s\n", (ret == BOARD_SUCC) ? "ok" : "failed");

    return ret;
}

static int ssi_switch_to_ssi_clk(void)
{
    int ret;

    ret = ssi_write16_etc(GPIO_SSI_REG(SSI_AON_CLKSEL), SSI_AON_CLKSEL_SSI);
    PS_PRINT_INFO("switch ssi clk %s\n", (ret == BOARD_SUCC) ? "ok" : "failed");

    return ret;
}

static int ssi_setup_clk(uint32 clk_type)
{
    if (clk_type == SSI_AON_CLKSEL_TCXO) {
        return ssi_switch_to_tcxo_clk();
    } else if (clk_type == SSI_AON_CLKSEL_SSI) {
        return ssi_switch_to_ssi_clk();
    } else {
        PS_PRINT_ERR("ssi clk_type %d is error\n", clk_type);
        return BOARD_FAIL;
    }
}

static int ssi_is_alive(void)
{
    uint16 value;

    value = ssi_read16_etc(GPIO_SSI_REG(SSI_SYS_CTL_ID));
    if (value == SSI_ALIVE) {
        return 1;
    } else {
        return 0;
    }
}

static int ssi_read_single_seg_reg(ssi_reg_info *pst_reg_info, void *buf, int32 buf_len,
                                        int seg_index, int32 seg_size)
{
    int ret;
    int step = 4;
    int j, k;
    uint32 reg;
    uint16 reg16;
    uint32 ssi_address;
    uint16 ssi_master_address;
    uint16 last_high_addr = 0x0;
    const uint32 ul_max_ssi_read_retry_times = 3;

    for (j = 0; j < seg_size; j += step) {
        ssi_address = pst_reg_info->base_addr + seg_index * buf_len + j;

        for (k = 0; k < ul_max_ssi_read_retry_times; k++) {
            reg = 0x0;
            reg16 = 0x0;
            if (pst_reg_info->rw_mod == SSI_RW_SSI_MOD) {
                ssi_master_address = (uint16)GPIO_SSI_REG(ssi_address);
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
            PS_PRINT_ERR("ssi read address 0x%x failed, retry %d times", ssi_address, k);
            goto fail_read_reg;
        }

        last_high_addr = (ssi_address >> 16);
        oal_writel(reg, buf + j);
    }

    return OAL_SUCC;

fail_read_reg:
    if (ssi_address > pst_reg_info->base_addr) {
#ifdef CONFIG_PRINTK
        /* print the read buf before errors */
        print_hex_dump(KERN_INFO, "gpio-ssi: ", DUMP_PREFIX_OFFSET, 32, 4,
                       buf, OAL_MIN(buf_len, ssi_address - pst_reg_info->base_addr), false); /* 内核函数固定传参 */
#endif
    }

    return -OAL_EFAIL;
}

static void ssi_ahb_mode_set(uint32 mode, int32 flag)
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

static int ssi_read_segs_reg(char *filename, oal_int32 is_file, ssi_reg_info *pst_reg_info,
                                 void *buf, int32 buf_len)
{
    int i, seg_size, seg_nums, left_size;
    int ret = -OAL_EFAIL;
    mm_segment_t fs;
    oal_int32 is_logfile;
    OS_KERNEL_FILE_STRU *fp = NULL;

    is_logfile = is_file;

    fs = get_fs();
    set_fs(KERNEL_DS);

    if (is_logfile) {
        fp = filp_open(filename, O_RDWR | O_CREAT, 0644);
        if (OAL_IS_ERR_OR_NULL(fp)) {
            PS_PRINT_INFO("open file %s failed ret=%ld\n", filename, PTR_ERR(fp));
            is_logfile = 0;
        } else {
            PS_PRINT_INFO("open file %s succ\n", filename);
            vfs_llseek(fp, 0, SEEK_SET);
        }
    }

    ssi_clear_ahb_highaddr();

    ssi_ahb_mode_set(pst_reg_info->rw_mod, SSI_AHB_MODE_SET_START);

retry:

    if (buf_len == 0) {
        PS_PRINT_ERR("buf_len is zero\n");
        return -OAL_EFAIL;
    }

    seg_nums = (pst_reg_info->len - 1 / buf_len) + 1;
    left_size = pst_reg_info->len;

    for (i = 0; i < seg_nums; i++) {
        seg_size = OAL_MIN(left_size, buf_len);
        ret = ssi_read_single_seg_reg(pst_reg_info, buf, buf_len, i, seg_size);
        if (ret < 0) {
            break;
        }
        left_size -= seg_size;

        if (is_logfile) {
            ret = oal_kernel_file_write_etc(fp, buf, seg_size);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
            vfs_fsync(fp, 0);
#else
            vfs_fsync(fp, fp->f_path.dentry, 0);
#endif
            if (ret != seg_size) {
                PS_PRINT_ERR("ssi print file failed, request %d, write %d actual\n", seg_size, ret);
                is_logfile = 0;
                filp_close(fp, NULL);
                goto retry;
            }
        } else {
#ifdef CONFIG_PRINTK
            /* print to kenrel msg */
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

static void *ssi_malloc_memory(uint32 size, oal_int32 is_atomic)
{
    void *buf = NULL;

    if (is_atomic) {
        buf = kmalloc(size, GFP_ATOMIC);
    } else {
        buf = OS_VMALLOC_GFP(size);
    }

    return buf;
}

static void ssi_free_memory(const void *buf, oal_int32 is_atomic)
{
    if (buf == NULL) {
        return;
    }

    if (is_atomic) {
        kfree(buf);
    } else {
        OS_MEM_VFREE(buf);
    }
}

static int ssi_read_reg_prep(ssi_reg_info *pst_reg_info, oal_int32 *is_logfile, oal_int32 *is_atomic,
                      char *filename, oal_uint32 len)
{
    struct timeval tv;
    struct rtc_time tm = {0};
    int ret = OAL_SUCC;

    if (oal_in_interrupt() || oal_in_atomic() || irqs_disabled()) {
        *is_logfile = 0;
        *is_atomic = 1;
    }

    if (*is_logfile) {
        do_gettimeofday(&tv);
        rtc_time_to_tm(tv.tv_sec, &tm);
        ret = snprintf_s(filename, len, len - 1, "%s/gpio_ssi_%08x_%08x_%04d%02d%02d%02d%02d%02d.bin",
                         str_gpio_ssi_dump_path,
                         pst_reg_info->base_addr,
                         pst_reg_info->base_addr + pst_reg_info->len - 1,
                         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                         tm.tm_hour, tm.tm_min, tm.tm_sec); /* 转换成当前时间 */
        if (ret < 0) {
            PS_PRINT_ERR("log str format err line[%d]\n", __LINE__);
            return -OAL_EFAIL;
        }
    }
    return ret;
}

static int ssi_read_master_regs(ssi_reg_info *pst_reg_info, void *buf, int32 size, oal_int32 is_file)
{
    oal_int32 is_atomic = 0;
    uint32 realloc = 0;
    int32 buf_len;
    oal_int32 is_logfile;
    const uint32 ul_filename_len = 200;
    char filename[ul_filename_len];

    is_logfile = is_file;
    buf_len = size;
    memset_s(filename, sizeof(filename), 0, sizeof(filename));

    if (ssi_read_reg_prep(pst_reg_info, &is_logfile, &is_atomic, filename, sizeof(filename)) < 0) {
        return -OAL_EFAIL;
    }

    if (buf == NULL) {
        buf = ssi_malloc_memory(pst_reg_info->len, is_atomic);
        if (buf == NULL) {
            PS_PRINT_INFO("alloc mem failed before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
            return -OAL_ENOMEM;
        }
        buf_len = pst_reg_info->len;
        realloc = 1;
    }

    PS_PRINT_INFO("dump reg info 0x%x:%u, buf len:%u \n", pst_reg_info->base_addr, pst_reg_info->len, buf_len);

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

static int ssi_setup_clk_enhance(uint32 clk_type)
{
    uint16 value;

    if (!ssi_is_alive()) {
        PS_PRINT_ERR("ssi is not alive, clk type %d\n", clk_type);
        return BOARD_FAIL;
    }

    if (clk_type == SSI_AON_CLKSEL_TCXO) {
        return ssi_setup_clk(clk_type);
    }

    /* switch clk to ssi clk */
    ssi_read_master_regs(&ssi_master_reg_full, NULL, 0, ssi_is_logfile);
    /* 判断TCXO是否在位,TCXO不在位需要切换到ssi clk */
    /* bit[5:3] b_cur_sts,bit[2:0] w_cur_sts,中有一个为0x3，表示TCXO在位 */
    value = ssi_read16_etc(GPIO_SSI_REG(SSI_RPT_STS_L));
    if (((value & 0x3) != 0x3) && ((value & 0x18) != 0x18)) {
        PS_PRINT_INFO("ssi tcxo not available, switch clk to ssi\n");
        return ssi_setup_clk(clk_type);
    }

    /* TCXO 在位，读取aon寄存器，读不出来需要切换到ssi clk */
    if (ssi_check_device_isalive()) {
        PS_PRINT_INFO("ssi read aon fail, switch clk to ssi\n");
        return ssi_setup_clk(clk_type);
    }

    return BOARD_SUCC;
}

int ssi_clk_auto_switch_is_support(void)
{
    /* after hi1105, we use tcxo to replase ssi clk,
       we switch to ssi clk just when tcxo gating */
    int32 chip_type = get_hi110x_subchip_type();

    switch (chip_type) {
        case BOARD_VERSION_HI1105:
            return OAL_TRUE;
        default:
            return OAL_FALSE;
    }
}

int ssi_switch_clk(uint32 clk_type)
{
    if (ssi_clk_auto_switch_is_support() == OAL_TRUE) {
        return ssi_setup_clk_enhance(clk_type);
    } else {
        return ssi_setup_clk(clk_type);
    }
}

oal_void ssi_force_dereset_reg(oal_void)
{
    /* 解复位AON，注意寄存器配置顺序 */
    ssi_write16_etc(GPIO_SSI_REG(SSI_SSI_CTRL), 0x60);
    ssi_write16_etc(GPIO_SSI_REG(SSI_SEL_CTRL), 0x60);
}

oal_void ssi_force_reset_reg(oal_void)
{
    /* 先复位再解复位AON，注意寄存器配置顺序 */
    ssi_write16_etc(GPIO_SSI_REG(SSI_SEL_CTRL), 0x60);
    ssi_write16_etc(GPIO_SSI_REG(SSI_SSI_CTRL), 0x60);
    PS_PRINT_INFO("ssi_ctrl:0x%x sel_ctrl:0x%x\n", ssi_read16_etc(GPIO_SSI_REG(SSI_SSI_CTRL)),
                  ssi_read16_etc(GPIO_SSI_REG(SSI_SEL_CTRL)));
}

int ssi_force_reset_aon(void)
{
    int ret;
    if ((board_info_etc.ssi_gpio_clk == 0) || (board_info_etc.ssi_gpio_data == 0)) {
        PS_PRINT_INFO("reset aon, gpio ssi don't support\n");
        return -1;
    }

    ret = ssi_request_gpio_etc(board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data);
    if (ret) {
        PS_PRINT_INFO("ssi_force_reset_aon request failed:%d, data:%d, ret=%d\n",
                      board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data, ret);
        return ret;
    }

    ssi_force_dereset_reg();

    PS_PRINT_INFO("ssi_force_reset_aon");

    ssi_free_gpio_etc();

    return 0;
}

int32 do_ssi_file_test(ssi_file_st *file_st, ssi_trans_test_st *pst_ssi_test)
{
    uint32 ul_addr;
    OS_KERNEL_FILE_STRU *fp = NULL;
    mm_segment_t fs = {0};
    uint16 data_buf = 0;
    int32 rdlen = 0;
    int32 l_ret = BOARD_FAIL;
    const uint32 ul_count_everytime = 2; /* 表示每次循环读的字节数 */

    if ((pst_ssi_test == NULL) || (file_st == NULL)) {
        return BOARD_FAIL;
    }

    fs = get_fs();
    set_fs(KERNEL_DS);

    fp = filp_open(file_st->file_name, O_RDONLY, 0);
    if (OAL_IS_ERR_OR_NULL(fp)) {
        set_fs(fs);
        fp = NULL;
        PS_PRINT_ERR("filp_open %s fail!!\n", file_st->file_name);
        return -EFAIL;
    }
    ul_addr = file_st->write_addr;
    PS_PRINT_INFO("begin file:%s", file_st->file_name);
    while (1) {
        data_buf = 0;
        rdlen = oal_file_read_ext(fp, fp->f_pos, (uint8 *)&data_buf, ul_count_everytime);

        if (rdlen > 0) {
            fp->f_pos += rdlen;
        } else if (rdlen == 0) {
            PS_PRINT_INFO("file read over:%s!!\n", file_st->file_name);
            break;
        } else {
            PS_PRINT_ERR("file read ERROR:%d!!\n", rdlen);
            goto test_fail;
        }
        l_ret = ssi_write32_etc(ul_addr, data_buf);
        if (l_ret != BOARD_SUCC) {
            PS_PRINT_ERR(" write data error, ul_addr=0x%x, l_ret=%d\n", ul_addr, l_ret);
            goto test_fail;
        }
        pst_ssi_test->trans_len += ul_count_everytime;
        ul_addr += ul_count_everytime;
    }
    filp_close(fp, NULL);
    set_fs(fs);
    fp = NULL;
    PS_PRINT_INFO("%s send finish\n", file_st->file_name);
    return BOARD_SUCC;
test_fail:
    filp_close(fp, NULL);
    set_fs(fs);
    fp = NULL;
    return BOARD_FAIL;
}

int32 test_hd_ssi_write(void)
{
    int32 i;
    if (ssi_request_gpio_etc(SSI_CLK_GPIO, SSI_DATA_GPIO) != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }

    if (ssi_switch_clk(SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    for (i = 0; i < sizeof(ht_cnt) / sizeof(ht_test_t); i++) {
        if (ssi_write32_etc(ht_cnt[i].add, ht_cnt[i].data) != 0) {
            PS_PRINT_ERR("error: ssi write fail s_addr:0x%x s_data:0x%x\n", ht_cnt[i].add, ht_cnt[i].data);
        } else {
        }
    }

    /* reset clk */
    if (ssi_switch_clk(SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    if (ssi_free_gpio_etc() != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }
    PS_PRINT_ERR("ALL reg finish---------------------");
    return 0;
err_exit:
    PS_PRINT_ERR("test reg fail---------------------");
    ssi_free_gpio_etc();
    return BOARD_FAIL;
}

int32 ssi_single_write(int32 addr, int16 data)
{
    if (ssi_request_gpio_etc(SSI_CLK_GPIO, SSI_DATA_GPIO) != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }

    if (ssi_switch_clk(SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    /* set wcpu wait */
    if (ssi_write32_etc(addr, data) != BOARD_SUCC) {
        goto err_exit;
    }
    /* reset clk */
    if (ssi_switch_clk(SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    if (ssi_free_gpio_etc() != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }
    return 0;
err_exit:
    ssi_free_gpio_etc();
    return BOARD_FAIL;
}

int32 ssi_single_read(int32 addr)
{
    int32 ret;
    if (ssi_request_gpio_etc(SSI_CLK_GPIO, SSI_DATA_GPIO) != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }
    if (ssi_switch_clk(SSI_AON_CLKSEL_SSI) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    ret = ssi_read32_etc(addr);
    /* reset clk */
    if (ssi_switch_clk(SSI_AON_CLKSEL_TCXO) != BOARD_SUCC) {
        PS_PRINT_ERR("set ssi clk fail\n");
        goto err_exit;
    }
    if (ssi_free_gpio_etc() != BOARD_SUCC) {
        PS_PRINT_ERR("ssi_request_gpio_etc fail\n");
        return BOARD_FAIL;
    }
    return ret;
err_exit:
    ssi_free_gpio_etc();
    return BOARD_FAIL;
}

int ssi_check_device_isalive(void)
{
    int i;
    uint32 reg;
    const uint32 check_times = 2;

    for (i = 0; i < check_times; i++) {
        reg = (uint32)ssi_read32_etc(0x50000000);
        if (reg == 0x101) {
            PS_PRINT_INFO("reg is 0x%x\n", reg);
            break;
        }
    }

    if (i == check_times) {
        PS_PRINT_INFO("ssi is fail, gpio-ssi did't support, reg=0x%x\n", reg);
        return -1;
    }
    return 0;
}

int ssi_read_reg_info(ssi_reg_info *pst_reg_info, void *buf, int32 size, oal_int32 is_file)
{
    oal_int32 is_atomic = 0;
    uint32 realloc = 0;
    int32 buf_len;
    oal_int32 is_logfile;
    const uint32 ul_filename_len = 200;
    char filename[ul_filename_len];

    is_logfile = is_file;
    buf_len = size;
    memset_s(filename, sizeof(filename), 0, sizeof(filename));

    if (ssi_read_reg_prep(pst_reg_info, &is_logfile, &is_atomic, filename, sizeof(filename)) < 0) {
        return -OAL_EFAIL;
    }

    if (ssi_check_device_isalive() < 0) {
        PS_PRINT_INFO("gpio-ssi maybe dead before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
        return -OAL_EFAIL;
    }

    if (buf == NULL) {
        buf = ssi_malloc_memory(pst_reg_info->len, is_atomic);
        if (buf == NULL) {
            PS_PRINT_INFO("alloc mem failed before read 0x%x:%u\n", pst_reg_info->base_addr, pst_reg_info->len);
            return -OAL_ENOMEM;
        }
        buf_len = pst_reg_info->len;
        realloc = 1;
    }

    PS_PRINT_INFO("dump reg info 0x%x:%u, buf len:%u \n", pst_reg_info->base_addr, pst_reg_info->len, buf_len);

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

int ssi_read_reg_info_test(uint32 base_addr, uint32 len, uint32 is_logfile, uint32 rw_mode)
{
    int ret;
    ssi_reg_info reg_info;

    struct st_exception_info *pst_exception_data = NULL;
    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return -OAL_EBUSY;
    }
    if ((!ssi_dfr_bypass) &&
        (oal_work_is_busy(&pst_exception_data->wifi_excp_worker) ||
         oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker) ||
         (atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE))) {
        PS_PRINT_ERR("dfr is doing ,not do ssi read\n");
        return -OAL_EBUSY;
    }

    memset_s(&reg_info, sizeof(reg_info), 0, sizeof(reg_info));

    reg_info.base_addr = base_addr;
    reg_info.len = len;
    reg_info.rw_mod = rw_mode;

    if ((board_info_etc.ssi_gpio_clk == 0) || (board_info_etc.ssi_gpio_data == 0)) {
        PS_PRINT_INFO("gpio ssi don't support, check dts\n");
        return -1;
    }

    /* get ssi lock */
    if (ssi_try_lock()) {
        PS_PRINT_INFO("ssi is locked, request return\n");
        return -OAL_EFAIL;
    }

    ret = ssi_request_gpio_etc(board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data);
    if (ret) {
        ssi_unlock();
        return ret;
    }

    ssi_read16_etc(GPIO_SSI_REG(SSI_SSI_CTRL));
    ssi_read16_etc(GPIO_SSI_REG(SSI_SEL_CTRL));

    ssi_switch_clk(SSI_AON_CLKSEL_SSI);

    ret = ssi_read_device_arm_register(1);
    if (ret) {
        goto ssi_fail;
    }

    PS_PRINT_INFO("ssi is ok, glb_ctrl is ready\n");

    ret = ssi_check_device_isalive();
    if (ret) {
        goto ssi_fail;
    }

    ret = ssi_read_reg_info(&reg_info, NULL, 0, is_logfile);
    if (ret) {
        goto ssi_fail;
    }

    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);

    ssi_free_gpio_etc();
    ssi_unlock();

    return 0;
ssi_fail:
    ssi_switch_clk(SSI_AON_CLKSEL_TCXO);
    ssi_free_gpio_etc();
    ssi_unlock();
    return ret;
}

int ssi_read_reg_info_arry(ssi_reg_info **pst_reg_info, oal_uint32 reg_nums, oal_int32 is_logfile)
{
    int ret;
    int i;

    if (OAL_UNLIKELY(pst_reg_info == NULL)) {
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

    if ((board_info_etc.ssi_gpio_clk == 0) || (board_info_etc.ssi_gpio_data == 0)) {
#ifndef _PRE_HI_DRV_GPIO
        PS_PRINT_ERR("gpio ssi don't support, check dts\n");
        return OAL_FALSE;
#endif
    }

    /* 系统crash后强行dump,系统正常时user版本受控 */
    if ((vtype == HI110X_VTYPE_RELEASE) && (hi11xx_kernel_crash == 0)) {
        /* user build, limit the ssi dump */
        if (!oal_print_rate_limit(30 * PRINT_RATE_SECOND)) { /* 30s打印一次 */
            /* print limit */
            module_set = 0;
            PS_PRINT_ERR("ssi dump print limit in release mode\n");
        }
    }

    if (module_set == 0) {
        PS_PRINT_ERR("ssi dump regs bypass\n");
        return OAL_FALSE;
    }

    get_exception_info_reference_etc(&pst_exception_data);
    if (pst_exception_data == NULL) {
        PS_PRINT_ERR("get exception info reference is error\n");
        return OAL_FALSE;
    }
    if ((!ssi_dfr_bypass) &&
        (oal_work_is_busy(&pst_exception_data->wifi_excp_worker) ||
         oal_work_is_busy(&pst_exception_data->wifi_excp_recovery_worker) ||
         (atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE))) {
        PS_PRINT_ERR("dfr is doing ,not do ssi read\n");
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
        PS_PRINT_INFO("ssi is locked, request return\n");
        return -OAL_EBUSY;
    }

    if (gpio_get_value(board_info_etc.power_on_enable) == 0) {
        PS_PRINT_INFO("110x power off,ssi return,power_on=%d\n", board_info_etc.power_on_enable);
        ssi_unlock();
        return -OAL_ENODEV;
    }

    ssi_module_set_parm_config(module_set);

    DECLARE_DFT_TRACE_KEY_INFO("ssi_dump_device_regs", OAL_DFT_TRACE_FAIL);

    ret = ssi_request_gpio_etc(board_info_etc.ssi_gpio_clk, board_info_etc.ssi_gpio_data);
    if (ret) {
        ssi_unlock();
        return ret;
    }

    PS_PRINT_INFO("module_set=0x%llx\n", module_set);

    ret = ssi_device_regs_dump(module_set);
    if (ret) {
        goto ssi_fail;
    }

    ssi_free_gpio_etc();
    ssi_unlock();

    return 0;

ssi_fail:
    ssi_free_gpio_etc();
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


