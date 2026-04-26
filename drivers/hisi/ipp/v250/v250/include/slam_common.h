#ifndef _SLAMCOMMON_CS_H
#define _SLAMCOMMON_CS_H

#define SLAM_LAYER_MAX          (8)
#define SLAM_SCOREHIST_NUM      (63)
#define SLAM_SCORE_TH_RANGE     (94)
#define SLAM_KPT_NUM_RANGE      (94)
#define SLAM_PATTERN_RANGE      (256)

#define MATCHER_KPT_MAX          (2048)
#define CPMPARE_INDEX_NUM        (1600)
#define CPMPARE_REG_NUM          (1920)


#define SLAM_OUT_Y_SIZE          (992*744)
#define SLAM_OUT_DESC_SIZE       (80*MATCHER_KPT_MAX+0x8000)//(80*1600+0x8000) 80B(Freak) or 64B(Brief)
#define SLAM_OUT_SCORE_MAP_SIZE  SLAM_OUT_Y_SIZE
#define SLAM_OUT_GRID_STAT_SIZE  (1496)
#define SLAM_OUT_SCORE_HIST_SIZE (252)

#define RDR_OUT_DESC_SIZE        (96*MATCHER_KPT_MAX+0x8000)
#define CMP_IN_INDEX_NUM         RDR_OUT_DESC_SIZE
#define CMP_OUT_RESULT_SIZE      (4*CPMPARE_INDEX_NUM)

typedef enum _ipp_cmdlst_resource_share_e {
	IPP_CMD_RES_SHARE_SLAM = 0,
	IPP_CMD_RES_SHARE_RDR,
	IPP_CMD_RES_SHARE_CMP,
} ipp_cmdlst_resource_share_e;

typedef enum _cmdlst_frame_prio_e {
	CMD_PRIO_LOW  = 0,
	CMD_PRIO_HIGH = 1,
} cmdlst_frame_prio_e;

/* IPP IRQ */
//orb irq
#define IPP_SLAM_CVDR_VP_WR_EOF_Y            0
#define IPP_SLAM_CVDR_VP_WR_EOF_STAT         1
#define IPP_SLAM_CVDR_VP_WR_EOF_SCORE        2
#define IPP_SLAM_CVDR_VP_RD_EOF_Y            3
#define IPP_SLAM_IRQ_DONE                    4
#define IPP_SLAM_CVDR_VP_WR_EOF_CMDLST       5
#define IPP_SLAM_CVDR_VP_RD_EOF_CMDLST       6
#define IPP_SLAM_CVDR_VP_WR_SOF_Y            7
#define IPP_SLAM_CVDR_VP_WR_SOF_STAT         8
#define IPP_SLAM_CVDR_VP_WR_SOF_SCORE        9
#define IPP_SLAM_CVDR_VP_WR_DROPPED_Y       10
#define IPP_SLAM_CVDR_VP_WR_DROPPED_STAT    11
#define IPP_SLAM_CVDR_VP_WR_DROPPED_SCORE   12
#define IPP_SLAM_CVDR_VP_RD_SOF_Y           13

//rdr irq
#define IPP_RDR_IRQ_DONE                    0
#define IPP_RDR_CVDR_VP_RD_EOF_FP           1
#define IPP_RDR_CVDR_VP_WR_EOF_CMDLST       2
#define IPP_RDR_CVDR_VP_RD_EOF_CMDSLT       3
#define IPP_RDR_CVDR_VP_RD_SOF_FP           4

//cmp irq
#define IPP_CMP_IRQ_DONE                    0
#define IPP_CMP_CVDR_VP_RD_EOF_FP           1
#define IPP_CMP_CVDR_VP_WR_EOF_CMDLST       2
#define IPP_CMP_CVDR_VP_RD_EOF_CMDLST       3
#define IPP_CMP_CVDR_VP_RD_SOF_FP           4

//ipp cmdlst R8 irq
#define IPP_CMDLST_R8_IRQ_CHANNEL_0         0
#define IPP_CMDLST_R8_IRQ_CHANNEL_1         1
#define IPP_CMDLST_R8_IRQ_CHANNEL_2         2
#define IPP_CMDLST_R8_IRQ_CHANNEL_3         3
#define IPP_CMDLST_R8_IRQ_CHANNEL_4         4
#define IPP_CMDLST_R8_IRQ_CHANNEL_5         5
#define IPP_CMDLST_R8_IRQ_CHANNEL_6         6
#define IPP_CMDLST_R8_IRQ_CHANNEL_7         7

typedef struct _slam_stream_t {
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	unsigned int buf;
	pix_format_e format;
} slam_stream_t;

#endif
