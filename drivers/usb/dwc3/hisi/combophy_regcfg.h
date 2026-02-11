#ifndef __COMBOPHY_REGCFG_H__
#define __COMBOPHY_REGCFG_H__

extern void combophy_regcfg_reset_misc(void);
extern void combophy_regcfg_unreset_misc(void);
extern bool combophy_regcfg_is_misc_ctrl_unreset(void);
extern bool combophy_regcfg_is_misc_ctrl_clk_en(void);
extern void combophy_regcfg_phyreset(void);
extern void combophy_regcfg_phyunreset(void);
extern void combophy_regcfg_isodis(void);
extern void combophy_regcfg_exit_testpowerdown(void);
extern void combophy_regcfg_power_stable(void);
extern void combophy_regcfg_enter_testpowerdown(void);
extern bool combophy_regcfg_is_controller_ref_clk_en(void);
extern bool combophy_regcfg_is_controller_bus_clk_en(void);

#endif /* __COMBOPHY_REGCFG_H__ */
