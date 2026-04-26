/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Header file for chip_usb2_bc.c
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef __CHIP_USB2_BC_H__
#define __CHIP_USB2_BC_H__
/*
 * usb otg ahbif registers definations
 */
union usbotg2_ctrl0 {
	u32 reg;
	struct {
		/*
		 * bit[0] : ID pull-up resistor enable:
		 * 0:controller:
		 * 1:registering
		 */
		uint32_t  idpullup_sel : 1;
		uint32_t  idpullup : 1; /* bit[1] : ID pull-up resistor enable */
		/*
		 * bit[2] : ACA interface enable selection source:
		 * 0:controller
		 * 1:registering
		 */
		uint32_t  acaenb_sel : 1;
		uint32_t  acaenb : 1; /* bit[3]  : ACA interface enable status */
		/*
		 * bit[4-5]  : ACA interface source selection:
		 * 00:From the register:
		 * 01:iddig from the PHY:
		 * 10:ACA interface from the PHY:
		 * Other values: reserved
		 */
		uint32_t  id_sel : 2;
		uint32_t  id : 1; /* bit[6] : Working mode identification */
		/*
		 * bit[7] : Vbus voltage enable:
		 * 0:From Controller:
		 * 1:From the register
		 */
		uint32_t  drvvbus_sel : 1;
		uint32_t  drvvbus : 1; /* bit[8] : Enables the VBUS to apply voltage */
		/*
		 * bit[9] : Controller SessVLD signal source select:
		 * 0:Vbusvalid output by the PHY;
		 * 1:Selects the internal drvbus signal
		 */
		uint32_t  vbusvalid_sel : 1;
		/*
		 * bit[10] : Controller SessVLD signal source select:
		 * 0:SessVLD output by the PHY;
		 * 1:SessVLD of the register
		 */
		uint32_t  sessvld_sel : 1;
		uint32_t  sessvld : 1; /* bit[11]   : Session validity indicator */
		/*
		 * bit[12]   : PHY DP/DM pull-down resistor enable:
		 * 0:Select the dp/mpulldown of the controller.:
		 * 1:Selects the dp/mpulldown register.
		 */
		uint32_t  dpdmpulldown_sel : 1;
		uint32_t  dppulldown : 1; /* bit[13]  : Enable the pull-down resistor of the DP signal. */
		uint32_t  dmpulldown : 1; /* bit[14]  : Enables the pull-down resistor of the DM signal */
		/*
		 * bit[15]  : Filters for vbusvalid, avalid, bvalid
		 * sessend and iddig signals are removed
		 */
		uint32_t  dbnce_fltr_bypass : 1;
		uint32_t  reserved : 16; /* bit[16-31]: Reserved */
	} bits;
};

union usbotg2_ctrl1 {
	u32 reg;
	struct {
		uint32_t _scaledown_mode : 2;
		uint32_t _reserved : 30;
	} bits;
};

union usbotg2_ctrl2 {
	u32 reg;
	struct {
		uint32_t  commononn : 1; /* bit[0]  : PHY COMMON circuit power switch */
		uint32_t  otgdisable : 1; /* bit[1]  : Disable the OTG feature of the PHY */
		/*
		 * bit[2]  : VBUS valid selection:
		 * 0:From the internal comparator of the PHY
		 * 1:From the register
		 */
		uint32_t  vbusvldsel : 1;
		uint32_t  vbusvldext : 1; /* bit[3]  : VBUS valid */
		uint32_t  txbitstuffen : 1;
		uint32_t  txbitstuffenh : 1;
		uint32_t  fsxcvrowner : 1;
		uint32_t  txenablen : 1;
		uint32_t  fsdataext : 1; /* bit[8]  : Serial interface data output */
		uint32_t  fsse0ext : 1; /* bit[9]  : Serial interface output SE0 */
		uint32_t  vatestenb : 2;
		uint32_t  reserved : 20; /* bit[12-31]: Reserved. */
	} bits;
};

union usbotg2_ctrl3 {
	u32 reg;
	struct {
		uint32_t  comdistune : 3;
		uint32_t  otgtune : 3;
		uint32_t  sqrxtune : 3;
		uint32_t  txfslstune : 4;
		uint32_t  txpreempamptune : 2;
		uint32_t  txpreemppulsetune : 1;
		/*
		 * bit[16-17]: Adjusting the Eye Pattern Slope.
		 * 11:-8.1%
		 * 10:-7.2%
		 * 01:Default value
		 * 00:+5.4%
		 */
		uint32_t  txrisetune : 2;
		uint32_t  txvreftune : 4;
		uint32_t  txhsxvtune : 2;
		uint32_t  txrestune : 2;
		uint32_t  vdatreftune : 2;
		uint32_t  reserved : 4;
	} bits;
};

union usbotg2_ctrl4 {
	u32 reg;
	struct {
		uint32_t  siddq : 1; /* bit[0]  : IDDQ mode */
		/*
		 * bit[1]  : Vreg18 select. The value cannot be changed in Scan mode.
		 * 1:The VDDH requires the 1.8 V
		 * and external power supply.
		 * 0:The 3.3 V voltage is
		 * supplied to the VDDH.
		 */
		uint32_t  vregbypass : 1;
		uint32_t  loopbackenb : 1; /* bit[2]  : Loopback test enable. Used only in test mode. */
		uint32_t  bypasssel : 1; /* bit[3]  : Controls the bypass mode of the transceiver. */
		uint32_t  bypassdmen : 1; /* bit[4]  : DM bypass enable */
		uint32_t  bypassdpen : 1; /* bit[5]  : DP bypass enable */
		uint32_t  bypassdmdata : 1; /* bit[6]  : DM bypass data */
		uint32_t  bypassdpdata : 1; /* bit[7]  : DP bypass data */
		/*
		 * bit[8] : HS transceiver asynchronous control.
		 * and this parameter is valid only in high-speed mode.
		 */
		uint32_t  hsxcvrrextctl : 1;
		/*
		 * bit[9]  : Indicates whether to enable the reversion function.
		 * and The default value is 1
		 */
		uint32_t  retenablen : 1;
		uint32_t  autorsmenb : 1; /* bit[10] : Automatic wakeup. */
		uint32_t  reserved : 21; /* bit[11-31]: Reserved */
	} bits;
};

union usbotg2_ctrl5 {
	u32 reg;
	struct {
		uint32_t _refclksel : 2;
		uint32_t _fsel : 3;
		uint32_t _reserved : 27;
	} bits;
};

union bc_ctrl0 {
	u32 reg;
	struct {
		uint32_t _chrg_det_en : 1;
		uint32_t _reserved : 31;
	} bits;
};

union bc_ctrl1 {
	u32 reg;
	struct {
		uint32_t _chrg_det_int_clr : 1;
		uint32_t _reserved : 31;
	} bits;
};

union bc_ctrl2 {
	u32 reg;
	struct {
		uint32_t _chrg_det_int_msk : 1;
		uint32_t _reserved : 31;
	} bits;
};

union bc_ctrl3 {
	u32 reg;
	struct {
		uint32_t  bc_mode : 1; /* bit[0]  : BC mode enable */
		uint32_t  reserved : 31; /* bit[1-31]: Reserved */
	} bits;
};

union bc_ctrl4 {
	u32 reg;
	struct {
		uint32_t  bc_opmode : 2;
		uint32_t  bc_xcvrselect : 2; /* bit[2-3]  : Transceiver selection */
		uint32_t  bc_termselect : 1; /* bit[4]    : Terminal selection */
		uint32_t  bc_txvalid : 1; /* bit[5]    : UTMI+ lower 8-bit data TX enable */
		uint32_t  bc_txvalidh : 1; /* bit[6]    : UTMI+ upper 8-bit data TX enable */
		uint32_t  bc_idpullup : 1; /* bit[7]    : ID pull-up resistor enable */
		uint32_t  bc_dppulldown : 1; /* bit[8]    : DP pull-down resistor enable */
		uint32_t  bc_dmpulldown : 1; /* bit[9]    : DM pull-down resistor enable */
		uint32_t  bc_suspendm : 1; /* bit[10]   : Suspend mode */
		uint32_t  bc_sleepm : 1; /* bit[11]   : sleep mode	 */
		uint32_t  reserved : 20; /* bit[12-31]: Reserved. */
	} bits;
};

union bc_ctrl5 {
	u32 reg;
	struct {
		uint32_t  bc_aca_en : 1; /* bit[0]   :ACA interface enable status */
		uint32_t  bc_chrg_sel : 1; /* bit[1]   : Selecting the applied level data line */
		uint32_t  bc_vdat_src_en : 1; /* bit[2]   : Data port level application enable */
		uint32_t  bc_vdat_det_en : 1; /* bit[3]   : Data port level detection enable */
		uint32_t  bc_dcd_en : 1; /* bit[4]   : DCD detection enable */
		uint32_t  reserved : 27; /* bit[5-31]: Reserved */
	} bits;
};

union bc_ctrl6 {
	u32 reg;
	struct {
		uint32_t _bc_chirp_int_clr : 1;
		uint32_t _reserved : 31;
	} bits;
};

union bc_ctrl7 {
	u32 reg;
	struct {
		uint32_t _bc_chirp_int_msk : 1;
		uint32_t _reserved : 31;
	} bits;
};

union bc_ctrl8 {
	u32 reg;
	struct {
		u32 _filter_len;
	} bits;
};

union bc_sts0 {
	u32 reg;
	struct {
		uint32_t _chrg_det_rawint : 1;
		uint32_t _reserved : 31;
	} bits;
};

union bc_sts1 {
	u32 reg;
	struct {
		uint32_t _chrg_det_mskint : 1;
		uint32_t _reserved : 31;
	} bits;
};

union bc_sts2 {
	u32 reg;
	struct {
		uint32_t  bc_vbus_valid : 1; /* bit[0]    : vbus valid */
		uint32_t  bc_sess_valid : 1; /* bit[1]    : The session is valid. */
		uint32_t  bc_fs_vplus : 1; /* bit[2]    : DP status */
		uint32_t  bc_fs_vminus : 1; /* bit[3]    : DM status */
		uint32_t  bc_chg_det : 1; /* bit[4]    : Charging port detection */
		uint32_t  bc_iddig : 1; /* bit[5]    : ID level */
		uint32_t  bc_rid_float : 1; /* bit[6]    : ACA interface status */
		uint32_t  bc_rid_gnd : 1; /* bit[7]    : ACA interface status */
		uint32_t  bc_rid_a : 1; /* bit[8]    : ACA interface status */
		uint32_t  bc_rid_b : 1; /* bit[9]    : ACA interface status */
		uint32_t  bc_rid_c : 1; /* bit[10]   : ACA interface status */
		uint32_t  bc_chirp_on : 1; /* bit[11]   : Indicates the chip status. */
		uint32_t  bc_linestate : 2; /* bit[12-13]: Data cable status */
		uint32_t  reserved : 18; /* bit[14-31]: Reserved */
	} bits;
};

union bc_sts3 {
	u32 reg;
	struct {
		uint32_t _bc_rawint : 1;
		uint32_t _reserved : 31;
	} bits;
};

union bc_sts4 {
	u32 reg;
	struct {
		uint32_t _bc_mskint : 1;
		uint32_t _reserved : 31;
	} bits;
};

union usbotg2_ctrl6 {
	u32 reg;
	struct {
		/*
		 * bit[0]  : Test the clock. No continuous clock is required.
		 * Only the rising edge is valid.
		 */
		uint32_t  testclk : 1;
		/*
		 * bit[1]  : Output bus data select.
		 * 1:Mode-defined test register Output
		 * 0:Output of the internally generated signal defined by the mode
		 */
		uint32_t  testdataoutsel : 1;
		/*
		 * bit[2]  : Test interface select.
		 * 1:Driven by the SoC test pin
		 * 0:Controlled by internal registers
		 */
		uint32_t  test_sel : 1;
		uint32_t  reserved_0 : 1; /* bit[3]    : Reserved */
		uint32_t  testaddr : 4; /* bit[4-7]  : Test register address */
		uint32_t  testdatain : 8; /* bit[8-15] : Test bus write data */
		uint32_t  test_mux : 4; /* bit[16-19]: Test pin selection */
		uint32_t  reserved_1 : 12; /* bit[20-31]: Reserved */
	} bits;
};

union usbotg2_sts {
	u32 reg;
	struct {
		uint32_t  testdataout : 4; /* bit[0-3] : Test bus read data */
		uint32_t  hssqyelch : 1; /* bit[4]   : HS squelch detector output */
		uint32_t  hsrxdat : 1; /* bit[5]   : HS asynchronous data */
		/*
		 * bit[6]   : Differential value indicator
		 * 1:The voltage on D+ is greater than D-
		 * 0:The voltage on D-is greater than D+.
		 */
		uint32_t  fslsrcv : 1;
		uint32_t  reserved : 25; /* bit[7-31]: Reserved. */
	} bits;
};

struct usb_ahbif_registers {
	union usbotg2_ctrl0     usbotg2_ctrl0;
	union usbotg2_ctrl1     usbotg2_ctrl1;
	union usbotg2_ctrl2     usbotg2_ctrl2;
	union usbotg2_ctrl3     usbotg2_ctrl3;
	union usbotg2_ctrl4     usbotg2_ctrl4;
	union usbotg2_ctrl5     usbotg2_ctrl5;
	union bc_ctrl0          bc_ctrl0;
	union bc_ctrl1          bc_ctrl1;
	union bc_ctrl2          bc_ctrl2;
	union bc_ctrl3          bc_ctrl3;
	union bc_ctrl4          bc_ctrl4;
	union bc_ctrl5          bc_ctrl5;
	union bc_ctrl6          bc_ctrl6;
	union bc_ctrl7          bc_ctrl7;
	union bc_ctrl8          bc_ctrl8;
	union bc_sts0           bc_sts0;
	union bc_sts1           bc_sts1;
	union bc_sts2           bc_sts2;
	union bc_sts3           bc_sts3;
	union bc_sts4           bc_sts4;
	union usbotg2_ctrl6     usbotg2_ctrl6;
	union usbotg2_sts       usbotg2_sts;
};
#endif /* __CHIP_USB2_BC_H__ */
