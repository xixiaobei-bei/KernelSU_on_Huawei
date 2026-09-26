enum {
	HS_PMCTRL,
	HS_SYSCTRL,
	HS_CRGCTRL,
	HS_PMUCTRL,
	HS_PCTRL,
	HS_MEDIACRG,
	HS_IOMCUCRG,
	HS_MEDIA1CRG,
	HS_MEDIA2CRG,
	HS_MMC1CRG,
	HS_HSDTCRG,
	HS_MMC0CRG,
};
enum {
	HS_UNBLOCK_MODE,
	HS_BLOCK_MODE,
};

#define LPM3_CMD_LEN			2

struct hs_clk {
	void __iomem	*pmctrl;
	void __iomem	*sctrl;
	void __iomem	*crgctrl;
	void __iomem	*pmuctrl;
	void __iomem	*pctrl;
	void __iomem	*mediacrg;
	void __iomem	*iomcucrg;
	void __iomem	*media1crg;
	void __iomem	*media2crg;
	void __iomem	*mmc1crg;
	void __iomem	*hsdtcrg;
	void __iomem	*mmc0crg;
	spinlock_t	lock;
};
extern struct hs_clk hs_clk;

#define DVFS_MAX_VOLT				3
#define DVFS_MAX_FREQ_NUM			3
#define DVFS_MAX_VOLT_NUM			4
#define DDR_HW_VOTE				1
#define PERI_AVS_LOOP_MAX			400
#define DDRC_MIN_CLK_ID			4
#define DDRC_MAX_CLK_ID			5

/*AVS DEFINE BEGIN*/
enum {
	AVS_ICS = 1,
	AVS_ICS2,
	AVS_ISP,
	/*DSS AVS 4 no need*/
	AVS_VENC= 5,
	AVS_VDEC,
	AVS_IVP,
	AVS_MAX_ID,
};
#define AVS_BITMASK_FLAG		28
#define SC_SCBAKDATA24_ADDR		0x46C
#define AVS_ENABLE_PLL		0
#define AVS_DISABLE_PLL		1
/*AVS DEFINE END*/

#ifndef CLOCK_MDEBUG_LEVEL
#define CLOCK_MDEBUG_LEVEL    0
#endif

#ifndef CLK_LOG_TAG
#define CLK_LOG_TAG	"clock"
#endif

#if CLOCK_MDEBUG_LEVEL
#define clk_log_dbg(fmt, ...) \
	pr_err("[%s]%s: " fmt "\n", CLK_LOG_TAG, __func__, ##__VA_ARGS__)
#else
#define clk_log_dbg(fmt, ...)
#endif

/***********DEFINE END*****************/
void __iomem *hs_clk_base(u32 ctrl);
void __iomem __init *hs_clk_get_base(struct device_node *np);
