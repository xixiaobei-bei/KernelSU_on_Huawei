#ifndef __COMBOPHY_H__
#define __COMBOPHY_H__

/* API form DP */
#if IS_ENABLED(CONFIG_HISI_FB_970) || IS_ENABLED(CONFIG_HISI_FB_V501)|| IS_ENABLED(CONFIG_HISI_FB_V510) /*lint !e553*/
extern int hisi_dptx_triger(bool benable);
extern int hisi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode, TYPEC_PLUG_ORIEN_E typec_orien);
extern int hisi_dptx_notify_switch(void);
#else
static inline int hisi_dptx_triger(bool benable) {return 0;}
static inline int hisi_dptx_hpd_trigger(TCA_IRQ_TYPE_E irq_type, TCPC_MUX_CTRL_TYPE mode, TYPEC_PLUG_ORIEN_E typec_orien) {return 0;}
static inline int hisi_dptx_notify_switch(void) {return 0;}
#endif

/* API form PD */
extern void dp_dfp_u_notify_dp_configuration_done(TCPC_MUX_CTRL_TYPE mode_type, int ack);

/* inner API form hisi usb */
extern int hisi_usb_otg_event_sync(TCPC_MUX_CTRL_TYPE mode_type, enum otg_dev_event_type event, TYPEC_PLUG_ORIEN_E typec_orien);
extern int dwc3_set_combophy_clk(void);
extern void dwc3_close_combophy_clk(void);

#endif /* __COMBOPHY_H__ */
