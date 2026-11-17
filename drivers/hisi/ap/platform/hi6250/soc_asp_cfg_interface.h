#ifndef __SOC_ASP_CFG_INTERFACE_H__
#define __SOC_ASP_CFG_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_ASP_CFG_R_RST_CTRLEN_ADDR(base) ((base) + (0x0000))
#define SOC_ASP_CFG_R_RST_CTRLDIS_ADDR(base) ((base) + (0x0004))
#define SOC_ASP_CFG_R_RST_CTRLSTAT_ADDR(base) ((base) + (0x0008))
#define SOC_ASP_CFG_R_GATE_EN_ADDR(base) ((base) + (0x000C))
#define SOC_ASP_CFG_R_GATE_DIS_ADDR(base) ((base) + (0x0010))
#define SOC_ASP_CFG_R_GATE_CLKEN_ADDR(base) ((base) + (0x0014))
#define SOC_ASP_CFG_R_GATE_CLKSTAT_ADDR(base) ((base) + (0x0018))
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_ADDR(base) ((base) + (0x001C))
#define SOC_ASP_CFG_R_CLK1_DIV_ADDR(base) ((base) + (0x0020))
#define SOC_ASP_CFG_R_CLK2_DIV_ADDR(base) ((base) + (0x0024))
#define SOC_ASP_CFG_R_CLK3_DIV_ADDR(base) ((base) + (0x0028))
#define SOC_ASP_CFG_R_CLK4_DIV_ADDR(base) ((base) + (0x002C))
#define SOC_ASP_CFG_R_CLK5_DIV_ADDR(base) ((base) + (0x0030))
#define SOC_ASP_CFG_R_CLK6_DIV_ADDR(base) ((base) + (0x0034))
#define SOC_ASP_CFG_R_CLK_SEL_ADDR(base) ((base) + (0x0038))
#define SOC_ASP_CFG_R_DSP_NMI_ADDR(base) ((base) + (0x003C))
#define SOC_ASP_CFG_R_DSP_PRID_ADDR(base) ((base) + (0x0040))
#define SOC_ASP_CFG_R_DSP_RUNSTALL_ADDR(base) ((base) + (0x0044))
#define SOC_ASP_CFG_R_DSP_STATVECTORSEL_ADDR(base) ((base) + (0x0048))
#define SOC_ASP_CFG_R_DSP_OCDHALTONRESET_ADDR(base) ((base) + (0x004C))
#define SOC_ASP_CFG_R_DSP_STATUS_ADDR(base) ((base) + (0x0050))
#define SOC_ASP_CFG_R_DMAC_SEL_ADDR(base) ((base) + (0x0054))
#define SOC_ASP_CFG_R_BUS_PRIORITY_ADDR(base) ((base) + (0x0058))
#define SOC_ASP_CFG_R_CG_EN_ADDR(base) ((base) + (0x005C))
#define SOC_ASP_CFG_R_OCRAM_RET_ADDR(base) ((base) + (0x0060))
#define SOC_ASP_CFG_R_INTR_NS_INI_ADDR(base) ((base) + (0x0064))
#define SOC_ASP_CFG_R_INTR_NS_EN_ADDR(base) ((base) + (0x0068))
#define SOC_ASP_CFG_R_INTR_NS_MASK_ADDR(base) ((base) + (0x006C))
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_ADDR(base) ((base) + (0x0070))
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_ADDR(base) ((base) + (0x0074))
#define SOC_ASP_CFG_R_DLOCK_BP_ADDR(base) ((base) + (0x0078))
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_ADDR(base) ((base) + (0x007C))
#define SOC_ASP_CFG_R_DSP_RAM_RET_ADDR(base) ((base) + (0x0084))
#define SOC_ASP_CFG_R_TZ_SECURE_N_ADDR(base) ((base) + (0x0100))
#define SOC_ASP_CFG_R_OCRAM_R0SIZE_ADDR(base) ((base) + (0x0104))
#define SOC_ASP_CFG_R_SIO_LOOP_SECURE_N_ADDR(base) ((base) + (0x0108))
#define SOC_ASP_CFG_R_INTR_S_INI_ADDR(base) ((base) + (0x010C))
#define SOC_ASP_CFG_R_INTR_S_EN_ADDR(base) ((base) + (0x0110))
#define SOC_ASP_CFG_R_INTR_S_MASK_ADDR(base) ((base) + (0x0114))
#define SOC_ASP_CFG_R_DSP_REMAPPING_EN_ADDR(base) ((base) + (0x0118))
#define SOC_ASP_CFG_R_DSP_REMAPPING_SRC_BASE_ADDR_ADDR(base) ((base) + (0x011C))
#define SOC_ASP_CFG_R_DSP_REMAPPING_DES_BASE_ADDR_ADDR(base) ((base) + (0x0120))
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_EN_ADDR(base) ((base) + (0x124))
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_SRC_BASE_ADDR_ADDR(base) ((base) + (0x128))
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_DES_BASE_ADDR_ADDR(base) ((base) + (0x12C))
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_EN_ADDR(base) ((base) + (0x130))
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_SRC_BASE_ADDR_ADDR(base) ((base) + (0x134))
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_DES_BASE_ADDR_ADDR(base) ((base) + (0x138))
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_EN_ADDR(base) ((base) + (0x13C))
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_SRC_BASE_ADDR_ADDR(base) ((base) + (0x140))
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_DES_BASE_ADDR_ADDR(base) ((base) + (0x144))
#define SOC_ASP_CFG_R_MMBUF_CTRL_ADDR(base) ((base) + (0x0148))
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_ADDR(base) ((base) + (0x014C))
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_ADDR(base) ((base) + (0x0150))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_ADDR(base) ((base) + (0x0160))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_CLR_ADDR_ADDR(base) ((base) + (0x0164))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_ADDR(base) ((base) + (0x0168))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_ADDR(base) ((base) + (0x016C))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_BASE_ADDR_ADDR(base) ((base) + (0x0170))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_LEN_ADDR_ADDR(base) ((base) + (0x0174))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_HIT_AWADDR_ADDR(base) ((base) + (0x0178))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_HIT_ARADDR_ADDR(base) ((base) + (0x017C))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_BASE_ADDR_ADDR(base) ((base) + (0x0180))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_LEN_ADDR_ADDR(base) ((base) + (0x0184))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_HIT_AWADDR_ADDR(base) ((base) + (0x0188))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_HIT_ARADDR_ADDR(base) ((base) + (0x018C))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_BASE_ADDR_ADDR(base) ((base) + (0x0190))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_LEN_ADDR_ADDR(base) ((base) + (0x0194))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_HIT_AWADDR_ADDR(base) ((base) + (0x0198))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_HIT_ARADDR_ADDR(base) ((base) + (0x019C))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_BASE_ADDR_ADDR(base) ((base) + (0x01A0))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_LEN_ADDR_ADDR(base) ((base) + (0x01A4))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_HIT_AWADDR_ADDR(base) ((base) + (0x01A8))
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_HIT_ARADDR_ADDR(base) ((base) + (0x01AC))
#define SOC_ASP_CFG_R_MEMORY_CTRL_ADDR(base) ((base) + (0x01B0))
#define SOC_ASP_CFG_R_SLIMBUS_PRIMARY_ADDR_ADDR(base) ((base) + (0x01B4))
#define SOC_ASP_CFG_R_SLIMBUS_ID_ADDR_ADDR(base) ((base) + (0x01B8))
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_ADDR(base) ((base) + (0x01BC))
#define SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_ADDR(base) ((base) + (0x01C0))
#define SOC_ASP_CFG_R_CLK0_DIV_ADDR(base) ((base) + (0x01D0))
#define SOC_ASP_CFG_R_VOICE_WAKEUP_INTR_EN_ADDR(base) ((base) + (0x01D4))
#define SOC_ASP_CFG_R_CODEC_SEL_EN_ADDR(base) ((base) + (0x01E0))
#define SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_ADDR(base) ((base) + (0x0200))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_sio_audio_n : 1;
        unsigned int rst_sio_voice_n : 1;
        unsigned int rst_sio_bt_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int rst_dsp_n : 1;
        unsigned int rst_dsp_debug_n : 1;
        unsigned int rst_ipc_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int rst_dmac_n : 1;
        unsigned int preset_codec_n : 1;
        unsigned int rst_codec_n : 1;
        unsigned int rst_ocram_n : 1;
        unsigned int rst_watchdog_n : 1;
        unsigned int rst_slimbus_base_n : 1;
        unsigned int rst_slimbus_n : 1;
        unsigned int rst_asp_h2x_n : 1;
        unsigned int rst_gpio_n : 1;
        unsigned int rst_timer0_n : 1;
        unsigned int rst_timer1_n : 1;
        unsigned int dsp_preset_req_n : 1;
        unsigned int dsp_jtag_srst_req_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 10;
    } reg;
} SOC_ASP_CFG_R_RST_CTRLEN_UNION;
#endif
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_sio_audio_n_START (0)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_sio_audio_n_END (0)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_sio_voice_n_START (1)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_sio_voice_n_END (1)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_sio_bt_n_START (2)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_sio_bt_n_END (2)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_dsp_n_START (4)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_dsp_n_END (4)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_dsp_debug_n_START (5)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_dsp_debug_n_END (5)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_ipc_n_START (6)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_ipc_n_END (6)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_dmac_n_START (8)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_dmac_n_END (8)
#define SOC_ASP_CFG_R_RST_CTRLEN_preset_codec_n_START (9)
#define SOC_ASP_CFG_R_RST_CTRLEN_preset_codec_n_END (9)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_codec_n_START (10)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_codec_n_END (10)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_ocram_n_START (11)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_ocram_n_END (11)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_watchdog_n_START (12)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_watchdog_n_END (12)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_slimbus_base_n_START (13)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_slimbus_base_n_END (13)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_slimbus_n_START (14)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_slimbus_n_END (14)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_asp_h2x_n_START (15)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_asp_h2x_n_END (15)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_gpio_n_START (16)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_gpio_n_END (16)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_timer0_n_START (17)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_timer0_n_END (17)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_timer1_n_START (18)
#define SOC_ASP_CFG_R_RST_CTRLEN_rst_timer1_n_END (18)
#define SOC_ASP_CFG_R_RST_CTRLEN_dsp_preset_req_n_START (19)
#define SOC_ASP_CFG_R_RST_CTRLEN_dsp_preset_req_n_END (19)
#define SOC_ASP_CFG_R_RST_CTRLEN_dsp_jtag_srst_req_n_START (20)
#define SOC_ASP_CFG_R_RST_CTRLEN_dsp_jtag_srst_req_n_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_sio_audio : 1;
        unsigned int rst_sio_voice_n : 1;
        unsigned int rst_sio_bt_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int rst_dsp_n : 1;
        unsigned int rst_dsp_debug_n : 1;
        unsigned int rst_ipc_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int rst_dmac_n : 1;
        unsigned int preset_codec_n : 1;
        unsigned int rst_codec_n : 1;
        unsigned int rst_ocram_n : 1;
        unsigned int rst_watchdog_n : 1;
        unsigned int rst_slimbus_base_n : 1;
        unsigned int rst_slimbus_n : 1;
        unsigned int rst_asp_h2x_n : 1;
        unsigned int rst_gpio_n : 1;
        unsigned int rst_timer0_n : 1;
        unsigned int rst_timer1_n : 1;
        unsigned int dsp_preset_req_n : 1;
        unsigned int dsp_jtag_srst_req_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 10;
    } reg;
} SOC_ASP_CFG_R_RST_CTRLDIS_UNION;
#endif
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_sio_audio_START (0)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_sio_audio_END (0)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_sio_voice_n_START (1)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_sio_voice_n_END (1)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_sio_bt_n_START (2)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_sio_bt_n_END (2)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_dsp_n_START (4)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_dsp_n_END (4)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_dsp_debug_n_START (5)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_dsp_debug_n_END (5)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_ipc_n_START (6)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_ipc_n_END (6)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_dmac_n_START (8)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_dmac_n_END (8)
#define SOC_ASP_CFG_R_RST_CTRLDIS_preset_codec_n_START (9)
#define SOC_ASP_CFG_R_RST_CTRLDIS_preset_codec_n_END (9)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_codec_n_START (10)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_codec_n_END (10)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_ocram_n_START (11)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_ocram_n_END (11)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_watchdog_n_START (12)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_watchdog_n_END (12)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_slimbus_base_n_START (13)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_slimbus_base_n_END (13)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_slimbus_n_START (14)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_slimbus_n_END (14)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_asp_h2x_n_START (15)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_asp_h2x_n_END (15)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_gpio_n_START (16)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_gpio_n_END (16)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_timer0_n_START (17)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_timer0_n_END (17)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_timer1_n_START (18)
#define SOC_ASP_CFG_R_RST_CTRLDIS_rst_timer1_n_END (18)
#define SOC_ASP_CFG_R_RST_CTRLDIS_dsp_preset_req_n_START (19)
#define SOC_ASP_CFG_R_RST_CTRLDIS_dsp_preset_req_n_END (19)
#define SOC_ASP_CFG_R_RST_CTRLDIS_dsp_jtag_srst_req_n_START (20)
#define SOC_ASP_CFG_R_RST_CTRLDIS_dsp_jtag_srst_req_n_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rst_sio_audio_n : 1;
        unsigned int rst_sio_voice_n : 1;
        unsigned int rst_sio_bt_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int rst_dsp_n : 1;
        unsigned int rst_dsp_debug_n : 1;
        unsigned int rst_ipc_n : 1;
        unsigned int reserved_1 : 1;
        unsigned int rst_dmac_n : 1;
        unsigned int preset_codec_n : 1;
        unsigned int rst_codec_n : 1;
        unsigned int rst_ocram_n : 1;
        unsigned int rst_watchdog_n : 1;
        unsigned int rst_slimbus_base_n : 1;
        unsigned int rst_slimbus_n : 1;
        unsigned int rst_asp_h2x_n : 1;
        unsigned int rst_gpio_n : 1;
        unsigned int rst_timer0_n : 1;
        unsigned int rst_timer1_n : 1;
        unsigned int dsp_preset_req_n : 1;
        unsigned int dsp_jtag_srst_req_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int reserved_3 : 10;
    } reg;
} SOC_ASP_CFG_R_RST_CTRLSTAT_UNION;
#endif
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_sio_audio_n_START (0)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_sio_audio_n_END (0)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_sio_voice_n_START (1)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_sio_voice_n_END (1)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_sio_bt_n_START (2)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_sio_bt_n_END (2)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_dsp_n_START (4)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_dsp_n_END (4)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_dsp_debug_n_START (5)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_dsp_debug_n_END (5)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_ipc_n_START (6)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_ipc_n_END (6)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_dmac_n_START (8)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_dmac_n_END (8)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_preset_codec_n_START (9)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_preset_codec_n_END (9)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_codec_n_START (10)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_codec_n_END (10)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_ocram_n_START (11)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_ocram_n_END (11)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_watchdog_n_START (12)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_watchdog_n_END (12)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_slimbus_base_n_START (13)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_slimbus_base_n_END (13)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_slimbus_n_START (14)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_slimbus_n_END (14)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_asp_h2x_n_START (15)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_asp_h2x_n_END (15)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_gpio_n_START (16)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_gpio_n_END (16)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_timer0_n_START (17)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_timer0_n_END (17)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_timer1_n_START (18)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_rst_timer1_n_END (18)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_dsp_preset_req_n_START (19)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_dsp_preset_req_n_END (19)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_dsp_jtag_srst_req_n_START (20)
#define SOC_ASP_CFG_R_RST_CTRLSTAT_dsp_jtag_srst_req_n_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_hifidspclk : 1;
        unsigned int gt_audiobclk_out : 1;
        unsigned int gt_sioaudio : 1;
        unsigned int gt_voicebclk_out : 1;
        unsigned int gt_siovoice : 1;
        unsigned int gt_btbclk_out : 1;
        unsigned int gt_siobt : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_ipchclk : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_dmachclk : 1;
        unsigned int gt_pclk_codec : 1;
        unsigned int gt_codec_sysclk : 1;
        unsigned int gt_ocramhclk : 1;
        unsigned int gt_wdhclk : 1;
        unsigned int gt_slimbushclk : 1;
        unsigned int gt_slimbus_base_clk : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_gpiohclk : 1;
        unsigned int gt_timer0hclk : 1;
        unsigned int gt_timer1hclk : 1;
        unsigned int gt_clk_ppll0_slimbus : 1;
        unsigned int reserved_6 : 7;
    } reg;
} SOC_ASP_CFG_R_GATE_EN_UNION;
#endif
#define SOC_ASP_CFG_R_GATE_EN_gt_hifidspclk_START (0)
#define SOC_ASP_CFG_R_GATE_EN_gt_hifidspclk_END (0)
#define SOC_ASP_CFG_R_GATE_EN_gt_audiobclk_out_START (1)
#define SOC_ASP_CFG_R_GATE_EN_gt_audiobclk_out_END (1)
#define SOC_ASP_CFG_R_GATE_EN_gt_sioaudio_START (2)
#define SOC_ASP_CFG_R_GATE_EN_gt_sioaudio_END (2)
#define SOC_ASP_CFG_R_GATE_EN_gt_voicebclk_out_START (3)
#define SOC_ASP_CFG_R_GATE_EN_gt_voicebclk_out_END (3)
#define SOC_ASP_CFG_R_GATE_EN_gt_siovoice_START (4)
#define SOC_ASP_CFG_R_GATE_EN_gt_siovoice_END (4)
#define SOC_ASP_CFG_R_GATE_EN_gt_btbclk_out_START (5)
#define SOC_ASP_CFG_R_GATE_EN_gt_btbclk_out_END (5)
#define SOC_ASP_CFG_R_GATE_EN_gt_siobt_START (6)
#define SOC_ASP_CFG_R_GATE_EN_gt_siobt_END (6)
#define SOC_ASP_CFG_R_GATE_EN_gt_ipchclk_START (9)
#define SOC_ASP_CFG_R_GATE_EN_gt_ipchclk_END (9)
#define SOC_ASP_CFG_R_GATE_EN_gt_dmachclk_START (11)
#define SOC_ASP_CFG_R_GATE_EN_gt_dmachclk_END (11)
#define SOC_ASP_CFG_R_GATE_EN_gt_pclk_codec_START (12)
#define SOC_ASP_CFG_R_GATE_EN_gt_pclk_codec_END (12)
#define SOC_ASP_CFG_R_GATE_EN_gt_codec_sysclk_START (13)
#define SOC_ASP_CFG_R_GATE_EN_gt_codec_sysclk_END (13)
#define SOC_ASP_CFG_R_GATE_EN_gt_ocramhclk_START (14)
#define SOC_ASP_CFG_R_GATE_EN_gt_ocramhclk_END (14)
#define SOC_ASP_CFG_R_GATE_EN_gt_wdhclk_START (15)
#define SOC_ASP_CFG_R_GATE_EN_gt_wdhclk_END (15)
#define SOC_ASP_CFG_R_GATE_EN_gt_slimbushclk_START (16)
#define SOC_ASP_CFG_R_GATE_EN_gt_slimbushclk_END (16)
#define SOC_ASP_CFG_R_GATE_EN_gt_slimbus_base_clk_START (17)
#define SOC_ASP_CFG_R_GATE_EN_gt_slimbus_base_clk_END (17)
#define SOC_ASP_CFG_R_GATE_EN_gt_gpiohclk_START (21)
#define SOC_ASP_CFG_R_GATE_EN_gt_gpiohclk_END (21)
#define SOC_ASP_CFG_R_GATE_EN_gt_timer0hclk_START (22)
#define SOC_ASP_CFG_R_GATE_EN_gt_timer0hclk_END (22)
#define SOC_ASP_CFG_R_GATE_EN_gt_timer1hclk_START (23)
#define SOC_ASP_CFG_R_GATE_EN_gt_timer1hclk_END (23)
#define SOC_ASP_CFG_R_GATE_EN_gt_clk_ppll0_slimbus_START (24)
#define SOC_ASP_CFG_R_GATE_EN_gt_clk_ppll0_slimbus_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_hifidspclk : 1;
        unsigned int gt_audiobclk_out : 1;
        unsigned int gt_sioaudio : 1;
        unsigned int gt_voicebclk_out : 1;
        unsigned int gt_siovoice : 1;
        unsigned int gt_btbclk_out : 1;
        unsigned int gt_siobt : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_ipchclk : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_dmachclk : 1;
        unsigned int gt_pclk_codec : 1;
        unsigned int gt_codec_sysclk : 1;
        unsigned int gt_ocramhclk : 1;
        unsigned int gt_wdhclk : 1;
        unsigned int gt_slimbushclk : 1;
        unsigned int gt_slimbus_base_clk : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_gpiohclk : 1;
        unsigned int gt_timer0hclk : 1;
        unsigned int gt_timer1hclk : 1;
        unsigned int gt_clk_ppll0_slimbus : 1;
        unsigned int reserved_6 : 7;
    } reg;
} SOC_ASP_CFG_R_GATE_DIS_UNION;
#endif
#define SOC_ASP_CFG_R_GATE_DIS_gt_hifidspclk_START (0)
#define SOC_ASP_CFG_R_GATE_DIS_gt_hifidspclk_END (0)
#define SOC_ASP_CFG_R_GATE_DIS_gt_audiobclk_out_START (1)
#define SOC_ASP_CFG_R_GATE_DIS_gt_audiobclk_out_END (1)
#define SOC_ASP_CFG_R_GATE_DIS_gt_sioaudio_START (2)
#define SOC_ASP_CFG_R_GATE_DIS_gt_sioaudio_END (2)
#define SOC_ASP_CFG_R_GATE_DIS_gt_voicebclk_out_START (3)
#define SOC_ASP_CFG_R_GATE_DIS_gt_voicebclk_out_END (3)
#define SOC_ASP_CFG_R_GATE_DIS_gt_siovoice_START (4)
#define SOC_ASP_CFG_R_GATE_DIS_gt_siovoice_END (4)
#define SOC_ASP_CFG_R_GATE_DIS_gt_btbclk_out_START (5)
#define SOC_ASP_CFG_R_GATE_DIS_gt_btbclk_out_END (5)
#define SOC_ASP_CFG_R_GATE_DIS_gt_siobt_START (6)
#define SOC_ASP_CFG_R_GATE_DIS_gt_siobt_END (6)
#define SOC_ASP_CFG_R_GATE_DIS_gt_ipchclk_START (9)
#define SOC_ASP_CFG_R_GATE_DIS_gt_ipchclk_END (9)
#define SOC_ASP_CFG_R_GATE_DIS_gt_dmachclk_START (11)
#define SOC_ASP_CFG_R_GATE_DIS_gt_dmachclk_END (11)
#define SOC_ASP_CFG_R_GATE_DIS_gt_pclk_codec_START (12)
#define SOC_ASP_CFG_R_GATE_DIS_gt_pclk_codec_END (12)
#define SOC_ASP_CFG_R_GATE_DIS_gt_codec_sysclk_START (13)
#define SOC_ASP_CFG_R_GATE_DIS_gt_codec_sysclk_END (13)
#define SOC_ASP_CFG_R_GATE_DIS_gt_ocramhclk_START (14)
#define SOC_ASP_CFG_R_GATE_DIS_gt_ocramhclk_END (14)
#define SOC_ASP_CFG_R_GATE_DIS_gt_wdhclk_START (15)
#define SOC_ASP_CFG_R_GATE_DIS_gt_wdhclk_END (15)
#define SOC_ASP_CFG_R_GATE_DIS_gt_slimbushclk_START (16)
#define SOC_ASP_CFG_R_GATE_DIS_gt_slimbushclk_END (16)
#define SOC_ASP_CFG_R_GATE_DIS_gt_slimbus_base_clk_START (17)
#define SOC_ASP_CFG_R_GATE_DIS_gt_slimbus_base_clk_END (17)
#define SOC_ASP_CFG_R_GATE_DIS_gt_gpiohclk_START (21)
#define SOC_ASP_CFG_R_GATE_DIS_gt_gpiohclk_END (21)
#define SOC_ASP_CFG_R_GATE_DIS_gt_timer0hclk_START (22)
#define SOC_ASP_CFG_R_GATE_DIS_gt_timer0hclk_END (22)
#define SOC_ASP_CFG_R_GATE_DIS_gt_timer1hclk_START (23)
#define SOC_ASP_CFG_R_GATE_DIS_gt_timer1hclk_END (23)
#define SOC_ASP_CFG_R_GATE_DIS_gt_clk_ppll0_slimbus_START (24)
#define SOC_ASP_CFG_R_GATE_DIS_gt_clk_ppll0_slimbus_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_hifidspclk : 1;
        unsigned int gt_audiobclk_out : 1;
        unsigned int gt_sioaudio : 1;
        unsigned int gt_voicebclk_out : 1;
        unsigned int gt_siovoice : 1;
        unsigned int gt_btbclk_out : 1;
        unsigned int gt_siobt : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_ipchclk : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_dmachclk : 1;
        unsigned int gt_pclk_codec : 1;
        unsigned int gt_codec_sysclk : 1;
        unsigned int gt_ocramhclk : 1;
        unsigned int gt_wdhclk : 1;
        unsigned int gt_slimbushclk : 1;
        unsigned int gt_slimbus_base_clk : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int gt_gpiohclk : 1;
        unsigned int gt_timer0hclk : 1;
        unsigned int gt_timer1hclk : 1;
        unsigned int gt_clk_ppll0_slimbus : 1;
        unsigned int reserved_6 : 7;
    } reg;
} SOC_ASP_CFG_R_GATE_CLKEN_UNION;
#endif
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_hifidspclk_START (0)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_hifidspclk_END (0)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_audiobclk_out_START (1)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_audiobclk_out_END (1)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_sioaudio_START (2)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_sioaudio_END (2)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_voicebclk_out_START (3)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_voicebclk_out_END (3)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_siovoice_START (4)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_siovoice_END (4)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_btbclk_out_START (5)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_btbclk_out_END (5)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_siobt_START (6)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_siobt_END (6)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_ipchclk_START (9)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_ipchclk_END (9)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_dmachclk_START (11)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_dmachclk_END (11)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_pclk_codec_START (12)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_pclk_codec_END (12)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_codec_sysclk_START (13)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_codec_sysclk_END (13)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_ocramhclk_START (14)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_ocramhclk_END (14)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_wdhclk_START (15)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_wdhclk_END (15)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_slimbushclk_START (16)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_slimbushclk_END (16)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_slimbus_base_clk_START (17)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_slimbus_base_clk_END (17)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_gpiohclk_START (21)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_gpiohclk_END (21)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_timer0hclk_START (22)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_timer0hclk_END (22)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_timer1hclk_START (23)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_timer1hclk_END (23)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_clk_ppll0_slimbus_START (24)
#define SOC_ASP_CFG_R_GATE_CLKEN_gt_clk_ppll0_slimbus_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int st_hifidspclk : 1;
        unsigned int st_audiobclk_out : 1;
        unsigned int st_audiobclk : 1;
        unsigned int gt_voicebclk_out : 1;
        unsigned int gt_siovoice : 1;
        unsigned int gt_btbclk_out : 1;
        unsigned int gt_siobt : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int st_ipchclk : 1;
        unsigned int reserved_2 : 1;
        unsigned int st_dmachclk0 : 1;
        unsigned int st_dmachclk1 : 1;
        unsigned int st_pclk_codec : 1;
        unsigned int st_codec_sysclk : 1;
        unsigned int st_ocramhclk : 1;
        unsigned int st_wdhclk : 1;
        unsigned int st_slimbushclk : 1;
        unsigned int st_slimbus_base_clk : 1;
        unsigned int reserved_3 : 1;
        unsigned int reserved_4 : 1;
        unsigned int reserved_5 : 1;
        unsigned int st_asp_h2x : 1;
        unsigned int st_audiohclk : 1;
        unsigned int st_voicehclk : 1;
        unsigned int st_bthclk : 1;
        unsigned int reserved_6 : 1;
        unsigned int st_gpiohclk : 1;
        unsigned int st_timer0hclk : 1;
        unsigned int st_timer1hclk : 1;
        unsigned int reserved_7 : 1;
        unsigned int reserved_8 : 1;
    } reg;
} SOC_ASP_CFG_R_GATE_CLKSTAT_UNION;
#endif
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_hifidspclk_START (0)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_hifidspclk_END (0)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_audiobclk_out_START (1)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_audiobclk_out_END (1)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_audiobclk_START (2)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_audiobclk_END (2)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_gt_voicebclk_out_START (3)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_gt_voicebclk_out_END (3)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_gt_siovoice_START (4)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_gt_siovoice_END (4)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_gt_btbclk_out_START (5)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_gt_btbclk_out_END (5)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_gt_siobt_START (6)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_gt_siobt_END (6)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_ipchclk_START (9)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_ipchclk_END (9)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_dmachclk0_START (11)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_dmachclk0_END (11)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_dmachclk1_START (12)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_dmachclk1_END (12)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_pclk_codec_START (13)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_pclk_codec_END (13)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_codec_sysclk_START (14)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_codec_sysclk_END (14)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_ocramhclk_START (15)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_ocramhclk_END (15)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_wdhclk_START (16)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_wdhclk_END (16)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_slimbushclk_START (17)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_slimbushclk_END (17)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_slimbus_base_clk_START (18)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_slimbus_base_clk_END (18)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_asp_h2x_START (22)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_asp_h2x_END (22)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_audiohclk_START (23)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_audiohclk_END (23)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_voicehclk_START (24)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_voicehclk_END (24)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_bthclk_START (25)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_bthclk_END (25)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_gpiohclk_START (27)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_gpiohclk_END (27)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_timer0hclk_START (28)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_timer0hclk_END (28)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_timer1hclk_START (29)
#define SOC_ASP_CFG_R_GATE_CLKSTAT_st_timer1hclk_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int gt_siobclk_div : 1;
        unsigned int gt_sio_audio_bclk_div : 1;
        unsigned int gt_sio_voice_bclk_div : 1;
        unsigned int gt_sio_bt_bclk_div : 1;
        unsigned int reserved_2 : 1;
        unsigned int gt_asp_tcxo_div : 1;
        unsigned int gt_slimbus_clk_div : 1;
        unsigned int reserved_3 : 23;
    } reg;
} SOC_ASP_CFG_R_GATE_CLKDIV_EN_UNION;
#endif
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_siobclk_div_START (2)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_siobclk_div_END (2)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_sio_audio_bclk_div_START (3)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_sio_audio_bclk_div_END (3)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_sio_voice_bclk_div_START (4)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_sio_voice_bclk_div_END (4)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_sio_bt_bclk_div_START (5)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_sio_bt_bclk_div_END (5)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_asp_tcxo_div_START (7)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_asp_tcxo_div_END (7)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_slimbus_clk_div_START (8)
#define SOC_ASP_CFG_R_GATE_CLKDIV_EN_gt_slimbus_clk_div_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_siobclk : 27;
        unsigned int reserved : 5;
    } reg;
} SOC_ASP_CFG_R_CLK1_DIV_UNION;
#endif
#define SOC_ASP_CFG_R_CLK1_DIV_div_siobclk_START (0)
#define SOC_ASP_CFG_R_CLK1_DIV_div_siobclk_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 27;
        unsigned int reserved_1: 5;
    } reg;
} SOC_ASP_CFG_R_CLK2_DIV_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_sio_audio_bclk : 8;
        unsigned int div_sio_voice_bclk : 8;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_CLK3_DIV_UNION;
#endif
#define SOC_ASP_CFG_R_CLK3_DIV_div_sio_audio_bclk_START (0)
#define SOC_ASP_CFG_R_CLK3_DIV_div_sio_audio_bclk_END (7)
#define SOC_ASP_CFG_R_CLK3_DIV_div_sio_voice_bclk_START (8)
#define SOC_ASP_CFG_R_CLK3_DIV_div_sio_voice_bclk_END (15)
#define SOC_ASP_CFG_R_CLK3_DIV_write_en_START (16)
#define SOC_ASP_CFG_R_CLK3_DIV_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_sio_bt_bclk : 8;
        unsigned int reserved : 8;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_CLK4_DIV_UNION;
#endif
#define SOC_ASP_CFG_R_CLK4_DIV_div_sio_bt_bclk_START (0)
#define SOC_ASP_CFG_R_CLK4_DIV_div_sio_bt_bclk_END (7)
#define SOC_ASP_CFG_R_CLK4_DIV_write_en_START (16)
#define SOC_ASP_CFG_R_CLK4_DIV_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_sio_audio_adws : 8;
        unsigned int div_sio_voice_adws : 8;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_CLK5_DIV_UNION;
#endif
#define SOC_ASP_CFG_R_CLK5_DIV_div_sio_audio_adws_START (0)
#define SOC_ASP_CFG_R_CLK5_DIV_div_sio_audio_adws_END (7)
#define SOC_ASP_CFG_R_CLK5_DIV_div_sio_voice_adws_START (8)
#define SOC_ASP_CFG_R_CLK5_DIV_div_sio_voice_adws_END (15)
#define SOC_ASP_CFG_R_CLK5_DIV_write_en_START (16)
#define SOC_ASP_CFG_R_CLK5_DIV_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_sio_bt_adws : 8;
        unsigned int reserved : 8;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_CLK6_DIV_UNION;
#endif
#define SOC_ASP_CFG_R_CLK6_DIV_div_sio_bt_adws_START (0)
#define SOC_ASP_CFG_R_CLK6_DIV_div_sio_bt_adws_END (7)
#define SOC_ASP_CFG_R_CLK6_DIV_write_en_START (16)
#define SOC_ASP_CFG_R_CLK6_DIV_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sio_audio_slave_en : 1;
        unsigned int sio_voice_slave_en : 1;
        unsigned int sio_bt_slave_en : 1;
        unsigned int asp_modemclk_sel : 1;
        unsigned int siobclk_sel : 1;
        unsigned int slimbus_base_clk_sel : 2;
        unsigned int watchdog_clken_sel : 1;
        unsigned int timer0_1_clken_sel : 1;
        unsigned int timer0_2_clken_sel : 1;
        unsigned int timer1_1_clken_sel : 1;
        unsigned int timer1_2_clken_sel : 1;
        unsigned int freq_tcxo_sel : 1;
        unsigned int freq_modem_sel : 1;
        unsigned int hifidsp_clk_sel : 1;
        unsigned int slimbus_clk_sel : 1;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_CLK_SEL_UNION;
#endif
#define SOC_ASP_CFG_R_CLK_SEL_sio_audio_slave_en_START (0)
#define SOC_ASP_CFG_R_CLK_SEL_sio_audio_slave_en_END (0)
#define SOC_ASP_CFG_R_CLK_SEL_sio_voice_slave_en_START (1)
#define SOC_ASP_CFG_R_CLK_SEL_sio_voice_slave_en_END (1)
#define SOC_ASP_CFG_R_CLK_SEL_sio_bt_slave_en_START (2)
#define SOC_ASP_CFG_R_CLK_SEL_sio_bt_slave_en_END (2)
#define SOC_ASP_CFG_R_CLK_SEL_asp_modemclk_sel_START (3)
#define SOC_ASP_CFG_R_CLK_SEL_asp_modemclk_sel_END (3)
#define SOC_ASP_CFG_R_CLK_SEL_siobclk_sel_START (4)
#define SOC_ASP_CFG_R_CLK_SEL_siobclk_sel_END (4)
#define SOC_ASP_CFG_R_CLK_SEL_slimbus_base_clk_sel_START (5)
#define SOC_ASP_CFG_R_CLK_SEL_slimbus_base_clk_sel_END (6)
#define SOC_ASP_CFG_R_CLK_SEL_watchdog_clken_sel_START (7)
#define SOC_ASP_CFG_R_CLK_SEL_watchdog_clken_sel_END (7)
#define SOC_ASP_CFG_R_CLK_SEL_timer0_1_clken_sel_START (8)
#define SOC_ASP_CFG_R_CLK_SEL_timer0_1_clken_sel_END (8)
#define SOC_ASP_CFG_R_CLK_SEL_timer0_2_clken_sel_START (9)
#define SOC_ASP_CFG_R_CLK_SEL_timer0_2_clken_sel_END (9)
#define SOC_ASP_CFG_R_CLK_SEL_timer1_1_clken_sel_START (10)
#define SOC_ASP_CFG_R_CLK_SEL_timer1_1_clken_sel_END (10)
#define SOC_ASP_CFG_R_CLK_SEL_timer1_2_clken_sel_START (11)
#define SOC_ASP_CFG_R_CLK_SEL_timer1_2_clken_sel_END (11)
#define SOC_ASP_CFG_R_CLK_SEL_freq_tcxo_sel_START (12)
#define SOC_ASP_CFG_R_CLK_SEL_freq_tcxo_sel_END (12)
#define SOC_ASP_CFG_R_CLK_SEL_freq_modem_sel_START (13)
#define SOC_ASP_CFG_R_CLK_SEL_freq_modem_sel_END (13)
#define SOC_ASP_CFG_R_CLK_SEL_hifidsp_clk_sel_START (14)
#define SOC_ASP_CFG_R_CLK_SEL_hifidsp_clk_sel_END (14)
#define SOC_ASP_CFG_R_CLK_SEL_slimbus_clk_sel_START (15)
#define SOC_ASP_CFG_R_CLK_SEL_slimbus_clk_sel_END (15)
#define SOC_ASP_CFG_R_CLK_SEL_write_en_START (16)
#define SOC_ASP_CFG_R_CLK_SEL_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int r_dsp_nmi : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_ASP_CFG_R_DSP_NMI_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_NMI_r_dsp_nmi_START (0)
#define SOC_ASP_CFG_R_DSP_NMI_r_dsp_nmi_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsp_prid : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_ASP_CFG_R_DSP_PRID_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_PRID_dsp_prid_START (0)
#define SOC_ASP_CFG_R_DSP_PRID_dsp_prid_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsp_runstall : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ASP_CFG_R_DSP_RUNSTALL_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_RUNSTALL_dsp_runstall_START (0)
#define SOC_ASP_CFG_R_DSP_RUNSTALL_dsp_runstall_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsp_statvectorsel : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ASP_CFG_R_DSP_STATVECTORSEL_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_STATVECTORSEL_dsp_statvectorsel_START (0)
#define SOC_ASP_CFG_R_DSP_STATVECTORSEL_dsp_statvectorsel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsp_ocdhaltonreset : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ASP_CFG_R_DSP_OCDHALTONRESET_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_OCDHALTONRESET_dsp_ocdhaltonreset_START (0)
#define SOC_ASP_CFG_R_DSP_OCDHALTONRESET_dsp_ocdhaltonreset_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsp_xocdmode : 1;
        unsigned int dsp_pwaitmode : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_ASP_CFG_R_DSP_STATUS_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_STATUS_dsp_xocdmode_START (0)
#define SOC_ASP_CFG_R_DSP_STATUS_dsp_xocdmode_END (0)
#define SOC_ASP_CFG_R_DSP_STATUS_dsp_pwaitmode_START (1)
#define SOC_ASP_CFG_R_DSP_STATUS_dsp_pwaitmode_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 1;
        unsigned int reserved_4: 1;
        unsigned int reserved_5: 1;
        unsigned int reserved_6: 1;
        unsigned int reserved_7: 1;
        unsigned int reserved_8: 2;
        unsigned int reserved_9: 6;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_DMAC_SEL_UNION;
#endif
#define SOC_ASP_CFG_R_DMAC_SEL_write_en_START (16)
#define SOC_ASP_CFG_R_DMAC_SEL_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dsp_ar_qos : 2;
        unsigned int dsp_aw_qos : 2;
        unsigned int dma_ar_qos : 2;
        unsigned int dma_aw_qos : 2;
        unsigned int ahb2axi_ar_qos : 2;
        unsigned int ahb2axi_aw_qos : 2;
        unsigned int asp_slv_priority : 2;
        unsigned int dsp_mst_priority : 2;
        unsigned int dmac_priority : 2;
        unsigned int ahb2axi_priority : 2;
        unsigned int asp_mst_priority : 3;
        unsigned int dsp_slv_priority : 3;
        unsigned int ocram_priority : 3;
        unsigned int x2h_priority : 3;
    } reg;
} SOC_ASP_CFG_R_BUS_PRIORITY_UNION;
#endif
#define SOC_ASP_CFG_R_BUS_PRIORITY_dsp_ar_qos_START (0)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dsp_ar_qos_END (1)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dsp_aw_qos_START (2)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dsp_aw_qos_END (3)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dma_ar_qos_START (4)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dma_ar_qos_END (5)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dma_aw_qos_START (6)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dma_aw_qos_END (7)
#define SOC_ASP_CFG_R_BUS_PRIORITY_ahb2axi_ar_qos_START (8)
#define SOC_ASP_CFG_R_BUS_PRIORITY_ahb2axi_ar_qos_END (9)
#define SOC_ASP_CFG_R_BUS_PRIORITY_ahb2axi_aw_qos_START (10)
#define SOC_ASP_CFG_R_BUS_PRIORITY_ahb2axi_aw_qos_END (11)
#define SOC_ASP_CFG_R_BUS_PRIORITY_asp_slv_priority_START (12)
#define SOC_ASP_CFG_R_BUS_PRIORITY_asp_slv_priority_END (13)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dsp_mst_priority_START (14)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dsp_mst_priority_END (15)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dmac_priority_START (16)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dmac_priority_END (17)
#define SOC_ASP_CFG_R_BUS_PRIORITY_ahb2axi_priority_START (18)
#define SOC_ASP_CFG_R_BUS_PRIORITY_ahb2axi_priority_END (19)
#define SOC_ASP_CFG_R_BUS_PRIORITY_asp_mst_priority_START (20)
#define SOC_ASP_CFG_R_BUS_PRIORITY_asp_mst_priority_END (22)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dsp_slv_priority_START (23)
#define SOC_ASP_CFG_R_BUS_PRIORITY_dsp_slv_priority_END (25)
#define SOC_ASP_CFG_R_BUS_PRIORITY_ocram_priority_START (26)
#define SOC_ASP_CFG_R_BUS_PRIORITY_ocram_priority_END (28)
#define SOC_ASP_CFG_R_BUS_PRIORITY_x2h_priority_START (29)
#define SOC_ASP_CFG_R_BUS_PRIORITY_x2h_priority_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dw_axi_m1_cg_en : 1;
        unsigned int dw_axi_m2_cg_en : 1;
        unsigned int dw_axi_m3_cg_en : 1;
        unsigned int dw_axi_m4_cg_en : 1;
        unsigned int dw_axi_s1_cg_en : 1;
        unsigned int dw_axi_s2_cg_en : 1;
        unsigned int dw_axi_s3_cg_en : 1;
        unsigned int dw_axi_s4_cg_en : 1;
        unsigned int dw_axi_s0_cg_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int dmac_hclk0_cg_en : 1;
        unsigned int dmac_hclk1_cg_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 3;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_CG_EN_UNION;
#endif
#define SOC_ASP_CFG_R_CG_EN_dw_axi_m1_cg_en_START (0)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_m1_cg_en_END (0)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_m2_cg_en_START (1)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_m2_cg_en_END (1)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_m3_cg_en_START (2)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_m3_cg_en_END (2)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_m4_cg_en_START (3)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_m4_cg_en_END (3)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s1_cg_en_START (4)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s1_cg_en_END (4)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s2_cg_en_START (5)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s2_cg_en_END (5)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s3_cg_en_START (6)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s3_cg_en_END (6)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s4_cg_en_START (7)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s4_cg_en_END (7)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s0_cg_en_START (8)
#define SOC_ASP_CFG_R_CG_EN_dw_axi_s0_cg_en_END (8)
#define SOC_ASP_CFG_R_CG_EN_dmac_hclk0_cg_en_START (10)
#define SOC_ASP_CFG_R_CG_EN_dmac_hclk0_cg_en_END (10)
#define SOC_ASP_CFG_R_CG_EN_dmac_hclk1_cg_en_START (11)
#define SOC_ASP_CFG_R_CG_EN_dmac_hclk1_cg_en_END (11)
#define SOC_ASP_CFG_R_CG_EN_write_en_START (16)
#define SOC_ASP_CFG_R_CG_EN_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int slp : 2;
        unsigned int reserved_0: 2;
        unsigned int dslp : 2;
        unsigned int reserved_1: 2;
        unsigned int sd : 2;
        unsigned int reserved_2: 6;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_OCRAM_RET_UNION;
#endif
#define SOC_ASP_CFG_R_OCRAM_RET_slp_START (0)
#define SOC_ASP_CFG_R_OCRAM_RET_slp_END (1)
#define SOC_ASP_CFG_R_OCRAM_RET_dslp_START (4)
#define SOC_ASP_CFG_R_OCRAM_RET_dslp_END (5)
#define SOC_ASP_CFG_R_OCRAM_RET_sd_START (8)
#define SOC_ASP_CFG_R_OCRAM_RET_sd_END (9)
#define SOC_ASP_CFG_R_OCRAM_RET_write_en_START (16)
#define SOC_ASP_CFG_R_OCRAM_RET_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int codec_intr : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int slimbus_intr : 1;
        unsigned int reserved_2 : 28;
    } reg;
} SOC_ASP_CFG_R_INTR_NS_INI_UNION;
#endif
#define SOC_ASP_CFG_R_INTR_NS_INI_codec_intr_START (0)
#define SOC_ASP_CFG_R_INTR_NS_INI_codec_intr_END (0)
#define SOC_ASP_CFG_R_INTR_NS_INI_slimbus_intr_START (3)
#define SOC_ASP_CFG_R_INTR_NS_INI_slimbus_intr_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int codec_intr_en : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int slimbus_intr_en : 1;
        unsigned int reserved_2 : 28;
    } reg;
} SOC_ASP_CFG_R_INTR_NS_EN_UNION;
#endif
#define SOC_ASP_CFG_R_INTR_NS_EN_codec_intr_en_START (0)
#define SOC_ASP_CFG_R_INTR_NS_EN_codec_intr_en_END (0)
#define SOC_ASP_CFG_R_INTR_NS_EN_slimbus_intr_en_START (3)
#define SOC_ASP_CFG_R_INTR_NS_EN_slimbus_intr_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int codec_intr_mask : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int slimbus_intr_mask : 1;
        unsigned int reserved_2 : 28;
    } reg;
} SOC_ASP_CFG_R_INTR_NS_MASK_UNION;
#endif
#define SOC_ASP_CFG_R_INTR_NS_MASK_codec_intr_mask_START (0)
#define SOC_ASP_CFG_R_INTR_NS_MASK_codec_intr_mask_END (0)
#define SOC_ASP_CFG_R_INTR_NS_MASK_slimbus_intr_mask_START (3)
#define SOC_ASP_CFG_R_INTR_NS_MASK_slimbus_intr_mask_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_wfq_weight_port2 : 4;
        unsigned int dbg_wfq_weight_port1 : 4;
        unsigned int dbg_wfq_weight_port0 : 4;
        unsigned int dbg_rd_outstd : 4;
        unsigned int dbg_wr_outstd : 4;
        unsigned int dbg_port_sel : 4;
        unsigned int dbg_incr_spen : 1;
        unsigned int dbg_h2x_sel : 1;
        unsigned int dbg_stat_clr : 1;
        unsigned int reserved : 5;
    } reg;
} SOC_ASP_CFG_R_DBG_SET_AHB2AXI_UNION;
#endif
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_wfq_weight_port2_START (0)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_wfq_weight_port2_END (3)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_wfq_weight_port1_START (4)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_wfq_weight_port1_END (7)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_wfq_weight_port0_START (8)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_wfq_weight_port0_END (11)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_rd_outstd_START (12)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_rd_outstd_END (15)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_wr_outstd_START (16)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_wr_outstd_END (19)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_port_sel_START (20)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_port_sel_END (23)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_incr_spen_START (24)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_incr_spen_END (24)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_h2x_sel_START (25)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_h2x_sel_END (25)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_stat_clr_START (26)
#define SOC_ASP_CFG_R_DBG_SET_AHB2AXI_dbg_stat_clr_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rlast_bresp_err : 1;
        unsigned int rdata_err : 1;
        unsigned int rdcmd_gnt : 1;
        unsigned int rdcmd_rearb : 1;
        unsigned int wrcmd_nt : 1;
        unsigned int wrcmd_rearb : 1;
        unsigned int cmd_arrdy : 1;
        unsigned int cmd_awrdy : 1;
        unsigned int rdfifo_full : 1;
        unsigned int rdfifo_empty : 1;
        unsigned int wrfifo_alfull : 1;
        unsigned int wrfifo_empty : 1;
        unsigned int rdfifo_rerr : 1;
        unsigned int rdfifo_werr : 1;
        unsigned int wrfifo_rerr : 1;
        unsigned int wrfifo_werr : 1;
        unsigned int rdcmd_cnt : 4;
        unsigned int rddata_cnt : 4;
        unsigned int wrcmd_cnt : 4;
        unsigned int wrdata_cnt : 4;
    } reg;
} SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_UNION;
#endif
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rlast_bresp_err_START (0)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rlast_bresp_err_END (0)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdata_err_START (1)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdata_err_END (1)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdcmd_gnt_START (2)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdcmd_gnt_END (2)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdcmd_rearb_START (3)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdcmd_rearb_END (3)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrcmd_nt_START (4)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrcmd_nt_END (4)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrcmd_rearb_START (5)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrcmd_rearb_END (5)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_cmd_arrdy_START (6)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_cmd_arrdy_END (6)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_cmd_awrdy_START (7)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_cmd_awrdy_END (7)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdfifo_full_START (8)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdfifo_full_END (8)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdfifo_empty_START (9)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdfifo_empty_END (9)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrfifo_alfull_START (10)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrfifo_alfull_END (10)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrfifo_empty_START (11)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrfifo_empty_END (11)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdfifo_rerr_START (12)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdfifo_rerr_END (12)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdfifo_werr_START (13)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdfifo_werr_END (13)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrfifo_rerr_START (14)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrfifo_rerr_END (14)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrfifo_werr_START (15)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrfifo_werr_END (15)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdcmd_cnt_START (16)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rdcmd_cnt_END (19)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rddata_cnt_START (20)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_rddata_cnt_END (23)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrcmd_cnt_START (24)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrcmd_cnt_END (27)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrdata_cnt_START (28)
#define SOC_ASP_CFG_R_DBG_STATUS_AHB2AXI_wrdata_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dlock_bp_codec : 1;
        unsigned int reserved_0 : 1;
        unsigned int dlock_bp_h2p : 1;
        unsigned int reserved_1 : 1;
        unsigned int dlock_bp_sioaudio : 1;
        unsigned int dlock_bp_siovoice : 1;
        unsigned int dlock_bp_siobt : 1;
        unsigned int reserved_2 : 1;
        unsigned int dlock_bp_slimbus : 1;
        unsigned int dlock_bp_edmac : 1;
        unsigned int dlock_bp_ipc : 1;
        unsigned int dlock_bp_watchdog : 1;
        unsigned int dlock_bp_cfg : 1;
        unsigned int dlock_bp_gpio : 1;
        unsigned int dlock_bp_timer0 : 1;
        unsigned int dlock_bp_timer1 : 1;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_DLOCK_BP_UNION;
#endif
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_codec_START (0)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_codec_END (0)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_h2p_START (2)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_h2p_END (2)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_sioaudio_START (4)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_sioaudio_END (4)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_siovoice_START (5)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_siovoice_END (5)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_siobt_START (6)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_siobt_END (6)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_slimbus_START (8)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_slimbus_END (8)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_edmac_START (9)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_edmac_END (9)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_ipc_START (10)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_ipc_END (10)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_watchdog_START (11)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_watchdog_END (11)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_cfg_START (12)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_cfg_END (12)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_gpio_START (13)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_gpio_END (13)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_timer0_START (14)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_timer0_END (14)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_timer1_START (15)
#define SOC_ASP_CFG_R_DLOCK_BP_dlock_bp_timer1_END (15)
#define SOC_ASP_CFG_R_DLOCK_BP_write_en_START (16)
#define SOC_ASP_CFG_R_DLOCK_BP_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intr_dsp_nmi : 1;
        unsigned int intr0_nonsecure_ipc : 1;
        unsigned int intr1_nonsecure_ipc : 1;
        unsigned int intr0_secure_ipc : 1;
        unsigned int intr1_secure_ipc : 1;
        unsigned int intr_asp_timer0_0 : 1;
        unsigned int intr_asp_timer0_1 : 1;
        unsigned int intr_asp_timer1_0 : 1;
        unsigned int intr_asp_timer1_1 : 1;
        unsigned int intr_sio_audio : 1;
        unsigned int intr_sio_voice : 1;
        unsigned int intr_sio_bt : 1;
        unsigned int mdm_hifi_harq_intr_2ff : 1;
        unsigned int codec_intr : 1;
        unsigned int dma_intr : 1;
        unsigned int dma_intr_ns : 1;
        unsigned int ldrx2hifi_time_int : 1;
        unsigned int intr_addr_monitor : 1;
        unsigned int intr_watch_dog : 1;
        unsigned int ipc_int : 1;
        unsigned int slimbus_intr : 1;
        unsigned int mcpu_hifi_intr : 1;
        unsigned int bbe16_hifi_intr : 1;
        unsigned int i2c_hifi_intr : 1;
        unsigned int intr_gpio_hifi : 1;
        unsigned int tds_hifi_intr : 1;
        unsigned int intr_freq_offset : 1;
        unsigned int freq_offset_intr_flag : 1;
        unsigned int reserved : 4;
    } reg;
} SOC_ASP_CFG_R_DSP_BINTERRUPT_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_dsp_nmi_START (0)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_dsp_nmi_END (0)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr0_nonsecure_ipc_START (1)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr0_nonsecure_ipc_END (1)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr1_nonsecure_ipc_START (2)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr1_nonsecure_ipc_END (2)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr0_secure_ipc_START (3)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr0_secure_ipc_END (3)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr1_secure_ipc_START (4)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr1_secure_ipc_END (4)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_asp_timer0_0_START (5)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_asp_timer0_0_END (5)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_asp_timer0_1_START (6)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_asp_timer0_1_END (6)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_asp_timer1_0_START (7)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_asp_timer1_0_END (7)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_asp_timer1_1_START (8)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_asp_timer1_1_END (8)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_sio_audio_START (9)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_sio_audio_END (9)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_sio_voice_START (10)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_sio_voice_END (10)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_sio_bt_START (11)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_sio_bt_END (11)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_mdm_hifi_harq_intr_2ff_START (12)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_mdm_hifi_harq_intr_2ff_END (12)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_codec_intr_START (13)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_codec_intr_END (13)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_dma_intr_START (14)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_dma_intr_END (14)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_dma_intr_ns_START (15)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_dma_intr_ns_END (15)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_ldrx2hifi_time_int_START (16)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_ldrx2hifi_time_int_END (16)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_addr_monitor_START (17)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_addr_monitor_END (17)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_watch_dog_START (18)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_watch_dog_END (18)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_ipc_int_START (19)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_ipc_int_END (19)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_slimbus_intr_START (20)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_slimbus_intr_END (20)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_mcpu_hifi_intr_START (21)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_mcpu_hifi_intr_END (21)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_bbe16_hifi_intr_START (22)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_bbe16_hifi_intr_END (22)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_i2c_hifi_intr_START (23)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_i2c_hifi_intr_END (23)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_gpio_hifi_START (24)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_gpio_hifi_END (24)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_tds_hifi_intr_START (25)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_tds_hifi_intr_END (25)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_freq_offset_START (26)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_intr_freq_offset_END (26)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_freq_offset_intr_flag_START (27)
#define SOC_ASP_CFG_R_DSP_BINTERRUPT_freq_offset_intr_flag_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dram0_pgt_ctrl : 3;
        unsigned int dram1_pgt_ctrl : 3;
        unsigned int iram0_pgt_ctrl : 3;
        unsigned int reserved : 7;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_DSP_RAM_RET_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_RAM_RET_dram0_pgt_ctrl_START (0)
#define SOC_ASP_CFG_R_DSP_RAM_RET_dram0_pgt_ctrl_END (2)
#define SOC_ASP_CFG_R_DSP_RAM_RET_dram1_pgt_ctrl_START (3)
#define SOC_ASP_CFG_R_DSP_RAM_RET_dram1_pgt_ctrl_END (5)
#define SOC_ASP_CFG_R_DSP_RAM_RET_iram0_pgt_ctrl_START (6)
#define SOC_ASP_CFG_R_DSP_RAM_RET_iram0_pgt_ctrl_END (8)
#define SOC_ASP_CFG_R_DSP_RAM_RET_write_en_START (16)
#define SOC_ASP_CFG_R_DSP_RAM_RET_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int tz_secure_codec_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int tz_secure_sioaudio_n : 1;
        unsigned int tz_secure_siovoice_n : 1;
        unsigned int tz_secure_siobt_n : 1;
        unsigned int reserved_2 : 1;
        unsigned int tz_secure_slimbus_n : 1;
        unsigned int tz_secure_edamc_n : 1;
        unsigned int tz_secure_ipc_n : 1;
        unsigned int tz_secure_watchdog_n : 1;
        unsigned int tz_secure_dsp_n : 1;
        unsigned int tz_secure_gpio_n : 1;
        unsigned int tz_secure_timer0_n : 1;
        unsigned int tz_secure_timer1_n : 1;
        unsigned int dsp_mst_secure_n : 1;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_TZ_SECURE_N_UNION;
#endif
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_codec_n_START (0)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_codec_n_END (0)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_sioaudio_n_START (3)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_sioaudio_n_END (3)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_siovoice_n_START (4)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_siovoice_n_END (4)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_siobt_n_START (5)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_siobt_n_END (5)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_slimbus_n_START (7)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_slimbus_n_END (7)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_edamc_n_START (8)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_edamc_n_END (8)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_ipc_n_START (9)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_ipc_n_END (9)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_watchdog_n_START (10)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_watchdog_n_END (10)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_dsp_n_START (11)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_dsp_n_END (11)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_gpio_n_START (12)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_gpio_n_END (12)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_timer0_n_START (13)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_timer0_n_END (13)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_timer1_n_START (14)
#define SOC_ASP_CFG_R_TZ_SECURE_N_tz_secure_timer1_n_END (14)
#define SOC_ASP_CFG_R_TZ_SECURE_N_dsp_mst_secure_n_START (15)
#define SOC_ASP_CFG_R_TZ_SECURE_N_dsp_mst_secure_n_END (15)
#define SOC_ASP_CFG_R_TZ_SECURE_N_write_en_START (16)
#define SOC_ASP_CFG_R_TZ_SECURE_N_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ocram_r0size : 10;
        unsigned int reserved : 22;
    } reg;
} SOC_ASP_CFG_R_OCRAM_R0SIZE_UNION;
#endif
#define SOC_ASP_CFG_R_OCRAM_R0SIZE_ocram_r0size_START (0)
#define SOC_ASP_CFG_R_OCRAM_R0SIZE_ocram_r0size_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sio_audio_loop_secure_n : 1;
        unsigned int sio_voice_loop_secure_n : 1;
        unsigned int sio_bt_loop_secure_n : 1;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 28;
    } reg;
} SOC_ASP_CFG_R_SIO_LOOP_SECURE_N_UNION;
#endif
#define SOC_ASP_CFG_R_SIO_LOOP_SECURE_N_sio_audio_loop_secure_n_START (0)
#define SOC_ASP_CFG_R_SIO_LOOP_SECURE_N_sio_audio_loop_secure_n_END (0)
#define SOC_ASP_CFG_R_SIO_LOOP_SECURE_N_sio_voice_loop_secure_n_START (1)
#define SOC_ASP_CFG_R_SIO_LOOP_SECURE_N_sio_voice_loop_secure_n_END (1)
#define SOC_ASP_CFG_R_SIO_LOOP_SECURE_N_sio_bt_loop_secure_n_START (2)
#define SOC_ASP_CFG_R_SIO_LOOP_SECURE_N_sio_bt_loop_secure_n_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 29;
    } reg;
} SOC_ASP_CFG_R_INTR_S_INI_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 29;
    } reg;
} SOC_ASP_CFG_R_INTR_S_EN_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0: 1;
        unsigned int reserved_1: 1;
        unsigned int reserved_2: 1;
        unsigned int reserved_3: 29;
    } reg;
} SOC_ASP_CFG_R_INTR_S_MASK_UNION;
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int remap_length : 8;
        unsigned int dsp_addr_remapping_en : 1;
        unsigned int reserved : 23;
    } reg;
} SOC_ASP_CFG_R_DSP_REMAPPING_EN_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_REMAPPING_EN_remap_length_START (0)
#define SOC_ASP_CFG_R_DSP_REMAPPING_EN_remap_length_END (7)
#define SOC_ASP_CFG_R_DSP_REMAPPING_EN_dsp_addr_remapping_en_START (8)
#define SOC_ASP_CFG_R_DSP_REMAPPING_EN_dsp_addr_remapping_en_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 20;
        unsigned int asp_reserved_start_addr : 12;
    } reg;
} SOC_ASP_CFG_R_DSP_REMAPPING_SRC_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_REMAPPING_SRC_BASE_ADDR_asp_reserved_start_addr_START (20)
#define SOC_ASP_CFG_R_DSP_REMAPPING_SRC_BASE_ADDR_asp_reserved_start_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 20;
        unsigned int ddr_start_addr : 12;
    } reg;
} SOC_ASP_CFG_R_DSP_REMAPPING_DES_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_DSP_REMAPPING_DES_BASE_ADDR_ddr_start_addr_START (20)
#define SOC_ASP_CFG_R_DSP_REMAPPING_DES_BASE_ADDR_ddr_start_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int remap_length : 9;
        unsigned int remap_en : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_ASP_CFG_R_DDR_HARQ_REMAP_EN_UNION;
#endif
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_EN_remap_length_START (0)
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_EN_remap_length_END (8)
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_EN_remap_en_START (9)
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_EN_remap_en_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int src_base_addr : 20;
    } reg;
} SOC_ASP_CFG_R_DDR_HARQ_REMAP_SRC_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_SRC_BASE_ADDR_src_base_addr_START (12)
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_SRC_BASE_ADDR_src_base_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int des_base_addr : 20;
    } reg;
} SOC_ASP_CFG_R_DDR_HARQ_REMAP_DES_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_DES_BASE_ADDR_des_base_addr_START (12)
#define SOC_ASP_CFG_R_DDR_HARQ_REMAP_DES_BASE_ADDR_des_base_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int remap_length : 9;
        unsigned int remap_en : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_ASP_CFG_R_DDR_MMBUF_REMAP_EN_UNION;
#endif
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_EN_remap_length_START (0)
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_EN_remap_length_END (8)
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_EN_remap_en_START (9)
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_EN_remap_en_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int src_base_addr : 20;
    } reg;
} SOC_ASP_CFG_R_DDR_MMBUF_REMAP_SRC_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_SRC_BASE_ADDR_src_base_addr_START (12)
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_SRC_BASE_ADDR_src_base_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int des_base_addr : 20;
    } reg;
} SOC_ASP_CFG_R_DDR_MMBUF_REMAP_DES_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_DES_BASE_ADDR_des_base_addr_START (12)
#define SOC_ASP_CFG_R_DDR_MMBUF_REMAP_DES_BASE_ADDR_des_base_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int remap_length : 9;
        unsigned int remap_en : 1;
        unsigned int reserved : 22;
    } reg;
} SOC_ASP_CFG_R_DDR_OCRAM_REMAP_EN_UNION;
#endif
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_EN_remap_length_START (0)
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_EN_remap_length_END (8)
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_EN_remap_en_START (9)
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_EN_remap_en_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int src_base_addr : 20;
    } reg;
} SOC_ASP_CFG_R_DDR_OCRAM_REMAP_SRC_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_SRC_BASE_ADDR_src_base_addr_START (12)
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_SRC_BASE_ADDR_src_base_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int des_base_addr : 20;
    } reg;
} SOC_ASP_CFG_R_DDR_OCRAM_REMAP_DES_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_DES_BASE_ADDR_des_base_addr_START (12)
#define SOC_ASP_CFG_R_DDR_OCRAM_REMAP_DES_BASE_ADDR_des_base_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ppll0_en : 1;
        unsigned int fll_en : 1;
        unsigned int mmbuf_clk_en : 1;
        unsigned int aobus_clk_en : 1;
        unsigned int mmbuf_clk_sel : 2;
        unsigned int aobus_clk_sel : 2;
        unsigned int reserved : 8;
        unsigned int write_en : 16;
    } reg;
} SOC_ASP_CFG_R_MMBUF_CTRL_UNION;
#endif
#define SOC_ASP_CFG_R_MMBUF_CTRL_ppll0_en_START (0)
#define SOC_ASP_CFG_R_MMBUF_CTRL_ppll0_en_END (0)
#define SOC_ASP_CFG_R_MMBUF_CTRL_fll_en_START (1)
#define SOC_ASP_CFG_R_MMBUF_CTRL_fll_en_END (1)
#define SOC_ASP_CFG_R_MMBUF_CTRL_mmbuf_clk_en_START (2)
#define SOC_ASP_CFG_R_MMBUF_CTRL_mmbuf_clk_en_END (2)
#define SOC_ASP_CFG_R_MMBUF_CTRL_aobus_clk_en_START (3)
#define SOC_ASP_CFG_R_MMBUF_CTRL_aobus_clk_en_END (3)
#define SOC_ASP_CFG_R_MMBUF_CTRL_mmbuf_clk_sel_START (4)
#define SOC_ASP_CFG_R_MMBUF_CTRL_mmbuf_clk_sel_END (5)
#define SOC_ASP_CFG_R_MMBUF_CTRL_aobus_clk_sel_START (6)
#define SOC_ASP_CFG_R_MMBUF_CTRL_aobus_clk_sel_END (7)
#define SOC_ASP_CFG_R_MMBUF_CTRL_write_en_START (16)
#define SOC_ASP_CFG_R_MMBUF_CTRL_write_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int hifidsp_idle_time_cfgcnt : 8;
        unsigned int div_hifidsp_clk_auto : 3;
        unsigned int reserved_0 : 1;
        unsigned int reserved_1 : 1;
        unsigned int reserved_2 : 1;
        unsigned int dsp_pwaitmode_bypass : 1;
        unsigned int dw_axi_m1_st_bypass : 1;
        unsigned int dw_axi_m2_st_bypass : 1;
        unsigned int dw_axi_m3_st_bypass : 1;
        unsigned int dw_axi_m4_st_bypass : 1;
        unsigned int reserved_3 : 13;
    } reg;
} SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_UNION;
#endif
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_hifidsp_idle_time_cfgcnt_START (0)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_hifidsp_idle_time_cfgcnt_END (7)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_div_hifidsp_clk_auto_START (8)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_div_hifidsp_clk_auto_END (10)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dsp_pwaitmode_bypass_START (14)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dsp_pwaitmode_bypass_END (14)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dw_axi_m1_st_bypass_START (15)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dw_axi_m1_st_bypass_END (15)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dw_axi_m2_st_bypass_START (16)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dw_axi_m2_st_bypass_END (16)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dw_axi_m3_st_bypass_START (17)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dw_axi_m3_st_bypass_END (17)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dw_axi_m4_st_bypass_START (18)
#define SOC_ASP_CFG_R_HIFIDSP_CLK_DIV_AUTO_dw_axi_m4_st_bypass_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int freq_offset_en : 1;
        unsigned int freq_offset_cnt_freq : 2;
        unsigned int intr_freq_offset_clr : 1;
        unsigned int intr_freq_offset_msk : 1;
        unsigned int reserved_0 : 3;
        unsigned int freq_offset_limen : 8;
        unsigned int reserved_1 : 16;
    } reg;
} SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_UNION;
#endif
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_freq_offset_en_START (0)
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_freq_offset_en_END (0)
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_freq_offset_cnt_freq_START (1)
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_freq_offset_cnt_freq_END (2)
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_intr_freq_offset_clr_START (3)
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_intr_freq_offset_clr_END (3)
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_intr_freq_offset_msk_START (4)
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_intr_freq_offset_msk_END (4)
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_freq_offset_limen_START (8)
#define SOC_ASP_CFG_R_INTR_FREQ_OFFSET_CFG_freq_offset_limen_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor0_en : 1;
        unsigned int addr_monitor1_en : 1;
        unsigned int addr_monitor2_en : 1;
        unsigned int addr_monitor3_en : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_addr_monitor0_en_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_addr_monitor0_en_END (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_addr_monitor1_en_START (1)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_addr_monitor1_en_END (1)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_addr_monitor2_en_START (2)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_addr_monitor2_en_END (2)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_addr_monitor3_en_START (3)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_EN_ADDR_addr_monitor3_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor_intr_clr : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_CLR_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_CLR_ADDR_addr_monitor_intr_clr_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_CLR_ADDR_addr_monitor_intr_clr_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor0_intr_en : 1;
        unsigned int addr_monitor1_intr_en : 1;
        unsigned int addr_monitor2_intr_en : 1;
        unsigned int addr_monitor3_intr_en : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_addr_monitor0_intr_en_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_addr_monitor0_intr_en_END (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_addr_monitor1_intr_en_START (1)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_addr_monitor1_intr_en_END (1)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_addr_monitor2_intr_en_START (2)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_addr_monitor2_intr_en_END (2)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_addr_monitor3_intr_en_START (3)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_EN_ADDR_addr_monitor3_intr_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor0_arhit_intr_stat : 1;
        unsigned int addr_monitor0_awhit_intr_stat : 1;
        unsigned int addr_monitor1_arhit_intr_stat : 1;
        unsigned int addr_monitor1_awhit_intr_stat : 1;
        unsigned int addr_monitor2_arhit_intr_stat : 1;
        unsigned int addr_monitor2_awhit_intr_stat : 1;
        unsigned int addr_monitor3_arhit_intr_stat : 1;
        unsigned int addr_monitor3_awhit_intr_stat : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor0_arhit_intr_stat_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor0_arhit_intr_stat_END (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor0_awhit_intr_stat_START (1)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor0_awhit_intr_stat_END (1)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor1_arhit_intr_stat_START (2)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor1_arhit_intr_stat_END (2)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor1_awhit_intr_stat_START (3)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor1_awhit_intr_stat_END (3)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor2_arhit_intr_stat_START (4)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor2_arhit_intr_stat_END (4)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor2_awhit_intr_stat_START (5)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor2_awhit_intr_stat_END (5)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor3_arhit_intr_stat_START (6)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor3_arhit_intr_stat_END (6)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor3_awhit_intr_stat_START (7)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR_INTR_STAT_ADDR_addr_monitor3_awhit_intr_stat_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int addr_monitor0_base : 20;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_BASE_ADDR_addr_monitor0_base_START (12)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_BASE_ADDR_addr_monitor0_base_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int addr_monitor0_len : 20;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_LEN_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_LEN_ADDR_addr_monitor0_len_START (12)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_LEN_ADDR_addr_monitor0_len_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor0_hit_awaddr : 32;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_HIT_AWADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_HIT_AWADDR_addr_monitor0_hit_awaddr_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_HIT_AWADDR_addr_monitor0_hit_awaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor0_hit_araddr : 32;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_HIT_ARADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_HIT_ARADDR_addr_monitor0_hit_araddr_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR0_HIT_ARADDR_addr_monitor0_hit_araddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int addr_monitor1_base : 20;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_BASE_ADDR_addr_monitor1_base_START (12)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_BASE_ADDR_addr_monitor1_base_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int addr_monitor1_len : 20;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_LEN_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_LEN_ADDR_addr_monitor1_len_START (12)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_LEN_ADDR_addr_monitor1_len_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor1_hit_awaddr : 32;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_HIT_AWADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_HIT_AWADDR_addr_monitor1_hit_awaddr_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_HIT_AWADDR_addr_monitor1_hit_awaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor1_hit_araddr : 32;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_HIT_ARADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_HIT_ARADDR_addr_monitor1_hit_araddr_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR1_HIT_ARADDR_addr_monitor1_hit_araddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int addr_monitor2_base : 20;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_BASE_ADDR_addr_monitor2_base_START (12)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_BASE_ADDR_addr_monitor2_base_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int addr_monitor2_len : 20;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_LEN_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_LEN_ADDR_addr_monitor2_len_START (12)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_LEN_ADDR_addr_monitor2_len_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor2_hit_awaddr : 32;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_HIT_AWADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_HIT_AWADDR_addr_monitor2_hit_awaddr_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_HIT_AWADDR_addr_monitor2_hit_awaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor2_hit_araddr : 32;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_HIT_ARADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_HIT_ARADDR_addr_monitor2_hit_araddr_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR2_HIT_ARADDR_addr_monitor2_hit_araddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int addr_monitor3_base : 20;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_BASE_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_BASE_ADDR_addr_monitor3_base_START (12)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_BASE_ADDR_addr_monitor3_base_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 12;
        unsigned int addr_monitor3_len : 20;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_LEN_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_LEN_ADDR_addr_monitor3_len_START (12)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_LEN_ADDR_addr_monitor3_len_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor3_hit_awaddr : 32;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_HIT_AWADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_HIT_AWADDR_addr_monitor3_hit_awaddr_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_HIT_AWADDR_addr_monitor3_hit_awaddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_monitor3_hit_araddr : 32;
    } reg;
} SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_HIT_ARADDR_UNION;
#endif
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_HIT_ARADDR_addr_monitor3_hit_araddr_START (0)
#define SOC_ASP_CFG_R_ACCESS_ADDR_MONITOR3_HIT_ARADDR_addr_monitor3_hit_araddr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mem_ctrl_s : 16;
        unsigned int mem_ctrl_tp : 16;
    } reg;
} SOC_ASP_CFG_R_MEMORY_CTRL_UNION;
#endif
#define SOC_ASP_CFG_R_MEMORY_CTRL_mem_ctrl_s_START (0)
#define SOC_ASP_CFG_R_MEMORY_CTRL_mem_ctrl_s_END (15)
#define SOC_ASP_CFG_R_MEMORY_CTRL_mem_ctrl_tp_START (16)
#define SOC_ASP_CFG_R_MEMORY_CTRL_mem_ctrl_tp_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int slimbus_primary : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ASP_CFG_R_SLIMBUS_PRIMARY_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_SLIMBUS_PRIMARY_ADDR_slimbus_primary_START (0)
#define SOC_ASP_CFG_R_SLIMBUS_PRIMARY_ADDR_slimbus_primary_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int slimbus_id : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_ASP_CFG_R_SLIMBUS_ID_ADDR_UNION;
#endif
#define SOC_ASP_CFG_R_SLIMBUS_ID_ADDR_slimbus_id_START (0)
#define SOC_ASP_CFG_R_SLIMBUS_ID_ADDR_slimbus_id_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dat_fmt_cfg_0 : 2;
        unsigned int dat_fmt_cfg_1 : 2;
        unsigned int dat_fmt_cfg_2 : 2;
        unsigned int dat_fmt_cfg_3 : 2;
        unsigned int dat_fmt_cfg_4 : 2;
        unsigned int dat_fmt_cfg_5 : 2;
        unsigned int dat_fmt_cfg_6 : 2;
        unsigned int dat_fmt_cfg_7 : 2;
        unsigned int dma_mask_clr : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_UNION;
#endif
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_0_START (0)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_0_END (1)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_1_START (2)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_1_END (3)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_2_START (4)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_2_END (5)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_3_START (6)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_3_END (7)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_4_START (8)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_4_END (9)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_5_START (10)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_5_END (11)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_6_START (12)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_6_END (13)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_7_START (14)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dat_fmt_cfg_7_END (15)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dma_mask_clr_START (16)
#define SOC_ASP_CFG_R_SLIM_DAT_FMT_CFG_dma_mask_clr_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bypass : 1;
        unsigned int reserved : 7;
        unsigned int chl_rght_fst : 8;
        unsigned int dat_rght_msb : 8;
        unsigned int dat_rght_fst : 8;
    } reg;
} SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_UNION;
#endif
#define SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_bypass_START (0)
#define SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_bypass_END (0)
#define SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_chl_rght_fst_START (8)
#define SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_chl_rght_fst_END (15)
#define SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_dat_rght_msb_START (16)
#define SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_dat_rght_msb_END (23)
#define SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_dat_rght_fst_START (24)
#define SOC_ASP_CFG_R_SLIM_DAT_CHNL_CFG_dat_rght_fst_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int div_slimbus_clk : 27;
        unsigned int reserved : 5;
    } reg;
} SOC_ASP_CFG_R_CLK0_DIV_UNION;
#endif
#define SOC_ASP_CFG_R_CLK0_DIV_div_slimbus_clk_START (0)
#define SOC_ASP_CFG_R_CLK0_DIV_div_slimbus_clk_END (26)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int voice_wakeup_intr_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_ASP_CFG_R_VOICE_WAKEUP_INTR_EN_UNION;
#endif
#define SOC_ASP_CFG_R_VOICE_WAKEUP_INTR_EN_voice_wakeup_intr_en_START (0)
#define SOC_ASP_CFG_R_VOICE_WAKEUP_INTR_EN_voice_wakeup_intr_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int codec_dma_sel : 1;
        unsigned int codec_i2s_sel : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_ASP_CFG_R_CODEC_SEL_EN_UNION;
#endif
#define SOC_ASP_CFG_R_CODEC_SEL_EN_codec_dma_sel_START (0)
#define SOC_ASP_CFG_R_CODEC_SEL_EN_codec_dma_sel_END (0)
#define SOC_ASP_CFG_R_CODEC_SEL_EN_codec_i2s_sel_START (1)
#define SOC_ASP_CFG_R_CODEC_SEL_EN_codec_i2s_sel_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int secure_en_ddr_remap : 1;
        unsigned int secure_en_ddr_harq_remap : 1;
        unsigned int secure_en_ddr_mmbuf_remap : 1;
        unsigned int secure_en_ddr_ocram_remap : 1;
        unsigned int reserved : 28;
    } reg;
} SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_UNION;
#endif
#define SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_secure_en_ddr_remap_START (0)
#define SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_secure_en_ddr_remap_END (0)
#define SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_secure_en_ddr_harq_remap_START (1)
#define SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_secure_en_ddr_harq_remap_END (1)
#define SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_secure_en_ddr_mmbuf_remap_START (2)
#define SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_secure_en_ddr_mmbuf_remap_END (2)
#define SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_secure_en_ddr_ocram_remap_START (3)
#define SOC_ASP_CFG_R_SECURE_AUTHORITY_EN_secure_en_ddr_ocram_remap_END (3)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
