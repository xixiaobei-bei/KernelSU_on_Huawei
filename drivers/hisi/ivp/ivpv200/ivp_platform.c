#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/uaccess.h>
#include "ivp_log.h"
#include "ivp_platform.h"
#include "ivp_reg.h"
#include "ivp_atf_subsys.h"
#include "ivp_sec.h"


#define REMAP_ADD                        (0xe8d00000)
#define DEAD_FLAG                        (0xdeadbeef)
#define SIZE_16K                         (16 * 1024)


extern void ivp_reg_write(unsigned int off, u32 val);
extern u32 ivp_reg_read(unsigned int off);
extern struct ivp_sec_device ivp_sec_dev;
extern struct ivp_sec_ion_s *ivp_secmem_ion;
static void *iram = NULL;

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


    for(i = 3; i < ivp_devp->sect_count; i++) {
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
    ivp_dbg("ivp_addr:0x%08X, len:0x%X, ddr_addr:0x%08X", ivp_addr / SIZE_1MB, len, (u32) (ddr_addr / SIZE_1MB));
    ivp_reg_write(ADDR_IVP_CFG_SEC_REG_START_REMAP_ADDR, ivp_addr / SIZE_1MB);
    ivp_reg_write(ADDR_IVP_CFG_SEC_REG_REMAP_LENGTH, len);
    ivp_reg_write(ADDR_IVP_CFG_SEC_REG_DDR_REMAP_ADDR, (u32) (ddr_addr / SIZE_1MB));
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

static int ivp_get_secure_attribute(struct platform_device *pdev, struct ivp_device *ivp_devp)
{
    int ret = 0;
    unsigned int sec_support = 0;

    if (pdev == NULL || ivp_devp == NULL) {
        ivp_err("pointer is NULL.");
        return -EINVAL;
    }
    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_SEC_SUPPORT, &sec_support);
    if (ret) {
        ivp_err("get ivp sec support flag fail, ret:%d", ret);
        return -EINVAL;
    }

    ivp_devp->ivp_sec_support = sec_support;
    ivp_info("get ivp sec support flag :%d", sec_support);

    return ret;
}

static void ivp_secure_config(void)
{
    ivpatf_change_slv_secmod(IVP_SEC);
    ivpatf_change_mst_secmod(IVP_SEC);
}


int ivp_poweron_pri(struct ivp_device *ivp_devp)
{
    int ret = 0;

    //0.Enable the power
    ret = regulator_enable(ivp_devp->ivp_media1_regulator);
    if (ret) {
        ivp_err("regularot enable failed [%d]!", ret);
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

    ivp_reg_write(0x0290, 0x02600260);
    ivp_reg_write(0x0294, 0x00000260);
    ivp_reg_write(0x0338, 0x00006A69);

    if (SECURE_MODE == ivp_devp->ivp_secmode)
    {
        ivp_secure_config();
    }

    return ret;

ERR_REGULATOR_ENABLE_IVP:
    clk_disable_unprepare(ivp_devp->clk);

ERR_CLK_PREPARE_ENABLE:
    ret |= clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->lowtemp_clk_rate);
    if (ret != 0) {
        ivp_err("err set lowfrq rate %#x fail(%d)",
                ivp_devp->lowtemp_clk_rate, ret);
    }

ERR_CLK_SET_RATE:
    ret |= regulator_disable(ivp_devp->ivp_media1_regulator);
    if (ret) {
        ivp_err("err regularot disable failed [%d]!", ret);
    }

    return ret;
}


int ivp_poweron_remap(struct ivp_device *ivp_devp)
{
    int ret = 0;

    if (SECURE_MODE == ivp_devp->ivp_secmode)
    {
        ret = ivp_alloc_secbuff(IVP_SEC_BUFF_SIZE);

        if(ret){
            ivp_err("ivp alloc secbuff failed!");
            return ret;
        }

        ret = ivpatf_poweron_remap_secmod(ivp_devp->sects[3].ivp_addr,
                                  IVP_SEC_BUFF_SIZE,
                                  ivp_secmem_ion->sec_phymem_addr);
    }
    else
    {
        ret = ivp_remap_addr_ivp2ddr(ivp_devp->sects[3].ivp_addr,
                                     ivp_devp->ivp_meminddr_len,
                                     ivp_devp->sects[3].acpu_addr << IVP_MMAP_SHIFT);

    }

    if (ret) {
        ivp_err("remap addr failed [%d]!", ret);
        return ret;
    }

    return ret;
}

int ivp_poweroff_pri(struct ivp_device *ivp_devp)
{
    int ret = 0;

    if (SECURE_MODE == ivp_devp->ivp_secmode)
    {
        ivp_free_secbuff();
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

    ret = regulator_disable(ivp_devp->ivp_media1_regulator);
    if (ret) {
        ivp_err("Power off failed [%d]!", ret);
    }


    return ret;
}

static int ivp_setup_regulator(struct platform_device *pdev,
                                          struct ivp_device *ivp_devp)
{
    struct regulator *ivp_media1_regulator = NULL;
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

    ivp_media1_regulator = devm_regulator_get(&pdev->dev, IVP_MEDIA_REGULATOR);
    if (IS_ERR(ivp_media1_regulator)) {
        ret = -ENODEV;
        ivp_err("Get ivp media regulator failed, ret:%d", ret);
        return ret;

    } else {
        ivp_devp->ivp_media1_regulator = ivp_media1_regulator;
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
        ivp_err("get rate failed, ret:%d", ret);
        return -ENOMEM;
    }
    ivp_devp->clk_rate = clk_rate;
    ivp_info("get clk rate: %u", clk_rate);

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_MIDDLE_CLK_RATE_NAME, &clk_rate);
    if (ret) {
        ivp_err("get rate failed, ret:%d", ret);
        return -ENOMEM;
    }
    ivp_devp->middle_clk_rate = clk_rate;
    ivp_info("get middle freq clk rate: %u", clk_rate);

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_LOW_CLK_RATE_NAME, &clk_rate);
    if (ret) {
        ivp_err("get rate failed, ret:%d", ret);
        return -ENOMEM;
    }
    ivp_devp->low_clk_rate = clk_rate;
    ivp_info("get low freq clk rate: %u", clk_rate);

    ret = of_property_read_u32(pdev->dev.of_node, OF_IVP_LOWFREQ_CLK_RATE_NAME, &clk_rate);
    if (ret) {
        ivp_err("get rate failed, ret:%d", ret);
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

int ivp_change_clk(struct ivp_device *ivp_devp)
{
    int ret = 0;

    switch (ivp_devp->clk_level) {
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

    ivp_info("try set core freq to: %ld", (unsigned long)ivp_devp->low_clk_rate);
    ret = clk_set_rate(ivp_devp->clk, (unsigned long)ivp_devp->low_clk_rate);
    if (ret != 0) {
        ivp_err("set low rate %#x fail, ret:%d", ivp_devp->low_clk_rate, ret);
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

    ret = ivp_get_secure_attribute(pdev, ivp_devp);
    if (ret) {
        ivp_err("get_secure_attribute failed, ret:%d", ret);
        return ret;
    }
    //create kthread should call at last
    if (ivp_devp->ivp_sec_support)
    {
        ret = ivp_create_secimage_thread(ivp_devp);
        if (ret) {
            ivp_err("ivp_create_secimage_thread failed, ret:%d", ret);
            return ret;
        }
    }

    return ret;
}

void ivp_deinit_pri(struct ivp_device *ivp_devp)
{
    ivp_free_memory_section(ivp_devp);
    if (ivp_devp->ivp_sec_support)
    {
        if (ivp_destroy_secimage_thread(ivp_devp)) {
            ivp_err("ivp_destroy_secimage_thread failed!");
        }
    }
}

int ivp_init_resethandler(struct ivp_device *pdev)
{
    if (SECURE_MODE == pdev->ivp_secmode){
        atomic_set(&ivp_sec_dev.ivp_image_success, 0);
    }
    else
    {
        /* init code to remap address */
        iram = ioremap(REMAP_ADD, SIZE_16K);
        if (!iram) {
            ivp_err("Can't map ivp base address");
            return -1;
        }

        iowrite32(DEAD_FLAG, iram);
    }

    return 0;
}

int ivp_check_resethandler(struct ivp_device *pdev)
{
    int inited = 0;
    uint32_t flag = 0;
    /* check init code in remap address */
    if (SECURE_MODE == pdev->ivp_secmode){
        if (1 == atomic_read(&ivp_sec_dev.ivp_image_success))
            inited = 1;

    }
    else
    {
        if(iram)
            flag = ioread32(iram);
        if (flag != DEAD_FLAG)
            inited = 1;
    }

    return inited;
}

void ivp_deinit_resethandler(struct ivp_device *pdev)
{

    if (SECURE_MODE == pdev->ivp_secmode){
        atomic_set(&ivp_sec_dev.ivp_image_success, 0);
    }
    else
    {
        /* deinit remap address */
        if(iram) {
            iounmap(iram);
            iram = NULL;
        }
    }
}

int ivp_sec_loadimage(struct ivp_device *pdev)
{

    return ivp_sec_load();
}

