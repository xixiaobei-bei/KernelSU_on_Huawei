/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub softtimer driver.
 * Author: Huawei
 * Create: 2016-03-15
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/semaphore.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/pm.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include "../common.h"
#include "hisi_softtimer.h"

#define HISI_SOFTTIME_FREQ                  (32*1024)
#define SOFTTIMER_MAX_LENGTH                (0xFFFFFFFF>>15)
#define ELAPESD_TIME_INVAILD                0xFFFFFFFF
#define TIMER_INIT_FLAG                     0X5A5A5A5A

#ifndef TIMER_TRUE
#define TIMER_TRUE                          1
#endif
#ifndef TIMER_FALSE
#define TIMER_FALSE                         0
#endif
#define TIMER_MASK                         0x80
#define TIMER_PARAMETER_MULTIPLE           1000
#define TIMER_MASK1                         0xA2

struct softtimer_ctrl {
	struct list_head        timer_list_head;
	unsigned int            softtimer_start_value;
	unsigned int            irqnum;
	void __iomem            *hard_timer_addr;
	spinlock_t              soft_timer_lock;
	struct tasklet_struct   softtimer_tasklet;
	unsigned int            rate;
	unsigned int            init_flag;
	struct clk              *clk;
	unsigned int            clk_is_enable;
	unsigned int            is_in_interrupt;
	};

static struct softtimer_ctrl timer_control;
/*lint  -e438 -e550 -e732 -e838*/
/*
 * 获取timer的数值
 */
static __inline__ unsigned int  timer_get_value(void)
{
	unsigned int ret = 0;

	ret = readl(timer_control.hard_timer_addr + TIMERCTRL(0));
	if (ret & TIMER_MASK) {
		ret = readl(timer_control.hard_timer_addr + TIMERVALUE(0));
		if (ret > timer_control.softtimer_start_value) {
			return timer_control.softtimer_start_value;
		} else {
			return ret;
		}
	}  else {
		return 0;
	}
}

/*
 * 开启硬件timer
 */
static void start_hard_timer(unsigned int ulvalue)
{
	unsigned int ret;

	pr_err("[%s] softtimer:start_hard_timer enter [%u] \n", __func__, ulvalue);
	timer_control.softtimer_start_value = ulvalue;
	ret = readl(timer_control.hard_timer_addr + TIMERCTRL(0));
	writel(ret&(~TIMER_MASK), timer_control.hard_timer_addr + TIMERCTRL(0));
	writel(ulvalue, timer_control.hard_timer_addr + TIMERLOAD(0));
	ret = readl(timer_control.hard_timer_addr + TIMERCTRL(0));
	writel(ret | TIMER_MASK1, timer_control.hard_timer_addr + TIMERCTRL(0));
}

/*
 * 停止硬件timer
 */
static void stop_hard_timer(void)
{
	unsigned int ret;
	ret = readl(timer_control.hard_timer_addr+TIMERCTRL(0));
	writel(ret&(~TIMER_MASK), timer_control.hard_timer_addr+TIMERCTRL(0));
	timer_control.softtimer_start_value = ELAPESD_TIME_INVAILD;
}
/*lint  +e438 +e550 +e732 +e838*/
/*lint  -e834 -e838*/
 /*
 * 硬计时器计时当前时间
 */
static unsigned int hard_timer_elapsed_time(void)
{
	unsigned int ul_temp_value = 0;
	if (ELAPESD_TIME_INVAILD == timer_control.softtimer_start_value) {
		return 0;
	}
	ul_temp_value = timer_get_value();
	return ((timer_control.softtimer_start_value < ul_temp_value) ? 0 :
			(timer_control.softtimer_start_value - ul_temp_value+1));
}
/*lint  +e834 +e838*/
/*lint  -e826 -e64*/
/*
 * 触发一个softtimer
 */
void hisi_softtimer_add(struct softtimer_list *timer)
{
	unsigned int elapsed_time;
	struct softtimer_list *p = NULL;

	if ((timer == NULL) || (timer_control.init_flag != TIMER_TRUE))  {
		return ;
	}

	spin_lock_bh(&timer_control.soft_timer_lock);
	elapsed_time = hard_timer_elapsed_time();
	pr_info("softtimer:%s ENTER [%d][0x%x]\n", __func__, timer->timeout, elapsed_time);
	/*if timer is added previous*/
	if (!list_empty(&timer->entry)) {
		spin_unlock_bh(&timer_control.soft_timer_lock);
		return ;
	}
	timer->timeout = timer->count_num;
	timer->timeout += elapsed_time;

	/*if timer list is empty*/
	if (list_empty(&(timer_control.timer_list_head))) {
		list_add_tail(&(timer->entry), &(timer_control.timer_list_head));
	} else {
		/*if not empty , added to timer list by timeout value*/
		p = list_first_entry(&(timer_control.timer_list_head), struct softtimer_list, entry);
		if ((TIMER_TRUE == p->is_running) && (0 == elapsed_time))    {
			timer->timeout += p->timeout;
		}
		if (elapsed_time != 0)   {
			timer->timeout -= 1;
			elapsed_time     -= 1;
		}
		list_for_each_entry(p, &(timer_control.timer_list_head), entry)  {
			if (p->timeout <= timer->timeout)     {
				timer->timeout -= p->timeout;
			}  else   {
				break;
			}
		}
		if (&(p->entry) != &(timer_control.timer_list_head))  {
			p->timeout -= timer->timeout;
		}
		list_add_tail(&(timer->entry), &(p->entry));
	}
	/*if the timer is first at list*/
	if (timer_control.timer_list_head.next == &(timer->entry)) {
		timer->timeout -= elapsed_time;
		if ((timer->entry.next) != (&(timer_control.timer_list_head))) {
			p = list_entry(timer->entry.next, struct softtimer_list, entry);
			if (TIMER_TRUE == p->is_running) {
				p->is_running = TIMER_FALSE;
			}
		}
		timer->is_running = TIMER_TRUE;
		start_hard_timer(timer->timeout);
	}
	spin_unlock_bh(&timer_control.soft_timer_lock);
}

/*
 * 软计时器删除
 */
static int hisi_softtimer_try_to_del_sync(struct softtimer_list *timer)
{
	struct softtimer_list *p = NULL;

	spin_lock_bh(&timer_control.soft_timer_lock);
	if (timer_control.is_in_interrupt) {
		spin_unlock_bh(&timer_control.soft_timer_lock);
		return -EACCES;
	}

	if (list_empty(&timer->entry))    {
		spin_unlock_bh(&timer_control.soft_timer_lock);
		return 0;
	}
	/*if the last one,just delete it*/
	if (timer->entry.next == &(timer_control.timer_list_head))        {
		timer->is_running = TIMER_FALSE;
		list_del_init(&(timer->entry));
	} else if ((timer->entry.prev == &(timer_control.timer_list_head))
	&& (timer->entry.next != &(timer_control.timer_list_head))) {
		/*if it is the first one,and timer list have other node*/
		timer->is_running = TIMER_FALSE;
		list_del_init(&(timer->entry));
		p = list_first_entry(&(timer_control.timer_list_head), struct softtimer_list, entry);
		p->timeout += timer->timeout - hard_timer_elapsed_time();
		start_hard_timer(p->timeout);
		p->is_running = TIMER_TRUE;
	} else {
		/*if it is middle node*/
		p = list_entry(timer->entry.next, struct softtimer_list, entry);
		p->timeout += timer->timeout;
		timer->is_running = TIMER_FALSE;
		list_del_init(&(timer->entry));
	}
	/*if timer list is empty after delete node*/
	if (list_empty(&(timer_control.timer_list_head))) {
		stop_hard_timer();
	}
	spin_unlock_bh(&timer_control.soft_timer_lock);
	return 0;
}
/*
 * 删除一个softtimer
 */
int hisi_softtimer_delete(struct softtimer_list *timer)
{
	if (NULL == timer)    {
		return  -EIO;
	}
	for (;;) {
		int ret = hisi_softtimer_try_to_del_sync(timer);
		if (ret == 0) {
			return ret;
		}
		cpu_relax();
	}
}
/*lint  +e826 +e64*/
/*
 * 创建软计时器
 */
int hisi_softtimer_create(struct softtimer_list *sft_info, softtimer_func func,
unsigned long para, unsigned int timeout)
{
	unsigned long delaytime;
	if (NULL == sft_info || NULL == func) {
		return -EPERM;
	}

	INIT_LIST_HEAD(&(sft_info->entry));
	sft_info->is_running = TIMER_FALSE;
	sft_info->init_flags = TIMER_INIT_FLAG;
	sft_info->func = func ;
	sft_info->para = para ;
	/*CHECK INPUT PARAMETER*/
	if (timeout > SOFTTIMER_MAX_LENGTH * TIMER_PARAMETER_MULTIPLE) {
		pr_err("[%s] softtimer:timeout err\n", __func__); /* [false alarm]:fortify */
		return -EIO;
	}
	delaytime = (timer_control.rate  * (unsigned long)timeout)/TIMER_PARAMETER_MULTIPLE;
	sft_info->timeout = (unsigned int)delaytime;
	sft_info->count_num = sft_info->timeout;
	return 0;
}

/*
 * 修改软计时器
 */
int hisi_softtimer_modify(struct softtimer_list *timer, unsigned int new_expire_time)
{
	unsigned long delaytime;

	if ((NULL == timer) || (!list_empty(&timer->entry)))    {
		return -EIO;
	}
	if (new_expire_time > SOFTTIMER_MAX_LENGTH * TIMER_PARAMETER_MULTIPLE)    {
		pr_err("[%s] softtimer:timeout err.\n", __func__);  /* [false alarm]:fortify */
		return -EIO;
	}
	delaytime = (timer_control.rate * (unsigned long)new_expire_time) / TIMER_PARAMETER_MULTIPLE;
	timer->timeout = (unsigned int)delaytime;
	timer->count_num = timer->timeout;
	return 0;
}

/*
 * 释放软计时器资源
 */
int hisi_softtimer_free(struct softtimer_list *p)
{
	if ((p == NULL) || (!list_empty(&p->entry)))	{
		return -EIO;
	}
	p->init_flags = 0;
	return 0;
}

static void fun_para(softtimer_func func, unsigned long para)
{
	if (func != NULL) {
		func(para);
	}
}
/*lint -e715 -e826*/
/*
 * 软计时器线程
 */
static void thread_softtimer_fun(unsigned long arg)
{
	struct softtimer_list *p = NULL;
	softtimer_func func = NULL;
	unsigned long para = 0;

	pr_err("[%s] softtimer:thread_softtimer_fun\n", __func__);
	spin_lock(&timer_control.soft_timer_lock);

	timer_control.softtimer_start_value = ELAPESD_TIME_INVAILD;
	if (!list_empty(&(timer_control.timer_list_head))) {
		p = list_first_entry(&(timer_control.timer_list_head), struct softtimer_list, entry);
		if (p->is_running == TIMER_TRUE) {
			list_del_init(&p->entry);
			p->is_running = TIMER_FALSE;
			func = p->func;
			para = p->para;
			spin_unlock(&timer_control.soft_timer_lock);
			fun_para(func, para);
			spin_lock(&timer_control.soft_timer_lock);

			while (!list_empty(&(timer_control.timer_list_head))) {
				p = list_first_entry(&(timer_control.timer_list_head), struct softtimer_list, entry);
				if (0 == p->timeout) {
					list_del_init(&p->entry);
					p->is_running = TIMER_FALSE;
					func = p->func;
					para = p->para;
					spin_unlock(&timer_control.soft_timer_lock);
					fun_para(func, para);
					spin_lock(&timer_control.soft_timer_lock);
				} else {
					break;
				}
			}
			/*if have other node*/
			if (!list_empty(&(timer_control.timer_list_head))) {
				p = list_first_entry(&(timer_control.timer_list_head),
						struct softtimer_list, entry);
				p->is_running = TIMER_TRUE;
				start_hard_timer(p->timeout);
			} else {
				stop_hard_timer();
			}
		}  else if (p->is_running == TIMER_FALSE)  {
			p->is_running = TIMER_TRUE;
			start_hard_timer(p->timeout);
		}
	} else {
		stop_hard_timer();
	}
	timer_control.is_in_interrupt = TIMER_FALSE;

	spin_unlock(&timer_control.soft_timer_lock) ;
}
/*lint +e715 +e826*/
/*lint -e715 -e732 -e838*/
/*
 * 计时器中断回调
 */
static irqreturn_t IntTimerHandler(int irq, void *dev_id)
{
	unsigned int read_value;

	read_value = readl(timer_control.hard_timer_addr+TIMERMIS(0));
	pr_err("[%s] softtimer:IntTimerHandler %d\n", __func__, read_value);

	if (0 != read_value)    {
		timer_control.is_in_interrupt = TIMER_TRUE;
		writel(0x1, timer_control.hard_timer_addr + TIMERINTCLR(0));
		tasklet_schedule(&timer_control.softtimer_tasklet);
	}
	return IRQ_HANDLED;
}
/*lint +e715 +e732 +e838*/
/*
 * 软计时器去初始化
 */
void hisi_softtimer_uninit(void)
{
	free_irq(timer_control.irqnum, NULL);
	if (timer_control.clk_is_enable) {
		clk_disable_unprepare(timer_control.clk);
		timer_control.clk_is_enable = TIMER_FALSE;
	}
	if (timer_control.hard_timer_addr) {
		iounmap(timer_control.hard_timer_addr);
	}
}

/*
 * 软计时器初始化
 */
static int hisi_softtimer_init (struct platform_device *pdev)
{
	int ret;
	struct device_node *node;

	node = pdev->dev.of_node;

	timer_control.irqnum = irq_of_parse_and_map(node, 0);
	timer_control.clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(timer_control.clk)) {
		pr_err("[%s] softtimer: devm_clk_get ERROR\n", __func__);
		return (int)PTR_ERR(timer_control.clk);
	}
	ret = clk_prepare_enable(timer_control.clk);
	if (ret) {
		pr_err("[%s] softtimer:clk_prepare_enable error\n", __func__);
		return ret;
	}
	timer_control.clk_is_enable = TIMER_TRUE;
	timer_control.hard_timer_addr = of_iomap(node, 0);
	if (!timer_control.hard_timer_addr) {
		pr_err("[%s] softtimer:hisi_softtimer_init ERROR.\n", __func__);
		goto iomap_fin;
	}
	INIT_LIST_HEAD(&(timer_control.timer_list_head));
	timer_control.rate                    = HISI_SOFTTIME_FREQ;
	timer_control.softtimer_start_value  = ELAPESD_TIME_INVAILD;
	/*in default state ,clk is opened*/
	spin_lock_init(&timer_control.soft_timer_lock);
	tasklet_init(&timer_control.softtimer_tasklet, thread_softtimer_fun, (unsigned long)0);
	ret = request_irq(timer_control.irqnum, IntTimerHandler, (unsigned long)IRQF_TIMER,
	"softtimer", NULL);
	if (ret) {
		goto error;
	}
	/*clear timer interrupt*/
	writel(0x1, timer_control.hard_timer_addr + TIMERINTCLR(0));
	timer_control.init_flag = TIMER_TRUE;
	timer_control.is_in_interrupt = TIMER_FALSE;
	pr_err("[%s] softtimer:hisi_softtimer_init enter,%d\n", __func__, timer_control.rate);
	return 0;
error:
	iounmap(timer_control.hard_timer_addr);
	timer_control.hard_timer_addr = NULL;
iomap_fin:
	clk_disable_unprepare(timer_control.clk);
	timer_control.clk_is_enable = TIMER_FALSE;
	return ret;
}

/*lint -e715*/
#ifdef CONFIG_HISI_DEBUG_FS
/*
 * 软计时器超时回调
 */
static void hisi_softtimer_timeout(unsigned long data)
{
	(void)data;
	pr_err("[%s] hisi_softtimer_timeout\n", __func__);
	return ;
}
/*lint -e727*/
struct softtimer_list timer = {0};
void hisi_softtimer_test(int timeout)
{
	int ret;
	static int  flag;
	if (0 == flag) {
		hisi_softtimer_create(&timer, hisi_softtimer_timeout, (unsigned long)0, 0);
		flag = 1;
	}
	pr_err("[%s] hisi_softtimer_test enter\n", __func__);
	hisi_softtimer_delete(&timer);
	ret = hisi_softtimer_modify(&timer, (unsigned int)timeout);
	if (ret) {
		pr_err("[%s] %d:\n", __func__, __LINE__);
	}
	hisi_softtimer_add(&timer);
}
#endif
/*lint +e727*/
/*lint -e715*/
/*
* softtimer probe 函数
*/
static int generic_softtimer_probe(struct platform_device *pdev)
{
	int ret;
	struct device *dev;
	struct device_node *dev_node;

	dev = &pdev->dev;
	dev_node = dev->of_node;

	ret = get_contexthub_dts_status();
	if (ret) {
		return ret;
	}

	if (!of_device_is_available(dev_node)) {
		return -ENODEV;
	}

	return hisi_softtimer_init(pdev);
}

/*
* softtimer remove 函数
*/
static int generic_softtimer_remove(struct platform_device *pdev)
{
	hisi_softtimer_uninit();
	return 0;
}

static const struct of_device_id generic_softtimer[] = {
	{ .compatible = "hisilicon,contexthub-softtimer" },
	{},
};
MODULE_DEVICE_TABLE(of, generic_softtimer);
/*lint -e64*/
static struct platform_driver generic_softtimer_platdrv = {
	.driver = {
	.name	= "hisi-softtimer",
	.owner	= THIS_MODULE,
	.of_match_table = of_match_ptr(generic_softtimer),
	},
	.probe = generic_softtimer_probe,
	.remove  = generic_softtimer_remove,
};
/*lint +e64*/
/*lint +e785*/
/*lint -e64*/
/*
* softtimer功能的初始化
*/
static int __init hisi_timer_init(void)
{
	return platform_driver_register(&generic_softtimer_platdrv);
}
/*lint +e64*/
/*
 * 释放softtimer功能资源
 */
static void __exit hisi_timer_exit(void)
{
	platform_driver_unregister(&generic_softtimer_platdrv);
}
/*lint -e528 -esym(528,*) */
late_initcall(hisi_timer_init);
module_exit(hisi_timer_exit);
