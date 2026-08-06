#ifndef _NPU_MAIN_H
#define _NPU_MAIN_H

#include <linux/cdev.h>
#include <linux/io.h>

#define NPU_NAME         "ipu"

#define SOFT_RST_ACK        (0x1)
#define NOC_POWER_IDLEREQ_0 (0x02000200)
#define NOC_POWER_IDLEREQ_1 (0x10001000)

#define CONFIG_NOC_POWER_IDLEACK_0_BIT9  (0x200)
#define CONFIG_NOC_POWER_IDLE_0_BIT9     (0x200)

#define CONFIG_NOC_POWER_IDLEACK_0_BIT12 (0x1000)
#define CONFIG_NOC_POWER_IDLE_0_BIT12    (0x1000)

#define CONFIG_PCTRL_PERI_STAT3_BIT22   (0x400000)
#define CONFIG_PCTRL_PERI_STAT3_BIT23   (0x800000)

#define CONFIG_MEDIA1_REG_GT_CLK_RET       (0xe)
#define CONFIG_MEDIA1_REG_IP_RESETBIT      (0x3c)
#define CONFIG_MEDIA1_REG_CLKDIV15_DISABLE (0x04000000)
#define CONFIG_MEDIA1_REG_CLKDIV15_ENABLE  (0x04000400)

#define CONFIG_MEDIA2_REG_IP_RESETNOC   (0xc00)
#define CONFIG_MEDIA2_REG_GT_CLK_NOC    (0x180000)
#define CONFIG_MEDIA2_REG_PERRSTEN0_V200     (0x1038)
#define CONFIG_MEDIA2_REG_PERRSTEN0_V150     (0x38)
#define CONFIG_MEDIA2_REG_PERRSTDIS0_V200     (0x1038)
#define CONFIG_MEDIA2_REG_PERRSTDIS0_V150     (0x38)

#define CONFIG_MEDIA2_REG_PERDIS0       (0x7)
#define CONFIG_MEDIA2_REG_PEREN0        (0x7)
#define CONFIG_NOC_NPU_POWER_IDLEREQ_DIS_0 (0x02000000)
#define CONFIG_NOC_NPU_POWER_IDLEREQ_DIS_1 (0x10000000)

#define CONFIG_SC_GT_CLK_NPU_DIS        (0x40000000)
#define CONFIG_SC_GT_CLK_NPU_EN         (0x40004000)

#define PERF_CNT_CLK_GT_ENABLE  (0x1)
#define PERF_CNT_CLK_GT_CLOSE   (0x0)
#define PERF_CNT_CLEAR          (0x1)
#define FRAME_START_CNT_CLEAR   (0x1)
#define FRAME_FINISH_CNT_CLEAR  (0x10)

/* npu configure register offset */
#define NPU_IRQ_CLEAR_IRQ_NS (0x00000001)
#define NPU_IRQ_UNMASK_NO_SECURITY (0x00000000)
#define NPU_IRQ_MASK_NO_SECURITY   (0x00000001)
#define NPU_IRQ_CLEAR_IRQ_LEVEL1_NS (0x2)

#define CTRL_LOCK(coreid)  \
    do { \
        mutex_lock(&adapter->core[coreid].ctrl_mutex); \
    } while(0)

#define CTRL_UNLOCK(coreid)  \
    do { \
        mutex_unlock(&adapter->core[coreid].ctrl_mutex); \
    } while(0)

#define MODE_LOCK(coreid)  \
    do { \
        mutex_lock(&adapter->core[coreid].mode_mutex); \
    } while(0)

#define MODE_UNLOCK(coreid)  \
    do { \
        mutex_unlock(&adapter->core[coreid].mode_mutex); \
    } while(0)

#define COMMON_LOCK(coreid)  \
    do { \
        mutex_lock(&adapter->core[coreid].mode_mutex); \
        mutex_lock(&adapter->core[coreid].ctrl_mutex); \
    } while(0)

#define COMMON_UNLOCK(coreid)  \
    do { \
        mutex_unlock(&adapter->core[coreid].ctrl_mutex); \
        mutex_unlock(&adapter->core[coreid].mode_mutex); \
    } while(0)

typedef enum {
    RESET_UNSUPPORT = 0,
    RESET_BY_CONFIG_NOC_BUS,
    SOFT_RESET,
} RESET_TRATEGY_E;

typedef enum {
    NON_SECURETY = 0,
    SECURETY,
} RUN_MODE_E;

typedef struct {
    unsigned int core_num;
    unsigned int is_selfstart;
    unsigned int level1_irq;
    unsigned int performance_monitor;
    unsigned int lpm3_set_vcodecbus;
    unsigned int npu_reset_when_in_error;
    unsigned int npu_bandwidth_lmt;
} FEATURE_S;

typedef struct {
    void __iomem *io_addr;
} PMCTRL_REG_S;

typedef struct {
    void __iomem *io_addr;
} PCTRL_REG_S;

typedef struct {
    void __iomem *io_addr;
} MEDIA2_REG_S;

typedef struct {
    void __iomem *io_addr;
} MEDIA1_REG_S;

typedef struct {
    void __iomem *io_addr;
} DMSS_REG_S;

typedef struct {
    void __iomem *io_addr;
} PERI_REG_S;

typedef struct {
    bool npu_opened;
    FEATURE_S feature;

    unsigned int npu_major;
    unsigned int npu_minor;

    struct cdev cdev; /* char device */
    struct class *dev_class;
    struct device *device;

    /*for reset npu device*/
    PMCTRL_REG_S pmctrl_reg;
    PCTRL_REG_S  pctrl_reg;
    MEDIA2_REG_S media2_reg;
    PERI_REG_S   peri_reg;
    MEDIA1_REG_S media1_reg;
    DMSS_REG_S   dmss;
}NPU_MAIN_PRIVATE_COMMON_S;

typedef struct {
    void __iomem *io_addr;
    unsigned int base_addr;
} NPU_IRQ_REG_S;

typedef struct {
    RUN_MODE_E security_mode;
    unsigned int irq;                      /*irq num*/
    struct mutex ctrl_mutex;
    struct mutex mode_mutex;
    struct wake_lock wakelock;
    NPU_IRQ_REG_S npu_irq;
} NPU_MAIN_PRIVATE_CORE_S;

typedef struct {
    struct device *dev;
} NPU_PM_RUNTIME_S;

/* npu private data */
typedef struct {
    NPU_MAIN_PRIVATE_COMMON_S common;
    NPU_MAIN_PRIVATE_CORE_S   core[MAX_SUPPORT_CORE_NUM];    /*core*/
} NPU_MAIN_PRIVATE_S;

#endif

