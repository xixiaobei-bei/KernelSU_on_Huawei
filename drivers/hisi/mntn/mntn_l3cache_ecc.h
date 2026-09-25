

#ifndef __MNTN_L3CACHE_ECC_H__
#define __MNTN_L3CACHE_ECC_H__

#define HISI_L3CACHE_FN_MAIN_ID			(0xc500f004u)
#define L3CACHE_ECC_READ			(0x00)
#define L3CACHE_ECC_WRITE			(0x01)
#define L3CACHE_ECC_OPEN			(0x02)
#define L3CACHE_ECC_CLOSE			(0x03)
#define L3CACHE_ECC_ERR_INPUT			(0x04)
#define L3CACHE_ECC_INT_CLEAR			(0x05)

#define ECC_OK					(0)
#define ECC_ERR					(-1)

#define ERR1STATUS_UE_BIT_MASK			(0x20000000)
#define ERR1STATUS_V_BIT_MASK			(0x40000000)

struct hisi_l3cache_ecc_info {
	int irq_fault;
	int irq_err;
};

enum {
	ERRSELR_EL1 = 0,		    /* Error Record Select Register */
	ERXCTLR_EL1,			    /* Selected Error Record Control Register */
	ERR1MISC0,			    /* Error Record Miscellaneous Register0 */
	ERR1STATUS,			    /* Error Record Primary Status Register */
	ERXPFGCDNR_EL1,			    /* Error Pseudo Fault Generation Count Down Register */
	ERXPFGCTLR_EL1,			    /* Error Pseudo Fault Generation Control Register */
};

enum serr_type {
	NO_EEROR = 0,
	INTERNAL_DATA_BUFFER,
	CACHE_DATA_RAM,
	CACHE_TAG_DIRTY_RAM,
	BUS_ERROR,
	INVALID_VAL,
};

#endif /* __MNTN_L3CACHE_ECC_H__ */
