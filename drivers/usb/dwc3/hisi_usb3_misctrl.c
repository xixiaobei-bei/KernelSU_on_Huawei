#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/bitops.h>

#include "dwc3-hisi.h"
#include "hisi_usb3_misctrl.h"

static void __iomem *misc_base_addr;
static void __iomem *sc_misc_base_addr;

void init_misc_ctrl_addr(void __iomem *base)
{
	pr_info("[USB.DBG]:misc base:%pK\n", base);
	misc_base_addr = base;
}

void init_sc_misc_ctrl_addr(void __iomem *base)
{
	pr_info("[USB.DBG]:sc misc base:%pK\n", base);
	sc_misc_base_addr = base;
}

void usb3_rw_reg_writel(u32 val, void __iomem *base, unsigned long int offset)
{
	writel(val, (base + offset));
}

u32 usb3_rw_reg_readl(void __iomem *base, unsigned long int offset)
{
	return  readl(base + offset);
}

void usb3_rw_reg_setbit(u32 bit, void __iomem *base, unsigned long int offset)
{
	volatile uint32_t temp;
	temp = readl(base + offset);
	temp |= (1u << bit);
	writel(temp, base + offset);
}

void usb3_rw_reg_clrbit(u32 bit, void __iomem *base, unsigned long int offset)
{
	volatile uint32_t temp;
	temp = readl(base + offset);
	temp &= ~(1u << bit);
	writel(temp, base + offset);
}

void usb3_rw_reg_setvalue(u32 val, void __iomem *base, unsigned long int offset)
{
	volatile uint32_t temp;
	temp = readl(base + offset);
	temp |= val;
	writel(temp, base + offset);
}

void usb3_rw_reg_clrvalue(u32 val, void __iomem *base, unsigned long int offset)
{
	volatile uint32_t temp;
	temp = readl(base + offset);
	temp &= val;
	writel(temp, base + offset);
}

void usb3_misc_reg_writel(u32 val, unsigned long int offset)
{
	usb3_rw_reg_writel(val, misc_base_addr, offset);
}

u32 usb3_misc_reg_readl(unsigned long int offset)
{
	return usb3_rw_reg_readl(misc_base_addr, offset);
}

void usb3_misc_reg_setbit(u32 bit, unsigned long int offset)
{
	usb3_rw_reg_setbit(bit, misc_base_addr, offset);
}

void usb3_misc_reg_clrbit(u32 bit, unsigned long int offset)
{
	usb3_rw_reg_clrbit(bit, misc_base_addr, offset);
}

void usb3_misc_reg_setvalue(u32 val, unsigned long int offset)
{
	usb3_rw_reg_setvalue(val, misc_base_addr, offset);
}

void usb3_misc_reg_clrvalue(u32 val, unsigned long int offset)
{
	usb3_rw_reg_clrvalue(val, misc_base_addr, offset);
}

u32 usb3_sc_misc_reg_readl(unsigned long int offset)
{
	return usb3_rw_reg_readl(sc_misc_base_addr, offset);
}

void usb3_sc_misc_reg_writel(u32 val, unsigned long int offset)
{
	usb3_rw_reg_writel(val, sc_misc_base_addr, offset);
}

void usb3_sc_misc_reg_setbit(u32 bit, unsigned long int offset)
{
	usb3_rw_reg_setbit(bit, sc_misc_base_addr, offset);
}

void usb3_sc_misc_reg_clrbit(u32 bit, unsigned long int offset)
{
	usb3_rw_reg_clrbit(bit, sc_misc_base_addr, offset);
}

void usb3_sc_misc_reg_setvalue(u32 val, unsigned long int offset)
{
	usb3_rw_reg_setvalue(val, sc_misc_base_addr, offset);
}

void usb3_sc_misc_reg_clrvalue(u32 val, unsigned long int offset)
{
	usb3_rw_reg_clrvalue(val, sc_misc_base_addr, offset);
}

