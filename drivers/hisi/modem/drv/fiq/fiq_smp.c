#include <linux/init.h>
#include <linux/printk.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/delay.h>
#include <bsp_fiq.h>
#include <bsp_sysctrl.h>
/*lint -save -e754 -e550 -e528*/
struct per_fiq{
	void *               sysctrl_fiq_enable_reg;
	unsigned int         sysctrl_fiq_enable_bit;
	void *               sysctrl_fiq_status_reg;
	unsigned int         sysctrl_fiq_status_bit;
	//void *               sysctrl_fiq_clear_reg;/*lint !e754 */
	//unsigned int         sysctrl_fiq_clear_bit;/*lint !e754 */
	void *               smem_fiq_status_addr;
	void *               smem_fiq_clear_addr;
	void *               smem_send_cnt_addr;
	void *               smem_recive_cnt_addr;
	void *               smem_cp_respond_time_addr;
};

struct fiq_ctrl{
	unsigned int         fiq_init;
	unsigned int         chip_type;
	void *               smem_base_addr;
	void *               sysctrl_base_addr;
	struct per_fiq       per_cpu_fiq[CCPU_CORE_NUM];
 };
struct fiq_ctrl g_fiq_ctrl;

static void update_fiq_status(fiq_num fiq_num_t)
{
	u32 regval;
	u32 index_fiq;

	for(index_fiq = (u32)0; index_fiq < (u32)CCPU_CORE_NUM; index_fiq++)
	{
		/*更新FIQ的状态*/
		regval = (u32)readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_status_addr);/*lint !e838*/
		regval |= ((u32)0x1 << fiq_num_t);
		writel(regval,(volatile void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_status_addr);

		/*更新中断处理次数*/
		regval = (u32)readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_send_cnt_addr);
		regval += 1;
		writel(regval, (volatile  void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_send_cnt_addr);
	}
}

/*
*接口注意事项:
*对于多核FIQ控制寄存器不是一个地址时，无法做到同时发送，此函数需要分别配置寄存器
*/
static void send_fiq(void)
{
	u32 regval;
	u32 index_fiq;
	u32 bit_tmp = 0;

	for(index_fiq = (u32)0; index_fiq < (u32)CCPU_CORE_NUM; index_fiq++)
	{
		bit_tmp |= 0x1 << g_fiq_ctrl.per_cpu_fiq[index_fiq].sysctrl_fiq_enable_bit;
	}
	if(g_fiq_ctrl.chip_type) /*mbb*/
	{
		writel((u32)bit_tmp, (volatile void *)g_fiq_ctrl.per_cpu_fiq[0].sysctrl_fiq_enable_reg);
	}
	else /*PHONE*/
	{
		regval = (u32)readl((volatile const void *)(g_fiq_ctrl.per_cpu_fiq[0].sysctrl_fiq_enable_reg));
		regval &= (~((u32)bit_tmp));
		writel(regval,(volatile void *)g_fiq_ctrl.per_cpu_fiq[0].sysctrl_fiq_enable_reg);
	}
	pr_err("[fiq]fiq send timestamp = 0x%x\n",bsp_get_slice_value());
}
static int check_fiq_receive(void)
{
	u32 index_fiq;
	u32 result = (u32)1;
	udelay(2000);/*lint !e747 !e774 !e778*/
	pr_err("[fiq]check this time fiq status***\n");

	for(index_fiq = (u32)0; index_fiq < (u32)CCPU_CORE_NUM; index_fiq++)
	{
		/*check R8 recieve FIQ*/
		pr_err("[fiq%d]fiq send cnt = %d,fiq receive cnt = %d\n",index_fiq,readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_send_cnt_addr),readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_recive_cnt_addr));
		pr_err("[fiq%d]smem clear value = 0x%x,smem status value = 0x%x\n",index_fiq,readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_clear_addr),readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_status_addr));
		if(readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_clear_addr) == readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_status_addr))
		{
			result = (u32)0;
		}else
		{
			pr_err("[fiq%d]modem cpu%d fiq no handle\n",index_fiq,index_fiq);
		}
	}
	if(result)
	{
	 	return BSP_ERROR;
	}else
	{
	 	return BSP_OK;
	}
}


static void check_fiq_send(void)
{
	u32 index_fiq;
	pr_err("[fiq]check lost time fiq status***\n");
	for(index_fiq = (u32)0; index_fiq < (u32)CCPU_CORE_NUM; index_fiq++)
	{
		pr_err("[fiq%d]fiq status regval[%pK]=0x%x[bit%d]\n",index_fiq,(g_fiq_ctrl.per_cpu_fiq[index_fiq].sysctrl_fiq_status_reg), readl((volatile const void *)(g_fiq_ctrl.per_cpu_fiq[index_fiq].sysctrl_fiq_status_reg)), g_fiq_ctrl.per_cpu_fiq[index_fiq].sysctrl_fiq_status_bit);
		pr_err("[fiq%d]fiq send cnt = %d,fiq receive cnt = %d\n",index_fiq,readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_send_cnt_addr),readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_recive_cnt_addr));
		pr_err("[fiq%d]smem clear value = 0x%x,smem status value = 0x%x\n",index_fiq,readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_clear_addr),readl((volatile const void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_status_addr));
		
		writel(0x0,(volatile  void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_clear_addr);
		writel(0x0,(volatile  void *)g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_status_addr);
	}
}
int bsp_send_cp_fiq(fiq_num fiq_num_t)
{
	if (!g_fiq_ctrl.fiq_init)
	{
		pr_err("[fiq]fiq no init,send_cp_fiq too early\n");
		return BSP_ERROR;
	}
	if(fiq_num_t >= FIQ_MAX)
	{
		pr_err("[fiq]fiq_num = %d error\n",fiq_num_t);
		return BSP_ERROR;
	}
	/*检查上一次发送的情况*/
	check_fiq_send();

	/*更新此次发送的状态*/
	update_fiq_status(fiq_num_t);

	/*发送FIQ*/
	send_fiq();

	/*检查此次接收情况*/
	return check_fiq_receive();
}

int fiq_init(void)
{
	struct device_node *node;
	u32 tmp;
	struct device_node temp_node;
	struct device_node *child_node = &temp_node;
	u32 index_fiq =0;
        /*coverity[secure_coding]*/
	(void)memset((void *)&g_fiq_ctrl,  0, sizeof(struct fiq_ctrl));
	g_fiq_ctrl.smem_base_addr = (void *)SHM_FIQ_BASE_ADDR;
        /*coverity[secure_coding]*/
	(void)memset(g_fiq_ctrl.smem_base_addr,  0, (unsigned long)SHM_SIZE_CCORE_FIQ);
	node = of_find_compatible_node(NULL, NULL, "hisilicon,fiq");
	if (!node)
	{
		pr_err("[fiq]get hisilicon,fiq fail!\n");
		return -1;
	}
	g_fiq_ctrl.sysctrl_base_addr = of_iomap(node, 0);
	if (!g_fiq_ctrl.sysctrl_base_addr)
	{
		pr_err("[fiq]of iomap fail\n");
		return -1;
	}
	if(of_property_read_u32_array(node, "chip_type", &g_fiq_ctrl.chip_type, (unsigned long)1))
	{
		pr_err("[fiq]hisilicon,fiq chip_type dts node not found!\n");
		return -1;
	}
	
	for_each_child_of_node(node, child_node)/*lint !e838*/
	{
		if(of_property_read_u32_array(child_node, "index", &index_fiq, (unsigned long)1))
		{
			pr_err("[fiq%d]hisilicon,fiq index dts node not found!\n",index_fiq);
			return -1;
		}
		if(of_property_read_u32_array(child_node, "fiq_enable_offset", &tmp, (unsigned long)1))
		{
			pr_err("[fiq%d]hisilicon,fiq fiq_enable_offset dts node not found!\n",index_fiq);
			return -1;
		}
		g_fiq_ctrl.per_cpu_fiq[index_fiq].sysctrl_fiq_enable_reg = (void *)((unsigned long)g_fiq_ctrl.sysctrl_base_addr + (unsigned long)tmp);
		
		if(of_property_read_u32_array(child_node, "fiq_enable_bit", &g_fiq_ctrl.per_cpu_fiq[index_fiq].sysctrl_fiq_enable_bit, (unsigned long)1))
		{
			pr_err("[fiq%d]hisilicon,fiq fiq_enable_bit dts node not found!\n",index_fiq);
			return -1;
		}
		
		if(of_property_read_u32_array(child_node, "fiq_status_offset", &tmp, (unsigned long)1))
		{
			pr_err("[fiq%d]hisilicon,fiq fiq_clear_reg dts node not found!\n",index_fiq);
			return -1;
		}
		g_fiq_ctrl.per_cpu_fiq[index_fiq].sysctrl_fiq_status_reg =(void *)((unsigned long)g_fiq_ctrl.sysctrl_base_addr + (unsigned long)tmp);
		
		if(of_property_read_u32_array(child_node, "fiq_status_bit", &g_fiq_ctrl.per_cpu_fiq[index_fiq].sysctrl_fiq_status_bit, (unsigned long)1))
		{
			pr_err("[fiq%d]hisilicon,fiq fiq_clear_reg dts node not found!\n",index_fiq);
			return -1;
		}
		g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_clear_addr				 = (void *)((unsigned long)SHM_FIQ_CLEAR_ADDR + (unsigned long)(0x4 * index_fiq));/*lint !e647*/
		g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_fiq_status_addr 			     = (void *)((unsigned long)SHM_FIQ_STATUS_ADDR + (unsigned long)(0x4 * index_fiq));/*lint !e647*/
		g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_cp_respond_time_addr          = (void *)((unsigned long)SHM_FIQ_CP_TIMER_ADDR + (unsigned long)(0x4 * index_fiq));/*lint !e647*/
		g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_send_cnt_addr			     = (void *)((unsigned long)SHM_FIQ_TOTAL_SEND_CNT + (unsigned long)(0x4 * index_fiq));/*lint !e647*/
		g_fiq_ctrl.per_cpu_fiq[index_fiq].smem_recive_cnt_addr 		         = (void *)((unsigned long)SHM_FIQ_TOTAL_RECIVE_CNT + (unsigned long)(0x4 * index_fiq));/*lint !e647*/
	}
	writel(0xFFFFFFFF, (volatile  void *)SHM_FIQ_BARRIER);

	g_fiq_ctrl.fiq_init = 0x1;
	pr_err("[fiq]init OK\n");
	return 0;
}
/*lint --e{528} */
arch_initcall(fiq_init);
EXPORT_SYMBOL(bsp_send_cp_fiq);            /*lint !e19 */

/*lint -restore +e754 +e550 +e528*/

