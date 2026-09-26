/*
 * Copyright (C) 2016 Richtek Technology Corp.
 * Author: TH <tsunghan_tsai@richtek.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef TCPM_PD_H_
#define TCPM_PD_H_

#include "tcpci_config.h"
#include <linux/hisi/usb/hisi_tcpm.h>
#include <linux/hisi/usb/hisi_typec.h>

/* PD30 Data Message Data Object */
#define PD_ADO_SIZE	1

/* --- PD data message helpers --- */

/* PDO : Power Data Object */
/*
 * 1. The vSafe5V Fixed Supply Object shall always be the first object.
 * 2. The remaining Fixed Supply Objects,
 *    if present, shall be sent in voltage order; lowest to highest.
 * 3. The Battery Supply Objects,
 *    if present shall be sent in Minimum Voltage order; lowest to highest.
 * 4. The Variable Supply (non battery) Objects,
 *    if present, shall be sent in Minimum Voltage order; lowest to highest.
 */
#define PDO_TYPE_FIXED		0          /* (0 << 30) */
#define PDO_TYPE_BATTERY	0x40000000 /* (1 << 30) */
#define PDO_TYPE_VARIABLE	0x80000000 /* (2 << 30) */
#define PDO_TYPE_APDO		0xC0000000 /* (3 << 30) */
#define PDO_TYPE_MASK		0xC0000000 /* (3 << 30) */

#define DPM_PDO_TYPE_FIXED	0
#define DPM_PDO_TYPE_BAT	1
#define DPM_PDO_TYPE_VAR	2
#define DPM_PDO_TYPE_APDO	3
#define TCPM_POWER_CAP_VAL_TYPE_UNKNOWN 0xFF
#define DPM_PDO_TYPE(pdo)	(((pdo) & PDO_TYPE_MASK) >> 30)

#define PDO_FIXED_DUAL_ROLE (1 << 29) /* Dual role device */
#define PDO_FIXED_SUSPEND   (1 << 28) /* USB Suspend supported (SRC) */
#define PDO_FIXED_HIGH_CAP  (1 << 28) /* Higher Capability (SNK ) */
#define PDO_FIXED_EXTERNAL  (1 << 27) /* Externally powered */ /* Unconstrained Power */
#define PDO_FIXED_COMM_CAP  (1 << 26) /* USB Communications Capable */
#define PDO_FIXED_DATA_SWAP (1 << 25) /* Data role swap command supported */

#define PDO_FIXED_PEAK_CURR(i) \
	(((i) & 0x03) << 20) /* [21..20] Peak current */
#define PDO_FIXED_VOLT(mv)  \
	((((mv) / 50) & 0x3fff) << 10) /* Voltage in 50mV units */
#define PDO_FIXED_CURR(ma)  \
	((((ma) / 10) & 0x3fff) << 0)  /* Max current in 10mA units */

#define PDO_TYPE(raw)		((raw) & PDO_TYPE_MASK)
#define PDO_TYPE_VAL(raw)	(PDO_TYPE(raw) >> 30)

#define PDO_FIXED_EXTRACT_VOLT_RAW(raw)	(((raw) >> 10) & 0x3ff)
#define PDO_FIXED_EXTRACT_CURR_RAW(raw)	(((raw) >> 0) & 0x3ff)
#define PDO_FIXED_EXTRACT_VOLT(raw)	(PDO_FIXED_EXTRACT_VOLT_RAW(raw) * 50)
#define PDO_FIXED_EXTRACT_CURR(raw)	(PDO_FIXED_EXTRACT_CURR_RAW(raw) * 10)
#define PDO_FIXED_RESET_CURR(raw, ma) \
	(((raw) & ~0x3ff) | PDO_FIXED_CURR(ma))

#define PDO_FIXED(mv, ma, flags) (PDO_FIXED_VOLT(mv) | \
				  PDO_FIXED_CURR(ma) | (flags))

#define PDO_VAR_OP_CURR(ma)  ((((ma) / 10) & 0x3FF) << 0)

#define PDO_VAR_EXTRACT_MAX_VOLT_RAW(raw)	(((raw) >> 20) & 0x3ff)
#define PDO_VAR_EXTRACT_MIN_VOLT_RAW(raw)	(((raw) >> 10) & 0x3ff)
#define PDO_VAR_EXTRACT_CURR_RAW(raw)		(((raw) >> 0) & 0x3ff)

#define PDO_VAR_EXTRACT_MAX_VOLT(raw)	(PDO_VAR_EXTRACT_MAX_VOLT_RAW(raw) * 50)
#define PDO_VAR_EXTRACT_MIN_VOLT(raw)	(PDO_VAR_EXTRACT_MIN_VOLT_RAW(raw) * 50)
#define PDO_VAR_EXTRACT_CURR(raw)	(PDO_VAR_EXTRACT_CURR_RAW(raw) * 10)

#define PDO_VAR_RESET_CURR(raw, ma) \
	(((raw) & ~0x3ff) | PDO_VAR_OP_CURR(ma))

#define PDO_POWER_MW_2_UW	1000

#define PDO_BATT_EXTRACT_MAX_VOLT_RAW(raw)	(((raw) >> 20) & 0x3ff)
#define PDO_BATT_EXTRACT_MIN_VOLT_RAW(raw)	(((raw) >> 10) & 0x3ff)
#define PDO_BATT_EXTRACT_OP_POWER_RAW(raw)	(((raw) >> 0) & 0x3ff)

#define PDO_BATT_EXTRACT_MAX_VOLT(raw)	\
	(PDO_BATT_EXTRACT_MAX_VOLT_RAW(raw) * 50)
#define PDO_BATT_EXTRACT_MIN_VOLT(raw)	\
	(PDO_BATT_EXTRACT_MIN_VOLT_RAW(raw) * 50)
#define PDO_BATT_EXTRACT_OP_POWER(raw)	\
	(PDO_BATT_EXTRACT_OP_POWER_RAW(raw) * 250)

/* APDO : Augmented Power Data Object */

#define APDO_TYPE_MASK		(3 << 28)
#define APDO_TYPE_PPS		(0 << 28)

#define APDO_TYPE(raw)	((raw) & APDO_TYPE_MASK)
#define APDO_TYPE_VAL(raw)	(APDO_TYPE(raw) >> 28)

#define APDO_PPS_CURR_FOLDBACK	(1 << 26)
#define APDO_PPS_EXTRACT_MAX_VOLT_RAW(raw)	(((raw) >> 17) & 0xff)
#define APDO_PPS_EXTRACT_MIN_VOLT_RAW(raw)	(((raw) >> 8) & 0Xff)
#define APDO_PPS_EXTRACT_CURR_RAW(raw)	(((raw) >> 0) & 0x7f)

#define APDO_PPS_EXTRACT_MAX_VOLT(raw)	\
	(APDO_PPS_EXTRACT_MAX_VOLT_RAW(raw) * 100)
#define APDO_PPS_EXTRACT_MIN_VOLT(raw)	\
	(APDO_PPS_EXTRACT_MIN_VOLT_RAW(raw) * 100)
#define APDO_PPS_EXTRACT_CURR(raw)	\
	(APDO_PPS_EXTRACT_CURR_RAW(raw) * 50)

/* RDO : Request Data Object */
#define RDO_OBJ_POS(n)             (((n) & 0x7) << 28)
#define RDO_POS(rdo)               (((rdo) >> 28) & 0x7)
#define RDO_GIVE_BACK              (1 << 27)
#define RDO_CAP_MISMATCH           (1 << 26)
#define RDO_COMM_CAP               (1 << 25)
#define RDO_NO_SUSPEND             (1 << 24)
#define RDO_FIXED_VAR_OP_CURR(ma)  ((((ma) / 10) & 0x3FF) << 10)
#define RDO_FIXED_VAR_MAX_CURR(ma) ((((ma) / 10) & 0x3FF) << 0)

#define RDO_FIXED_VAR_EXTRACT_OP_CURR(raw)	(((((raw) >> 10) & 0x3ff)) * 10)
#define RDO_FIXED_VAR_EXTRACT_MAX_CURR(raw)	(((((raw) >> 0) & 0x3ff)) * 10)

#define RDO_BATT_OP_POWER(mw)      ((((mw) / 250) & 0x3FF) << 10)
#define RDO_BATT_MAX_POWER(mw)     ((((mw) / 250) & 0x3FF) << 0)

#define RDO_APDO_OP_MV(mv)	((((mv) / 20) & 0x7FF) << 9)
#define RDO_APDO_OP_MA(ma)	((((ma) / 50) & 0x7F) << 0)

#define RDO_BATT_EXTRACT_OP_POWER(raw)	(((((raw) >> 10) & 0x3ff)) * 250)
#define RDO_BATT_EXTRACT_MAX_POWER(raw)	(((((raw) >> 0) & 0x3ff)) * 250)

#define RDO_FIXED(n, op_ma, max_ma, flags) \
		(RDO_OBJ_POS(n) | (flags) | RDO_FIXED_VAR_OP_CURR(op_ma) | \
		RDO_FIXED_VAR_MAX_CURR(max_ma))

#define RDO_BATT(n, op_mw, max_mw, flags) \
		(RDO_OBJ_POS(n) | (flags) | RDO_BATT_OP_POWER(op_mw) | \
		RDO_BATT_MAX_POWER(max_mw))

#define RDO_APDO(n, op_mv, op_ma, flags)	\
		(RDO_OBJ_POS(n) | (flags) | RDO_APDO_OP_MV(op_mv) | \
		RDO_APDO_OP_MA(op_ma))

/* BDO : BIST Data Object */
#define BDO_MODE_RECV       (0 << 28)
#define BDO_MODE_TRANSMIT   (1 << 28)
#define BDO_MODE_COUNTERS   (2 << 28)
#define BDO_MODE_CARRIER0   (3 << 28)
#define BDO_MODE_CARRIER1   (4 << 28)
#define BDO_MODE_CARRIER2   (5 << 28)
#define BDO_MODE_CARRIER3   (6 << 28)
#define BDO_MODE_EYE        (7 << 28)
#define BDO_MODE_TEST_DATA	0x80000000

#define BDO_MODE(obj)		((obj) & (0xF0000000))
#define BDO(mode, cnt)		((mode) | ((cnt) & 0xFFFF))

#define SVID_DISCOVERY_MAX 16

/* build message header */
#define MAX_EXTENDED_MSG_CHUNK_LEN	26
#define MAX_EXTENDED_MSG_LEGACY_LEN	26

#define PD_HEADER_SOP(msg_type, pd_rev, prole, drole, id, cnt) \
		((msg_type) | ((pd_rev) << 6) | \
		 ((drole) << 5) | ((prole) << 8) | \
		 ((id) << 9) | ((cnt) << 12))

#define PD_HEADER_SOP_PRIME(msg_type, pd_rev, cable_plug, id, cnt) \
		((msg_type) | ((pd_rev) << 6) | \
		 ((cable_plug) << 8) | \
		 ((id) << 9) | ((cnt) << 12))

#define PD_HEADER_EXT(header) (((header) >> 15) & 1)
#define PD_HEADER_CNT(header)  (((header) >> 12) & 7)
#define PD_HEADER_TYPE(header) ((header) & 0xF)
#define PD_HEADER_ID(header)   (((header) >> 9) & 7)
#define PD_HEADER_PR(header)	(((header) >> 8) & 1)
#define PD_HEADER_REV(header)  (((header) >> 6) & 3)
#define PD_HEADER_DR(header)	(((header) >> 5) & 1)

#define PD_EXT_HEADER_PAYLOAD_INDEX	2
#define PD_EXT_HEADER_DATA_SIZE(header)	(((header) >> 0) & 0x1FF)
#define PD_EXT_HEADER_CK(data_size, req, chunk_nr, chunked)	\
		((data_size) | ((req) << 10) | \
		 ((chunk_nr) << 11) | ((chunked) << 15))
/*
 * VDM header
 * ----------
 * <31:16>  :: SVID
 * <15>     :: VDM type ( 1b == structured, 0b == unstructured )
 * <14:13>  :: Structured VDM version (can only be 00 == 1.0 currently)
 * <12:11>  :: reserved
 * <10:8>   :: object position (1-7 valid ... used for enter/exit mode only)
 * <7:6>    :: command type (SVDM only?)
 * <5>      :: reserved (SVDM), command type (UVDM)
 * <4:0>    :: command
 */
#define VDO_MAX_SIZE		7
#define VDO_MAX_DATA_SIZE	(VDO_MAX_SIZE - 1)
#define VDO_MAX_SVID_SIZE	(VDO_MAX_DATA_SIZE * 2)

#define VDO(vid, type, custom) \
	(((vid) << 16) | ((type) << 15) | ((custom) & 0x7FFF))

#define VDO_S(svid, cmd_type, cmd, obj)	\
	VDO((svid), 1, VDO_CMDT(cmd_type) | VDO_OPOS(obj) | (cmd))

#define VDO_SVDM_TYPE     (1 << 15)
#define VDO_SVDM_VERS(x)  ((x) << 13)
#define VDO_OPOS(x)       ((x) << 8)
#define VDO_CMDT(x)       ((x) << 6)
#define VDO_OPOS_MASK     VDO_OPOS(0x7)
#define VDO_CMDT_MASK     VDO_CMDT(0x3)

#define CMDT_INIT     0
#define CMDT_RSP_ACK  1
#define CMDT_RSP_NAK  2
#define CMDT_RSP_BUSY 3

#define CMD_DISCOVER_IDENT  1
#define CMD_DISCOVER_SVID   2
#define CMD_DISCOVER_MODES  3
#define CMD_ENTER_MODE      4
#define CMD_EXIT_MODE       5
#define CMD_ATTENTION       6
#define CMD_DP_STATUS      16
#define CMD_DP_CONFIG      17

#define VDO_CMD_VENDOR(x)    (((10 + (x)) & 0x1f))

#define PD_VDO_VID(vdo)  ((vdo) >> 16)
#define PD_VDO_SVDM(vdo) (((vdo) >> 15) & 1)
#define PD_VDO_OPOS(vdo) (((vdo) >> 8) & 0x7)
#define PD_VDO_CMD(vdo)  ((vdo) & 0x1f)
#define PD_VDO_CMDT(vdo) (((vdo) >> 6) & 0x3)

/*
 * SVDM Identity request -> response
 *
 * Request is simply properly formatted SVDM header
 *
 * Response is 4 data objects:
 * [0] :: SVDM header
 * [1] :: Identitiy header
 * [2] :: Cert Stat VDO
 * [3] :: (Product | Cable) VDO
 * [4] :: AMA VDO
 *
 */
#define VDO_INDEX_HDR         0
#define VDO_INDEX_IDH         1
#define VDO_INDEX_CABLE_EXT   3
#define VDO_INDEX_CABLE       4

#define PD_IDH_MODAL_SUPPORT   (1 << 26)

/*
 * Cable VDO
 */
#define CABLE_CURR_1A5   0
#define CABLE_CURR_3A    1
#define CABLE_CURR_5A    2
#define PD_VDO_CABLE_CURR(x)	(((x) >> 5) & 0x03)

/* PD30 Extend Message Data Object */

/* SDB, Status */
#define PD_SDB_SIZE	6

#define PD_STASUS_EVENT_OCP		(1 << 1)
#define PD_STATUS_EVENT_OTP		(1 << 2)
#define PD_STATUS_EVENT_OVP		(1 << 3)
#define PD_STATUS_EVENT_CF_MODE		(1 << 4)

#define PD_STASUS_EVENT_READ_CLEAR ( \
	PD_STASUS_EVENT_OCP | PD_STATUS_EVENT_OTP | PD_STATUS_EVENT_OVP)

#define PD_STASUS_EVENT_MASK ( \
	PD_STASUS_EVENT_OCP | PD_STATUS_EVENT_OTP | PD_STATUS_EVENT_OVP | \
	PD_STATUS_EVENT_CF_MODE)

#define PD_STATUS_TEMP_PTF(raw)		(((raw) & 0x06) >> 1)
#define PD_STATUS_TEMP_SET_PTF(val)	(((val) & 0x03) << 1)

/*
 * SVDM Discover SVIDs request -> response
 *
 * Request is properly formatted VDM Header with discover SVIDs command.
 * Response is a set of SVIDs of all all supported SVIDs with all zero's to
 * mark the end of SVIDs.  If more than 12 SVIDs are supported command SHOULD be
 * repeated.
 */
#define VDO_SVID(svid0, svid1) ((((svid0) & 0xffff) << 16) | ((svid1) & 0xffff))
#define PD_VDO_SVID_SVID0(vdo) ((vdo) >> 16)
#define PD_VDO_SVID_SVID1(vdo) ((vdo) & 0xffff)

/*
 * Mode Capabilities
 *
 * Number of VDOs supplied is SID dependent (but <= 6 VDOS?)
 */
#define VDO_MODE_CNT_DISPLAYPORT 1

/*
 * DisplayPort modes capabilities
 * -------------------------------
 * <31:24> : SBZ
 * <23:16> : UFP_D pin assignment supported
 * <15:8>  : DFP_D pin assignment supported
 * <7>     : USB 2.0 signaling (0b=yes, 1b=no)
 * <6>     : Plug | Receptacle (0b == plug, 1b == receptacle)
 * <5:2>   : xxx1: Supports DPv1.3, xx1x Supports USB Gen 2 signaling
 *           Other bits are reserved.
 * <1:0>   : signal direction ( 00b=rsv, 01b=sink, 10b=src 11b=both )
 */
#define VDO_MODE_DP(snkp, srcp, usb, gdr, sign, sdir)	(	\
		(((snkp) & 0xff) << 16)		\
		| (((srcp) & 0xff) << 8)		\
		| (((usb) & 1) << 7)		\
		| (((gdr) & 1) << 6)		\
		| (((sign) & 0xF) << 2)		\
		| (((sdir) & 0x3)))
#define PD_DP_PIN_CAPS(x) ((((x) >> 6) & 0x1) ? (((x) >> 16) & 0x3f) : (((x) >> 8) & 0x3f))

#define MODE_DP_PIN_A 0x01
#define MODE_DP_PIN_B 0x02
#define MODE_DP_PIN_C 0x04
#define MODE_DP_PIN_D 0x08
#define MODE_DP_PIN_E 0x10
#define MODE_DP_PIN_F 0x20

/* Pin configs B/D/F support multi-function */
#define MODE_DP_PIN_MF_MASK 0x2a
/* Pin configs A/B support BR2 signaling levels */
#define MODE_DP_PIN_BR2_MASK 0x3
/* Pin configs C/D/E/F support DP signaling levels */
#define MODE_DP_PIN_DP_MASK 0x3c

#define MODE_DP_V13  0x1
#define MODE_DP_GEN2 0x2

#define MODE_DP_SNK  0x1
#define MODE_DP_SRC  0x2
#define MODE_DP_BOTH 0x3

#define MODE_DP_PORT_CAP(raw)		((raw) & 0x03)
#define MODE_DP_SIGNAL_SUPPORT(raw)	(((raw) >> 2) & 0x0f)
#define MODE_DP_RECEPT(mode)	(((mode) >> 6) & 0x01)

#define MODE_DP_PIN_DFP(mode)	(((mode) >> 8) & 0xff)
#define MODE_DP_PIN_UFP(mode)	(((mode) >> 16) & 0xff)

#define PD_DP_DFP_D_PIN_CAPS(x)	(MODE_DP_RECEPT(x) ? \
		MODE_DP_PIN_DFP(x) : MODE_DP_PIN_UFP(x))

#define PD_DP_UFP_D_PIN_CAPS(x)	(MODE_DP_RECEPT(x) ? \
		MODE_DP_PIN_UFP(x) : MODE_DP_PIN_DFP(x))

/*
 * DisplayPort Status VDO
 * ----------------------
 * <31:9> : SBZ
 * <8>    : IRQ_HPD : 1 == irq arrived since last message otherwise 0.
 * <7>    : HPD state : 0 = HPD_LOW, 1 == HPD_HIGH
 * <6>    : Exit DP Alt mode: 0 == maintain, 1 == exit
 * <5>    : USB config : 0 == maintain current, 1 == switch to USB from DP
 * <4>    : Multi-function preference : 0 == no pref, 1 == MF preferred.
 * <3>    : enabled : is DPout on/off.
 * <2>    : power low : 0 == normal or LPM disabled, 1 == DP disabled for LPM
 * <1:0>  : connect status : 00b ==  no (DFP|UFP)_D is connected or disabled.
 *          01b == DFP_D connected, 10b == UFP_D connected, 11b == both.
 */

#define PD_VDO_DPSTS_HPD_IRQ(x) (((x) >> 8) & 1)
#define PD_VDO_DPSTS_HPD_LVL(x) (((x) >> 7) & 1)
#define PD_VDO_DPSTS_MF_PREF(x) (((x) >> 4) & 1)

#define PD_VDO_DPSTS_CONNECT(x)	(((x) >> 0) & 0x03)

#define DPSTS_DISCONNECT		0

#define DPSTS_DFP_D_CONNECTED	(1 << 0)
#define DPSTS_UFP_D_CONNECTED	(1 << 1)
#define DPSTS_BOTH_CONNECTED	(DPSTS_DFP_D_CONNECTED | DPSTS_UFP_D_CONNECTED)

/* UFP_U only */
#define DPSTS_DP_ENABLED		(1 << 3)
#define DPSTS_DP_MF_PREF		(1 << 4)

/* UFP_D only */
#define DPSTS_DP_HPD_STATUS		(1 << 7)

/*
 * DisplayPort Configure VDO
 * -------------------------
 * <31:24> : SBZ
 * <23:16> : SBZ
 * <15:8>  : Pin assignment requested.  Choose one from mode caps.
 * <7:6>   : SBZ
 * <5:2>   : signalling : 1h == DP v1.3, 2h == Gen 2
 *           Oh is only for USB, remaining values are reserved
 * <1:0>   : cfg : 00 == USB, 01 == DFP_D, 10 == UFP_D, 11 == reserved
 */

#define DP_CONFIG_USB				0
#define DP_CONFIG_DFP_D				1
#define DP_CONFIG_UFP_D				2

#define VDO_DP_CFG(pin, sig, cfg) \
	((((pin) & 0xff) << 8) | (((sig) & 0xf) << 2) | ((cfg) & 0x3))
#define VDO_DP_DFP_CFG(pin, sig) VDO_DP_CFG(pin, sig, DP_CONFIG_DFP_D)
#define VDO_DP_UFP_CFG(pin, sig) VDO_DP_CFG(pin, sig, DP_CONFIG_UFP_D)

#define PD_DP_CFG_USB(x)	(((x) & 0x3) == DP_CONFIG_USB)
#define PD_DP_CFG_DFP_D(x) (((x) & 0x3) == DP_CONFIG_DFP_D)
#define PD_DP_CFG_UFP_D(x) (((x) & 0x3) == DP_CONFIG_UFP_D)
#define PD_DP_CFG_DPON(x) (PD_DP_CFG_DFP_D(x) | PD_DP_CFG_UFP_D(x))

#define DP_SIG_DPV13 0x01
#define DP_SIG_GEN2	0x02

#define DP_PIN_ASSIGN_SUPPORT_A		(1 << 0)
#define DP_PIN_ASSIGN_SUPPORT_B		(1 << 1)
#define DP_PIN_ASSIGN_SUPPORT_C		(1 << 2)
#define DP_PIN_ASSIGN_SUPPORT_D		(1 << 3)
#define DP_PIN_ASSIGN_SUPPORT_E		(1 << 4)
#define DP_PIN_ASSIGN_SUPPORT_F		(1 << 5)

/*
 * Get the pin assignment mask
 * for backward compatibility, if it is null,
 * get the former sink pin assignment we used to be in <23:16>.
 */

#define PD_DP_CFG_PIN(x) (((x) >> 8) & 0xff)

#define HW_DEV_ID_MAJ(x) ((x) & 0x3ff)
#define HW_DEV_ID_MIN(x) ((x) >> 10)

/* USB-IF SIDs */
#define USB_SID_PD		0xff00 /* power delivery */
#define USB_SID_DISPLAYPORT	0xff01 /* display port */

/* Extend Message Data Object */
#define PD_PPSDB_SIZE 4

/* PD counter definitions */
#define PD_MESSAGE_ID_COUNT	7
#define PD_HARD_RESET_COUNT	2
#define PD_CAPS_COUNT		50
#define PD_GET_SNK_CAP_RETRIES	3
#define PD_GET_SRC_CAP_RETRIES	3
#define PD_DISCOVER_ID_COUNT	3 /* max : 20 */

#define PD_PORT_PDOS_MAX_SIZE	TCPM_PDO_MAX_SIZE

enum {
	PD_WAIT_VBUS_DISABLE = 0,
	PD_WAIT_VBUS_VALID_ONCE = 1,
	PD_WAIT_VBUS_INVALID_ONCE = 2,
	PD_WAIT_VBUS_SAFE0V_ONCE = 3,
	PD_WAIT_VBUS_STABLE_ONCE = 4,
};

enum tcp_dpm_return_code {
	TCP_DPM_RET_SUCCESS = 0,
	TCP_DPM_RET_SENT = 0,
	TCP_DPM_RET_VDM_ACK = 0,

	TCP_DPM_RET_DENIED_UNKNOWN,
	TCP_DPM_RET_DENIED_NOT_READY,
	TCP_DPM_RET_DENIED_LOCAL_CAP,
	TCP_DPM_RET_DENIED_PARTNER_CAP,
	TCP_DPM_RET_DENIED_SAME_ROLE, /* 5 */
	TCP_DPM_RET_DENIED_INVALID_REQUEST,
	TCP_DPM_RET_DENIED_REPEAT_REQUEST,
	TCP_DPM_RET_DENIED_WRONG_DATA_ROLE,
	TCP_DPM_RET_DENIED_PD_REV,

	TCP_DPM_RET_DROP_CC_DETACH, /* 10 */
	TCP_DPM_RET_DROP_SENT_SRESET,
	TCP_DPM_RET_DROP_RECV_SRESET,
	TCP_DPM_RET_DROP_SENT_HRESET,
	TCP_DPM_RET_DROP_RECV_HRESET,
	TCP_DPM_RET_DROP_ERROR_REOCVERY, /* 15 */
	TCP_DPM_RET_DROP_SEND_BIST,
	TCP_DPM_RET_DROP_PE_BUSY, /* SinkTXNg */
	TCP_DPM_RET_DROP_DISCARD,
	TCP_DPM_RET_DROP_UNEXPECTED,

	TCP_DPM_RET_WAIT, /* 20 */
	TCP_DPM_RET_REJECT,
	TCP_DPM_RET_TIMEOUT,
	TCP_DPM_RET_VDM_NAK,
	TCP_DPM_RET_NOT_SUPPORT,

	TCP_DPM_RET_BK_TIMEOUT, /* 25 */
	TCP_DPM_RET_NO_RESPONSE,

	TCP_DPM_RET_NR,
};

#endif /* TCPM_PD_H_ */
