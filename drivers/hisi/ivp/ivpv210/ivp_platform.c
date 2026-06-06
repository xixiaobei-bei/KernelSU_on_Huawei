#include <linux/of.h>
#include "ivp_log.h"
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include "ivp_platform.h"
#include "ivp_reg.h"
#include "ivp_rdr.h"

#define REMAP_ADD                        (0xe9100000)
#define DEAD_FLAG                        (0xdeadbeef)
#define SIZE_16K                         (16 * 1024)

#define IVP_CORE_RD_QOS_MODE   (0x000C)
#define IVP_CORE_RD_QOS_BANDWIDTH  (0x0010)
#define IVP_CORE_RD_QOS_SATURATION  (0x0014)
#define IVP_CORE_RD_QOS_EXTCONTROL  (0x0018)

#define IVP_CORE_WR_QOS_MODE               (IVP_CORE_RD_QOS_MODE+0x80)
#define IVP_CORE_WR_QOS_BANDWIDTH    (IVP_CORE_RD_QOS_BANDWIDTH+0x80)
#define IVP_CORE_WR_QOS_SATURATION   (IVP_CORE_RD_QOS_SATURATION+0x80)
#define IVP_CORE_WR_QOS_EXTCONTROL   (IVP_CORE_RD_QOS_EXTCONTROL+0x80)

#define IVP_IDMA_RD_QOS_MODE               (IVP_CORE_RD_QOS_MODE+0x100)
#define IVP_IDMA_RD_QOS_BANDWIDTH    (IVP_CORE_RD_QOS_BANDWIDTH+0x100)
#define IVP_IDMA_RD_QOS_SATURATION   (IVP_CORE_RD_QOS_SATURATION+0x100)
#define IVP_IDMA_RD_QOS_EXTCONTROL   (IVP_CORE_RD_QOS_EXTCONTROL+0x100)

#define IVP_IDMA_WR_QOS_MODE               (IVP_CORE_RD_QOS_MODE+0x180)
#define IVP_IDMA_WR_QOS_BANDWIDTH    (IVP_CORE_RD_QOS_BANDWIDTH+0x180)
#define IVP_IDMA_WR_QOS_SATURATION   (IVP_CORE_RD_QOS_SATURATION+0x180)
#define IVP_IDMA_WR_QOS_EXTCONTROL   (IVP_CORE_RD_QOS_EXTCONTROL+0x180)

#define IVP32_QOS_PRIORITY        (0x0208)
#define IVP32_QOS_MODE              (0x020C)

#define IVP_PLL0_TRANS_CLK       (406000000)
#define IVP_PLL3_TRANS_CLK       (300000000)


extern void ivp_reg_write(unsigned int off, u32 val);
extern u32 ivp_reg_read(unsigned int off);
void ivp_set_qos_cfg(struct ivp_device *dev);
int ivp_set_transitional_clk(struct ivp_device *ivp_devp,unsigned int pre_level, unsigned int cur_level);

static void *iram = NULL;

u32 noc_ivp_reg_read(struct ivp_device *ivp_dev, unsigned int off)
{
    char __iomem * reg = ivp_dev->io_res.noc_ivp_base_addr + off;
    u32 val = readl(reg);
    return val;
}

void noc_ivp_reg_write(struct ivp_device *ivp_dev, unsigned int off, u32 val)
{
    char __iomem * reg = ivp_dev->io_res.noc_ivp_base_addr + off;
    writel(val, reg);
}

static int ivp_get_memory_section(struct platform_device *pdev,
                                              struct ivp_device *ivp_devp)
{
    int i = 0;
    int ret = -1;
    unsigned int size = 0;
    dma_addr_t dma_addr = 0;

    if (pdev == NULL || ivp_devp == NULL) {
        ivp_err("pointer is NULL.");
        return -EINVAL;
    }

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_DYNAMIC_MEM, &size);
    if ((0 != ret) || (0 == size)) {
        ivp_err("get failed/not use dynamic mem, ret:%d", ret);
        return ret;
    }
    ivp_devp->dynamic_mem_size = size;

    ivp_devp->ivp_meminddr_len = size;

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_DYNAMIC_MEM_SEC_SIZE, &size);
    if ((0 != ret) || (0 == size)) {
        ivp_err("get failed/not use dynamic mem, ret:%d", ret);
        return ret;
    }
    ivp_devp->dynamic_mem_section_size = size;

    if ((ivp_devp->dynamic_mem_section_size * (ivp_devp->sect_count - 3)) != ivp_devp->dynamic_mem_size) {
        ivp_err("dynamic_mem should be sect_count-3 times dynamic_mem_section");
        return -EINVAL;
    }

    ivp_devp->vaddr_memory = NULL;
    /*lint -save -e598 -e648*/
    dma_set_mask_and_coherent(&ivp_devp->ivp_pdev->dev, DMA_BIT_MASK(64));
    /*lint -restore */
    if ((ivp_devp->vaddr_memory = dma_alloc_coherent(&ivp_devp->ivp_pdev->dev,
                    ivp_devp->dynamic_mem_size, &dma_addr, GFP_KERNEL)) == NULL) {
        ivp_err("[%s] ivp_get_vaddr.0x%pK\n", __func__, ivp_devp->vaddr_memory);
        return -EINVAL;
    }

    for (i = 3; i < ivp_devp->sect_count; i++) {
        if (i == 3) {
            ivp_devp->sects[i].acpu_addr = dma_addr >> 4;

        } else {
            ivp_devp->sects[i].acpu_addr = ((ivp_devp->sects[i-1].acpu_addr << 4)
                     + ivp_devp->sects[i-1].len) >> 4;
            ivp_devp->sects[i].ivp_addr = ivp_devp->sects[i-1].ivp_addr
                     + ivp_devp->sects[i-1].len ;
        }
        ivp_devp->sects[i].len = ivp_devp->dynamic_mem_section_size;
        ivp_dbg("________ivp sections 0x%pK\n", ivp_devp->sects[i].acpu_addr);
    }

    return 0;
}

static void ivp_free_memory_section(struct ivp_device *ivp_devp)
{
    dma_addr_t dma_addr = 0;
    dma_addr = ivp_devp->sects[3].acpu_addr << 4;

    if (ivp_devp->vaddr_memory != NULL) {
        dma_free_coherent(&ivp_devp->ivp_pdev->dev, ivp_devp->dynamic_mem_size,
                ivp_devp->vaddr_memory, dma_addr);
        ivp_devp->vaddr_memory = NULL;
    }
}

static inline void ivp_hw_remap_ivp2ddr(unsigned int ivp_addr,
                                                   unsigned int len,
                                                   unsigned long ddr_addr)
{
    unsigned int ivp_addr_base = ((ivp_addr/SIZE_1MB) << 16) + (ivp_addr/SIZE_1MB);
    unsigned int ddr_addr_base = ((ddr_addr/SIZE_1MB) << 16) + (ddr_addr/SIZE_1MB);
    unsigned int remap_len = (len << 8) + len;

    ivp_info("[%s]: Enter", __func__);
    ivp_reg_write(ADDR_IVP_CFG_SEC_REG_START_REMAP_ADDR, ivp_addr_base);
    ivp_reg_write(ADDR_IVP_CFG_SEC_REG_REMAP_LENGTH, remap_len);
    ivp_reg_write(ADDR_IVP_CFG_SEC_REG_DDR_REMAP_ADDR, (u32) ddr_addr_base);
}

static inline int ivp_remap_addr_ivp2ddr(unsigned int ivp_addr,
                                                     int len,
                                                     unsigned long ddr_addr)
{
    ivp_dbg("ivp_addr:%#pK, len:%#x, ddr_addr:%#pK", ivp_addr, len, ddr_addr);
    if ((ivp_addr & MASK_1MB) != 0 ||
        (ddr_addr & MASK_1MB) != 0 ||
        len >= 128 * SIZE_1MB) {
        ivp_err("not aligned");
        return -EINVAL;
    }
    len = (len + SIZE_1MB - 1) / SIZE_1MB - 1;
    ivp_hw_remap_ivp2ddr(ivp_addr, (u32)len, ddr_addr);

    return 0;
}

int ivp_poweron_pri(struct ivp_device *ivp_devp)
{
    int ret = 0;

    //0.Enable the power
    ret = regulator_enable(ivp_devp->ivp_media2_regulator);
    if (ret) {
        ivp_err("regulator enable failed [%d]!", ret);
        return ret;
    }

    //1.Set Clock rate
    ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->clk_rate);
    if (ret != 0) {
        ivp_err("set rate %#x fail, ret:%d", ivp_devp->clk_rate, ret);
        goto TRY_DOWN_FREQ;
    }

    //2.Enable the clock
    ret = clk_prepare_enable(ivp_devp->clk);
    if (ret ) {
        ivp_err("i2c2_clk :clk prepare enable failed,ret=%d ",ret);
        goto TRY_DOWN_FREQ;
    }

    if (0 == ret)
        goto NORMAL_FRQ_SUCESS;

TRY_DOWN_FREQ:
    ivp_info("try set core freq to: %ld", (unsigned long)ivp_devp->lowtemp_clk_rate);

    ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->lowtemp_clk_rate);
    if (ret != 0) {
        ivp_err("set low rate %#x fail, ret:%d", ivp_devp->clk_rate, ret);
        goto ERR_CLK_SET_RATE;
    }

    ret = clk_prepare_enable(ivp_devp->clk);
    if (ret ) {
        ivp_err("i2c2_clk :low clk prepare enable failed,ret=%d ", ret);
        goto ERR_CLK_PREPARE_ENABLE;
    }

NORMAL_FRQ_SUCESS:
    ivp_info("set core success to: %ld", clk_get_rate(ivp_devp->clk));

    //3.Enable the power
    ret = regulator_enable(ivp_devp->regulator);
    if (ret) {
        ivp_err("regularot enable failed [%d]!", ret);
        goto ERR_REGULATOR_ENABLE_IVP;
    }

    //config Micro-DMA and core MID£»
    ivp_reg_write(0x0338, 0x00696A74);

    ivp_set_qos_cfg(ivp_devp);

    return ret;

ERR_REGULATOR_ENABLE_IVP:
    clk_disable_unprepare(ivp_devp->clk);

ERR_CLK_PREPARE_ENABLE:
    ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->lowtemp_clk_rate);
    if (ret != 0) {
        ivp_err("err set lowfrq rate %#x fail(%d)",
                ivp_devp->lowtemp_clk_rate, ret);
    }

ERR_CLK_SET_RATE:
    ret = regulator_disable(ivp_devp->ivp_media2_regulator);
    if (ret) {
        ivp_err("err regularot disable failed [%d]!", ret);
    }

    return -1;
}


int ivp_poweron_remap(struct ivp_device *ivp_devp)
{
    int ret = 0;

    ret = ivp_remap_addr_ivp2ddr(ivp_devp->sects[3].ivp_addr,
                                 ivp_devp->ivp_meminddr_len,
                                 ivp_devp->sects[3].acpu_addr << IVP_MMAP_SHIFT);
    if (ret) {
        ivp_err("remap addr failed [%d]!", ret);
        return ret;
    }

    return ret;
}

int ivp_poweroff_pri(struct ivp_device *ivp_devp)
{
    int ret = 0;
    int i = 0;

    unsigned int waiti = ivp_reg_read(IVP_REG_OFF_PWAITMODE);
    while (((waiti&0x01) == 0)&&(i < 3)) {
        udelay(100);
        ivp_err("ivp core is not in wfi mode, 0x%x",waiti);
        waiti = ivp_reg_read(IVP_REG_OFF_PWAITMODE);
        i++;
    }

    ret = regulator_disable(ivp_devp->regulator);
    if (ret) {
        ivp_err("Power off failed [%d]!", ret);
    }

    clk_disable_unprepare(ivp_devp->clk);

    ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->lowfrq_pd_clk_rate);
    if (ret != 0) {
        ivp_warn("set lfrq pd rate %#x fail, ret:%d",
                 ivp_devp->lowfrq_pd_clk_rate, ret);
    }

    ret = regulator_disable(ivp_devp->ivp_media2_regulator);
    if (ret) {
        ivp_err("Power off failed [%d]!", ret);
    }


    return ret;
}

static int ivp_setup_regulator(struct platform_device *pdev,
                                          struct ivp_device *ivp_devp)
{
    struct regulator *ivp_media2_regulator = NULL;
    struct regulator *regulator = NULL;
    int ret = 0;

    regulator = devm_regulator_get(&pdev->dev, IVP_REGULATOR);
    if (IS_ERR(regulator)) {
        ret = -ENODEV;
        ivp_err("Get ivp regulator failed, ret:%d", ret);
        return ret;

    } else {
        ivp_devp->regulator = regulator;
    }

    ivp_media2_regulator = devm_regulator_get(&pdev->dev, IVP_MEDIA_REGULATOR);
    if (IS_ERR(ivp_media2_regulator)) {
        ret = -ENODEV;
        ivp_err("Get ivp media regulator failed, ret:%d", ret);
        return ret;

    } else {
        ivp_devp->ivp_media2_regulator = ivp_media2_regulator;
    }

    return ret;
}

static int ivp_setup_clk(struct platform_device *pdev,
                                struct ivp_device *ivp_devp)
{
    int ret = 0;
    u32 clk_rate = 0;

    ivp_devp->clk = devm_clk_get(&pdev->dev, OF_IVP_CLK_NAME);
    if (IS_ERR(ivp_devp->clk)) {
        ivp_err("get clk failed");
        return -ENODEV;
    }
    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_CLK_RATE_NAME, &clk_rate);
    if (ret) {
        ivp_err("get clk rate failed, ret:%d", ret);
        return -ENOMEM;
    }
    ivp_devp->clk_rate = clk_rate;
    ivp_info("get clk rate: %u", clk_rate);

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_MIDDLE_CLK_RATE_NAME, &clk_rate);
    if (ret) {
        ivp_err("get middle freq rate failed, ret:%d", ret);
        return -ENOMEM;
    }
    ivp_devp->middle_clk_rate = clk_rate;
    ivp_info("get middle freq clk rate: %u", clk_rate);

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_LOW_CLK_RATE_NAME, &clk_rate);
    if (ret) {
        ivp_err("get low freq rate failed, ret:%d", ret);
        return -ENOMEM;
    }
    ivp_devp->low_clk_rate = clk_rate;
    ivp_info("get low freq clk rate: %u", clk_rate);

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_ULTRA_LOW_CLK_RATE_NAME, &clk_rate);
    if (ret) {
        ivp_err("get ultra low freq clk rate, ret:%d", ret);
        return -ENOMEM;
    }
    ivp_devp->ultra_low_clk_rate = clk_rate;
    ivp_info("get ultra low freq clk rate: %u", clk_rate);

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_LOWFREQ_CLK_RATE_NAME, &clk_rate);
    if (ret) {
        ivp_err("get lowfreq pd clk rate failed, ret:%d", ret);
        return -ENOMEM;
    }
    ivp_devp->lowfrq_pd_clk_rate = clk_rate;
    ivp_info("get lowfrq pd clk rate: %u", clk_rate);

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_LOW_TEMP_RATE_NAME, &clk_rate);
    if (ret) {
        ivp_err("get rate failed, ret:%d", ret);
        return -ENOMEM;
    }
    ivp_devp->lowtemp_clk_rate = clk_rate;
    ivp_info("get low temperature clk rate: %u", clk_rate);

    return ret;
}

int ivp_change_clk(struct ivp_device *ivp_devp, unsigned int level)
{
    int ret = 0;

    ret = ivp_set_transitional_clk(ivp_devp,ivp_devp->clk_level, level);
    if (ret != 0) {
        ivp_err("set transitional freq fail, ret:%d", ret);
        return ret;
    }

    ivp_devp->clk_level = level;

    switch (ivp_devp->clk_level) {
    case IVP_CLK_LEVEL_ULTRA_LOW:
        ivp_info("ivp freq to ultra low level.");
        ivp_devp->clk_usrsetrate = ivp_devp->ultra_low_clk_rate;
        break;

    case IVP_CLK_LEVEL_LOW:
        ivp_info("ivp freq to low level.");
        ivp_devp->clk_usrsetrate = ivp_devp->low_clk_rate;
        break;

    case IVP_CLK_LEVEL_MEDIUM:
        ivp_info("ivp freq to media level.");
        ivp_devp->clk_usrsetrate = ivp_devp->middle_clk_rate;
        break;

    case IVP_CLK_LEVEL_HIGH:
        ivp_info("ivp freq to high level.");
        ivp_devp->clk_usrsetrate = ivp_devp->clk_rate ;
        break;

    case IVP_CLK_LEVEL_DEFAULT:
    default:
        ivp_info("use default freq.");
        ivp_devp->clk_usrsetrate = ivp_devp->clk_rate;
        break;
    }

    //Set Clock rate
    ivp_info("set clock rate.");
    ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->clk_usrsetrate);
    if (0 == ret) {
        ivp_info("set core success to: %ld", clk_get_rate(ivp_devp->clk));
        return ret;
    }

    ivp_info("try set core freq to: %ld", (unsigned long)ivp_devp->ultra_low_clk_rate);
    ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->ultra_low_clk_rate);
    if (ret != 0) {
        ivp_err("set low rate %#x fail, ret:%d", ivp_devp->ultra_low_clk_rate, ret);
    }

    return ret;
}

int ivp_init_pri(struct platform_device *pdev, struct ivp_device *ivp_devp)
{
    int ret = 0;

    ret = ivp_setup_regulator(pdev, ivp_devp);
    if (ret) {
        ivp_err("setup regulator failed, ret:%d", ret);
        return ret;
    }

    ret = ivp_setup_clk(pdev, ivp_devp);
    if (ret) {
        ivp_err("setup clk failed, ret:%d", ret);
        return ret;
    }

    ret = ivp_get_memory_section(pdev, ivp_devp);
    if (ret) {
        ivp_err("get memory section failed, ret:%d", ret);
        return ret;
    }

    ret = ivp_rdr_init(ivp_devp);
    if (ret) {
        ivp_err("rdr init  failed, ret:%d", ret);
        return ret;
    }

    return ret;
}

void ivp_deinit_pri(struct ivp_device *ivp_devp)
{
    ivp_free_memory_section(ivp_devp);
    ivp_rdr_deinit();

}

int ivp_init_resethandler(struct ivp_device *pdev)
{
    /* init code to remap address */
    iram = ioremap(REMAP_ADD, SIZE_16K);
    if (iram == NULL) {
        ivp_err("Can't map ivp base address");
        return -1;
    }

    iowrite32(DEAD_FLAG, iram);

    return 0;
}

int ivp_check_resethandler(struct ivp_device *pdev)
{
    /* check init code in remap address */
    int inited = 0;
    uint32_t flag = ioread32(iram);
    if (flag != DEAD_FLAG)
        inited = 1;

    return inited;
}

void ivp_deinit_resethandler(struct ivp_device *pdev)
{
    /* deinit remap address */
    if(iram != NULL) {
        iounmap(iram);
    }
}

int ivp_sec_loadimage(struct ivp_device *pdev)
{
    ivp_err("not support sec ivp!");
    return -EINVAL;
}

void ivp_dev_hwa_enable(void)
{
    /*enable apb gate clock , watdog ,timer*/
    ivp_info("ivp will enable hwa.");
    ivp_reg_write(IVP_REG_OFF_APB_GATE_CLOCK, 0x0000003F);
    ivp_reg_write(IVP_REG_OFF_TIMER_WDG_RST_DIS, 0x00000007);

    return;
}

void ivp_hw_enable_reset(struct ivp_device *devp)
{
    ivp_reg_write(IVP_REG_OFF_DSP_CORE_RESET_EN, 0x02);
    ivp_reg_write(IVP_REG_OFF_DSP_CORE_RESET_EN, 0x01);
    ivp_reg_write(IVP_REG_OFF_DSP_CORE_RESET_EN, 0x04);
}

void ivp_set_qos_cfg(struct ivp_device *dev)
{
    ivp_info("%s, enter", __func__);
    //ivp QoS reg
    ivp_reg_write(IVP_REG_OFF_IVP_SYSTEM_QOS_CFG, 0x30000);
    //noc ivp QoS reg
    noc_ivp_reg_write(dev, IVP_CORE_RD_QOS_MODE,            0x1);
    noc_ivp_reg_write(dev, IVP_CORE_RD_QOS_BANDWIDTH,  0x1000);
    noc_ivp_reg_write(dev, IVP_CORE_RD_QOS_SATURATION, 0x20);
    noc_ivp_reg_write(dev, IVP_CORE_RD_QOS_EXTCONTROL, 0x1);

    noc_ivp_reg_write(dev, IVP_CORE_WR_QOS_MODE,            0x1);
    noc_ivp_reg_write(dev, IVP_CORE_WR_QOS_BANDWIDTH,  0x1000);
    noc_ivp_reg_write(dev, IVP_CORE_WR_QOS_SATURATION, 0x20);
    noc_ivp_reg_write(dev, IVP_CORE_WR_QOS_EXTCONTROL, 0x1);

    noc_ivp_reg_write(dev, IVP_IDMA_RD_QOS_MODE,            0x1);
    noc_ivp_reg_write(dev, IVP_IDMA_RD_QOS_BANDWIDTH,  0x1000);
    noc_ivp_reg_write(dev, IVP_IDMA_RD_QOS_SATURATION, 0x20);
    noc_ivp_reg_write(dev, IVP_IDMA_RD_QOS_EXTCONTROL, 0x1);

    noc_ivp_reg_write(dev, IVP_IDMA_WR_QOS_MODE,            0x1);
    noc_ivp_reg_write(dev, IVP_IDMA_WR_QOS_BANDWIDTH,  0x1000);
    noc_ivp_reg_write(dev, IVP_IDMA_WR_QOS_SATURATION, 0x20);
    noc_ivp_reg_write(dev, IVP_IDMA_WR_QOS_EXTCONTROL, 0x1);

    noc_ivp_reg_write(dev, IVP32_QOS_PRIORITY,  0x0303);
    noc_ivp_reg_write(dev, IVP32_QOS_MODE,       0x0);

}

int ivp_set_transitional_clk(struct ivp_device *ivp_devp,unsigned int pre_level, unsigned int cur_level)
{
    int ret = 0;
    const unsigned int transitional_table[4][4] =
    {
        {0,  0, (IVP_PLL0_TRANS_CLK), (IVP_PLL3_TRANS_CLK)},
        {0,  0, (IVP_PLL0_TRANS_CLK), 0},
        {0,  0, 0,                    0},
        {0,  0, 0,                    0}
    };
    /*----------------------
               1   2     3     4
          pre 480 640   811   400
    cur
   480       x     x    406    300
   640       x     x    406     x
   811       x     x     x       x
   400       x     x     x       x
  ---------------------------------*/
    int row =  ((cur_level > 4)||(cur_level < 1)) ? IVP_CLK_LEVEL_HIGH : cur_level;
    int col = ((pre_level > 4)||(pre_level < 1)) ? IVP_CLK_LEVEL_HIGH : pre_level;

    unsigned int transitional_clk = transitional_table[row-1][col-1];
    if (transitional_clk) {
        ivp_info("need to change transitional_clk %u",transitional_clk);
        ret = clk_set_rate(ivp_devp->clk, transitional_clk);
        if (ret != 0) {
            ivp_err("set transitional_clk fail,ret = %d!", ret);
            return ret;
        }
    }

    return ret;
}


