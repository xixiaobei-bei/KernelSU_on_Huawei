/*
 *  linux/include/linux/mmc/core.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef LINUX_MMC_CORE_H
#define LINUX_MMC_CORE_H

#include <linux/completion.h>
#include <linux/types.h>
#include <linux/fs.h>

struct mmc_data;
struct mmc_request;

enum mmc_blk_status {
	MMC_BLK_SUCCESS = 0,
	MMC_BLK_PARTIAL,
	MMC_BLK_CMD_ERR,
	MMC_BLK_RETRY,
	MMC_BLK_ABORT,
	MMC_BLK_DATA_ERR,
	MMC_BLK_ECC_ERR,
	MMC_BLK_NOMEDIUM,
	MMC_BLK_NEW_REQUEST,
};

struct mmc_command {
	u32			opcode;
	u32			arg;
#define MMC_CMD23_ARG_REL_WR	(1 << 31)
#define MMC_CMD23_ARG_PACKED	((0 << 31) | (1 << 30))
#define MMC_CMD23_ARG_TAG_REQ	(1 << 29)
	u32			resp[4];
	unsigned int		flags;		/* expected response type */
#define MMC_RSP_PRESENT	(1 << 0)
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */

#define MMC_CMD_MASK	(3 << 5)		/* non-SPI command type */
#define MMC_CMD_AC	(0 << 5)
#define MMC_CMD_ADTC	(1 << 5)
#define MMC_CMD_BC	(2 << 5)
#define MMC_CMD_BCR	(3 << 5)

#define MMC_RSP_SPI_S1	(1 << 7)		/* one status byte */
#define MMC_RSP_SPI_S2	(1 << 8)		/* second byte */
#define MMC_RSP_SPI_B4	(1 << 9)		/* four data bytes */
#define MMC_RSP_SPI_BUSY (1 << 10)		/* card may send busy */

/*
 * These are the native response types, and correspond to valid bit
 * patterns of the above flags.  One additional valid pattern
 * is all zeros, which means we don't expect a response.
 */
#define MMC_RSP_NONE	(0)
#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1B	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE|MMC_RSP_BUSY)
#define MMC_RSP_R2	(MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC)
#define MMC_RSP_R3	(MMC_RSP_PRESENT)
#define MMC_RSP_R4	(MMC_RSP_PRESENT)
#define MMC_RSP_R5	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R6	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R7	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)

/* Can be used by core to poll after switch to MMC HS mode */
#define MMC_RSP_R1_NO_CRC	(MMC_RSP_PRESENT|MMC_RSP_OPCODE)

#define mmc_resp_type(cmd)	((cmd)->flags & (MMC_RSP_PRESENT|MMC_RSP_136|MMC_RSP_CRC|MMC_RSP_BUSY|MMC_RSP_OPCODE))

/*
 * These are the SPI response types for MMC, SD, and SDIO cards.
 * Commands return R1, with maybe more info.  Zero is an error type;
 * callers must always provide the appropriate MMC_RSP_SPI_Rx flags.
 */
#define MMC_RSP_SPI_R1	(MMC_RSP_SPI_S1)
#define MMC_RSP_SPI_R1B	(MMC_RSP_SPI_S1|MMC_RSP_SPI_BUSY)
#define MMC_RSP_SPI_R2	(MMC_RSP_SPI_S1|MMC_RSP_SPI_S2)
#define MMC_RSP_SPI_R3	(MMC_RSP_SPI_S1|MMC_RSP_SPI_B4)
#define MMC_RSP_SPI_R4	(MMC_RSP_SPI_S1|MMC_RSP_SPI_B4)
#define MMC_RSP_SPI_R5	(MMC_RSP_SPI_S1|MMC_RSP_SPI_S2)
#define MMC_RSP_SPI_R7	(MMC_RSP_SPI_S1|MMC_RSP_SPI_B4)

#define mmc_spi_resp_type(cmd)	((cmd)->flags & \
		(MMC_RSP_SPI_S1|MMC_RSP_SPI_BUSY|MMC_RSP_SPI_S2|MMC_RSP_SPI_B4))

/*
 * These are the command types.
 */
#define mmc_cmd_type(cmd)	((cmd)->flags & MMC_CMD_MASK)

	unsigned int		retries;	/* max number of retries */
	int			error;		/* command error */

/*
 * Standard errno values are used for errors, but some have specific
 * meaning in the MMC layer:
 *
 * ETIMEDOUT    Card took too long to respond
 * EILSEQ       Basic format problem with the received or sent data
 *              (e.g. CRC check failed, incorrect opcode in response
 *              or bad end bit)
 * EINVAL       Request cannot be performed because of restrictions
 *              in hardware and/or the driver
 * ENOMEDIUM    Host can determine that the slot is empty and is
 *              actively failing requests
 */

/*do not use the system timer to check busy*/
#define MMC_TIMEOUT_INVALID (0xFFFFFFFF)
	unsigned int		busy_timeout;	/* busy detect timeout in ms */
	/* Set this flag only for blocking sanitize request */
	bool			sanitize_busy;

	struct mmc_data		*data;		/* data segment associated with cmd */
	struct mmc_request	*mrq;		/* associated request */
};

struct mmc_data {
	unsigned int		timeout_ns;	/* data timeout (in ns, max 80ms) */
	unsigned int		timeout_clks;	/* data timeout (in clocks) */
	unsigned int		blksz;		/* data block size */
	unsigned int		blocks;		/* number of blocks */
	unsigned int		blk_addr;	/* block address */
	int			error;		/* data error */
	unsigned int		flags;

#define MMC_DATA_WRITE		BIT(8)
#define MMC_DATA_READ		BIT(9)
/* Extra flags used by CQE */
#define MMC_DATA_QBR		BIT(10)		/* CQE queue barrier*/
#define MMC_DATA_PRIO		BIT(11)		/* CQE high priority */
#define MMC_DATA_REL_WR		BIT(12)		/* Reliable write */
#define MMC_DATA_DAT_TAG	BIT(13)		/* Tag request */
#define MMC_DATA_FORCED_PRG	BIT(14)		/* Forced programming */

	unsigned int		bytes_xfered;

	struct mmc_command	*stop;		/* stop command */
	struct mmc_request	*mrq;		/* associated request */

	unsigned int		sg_len;		/* size of scatter list */
	int			sg_count;	/* mapped sg entries */
	struct scatterlist	*sg;		/* I/O scatter list */
	s32			host_cookie;	/* host private data */
};

struct mmc_host;
struct mmc_request {
	struct mmc_command	*sbc;		/* SET_BLOCK_COUNT for multiblock */
	struct mmc_command	*cmd;
	struct mmc_data		*data;
	struct mmc_command	*stop;
	struct mmc_command	*task_mgmt;

	struct completion	completion;
	struct completion	cmd_completion;
	void			(*done)(struct mmc_request *);/* completion function */
	/*
	 * Notify uppers layers (e.g. mmc block driver) that recovery is needed
	 * due to an error associated with the mmc_request. Currently used only
	 * by CQE.
	 */
	void			(*recovery_notifier)(struct mmc_request *);
	struct mmc_host		*host;

#ifdef CONFIG_HUAWEI_DSM_IOMT_EMMC_HOST
	unsigned long io_start_ticks;
#endif

	/* Allow other commands during this ongoing data transfer or busy wait */
	bool			cap_cmd_during_tfr;
	struct mmc_cmdq_req	*cmdq_req;
	struct completion	cmdq_completion;
	struct request		*req; /* associated block request */
	int			tag;
};

struct mmc_card;
struct mmc_cmdq_req;

#ifdef CONFIG_HISI_MMC
extern void mmc_wait_cmdq_empty(struct mmc_card *);
extern int mmc_cmdq_start_req(struct mmc_host *host,
			      struct mmc_cmdq_req *cmdq_req);
extern void mmc_blk_cmdq_req_done(struct mmc_request *mrq);
extern int mmc_prep_request(struct request_queue *q, struct request *req);
extern void mmc_queue_setup_discard(struct request_queue *q,
				    struct mmc_card *card);
extern struct scatterlist *mmc_alloc_sg(int sg_len, gfp_t gfp);
extern int mmc_init_request(struct request_queue *q, struct request *req, gfp_t gfp);
extern void mmc_exit_request(struct request_queue *q, struct request *req);
#endif
void mmc_wait_for_req(struct mmc_host *host, struct mmc_request *mrq);
int mmc_wait_for_cmd(struct mmc_host *host, struct mmc_command *cmd,
		int retries);

extern int mmc_switch(struct mmc_card *, u8, u8, u8, unsigned int);
#ifdef CONFIG_HISI_MMC
extern int __mmc_switch_cmdq_mode(struct mmc_command *cmd, u8 set, u8 index,
					u8 value, unsigned int timeout_ms,
					bool use_busy_signal, bool ignore_timeout);
extern int mmc_cmdq_halt(struct mmc_host *host, bool enable);
extern void mmc_cmdq_post_req(struct mmc_host *host, struct mmc_request *mrq,
				int err);
extern int mmc_start_cmdq_request(struct mmc_host *host,
				   struct mmc_request *mrq);
extern int mmc_send_tuning(struct mmc_host *host, u32 opcode, int *cmd_error);
extern unsigned int mmc_erase_timeout(struct mmc_card *card,
				      unsigned int arg,
				      unsigned int qty);
int mmc_cmdq_hw_reset(struct mmc_host *host);
#endif
int mmc_hw_reset(struct mmc_host *host);
#ifdef CONFIG_HISI_MMC
extern int mmc_sd_reset(struct mmc_host *host);

extern int __mmc_claim_host(struct mmc_host *host, atomic_t *abort);
extern void mmc_release_host(struct mmc_host *host);
extern int mmc_try_claim_host(struct mmc_host *host);

extern int mmc_blk_cmdq_hangup(struct mmc_card *card);
extern void mmc_blk_cmdq_restore(struct mmc_card *card);
#endif

#ifdef CONFIG_SD_SDIO_CRC_RETUNING
extern int mmc_retuning(struct mmc_host *host);
#endif

#ifdef CONFIG_HISI_MMC
extern int mmc_switch_irq_safe(struct mmc_card *card, u8 set, u8 index, u8 value);
extern int mmc_get_card_hisi(struct mmc_card *card, bool use_irq);
extern int mmc_flush_cache_direct(struct mmc_card *card);
extern void mmc_put_card_irq_safe(struct mmc_card *card);
#endif

extern int mmc_erase(struct mmc_card *card, unsigned int from, unsigned int nr,
		unsigned int arg);
extern int mmc_can_erase(struct mmc_card *card);
extern int mmc_can_trim(struct mmc_card *card);
extern int mmc_can_discard(struct mmc_card *card);
extern int mmc_can_sanitize(struct mmc_card *card);
extern int mmc_can_secure_erase_trim(struct mmc_card *card);
extern int mmc_erase_group_aligned(struct mmc_card *card, unsigned int from,
			unsigned int nr);
extern unsigned int mmc_calc_max_discard(struct mmc_card *card);

extern int mmc_set_blocklen(struct mmc_card *card, unsigned int blocklen);
extern int mmc_set_blockcount(struct mmc_card *card, unsigned int blockcount,
			bool is_rel_write);

void mmc_set_data_timeout(struct mmc_data *data, const struct mmc_card *card);
/**
 *	mmc_claim_host - exclusively claim a host
 *	@host: mmc host to claim
 *
 *	Claim a host for a set of operations.
 */
static inline void mmc_claim_host(struct mmc_host *host)
{
	__mmc_claim_host(host, NULL);
}

#endif /* LINUX_MMC_CORE_H */
