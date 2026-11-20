/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _SCSI_SCSI_EH_H
#define _SCSI_SCSI_EH_H

#include <linux/scatterlist.h>

#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_common.h>
struct scsi_device;
struct Scsi_Host;

extern void scsi_eh_finish_cmd(struct scsi_cmnd *scmd,
			       struct list_head *done_q);
extern void scsi_eh_flush_done_q(struct list_head *done_q);
extern void scsi_report_bus_reset(struct Scsi_Host *, int);
extern void scsi_report_device_reset(struct Scsi_Host *, int, int);
extern int scsi_block_when_processing_errors(struct scsi_device *);
extern bool scsi_command_normalize_sense(const struct scsi_cmnd *cmd,
					 struct scsi_sense_hdr *sshdr);
extern int scsi_check_sense(struct scsi_cmnd *);

static inline bool scsi_sense_is_deferred(const struct scsi_sense_hdr *sshdr)
{
	return ((sshdr->response_code >= 0x70) && (sshdr->response_code & 1));
}

extern bool scsi_get_sense_info_fld(const u8 *sense_buffer, int sb_len,
				    u64 *info_out);

extern int scsi_ioctl_reset(struct scsi_device *, int __user *);

struct scsi_eh_save {
	/* saved state */
	int result;
	unsigned int resid_len;
	int eh_eflags;
	enum dma_data_direction data_direction;
	unsigned underflow;
	unsigned char cmd_len;
	unsigned char prot_op;
	unsigned char *cmnd;
	struct scsi_data_buffer sdb;
	struct request *next_rq;
	/* new command support */
	unsigned char eh_cmnd[BLK_MAX_CDB];
	struct scatterlist sense_sgl;
};

extern void scsi_eh_prep_cmnd(struct scsi_cmnd *scmd,
		struct scsi_eh_save *ses, unsigned char *cmnd,
		int cmnd_size, unsigned sense_bytes);

extern void scsi_eh_restore_cmnd(struct scsi_cmnd* scmd,
		struct scsi_eh_save *ses);

#if defined(CONFIG_MAS_ORDER_PRESERVE) || defined(CONFIG_MAS_UNISTORE_PRESERVE)
static inline bool scsi_order(struct scsi_cmnd *scmd)
{
	return (scmd->request && scmd->request->q &&
		blk_queue_query_order_enable(scmd->request->q));
}

static inline bool scsi_unistore(struct scsi_cmnd *scmd)
{
	return (scmd->request && scmd->request->q &&
		blk_queue_query_unistore_enable(scmd->request->q));
}

static inline bool scsi_order_enable(struct scsi_cmnd *scmd)
{
	return scsi_unistore(scmd) || scsi_order(scmd);
}

static inline bool scsi_is_order_cmd(struct scsi_cmnd *scmd)
{
#ifdef CONFIG_MAS_UNISTORE_PRESERVE
	if (scsi_unistore(scmd))
		return (scmd->cmnd[0] == WRITE_10) ||
			((scmd->cmnd[0] == SECURITY_PROTOCOL_OUT) &&
			(scmd->request->mas_req.make_req_nr));
#endif

	return (scmd->cmnd[0] == WRITE_10) || (scmd->cmnd[0] == UNMAP) ||
		((scmd->cmnd[0] == SECURITY_PROTOCOL_OUT) &&
		(scmd->request->mas_req.make_req_nr));
}

#endif

#endif /* _SCSI_SCSI_EH_H */
