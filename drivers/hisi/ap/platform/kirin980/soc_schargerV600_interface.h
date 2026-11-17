#ifndef __SOC_SCHARGER_INTERFACE_H__
#define __SOC_SCHARGER_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_SCHARGER_VENDIDL_ADDR(base) ((base) + (0x00))
#define SOC_SCHARGER_VENDIDH_ADDR(base) ((base) + (0x01))
#define SOC_SCHARGER_PRODIDL_ADDR(base) ((base) + (0x02))
#define SOC_SCHARGER_PRODIDH_ADDR(base) ((base) + (0x03))
#define SOC_SCHARGER_DEVIDL_ADDR(base) ((base) + (0x04))
#define SOC_SCHARGER_DEVIDH_ADDR(base) ((base) + (0x05))
#define SOC_SCHARGER_TYPECREVL_ADDR(base) ((base) + (0x06))
#define SOC_SCHARGER_TYPECREVH_ADDR(base) ((base) + (0x07))
#define SOC_SCHARGER_USBPDVER_ADDR(base) ((base) + (0x08))
#define SOC_SCHARGER_USBPDREV_ADDR(base) ((base) + (0x09))
#define SOC_SCHARGER_PDIFREVL_ADDR(base) ((base) + (0x0A))
#define SOC_SCHARGER_PDIFREVH_ADDR(base) ((base) + (0x0B))
#define SOC_SCHARGER_PD_ALERT_L_ADDR(base) ((base) + (0x10))
#define SOC_SCHARGER_PD_ALERT_H_ADDR(base) ((base) + (0x11))
#define SOC_SCHARGER_PD_ALERT_MSK_L_ADDR(base) ((base) + (0x12))
#define SOC_SCHARGER_PD_ALERT_MSK_H_ADDR(base) ((base) + (0x13))
#define SOC_SCHARGER_PD_PWRSTAT_MSK_ADDR(base) ((base) + (0x14))
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_ADDR(base) ((base) + (0x15))
#define SOC_SCHARGER_PD_TCPC_CTRL_ADDR(base) ((base) + (0x19))
#define SOC_SCHARGER_PD_ROLE_CTRL_ADDR(base) ((base) + (0x1A))
#define SOC_SCHARGER_PD_FAULT_CTRL_ADDR(base) ((base) + (0x1B))
#define SOC_SCHARGER_PD_PWR_CTRL_ADDR(base) ((base) + (0x1C))
#define SOC_SCHARGER_PD_CC_STATUS_ADDR(base) ((base) + (0x1D))
#define SOC_SCHARGER_PD_PWR_STATUS_ADDR(base) ((base) + (0x1E))
#define SOC_SCHARGER_PD_FAULT_STATUS_ADDR(base) ((base) + (0x1F))
#define SOC_SCHARGER_PD_COMMAND_ADDR(base) ((base) + (0x23))
#define SOC_SCHARGER_PD_DEVCAP1L_ADDR(base) ((base) + (0x24))
#define SOC_SCHARGER_PD_DEVCAP1H_ADDR(base) ((base) + (0x25))
#define SOC_SCHARGER_PD_DEVCAP2L_ADDR(base) ((base) + (0x26))
#define SOC_SCHARGER_PD_DEVCAP2H_ADDR(base) ((base) + (0x27))
#define SOC_SCHARGER_PD_STDIN_CAP_ADDR(base) ((base) + (0x28))
#define SOC_SCHARGER_PD_STDOUT_CAP_ADDR(base) ((base) + (0x29))
#define SOC_SCHARGER_PD_MSG_HEADR_ADDR(base) ((base) + (0x2E))
#define SOC_SCHARGER_PD_RXDETECT_ADDR(base) ((base) + (0x2F))
#define SOC_SCHARGER_PD_RXBYTECNT_ADDR(base) ((base) + (0x30))
#define SOC_SCHARGER_PD_RXTYPE_ADDR(base) ((base) + (0x31))
#define SOC_SCHARGER_PD_RXHEADL_ADDR(base) ((base) + (0x32))
#define SOC_SCHARGER_PD_RXHEADH_ADDR(base) ((base) + (0x33))
#define SOC_SCHARGER_PD_RXDATA_0_ADDR(base) ((base) + (k*1+0x34))
#define SOC_SCHARGER_PD_TRANSMIT_ADDR(base) ((base) + (0x50))
#define SOC_SCHARGER_PD_TXBYTECNT_ADDR(base) ((base) + (0x51))
#define SOC_SCHARGER_PD_TXHEADL_ADDR(base) ((base) + (0x52))
#define SOC_SCHARGER_PD_TXHEADH_ADDR(base) ((base) + (0x53))
#define SOC_SCHARGER_PD_TXDATA_ADDR(base) ((base) + (k*1+0x54))
#define SOC_SCHARGER_PD_VBUS_VOL_L_ADDR(base) ((base) + (0x70))
#define SOC_SCHARGER_PD_VBUS_VOL_H_ADDR(base) ((base) + (0x71))
#define SOC_SCHARGER_PD_VBUS_SNK_DISCL_ADDR(base) ((base) + (0x72))
#define SOC_SCHARGER_PD_VBUS_SNK_DISCH_ADDR(base) ((base) + (0x73))
#define SOC_SCHARGER_PD_VBUS_STOP_DISCL_ADDR(base) ((base) + (0x74))
#define SOC_SCHARGER_PD_VBUS_STOP_DISCH_ADDR(base) ((base) + (0x75))
#define SOC_SCHARGER_PD_VALARMH_CFGL_ADDR(base) ((base) + (0x76))
#define SOC_SCHARGER_PD_VALARMH_CFGH_ADDR(base) ((base) + (0x77))
#define SOC_SCHARGER_PD_VALARML_CFGL_ADDR(base) ((base) + (0x78))
#define SOC_SCHARGER_PD_VALARML_CFGH_ADDR(base) ((base) + (0x79))
#define SOC_SCHARGER_PD_VDM_CFG_0_ADDR(base) ((base) + (0x7A))
#define SOC_SCHARGER_PD_VDM_ENABLE_ADDR(base) ((base) + (0x7B))
#define SOC_SCHARGER_PD_VDM_CFG_1_ADDR(base) ((base) + (0x7C))
#define SOC_SCHARGER_PD_DBG_RDATA_CFG_ADDR(base) ((base) + (0x7D))
#define SOC_SCHARGER_PD_DBG_RDATA_ADDR(base) ((base) + (0x7E))
#define SOC_SCHARGER_VDM_PAGE_SELECT_ADDR(base) ((base) + (0x7F))
 #define PAGE0_BASE (0x0080)
#define SOC_SCHARGER_STATUIS_ADDR(base) ((base) + (0x00) + PAGE0_BASE)
#define SOC_SCHARGER_CNTL_ADDR(base) ((base) + (0x01) + PAGE0_BASE)
#define SOC_SCHARGER_CMD_ADDR(base) ((base) + (0x04) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_LENGTH_ADDR(base) ((base) + (0x05) + PAGE0_BASE)
#define SOC_SCHARGER_ADDR_ADDR(base) ((base) + (0x07) + PAGE0_BASE)
#define SOC_SCHARGER_DATA0_ADDR(base) ((base) + (0x08) + PAGE0_BASE)
#define SOC_SCHARGER_DATA1_ADDR(base) ((base) + (0x09) + PAGE0_BASE)
#define SOC_SCHARGER_DATA2_ADDR(base) ((base) + (0x0A) + PAGE0_BASE)
#define SOC_SCHARGER_DATA3_ADDR(base) ((base) + (0x0B) + PAGE0_BASE)
#define SOC_SCHARGER_DATA4_ADDR(base) ((base) + (0x0C) + PAGE0_BASE)
#define SOC_SCHARGER_DATA5_ADDR(base) ((base) + (0x0D) + PAGE0_BASE)
#define SOC_SCHARGER_DATA6_ADDR(base) ((base) + (0x0E) + PAGE0_BASE)
#define SOC_SCHARGER_DATA7_ADDR(base) ((base) + (0x0F) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_RDATA0_ADDR(base) ((base) + (0x10) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_RDATA1_ADDR(base) ((base) + (0x11) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_RDATA2_ADDR(base) ((base) + (0x12) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_RDATA3_ADDR(base) ((base) + (0x13) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_RDATA4_ADDR(base) ((base) + (0x14) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_RDATA5_ADDR(base) ((base) + (0x15) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_RDATA6_ADDR(base) ((base) + (0x16) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_RDATA7_ADDR(base) ((base) + (0x17) + PAGE0_BASE)
#define SOC_SCHARGER_ISR1_ADDR(base) ((base) + (0x19) + PAGE0_BASE)
#define SOC_SCHARGER_ISR2_ADDR(base) ((base) + (0x1A) + PAGE0_BASE)
#define SOC_SCHARGER_IMR1_ADDR(base) ((base) + (0x1B) + PAGE0_BASE)
#define SOC_SCHARGER_IMR2_ADDR(base) ((base) + (0x1C) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_IRQ5_ADDR(base) ((base) + (0x1D) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_IRQ5_MASK_ADDR(base) ((base) + (0x1E) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_CTRL_ADDR(base) ((base) + (0x1F) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_MODE2_SET_ADDR(base) ((base) + (0x20) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_ADAP_CTRL_ADDR(base) ((base) + (0x21) + PAGE0_BASE)
#define SOC_SCHARGER_RDATA_READY_ADDR(base) ((base) + (0x22) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_SOFT_RST_CTRL_ADDR(base) ((base) + (0x23) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_ADDR(base) ((base) + (0x25) + PAGE0_BASE)
#define SOC_SCHARGER_FCP_INIT_RETRY_CFG_ADDR(base) ((base) + (0x26) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_SOFT_RST_CTRL_ADDR(base) ((base) + (0x28) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_CTRL0_ADDR(base) ((base) + (0x29) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_START_ADDR(base) ((base) + (0x2A) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_CTRL1_ADDR(base) ((base) + (0x2B) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_SEQ_CH_H_ADDR(base) ((base) + (0x2C) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_SEQ_CH_L_ADDR(base) ((base) + (0x2D) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_CTRL2_ADDR(base) ((base) + (0x2E) + PAGE0_BASE)
#define SOC_SCHARGER_SOH_DISCHG_EN_ADDR(base) ((base) + (0x2F) + PAGE0_BASE)
#define SOC_SCHARGER_ACR_CTRL_ADDR(base) ((base) + (0x30) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_RD_SEQ_ADDR(base) ((base) + (0x31) + PAGE0_BASE)
#define SOC_SCHARGER_PULSE_NON_CHG_FLAG_ADDR(base) ((base) + (0x32) + PAGE0_BASE)
#define SOC_SCHARGER_VUSB_ADC_L_ADDR(base) ((base) + (0x33) + PAGE0_BASE)
#define SOC_SCHARGER_VUSB_ADC_H_ADDR(base) ((base) + (0x34) + PAGE0_BASE)
#define SOC_SCHARGER_IBUS_ADC_L_ADDR(base) ((base) + (0x35) + PAGE0_BASE)
#define SOC_SCHARGER_IBUS_ADC_H_ADDR(base) ((base) + (0x36) + PAGE0_BASE)
#define SOC_SCHARGER_VBUS_ADC_L_ADDR(base) ((base) + (0x37) + PAGE0_BASE)
#define SOC_SCHARGER_VBUS_ADC_H_ADDR(base) ((base) + (0x38) + PAGE0_BASE)
#define SOC_SCHARGER_VOUT_ADC_L_ADDR(base) ((base) + (0x39) + PAGE0_BASE)
#define SOC_SCHARGER_VOUT_ADC_H_ADDR(base) ((base) + (0x3A) + PAGE0_BASE)
#define SOC_SCHARGER_VBAT_ADC_L_ADDR(base) ((base) + (0x3B) + PAGE0_BASE)
#define SOC_SCHARGER_VBAT_ADC_H_ADDR(base) ((base) + (0x3C) + PAGE0_BASE)
#define SOC_SCHARGER_IBAT_ADC_L_ADDR(base) ((base) + (0x3D) + PAGE0_BASE)
#define SOC_SCHARGER_IBAT_ADC_H_ADDR(base) ((base) + (0x3E) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_6_L_ADDR(base) ((base) + (0x3F) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_6_H_ADDR(base) ((base) + (0x40) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_7_L_ADDR(base) ((base) + (0x41) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_7_H_ADDR(base) ((base) + (0x42) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_8_L_ADDR(base) ((base) + (0x43) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_8_H_ADDR(base) ((base) + (0x44) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_9_L_ADDR(base) ((base) + (0x45) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_9_H_ADDR(base) ((base) + (0x46) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_10_L_ADDR(base) ((base) + (0x47) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_10_H_ADDR(base) ((base) + (0x48) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_11_L_ADDR(base) ((base) + (0x49) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_11_H_ADDR(base) ((base) + (0x4A) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_12_L_ADDR(base) ((base) + (0x4B) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_12_H_ADDR(base) ((base) + (0x4C) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_13_L_ADDR(base) ((base) + (0x4D) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_13_H_ADDR(base) ((base) + (0x4E) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_14_L_ADDR(base) ((base) + (0x4F) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_14_H_ADDR(base) ((base) + (0x50) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_15_L_ADDR(base) ((base) + (0x51) + PAGE0_BASE)
#define SOC_SCHARGER_HKADC_DATA_15_H_ADDR(base) ((base) + (0x52) + PAGE0_BASE)
#define SOC_SCHARGER_PD_CDR_CFG_0_ADDR(base) ((base) + (0x58) + PAGE0_BASE)
#define SOC_SCHARGER_PD_CDR_CFG_1_ADDR(base) ((base) + (0x59) + PAGE0_BASE)
#define SOC_SCHARGER_PD_DBG_CFG_0_ADDR(base) ((base) + (0x5A) + PAGE0_BASE)
#define SOC_SCHARGER_PD_DBG_CFG_1_ADDR(base) ((base) + (0x5B) + PAGE0_BASE)
#define SOC_SCHARGER_PD_DBG_RO_0_ADDR(base) ((base) + (0x5C) + PAGE0_BASE)
#define SOC_SCHARGER_PD_DBG_RO_1_ADDR(base) ((base) + (0x5D) + PAGE0_BASE)
#define SOC_SCHARGER_PD_DBG_RO_2_ADDR(base) ((base) + (0x5E) + PAGE0_BASE)
#define SOC_SCHARGER_PD_DBG_RO_3_ADDR(base) ((base) + (0x5F) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FAKE_SEL_ADDR(base) ((base) + (0x60) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FAKE_ADDR(base) ((base) + (0x61) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FLAG_ADDR(base) ((base) + (0x62) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FLAG_0_ADDR(base) ((base) + (0x63) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FLAG_1_ADDR(base) ((base) + (0x64) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FLAG_2_ADDR(base) ((base) + (0x65) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FLAG_3_ADDR(base) ((base) + (0x66) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FLAG_4_ADDR(base) ((base) + (0x67) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FLAG_5_ADDR(base) ((base) + (0x68) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FLAG_6_ADDR(base) ((base) + (0x69) + PAGE0_BASE)
#define SOC_SCHARGER_IRQ_FLAG_7_ADDR(base) ((base) + (0x6A) + PAGE0_BASE)
#define SOC_SCHARGER_WDT_SOFT_RST_ADDR(base) ((base) + (0x6B) + PAGE0_BASE)
#define SOC_SCHARGER_WDT_CTRL_ADDR(base) ((base) + (0x6C) + PAGE0_BASE)
#define SOC_SCHARGER_DC_IBAT_REGULATOR_ADDR(base) ((base) + (0x6D) + PAGE0_BASE)
#define SOC_SCHARGER_DC_VBAT_REGULATOR_ADDR(base) ((base) + (0x6E) + PAGE0_BASE)
#define SOC_SCHARGER_DC_VOUT_REGULATOR_ADDR(base) ((base) + (0x6F) + PAGE0_BASE)
#define SOC_SCHARGER_OTG_CFG_ADDR(base) ((base) + (0x70) + PAGE0_BASE)
#define SOC_SCHARGER_PULSE_CHG_CFG0_ADDR(base) ((base) + (0x71) + PAGE0_BASE)
#define SOC_SCHARGER_PULSE_CHG_CFG1_ADDR(base) ((base) + (0x72) + PAGE0_BASE)
#define SOC_SCHARGER_DISCHG_TIME_ADDR(base) ((base) + (0x73) + PAGE0_BASE)
#define SOC_SCHARGER_DIG_STATUS0_ADDR(base) ((base) + (0x74) + PAGE0_BASE)
#define SOC_SCHARGER_SC_TESTBUS_CFG_ADDR(base) ((base) + (0x76) + PAGE0_BASE)
#define SOC_SCHARGER_SC_TESTBUS_RDATA_ADDR(base) ((base) + (0x77) + PAGE0_BASE)
#define SOC_SCHARGER_GLB_SOFT_RST_CTRL_ADDR(base) ((base) + (0x78) + PAGE0_BASE)
#define SOC_SCHARGER_EFUSE_SOFT_RST_CTRL_ADDR(base) ((base) + (0x79) + PAGE0_BASE)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_ADDR(base) ((base) + (0x7A) + PAGE0_BASE)
#define SOC_SCHARGER_BUCK_MODE_CFG_ADDR(base) ((base) + (0x7B) + PAGE0_BASE)
#define SOC_SCHARGER_SC_MODE_CFG_ADDR(base) ((base) + (0x7C) + PAGE0_BASE)
#define SOC_SCHARGER_LVC_MODE_CFG_ADDR(base) ((base) + (0x7D) + PAGE0_BASE)
#define SOC_SCHARGER_SC_BUCK_ENB_ADDR(base) ((base) + (0x7E) + PAGE0_BASE)
#define SOC_SCHARGER_SC_OVP_MOS_EN_ADDR(base) ((base) + (0x7F) + PAGE0_BASE)
 #define PAGE1_BASE (0x0180)
#define SOC_SCHARGER_FCP_IRQ3_ADDR(base) ((base) + (0x00) + PAGE1_BASE)
#define SOC_SCHARGER_FCP_IRQ4_ADDR(base) ((base) + (0x01) + PAGE1_BASE)
#define SOC_SCHARGER_FCP_IRQ3_MASK_ADDR(base) ((base) + (0x02) + PAGE1_BASE)
#define SOC_SCHARGER_FCP_IRQ4_MASK_ADDR(base) ((base) + (0x03) + PAGE1_BASE)
#define SOC_SCHARGER_MSTATE_ADDR(base) ((base) + (0x04) + PAGE1_BASE)
#define SOC_SCHARGER_CRC_ENABLE_ADDR(base) ((base) + (0x05) + PAGE1_BASE)
#define SOC_SCHARGER_CRC_START_VALUE_ADDR(base) ((base) + (0x06) + PAGE1_BASE)
#define SOC_SCHARGER_SAMPLE_CNT_ADJ_ADDR(base) ((base) + (0x07) + PAGE1_BASE)
#define SOC_SCHARGER_RX_PING_WIDTH_MIN_H_ADDR(base) ((base) + (0x08) + PAGE1_BASE)
#define SOC_SCHARGER_RX_PING_WIDTH_MIN_L_ADDR(base) ((base) + (0x09) + PAGE1_BASE)
#define SOC_SCHARGER_RX_PING_WIDTH_MAX_H_ADDR(base) ((base) + (0x0A) + PAGE1_BASE)
#define SOC_SCHARGER_RX_PING_WIDTH_MAX_L_ADDR(base) ((base) + (0x0B) + PAGE1_BASE)
#define SOC_SCHARGER_DATA_WAIT_TIME_ADDR(base) ((base) + (0x0C) + PAGE1_BASE)
#define SOC_SCHARGER_RETRY_CNT_ADDR(base) ((base) + (0x0D) + PAGE1_BASE)
#define SOC_SCHARGER_FCP_RO_RESERVE_ADDR(base) ((base) + (0x0E) + PAGE1_BASE)
#define SOC_SCHARGER_FCP_DEBUG_REG0_ADDR(base) ((base) + (0x0F) + PAGE1_BASE)
#define SOC_SCHARGER_FCP_DEBUG_REG1_ADDR(base) ((base) + (0x10) + PAGE1_BASE)
#define SOC_SCHARGER_FCP_DEBUG_REG2_ADDR(base) ((base) + (0x11) + PAGE1_BASE)
#define SOC_SCHARGER_FCP_DEBUG_REG3_ADDR(base) ((base) + (0x12) + PAGE1_BASE)
#define SOC_SCHARGER_FCP_DEBUG_REG4_ADDR(base) ((base) + (0x13) + PAGE1_BASE)
#define SOC_SCHARGER_RX_PACKET_WAIT_ADJUST_ADDR(base) ((base) + (0x14) + PAGE1_BASE)
#define SOC_SCHARGER_SAMPLE_CNT_TINYJUST_ADDR(base) ((base) + (0x15) + PAGE1_BASE)
#define SOC_SCHARGER_RX_PING_CNT_TINYJUST_ADDR(base) ((base) + (0x16) + PAGE1_BASE)
#define SOC_SCHARGER_SHIFT_CNT_CFG_MAX_ADDR(base) ((base) + (0x17) + PAGE1_BASE)
#define SOC_SCHARGER_HKADC_CFG_REG_0_ADDR(base) ((base) + (0x18) + PAGE1_BASE)
#define SOC_SCHARGER_HKADC_CFG_REG_1_ADDR(base) ((base) + (0x19) + PAGE1_BASE)
#define SOC_SCHARGER_HKADC_CFG_REG_2_ADDR(base) ((base) + (0x1A) + PAGE1_BASE)
#define SOC_SCHARGER_HKADC_OFFSET_0P1_ADDR(base) ((base) + (0x1B) + PAGE1_BASE)
#define SOC_SCHARGER_HKADC_OFFSET_2P45_ADDR(base) ((base) + (0x1C) + PAGE1_BASE)
#define SOC_SCHARGER_SOH_OVH_TH0_L_ADDR(base) ((base) + (0x1D) + PAGE1_BASE)
#define SOC_SCHARGER_SOH_OVH_TH0_H_ADDR(base) ((base) + (0x1E) + PAGE1_BASE)
#define SOC_SCHARGER_TSBAT_OVH_TH0_L_ADDR(base) ((base) + (0x1F) + PAGE1_BASE)
#define SOC_SCHARGER_TSBAT_OVH_TH0_H_ADDR(base) ((base) + (0x20) + PAGE1_BASE)
#define SOC_SCHARGER_SOH_OVH_TH1_L_ADDR(base) ((base) + (0x21) + PAGE1_BASE)
#define SOC_SCHARGER_SOH_OVH_TH1_H_ADDR(base) ((base) + (0x22) + PAGE1_BASE)
#define SOC_SCHARGER_TSBAT_OVH_TH1_L_ADDR(base) ((base) + (0x23) + PAGE1_BASE)
#define SOC_SCHARGER_TSBAT_OVH_TH1_H_ADDR(base) ((base) + (0x24) + PAGE1_BASE)
#define SOC_SCHARGER_SOH_OVH_TH2_L_ADDR(base) ((base) + (0x25) + PAGE1_BASE)
#define SOC_SCHARGER_SOH_OVH_TH2_H_ADDR(base) ((base) + (0x26) + PAGE1_BASE)
#define SOC_SCHARGER_TSBAT_OVH_TH2_L_ADDR(base) ((base) + (0x27) + PAGE1_BASE)
#define SOC_SCHARGER_TSBAT_OVH_TH2_H_ADDR(base) ((base) + (0x28) + PAGE1_BASE)
#define SOC_SCHARGER_SOH_OVL_TH_L_ADDR(base) ((base) + (0x29) + PAGE1_BASE)
#define SOC_SCHARGER_SOH_OVL_TH_H_ADDR(base) ((base) + (0x2A) + PAGE1_BASE)
#define SOC_SCHARGER_SOH_OVP_REAL_ADDR(base) ((base) + (0x2B) + PAGE1_BASE)
#define SOC_SCHARGER_HKADC_TB_EN_SEL_ADDR(base) ((base) + (0x2C) + PAGE1_BASE)
#define SOC_SCHARGER_HKADC_TB_DATA0_ADDR(base) ((base) + (0x2D) + PAGE1_BASE)
#define SOC_SCHARGER_HKADC_TB_DATA1_ADDR(base) ((base) + (0x2E) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_ADDR(base) ((base) + (0x30) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_1_ADDR(base) ((base) + (0x31) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_2_ADDR(base) ((base) + (0x32) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_SAMPLE_TIME_H_ADDR(base) ((base) + (0x33) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_SAMPLE_TIME_L_ADDR(base) ((base) + (0x34) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA0_L_ADDR(base) ((base) + (0x35) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA0_H_ADDR(base) ((base) + (0x36) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA1_L_ADDR(base) ((base) + (0x37) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA1_H_ADDR(base) ((base) + (0x38) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA2_L_ADDR(base) ((base) + (0x39) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA2_H_ADDR(base) ((base) + (0x3A) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA3_L_ADDR(base) ((base) + (0x3B) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA3_H_ADDR(base) ((base) + (0x3C) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA4_L_ADDR(base) ((base) + (0x3D) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA4_H_ADDR(base) ((base) + (0x3E) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA5_L_ADDR(base) ((base) + (0x3F) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA5_H_ADDR(base) ((base) + (0x40) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA6_L_ADDR(base) ((base) + (0x41) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA6_H_ADDR(base) ((base) + (0x42) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA7_L_ADDR(base) ((base) + (0x43) + PAGE1_BASE)
#define SOC_SCHARGER_ACR_DATA7_H_ADDR(base) ((base) + (0x44) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_MASK_ADDR(base) ((base) + (0x48) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_MASK_0_ADDR(base) ((base) + (0x49) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_MASK_1_ADDR(base) ((base) + (0x4A) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_MASK_2_ADDR(base) ((base) + (0x4B) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_MASK_3_ADDR(base) ((base) + (0x4C) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_MASK_4_ADDR(base) ((base) + (0x4D) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_MASK_5_ADDR(base) ((base) + (0x4E) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_MASK_6_ADDR(base) ((base) + (0x4F) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_MASK_7_ADDR(base) ((base) + (0x50) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_STATUS_0_ADDR(base) ((base) + (0x51) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_STATUS_1_ADDR(base) ((base) + (0x52) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_STATUS_2_ADDR(base) ((base) + (0x53) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_STATUS_3_ADDR(base) ((base) + (0x54) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_STATUS_4_ADDR(base) ((base) + (0x55) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_STATUS_5_ADDR(base) ((base) + (0x56) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_STATUS_6_ADDR(base) ((base) + (0x57) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_STATUS_7_ADDR(base) ((base) + (0x58) + PAGE1_BASE)
#define SOC_SCHARGER_IRQ_STATUS_8_ADDR(base) ((base) + (0x59) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_SEL_ADDR(base) ((base) + (0x60) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_CFG_0_ADDR(base) ((base) + (0x61) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_WE_0_ADDR(base) ((base) + (0x62) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_WE_1_ADDR(base) ((base) + (0x63) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_WE_2_ADDR(base) ((base) + (0x64) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_WE_3_ADDR(base) ((base) + (0x65) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_WE_4_ADDR(base) ((base) + (0x66) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_WE_5_ADDR(base) ((base) + (0x67) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_WE_6_ADDR(base) ((base) + (0x68) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_WE_7_ADDR(base) ((base) + (0x69) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_PDOB_PRE_ADDR_WE_ADDR(base) ((base) + (0x6A) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_PDOB_PRE_WDATA_ADDR(base) ((base) + (0x6B) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE1_TESTBUS_0_ADDR(base) ((base) + (0x6C) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE2_TESTBUS_0_ADDR(base) ((base) + (0x6D) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE3_TESTBUS_0_ADDR(base) ((base) + (0x6E) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_TESTBUS_SEL_ADDR(base) ((base) + (0x6F) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_ADDR(base) ((base) + (0x70) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE1_TESTBUS_RDATA_ADDR(base) ((base) + (0x71) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE2_TESTBUS_RDATA_ADDR(base) ((base) + (0x72) + PAGE1_BASE)
#define SOC_SCHARGER_EFUSE3_TESTBUS_RDATA_ADDR(base) ((base) + (0x73) + PAGE1_BASE)
#define SOC_SCHARGER_GLB_TESTBUS_CFG_ADDR(base) ((base) + (0x74) + PAGE1_BASE)
#define SOC_SCHARGER_GLB_TEST_DATA_ADDR(base) ((base) + (0x75) + PAGE1_BASE)
 #define PAGE2_BASE (0x0280)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_ADDR(base) ((base) + (0x00) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_ADDR(base) ((base) + (0x01) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_CFG_REG_2_ADDR(base) ((base) + (0x02) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_CFG_REG_3_ADDR(base) ((base) + (0x03) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_CFG_REG_4_ADDR(base) ((base) + (0x04) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_ADDR(base) ((base) + (0x05) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_CFG_REG_6_ADDR(base) ((base) + (0x06) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_CFG_REG_7_ADDR(base) ((base) + (0x07) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_ADDR(base) ((base) + (0x08) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_CFG_REG_9_ADDR(base) ((base) + (0x09) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ADDR(base) ((base) + (0x0A) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_RO_REG_11_ADDR(base) ((base) + (0x0B) + PAGE2_BASE)
#define SOC_SCHARGER_USB_OVP_CFG_REG_0_ADDR(base) ((base) + (0x0C) + PAGE2_BASE)
#define SOC_SCHARGER_USB_OVP_CFG_REG_1_ADDR(base) ((base) + (0x0D) + PAGE2_BASE)
#define SOC_SCHARGER_TCPC_CFG_REG_1_ADDR(base) ((base) + (0x0E) + PAGE2_BASE)
#define SOC_SCHARGER_TCPC_CFG_REG_2_ADDR(base) ((base) + (0x0F) + PAGE2_BASE)
#define SOC_SCHARGER_TCPC_CFG_REG_3_ADDR(base) ((base) + (0x10) + PAGE2_BASE)
#define SOC_SCHARGER_TCPC_RO_REG_5_ADDR(base) ((base) + (0x11) + PAGE2_BASE)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_ADDR(base) ((base) + (0x12) + PAGE2_BASE)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_1_ADDR(base) ((base) + (0x13) + PAGE2_BASE)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_2_ADDR(base) ((base) + (0x14) + PAGE2_BASE)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_3_ADDR(base) ((base) + (0x15) + PAGE2_BASE)
#define SOC_SCHARGER_SCHG_LOGIC_RO_REG_4_ADDR(base) ((base) + (0x16) + PAGE2_BASE)
#define SOC_SCHARGER_CHARGER_BATFET_CTRL_ADDR(base) ((base) + (0x17) + PAGE2_BASE)
#define SOC_SCHARGER_VBAT_LV_REG_ADDR(base) ((base) + (0x18) + PAGE2_BASE)
#define SOC_SCHARGER_VDM_CFG_REG_0_ADDR(base) ((base) + (0x1A) + PAGE2_BASE)
#define SOC_SCHARGER_VDM_CFG_REG_1_ADDR(base) ((base) + (0x1B) + PAGE2_BASE)
#define SOC_SCHARGER_VDM_CFG_REG_2_ADDR(base) ((base) + (0x1C) + PAGE2_BASE)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_ADDR(base) ((base) + (0x20) + PAGE2_BASE)
#define SOC_SCHARGER_DET_TOP_CFG_REG_1_ADDR(base) ((base) + (0x21) + PAGE2_BASE)
#define SOC_SCHARGER_DET_TOP_CFG_REG_2_ADDR(base) ((base) + (0x22) + PAGE2_BASE)
#define SOC_SCHARGER_DET_TOP_CFG_REG_3_ADDR(base) ((base) + (0x23) + PAGE2_BASE)
#define SOC_SCHARGER_DET_TOP_CFG_REG_4_ADDR(base) ((base) + (0x24) + PAGE2_BASE)
#define SOC_SCHARGER_DET_TOP_CFG_REG_5_ADDR(base) ((base) + (0x25) + PAGE2_BASE)
#define SOC_SCHARGER_DET_TOP_CFG_REG_6_ADDR(base) ((base) + (0x26) + PAGE2_BASE)
#define SOC_SCHARGER_PSEL_CFG_REG_0_ADDR(base) ((base) + (0x27) + PAGE2_BASE)
#define SOC_SCHARGER_PSEL_RO_REG_1_ADDR(base) ((base) + (0x28) + PAGE2_BASE)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_ADDR(base) ((base) + (0x29) + PAGE2_BASE)
#define SOC_SCHARGER_REF_TOP_CFG_REG_1_ADDR(base) ((base) + (0x2A) + PAGE2_BASE)
#define SOC_SCHARGER_REF_TOP_CFG_REG_2_ADDR(base) ((base) + (0x2B) + PAGE2_BASE)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_ADDR(base) ((base) + (0x2C) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_ADDR(base) ((base) + (0x30) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_1_ADDR(base) ((base) + (0x31) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_2_ADDR(base) ((base) + (0x32) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_3_ADDR(base) ((base) + (0x33) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_4_ADDR(base) ((base) + (0x34) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_5_ADDR(base) ((base) + (0x35) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_6_ADDR(base) ((base) + (0x36) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_ADDR(base) ((base) + (0x37) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_ADDR(base) ((base) + (0x38) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_9_ADDR(base) ((base) + (0x39) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_10_ADDR(base) ((base) + (0x3A) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_ADDR(base) ((base) + (0x3B) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_12_ADDR(base) ((base) + (0x3C) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_13_ADDR(base) ((base) + (0x3D) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_14_ADDR(base) ((base) + (0x3E) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_15_ADDR(base) ((base) + (0x3F) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_16_ADDR(base) ((base) + (0x40) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_17_ADDR(base) ((base) + (0x41) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_ADDR(base) ((base) + (0x42) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_19_ADDR(base) ((base) + (0x43) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_ADDR(base) ((base) + (0x44) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_21_ADDR(base) ((base) + (0x45) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_CFG_REG_22_ADDR(base) ((base) + (0x46) + PAGE2_BASE)
#define SOC_SCHARGER_DC_TOP_RO_REG_23_ADDR(base) ((base) + (0x47) + PAGE2_BASE)
#define SOC_SCHARGER_VERSION0_RO_REG_0_ADDR(base) ((base) + (0x49) + PAGE2_BASE)
#define SOC_SCHARGER_VERSION1_RO_REG_0_ADDR(base) ((base) + (0x4A) + PAGE2_BASE)
#define SOC_SCHARGER_VERSION2_RO_REG_0_ADDR(base) ((base) + (0x4B) + PAGE2_BASE)
#define SOC_SCHARGER_VERSION3_RO_REG_0_ADDR(base) ((base) + (0x4C) + PAGE2_BASE)
#define SOC_SCHARGER_VERSION4_RO_REG_0_ADDR(base) ((base) + (0x4D) + PAGE2_BASE)
#define SOC_SCHARGER_VERSION5_RO_REG_0_ADDR(base) ((base) + (0x4E) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_0_ADDR(base) ((base) + (0x50) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_1_ADDR(base) ((base) + (0x51) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_2_ADDR(base) ((base) + (0x52) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_3_ADDR(base) ((base) + (0x53) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_4_ADDR(base) ((base) + (0x54) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_5_ADDR(base) ((base) + (0x55) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_6_ADDR(base) ((base) + (0x56) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_7_ADDR(base) ((base) + (0x57) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_8_ADDR(base) ((base) + (0x58) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_9_ADDR(base) ((base) + (0x59) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_10_ADDR(base) ((base) + (0x5A) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_11_ADDR(base) ((base) + (0x5B) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_12_ADDR(base) ((base) + (0x5C) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_13_ADDR(base) ((base) + (0x5D) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_14_ADDR(base) ((base) + (0x5E) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_15_ADDR(base) ((base) + (0x5F) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_16_ADDR(base) ((base) + (0x60) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_17_ADDR(base) ((base) + (0x61) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_18_ADDR(base) ((base) + (0x62) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_19_ADDR(base) ((base) + (0x63) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_20_ADDR(base) ((base) + (0x64) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_21_ADDR(base) ((base) + (0x65) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_22_ADDR(base) ((base) + (0x66) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_23_ADDR(base) ((base) + (0x67) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_24_ADDR(base) ((base) + (0x68) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_CFG_REG_25_ADDR(base) ((base) + (0x69) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_RO_REG_26_ADDR(base) ((base) + (0x6A) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_RO_REG_27_ADDR(base) ((base) + (0x6B) + PAGE2_BASE)
#define SOC_SCHARGER_BUCK_RO_REG_28_ADDR(base) ((base) + (0x6C) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_CFG_REG_0_ADDR(base) ((base) + (0x6D) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_CFG_REG_1_ADDR(base) ((base) + (0x6E) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_CFG_REG_2_ADDR(base) ((base) + (0x6F) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_CFG_REG_3_ADDR(base) ((base) + (0x70) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_CFG_REG_4_ADDR(base) ((base) + (0x71) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_CFG_REG_5_ADDR(base) ((base) + (0x72) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_CFG_REG_6_ADDR(base) ((base) + (0x73) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_RO_REG_7_ADDR(base) ((base) + (0x74) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_RO_REG_8_ADDR(base) ((base) + (0x75) + PAGE2_BASE)
#define SOC_SCHARGER_OTG_RO_REG_9_ADDR(base) ((base) + (0x76) + PAGE2_BASE)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vend_id_l : 8;
    } reg;
} SOC_SCHARGER_VENDIDL_UNION;
#endif
#define SOC_SCHARGER_VENDIDL_pd_vend_id_l_START (0)
#define SOC_SCHARGER_VENDIDL_pd_vend_id_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vend_id_h : 8;
    } reg;
} SOC_SCHARGER_VENDIDH_UNION;
#endif
#define SOC_SCHARGER_VENDIDH_pd_vend_id_h_START (0)
#define SOC_SCHARGER_VENDIDH_pd_vend_id_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_prod_id_l : 8;
    } reg;
} SOC_SCHARGER_PRODIDL_UNION;
#endif
#define SOC_SCHARGER_PRODIDL_pd_prod_id_l_START (0)
#define SOC_SCHARGER_PRODIDL_pd_prod_id_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_prod_id_h : 8;
    } reg;
} SOC_SCHARGER_PRODIDH_UNION;
#endif
#define SOC_SCHARGER_PRODIDH_pd_prod_id_h_START (0)
#define SOC_SCHARGER_PRODIDH_pd_prod_id_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_id_l : 8;
    } reg;
} SOC_SCHARGER_DEVIDL_UNION;
#endif
#define SOC_SCHARGER_DEVIDL_pd_dev_id_l_START (0)
#define SOC_SCHARGER_DEVIDL_pd_dev_id_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_id_h : 8;
    } reg;
} SOC_SCHARGER_DEVIDH_UNION;
#endif
#define SOC_SCHARGER_DEVIDH_pd_dev_id_h_START (0)
#define SOC_SCHARGER_DEVIDH_pd_dev_id_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_typc_revision_l : 8;
    } reg;
} SOC_SCHARGER_TYPECREVL_UNION;
#endif
#define SOC_SCHARGER_TYPECREVL_pd_typc_revision_l_START (0)
#define SOC_SCHARGER_TYPECREVL_pd_typc_revision_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_typc_revision_h : 8;
    } reg;
} SOC_SCHARGER_TYPECREVH_UNION;
#endif
#define SOC_SCHARGER_TYPECREVH_pd_typc_revision_h_START (0)
#define SOC_SCHARGER_TYPECREVH_pd_typc_revision_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_version : 8;
    } reg;
} SOC_SCHARGER_USBPDVER_UNION;
#endif
#define SOC_SCHARGER_USBPDVER_pd_version_START (0)
#define SOC_SCHARGER_USBPDVER_pd_version_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_revision : 8;
    } reg;
} SOC_SCHARGER_USBPDREV_UNION;
#endif
#define SOC_SCHARGER_USBPDREV_pd_revision_START (0)
#define SOC_SCHARGER_USBPDREV_pd_revision_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_itf_revision_l : 8;
    } reg;
} SOC_SCHARGER_PDIFREVL_UNION;
#endif
#define SOC_SCHARGER_PDIFREVL_pd_itf_revision_l_START (0)
#define SOC_SCHARGER_PDIFREVL_pd_itf_revision_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_itf_revision_h : 8;
    } reg;
} SOC_SCHARGER_PDIFREVH_UNION;
#endif
#define SOC_SCHARGER_PDIFREVH_pd_itf_revision_h_START (0)
#define SOC_SCHARGER_PDIFREVH_pd_itf_revision_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_int_ccstatus : 1;
        unsigned char pd_int_port_pwr : 1;
        unsigned char pd_int_rxstat : 1;
        unsigned char pd_int_rxhardrst : 1;
        unsigned char pd_int_txfail : 1;
        unsigned char pd_int_txdisc : 1;
        unsigned char pd_int_txsucc : 1;
        unsigned char pd_int_vbus_alrm_h : 1;
    } reg;
} SOC_SCHARGER_PD_ALERT_L_UNION;
#endif
#define SOC_SCHARGER_PD_ALERT_L_pd_int_ccstatus_START (0)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_ccstatus_END (0)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_port_pwr_START (1)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_port_pwr_END (1)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_rxstat_START (2)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_rxstat_END (2)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_rxhardrst_START (3)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_rxhardrst_END (3)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_txfail_START (4)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_txfail_END (4)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_txdisc_START (5)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_txdisc_END (5)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_txsucc_START (6)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_txsucc_END (6)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_vbus_alrm_h_START (7)
#define SOC_SCHARGER_PD_ALERT_L_pd_int_vbus_alrm_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_int_vbus_alrm_l : 1;
        unsigned char pd_int_fault : 1;
        unsigned char pd_int_rx_full : 1;
        unsigned char pd_int_vbus_snk_disc : 1;
        unsigned char reserved : 3;
        unsigned char pd_int_fr_swap : 1;
    } reg;
} SOC_SCHARGER_PD_ALERT_H_UNION;
#endif
#define SOC_SCHARGER_PD_ALERT_H_pd_int_vbus_alrm_l_START (0)
#define SOC_SCHARGER_PD_ALERT_H_pd_int_vbus_alrm_l_END (0)
#define SOC_SCHARGER_PD_ALERT_H_pd_int_fault_START (1)
#define SOC_SCHARGER_PD_ALERT_H_pd_int_fault_END (1)
#define SOC_SCHARGER_PD_ALERT_H_pd_int_rx_full_START (2)
#define SOC_SCHARGER_PD_ALERT_H_pd_int_rx_full_END (2)
#define SOC_SCHARGER_PD_ALERT_H_pd_int_vbus_snk_disc_START (3)
#define SOC_SCHARGER_PD_ALERT_H_pd_int_vbus_snk_disc_END (3)
#define SOC_SCHARGER_PD_ALERT_H_pd_int_fr_swap_START (7)
#define SOC_SCHARGER_PD_ALERT_H_pd_int_fr_swap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_ccstatus : 1;
        unsigned char pd_msk_port_pwr : 1;
        unsigned char pd_msk_rxstat : 1;
        unsigned char pd_msk_rxhardrst : 1;
        unsigned char pd_msk_txfail : 1;
        unsigned char pd_msk_txdisc : 1;
        unsigned char pd_msk_txsucc : 1;
        unsigned char pd_msk_vbus_alrm_h : 1;
    } reg;
} SOC_SCHARGER_PD_ALERT_MSK_L_UNION;
#endif
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_ccstatus_START (0)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_ccstatus_END (0)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_port_pwr_START (1)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_port_pwr_END (1)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_rxstat_START (2)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_rxstat_END (2)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_rxhardrst_START (3)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_rxhardrst_END (3)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_txfail_START (4)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_txfail_END (4)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_txdisc_START (5)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_txdisc_END (5)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_txsucc_START (6)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_txsucc_END (6)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_vbus_alrm_h_START (7)
#define SOC_SCHARGER_PD_ALERT_MSK_L_pd_msk_vbus_alrm_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_vbus_alrm_l : 1;
        unsigned char pd_msk_fault : 1;
        unsigned char pd_msk_rx_full : 1;
        unsigned char pd_msk_vbus_snk_disc : 1;
        unsigned char reserved : 3;
        unsigned char pd_msk_fr_swap : 1;
    } reg;
} SOC_SCHARGER_PD_ALERT_MSK_H_UNION;
#endif
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_vbus_alrm_l_START (0)
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_vbus_alrm_l_END (0)
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_fault_START (1)
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_fault_END (1)
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_rx_full_START (2)
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_rx_full_END (2)
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_vbus_snk_disc_START (3)
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_vbus_snk_disc_END (3)
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_fr_swap_START (7)
#define SOC_SCHARGER_PD_ALERT_MSK_H_pd_msk_fr_swap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_snkvbus : 1;
        unsigned char pd_msk_vconn_vld : 1;
        unsigned char pd_msk_vbus_vld : 1;
        unsigned char pd_msk_vbus_vld_en : 1;
        unsigned char pd_msk_src_vbus : 1;
        unsigned char reserved : 1;
        unsigned char pd_msk_init_stat : 1;
        unsigned char pd_msk_debug_acc : 1;
    } reg;
} SOC_SCHARGER_PD_PWRSTAT_MSK_UNION;
#endif
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_snkvbus_START (0)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_snkvbus_END (0)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_vconn_vld_START (1)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_vconn_vld_END (1)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_vbus_vld_START (2)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_vbus_vld_END (2)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_vbus_vld_en_START (3)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_vbus_vld_en_END (3)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_src_vbus_START (4)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_src_vbus_END (4)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_init_stat_START (6)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_init_stat_END (6)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_debug_acc_START (7)
#define SOC_SCHARGER_PD_PWRSTAT_MSK_pd_msk_debug_acc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msk_i2c_err : 1;
        unsigned char pd_msk_vconn_ocp : 1;
        unsigned char pd_msk_vbus_ovp : 1;
        unsigned char reserved_0 : 1;
        unsigned char pd_msk_force_disch_fail : 1;
        unsigned char pd_msk_auto_disch_fail : 1;
        unsigned char reserved_1 : 2;
    } reg;
} SOC_SCHARGER_PD_FAULTSTAT_MSK_UNION;
#endif
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_i2c_err_START (0)
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_i2c_err_END (0)
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_vconn_ocp_START (1)
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_vconn_ocp_END (1)
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_vbus_ovp_START (2)
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_vbus_ovp_END (2)
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_force_disch_fail_START (4)
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_force_disch_fail_END (4)
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_auto_disch_fail_START (5)
#define SOC_SCHARGER_PD_FAULTSTAT_MSK_pd_msk_auto_disch_fail_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_orient : 1;
        unsigned char pd_bist_mode : 1;
        unsigned char reserved_0 : 2;
        unsigned char pd_debug_acc_ctrl : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_PD_TCPC_CTRL_UNION;
#endif
#define SOC_SCHARGER_PD_TCPC_CTRL_pd_orient_START (0)
#define SOC_SCHARGER_PD_TCPC_CTRL_pd_orient_END (0)
#define SOC_SCHARGER_PD_TCPC_CTRL_pd_bist_mode_START (1)
#define SOC_SCHARGER_PD_TCPC_CTRL_pd_bist_mode_END (1)
#define SOC_SCHARGER_PD_TCPC_CTRL_pd_debug_acc_ctrl_START (4)
#define SOC_SCHARGER_PD_TCPC_CTRL_pd_debug_acc_ctrl_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_cc1_cfg : 2;
        unsigned char pd_cc2_cfg : 2;
        unsigned char pd_rp_cfg : 2;
        unsigned char pd_drp : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_PD_ROLE_CTRL_UNION;
#endif
#define SOC_SCHARGER_PD_ROLE_CTRL_pd_cc1_cfg_START (0)
#define SOC_SCHARGER_PD_ROLE_CTRL_pd_cc1_cfg_END (1)
#define SOC_SCHARGER_PD_ROLE_CTRL_pd_cc2_cfg_START (2)
#define SOC_SCHARGER_PD_ROLE_CTRL_pd_cc2_cfg_END (3)
#define SOC_SCHARGER_PD_ROLE_CTRL_pd_rp_cfg_START (4)
#define SOC_SCHARGER_PD_ROLE_CTRL_pd_rp_cfg_END (5)
#define SOC_SCHARGER_PD_ROLE_CTRL_pd_drp_START (6)
#define SOC_SCHARGER_PD_ROLE_CTRL_pd_drp_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vconn_ocp_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char pd_disch_timer_dis : 1;
        unsigned char reserved_1 : 4;
    } reg;
} SOC_SCHARGER_PD_FAULT_CTRL_UNION;
#endif
#define SOC_SCHARGER_PD_FAULT_CTRL_pd_vconn_ocp_en_START (0)
#define SOC_SCHARGER_PD_FAULT_CTRL_pd_vconn_ocp_en_END (0)
#define SOC_SCHARGER_PD_FAULT_CTRL_pd_disch_timer_dis_START (3)
#define SOC_SCHARGER_PD_FAULT_CTRL_pd_disch_timer_dis_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vconn_en : 1;
        unsigned char reserved_0 : 1;
        unsigned char pd_force_disch_en : 1;
        unsigned char pd_bleed_disch_en : 1;
        unsigned char pd_auto_disch : 1;
        unsigned char pd_valarm_dis : 1;
        unsigned char pd_vbus_mon_dis : 1;
        unsigned char reserved_1 : 1;
    } reg;
} SOC_SCHARGER_PD_PWR_CTRL_UNION;
#endif
#define SOC_SCHARGER_PD_PWR_CTRL_pd_vconn_en_START (0)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_vconn_en_END (0)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_force_disch_en_START (2)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_force_disch_en_END (2)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_bleed_disch_en_START (3)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_bleed_disch_en_END (3)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_auto_disch_START (4)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_auto_disch_END (4)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_valarm_dis_START (5)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_valarm_dis_END (5)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_vbus_mon_dis_START (6)
#define SOC_SCHARGER_PD_PWR_CTRL_pd_vbus_mon_dis_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_cc1_stat : 2;
        unsigned char pd_cc2_stat : 2;
        unsigned char pd_con_result : 1;
        unsigned char pd_look4con : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_PD_CC_STATUS_UNION;
#endif
#define SOC_SCHARGER_PD_CC_STATUS_pd_cc1_stat_START (0)
#define SOC_SCHARGER_PD_CC_STATUS_pd_cc1_stat_END (1)
#define SOC_SCHARGER_PD_CC_STATUS_pd_cc2_stat_START (2)
#define SOC_SCHARGER_PD_CC_STATUS_pd_cc2_stat_END (3)
#define SOC_SCHARGER_PD_CC_STATUS_pd_con_result_START (4)
#define SOC_SCHARGER_PD_CC_STATUS_pd_con_result_END (4)
#define SOC_SCHARGER_PD_CC_STATUS_pd_look4con_START (5)
#define SOC_SCHARGER_PD_CC_STATUS_pd_look4con_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_sinking_vbus : 1;
        unsigned char pd_vconn_present : 1;
        unsigned char pd_vbus_present : 1;
        unsigned char pd_vbus_pres_detect_en : 1;
        unsigned char pd_source_vbus : 1;
        unsigned char reserved : 1;
        unsigned char pd_tcpc_init_stat : 1;
        unsigned char pd_debug_acc_connect : 1;
    } reg;
} SOC_SCHARGER_PD_PWR_STATUS_UNION;
#endif
#define SOC_SCHARGER_PD_PWR_STATUS_pd_sinking_vbus_START (0)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_sinking_vbus_END (0)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_vconn_present_START (1)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_vconn_present_END (1)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_vbus_present_START (2)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_vbus_present_END (2)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_vbus_pres_detect_en_START (3)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_vbus_pres_detect_en_END (3)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_source_vbus_START (4)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_source_vbus_END (4)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_tcpc_init_stat_START (6)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_tcpc_init_stat_END (6)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_debug_acc_connect_START (7)
#define SOC_SCHARGER_PD_PWR_STATUS_pd_debug_acc_connect_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_i2c_err : 1;
        unsigned char pd_vconn_ocp : 1;
        unsigned char pd_vusb_ovp : 1;
        unsigned char reserved_0 : 1;
        unsigned char pd_force_disch_fail : 1;
        unsigned char pd_auto_disch_fail : 1;
        unsigned char reserved_1 : 1;
        unsigned char pd_reg_reset_default : 1;
    } reg;
} SOC_SCHARGER_PD_FAULT_STATUS_UNION;
#endif
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_i2c_err_START (0)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_i2c_err_END (0)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_vconn_ocp_START (1)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_vconn_ocp_END (1)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_vusb_ovp_START (2)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_vusb_ovp_END (2)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_force_disch_fail_START (4)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_force_disch_fail_END (4)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_auto_disch_fail_START (5)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_auto_disch_fail_END (5)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_reg_reset_default_START (7)
#define SOC_SCHARGER_PD_FAULT_STATUS_pd_reg_reset_default_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_command : 8;
    } reg;
} SOC_SCHARGER_PD_COMMAND_UNION;
#endif
#define SOC_SCHARGER_PD_COMMAND_pd_command_START (0)
#define SOC_SCHARGER_PD_COMMAND_pd_command_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_cap1_l : 8;
    } reg;
} SOC_SCHARGER_PD_DEVCAP1L_UNION;
#endif
#define SOC_SCHARGER_PD_DEVCAP1L_pd_dev_cap1_l_START (0)
#define SOC_SCHARGER_PD_DEVCAP1L_pd_dev_cap1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_cap1_h : 8;
    } reg;
} SOC_SCHARGER_PD_DEVCAP1H_UNION;
#endif
#define SOC_SCHARGER_PD_DEVCAP1H_pd_dev_cap1_h_START (0)
#define SOC_SCHARGER_PD_DEVCAP1H_pd_dev_cap1_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_cap2_l : 8;
    } reg;
} SOC_SCHARGER_PD_DEVCAP2L_UNION;
#endif
#define SOC_SCHARGER_PD_DEVCAP2L_pd_dev_cap2_l_START (0)
#define SOC_SCHARGER_PD_DEVCAP2L_pd_dev_cap2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dev_cap2_h : 8;
    } reg;
} SOC_SCHARGER_PD_DEVCAP2H_UNION;
#endif
#define SOC_SCHARGER_PD_DEVCAP2H_pd_dev_cap2_h_START (0)
#define SOC_SCHARGER_PD_DEVCAP2H_pd_dev_cap2_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_stdin_cap : 8;
    } reg;
} SOC_SCHARGER_PD_STDIN_CAP_UNION;
#endif
#define SOC_SCHARGER_PD_STDIN_CAP_pd_stdin_cap_START (0)
#define SOC_SCHARGER_PD_STDIN_CAP_pd_stdin_cap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_stdout_cap : 8;
    } reg;
} SOC_SCHARGER_PD_STDOUT_CAP_UNION;
#endif
#define SOC_SCHARGER_PD_STDOUT_CAP_pd_stdout_cap_START (0)
#define SOC_SCHARGER_PD_STDOUT_CAP_pd_stdout_cap_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_msg_header : 8;
    } reg;
} SOC_SCHARGER_PD_MSG_HEADR_UNION;
#endif
#define SOC_SCHARGER_PD_MSG_HEADR_pd_msg_header_START (0)
#define SOC_SCHARGER_PD_MSG_HEADR_pd_msg_header_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_en_sop : 1;
        unsigned char pd_en_sop1 : 1;
        unsigned char pd_en_sop2 : 1;
        unsigned char pd_en_sop1_debug : 1;
        unsigned char pd_en_sop2_debug : 1;
        unsigned char pd_hard_rst : 1;
        unsigned char pd_cable_rst : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_PD_RXDETECT_UNION;
#endif
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop_START (0)
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop_END (0)
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop1_START (1)
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop1_END (1)
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop2_START (2)
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop2_END (2)
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop1_debug_START (3)
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop1_debug_END (3)
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop2_debug_START (4)
#define SOC_SCHARGER_PD_RXDETECT_pd_en_sop2_debug_END (4)
#define SOC_SCHARGER_PD_RXDETECT_pd_hard_rst_START (5)
#define SOC_SCHARGER_PD_RXDETECT_pd_hard_rst_END (5)
#define SOC_SCHARGER_PD_RXDETECT_pd_cable_rst_START (6)
#define SOC_SCHARGER_PD_RXDETECT_pd_cable_rst_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_bytecnt : 8;
    } reg;
} SOC_SCHARGER_PD_RXBYTECNT_UNION;
#endif
#define SOC_SCHARGER_PD_RXBYTECNT_pd_rx_bytecnt_START (0)
#define SOC_SCHARGER_PD_RXBYTECNT_pd_rx_bytecnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_type : 3;
        unsigned char reserved : 5;
    } reg;
} SOC_SCHARGER_PD_RXTYPE_UNION;
#endif
#define SOC_SCHARGER_PD_RXTYPE_pd_rx_type_START (0)
#define SOC_SCHARGER_PD_RXTYPE_pd_rx_type_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_head_l : 8;
    } reg;
} SOC_SCHARGER_PD_RXHEADL_UNION;
#endif
#define SOC_SCHARGER_PD_RXHEADL_pd_rx_head_l_START (0)
#define SOC_SCHARGER_PD_RXHEADL_pd_rx_head_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_head_h : 8;
    } reg;
} SOC_SCHARGER_PD_RXHEADH_UNION;
#endif
#define SOC_SCHARGER_PD_RXHEADH_pd_rx_head_h_START (0)
#define SOC_SCHARGER_PD_RXHEADH_pd_rx_head_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_rx_data : 8;
    } reg;
} SOC_SCHARGER_PD_RXDATA_0_UNION;
#endif
#define SOC_SCHARGER_PD_RXDATA_0_pd_rx_data_START (0)
#define SOC_SCHARGER_PD_RXDATA_0_pd_rx_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_transmit : 3;
        unsigned char reserved_0 : 1;
        unsigned char pd_retry_cnt : 2;
        unsigned char reserved_1 : 2;
    } reg;
} SOC_SCHARGER_PD_TRANSMIT_UNION;
#endif
#define SOC_SCHARGER_PD_TRANSMIT_pd_transmit_START (0)
#define SOC_SCHARGER_PD_TRANSMIT_pd_transmit_END (2)
#define SOC_SCHARGER_PD_TRANSMIT_pd_retry_cnt_START (4)
#define SOC_SCHARGER_PD_TRANSMIT_pd_retry_cnt_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_tx_bytecnt : 8;
    } reg;
} SOC_SCHARGER_PD_TXBYTECNT_UNION;
#endif
#define SOC_SCHARGER_PD_TXBYTECNT_pd_tx_bytecnt_START (0)
#define SOC_SCHARGER_PD_TXBYTECNT_pd_tx_bytecnt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_tx_head_l : 8;
    } reg;
} SOC_SCHARGER_PD_TXHEADL_UNION;
#endif
#define SOC_SCHARGER_PD_TXHEADL_pd_tx_head_l_START (0)
#define SOC_SCHARGER_PD_TXHEADL_pd_tx_head_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_tx_head_h : 8;
    } reg;
} SOC_SCHARGER_PD_TXHEADH_UNION;
#endif
#define SOC_SCHARGER_PD_TXHEADH_pd_tx_head_h_START (0)
#define SOC_SCHARGER_PD_TXHEADH_pd_tx_head_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_tx_data : 8;
    } reg;
} SOC_SCHARGER_PD_TXDATA_UNION;
#endif
#define SOC_SCHARGER_PD_TXDATA_pd_tx_data_START (0)
#define SOC_SCHARGER_PD_TXDATA_pd_tx_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_vol_l : 8;
    } reg;
} SOC_SCHARGER_PD_VBUS_VOL_L_UNION;
#endif
#define SOC_SCHARGER_PD_VBUS_VOL_L_pd_vbus_vol_l_START (0)
#define SOC_SCHARGER_PD_VBUS_VOL_L_pd_vbus_vol_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_vol_h : 8;
    } reg;
} SOC_SCHARGER_PD_VBUS_VOL_H_UNION;
#endif
#define SOC_SCHARGER_PD_VBUS_VOL_H_pd_vbus_vol_h_START (0)
#define SOC_SCHARGER_PD_VBUS_VOL_H_pd_vbus_vol_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_snk_disc_l : 8;
    } reg;
} SOC_SCHARGER_PD_VBUS_SNK_DISCL_UNION;
#endif
#define SOC_SCHARGER_PD_VBUS_SNK_DISCL_pd_vbus_snk_disc_l_START (0)
#define SOC_SCHARGER_PD_VBUS_SNK_DISCL_pd_vbus_snk_disc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_snk_disc_h : 8;
    } reg;
} SOC_SCHARGER_PD_VBUS_SNK_DISCH_UNION;
#endif
#define SOC_SCHARGER_PD_VBUS_SNK_DISCH_pd_vbus_snk_disc_h_START (0)
#define SOC_SCHARGER_PD_VBUS_SNK_DISCH_pd_vbus_snk_disc_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_stop_disc_l : 8;
    } reg;
} SOC_SCHARGER_PD_VBUS_STOP_DISCL_UNION;
#endif
#define SOC_SCHARGER_PD_VBUS_STOP_DISCL_pd_vbus_stop_disc_l_START (0)
#define SOC_SCHARGER_PD_VBUS_STOP_DISCL_pd_vbus_stop_disc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_vbus_stop_disc_h : 8;
    } reg;
} SOC_SCHARGER_PD_VBUS_STOP_DISCH_UNION;
#endif
#define SOC_SCHARGER_PD_VBUS_STOP_DISCH_pd_vbus_stop_disc_h_START (0)
#define SOC_SCHARGER_PD_VBUS_STOP_DISCH_pd_vbus_stop_disc_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_valarm_high_l : 8;
    } reg;
} SOC_SCHARGER_PD_VALARMH_CFGL_UNION;
#endif
#define SOC_SCHARGER_PD_VALARMH_CFGL_pd_valarm_high_l_START (0)
#define SOC_SCHARGER_PD_VALARMH_CFGL_pd_valarm_high_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_valarm_high_h : 8;
    } reg;
} SOC_SCHARGER_PD_VALARMH_CFGH_UNION;
#endif
#define SOC_SCHARGER_PD_VALARMH_CFGH_pd_valarm_high_h_START (0)
#define SOC_SCHARGER_PD_VALARMH_CFGH_pd_valarm_high_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_valarm_low_l : 8;
    } reg;
} SOC_SCHARGER_PD_VALARML_CFGL_UNION;
#endif
#define SOC_SCHARGER_PD_VALARML_CFGL_pd_valarm_low_l_START (0)
#define SOC_SCHARGER_PD_VALARML_CFGL_pd_valarm_low_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_valarm_low_h : 8;
    } reg;
} SOC_SCHARGER_PD_VALARML_CFGH_UNION;
#endif
#define SOC_SCHARGER_PD_VALARML_CFGH_pd_valarm_low_h_START (0)
#define SOC_SCHARGER_PD_VALARML_CFGH_pd_valarm_low_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_frs_en : 1;
        unsigned char da_vconn_dis_en : 1;
        unsigned char pd_drp_thres : 2;
        unsigned char pd_try_snk_en : 1;
        unsigned char da_force_unplug_en : 1;
        unsigned char pd_bmc_cdr_sel : 1;
        unsigned char pd_cc_orient_sel : 1;
    } reg;
} SOC_SCHARGER_PD_VDM_CFG_0_UNION;
#endif
#define SOC_SCHARGER_PD_VDM_CFG_0_da_frs_en_START (0)
#define SOC_SCHARGER_PD_VDM_CFG_0_da_frs_en_END (0)
#define SOC_SCHARGER_PD_VDM_CFG_0_da_vconn_dis_en_START (1)
#define SOC_SCHARGER_PD_VDM_CFG_0_da_vconn_dis_en_END (1)
#define SOC_SCHARGER_PD_VDM_CFG_0_pd_drp_thres_START (2)
#define SOC_SCHARGER_PD_VDM_CFG_0_pd_drp_thres_END (3)
#define SOC_SCHARGER_PD_VDM_CFG_0_pd_try_snk_en_START (4)
#define SOC_SCHARGER_PD_VDM_CFG_0_pd_try_snk_en_END (4)
#define SOC_SCHARGER_PD_VDM_CFG_0_da_force_unplug_en_START (5)
#define SOC_SCHARGER_PD_VDM_CFG_0_da_force_unplug_en_END (5)
#define SOC_SCHARGER_PD_VDM_CFG_0_pd_bmc_cdr_sel_START (6)
#define SOC_SCHARGER_PD_VDM_CFG_0_pd_bmc_cdr_sel_END (6)
#define SOC_SCHARGER_PD_VDM_CFG_0_pd_cc_orient_sel_START (7)
#define SOC_SCHARGER_PD_VDM_CFG_0_pd_cc_orient_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char enable_pd : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_PD_VDM_ENABLE_UNION;
#endif
#define SOC_SCHARGER_PD_VDM_ENABLE_enable_pd_START (0)
#define SOC_SCHARGER_PD_VDM_ENABLE_enable_pd_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tc_fsm_reset : 1;
        unsigned char pd_fsm_reset : 1;
        unsigned char pd_tx_phy_soft_reset : 1;
        unsigned char pd_rx_phy_soft_reset : 1;
        unsigned char pd_snk_disc_by_cc : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_PD_VDM_CFG_1_UNION;
#endif
#define SOC_SCHARGER_PD_VDM_CFG_1_tc_fsm_reset_START (0)
#define SOC_SCHARGER_PD_VDM_CFG_1_tc_fsm_reset_END (0)
#define SOC_SCHARGER_PD_VDM_CFG_1_pd_fsm_reset_START (1)
#define SOC_SCHARGER_PD_VDM_CFG_1_pd_fsm_reset_END (1)
#define SOC_SCHARGER_PD_VDM_CFG_1_pd_tx_phy_soft_reset_START (2)
#define SOC_SCHARGER_PD_VDM_CFG_1_pd_tx_phy_soft_reset_END (2)
#define SOC_SCHARGER_PD_VDM_CFG_1_pd_rx_phy_soft_reset_START (3)
#define SOC_SCHARGER_PD_VDM_CFG_1_pd_rx_phy_soft_reset_END (3)
#define SOC_SCHARGER_PD_VDM_CFG_1_pd_snk_disc_by_cc_START (4)
#define SOC_SCHARGER_PD_VDM_CFG_1_pd_snk_disc_by_cc_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_rdata_sel : 4;
        unsigned char pd_dbg_module_sel : 3;
        unsigned char pd_dbg_rdata_en : 1;
    } reg;
} SOC_SCHARGER_PD_DBG_RDATA_CFG_UNION;
#endif
#define SOC_SCHARGER_PD_DBG_RDATA_CFG_pd_dbg_rdata_sel_START (0)
#define SOC_SCHARGER_PD_DBG_RDATA_CFG_pd_dbg_rdata_sel_END (3)
#define SOC_SCHARGER_PD_DBG_RDATA_CFG_pd_dbg_module_sel_START (4)
#define SOC_SCHARGER_PD_DBG_RDATA_CFG_pd_dbg_module_sel_END (6)
#define SOC_SCHARGER_PD_DBG_RDATA_CFG_pd_dbg_rdata_en_START (7)
#define SOC_SCHARGER_PD_DBG_RDATA_CFG_pd_dbg_rdata_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_rdata : 8;
    } reg;
} SOC_SCHARGER_PD_DBG_RDATA_UNION;
#endif
#define SOC_SCHARGER_PD_DBG_RDATA_pd_dbg_rdata_START (0)
#define SOC_SCHARGER_PD_DBG_RDATA_pd_dbg_rdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char page_select : 2;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_VDM_PAGE_SELECT_UNION;
#endif
#define SOC_SCHARGER_VDM_PAGE_SELECT_page_select_START (0)
#define SOC_SCHARGER_VDM_PAGE_SELECT_page_select_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char attach : 1;
        unsigned char reserved : 5;
        unsigned char dvc : 2;
    } reg;
} SOC_SCHARGER_STATUIS_UNION;
#endif
#define SOC_SCHARGER_STATUIS_attach_START (0)
#define SOC_SCHARGER_STATUIS_attach_END (0)
#define SOC_SCHARGER_STATUIS_dvc_START (6)
#define SOC_SCHARGER_STATUIS_dvc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sndcmd : 1;
        unsigned char reserved_0: 1;
        unsigned char mstr_rst : 1;
        unsigned char enable : 1;
        unsigned char reserved_1: 4;
    } reg;
} SOC_SCHARGER_CNTL_UNION;
#endif
#define SOC_SCHARGER_CNTL_sndcmd_START (0)
#define SOC_SCHARGER_CNTL_sndcmd_END (0)
#define SOC_SCHARGER_CNTL_mstr_rst_START (2)
#define SOC_SCHARGER_CNTL_mstr_rst_END (2)
#define SOC_SCHARGER_CNTL_enable_START (3)
#define SOC_SCHARGER_CNTL_enable_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cmd : 8;
    } reg;
} SOC_SCHARGER_CMD_UNION;
#endif
#define SOC_SCHARGER_CMD_cmd_START (0)
#define SOC_SCHARGER_CMD_cmd_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_length : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_FCP_LENGTH_UNION;
#endif
#define SOC_SCHARGER_FCP_LENGTH_fcp_length_START (0)
#define SOC_SCHARGER_FCP_LENGTH_fcp_length_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char addr : 8;
    } reg;
} SOC_SCHARGER_ADDR_UNION;
#endif
#define SOC_SCHARGER_ADDR_addr_START (0)
#define SOC_SCHARGER_ADDR_addr_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data0 : 8;
    } reg;
} SOC_SCHARGER_DATA0_UNION;
#endif
#define SOC_SCHARGER_DATA0_data0_START (0)
#define SOC_SCHARGER_DATA0_data0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data1 : 8;
    } reg;
} SOC_SCHARGER_DATA1_UNION;
#endif
#define SOC_SCHARGER_DATA1_data1_START (0)
#define SOC_SCHARGER_DATA1_data1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data2 : 8;
    } reg;
} SOC_SCHARGER_DATA2_UNION;
#endif
#define SOC_SCHARGER_DATA2_data2_START (0)
#define SOC_SCHARGER_DATA2_data2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data3 : 8;
    } reg;
} SOC_SCHARGER_DATA3_UNION;
#endif
#define SOC_SCHARGER_DATA3_data3_START (0)
#define SOC_SCHARGER_DATA3_data3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data4 : 8;
    } reg;
} SOC_SCHARGER_DATA4_UNION;
#endif
#define SOC_SCHARGER_DATA4_data4_START (0)
#define SOC_SCHARGER_DATA4_data4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data5 : 8;
    } reg;
} SOC_SCHARGER_DATA5_UNION;
#endif
#define SOC_SCHARGER_DATA5_data5_START (0)
#define SOC_SCHARGER_DATA5_data5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data6 : 8;
    } reg;
} SOC_SCHARGER_DATA6_UNION;
#endif
#define SOC_SCHARGER_DATA6_data6_START (0)
#define SOC_SCHARGER_DATA6_data6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data7 : 8;
    } reg;
} SOC_SCHARGER_DATA7_UNION;
#endif
#define SOC_SCHARGER_DATA7_data7_START (0)
#define SOC_SCHARGER_DATA7_data7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata0 : 8;
    } reg;
} SOC_SCHARGER_FCP_RDATA0_UNION;
#endif
#define SOC_SCHARGER_FCP_RDATA0_fcp_rdata0_START (0)
#define SOC_SCHARGER_FCP_RDATA0_fcp_rdata0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata1 : 8;
    } reg;
} SOC_SCHARGER_FCP_RDATA1_UNION;
#endif
#define SOC_SCHARGER_FCP_RDATA1_fcp_rdata1_START (0)
#define SOC_SCHARGER_FCP_RDATA1_fcp_rdata1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata2 : 8;
    } reg;
} SOC_SCHARGER_FCP_RDATA2_UNION;
#endif
#define SOC_SCHARGER_FCP_RDATA2_fcp_rdata2_START (0)
#define SOC_SCHARGER_FCP_RDATA2_fcp_rdata2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata3 : 8;
    } reg;
} SOC_SCHARGER_FCP_RDATA3_UNION;
#endif
#define SOC_SCHARGER_FCP_RDATA3_fcp_rdata3_START (0)
#define SOC_SCHARGER_FCP_RDATA3_fcp_rdata3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata4 : 8;
    } reg;
} SOC_SCHARGER_FCP_RDATA4_UNION;
#endif
#define SOC_SCHARGER_FCP_RDATA4_fcp_rdata4_START (0)
#define SOC_SCHARGER_FCP_RDATA4_fcp_rdata4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata5 : 8;
    } reg;
} SOC_SCHARGER_FCP_RDATA5_UNION;
#endif
#define SOC_SCHARGER_FCP_RDATA5_fcp_rdata5_START (0)
#define SOC_SCHARGER_FCP_RDATA5_fcp_rdata5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata6 : 8;
    } reg;
} SOC_SCHARGER_FCP_RDATA6_UNION;
#endif
#define SOC_SCHARGER_FCP_RDATA6_fcp_rdata6_START (0)
#define SOC_SCHARGER_FCP_RDATA6_fcp_rdata6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata7 : 8;
    } reg;
} SOC_SCHARGER_FCP_RDATA7_UNION;
#endif
#define SOC_SCHARGER_FCP_RDATA7_fcp_rdata7_START (0)
#define SOC_SCHARGER_FCP_RDATA7_fcp_rdata7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char nack_alarm : 1;
        unsigned char ack_alarm : 1;
        unsigned char crcpar : 1;
        unsigned char nack : 1;
        unsigned char reserved_1 : 1;
        unsigned char ack : 1;
        unsigned char cmdcpl : 1;
    } reg;
} SOC_SCHARGER_ISR1_UNION;
#endif
#define SOC_SCHARGER_ISR1_nack_alarm_START (1)
#define SOC_SCHARGER_ISR1_nack_alarm_END (1)
#define SOC_SCHARGER_ISR1_ack_alarm_START (2)
#define SOC_SCHARGER_ISR1_ack_alarm_END (2)
#define SOC_SCHARGER_ISR1_crcpar_START (3)
#define SOC_SCHARGER_ISR1_crcpar_END (3)
#define SOC_SCHARGER_ISR1_nack_START (4)
#define SOC_SCHARGER_ISR1_nack_END (4)
#define SOC_SCHARGER_ISR1_ack_START (6)
#define SOC_SCHARGER_ISR1_ack_END (6)
#define SOC_SCHARGER_ISR1_cmdcpl_START (7)
#define SOC_SCHARGER_ISR1_cmdcpl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0: 1;
        unsigned char protstat : 1;
        unsigned char reserved_1: 1;
        unsigned char parrx : 1;
        unsigned char crcrx : 1;
        unsigned char reserved_2: 3;
    } reg;
} SOC_SCHARGER_ISR2_UNION;
#endif
#define SOC_SCHARGER_ISR2_protstat_START (1)
#define SOC_SCHARGER_ISR2_protstat_END (1)
#define SOC_SCHARGER_ISR2_parrx_START (3)
#define SOC_SCHARGER_ISR2_parrx_END (3)
#define SOC_SCHARGER_ISR2_crcrx_START (4)
#define SOC_SCHARGER_ISR2_crcrx_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char nack_alarm_mk : 1;
        unsigned char ack_alarm_mk : 1;
        unsigned char crcpar_mk : 1;
        unsigned char nack_mk : 1;
        unsigned char reserved_1 : 1;
        unsigned char ack_mk : 1;
        unsigned char cmdcpl_mk : 1;
    } reg;
} SOC_SCHARGER_IMR1_UNION;
#endif
#define SOC_SCHARGER_IMR1_nack_alarm_mk_START (1)
#define SOC_SCHARGER_IMR1_nack_alarm_mk_END (1)
#define SOC_SCHARGER_IMR1_ack_alarm_mk_START (2)
#define SOC_SCHARGER_IMR1_ack_alarm_mk_END (2)
#define SOC_SCHARGER_IMR1_crcpar_mk_START (3)
#define SOC_SCHARGER_IMR1_crcpar_mk_END (3)
#define SOC_SCHARGER_IMR1_nack_mk_START (4)
#define SOC_SCHARGER_IMR1_nack_mk_END (4)
#define SOC_SCHARGER_IMR1_ack_mk_START (6)
#define SOC_SCHARGER_IMR1_ack_mk_END (6)
#define SOC_SCHARGER_IMR1_cmdcpl_mk_START (7)
#define SOC_SCHARGER_IMR1_cmdcpl_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved_0 : 1;
        unsigned char protstat_mk : 1;
        unsigned char reserved_1 : 1;
        unsigned char parrx_mk : 1;
        unsigned char crcrx_mk : 1;
        unsigned char reserved_2 : 3;
    } reg;
} SOC_SCHARGER_IMR2_UNION;
#endif
#define SOC_SCHARGER_IMR2_protstat_mk_START (1)
#define SOC_SCHARGER_IMR2_protstat_mk_END (1)
#define SOC_SCHARGER_IMR2_parrx_mk_START (3)
#define SOC_SCHARGER_IMR2_parrx_mk_END (3)
#define SOC_SCHARGER_IMR2_crcrx_mk_START (4)
#define SOC_SCHARGER_IMR2_crcrx_mk_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_set_d60m_int : 1;
        unsigned char fcp_en_det_int : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_FCP_IRQ5_UNION;
#endif
#define SOC_SCHARGER_FCP_IRQ5_fcp_set_d60m_int_START (0)
#define SOC_SCHARGER_FCP_IRQ5_fcp_set_d60m_int_END (0)
#define SOC_SCHARGER_FCP_IRQ5_fcp_en_det_int_START (1)
#define SOC_SCHARGER_FCP_IRQ5_fcp_en_det_int_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_set_d60m_r_mk : 1;
        unsigned char fcp_en_det_int_mk : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_FCP_IRQ5_MASK_UNION;
#endif
#define SOC_SCHARGER_FCP_IRQ5_MASK_fcp_set_d60m_r_mk_START (0)
#define SOC_SCHARGER_FCP_IRQ5_MASK_fcp_set_d60m_r_mk_END (0)
#define SOC_SCHARGER_FCP_IRQ5_MASK_fcp_en_det_int_mk_START (1)
#define SOC_SCHARGER_FCP_IRQ5_MASK_fcp_en_det_int_mk_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_clk_test : 1;
        unsigned char fcp_mode : 1;
        unsigned char fcp_cmp_en : 1;
        unsigned char fcp_det_en : 1;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_FCP_CTRL_UNION;
#endif
#define SOC_SCHARGER_FCP_CTRL_fcp_clk_test_START (0)
#define SOC_SCHARGER_FCP_CTRL_fcp_clk_test_END (0)
#define SOC_SCHARGER_FCP_CTRL_fcp_mode_START (1)
#define SOC_SCHARGER_FCP_CTRL_fcp_mode_END (1)
#define SOC_SCHARGER_FCP_CTRL_fcp_cmp_en_START (2)
#define SOC_SCHARGER_FCP_CTRL_fcp_cmp_en_END (2)
#define SOC_SCHARGER_FCP_CTRL_fcp_det_en_START (3)
#define SOC_SCHARGER_FCP_CTRL_fcp_det_en_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_mod2_set : 2;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_FCP_MODE2_SET_UNION;
#endif
#define SOC_SCHARGER_FCP_MODE2_SET_fcp_mod2_set_START (0)
#define SOC_SCHARGER_FCP_MODE2_SET_fcp_mod2_set_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_set_d60m_r : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_FCP_ADAP_CTRL_UNION;
#endif
#define SOC_SCHARGER_FCP_ADAP_CTRL_fcp_set_d60m_r_START (0)
#define SOC_SCHARGER_FCP_ADAP_CTRL_fcp_set_d60m_r_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_rdata_ready : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_RDATA_READY_UNION;
#endif
#define SOC_SCHARGER_RDATA_READY_fcp_rdata_ready_START (0)
#define SOC_SCHARGER_RDATA_READY_fcp_rdata_ready_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_soft_rst_cfg : 8;
    } reg;
} SOC_SCHARGER_FCP_SOFT_RST_CTRL_UNION;
#endif
#define SOC_SCHARGER_FCP_SOFT_RST_CTRL_fcp_soft_rst_cfg_START (0)
#define SOC_SCHARGER_FCP_SOFT_RST_CTRL_fcp_soft_rst_cfg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rdata_parity_err0 : 1;
        unsigned char rdata_parity_err1 : 1;
        unsigned char rdata_parity_err2 : 1;
        unsigned char rdata_parity_err3 : 1;
        unsigned char rdata_parity_err4 : 1;
        unsigned char rdata_parity_err5 : 1;
        unsigned char rdata_parity_err6 : 1;
        unsigned char rdata_parity_err7 : 1;
    } reg;
} SOC_SCHARGER_FCP_RDATA_PARITY_ERR_UNION;
#endif
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err0_START (0)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err0_END (0)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err1_START (1)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err1_END (1)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err2_START (2)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err2_END (2)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err3_START (3)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err3_END (3)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err4_START (4)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err4_END (4)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err5_START (5)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err5_END (5)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err6_START (6)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err6_END (6)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err7_START (7)
#define SOC_SCHARGER_FCP_RDATA_PARITY_ERR_rdata_parity_err7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_init_retry_cfg : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_FCP_INIT_RETRY_CFG_UNION;
#endif
#define SOC_SCHARGER_FCP_INIT_RETRY_CFG_fcp_init_retry_cfg_START (0)
#define SOC_SCHARGER_FCP_INIT_RETRY_CFG_fcp_init_retry_cfg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_soft_rst_n : 8;
    } reg;
} SOC_SCHARGER_HKADC_SOFT_RST_CTRL_UNION;
#endif
#define SOC_SCHARGER_HKADC_SOFT_RST_CTRL_sc_hkadc_soft_rst_n_START (0)
#define SOC_SCHARGER_HKADC_SOFT_RST_CTRL_sc_hkadc_soft_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_chanel_sel : 4;
        unsigned char sc_hkadc_seq_conv : 1;
        unsigned char sc_hkadc_seq_loop : 1;
        unsigned char sc_hkadc_cali_ref : 1;
        unsigned char sc_hkadc_en : 1;
    } reg;
} SOC_SCHARGER_HKADC_CTRL0_UNION;
#endif
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_chanel_sel_START (0)
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_chanel_sel_END (3)
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_seq_conv_START (4)
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_seq_conv_END (4)
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_seq_loop_START (5)
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_seq_loop_END (5)
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_cali_ref_START (6)
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_cali_ref_END (6)
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_en_START (7)
#define SOC_SCHARGER_HKADC_CTRL0_sc_hkadc_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_start_cfg : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_HKADC_START_UNION;
#endif
#define SOC_SCHARGER_HKADC_START_sc_hkadc_start_cfg_START (0)
#define SOC_SCHARGER_HKADC_START_sc_hkadc_start_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_avg_times : 2;
        unsigned char sc_hkadc_cali_set : 1;
        unsigned char sc_hkadc_cali_realtime : 1;
        unsigned char sc_hkadc_chopper : 1;
        unsigned char sc_hkadc_chopper_time : 1;
        unsigned char sc_hkadc_chopper_en : 1;
        unsigned char sc_hkadc_cul_time : 1;
    } reg;
} SOC_SCHARGER_HKADC_CTRL1_UNION;
#endif
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_avg_times_START (0)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_avg_times_END (1)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_cali_set_START (2)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_cali_set_END (2)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_cali_realtime_START (3)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_cali_realtime_END (3)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_chopper_START (4)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_chopper_END (4)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_chopper_time_START (5)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_chopper_time_END (5)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_chopper_en_START (6)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_chopper_en_END (6)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_cul_time_START (7)
#define SOC_SCHARGER_HKADC_CTRL1_sc_hkadc_cul_time_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_seq_chanel_h : 8;
    } reg;
} SOC_SCHARGER_HKADC_SEQ_CH_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_SEQ_CH_H_sc_hkadc_seq_chanel_h_START (0)
#define SOC_SCHARGER_HKADC_SEQ_CH_H_sc_hkadc_seq_chanel_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_seq_chanel_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_SEQ_CH_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_SEQ_CH_L_sc_hkadc_seq_chanel_l_START (0)
#define SOC_SCHARGER_HKADC_SEQ_CH_L_sc_hkadc_seq_chanel_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_test_mode : 1;
        unsigned char reserved_0 : 3;
        unsigned char sc_sohov_timer : 2;
        unsigned char reserved_1 : 1;
        unsigned char sc_sohov_en : 1;
    } reg;
} SOC_SCHARGER_HKADC_CTRL2_UNION;
#endif
#define SOC_SCHARGER_HKADC_CTRL2_sc_soh_test_mode_START (0)
#define SOC_SCHARGER_HKADC_CTRL2_sc_soh_test_mode_END (0)
#define SOC_SCHARGER_HKADC_CTRL2_sc_sohov_timer_START (4)
#define SOC_SCHARGER_HKADC_CTRL2_sc_sohov_timer_END (5)
#define SOC_SCHARGER_HKADC_CTRL2_sc_sohov_en_START (7)
#define SOC_SCHARGER_HKADC_CTRL2_sc_sohov_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_dischg_en : 8;
    } reg;
} SOC_SCHARGER_SOH_DISCHG_EN_UNION;
#endif
#define SOC_SCHARGER_SOH_DISCHG_EN_sc_soh_dischg_en_START (0)
#define SOC_SCHARGER_SOH_DISCHG_EN_sc_soh_dischg_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_pulse_num : 2;
        unsigned char reserved_0 : 2;
        unsigned char sc_acr_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_ACR_CTRL_UNION;
#endif
#define SOC_SCHARGER_ACR_CTRL_sc_acr_pulse_num_START (0)
#define SOC_SCHARGER_ACR_CTRL_sc_acr_pulse_num_END (1)
#define SOC_SCHARGER_ACR_CTRL_sc_acr_en_START (4)
#define SOC_SCHARGER_ACR_CTRL_sc_acr_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_rd_req : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_HKADC_RD_SEQ_UNION;
#endif
#define SOC_SCHARGER_HKADC_RD_SEQ_sc_hkadc_rd_req_START (0)
#define SOC_SCHARGER_HKADC_RD_SEQ_sc_hkadc_rd_req_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pulse_non_chg_flag : 1;
        unsigned char reserved_0 : 3;
        unsigned char hkadc_data_valid : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_PULSE_NON_CHG_FLAG_UNION;
#endif
#define SOC_SCHARGER_PULSE_NON_CHG_FLAG_pulse_non_chg_flag_START (0)
#define SOC_SCHARGER_PULSE_NON_CHG_FLAG_pulse_non_chg_flag_END (0)
#define SOC_SCHARGER_PULSE_NON_CHG_FLAG_hkadc_data_valid_START (4)
#define SOC_SCHARGER_PULSE_NON_CHG_FLAG_hkadc_data_valid_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vusb_adc_l : 8;
    } reg;
} SOC_SCHARGER_VUSB_ADC_L_UNION;
#endif
#define SOC_SCHARGER_VUSB_ADC_L_vusb_adc_l_START (0)
#define SOC_SCHARGER_VUSB_ADC_L_vusb_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vusb_adc_h : 6;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_VUSB_ADC_H_UNION;
#endif
#define SOC_SCHARGER_VUSB_ADC_H_vusb_adc_h_START (0)
#define SOC_SCHARGER_VUSB_ADC_H_vusb_adc_h_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibus_adc_l : 8;
    } reg;
} SOC_SCHARGER_IBUS_ADC_L_UNION;
#endif
#define SOC_SCHARGER_IBUS_ADC_L_ibus_adc_l_START (0)
#define SOC_SCHARGER_IBUS_ADC_L_ibus_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibus_adc_h : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_IBUS_ADC_H_UNION;
#endif
#define SOC_SCHARGER_IBUS_ADC_H_ibus_adc_h_START (0)
#define SOC_SCHARGER_IBUS_ADC_H_ibus_adc_h_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbus_adc_l : 8;
    } reg;
} SOC_SCHARGER_VBUS_ADC_L_UNION;
#endif
#define SOC_SCHARGER_VBUS_ADC_L_vbus_adc_l_START (0)
#define SOC_SCHARGER_VBUS_ADC_L_vbus_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbus_adc_h : 6;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_VBUS_ADC_H_UNION;
#endif
#define SOC_SCHARGER_VBUS_ADC_H_vbus_adc_h_START (0)
#define SOC_SCHARGER_VBUS_ADC_H_vbus_adc_h_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vout_adc_l : 8;
    } reg;
} SOC_SCHARGER_VOUT_ADC_L_UNION;
#endif
#define SOC_SCHARGER_VOUT_ADC_L_vout_adc_l_START (0)
#define SOC_SCHARGER_VOUT_ADC_L_vout_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vout_adc_h : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_VOUT_ADC_H_UNION;
#endif
#define SOC_SCHARGER_VOUT_ADC_H_vout_adc_h_START (0)
#define SOC_SCHARGER_VOUT_ADC_H_vout_adc_h_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat_adc_l : 8;
    } reg;
} SOC_SCHARGER_VBAT_ADC_L_UNION;
#endif
#define SOC_SCHARGER_VBAT_ADC_L_vbat_adc_l_START (0)
#define SOC_SCHARGER_VBAT_ADC_L_vbat_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat_adc_h : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_VBAT_ADC_H_UNION;
#endif
#define SOC_SCHARGER_VBAT_ADC_H_vbat_adc_h_START (0)
#define SOC_SCHARGER_VBAT_ADC_H_vbat_adc_h_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibat_adc_l : 8;
    } reg;
} SOC_SCHARGER_IBAT_ADC_L_UNION;
#endif
#define SOC_SCHARGER_IBAT_ADC_L_ibat_adc_l_START (0)
#define SOC_SCHARGER_IBAT_ADC_L_ibat_adc_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ibat_adc_h : 6;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_IBAT_ADC_H_UNION;
#endif
#define SOC_SCHARGER_IBAT_ADC_H_ibat_adc_h_START (0)
#define SOC_SCHARGER_IBAT_ADC_H_ibat_adc_h_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_6_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_6_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_6_L_hkadc_data_6_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_6_L_hkadc_data_6_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_6_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_6_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_6_H_hkadc_data_6_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_6_H_hkadc_data_6_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_7_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_7_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_7_L_hkadc_data_7_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_7_L_hkadc_data_7_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_7_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_7_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_7_H_hkadc_data_7_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_7_H_hkadc_data_7_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_8_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_8_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_8_L_hkadc_data_8_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_8_L_hkadc_data_8_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_8_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_8_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_8_H_hkadc_data_8_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_8_H_hkadc_data_8_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_9_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_9_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_9_L_hkadc_data_9_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_9_L_hkadc_data_9_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_9_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_9_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_9_H_hkadc_data_9_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_9_H_hkadc_data_9_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_10_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_10_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_10_L_hkadc_data_10_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_10_L_hkadc_data_10_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_10_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_10_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_10_H_hkadc_data_10_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_10_H_hkadc_data_10_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_11_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_11_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_11_L_hkadc_data_11_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_11_L_hkadc_data_11_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_11_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_11_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_11_H_hkadc_data_11_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_11_H_hkadc_data_11_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_12_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_12_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_12_L_hkadc_data_12_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_12_L_hkadc_data_12_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_12_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_12_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_12_H_hkadc_data_12_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_12_H_hkadc_data_12_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_13_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_13_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_13_L_hkadc_data_13_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_13_L_hkadc_data_13_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_13_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_13_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_13_H_hkadc_data_13_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_13_H_hkadc_data_13_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_14_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_14_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_14_L_hkadc_data_14_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_14_L_hkadc_data_14_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_14_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_14_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_14_H_hkadc_data_14_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_14_H_hkadc_data_14_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_15_l : 8;
    } reg;
} SOC_SCHARGER_HKADC_DATA_15_L_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_15_L_hkadc_data_15_l_START (0)
#define SOC_SCHARGER_HKADC_DATA_15_L_hkadc_data_15_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_data_15_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_HKADC_DATA_15_H_UNION;
#endif
#define SOC_SCHARGER_HKADC_DATA_15_H_hkadc_data_15_h_START (0)
#define SOC_SCHARGER_HKADC_DATA_15_H_hkadc_data_15_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_cdr_cfg_0 : 8;
    } reg;
} SOC_SCHARGER_PD_CDR_CFG_0_UNION;
#endif
#define SOC_SCHARGER_PD_CDR_CFG_0_pd_cdr_cfg_0_START (0)
#define SOC_SCHARGER_PD_CDR_CFG_0_pd_cdr_cfg_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_cdr_cfg_1 : 8;
    } reg;
} SOC_SCHARGER_PD_CDR_CFG_1_UNION;
#endif
#define SOC_SCHARGER_PD_CDR_CFG_1_pd_cdr_cfg_1_START (0)
#define SOC_SCHARGER_PD_CDR_CFG_1_pd_cdr_cfg_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_cfg_0 : 8;
    } reg;
} SOC_SCHARGER_PD_DBG_CFG_0_UNION;
#endif
#define SOC_SCHARGER_PD_DBG_CFG_0_pd_dbg_cfg_0_START (0)
#define SOC_SCHARGER_PD_DBG_CFG_0_pd_dbg_cfg_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_cfg_1 : 8;
    } reg;
} SOC_SCHARGER_PD_DBG_CFG_1_UNION;
#endif
#define SOC_SCHARGER_PD_DBG_CFG_1_pd_dbg_cfg_1_START (0)
#define SOC_SCHARGER_PD_DBG_CFG_1_pd_dbg_cfg_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_status_0 : 8;
    } reg;
} SOC_SCHARGER_PD_DBG_RO_0_UNION;
#endif
#define SOC_SCHARGER_PD_DBG_RO_0_pd_dbg_status_0_START (0)
#define SOC_SCHARGER_PD_DBG_RO_0_pd_dbg_status_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_status_1 : 8;
    } reg;
} SOC_SCHARGER_PD_DBG_RO_1_UNION;
#endif
#define SOC_SCHARGER_PD_DBG_RO_1_pd_dbg_status_1_START (0)
#define SOC_SCHARGER_PD_DBG_RO_1_pd_dbg_status_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_status_2 : 8;
    } reg;
} SOC_SCHARGER_PD_DBG_RO_2_UNION;
#endif
#define SOC_SCHARGER_PD_DBG_RO_2_pd_dbg_status_2_START (0)
#define SOC_SCHARGER_PD_DBG_RO_2_pd_dbg_status_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char pd_dbg_status_3 : 8;
    } reg;
} SOC_SCHARGER_PD_DBG_RO_3_UNION;
#endif
#define SOC_SCHARGER_PD_DBG_RO_3_pd_dbg_status_3_START (0)
#define SOC_SCHARGER_PD_DBG_RO_3_pd_dbg_status_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_sel : 8;
    } reg;
} SOC_SCHARGER_IRQ_FAKE_SEL_UNION;
#endif
#define SOC_SCHARGER_IRQ_FAKE_SEL_sc_irq_sel_START (0)
#define SOC_SCHARGER_IRQ_FAKE_SEL_sc_irq_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_fake_irq : 8;
    } reg;
} SOC_SCHARGER_IRQ_FAKE_UNION;
#endif
#define SOC_SCHARGER_IRQ_FAKE_sc_fake_irq_START (0)
#define SOC_SCHARGER_IRQ_FAKE_sc_fake_irq_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag : 8;
    } reg;
} SOC_SCHARGER_IRQ_FLAG_UNION;
#endif
#define SOC_SCHARGER_IRQ_FLAG_sc_irq_flag_START (0)
#define SOC_SCHARGER_IRQ_FLAG_sc_irq_flag_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_0 : 8;
    } reg;
} SOC_SCHARGER_IRQ_FLAG_0_UNION;
#endif
#define SOC_SCHARGER_IRQ_FLAG_0_sc_irq_flag_0_START (0)
#define SOC_SCHARGER_IRQ_FLAG_0_sc_irq_flag_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_1 : 8;
    } reg;
} SOC_SCHARGER_IRQ_FLAG_1_UNION;
#endif
#define SOC_SCHARGER_IRQ_FLAG_1_sc_irq_flag_1_START (0)
#define SOC_SCHARGER_IRQ_FLAG_1_sc_irq_flag_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_2 : 8;
    } reg;
} SOC_SCHARGER_IRQ_FLAG_2_UNION;
#endif
#define SOC_SCHARGER_IRQ_FLAG_2_sc_irq_flag_2_START (0)
#define SOC_SCHARGER_IRQ_FLAG_2_sc_irq_flag_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_3 : 8;
    } reg;
} SOC_SCHARGER_IRQ_FLAG_3_UNION;
#endif
#define SOC_SCHARGER_IRQ_FLAG_3_sc_irq_flag_3_START (0)
#define SOC_SCHARGER_IRQ_FLAG_3_sc_irq_flag_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_4 : 8;
    } reg;
} SOC_SCHARGER_IRQ_FLAG_4_UNION;
#endif
#define SOC_SCHARGER_IRQ_FLAG_4_sc_irq_flag_4_START (0)
#define SOC_SCHARGER_IRQ_FLAG_4_sc_irq_flag_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_5 : 8;
    } reg;
} SOC_SCHARGER_IRQ_FLAG_5_UNION;
#endif
#define SOC_SCHARGER_IRQ_FLAG_5_sc_irq_flag_5_START (0)
#define SOC_SCHARGER_IRQ_FLAG_5_sc_irq_flag_5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_6 : 8;
    } reg;
} SOC_SCHARGER_IRQ_FLAG_6_UNION;
#endif
#define SOC_SCHARGER_IRQ_FLAG_6_sc_irq_flag_6_START (0)
#define SOC_SCHARGER_IRQ_FLAG_6_sc_irq_flag_6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_flag_7 : 8;
    } reg;
} SOC_SCHARGER_IRQ_FLAG_7_UNION;
#endif
#define SOC_SCHARGER_IRQ_FLAG_7_sc_irq_flag_7_START (0)
#define SOC_SCHARGER_IRQ_FLAG_7_sc_irq_flag_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wd_rst_n : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_WDT_SOFT_RST_UNION;
#endif
#define SOC_SCHARGER_WDT_SOFT_RST_wd_rst_n_START (0)
#define SOC_SCHARGER_WDT_SOFT_RST_wd_rst_n_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_watchdog_timer : 3;
        unsigned char reserved_0 : 1;
        unsigned char sc_wdt_test_sel : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_WDT_CTRL_UNION;
#endif
#define SOC_SCHARGER_WDT_CTRL_sc_watchdog_timer_START (0)
#define SOC_SCHARGER_WDT_CTRL_sc_watchdog_timer_END (2)
#define SOC_SCHARGER_WDT_CTRL_sc_wdt_test_sel_START (4)
#define SOC_SCHARGER_WDT_CTRL_sc_wdt_test_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dc_ibat_regulator : 8;
    } reg;
} SOC_SCHARGER_DC_IBAT_REGULATOR_UNION;
#endif
#define SOC_SCHARGER_DC_IBAT_REGULATOR_sc_dc_ibat_regulator_START (0)
#define SOC_SCHARGER_DC_IBAT_REGULATOR_sc_dc_ibat_regulator_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dc_vbat_regulator : 7;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_DC_VBAT_REGULATOR_UNION;
#endif
#define SOC_SCHARGER_DC_VBAT_REGULATOR_sc_dc_vbat_regulator_START (0)
#define SOC_SCHARGER_DC_VBAT_REGULATOR_sc_dc_vbat_regulator_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dc_vout_regulator : 7;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_DC_VOUT_REGULATOR_UNION;
#endif
#define SOC_SCHARGER_DC_VOUT_REGULATOR_sc_dc_vout_regulator_START (0)
#define SOC_SCHARGER_DC_VOUT_REGULATOR_sc_dc_vout_regulator_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_otg_dmd_ofs : 4;
        unsigned char sc_otg_dmd_ramp : 3;
        unsigned char sc_otg_en : 1;
    } reg;
} SOC_SCHARGER_OTG_CFG_UNION;
#endif
#define SOC_SCHARGER_OTG_CFG_sc_otg_dmd_ofs_START (0)
#define SOC_SCHARGER_OTG_CFG_sc_otg_dmd_ofs_END (3)
#define SOC_SCHARGER_OTG_CFG_sc_otg_dmd_ramp_START (4)
#define SOC_SCHARGER_OTG_CFG_sc_otg_dmd_ramp_END (6)
#define SOC_SCHARGER_OTG_CFG_sc_otg_en_START (7)
#define SOC_SCHARGER_OTG_CFG_sc_otg_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_lvc_en : 1;
        unsigned char sc_pulse_mode_en : 1;
        unsigned char reserved_0 : 2;
        unsigned char sc_chg_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_PULSE_CHG_CFG0_UNION;
#endif
#define SOC_SCHARGER_PULSE_CHG_CFG0_sc_lvc_en_START (0)
#define SOC_SCHARGER_PULSE_CHG_CFG0_sc_lvc_en_END (0)
#define SOC_SCHARGER_PULSE_CHG_CFG0_sc_pulse_mode_en_START (1)
#define SOC_SCHARGER_PULSE_CHG_CFG0_sc_pulse_mode_en_END (1)
#define SOC_SCHARGER_PULSE_CHG_CFG0_sc_chg_en_START (4)
#define SOC_SCHARGER_PULSE_CHG_CFG0_sc_chg_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_chg_time_set : 5;
        unsigned char sc_sc_en : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_PULSE_CHG_CFG1_UNION;
#endif
#define SOC_SCHARGER_PULSE_CHG_CFG1_sc_chg_time_set_START (0)
#define SOC_SCHARGER_PULSE_CHG_CFG1_sc_chg_time_set_END (4)
#define SOC_SCHARGER_PULSE_CHG_CFG1_sc_sc_en_START (5)
#define SOC_SCHARGER_PULSE_CHG_CFG1_sc_sc_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_dischg_time_set : 8;
    } reg;
} SOC_SCHARGER_DISCHG_TIME_UNION;
#endif
#define SOC_SCHARGER_DISCHG_TIME_sc_dischg_time_set_START (0)
#define SOC_SCHARGER_DISCHG_TIME_sc_dischg_time_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char wdt_time_out_n : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_DIG_STATUS0_UNION;
#endif
#define SOC_SCHARGER_DIG_STATUS0_wdt_time_out_n_START (0)
#define SOC_SCHARGER_DIG_STATUS0_wdt_time_out_n_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_testbus_sel : 4;
        unsigned char sc_cg_testsig_sel : 3;
        unsigned char sc_cg_testbus_en : 1;
    } reg;
} SOC_SCHARGER_SC_TESTBUS_CFG_UNION;
#endif
#define SOC_SCHARGER_SC_TESTBUS_CFG_sc_cg_testbus_sel_START (0)
#define SOC_SCHARGER_SC_TESTBUS_CFG_sc_cg_testbus_sel_END (3)
#define SOC_SCHARGER_SC_TESTBUS_CFG_sc_cg_testsig_sel_START (4)
#define SOC_SCHARGER_SC_TESTBUS_CFG_sc_cg_testsig_sel_END (6)
#define SOC_SCHARGER_SC_TESTBUS_CFG_sc_cg_testbus_en_START (7)
#define SOC_SCHARGER_SC_TESTBUS_CFG_sc_cg_testbus_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cg_testbus_rdata : 8;
    } reg;
} SOC_SCHARGER_SC_TESTBUS_RDATA_UNION;
#endif
#define SOC_SCHARGER_SC_TESTBUS_RDATA_cg_testbus_rdata_START (0)
#define SOC_SCHARGER_SC_TESTBUS_RDATA_cg_testbus_rdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_glb_soft_rst_n : 8;
    } reg;
} SOC_SCHARGER_GLB_SOFT_RST_CTRL_UNION;
#endif
#define SOC_SCHARGER_GLB_SOFT_RST_CTRL_sc_cg_glb_soft_rst_n_START (0)
#define SOC_SCHARGER_GLB_SOFT_RST_CTRL_sc_cg_glb_soft_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_efuse_soft_rst_n : 8;
    } reg;
} SOC_SCHARGER_EFUSE_SOFT_RST_CTRL_UNION;
#endif
#define SOC_SCHARGER_EFUSE_SOFT_RST_CTRL_sc_cg_efuse_soft_rst_n_START (0)
#define SOC_SCHARGER_EFUSE_SOFT_RST_CTRL_sc_cg_efuse_soft_rst_n_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_cg_com_clk_en : 1;
        unsigned char sc_cg_red_clk_en : 1;
        unsigned char sc_cg_pd_clk_en : 1;
        unsigned char sc_cg_2m_clk_en : 1;
        unsigned char sc_cg_pd_clk_en_sel : 1;
        unsigned char sc_cg_2m_clk_en_sel : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_UNION;
#endif
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_com_clk_en_START (0)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_com_clk_en_END (0)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_red_clk_en_START (1)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_red_clk_en_END (1)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_pd_clk_en_START (2)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_pd_clk_en_END (2)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_2m_clk_en_START (3)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_2m_clk_en_END (3)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_pd_clk_en_sel_START (4)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_pd_clk_en_sel_END (4)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_2m_clk_en_sel_START (5)
#define SOC_SCHARGER_SC_CRG_CLK_EN_CTRL_sc_cg_2m_clk_en_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char buck_mode_cfg : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_BUCK_MODE_CFG_UNION;
#endif
#define SOC_SCHARGER_BUCK_MODE_CFG_buck_mode_cfg_START (0)
#define SOC_SCHARGER_BUCK_MODE_CFG_buck_mode_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_mode_cfg : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_SC_MODE_CFG_UNION;
#endif
#define SOC_SCHARGER_SC_MODE_CFG_sc_mode_cfg_START (0)
#define SOC_SCHARGER_SC_MODE_CFG_sc_mode_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char lvc_mode_cfg : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_LVC_MODE_CFG_UNION;
#endif
#define SOC_SCHARGER_LVC_MODE_CFG_lvc_mode_cfg_START (0)
#define SOC_SCHARGER_LVC_MODE_CFG_lvc_mode_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_buck_enb : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_SC_BUCK_ENB_UNION;
#endif
#define SOC_SCHARGER_SC_BUCK_ENB_sc_buck_enb_START (0)
#define SOC_SCHARGER_SC_BUCK_ENB_sc_buck_enb_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_ovp_mos_en : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_SC_OVP_MOS_EN_UNION;
#endif
#define SOC_SCHARGER_SC_OVP_MOS_EN_sc_ovp_mos_en_START (0)
#define SOC_SCHARGER_SC_OVP_MOS_EN_sc_ovp_mos_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char last_hand_fail_irq : 1;
        unsigned char tail_hand_fail_irq : 1;
        unsigned char trans_hand_fail_irq : 1;
        unsigned char init_hand_fail_irq : 1;
        unsigned char rx_data_det_irq : 1;
        unsigned char rx_head_det_irq : 1;
        unsigned char cmd_err_irq : 1;
        unsigned char length_err_irq : 1;
    } reg;
} SOC_SCHARGER_FCP_IRQ3_UNION;
#endif
#define SOC_SCHARGER_FCP_IRQ3_last_hand_fail_irq_START (0)
#define SOC_SCHARGER_FCP_IRQ3_last_hand_fail_irq_END (0)
#define SOC_SCHARGER_FCP_IRQ3_tail_hand_fail_irq_START (1)
#define SOC_SCHARGER_FCP_IRQ3_tail_hand_fail_irq_END (1)
#define SOC_SCHARGER_FCP_IRQ3_trans_hand_fail_irq_START (2)
#define SOC_SCHARGER_FCP_IRQ3_trans_hand_fail_irq_END (2)
#define SOC_SCHARGER_FCP_IRQ3_init_hand_fail_irq_START (3)
#define SOC_SCHARGER_FCP_IRQ3_init_hand_fail_irq_END (3)
#define SOC_SCHARGER_FCP_IRQ3_rx_data_det_irq_START (4)
#define SOC_SCHARGER_FCP_IRQ3_rx_data_det_irq_END (4)
#define SOC_SCHARGER_FCP_IRQ3_rx_head_det_irq_START (5)
#define SOC_SCHARGER_FCP_IRQ3_rx_head_det_irq_END (5)
#define SOC_SCHARGER_FCP_IRQ3_cmd_err_irq_START (6)
#define SOC_SCHARGER_FCP_IRQ3_cmd_err_irq_END (6)
#define SOC_SCHARGER_FCP_IRQ3_length_err_irq_START (7)
#define SOC_SCHARGER_FCP_IRQ3_length_err_irq_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char last_hand_no_respond_irq : 1;
        unsigned char tail_hand_no_respond_irq : 1;
        unsigned char trans_hand_no_respond_irq : 1;
        unsigned char init_hand_no_respond_irq : 1;
        unsigned char enable_hand_fail_irq : 1;
        unsigned char enable_hand_no_respond_irq : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_FCP_IRQ4_UNION;
#endif
#define SOC_SCHARGER_FCP_IRQ4_last_hand_no_respond_irq_START (0)
#define SOC_SCHARGER_FCP_IRQ4_last_hand_no_respond_irq_END (0)
#define SOC_SCHARGER_FCP_IRQ4_tail_hand_no_respond_irq_START (1)
#define SOC_SCHARGER_FCP_IRQ4_tail_hand_no_respond_irq_END (1)
#define SOC_SCHARGER_FCP_IRQ4_trans_hand_no_respond_irq_START (2)
#define SOC_SCHARGER_FCP_IRQ4_trans_hand_no_respond_irq_END (2)
#define SOC_SCHARGER_FCP_IRQ4_init_hand_no_respond_irq_START (3)
#define SOC_SCHARGER_FCP_IRQ4_init_hand_no_respond_irq_END (3)
#define SOC_SCHARGER_FCP_IRQ4_enable_hand_fail_irq_START (4)
#define SOC_SCHARGER_FCP_IRQ4_enable_hand_fail_irq_END (4)
#define SOC_SCHARGER_FCP_IRQ4_enable_hand_no_respond_irq_START (5)
#define SOC_SCHARGER_FCP_IRQ4_enable_hand_no_respond_irq_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char last_hand_fail_irq_mk : 1;
        unsigned char tail_hand_fail_irq_mk : 1;
        unsigned char trans_hand_fail_irq_mk : 1;
        unsigned char init_hand_fail_irq_mk : 1;
        unsigned char rx_data_det_irq_mk : 1;
        unsigned char rx_head_det_irq_mk : 1;
        unsigned char cmd_err_irq_mk : 1;
        unsigned char length_err_irq_mk : 1;
    } reg;
} SOC_SCHARGER_FCP_IRQ3_MASK_UNION;
#endif
#define SOC_SCHARGER_FCP_IRQ3_MASK_last_hand_fail_irq_mk_START (0)
#define SOC_SCHARGER_FCP_IRQ3_MASK_last_hand_fail_irq_mk_END (0)
#define SOC_SCHARGER_FCP_IRQ3_MASK_tail_hand_fail_irq_mk_START (1)
#define SOC_SCHARGER_FCP_IRQ3_MASK_tail_hand_fail_irq_mk_END (1)
#define SOC_SCHARGER_FCP_IRQ3_MASK_trans_hand_fail_irq_mk_START (2)
#define SOC_SCHARGER_FCP_IRQ3_MASK_trans_hand_fail_irq_mk_END (2)
#define SOC_SCHARGER_FCP_IRQ3_MASK_init_hand_fail_irq_mk_START (3)
#define SOC_SCHARGER_FCP_IRQ3_MASK_init_hand_fail_irq_mk_END (3)
#define SOC_SCHARGER_FCP_IRQ3_MASK_rx_data_det_irq_mk_START (4)
#define SOC_SCHARGER_FCP_IRQ3_MASK_rx_data_det_irq_mk_END (4)
#define SOC_SCHARGER_FCP_IRQ3_MASK_rx_head_det_irq_mk_START (5)
#define SOC_SCHARGER_FCP_IRQ3_MASK_rx_head_det_irq_mk_END (5)
#define SOC_SCHARGER_FCP_IRQ3_MASK_cmd_err_irq_mk_START (6)
#define SOC_SCHARGER_FCP_IRQ3_MASK_cmd_err_irq_mk_END (6)
#define SOC_SCHARGER_FCP_IRQ3_MASK_length_err_irq_mk_START (7)
#define SOC_SCHARGER_FCP_IRQ3_MASK_length_err_irq_mk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char last_hand_no_respond_irq_mk : 1;
        unsigned char tail_hand_no_respond_irq_mk : 1;
        unsigned char trans_hand_no_respond_irq_mk : 1;
        unsigned char init_hand_no_respond_irq_mk : 1;
        unsigned char enable_hand_fail_irq_mk : 1;
        unsigned char enable_hand_no_respond_irq_mk : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_FCP_IRQ4_MASK_UNION;
#endif
#define SOC_SCHARGER_FCP_IRQ4_MASK_last_hand_no_respond_irq_mk_START (0)
#define SOC_SCHARGER_FCP_IRQ4_MASK_last_hand_no_respond_irq_mk_END (0)
#define SOC_SCHARGER_FCP_IRQ4_MASK_tail_hand_no_respond_irq_mk_START (1)
#define SOC_SCHARGER_FCP_IRQ4_MASK_tail_hand_no_respond_irq_mk_END (1)
#define SOC_SCHARGER_FCP_IRQ4_MASK_trans_hand_no_respond_irq_mk_START (2)
#define SOC_SCHARGER_FCP_IRQ4_MASK_trans_hand_no_respond_irq_mk_END (2)
#define SOC_SCHARGER_FCP_IRQ4_MASK_init_hand_no_respond_irq_mk_START (3)
#define SOC_SCHARGER_FCP_IRQ4_MASK_init_hand_no_respond_irq_mk_END (3)
#define SOC_SCHARGER_FCP_IRQ4_MASK_enable_hand_fail_irq_mk_START (4)
#define SOC_SCHARGER_FCP_IRQ4_MASK_enable_hand_fail_irq_mk_END (4)
#define SOC_SCHARGER_FCP_IRQ4_MASK_enable_hand_no_respond_irq_mk_START (5)
#define SOC_SCHARGER_FCP_IRQ4_MASK_enable_hand_no_respond_irq_mk_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char mstate : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_MSTATE_UNION;
#endif
#define SOC_SCHARGER_MSTATE_mstate_START (0)
#define SOC_SCHARGER_MSTATE_mstate_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char crc_en : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_CRC_ENABLE_UNION;
#endif
#define SOC_SCHARGER_CRC_ENABLE_crc_en_START (0)
#define SOC_SCHARGER_CRC_ENABLE_crc_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char crc_start_val : 8;
    } reg;
} SOC_SCHARGER_CRC_START_VALUE_UNION;
#endif
#define SOC_SCHARGER_CRC_START_VALUE_crc_start_val_START (0)
#define SOC_SCHARGER_CRC_START_VALUE_crc_start_val_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sample_cnt_adjust : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_SAMPLE_CNT_ADJ_UNION;
#endif
#define SOC_SCHARGER_SAMPLE_CNT_ADJ_sample_cnt_adjust_START (0)
#define SOC_SCHARGER_SAMPLE_CNT_ADJ_sample_cnt_adjust_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_width_min_h : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_RX_PING_WIDTH_MIN_H_UNION;
#endif
#define SOC_SCHARGER_RX_PING_WIDTH_MIN_H_rx_ping_width_min_h_START (0)
#define SOC_SCHARGER_RX_PING_WIDTH_MIN_H_rx_ping_width_min_h_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_width_min_l : 8;
    } reg;
} SOC_SCHARGER_RX_PING_WIDTH_MIN_L_UNION;
#endif
#define SOC_SCHARGER_RX_PING_WIDTH_MIN_L_rx_ping_width_min_l_START (0)
#define SOC_SCHARGER_RX_PING_WIDTH_MIN_L_rx_ping_width_min_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_width_max_h : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_RX_PING_WIDTH_MAX_H_UNION;
#endif
#define SOC_SCHARGER_RX_PING_WIDTH_MAX_H_rx_ping_width_max_h_START (0)
#define SOC_SCHARGER_RX_PING_WIDTH_MAX_H_rx_ping_width_max_h_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_width_max_l : 8;
    } reg;
} SOC_SCHARGER_RX_PING_WIDTH_MAX_L_UNION;
#endif
#define SOC_SCHARGER_RX_PING_WIDTH_MAX_L_rx_ping_width_max_l_START (0)
#define SOC_SCHARGER_RX_PING_WIDTH_MAX_L_rx_ping_width_max_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char data_wait_time : 7;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_DATA_WAIT_TIME_UNION;
#endif
#define SOC_SCHARGER_DATA_WAIT_TIME_data_wait_time_START (0)
#define SOC_SCHARGER_DATA_WAIT_TIME_data_wait_time_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char cmd_retry_config : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_RETRY_CNT_UNION;
#endif
#define SOC_SCHARGER_RETRY_CNT_cmd_retry_config_START (0)
#define SOC_SCHARGER_RETRY_CNT_cmd_retry_config_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char fcp_ro_reserve : 8;
    } reg;
} SOC_SCHARGER_FCP_RO_RESERVE_UNION;
#endif
#define SOC_SCHARGER_FCP_RO_RESERVE_fcp_ro_reserve_START (0)
#define SOC_SCHARGER_FCP_RO_RESERVE_fcp_ro_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slv_crc_err : 1;
        unsigned char resp_ack_parity_err : 1;
        unsigned char fcp_head_early_err : 1;
        unsigned char slv_crc_parity_err : 1;
        unsigned char rdata_range_err : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_FCP_DEBUG_REG0_UNION;
#endif
#define SOC_SCHARGER_FCP_DEBUG_REG0_slv_crc_err_START (0)
#define SOC_SCHARGER_FCP_DEBUG_REG0_slv_crc_err_END (0)
#define SOC_SCHARGER_FCP_DEBUG_REG0_resp_ack_parity_err_START (1)
#define SOC_SCHARGER_FCP_DEBUG_REG0_resp_ack_parity_err_END (1)
#define SOC_SCHARGER_FCP_DEBUG_REG0_fcp_head_early_err_START (2)
#define SOC_SCHARGER_FCP_DEBUG_REG0_fcp_head_early_err_END (2)
#define SOC_SCHARGER_FCP_DEBUG_REG0_slv_crc_parity_err_START (3)
#define SOC_SCHARGER_FCP_DEBUG_REG0_slv_crc_parity_err_END (3)
#define SOC_SCHARGER_FCP_DEBUG_REG0_rdata_range_err_START (4)
#define SOC_SCHARGER_FCP_DEBUG_REG0_rdata_range_err_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char resp_ack : 8;
    } reg;
} SOC_SCHARGER_FCP_DEBUG_REG1_UNION;
#endif
#define SOC_SCHARGER_FCP_DEBUG_REG1_resp_ack_START (0)
#define SOC_SCHARGER_FCP_DEBUG_REG1_resp_ack_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char slv_crc : 8;
    } reg;
} SOC_SCHARGER_FCP_DEBUG_REG2_UNION;
#endif
#define SOC_SCHARGER_FCP_DEBUG_REG2_slv_crc_START (0)
#define SOC_SCHARGER_FCP_DEBUG_REG2_slv_crc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_cnt_l : 8;
    } reg;
} SOC_SCHARGER_FCP_DEBUG_REG3_UNION;
#endif
#define SOC_SCHARGER_FCP_DEBUG_REG3_rx_ping_cnt_l_START (0)
#define SOC_SCHARGER_FCP_DEBUG_REG3_rx_ping_cnt_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 2;
        unsigned char rx_ping_low_cnt : 5;
        unsigned char rx_ping_cnt_h : 1;
    } reg;
} SOC_SCHARGER_FCP_DEBUG_REG4_UNION;
#endif
#define SOC_SCHARGER_FCP_DEBUG_REG4_rx_ping_low_cnt_START (2)
#define SOC_SCHARGER_FCP_DEBUG_REG4_rx_ping_low_cnt_END (6)
#define SOC_SCHARGER_FCP_DEBUG_REG4_rx_ping_cnt_h_START (7)
#define SOC_SCHARGER_FCP_DEBUG_REG4_rx_ping_cnt_h_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_packet_wait_adjust : 7;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_RX_PACKET_WAIT_ADJUST_UNION;
#endif
#define SOC_SCHARGER_RX_PACKET_WAIT_ADJUST_rx_packet_wait_adjust_START (0)
#define SOC_SCHARGER_RX_PACKET_WAIT_ADJUST_rx_packet_wait_adjust_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sample_cnt_tinyjust : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_SAMPLE_CNT_TINYJUST_UNION;
#endif
#define SOC_SCHARGER_SAMPLE_CNT_TINYJUST_sample_cnt_tinyjust_START (0)
#define SOC_SCHARGER_SAMPLE_CNT_TINYJUST_sample_cnt_tinyjust_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char rx_ping_cnt_tinyjust : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_RX_PING_CNT_TINYJUST_UNION;
#endif
#define SOC_SCHARGER_RX_PING_CNT_TINYJUST_rx_ping_cnt_tinyjust_START (0)
#define SOC_SCHARGER_RX_PING_CNT_TINYJUST_rx_ping_cnt_tinyjust_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char shift_cnt_cfg_max : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_SHIFT_CNT_CFG_MAX_UNION;
#endif
#define SOC_SCHARGER_SHIFT_CNT_CFG_MAX_shift_cnt_cfg_max_START (0)
#define SOC_SCHARGER_SHIFT_CNT_CFG_MAX_shift_cnt_cfg_max_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_hkadc_fdiv : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_HKADC_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_HKADC_CFG_REG_0_da_hkadc_fdiv_START (0)
#define SOC_SCHARGER_HKADC_CFG_REG_0_da_hkadc_fdiv_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_hkadc_ibias_sel : 8;
    } reg;
} SOC_SCHARGER_HKADC_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_HKADC_CFG_REG_1_da_hkadc_ibias_sel_START (0)
#define SOC_SCHARGER_HKADC_CFG_REG_1_da_hkadc_ibias_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_hkadc_reset : 1;
        unsigned char da_hkadc_reserved : 2;
        unsigned char da_hkadc_ibsel : 3;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_HKADC_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_HKADC_CFG_REG_2_da_hkadc_reset_START (0)
#define SOC_SCHARGER_HKADC_CFG_REG_2_da_hkadc_reset_END (0)
#define SOC_SCHARGER_HKADC_CFG_REG_2_da_hkadc_reserved_START (1)
#define SOC_SCHARGER_HKADC_CFG_REG_2_da_hkadc_reserved_END (2)
#define SOC_SCHARGER_HKADC_CFG_REG_2_da_hkadc_ibsel_START (3)
#define SOC_SCHARGER_HKADC_CFG_REG_2_da_hkadc_ibsel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_offset_0p1_r : 7;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_HKADC_OFFSET_0P1_UNION;
#endif
#define SOC_SCHARGER_HKADC_OFFSET_0P1_sc_offset_0p1_r_START (0)
#define SOC_SCHARGER_HKADC_OFFSET_0P1_sc_offset_0p1_r_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_offset_2p45_r : 7;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_HKADC_OFFSET_2P45_UNION;
#endif
#define SOC_SCHARGER_HKADC_OFFSET_2P45_sc_offset_2p45_r_START (0)
#define SOC_SCHARGER_HKADC_OFFSET_2P45_sc_offset_2p45_r_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_th0_l : 8;
    } reg;
} SOC_SCHARGER_SOH_OVH_TH0_L_UNION;
#endif
#define SOC_SCHARGER_SOH_OVH_TH0_L_sc_soh_ovh_th0_l_START (0)
#define SOC_SCHARGER_SOH_OVH_TH0_L_sc_soh_ovh_th0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_th0_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_SOH_OVH_TH0_H_UNION;
#endif
#define SOC_SCHARGER_SOH_OVH_TH0_H_sc_soh_ovh_th0_h_START (0)
#define SOC_SCHARGER_SOH_OVH_TH0_H_sc_soh_ovh_th0_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tsbat_ot0_l : 8;
    } reg;
} SOC_SCHARGER_TSBAT_OVH_TH0_L_UNION;
#endif
#define SOC_SCHARGER_TSBAT_OVH_TH0_L_sc_tsbat_ot0_l_START (0)
#define SOC_SCHARGER_TSBAT_OVH_TH0_L_sc_tsbat_ot0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tsbat_ot0_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_TSBAT_OVH_TH0_H_UNION;
#endif
#define SOC_SCHARGER_TSBAT_OVH_TH0_H_sc_tsbat_ot0_h_START (0)
#define SOC_SCHARGER_TSBAT_OVH_TH0_H_sc_tsbat_ot0_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_th1_l : 8;
    } reg;
} SOC_SCHARGER_SOH_OVH_TH1_L_UNION;
#endif
#define SOC_SCHARGER_SOH_OVH_TH1_L_sc_soh_ovh_th1_l_START (0)
#define SOC_SCHARGER_SOH_OVH_TH1_L_sc_soh_ovh_th1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_th1_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_SOH_OVH_TH1_H_UNION;
#endif
#define SOC_SCHARGER_SOH_OVH_TH1_H_sc_soh_ovh_th1_h_START (0)
#define SOC_SCHARGER_SOH_OVH_TH1_H_sc_soh_ovh_th1_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tsbat_ot1_l : 8;
    } reg;
} SOC_SCHARGER_TSBAT_OVH_TH1_L_UNION;
#endif
#define SOC_SCHARGER_TSBAT_OVH_TH1_L_sc_tsbat_ot1_l_START (0)
#define SOC_SCHARGER_TSBAT_OVH_TH1_L_sc_tsbat_ot1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tsbat_ot1_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_TSBAT_OVH_TH1_H_UNION;
#endif
#define SOC_SCHARGER_TSBAT_OVH_TH1_H_sc_tsbat_ot1_h_START (0)
#define SOC_SCHARGER_TSBAT_OVH_TH1_H_sc_tsbat_ot1_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_th2_l : 8;
    } reg;
} SOC_SCHARGER_SOH_OVH_TH2_L_UNION;
#endif
#define SOC_SCHARGER_SOH_OVH_TH2_L_sc_soh_ovh_th2_l_START (0)
#define SOC_SCHARGER_SOH_OVH_TH2_L_sc_soh_ovh_th2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovh_th2_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_SOH_OVH_TH2_H_UNION;
#endif
#define SOC_SCHARGER_SOH_OVH_TH2_H_sc_soh_ovh_th2_h_START (0)
#define SOC_SCHARGER_SOH_OVH_TH2_H_sc_soh_ovh_th2_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tsbat_ot2_l : 8;
    } reg;
} SOC_SCHARGER_TSBAT_OVH_TH2_L_UNION;
#endif
#define SOC_SCHARGER_TSBAT_OVH_TH2_L_sc_tsbat_ot2_l_START (0)
#define SOC_SCHARGER_TSBAT_OVH_TH2_L_sc_tsbat_ot2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_tsbat_ot2_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_TSBAT_OVH_TH2_H_UNION;
#endif
#define SOC_SCHARGER_TSBAT_OVH_TH2_H_sc_tsbat_ot2_h_START (0)
#define SOC_SCHARGER_TSBAT_OVH_TH2_H_sc_tsbat_ot2_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovl_th_l : 8;
    } reg;
} SOC_SCHARGER_SOH_OVL_TH_L_UNION;
#endif
#define SOC_SCHARGER_SOH_OVL_TH_L_sc_soh_ovl_th_l_START (0)
#define SOC_SCHARGER_SOH_OVL_TH_L_sc_soh_ovl_th_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_soh_ovl_th_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_SOH_OVL_TH_H_UNION;
#endif
#define SOC_SCHARGER_SOH_OVL_TH_H_sc_soh_ovl_th_h_START (0)
#define SOC_SCHARGER_SOH_OVL_TH_H_sc_soh_ovl_th_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_soh_ovh : 1;
        unsigned char tb_soh_ovl : 1;
        unsigned char tb_tmp_ovh_2 : 1;
        unsigned char tb_soh_ovh_2 : 1;
        unsigned char tb_tmp_ovh_1 : 1;
        unsigned char tb_soh_ovh_1 : 1;
        unsigned char tb_tmp_ovh_0 : 1;
        unsigned char tb_soh_ovh_0 : 1;
    } reg;
} SOC_SCHARGER_SOH_OVP_REAL_UNION;
#endif
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovh_START (0)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovh_END (0)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovl_START (1)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovl_END (1)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_tmp_ovh_2_START (2)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_tmp_ovh_2_END (2)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovh_2_START (3)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovh_2_END (3)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_tmp_ovh_1_START (4)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_tmp_ovh_1_END (4)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovh_1_START (5)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovh_1_END (5)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_tmp_ovh_0_START (6)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_tmp_ovh_0_END (6)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovh_0_START (7)
#define SOC_SCHARGER_SOH_OVP_REAL_tb_soh_ovh_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_hkadc_tb_sel : 7;
        unsigned char sc_hkadc_tb_en : 1;
    } reg;
} SOC_SCHARGER_HKADC_TB_EN_SEL_UNION;
#endif
#define SOC_SCHARGER_HKADC_TB_EN_SEL_sc_hkadc_tb_sel_START (0)
#define SOC_SCHARGER_HKADC_TB_EN_SEL_sc_hkadc_tb_sel_END (6)
#define SOC_SCHARGER_HKADC_TB_EN_SEL_sc_hkadc_tb_en_START (7)
#define SOC_SCHARGER_HKADC_TB_EN_SEL_sc_hkadc_tb_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_tb_bus_0 : 8;
    } reg;
} SOC_SCHARGER_HKADC_TB_DATA0_UNION;
#endif
#define SOC_SCHARGER_HKADC_TB_DATA0_hkadc_tb_bus_0_START (0)
#define SOC_SCHARGER_HKADC_TB_DATA0_hkadc_tb_bus_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char hkadc_tb_bus_1 : 8;
    } reg;
} SOC_SCHARGER_HKADC_TB_DATA1_UNION;
#endif
#define SOC_SCHARGER_HKADC_TB_DATA1_hkadc_tb_bus_1_START (0)
#define SOC_SCHARGER_HKADC_TB_DATA1_hkadc_tb_bus_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_acr_mul_sel : 2;
        unsigned char da_acr_iref_sel : 1;
        unsigned char da_acr_cap_sel : 2;
        unsigned char da_acr_vdetop_cap : 1;
        unsigned char da_acr_testmode : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_ACR_TOP_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_mul_sel_START (0)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_mul_sel_END (1)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_iref_sel_START (2)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_iref_sel_END (2)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_cap_sel_START (3)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_cap_sel_END (4)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_vdetop_cap_START (5)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_vdetop_cap_END (5)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_testmode_START (6)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_0_da_acr_testmode_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_acr_reserve : 8;
    } reg;
} SOC_SCHARGER_ACR_TOP_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_ACR_TOP_CFG_REG_1_da_acr_reserve_START (0)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_1_da_acr_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ovp_idis_sel : 4;
        unsigned char da_pc_idis_sel : 4;
    } reg;
} SOC_SCHARGER_ACR_TOP_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_ACR_TOP_CFG_REG_2_da_ovp_idis_sel_START (0)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_2_da_ovp_idis_sel_END (3)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_2_da_pc_idis_sel_START (4)
#define SOC_SCHARGER_ACR_TOP_CFG_REG_2_da_pc_idis_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_sample_t2 : 3;
        unsigned char reserved_0 : 1;
        unsigned char sc_acr_sample_t1 : 3;
        unsigned char reserved_1 : 1;
    } reg;
} SOC_SCHARGER_ACR_SAMPLE_TIME_H_UNION;
#endif
#define SOC_SCHARGER_ACR_SAMPLE_TIME_H_sc_acr_sample_t2_START (0)
#define SOC_SCHARGER_ACR_SAMPLE_TIME_H_sc_acr_sample_t2_END (2)
#define SOC_SCHARGER_ACR_SAMPLE_TIME_H_sc_acr_sample_t1_START (4)
#define SOC_SCHARGER_ACR_SAMPLE_TIME_H_sc_acr_sample_t1_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_acr_sample_t4 : 3;
        unsigned char reserved_0 : 1;
        unsigned char sc_acr_sample_t3 : 3;
        unsigned char reserved_1 : 1;
    } reg;
} SOC_SCHARGER_ACR_SAMPLE_TIME_L_UNION;
#endif
#define SOC_SCHARGER_ACR_SAMPLE_TIME_L_sc_acr_sample_t4_START (0)
#define SOC_SCHARGER_ACR_SAMPLE_TIME_L_sc_acr_sample_t4_END (2)
#define SOC_SCHARGER_ACR_SAMPLE_TIME_L_sc_acr_sample_t3_START (4)
#define SOC_SCHARGER_ACR_SAMPLE_TIME_L_sc_acr_sample_t3_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data0_l : 8;
    } reg;
} SOC_SCHARGER_ACR_DATA0_L_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA0_L_acr_data0_l_START (0)
#define SOC_SCHARGER_ACR_DATA0_L_acr_data0_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data0_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_ACR_DATA0_H_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA0_H_acr_data0_h_START (0)
#define SOC_SCHARGER_ACR_DATA0_H_acr_data0_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data1_l : 8;
    } reg;
} SOC_SCHARGER_ACR_DATA1_L_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA1_L_acr_data1_l_START (0)
#define SOC_SCHARGER_ACR_DATA1_L_acr_data1_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data1_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_ACR_DATA1_H_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA1_H_acr_data1_h_START (0)
#define SOC_SCHARGER_ACR_DATA1_H_acr_data1_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data2_l : 8;
    } reg;
} SOC_SCHARGER_ACR_DATA2_L_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA2_L_acr_data2_l_START (0)
#define SOC_SCHARGER_ACR_DATA2_L_acr_data2_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data2_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_ACR_DATA2_H_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA2_H_acr_data2_h_START (0)
#define SOC_SCHARGER_ACR_DATA2_H_acr_data2_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data3_l : 8;
    } reg;
} SOC_SCHARGER_ACR_DATA3_L_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA3_L_acr_data3_l_START (0)
#define SOC_SCHARGER_ACR_DATA3_L_acr_data3_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data3_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_ACR_DATA3_H_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA3_H_acr_data3_h_START (0)
#define SOC_SCHARGER_ACR_DATA3_H_acr_data3_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data4_l : 8;
    } reg;
} SOC_SCHARGER_ACR_DATA4_L_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA4_L_acr_data4_l_START (0)
#define SOC_SCHARGER_ACR_DATA4_L_acr_data4_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data4_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_ACR_DATA4_H_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA4_H_acr_data4_h_START (0)
#define SOC_SCHARGER_ACR_DATA4_H_acr_data4_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data5_l : 8;
    } reg;
} SOC_SCHARGER_ACR_DATA5_L_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA5_L_acr_data5_l_START (0)
#define SOC_SCHARGER_ACR_DATA5_L_acr_data5_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data5_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_ACR_DATA5_H_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA5_H_acr_data5_h_START (0)
#define SOC_SCHARGER_ACR_DATA5_H_acr_data5_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data6_l : 8;
    } reg;
} SOC_SCHARGER_ACR_DATA6_L_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA6_L_acr_data6_l_START (0)
#define SOC_SCHARGER_ACR_DATA6_L_acr_data6_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data6_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_ACR_DATA6_H_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA6_H_acr_data6_h_START (0)
#define SOC_SCHARGER_ACR_DATA6_H_acr_data6_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data7_l : 8;
    } reg;
} SOC_SCHARGER_ACR_DATA7_L_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA7_L_acr_data7_l_START (0)
#define SOC_SCHARGER_ACR_DATA7_L_acr_data7_l_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char acr_data7_h : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_ACR_DATA7_H_UNION;
#endif
#define SOC_SCHARGER_ACR_DATA7_H_acr_data7_h_START (0)
#define SOC_SCHARGER_ACR_DATA7_H_acr_data7_h_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask : 8;
    } reg;
} SOC_SCHARGER_IRQ_MASK_UNION;
#endif
#define SOC_SCHARGER_IRQ_MASK_sc_irq_mask_START (0)
#define SOC_SCHARGER_IRQ_MASK_sc_irq_mask_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_0 : 8;
    } reg;
} SOC_SCHARGER_IRQ_MASK_0_UNION;
#endif
#define SOC_SCHARGER_IRQ_MASK_0_sc_irq_mask_0_START (0)
#define SOC_SCHARGER_IRQ_MASK_0_sc_irq_mask_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_1 : 8;
    } reg;
} SOC_SCHARGER_IRQ_MASK_1_UNION;
#endif
#define SOC_SCHARGER_IRQ_MASK_1_sc_irq_mask_1_START (0)
#define SOC_SCHARGER_IRQ_MASK_1_sc_irq_mask_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_2 : 8;
    } reg;
} SOC_SCHARGER_IRQ_MASK_2_UNION;
#endif
#define SOC_SCHARGER_IRQ_MASK_2_sc_irq_mask_2_START (0)
#define SOC_SCHARGER_IRQ_MASK_2_sc_irq_mask_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_3 : 8;
    } reg;
} SOC_SCHARGER_IRQ_MASK_3_UNION;
#endif
#define SOC_SCHARGER_IRQ_MASK_3_sc_irq_mask_3_START (0)
#define SOC_SCHARGER_IRQ_MASK_3_sc_irq_mask_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_4 : 8;
    } reg;
} SOC_SCHARGER_IRQ_MASK_4_UNION;
#endif
#define SOC_SCHARGER_IRQ_MASK_4_sc_irq_mask_4_START (0)
#define SOC_SCHARGER_IRQ_MASK_4_sc_irq_mask_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_5 : 8;
    } reg;
} SOC_SCHARGER_IRQ_MASK_5_UNION;
#endif
#define SOC_SCHARGER_IRQ_MASK_5_sc_irq_mask_5_START (0)
#define SOC_SCHARGER_IRQ_MASK_5_sc_irq_mask_5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_6 : 8;
    } reg;
} SOC_SCHARGER_IRQ_MASK_6_UNION;
#endif
#define SOC_SCHARGER_IRQ_MASK_6_sc_irq_mask_6_START (0)
#define SOC_SCHARGER_IRQ_MASK_6_sc_irq_mask_6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_irq_mask_7 : 8;
    } reg;
} SOC_SCHARGER_IRQ_MASK_7_UNION;
#endif
#define SOC_SCHARGER_IRQ_MASK_7_sc_irq_mask_7_START (0)
#define SOC_SCHARGER_IRQ_MASK_7_sc_irq_mask_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_irq_status_0 : 8;
    } reg;
} SOC_SCHARGER_IRQ_STATUS_0_UNION;
#endif
#define SOC_SCHARGER_IRQ_STATUS_0_tb_irq_status_0_START (0)
#define SOC_SCHARGER_IRQ_STATUS_0_tb_irq_status_0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_irq_status_1 : 8;
    } reg;
} SOC_SCHARGER_IRQ_STATUS_1_UNION;
#endif
#define SOC_SCHARGER_IRQ_STATUS_1_tb_irq_status_1_START (0)
#define SOC_SCHARGER_IRQ_STATUS_1_tb_irq_status_1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_irq_status_2 : 8;
    } reg;
} SOC_SCHARGER_IRQ_STATUS_2_UNION;
#endif
#define SOC_SCHARGER_IRQ_STATUS_2_tb_irq_status_2_START (0)
#define SOC_SCHARGER_IRQ_STATUS_2_tb_irq_status_2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_irq_status_3 : 8;
    } reg;
} SOC_SCHARGER_IRQ_STATUS_3_UNION;
#endif
#define SOC_SCHARGER_IRQ_STATUS_3_tb_irq_status_3_START (0)
#define SOC_SCHARGER_IRQ_STATUS_3_tb_irq_status_3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_irq_status_4 : 8;
    } reg;
} SOC_SCHARGER_IRQ_STATUS_4_UNION;
#endif
#define SOC_SCHARGER_IRQ_STATUS_4_tb_irq_status_4_START (0)
#define SOC_SCHARGER_IRQ_STATUS_4_tb_irq_status_4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_irq_status_5 : 8;
    } reg;
} SOC_SCHARGER_IRQ_STATUS_5_UNION;
#endif
#define SOC_SCHARGER_IRQ_STATUS_5_tb_irq_status_5_START (0)
#define SOC_SCHARGER_IRQ_STATUS_5_tb_irq_status_5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_irq_status_6 : 8;
    } reg;
} SOC_SCHARGER_IRQ_STATUS_6_UNION;
#endif
#define SOC_SCHARGER_IRQ_STATUS_6_tb_irq_status_6_START (0)
#define SOC_SCHARGER_IRQ_STATUS_6_tb_irq_status_6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_irq_status_7 : 8;
    } reg;
} SOC_SCHARGER_IRQ_STATUS_7_UNION;
#endif
#define SOC_SCHARGER_IRQ_STATUS_7_tb_irq_status_7_START (0)
#define SOC_SCHARGER_IRQ_STATUS_7_tb_irq_status_7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_irq_status_8 : 8;
    } reg;
} SOC_SCHARGER_IRQ_STATUS_8_UNION;
#endif
#define SOC_SCHARGER_IRQ_STATUS_8_tb_irq_status_8_START (0)
#define SOC_SCHARGER_IRQ_STATUS_8_tb_irq_status_8_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_sel : 2;
        unsigned char reserved_0 : 2;
        unsigned char sc_efuse_pdob_sel : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_EFUSE_SEL_UNION;
#endif
#define SOC_SCHARGER_EFUSE_SEL_sc_efuse_sel_START (0)
#define SOC_SCHARGER_EFUSE_SEL_sc_efuse_sel_END (1)
#define SOC_SCHARGER_EFUSE_SEL_sc_efuse_pdob_sel_START (4)
#define SOC_SCHARGER_EFUSE_SEL_sc_efuse_pdob_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_nr_cfg : 1;
        unsigned char sc_efuse_pgenb_cfg : 1;
        unsigned char sc_efuse_strobe_cfg : 1;
        unsigned char sc_efuse_rd_ctrl : 1;
        unsigned char sc_efuse_prog_int : 1;
        unsigned char sc_efuse_prog_sel : 1;
        unsigned char sc_efuse_inctrl_sel : 1;
        unsigned char sc_efuse_rd_mode_sel : 1;
    } reg;
} SOC_SCHARGER_EFUSE_CFG_0_UNION;
#endif
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_nr_cfg_START (0)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_nr_cfg_END (0)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_pgenb_cfg_START (1)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_pgenb_cfg_END (1)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_strobe_cfg_START (2)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_strobe_cfg_END (2)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_rd_ctrl_START (3)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_rd_ctrl_END (3)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_prog_int_START (4)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_prog_int_END (4)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_prog_sel_START (5)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_prog_sel_END (5)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_inctrl_sel_START (6)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_inctrl_sel_END (6)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_rd_mode_sel_START (7)
#define SOC_SCHARGER_EFUSE_CFG_0_sc_efuse_rd_mode_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_we0 : 8;
    } reg;
} SOC_SCHARGER_EFUSE_WE_0_UNION;
#endif
#define SOC_SCHARGER_EFUSE_WE_0_sc_efuse_we0_START (0)
#define SOC_SCHARGER_EFUSE_WE_0_sc_efuse_we0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_we1 : 8;
    } reg;
} SOC_SCHARGER_EFUSE_WE_1_UNION;
#endif
#define SOC_SCHARGER_EFUSE_WE_1_sc_efuse_we1_START (0)
#define SOC_SCHARGER_EFUSE_WE_1_sc_efuse_we1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_we2 : 8;
    } reg;
} SOC_SCHARGER_EFUSE_WE_2_UNION;
#endif
#define SOC_SCHARGER_EFUSE_WE_2_sc_efuse_we2_START (0)
#define SOC_SCHARGER_EFUSE_WE_2_sc_efuse_we2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_we3 : 8;
    } reg;
} SOC_SCHARGER_EFUSE_WE_3_UNION;
#endif
#define SOC_SCHARGER_EFUSE_WE_3_sc_efuse_we3_START (0)
#define SOC_SCHARGER_EFUSE_WE_3_sc_efuse_we3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_we4 : 8;
    } reg;
} SOC_SCHARGER_EFUSE_WE_4_UNION;
#endif
#define SOC_SCHARGER_EFUSE_WE_4_sc_efuse_we4_START (0)
#define SOC_SCHARGER_EFUSE_WE_4_sc_efuse_we4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_we5 : 8;
    } reg;
} SOC_SCHARGER_EFUSE_WE_5_UNION;
#endif
#define SOC_SCHARGER_EFUSE_WE_5_sc_efuse_we5_START (0)
#define SOC_SCHARGER_EFUSE_WE_5_sc_efuse_we5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_we6 : 8;
    } reg;
} SOC_SCHARGER_EFUSE_WE_6_UNION;
#endif
#define SOC_SCHARGER_EFUSE_WE_6_sc_efuse_we6_START (0)
#define SOC_SCHARGER_EFUSE_WE_6_sc_efuse_we6_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_we7 : 8;
    } reg;
} SOC_SCHARGER_EFUSE_WE_7_UNION;
#endif
#define SOC_SCHARGER_EFUSE_WE_7_sc_efuse_we7_START (0)
#define SOC_SCHARGER_EFUSE_WE_7_sc_efuse_we7_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_pdob_pre_addr : 3;
        unsigned char reserved_0 : 1;
        unsigned char sc_efuse_pdob_pre_we : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_EFUSE_PDOB_PRE_ADDR_WE_UNION;
#endif
#define SOC_SCHARGER_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_addr_START (0)
#define SOC_SCHARGER_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_addr_END (2)
#define SOC_SCHARGER_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_we_START (4)
#define SOC_SCHARGER_EFUSE_PDOB_PRE_ADDR_WE_sc_efuse_pdob_pre_we_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse_pdob_pre_wdata : 8;
    } reg;
} SOC_SCHARGER_EFUSE_PDOB_PRE_WDATA_UNION;
#endif
#define SOC_SCHARGER_EFUSE_PDOB_PRE_WDATA_sc_efuse_pdob_pre_wdata_START (0)
#define SOC_SCHARGER_EFUSE_PDOB_PRE_WDATA_sc_efuse_pdob_pre_wdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_efuse1_state : 4;
        unsigned char tb_efuse1_por_int_ro : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_EFUSE1_TESTBUS_0_UNION;
#endif
#define SOC_SCHARGER_EFUSE1_TESTBUS_0_tb_efuse1_state_START (0)
#define SOC_SCHARGER_EFUSE1_TESTBUS_0_tb_efuse1_state_END (3)
#define SOC_SCHARGER_EFUSE1_TESTBUS_0_tb_efuse1_por_int_ro_START (4)
#define SOC_SCHARGER_EFUSE1_TESTBUS_0_tb_efuse1_por_int_ro_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_efuse2_state : 4;
        unsigned char tb_efuse2_por_int_ro : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_EFUSE2_TESTBUS_0_UNION;
#endif
#define SOC_SCHARGER_EFUSE2_TESTBUS_0_tb_efuse2_state_START (0)
#define SOC_SCHARGER_EFUSE2_TESTBUS_0_tb_efuse2_state_END (3)
#define SOC_SCHARGER_EFUSE2_TESTBUS_0_tb_efuse2_por_int_ro_START (4)
#define SOC_SCHARGER_EFUSE2_TESTBUS_0_tb_efuse2_por_int_ro_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_efuse3_state : 4;
        unsigned char tb_efuse3_por_int_ro : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_EFUSE3_TESTBUS_0_UNION;
#endif
#define SOC_SCHARGER_EFUSE3_TESTBUS_0_tb_efuse3_state_START (0)
#define SOC_SCHARGER_EFUSE3_TESTBUS_0_tb_efuse3_state_END (3)
#define SOC_SCHARGER_EFUSE3_TESTBUS_0_tb_efuse3_por_int_ro_START (4)
#define SOC_SCHARGER_EFUSE3_TESTBUS_0_tb_efuse3_por_int_ro_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse2_testbus_sel : 4;
        unsigned char sc_efuse3_testbus_sel : 4;
    } reg;
} SOC_SCHARGER_EFUSE_TESTBUS_SEL_UNION;
#endif
#define SOC_SCHARGER_EFUSE_TESTBUS_SEL_sc_efuse2_testbus_sel_START (0)
#define SOC_SCHARGER_EFUSE_TESTBUS_SEL_sc_efuse2_testbus_sel_END (3)
#define SOC_SCHARGER_EFUSE_TESTBUS_SEL_sc_efuse3_testbus_sel_START (4)
#define SOC_SCHARGER_EFUSE_TESTBUS_SEL_sc_efuse3_testbus_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_efuse1_state_reset : 1;
        unsigned char sc_efuse2_state_reset : 1;
        unsigned char sc_efuse3_state_reset : 1;
        unsigned char sc_efuse_testbus_en : 1;
        unsigned char sc_efuse1_testbus_sel : 4;
    } reg;
} SOC_SCHARGER_EFUSE_TESTBUS_CFG_UNION;
#endif
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse1_state_reset_START (0)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse1_state_reset_END (0)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse2_state_reset_START (1)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse2_state_reset_END (1)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse3_state_reset_START (2)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse3_state_reset_END (2)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse_testbus_en_START (3)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse_testbus_en_END (3)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse1_testbus_sel_START (4)
#define SOC_SCHARGER_EFUSE_TESTBUS_CFG_sc_efuse1_testbus_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efuse1_testbus_rdata : 8;
    } reg;
} SOC_SCHARGER_EFUSE1_TESTBUS_RDATA_UNION;
#endif
#define SOC_SCHARGER_EFUSE1_TESTBUS_RDATA_efuse1_testbus_rdata_START (0)
#define SOC_SCHARGER_EFUSE1_TESTBUS_RDATA_efuse1_testbus_rdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efuse2_testbus_rdata : 8;
    } reg;
} SOC_SCHARGER_EFUSE2_TESTBUS_RDATA_UNION;
#endif
#define SOC_SCHARGER_EFUSE2_TESTBUS_RDATA_efuse2_testbus_rdata_START (0)
#define SOC_SCHARGER_EFUSE2_TESTBUS_RDATA_efuse2_testbus_rdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char efuse3_testbus_rdata : 8;
    } reg;
} SOC_SCHARGER_EFUSE3_TESTBUS_RDATA_UNION;
#endif
#define SOC_SCHARGER_EFUSE3_TESTBUS_RDATA_efuse3_testbus_rdata_START (0)
#define SOC_SCHARGER_EFUSE3_TESTBUS_RDATA_efuse3_testbus_rdata_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_glb_tb_sel : 4;
        unsigned char reserved_0 : 1;
        unsigned char sc_glb_tb_en : 1;
        unsigned char reserved_1 : 2;
    } reg;
} SOC_SCHARGER_GLB_TESTBUS_CFG_UNION;
#endif
#define SOC_SCHARGER_GLB_TESTBUS_CFG_sc_glb_tb_sel_START (0)
#define SOC_SCHARGER_GLB_TESTBUS_CFG_sc_glb_tb_sel_END (3)
#define SOC_SCHARGER_GLB_TESTBUS_CFG_sc_glb_tb_en_START (5)
#define SOC_SCHARGER_GLB_TESTBUS_CFG_sc_glb_tb_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char tb_glb_data : 8;
    } reg;
} SOC_SCHARGER_GLB_TEST_DATA_UNION;
#endif
#define SOC_SCHARGER_GLB_TEST_DATA_tb_glb_data_START (0)
#define SOC_SCHARGER_GLB_TEST_DATA_tb_glb_data_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_cap2_sel : 2;
        unsigned char da_chg_cap1_sel : 2;
        unsigned char da_chg_bat_open : 1;
        unsigned char da_chg_ate_mode : 1;
        unsigned char da_chg_acl_rpt_en : 1;
        unsigned char da_chg_2x_ibias_en : 1;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_cap2_sel_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_cap2_sel_END (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_cap1_sel_START (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_cap1_sel_END (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_bat_open_START (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_bat_open_END (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_ate_mode_START (5)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_ate_mode_END (5)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_acl_rpt_en_START (6)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_acl_rpt_en_END (6)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_2x_ibias_en_START (7)
#define SOC_SCHARGER_CHARGER_CFG_REG_0_da_chg_2x_ibias_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_eoc_del2m_en : 1;
        unsigned char da_chg_en_term : 1;
        unsigned char da_chg_cp_src_sel : 1;
        unsigned char da_chg_clk_div2_shd : 1;
        unsigned char da_chg_cap4_sel : 2;
        unsigned char da_chg_cap3_sel : 2;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_eoc_del2m_en_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_eoc_del2m_en_END (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_en_term_START (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_en_term_END (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_cp_src_sel_START (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_cp_src_sel_END (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_clk_div2_shd_START (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_clk_div2_shd_END (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_cap4_sel_START (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_cap4_sel_END (5)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_cap3_sel_START (6)
#define SOC_SCHARGER_CHARGER_CFG_REG_1_da_chg_cap3_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_fast_ichg : 5;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_2_da_chg_fast_ichg_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_2_da_chg_fast_ichg_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_fastchg_timer : 2;
        unsigned char da_chg_fast_vchg : 5;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_3_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_3_da_chg_fastchg_timer_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_3_da_chg_fastchg_timer_END (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_3_da_chg_fast_vchg_START (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_3_da_chg_fast_vchg_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_ocp_adj : 2;
        unsigned char da_chg_md_sel : 1;
        unsigned char da_chg_iref_clamp : 2;
        unsigned char da_chg_ir_set : 3;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_4_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_4_da_chg_ocp_adj_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_4_da_chg_ocp_adj_END (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_4_da_chg_md_sel_START (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_4_da_chg_md_sel_END (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_4_da_chg_iref_clamp_START (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_4_da_chg_iref_clamp_END (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_4_da_chg_ir_set_START (5)
#define SOC_SCHARGER_CHARGER_CFG_REG_4_da_chg_ir_set_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_pre_vchg : 2;
        unsigned char da_chg_pre_ichg : 2;
        unsigned char da_chg_ocp_test : 1;
        unsigned char da_chg_ocp_shd : 1;
        unsigned char da_chg_ocp_delay_shd : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_5_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_pre_vchg_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_pre_vchg_END (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_pre_ichg_START (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_pre_ichg_END (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_ocp_test_START (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_ocp_test_END (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_ocp_shd_START (5)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_ocp_shd_END (5)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_ocp_delay_shd_START (6)
#define SOC_SCHARGER_CHARGER_CFG_REG_5_da_chg_ocp_delay_shd_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_rechg_time : 2;
        unsigned char da_chg_q4_3x_shd : 1;
        unsigned char da_chg_prechg_timer : 2;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_6_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_6_da_chg_rechg_time_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_6_da_chg_rechg_time_END (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_6_da_chg_q4_3x_shd_START (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_6_da_chg_q4_3x_shd_END (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_6_da_chg_prechg_timer_START (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_6_da_chg_prechg_timer_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_scp_en : 1;
        unsigned char da_chg_rpt_en : 1;
        unsigned char da_chg_rev_mode : 2;
        unsigned char da_chg_resvo : 4;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_7_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_7_da_chg_scp_en_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_7_da_chg_scp_en_END (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_7_da_chg_rpt_en_START (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_7_da_chg_rpt_en_END (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_7_da_chg_rev_mode_START (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_7_da_chg_rev_mode_END (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_7_da_chg_resvo_START (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_7_da_chg_resvo_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_vclamp_set : 3;
        unsigned char da_chg_vbat_plus6mv : 1;
        unsigned char da_chg_timer_en : 1;
        unsigned char da_chg_test_md : 1;
        unsigned char da_chg_term_ichg : 2;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_8_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_vclamp_set_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_vclamp_set_END (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_vbat_plus6mv_START (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_vbat_plus6mv_END (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_timer_en_START (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_timer_en_END (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_test_md_START (5)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_test_md_END (5)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_term_ichg_START (6)
#define SOC_SCHARGER_CHARGER_CFG_REG_8_da_chg_term_ichg_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_chg_vres2_sel : 2;
        unsigned char da_chg_vres1_sel : 2;
        unsigned char da_chg_vrechg_hys : 2;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_CHARGER_CFG_REG_9_UNION;
#endif
#define SOC_SCHARGER_CHARGER_CFG_REG_9_da_chg_vres2_sel_START (0)
#define SOC_SCHARGER_CHARGER_CFG_REG_9_da_chg_vres2_sel_END (1)
#define SOC_SCHARGER_CHARGER_CFG_REG_9_da_chg_vres1_sel_START (2)
#define SOC_SCHARGER_CHARGER_CFG_REG_9_da_chg_vres1_sel_END (3)
#define SOC_SCHARGER_CHARGER_CFG_REG_9_da_chg_vrechg_hys_START (4)
#define SOC_SCHARGER_CHARGER_CFG_REG_9_da_chg_vrechg_hys_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_chg_sft_pd : 1;
        unsigned char ad_chg_rev_ok : 1;
        unsigned char ad_chg_rev_en : 1;
        unsigned char ad_chg_pu_btft : 1;
        unsigned char ad_chg_pre_state : 1;
        unsigned char ad_chg_fwd_en : 1;
        unsigned char ad_chg_dpm_state : 1;
        unsigned char ad_chg_acl_state : 1;
    } reg;
} SOC_SCHARGER_CHARGER_RO_REG_10_UNION;
#endif
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_sft_pd_START (0)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_sft_pd_END (0)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_rev_ok_START (1)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_rev_ok_END (1)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_rev_en_START (2)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_rev_en_END (2)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_pu_btft_START (3)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_pu_btft_END (3)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_pre_state_START (4)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_pre_state_END (4)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_fwd_en_START (5)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_fwd_en_END (5)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_dpm_state_START (6)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_dpm_state_END (6)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_acl_state_START (7)
#define SOC_SCHARGER_CHARGER_RO_REG_10_ad_chg_acl_state_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_chg_tri_state : 1;
        unsigned char ad_chg_therm_state : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_CHARGER_RO_REG_11_UNION;
#endif
#define SOC_SCHARGER_CHARGER_RO_REG_11_ad_chg_tri_state_START (0)
#define SOC_SCHARGER_CHARGER_RO_REG_11_ad_chg_tri_state_END (0)
#define SOC_SCHARGER_CHARGER_RO_REG_11_ad_chg_therm_state_START (1)
#define SOC_SCHARGER_CHARGER_RO_REG_11_ad_chg_therm_state_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vusb_ovp_resv : 4;
        unsigned char da_vusb_ovp_lpf_sel : 1;
        unsigned char da_vusb_force_ovpok : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_USB_OVP_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_USB_OVP_CFG_REG_0_da_vusb_ovp_resv_START (0)
#define SOC_SCHARGER_USB_OVP_CFG_REG_0_da_vusb_ovp_resv_END (3)
#define SOC_SCHARGER_USB_OVP_CFG_REG_0_da_vusb_ovp_lpf_sel_START (4)
#define SOC_SCHARGER_USB_OVP_CFG_REG_0_da_vusb_ovp_lpf_sel_END (4)
#define SOC_SCHARGER_USB_OVP_CFG_REG_0_da_vusb_force_ovpok_START (5)
#define SOC_SCHARGER_USB_OVP_CFG_REG_0_da_vusb_force_ovpok_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vusb_uvlo_sel : 1;
        unsigned char da_vusb_ovp_shield : 1;
        unsigned char da_vusb_ovp_set : 4;
        unsigned char da_vusb_sc_auto_ovp_enb : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_USB_OVP_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_USB_OVP_CFG_REG_1_da_vusb_uvlo_sel_START (0)
#define SOC_SCHARGER_USB_OVP_CFG_REG_1_da_vusb_uvlo_sel_END (0)
#define SOC_SCHARGER_USB_OVP_CFG_REG_1_da_vusb_ovp_shield_START (1)
#define SOC_SCHARGER_USB_OVP_CFG_REG_1_da_vusb_ovp_shield_END (1)
#define SOC_SCHARGER_USB_OVP_CFG_REG_1_da_vusb_ovp_set_START (2)
#define SOC_SCHARGER_USB_OVP_CFG_REG_1_da_vusb_ovp_set_END (5)
#define SOC_SCHARGER_USB_OVP_CFG_REG_1_da_vusb_sc_auto_ovp_enb_START (6)
#define SOC_SCHARGER_USB_OVP_CFG_REG_1_da_vusb_sc_auto_ovp_enb_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_typ_cfg0 : 8;
    } reg;
} SOC_SCHARGER_TCPC_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_TCPC_CFG_REG_1_da_typ_cfg0_START (0)
#define SOC_SCHARGER_TCPC_CFG_REG_1_da_typ_cfg0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_typ_cfg1 : 8;
    } reg;
} SOC_SCHARGER_TCPC_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_TCPC_CFG_REG_2_da_typ_cfg1_START (0)
#define SOC_SCHARGER_TCPC_CFG_REG_2_da_typ_cfg1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_typ_cfg2 : 8;
    } reg;
} SOC_SCHARGER_TCPC_CFG_REG_3_UNION;
#endif
#define SOC_SCHARGER_TCPC_CFG_REG_3_da_typ_cfg2_START (0)
#define SOC_SCHARGER_TCPC_CFG_REG_3_da_typ_cfg2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_cc_resvi : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_TCPC_RO_REG_5_UNION;
#endif
#define SOC_SCHARGER_TCPC_RO_REG_5_ad_cc_resvi_START (0)
#define SOC_SCHARGER_TCPC_RO_REG_5_ad_cc_resvi_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_regn_ocp_shield : 1;
        unsigned char da_ovp_wl_en : 1;
        unsigned char da_otg_wl_en : 1;
        unsigned char da_otg_delay_sel : 2;
        unsigned char da_ldo33_en : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_regn_ocp_shield_START (0)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_regn_ocp_shield_END (0)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_ovp_wl_en_START (1)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_ovp_wl_en_END (1)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_otg_wl_en_START (2)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_otg_wl_en_END (2)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_otg_delay_sel_START (3)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_otg_delay_sel_END (4)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_ldo33_en_START (5)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_0_da_ldo33_en_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sys_resvo1 : 8;
    } reg;
} SOC_SCHARGER_SCHG_LOGIC_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_1_da_sys_resvo1_START (0)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_1_da_sys_resvo1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sys_resvo2 : 8;
    } reg;
} SOC_SCHARGER_SCHG_LOGIC_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_2_da_sys_resvo2_START (0)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_2_da_sys_resvo2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_wl_en : 1;
        unsigned char da_timer_test : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_SCHG_LOGIC_CFG_REG_3_UNION;
#endif
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_3_da_wl_en_START (0)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_3_da_wl_en_END (0)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_3_da_timer_test_START (1)
#define SOC_SCHARGER_SCHG_LOGIC_CFG_REG_3_da_timer_test_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_sys_resvi1 : 8;
    } reg;
} SOC_SCHARGER_SCHG_LOGIC_RO_REG_4_UNION;
#endif
#define SOC_SCHARGER_SCHG_LOGIC_RO_REG_4_ad_sys_resvi1_START (0)
#define SOC_SCHARGER_SCHG_LOGIC_RO_REG_4_ad_sys_resvi1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_batfet_ctrl : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_CHARGER_BATFET_CTRL_UNION;
#endif
#define SOC_SCHARGER_CHARGER_BATFET_CTRL_sc_batfet_ctrl_START (0)
#define SOC_SCHARGER_CHARGER_BATFET_CTRL_sc_batfet_ctrl_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char vbat_lv_cfg : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_VBAT_LV_REG_UNION;
#endif
#define SOC_SCHARGER_VBAT_LV_REG_vbat_lv_cfg_START (0)
#define SOC_SCHARGER_VBAT_LV_REG_vbat_lv_cfg_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_pmos_en : 1;
        unsigned char reserved_0 : 3;
        unsigned char sc_chg_lp : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_VDM_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_VDM_CFG_REG_0_sc_pmos_en_START (0)
#define SOC_SCHARGER_VDM_CFG_REG_0_sc_pmos_en_END (0)
#define SOC_SCHARGER_VDM_CFG_REG_0_sc_chg_lp_START (4)
#define SOC_SCHARGER_VDM_CFG_REG_0_sc_chg_lp_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_uart_en : 1;
        unsigned char reserved_0 : 3;
        unsigned char sc_boot_en : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_VDM_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_VDM_CFG_REG_1_sc_uart_en_START (0)
#define SOC_SCHARGER_VDM_CFG_REG_1_sc_uart_en_END (0)
#define SOC_SCHARGER_VDM_CFG_REG_1_sc_boot_en_START (4)
#define SOC_SCHARGER_VDM_CFG_REG_1_sc_boot_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char sc_regn_en : 1;
        unsigned char reserved_0 : 3;
        unsigned char sc_vusb_off_mask : 1;
        unsigned char reserved_1 : 3;
    } reg;
} SOC_SCHARGER_VDM_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_VDM_CFG_REG_2_sc_regn_en_START (0)
#define SOC_SCHARGER_VDM_CFG_REG_2_sc_regn_en_END (0)
#define SOC_SCHARGER_VDM_CFG_REG_2_sc_vusb_off_mask_START (4)
#define SOC_SCHARGER_VDM_CFG_REG_2_sc_vusb_off_mask_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char reserved : 1;
        unsigned char da_dp_res_det_iqsel : 2;
        unsigned char da_dp_res_det_en : 1;
        unsigned char da_bat_gd_shield : 1;
        unsigned char da_bat_gd_sel : 1;
        unsigned char da_app_det_en : 1;
        unsigned char da_app_det_chsel : 1;
    } reg;
} SOC_SCHARGER_DET_TOP_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_dp_res_det_iqsel_START (1)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_dp_res_det_iqsel_END (2)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_dp_res_det_en_START (3)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_dp_res_det_en_END (3)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_bat_gd_shield_START (4)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_bat_gd_shield_END (4)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_bat_gd_sel_START (5)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_bat_gd_sel_END (5)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_app_det_en_START (6)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_app_det_en_END (6)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_app_det_chsel_START (7)
#define SOC_SCHARGER_DET_TOP_CFG_REG_0_da_app_det_chsel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vbat_dc_ovp_t : 1;
        unsigned char da_slp_vset : 1;
        unsigned char da_sleep_block : 1;
        unsigned char da_io_level : 1;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_DET_TOP_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_DET_TOP_CFG_REG_1_da_vbat_dc_ovp_t_START (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_1_da_vbat_dc_ovp_t_END (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_1_da_slp_vset_START (1)
#define SOC_SCHARGER_DET_TOP_CFG_REG_1_da_slp_vset_END (1)
#define SOC_SCHARGER_DET_TOP_CFG_REG_1_da_sleep_block_START (2)
#define SOC_SCHARGER_DET_TOP_CFG_REG_1_da_sleep_block_END (2)
#define SOC_SCHARGER_DET_TOP_CFG_REG_1_da_io_level_START (3)
#define SOC_SCHARGER_DET_TOP_CFG_REG_1_da_io_level_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vbat_lv_tset : 1;
        unsigned char da_vbat_lv_t : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_DET_TOP_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_DET_TOP_CFG_REG_2_da_vbat_lv_tset_START (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_2_da_vbat_lv_tset_END (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_2_da_vbat_lv_t_START (1)
#define SOC_SCHARGER_DET_TOP_CFG_REG_2_da_vbat_lv_t_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vbat_sc_uvp_t : 1;
        unsigned char da_vbat_ov_dc : 7;
    } reg;
} SOC_SCHARGER_DET_TOP_CFG_REG_3_UNION;
#endif
#define SOC_SCHARGER_DET_TOP_CFG_REG_3_da_vbat_sc_uvp_t_START (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_3_da_vbat_sc_uvp_t_END (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_3_da_vbat_ov_dc_START (1)
#define SOC_SCHARGER_DET_TOP_CFG_REG_3_da_vbat_ov_dc_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vbus_lvc_ov_t : 1;
        unsigned char da_vbus_hi_ovp_sel : 1;
        unsigned char da_vbus_bkvset : 2;
        unsigned char da_vbat_uv_dc : 3;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_DET_TOP_CFG_REG_4_UNION;
#endif
#define SOC_SCHARGER_DET_TOP_CFG_REG_4_da_vbus_lvc_ov_t_START (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_4_da_vbus_lvc_ov_t_END (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_4_da_vbus_hi_ovp_sel_START (1)
#define SOC_SCHARGER_DET_TOP_CFG_REG_4_da_vbus_hi_ovp_sel_END (1)
#define SOC_SCHARGER_DET_TOP_CFG_REG_4_da_vbus_bkvset_START (2)
#define SOC_SCHARGER_DET_TOP_CFG_REG_4_da_vbus_bkvset_END (3)
#define SOC_SCHARGER_DET_TOP_CFG_REG_4_da_vbat_uv_dc_START (4)
#define SOC_SCHARGER_DET_TOP_CFG_REG_4_da_vbat_uv_dc_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vbus_sc_ov_t : 1;
        unsigned char da_vbus_ov_dc : 5;
        unsigned char da_vbus_ov_bk : 2;
    } reg;
} SOC_SCHARGER_DET_TOP_CFG_REG_5_UNION;
#endif
#define SOC_SCHARGER_DET_TOP_CFG_REG_5_da_vbus_sc_ov_t_START (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_5_da_vbus_sc_ov_t_END (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_5_da_vbus_ov_dc_START (1)
#define SOC_SCHARGER_DET_TOP_CFG_REG_5_da_vbus_ov_dc_END (5)
#define SOC_SCHARGER_DET_TOP_CFG_REG_5_da_vbus_ov_bk_START (6)
#define SOC_SCHARGER_DET_TOP_CFG_REG_5_da_vbus_ov_bk_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vbus_uv_sc : 3;
        unsigned char da_vbus_uv_bk : 2;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_DET_TOP_CFG_REG_6_UNION;
#endif
#define SOC_SCHARGER_DET_TOP_CFG_REG_6_da_vbus_uv_sc_START (0)
#define SOC_SCHARGER_DET_TOP_CFG_REG_6_da_vbus_uv_sc_END (2)
#define SOC_SCHARGER_DET_TOP_CFG_REG_6_da_vbus_uv_bk_START (3)
#define SOC_SCHARGER_DET_TOP_CFG_REG_6_da_vbus_uv_bk_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ldo33_tsmod : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_PSEL_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_PSEL_CFG_REG_0_da_ldo33_tsmod_START (0)
#define SOC_SCHARGER_PSEL_CFG_REG_0_da_ldo33_tsmod_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_ldo33_ok : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_PSEL_RO_REG_1_UNION;
#endif
#define SOC_SCHARGER_PSEL_RO_REG_1_ad_ldo33_ok_START (0)
#define SOC_SCHARGER_PSEL_RO_REG_1_ad_ldo33_ok_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ref_ichg_sel : 2;
        unsigned char da_ref_clk_chop_sel : 2;
        unsigned char da_ref_chop_en : 1;
        unsigned char da_otmp_adj : 2;
        unsigned char da_ibias_switch_en : 1;
    } reg;
} SOC_SCHARGER_REF_TOP_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_ref_ichg_sel_START (0)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_ref_ichg_sel_END (1)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_ref_clk_chop_sel_START (2)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_ref_clk_chop_sel_END (3)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_ref_chop_en_START (4)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_ref_chop_en_END (4)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_otmp_adj_START (5)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_otmp_adj_END (6)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_ibias_switch_en_START (7)
#define SOC_SCHARGER_REF_TOP_CFG_REG_0_da_ibias_switch_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ref_iref_sel : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_REF_TOP_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_REF_TOP_CFG_REG_1_da_ref_iref_sel_START (0)
#define SOC_SCHARGER_REF_TOP_CFG_REG_1_da_ref_iref_sel_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ref_reserve : 8;
    } reg;
} SOC_SCHARGER_REF_TOP_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_REF_TOP_CFG_REG_2_da_ref_reserve_START (0)
#define SOC_SCHARGER_REF_TOP_CFG_REG_2_da_ref_reserve_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_tdie_treg_set : 2;
        unsigned char da_tdie_alm_set : 2;
        unsigned char da_ref_vrc_en : 1;
        unsigned char da_ref_testib_en : 1;
        unsigned char da_ref_testbg_en : 1;
        unsigned char da_ref_selgate_en : 1;
    } reg;
} SOC_SCHARGER_REF_TOP_CFG_REG_3_UNION;
#endif
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_tdie_treg_set_START (0)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_tdie_treg_set_END (1)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_tdie_alm_set_START (2)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_tdie_alm_set_END (3)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_ref_vrc_en_START (4)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_ref_vrc_en_END (4)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_ref_testib_en_START (5)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_ref_testib_en_END (5)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_ref_testbg_en_START (6)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_ref_testbg_en_END (6)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_ref_selgate_en_START (7)
#define SOC_SCHARGER_REF_TOP_CFG_REG_3_da_ref_selgate_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dc_ibat_regl_sns_cap : 1;
        unsigned char da_dc_ibat_regl_res : 2;
        unsigned char da_dc_ibat_regl_hys : 1;
        unsigned char da_dc_ibat_regl_cap : 2;
        unsigned char da_dc_det_psechop_en : 1;
        unsigned char da_cfly_scp_en : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_ibat_regl_sns_cap_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_ibat_regl_sns_cap_END (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_ibat_regl_res_START (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_ibat_regl_res_END (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_ibat_regl_hys_START (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_ibat_regl_hys_END (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_ibat_regl_cap_START (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_ibat_regl_cap_END (5)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_det_psechop_en_START (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_dc_det_psechop_en_END (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_cfly_scp_en_START (7)
#define SOC_SCHARGER_DC_TOP_CFG_REG_0_da_cfly_scp_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dc_ibus_regl_res : 2;
        unsigned char da_dc_ibus_regl_hys : 1;
        unsigned char da_dc_ibus_regl_cap : 2;
        unsigned char da_dc_ibat_sns_ibias_sel : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_1_da_dc_ibus_regl_res_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_1_da_dc_ibus_regl_res_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_1_da_dc_ibus_regl_hys_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_1_da_dc_ibus_regl_hys_END (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_1_da_dc_ibus_regl_cap_START (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_1_da_dc_ibus_regl_cap_END (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_1_da_dc_ibat_sns_ibias_sel_START (5)
#define SOC_SCHARGER_DC_TOP_CFG_REG_1_da_dc_ibat_sns_ibias_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dc_rcpen_delay_adj : 2;
        unsigned char da_dc_ibus_regl_sel : 6;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_2_da_dc_rcpen_delay_adj_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_2_da_dc_rcpen_delay_adj_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_2_da_dc_ibus_regl_sel_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_2_da_dc_ibus_regl_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dc_regl_lpf_time : 1;
        unsigned char da_dc_regl_en : 4;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_3_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_3_da_dc_regl_lpf_time_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_3_da_dc_regl_lpf_time_END (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_3_da_dc_regl_en_START (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_3_da_dc_regl_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dc_regl_resv : 4;
        unsigned char reserved : 4;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_4_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_4_da_dc_regl_resv_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_4_da_dc_regl_resv_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dc_reserve0 : 8;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_5_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_5_da_dc_reserve0_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_5_da_dc_reserve0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dc_reserve1 : 8;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_6_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_6_da_dc_reserve1_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_6_da_dc_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_dc_vbat_regl_res : 2;
        unsigned char da_dc_vbat_regl_cap : 2;
        unsigned char da_dc_ucpen_mode : 1;
        unsigned char da_dc_ucpen_delay_adj : 2;
        unsigned char da_dc_ss_time : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_7_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_vbat_regl_res_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_vbat_regl_res_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_vbat_regl_cap_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_vbat_regl_cap_END (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_ucpen_mode_START (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_ucpen_mode_END (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_ucpen_delay_adj_START (5)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_ucpen_delay_adj_END (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_ss_time_START (7)
#define SOC_SCHARGER_DC_TOP_CFG_REG_7_da_dc_ss_time_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ibat_ocp_en : 1;
        unsigned char da_ibat_ocp_dbt : 1;
        unsigned char da_dc_vout_regl_res : 2;
        unsigned char da_dc_vout_regl_hys : 1;
        unsigned char da_dc_vout_regl_cap : 2;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_8_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_ibat_ocp_en_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_ibat_ocp_en_END (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_ibat_ocp_dbt_START (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_ibat_ocp_dbt_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_dc_vout_regl_res_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_dc_vout_regl_res_END (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_dc_vout_regl_hys_START (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_dc_vout_regl_hys_END (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_dc_vout_regl_cap_START (5)
#define SOC_SCHARGER_DC_TOP_CFG_REG_8_da_dc_vout_regl_cap_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ibat_res_sel : 1;
        unsigned char da_ibat_ocp_sel : 7;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_9_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_9_da_ibat_res_sel_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_9_da_ibat_res_sel_END (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_9_da_ibat_ocp_sel_START (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_9_da_ibat_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ibus_ocp_dbt : 1;
        unsigned char da_ibat_ucp_alm_sel : 5;
        unsigned char da_ibat_ucp_alm_en : 1;
        unsigned char da_ibat_ucp_alm_dbt : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_10_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_10_da_ibus_ocp_dbt_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_10_da_ibus_ocp_dbt_END (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_10_da_ibat_ucp_alm_sel_START (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_10_da_ibat_ucp_alm_sel_END (5)
#define SOC_SCHARGER_DC_TOP_CFG_REG_10_da_ibat_ucp_alm_en_START (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_10_da_ibat_ucp_alm_en_END (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_10_da_ibat_ucp_alm_dbt_START (7)
#define SOC_SCHARGER_DC_TOP_CFG_REG_10_da_ibat_ucp_alm_dbt_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ibus_rcp_sel : 2;
        unsigned char da_ibus_rcp_en : 1;
        unsigned char da_ibus_rcp_dbt : 1;
        unsigned char da_ibus_ocp_sel : 3;
        unsigned char da_ibus_ocp_en : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_11_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_rcp_sel_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_rcp_sel_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_rcp_en_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_rcp_en_END (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_rcp_dbt_START (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_rcp_dbt_END (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_ocp_sel_START (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_ocp_sel_END (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_ocp_en_START (7)
#define SOC_SCHARGER_DC_TOP_CFG_REG_11_da_ibus_ocp_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_ilim_sc_ocp_en : 1;
        unsigned char da_ilim_bus_sc_ocp_sel : 5;
        unsigned char da_ilim_bus_sc_ocp_en : 1;
        unsigned char da_ibus_ucp_en : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_12_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_12_da_ilim_sc_ocp_en_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_12_da_ilim_sc_ocp_en_END (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_12_da_ilim_bus_sc_ocp_sel_START (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_12_da_ilim_bus_sc_ocp_sel_END (5)
#define SOC_SCHARGER_DC_TOP_CFG_REG_12_da_ilim_bus_sc_ocp_en_START (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_12_da_ilim_bus_sc_ocp_en_END (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_12_da_ibus_ucp_en_START (7)
#define SOC_SCHARGER_DC_TOP_CFG_REG_12_da_ibus_ucp_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sc_drvq1f : 3;
        unsigned char da_ilim_sc_ocp_sel : 5;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_13_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_13_da_sc_drvq1f_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_13_da_sc_drvq1f_END (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_13_da_ilim_sc_ocp_sel_START (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_13_da_ilim_sc_ocp_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sc_drvq2f : 3;
        unsigned char da_sc_drvq1r : 3;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_14_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_14_da_sc_drvq2f_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_14_da_sc_drvq2f_END (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_14_da_sc_drvq1r_START (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_14_da_sc_drvq1r_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sc_drvq3f : 3;
        unsigned char da_sc_drvq2r : 3;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_15_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_15_da_sc_drvq3f_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_15_da_sc_drvq3f_END (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_15_da_sc_drvq2r_START (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_15_da_sc_drvq2r_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sc_drvq4f : 3;
        unsigned char da_sc_drvq3r : 3;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_16_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_16_da_sc_drvq4f_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_16_da_sc_drvq4f_END (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_16_da_sc_drvq3r_START (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_16_da_sc_drvq3r_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sc_dtgbga_dly : 2;
        unsigned char da_sc_dt_grev : 2;
        unsigned char da_sc_drvq4r : 3;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_17_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_17_da_sc_dtgbga_dly_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_17_da_sc_dtgbga_dly_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_17_da_sc_dt_grev_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_17_da_sc_dt_grev_END (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_17_da_sc_drvq4r_START (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_17_da_sc_drvq4r_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sc_ts_mod : 1;
        unsigned char da_sc_ini_dt : 1;
        unsigned char da_sc_gbdt_adj : 1;
        unsigned char da_sc_gadt_adj : 1;
        unsigned char da_sc_freq_sh : 1;
        unsigned char da_sc_freq : 3;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_18_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_ts_mod_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_ts_mod_END (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_ini_dt_START (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_ini_dt_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_gbdt_adj_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_gbdt_adj_END (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_gadt_adj_START (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_gadt_adj_END (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_freq_sh_START (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_freq_sh_END (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_freq_START (5)
#define SOC_SCHARGER_DC_TOP_CFG_REG_18_da_sc_freq_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_sc_ts_patha : 1;
        unsigned char da_sc_ts_muxselb : 3;
        unsigned char da_sc_ts_muxsela : 3;
        unsigned char da_sc_ts_mux : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_19_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_19_da_sc_ts_patha_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_19_da_sc_ts_patha_END (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_19_da_sc_ts_muxselb_START (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_19_da_sc_ts_muxselb_END (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_19_da_sc_ts_muxsela_START (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_19_da_sc_ts_muxsela_END (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_19_da_sc_ts_mux_START (7)
#define SOC_SCHARGER_DC_TOP_CFG_REG_19_da_sc_ts_mux_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vdrop_lvc_ovp_nflt_sel : 2;
        unsigned char da_vdrop_lvc_ovp_en : 1;
        unsigned char da_vdrop_lvc_ovp_dbt : 1;
        unsigned char da_vdrop_lvc_min_nflt_sel : 2;
        unsigned char da_vdrop_lvc_min_en : 1;
        unsigned char da_sc_ts_pathb : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_20_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_ovp_nflt_sel_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_ovp_nflt_sel_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_ovp_en_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_ovp_en_END (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_ovp_dbt_START (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_ovp_dbt_END (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_min_nflt_sel_START (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_min_nflt_sel_END (5)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_min_en_START (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_vdrop_lvc_min_en_END (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_sc_ts_pathb_START (7)
#define SOC_SCHARGER_DC_TOP_CFG_REG_20_da_sc_ts_pathb_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vin2vout_sc_en : 1;
        unsigned char da_vin2vout_sc_dbt : 1;
        unsigned char da_vdrop_lvc_ovp_sel : 5;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_21_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_21_da_vin2vout_sc_en_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_21_da_vin2vout_sc_en_END (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_21_da_vin2vout_sc_dbt_START (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_21_da_vin2vout_sc_dbt_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_21_da_vdrop_lvc_ovp_sel_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_21_da_vdrop_lvc_ovp_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_vin2vout_sc_min_sel : 2;
        unsigned char da_vin2vout_sc_min_nflt_sel : 2;
        unsigned char da_vin2vout_sc_max_sel : 2;
        unsigned char da_vin2vout_sc_max_nflt_sel : 2;
    } reg;
} SOC_SCHARGER_DC_TOP_CFG_REG_22_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_CFG_REG_22_da_vin2vout_sc_min_sel_START (0)
#define SOC_SCHARGER_DC_TOP_CFG_REG_22_da_vin2vout_sc_min_sel_END (1)
#define SOC_SCHARGER_DC_TOP_CFG_REG_22_da_vin2vout_sc_min_nflt_sel_START (2)
#define SOC_SCHARGER_DC_TOP_CFG_REG_22_da_vin2vout_sc_min_nflt_sel_END (3)
#define SOC_SCHARGER_DC_TOP_CFG_REG_22_da_vin2vout_sc_max_sel_START (4)
#define SOC_SCHARGER_DC_TOP_CFG_REG_22_da_vin2vout_sc_max_sel_END (5)
#define SOC_SCHARGER_DC_TOP_CFG_REG_22_da_vin2vout_sc_max_nflt_sel_START (6)
#define SOC_SCHARGER_DC_TOP_CFG_REG_22_da_vin2vout_sc_max_nflt_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_scc_cflyp_scp : 1;
        unsigned char ad_scc_cflyn_scp : 1;
        unsigned char ad_sc_rdy : 1;
        unsigned char reserved : 5;
    } reg;
} SOC_SCHARGER_DC_TOP_RO_REG_23_UNION;
#endif
#define SOC_SCHARGER_DC_TOP_RO_REG_23_ad_scc_cflyp_scp_START (0)
#define SOC_SCHARGER_DC_TOP_RO_REG_23_ad_scc_cflyp_scp_END (0)
#define SOC_SCHARGER_DC_TOP_RO_REG_23_ad_scc_cflyn_scp_START (1)
#define SOC_SCHARGER_DC_TOP_RO_REG_23_ad_scc_cflyn_scp_END (1)
#define SOC_SCHARGER_DC_TOP_RO_REG_23_ad_sc_rdy_START (2)
#define SOC_SCHARGER_DC_TOP_RO_REG_23_ad_sc_rdy_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id0 : 8;
    } reg;
} SOC_SCHARGER_VERSION0_RO_REG_0_UNION;
#endif
#define SOC_SCHARGER_VERSION0_RO_REG_0_chip_id0_START (0)
#define SOC_SCHARGER_VERSION0_RO_REG_0_chip_id0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id1 : 8;
    } reg;
} SOC_SCHARGER_VERSION1_RO_REG_0_UNION;
#endif
#define SOC_SCHARGER_VERSION1_RO_REG_0_chip_id1_START (0)
#define SOC_SCHARGER_VERSION1_RO_REG_0_chip_id1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id2 : 8;
    } reg;
} SOC_SCHARGER_VERSION2_RO_REG_0_UNION;
#endif
#define SOC_SCHARGER_VERSION2_RO_REG_0_chip_id2_START (0)
#define SOC_SCHARGER_VERSION2_RO_REG_0_chip_id2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id3 : 8;
    } reg;
} SOC_SCHARGER_VERSION3_RO_REG_0_UNION;
#endif
#define SOC_SCHARGER_VERSION3_RO_REG_0_chip_id3_START (0)
#define SOC_SCHARGER_VERSION3_RO_REG_0_chip_id3_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id4 : 8;
    } reg;
} SOC_SCHARGER_VERSION4_RO_REG_0_UNION;
#endif
#define SOC_SCHARGER_VERSION4_RO_REG_0_chip_id4_START (0)
#define SOC_SCHARGER_VERSION4_RO_REG_0_chip_id4_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char chip_id5 : 8;
    } reg;
} SOC_SCHARGER_VERSION5_RO_REG_0_UNION;
#endif
#define SOC_SCHARGER_VERSION5_RO_REG_0_chip_id5_START (0)
#define SOC_SCHARGER_VERSION5_RO_REG_0_chip_id5_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_12v_maxduty_en : 1;
        unsigned char da_buck_12v_maxduty_adj : 2;
        unsigned char da_buck_ilimit : 5;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_0_da_buck_12v_maxduty_en_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_0_da_buck_12v_maxduty_en_END (0)
#define SOC_SCHARGER_BUCK_CFG_REG_0_da_buck_12v_maxduty_adj_START (1)
#define SOC_SCHARGER_BUCK_CFG_REG_0_da_buck_12v_maxduty_adj_END (2)
#define SOC_SCHARGER_BUCK_CFG_REG_0_da_buck_ilimit_START (3)
#define SOC_SCHARGER_BUCK_CFG_REG_0_da_buck_ilimit_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_9v_maxduty_en : 1;
        unsigned char da_buck_9v_maxduty_adj : 2;
        unsigned char reserved : 5;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_1_da_buck_9v_maxduty_en_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_1_da_buck_9v_maxduty_en_END (0)
#define SOC_SCHARGER_BUCK_CFG_REG_1_da_buck_9v_maxduty_adj_START (1)
#define SOC_SCHARGER_BUCK_CFG_REG_1_da_buck_9v_maxduty_adj_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_antibst : 8;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_2_da_buck_antibst_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_2_da_buck_antibst_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_cap2_ea : 2;
        unsigned char da_buck_cap1_ea : 3;
        unsigned char da_buck_block_ctrl : 3;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_3_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_3_da_buck_cap2_ea_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_3_da_buck_cap2_ea_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_3_da_buck_cap1_ea_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_3_da_buck_cap1_ea_END (4)
#define SOC_SCHARGER_BUCK_CFG_REG_3_da_buck_block_ctrl_START (5)
#define SOC_SCHARGER_BUCK_CFG_REG_3_da_buck_block_ctrl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_cap7_acl : 2;
        unsigned char da_buck_cap6_dpm : 2;
        unsigned char da_buck_cap5_cc : 2;
        unsigned char da_buck_cap3_ea : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_4_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_4_da_buck_cap7_acl_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_4_da_buck_cap7_acl_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_4_da_buck_cap6_dpm_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_4_da_buck_cap6_dpm_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_4_da_buck_cap5_cc_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_4_da_buck_cap5_cc_END (5)
#define SOC_SCHARGER_BUCK_CFG_REG_4_da_buck_cap3_ea_START (6)
#define SOC_SCHARGER_BUCK_CFG_REG_4_da_buck_cap3_ea_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_dmd_ibias : 2;
        unsigned char da_buck_dmd_en : 1;
        unsigned char da_buck_dmd_delay : 1;
        unsigned char da_buck_dmd_clamp : 1;
        unsigned char da_buck_cmp_ibias_adj : 1;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_5_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_dmd_ibias_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_dmd_ibias_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_dmd_en_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_dmd_en_END (2)
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_dmd_delay_START (3)
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_dmd_delay_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_dmd_clamp_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_dmd_clamp_END (4)
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_cmp_ibias_adj_START (5)
#define SOC_SCHARGER_BUCK_CFG_REG_5_da_buck_cmp_ibias_adj_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_dpm_auto : 1;
        unsigned char da_buck_dmd_sel : 4;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_6_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_6_da_buck_dpm_auto_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_6_da_buck_dpm_auto_END (0)
#define SOC_SCHARGER_BUCK_CFG_REG_6_da_buck_dmd_sel_START (1)
#define SOC_SCHARGER_BUCK_CFG_REG_6_da_buck_dmd_sel_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_dt_lshs_delay : 2;
        unsigned char da_buck_dt_lshs : 1;
        unsigned char da_buck_dt_hsls : 1;
        unsigned char da_buck_dpm_sel : 4;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_7_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_7_da_buck_dt_lshs_delay_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_7_da_buck_dt_lshs_delay_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_7_da_buck_dt_lshs_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_7_da_buck_dt_lshs_END (2)
#define SOC_SCHARGER_BUCK_CFG_REG_7_da_buck_dt_hsls_START (3)
#define SOC_SCHARGER_BUCK_CFG_REG_7_da_buck_dt_hsls_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_7_da_buck_dpm_sel_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_7_da_buck_dpm_sel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_fullduty_en : 1;
        unsigned char da_buck_fullduty_delaytime : 4;
        unsigned char da_buck_dt_sel : 2;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_8_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_8_da_buck_fullduty_en_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_8_da_buck_fullduty_en_END (0)
#define SOC_SCHARGER_BUCK_CFG_REG_8_da_buck_fullduty_delaytime_START (1)
#define SOC_SCHARGER_BUCK_CFG_REG_8_da_buck_fullduty_delaytime_END (4)
#define SOC_SCHARGER_BUCK_CFG_REG_8_da_buck_dt_sel_START (5)
#define SOC_SCHARGER_BUCK_CFG_REG_8_da_buck_dt_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_gap_auto : 1;
        unsigned char da_buck_gap : 3;
        unsigned char da_buck_fullduty_offtime : 2;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_9_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_9_da_buck_gap_auto_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_9_da_buck_gap_auto_END (0)
#define SOC_SCHARGER_BUCK_CFG_REG_9_da_buck_gap_START (1)
#define SOC_SCHARGER_BUCK_CFG_REG_9_da_buck_gap_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_9_da_buck_fullduty_offtime_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_9_da_buck_fullduty_offtime_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_hmosocp_lpf : 2;
        unsigned char da_buck_hmos_rise : 3;
        unsigned char da_buck_hmos_fall : 3;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_10_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_10_da_buck_hmosocp_lpf_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_10_da_buck_hmosocp_lpf_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_10_da_buck_hmos_rise_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_10_da_buck_hmos_rise_END (4)
#define SOC_SCHARGER_BUCK_CFG_REG_10_da_buck_hmos_fall_START (5)
#define SOC_SCHARGER_BUCK_CFG_REG_10_da_buck_hmos_fall_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_lmos_rise : 3;
        unsigned char da_buck_lmos_on_sel : 1;
        unsigned char da_buck_lmos_on_en : 1;
        unsigned char da_buck_lmos_fall : 3;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_11_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_11_da_buck_lmos_rise_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_11_da_buck_lmos_rise_END (2)
#define SOC_SCHARGER_BUCK_CFG_REG_11_da_buck_lmos_on_sel_START (3)
#define SOC_SCHARGER_BUCK_CFG_REG_11_da_buck_lmos_on_sel_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_11_da_buck_lmos_on_en_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_11_da_buck_lmos_on_en_END (4)
#define SOC_SCHARGER_BUCK_CFG_REG_11_da_buck_lmos_fall_START (5)
#define SOC_SCHARGER_BUCK_CFG_REG_11_da_buck_lmos_fall_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_min_ontime_sel : 1;
        unsigned char da_buck_min_ontime : 2;
        unsigned char da_buck_min_offtime_sel : 1;
        unsigned char da_buck_min_offtime : 2;
        unsigned char da_buck_lmosocp_lpf : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_12_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_min_ontime_sel_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_min_ontime_sel_END (0)
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_min_ontime_START (1)
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_min_ontime_END (2)
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_min_offtime_sel_START (3)
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_min_offtime_sel_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_min_offtime_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_min_offtime_END (5)
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_lmosocp_lpf_START (6)
#define SOC_SCHARGER_BUCK_CFG_REG_12_da_buck_lmosocp_lpf_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_ocp_delay : 2;
        unsigned char da_buck_ocp_cap : 1;
        unsigned char da_buck_ocp_9_enb : 1;
        unsigned char da_buck_ocp_300ma : 2;
        unsigned char da_buck_ocp_12vsel : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_13_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_delay_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_delay_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_cap_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_cap_END (2)
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_9_enb_START (3)
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_9_enb_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_300ma_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_300ma_END (5)
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_12vsel_START (6)
#define SOC_SCHARGER_BUCK_CFG_REG_13_da_buck_ocp_12vsel_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_offtime_judge : 2;
        unsigned char da_buck_ocp_vally : 2;
        unsigned char da_buck_ocp_peak : 2;
        unsigned char da_buck_ocp_mode_sel : 1;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_14_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_14_da_buck_offtime_judge_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_14_da_buck_offtime_judge_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_14_da_buck_ocp_vally_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_14_da_buck_ocp_vally_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_14_da_buck_ocp_peak_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_14_da_buck_ocp_peak_END (5)
#define SOC_SCHARGER_BUCK_CFG_REG_14_da_buck_ocp_mode_sel_START (6)
#define SOC_SCHARGER_BUCK_CFG_REG_14_da_buck_ocp_mode_sel_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_osc_frq : 4;
        unsigned char da_buck_offtime_judge_en : 1;
        unsigned char da_buck_offtime_judge_delay : 3;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_15_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_15_da_buck_osc_frq_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_15_da_buck_osc_frq_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_15_da_buck_offtime_judge_en_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_15_da_buck_offtime_judge_en_END (4)
#define SOC_SCHARGER_BUCK_CFG_REG_15_da_buck_offtime_judge_delay_START (5)
#define SOC_SCHARGER_BUCK_CFG_REG_15_da_buck_offtime_judge_delay_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_pre_op : 8;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_16_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_16_da_buck_pre_op_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_16_da_buck_pre_op_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_res1_ea : 3;
        unsigned char da_buck_rdy_en : 1;
        unsigned char da_buck_rc_thm : 2;
        unsigned char da_buck_q1ocp_adj : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_17_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_17_da_buck_res1_ea_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_17_da_buck_res1_ea_END (2)
#define SOC_SCHARGER_BUCK_CFG_REG_17_da_buck_rdy_en_START (3)
#define SOC_SCHARGER_BUCK_CFG_REG_17_da_buck_rdy_en_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_17_da_buck_rc_thm_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_17_da_buck_rc_thm_END (5)
#define SOC_SCHARGER_BUCK_CFG_REG_17_da_buck_q1ocp_adj_START (6)
#define SOC_SCHARGER_BUCK_CFG_REG_17_da_buck_q1ocp_adj_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_res13_dpm_pz : 2;
        unsigned char da_buck_res12_cc_pz : 2;
        unsigned char da_buck_res11_sel : 2;
        unsigned char da_buck_res10_cc_rout : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_18_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_18_da_buck_res13_dpm_pz_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_18_da_buck_res13_dpm_pz_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_18_da_buck_res12_cc_pz_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_18_da_buck_res12_cc_pz_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_18_da_buck_res11_sel_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_18_da_buck_res11_sel_END (5)
#define SOC_SCHARGER_BUCK_CFG_REG_18_da_buck_res10_cc_rout_START (6)
#define SOC_SCHARGER_BUCK_CFG_REG_18_da_buck_res10_cc_rout_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_res2_ea : 2;
        unsigned char da_buck_res15_acl_z : 2;
        unsigned char da_buck_res14_acl_p : 2;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_19_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_19_da_buck_res2_ea_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_19_da_buck_res2_ea_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_19_da_buck_res15_acl_z_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_19_da_buck_res15_acl_z_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_19_da_buck_res14_acl_p_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_19_da_buck_res14_acl_p_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_res5_acl_rout : 2;
        unsigned char da_buck_res4_dpm_gm : 2;
        unsigned char da_buck_res3_ea : 3;
        unsigned char reserved : 1;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_20_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_20_da_buck_res5_acl_rout_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_20_da_buck_res5_acl_rout_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_20_da_buck_res4_dpm_gm_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_20_da_buck_res4_dpm_gm_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_20_da_buck_res3_ea_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_20_da_buck_res3_ea_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_res9_cc_gm : 2;
        unsigned char da_buck_res8_cv_gm : 2;
        unsigned char da_buck_res7_sys_gm : 2;
        unsigned char da_buck_res6_thm_gm : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_21_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_21_da_buck_res9_cc_gm_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_21_da_buck_res9_cc_gm_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_21_da_buck_res8_cv_gm_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_21_da_buck_res8_cv_gm_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_21_da_buck_res7_sys_gm_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_21_da_buck_res7_sys_gm_END (5)
#define SOC_SCHARGER_BUCK_CFG_REG_21_da_buck_res6_thm_gm_START (6)
#define SOC_SCHARGER_BUCK_CFG_REG_21_da_buck_res6_thm_gm_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_reserve1 : 8;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_22_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_22_da_buck_reserve1_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_22_da_buck_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_reserve2 : 8;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_23_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_23_da_buck_reserve2_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_23_da_buck_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buck_ssmode_en : 1;
        unsigned char da_buck_ss_init_dis : 1;
        unsigned char da_buck_sft_scp_enb : 1;
        unsigned char da_buck_sft_ocp_en : 1;
        unsigned char da_buck_sft_maxduty_en : 1;
        unsigned char da_buck_scp_dis : 1;
        unsigned char da_buck_saw_vally_adj : 1;
        unsigned char da_buck_saw_peak_adj : 1;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_24_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_ssmode_en_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_ssmode_en_END (0)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_ss_init_dis_START (1)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_ss_init_dis_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_sft_scp_enb_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_sft_scp_enb_END (2)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_sft_ocp_en_START (3)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_sft_ocp_en_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_sft_maxduty_en_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_sft_maxduty_en_END (4)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_scp_dis_START (5)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_scp_dis_END (5)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_saw_vally_adj_START (6)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_saw_vally_adj_END (6)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_saw_peak_adj_START (7)
#define SOC_SCHARGER_BUCK_CFG_REG_24_da_buck_saw_peak_adj_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_buckotg_ss_speed : 2;
        unsigned char da_buckotg_acl_downen : 1;
        unsigned char da_buck_sysovp_en : 1;
        unsigned char da_buck_sysmin_sel : 2;
        unsigned char reserved : 2;
    } reg;
} SOC_SCHARGER_BUCK_CFG_REG_25_UNION;
#endif
#define SOC_SCHARGER_BUCK_CFG_REG_25_da_buckotg_ss_speed_START (0)
#define SOC_SCHARGER_BUCK_CFG_REG_25_da_buckotg_ss_speed_END (1)
#define SOC_SCHARGER_BUCK_CFG_REG_25_da_buckotg_acl_downen_START (2)
#define SOC_SCHARGER_BUCK_CFG_REG_25_da_buckotg_acl_downen_END (2)
#define SOC_SCHARGER_BUCK_CFG_REG_25_da_buck_sysovp_en_START (3)
#define SOC_SCHARGER_BUCK_CFG_REG_25_da_buck_sysovp_en_END (3)
#define SOC_SCHARGER_BUCK_CFG_REG_25_da_buck_sysmin_sel_START (4)
#define SOC_SCHARGER_BUCK_CFG_REG_25_da_buck_sysmin_sel_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_buck_ok : 1;
        unsigned char reserved : 7;
    } reg;
} SOC_SCHARGER_BUCK_RO_REG_26_UNION;
#endif
#define SOC_SCHARGER_BUCK_RO_REG_26_ad_buck_ok_START (0)
#define SOC_SCHARGER_BUCK_RO_REG_26_ad_buck_ok_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_buck_state0 : 8;
    } reg;
} SOC_SCHARGER_BUCK_RO_REG_27_UNION;
#endif
#define SOC_SCHARGER_BUCK_RO_REG_27_ad_buck_state0_START (0)
#define SOC_SCHARGER_BUCK_RO_REG_27_ad_buck_state0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_buck_state1 : 8;
    } reg;
} SOC_SCHARGER_BUCK_RO_REG_28_UNION;
#endif
#define SOC_SCHARGER_BUCK_RO_REG_28_ad_buck_state1_START (0)
#define SOC_SCHARGER_BUCK_RO_REG_28_ad_buck_state1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_hmos : 1;
        unsigned char da_otg_dmd : 1;
        unsigned char da_otg_clp_l_en : 1;
        unsigned char da_otg_clp_h_en : 1;
        unsigned char da_otg_ckmin : 2;
        unsigned char da_otg_30ma : 1;
        unsigned char da_com_otg_mode : 1;
    } reg;
} SOC_SCHARGER_OTG_CFG_REG_0_UNION;
#endif
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_hmos_START (0)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_hmos_END (0)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_dmd_START (1)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_dmd_END (1)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_clp_l_en_START (2)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_clp_l_en_END (2)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_clp_h_en_START (3)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_clp_h_en_END (3)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_ckmin_START (4)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_ckmin_END (5)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_30ma_START (6)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_otg_30ma_END (6)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_com_otg_mode_START (7)
#define SOC_SCHARGER_OTG_CFG_REG_0_da_com_otg_mode_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_ocp_en : 1;
        unsigned char da_otg_lmos_ocp : 2;
        unsigned char da_otg_lim_set : 2;
        unsigned char da_otg_lim_dcoffset : 3;
    } reg;
} SOC_SCHARGER_OTG_CFG_REG_1_UNION;
#endif
#define SOC_SCHARGER_OTG_CFG_REG_1_da_otg_ocp_en_START (0)
#define SOC_SCHARGER_OTG_CFG_REG_1_da_otg_ocp_en_END (0)
#define SOC_SCHARGER_OTG_CFG_REG_1_da_otg_lmos_ocp_START (1)
#define SOC_SCHARGER_OTG_CFG_REG_1_da_otg_lmos_ocp_END (2)
#define SOC_SCHARGER_OTG_CFG_REG_1_da_otg_lim_set_START (3)
#define SOC_SCHARGER_OTG_CFG_REG_1_da_otg_lim_set_END (4)
#define SOC_SCHARGER_OTG_CFG_REG_1_da_otg_lim_dcoffset_START (5)
#define SOC_SCHARGER_OTG_CFG_REG_1_da_otg_lim_dcoffset_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_ovp_en : 1;
        unsigned char da_otg_osc_ckmax : 2;
        unsigned char da_otg_osc : 4;
        unsigned char da_otg_ocp_sys : 1;
    } reg;
} SOC_SCHARGER_OTG_CFG_REG_2_UNION;
#endif
#define SOC_SCHARGER_OTG_CFG_REG_2_da_otg_ovp_en_START (0)
#define SOC_SCHARGER_OTG_CFG_REG_2_da_otg_ovp_en_END (0)
#define SOC_SCHARGER_OTG_CFG_REG_2_da_otg_osc_ckmax_START (1)
#define SOC_SCHARGER_OTG_CFG_REG_2_da_otg_osc_ckmax_END (2)
#define SOC_SCHARGER_OTG_CFG_REG_2_da_otg_osc_START (3)
#define SOC_SCHARGER_OTG_CFG_REG_2_da_otg_osc_END (6)
#define SOC_SCHARGER_OTG_CFG_REG_2_da_otg_ocp_sys_START (7)
#define SOC_SCHARGER_OTG_CFG_REG_2_da_otg_ocp_sys_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_reserve1 : 8;
    } reg;
} SOC_SCHARGER_OTG_CFG_REG_3_UNION;
#endif
#define SOC_SCHARGER_OTG_CFG_REG_3_da_otg_reserve1_START (0)
#define SOC_SCHARGER_OTG_CFG_REG_3_da_otg_reserve1_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_reserve2 : 8;
    } reg;
} SOC_SCHARGER_OTG_CFG_REG_4_UNION;
#endif
#define SOC_SCHARGER_OTG_CFG_REG_4_da_otg_reserve2_START (0)
#define SOC_SCHARGER_OTG_CFG_REG_4_da_otg_reserve2_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char da_otg_slop_set : 2;
        unsigned char da_otg_slop_en : 1;
        unsigned char da_otg_skip_set : 1;
        unsigned char da_otg_scp_time : 1;
        unsigned char da_otg_scp_en : 1;
        unsigned char da_otg_rf : 2;
    } reg;
} SOC_SCHARGER_OTG_CFG_REG_5_UNION;
#endif
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_slop_set_START (0)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_slop_set_END (1)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_slop_en_START (2)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_slop_en_END (2)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_skip_set_START (3)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_skip_set_END (3)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_scp_time_START (4)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_scp_time_END (4)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_scp_en_START (5)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_scp_en_END (5)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_rf_START (6)
#define SOC_SCHARGER_OTG_CFG_REG_5_da_otg_rf_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char otg_clp_l_set : 1;
        unsigned char da_wl_otg_mode : 1;
        unsigned char da_otg_uvp_en : 1;
        unsigned char da_otg_switch : 1;
        unsigned char da_otg_pfm_v_en : 1;
        unsigned char reserved : 3;
    } reg;
} SOC_SCHARGER_OTG_CFG_REG_6_UNION;
#endif
#define SOC_SCHARGER_OTG_CFG_REG_6_otg_clp_l_set_START (0)
#define SOC_SCHARGER_OTG_CFG_REG_6_otg_clp_l_set_END (0)
#define SOC_SCHARGER_OTG_CFG_REG_6_da_wl_otg_mode_START (1)
#define SOC_SCHARGER_OTG_CFG_REG_6_da_wl_otg_mode_END (1)
#define SOC_SCHARGER_OTG_CFG_REG_6_da_otg_uvp_en_START (2)
#define SOC_SCHARGER_OTG_CFG_REG_6_da_otg_uvp_en_END (2)
#define SOC_SCHARGER_OTG_CFG_REG_6_da_otg_switch_START (3)
#define SOC_SCHARGER_OTG_CFG_REG_6_da_otg_switch_END (3)
#define SOC_SCHARGER_OTG_CFG_REG_6_da_otg_pfm_v_en_START (4)
#define SOC_SCHARGER_OTG_CFG_REG_6_da_otg_pfm_v_en_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_otg_on : 1;
        unsigned char ad_otg_en_in : 1;
        unsigned char reserved : 6;
    } reg;
} SOC_SCHARGER_OTG_RO_REG_7_UNION;
#endif
#define SOC_SCHARGER_OTG_RO_REG_7_ad_otg_on_START (0)
#define SOC_SCHARGER_OTG_RO_REG_7_ad_otg_on_END (0)
#define SOC_SCHARGER_OTG_RO_REG_7_ad_otg_en_in_START (1)
#define SOC_SCHARGER_OTG_RO_REG_7_ad_otg_en_in_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_otg_state0 : 8;
    } reg;
} SOC_SCHARGER_OTG_RO_REG_8_UNION;
#endif
#define SOC_SCHARGER_OTG_RO_REG_8_ad_otg_state0_START (0)
#define SOC_SCHARGER_OTG_RO_REG_8_ad_otg_state0_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned char value;
    struct
    {
        unsigned char ad_otg_state1 : 8;
    } reg;
} SOC_SCHARGER_OTG_RO_REG_9_UNION;
#endif
#define SOC_SCHARGER_OTG_RO_REG_9_ad_otg_state1_START (0)
#define SOC_SCHARGER_OTG_RO_REG_9_ad_otg_state1_END (7)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
