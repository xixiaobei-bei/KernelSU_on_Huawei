#USB type-c port controller
obj-$(CONFIG_HW_TCPC_RT1711H) += tcpc/tcpc_rt1711h.o
#USB type-c port controller interface
obj-$(CONFIG_HW_TCPC_CLASS) += tcpci/tcpci.o
#common
obj-$(CONFIG_HW_TCPC_CLASS) += common/timer.o
#type-c port
obj-$(CONFIG_HW_TCPC_CLASS) += typec/tcpc_core.o
obj-$(CONFIG_HW_TCPC_CLASS) += typec/tcpci_alert.o
obj-$(CONFIG_HW_TCPC_CLASS) += typec/typec_lpm.o
obj-$(CONFIG_HW_TCPC_CLASS) += typec/typec_vbus.o
obj-$(CONFIG_HW_TCPC_CLASS) += typec/typec_legacy_cable.o
obj-$(CONFIG_HW_TCPC_CLASS) += typec/typec_state_machine.o
obj-$(CONFIG_HW_TCPC_CLASS) += typec/typec_state.o
obj-$(CONFIG_HW_TCPC_CLASS) += typec/typec_snk_state.o
obj-$(CONFIG_HW_TCPC_CLASS) += typec/typec_src_state.o
#protocol
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += protocol/tcpci_event.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += protocol/pd_core.o
#policy manager
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt_snk.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt_src.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt_vdm.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt_drs.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt_prs.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt_vcs.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt_dbg.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt_tcp.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_process_evt_com.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_src.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_snk.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_ufp.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_vcs.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_dfp.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_dr.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_drs.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_prs.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_dbg.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += policy_engine/pd_policy_engine_com.o
#device policy manager
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += dpm/pd_dpm_core.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += dpm/pd_dpm_uvdm.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += dpm/pd_dpm_alt_mode_dp.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += dpm/pd_dpm_pdo_select.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += dpm/pd_dpm_reaction.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += dpm/pd_dpm_alt_mode_dc.o
obj-$(CONFIG_HW_USB_POWER_DELIVERY) += dpm/dpm_event.o
#interface
obj-$(CONFIG_HW_TCPC_CLASS) += tcpm/tcpm.o
obj-$(CONFIG_HW_TCPC_CLASS) += tcpm/tcpm_notifier.o
obj-$(CONFIG_HW_DUAL_ROLE_USB_INTF) += tcpm/tcpm_dual_role.o