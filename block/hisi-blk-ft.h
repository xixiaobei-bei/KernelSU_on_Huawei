#ifndef __HISI_BLK_FT_H__
#define __HISI_BLK_FT_H__

bool hisi_blk_ft_mq_queue_rq_redirection(
	struct request *rq, struct request_queue *q);
bool hisi_blk_ft_mq_complete_rq_redirection(struct request *rq, bool succ_done);

#endif /* __HISI_BLK_FT_H__ */
