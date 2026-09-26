#ifndef __UFSTT_H
#define __UFSTT_H

#include "ufshcd.h"

#define UFSTT_READ_BUFFER		0xF9
#define UFSTT_READ_BUFFER_ID		0x01
#define UFSTT_READ_BUFFER_CONTROL	0x00

/* Device descriptor parameters offsets in bytes */
enum ttunit_desc_param {
	TTUNIT_DESC_PARAM_LEN			= 0x0,
	TTUNIT_DESC_PARAM_DESCRIPTOR_IDN	= 0x1,
	TTUNIT_DESC_PARAM_TURBO_TABLE_EN	= 0x2,
	TTUNIT_DESC_PARAM_L2P_SIZE		= 0x3,
};

#define SAM_STAT_GOOD_NEED_UPDATE 0x80

/* turbo table parameters offsets in bytes */
enum ufstt_desc_id {
	TURBO_TABLE_READ_BITMAP		= 0x4,
};

#define ufstt_hex_dump(prefix_str, buf, len)                                   \
	print_hex_dump(KERN_ERR, prefix_str, DUMP_PREFIX_OFFSET, 16, 4, buf,   \
		       len, false)

#ifdef CONFIG_SCSI_UFS_HI1861_VCMD
void ufstt_set_sdev(struct scsi_device *sdev);
void ufstt_probe(struct ufs_hba *hba);
void ufstt_remove(struct ufs_hba *hba);
void ufstt_prep_fn(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
void ufstt_unprep_fn(struct ufs_hba *hba, struct ufshcd_lrb *lrbp);
void ufstt_idle_handler(struct ufs_hba *hba, ktime_t now_time);
void ufstt_unprep_handler(struct ufs_hba *hba, struct ufshcd_lrb *lrbp,
			  ktime_t now_time);
void ufstt_node_update(void);
bool is_ufstt_batch_mode(void);
#else
static inline void ufstt_set_sdev(struct scsi_device *sdev)
{
}
static inline void ufstt_probe(struct ufs_hba *hba)
{
}
static inline void ufstt_remove(struct ufs_hba *hba)
{
}
static inline void ufstt_prep_fn(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
}
static inline void ufstt_unprep_fn(struct ufs_hba *hba, struct ufshcd_lrb *lrbp)
{
}
static inline void ufstt_idle_handler(struct ufs_hba *hba, ktime_t now_time)
{
}
static inline void ufstt_unprep_handler(struct ufs_hba *hba,
					struct ufshcd_lrb *lrbp,
					ktime_t now_time)
{
}
static inline void ufstt_node_update(void)
{
}
static inline bool is_ufstt_batch_mode(void)
{
	return false;
}
#endif

#endif
