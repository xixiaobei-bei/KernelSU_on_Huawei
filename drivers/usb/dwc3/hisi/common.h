#ifndef __LINUX_DWC3_COMMON_H__
#define __LINUX_DWC3_COMMON_H__

#define HM_EN(n)		(0x10001ul << (n))
#define HM_DIS(n)		(0x10000ul << (n))
#define writel_mask(mask, data, addr)	/*lint -save -e717 */do {writel((((u32)readl(addr)) & (~((u32)(mask)))) | ((data) & (mask)), (addr));} while (0)/*lint -restore */

static inline unsigned int is_bits_clr(unsigned int  mask, volatile void __iomem *addr)
{
    return (((*(volatile unsigned int *) (addr)) & (mask)) == 0);
}

static inline unsigned int is_bits_set(unsigned int  mask, volatile void __iomem *addr)
{
	return (((*(volatile unsigned int*) (addr))&(mask)) == (mask));
}

static inline void set_bits(unsigned int  mask, volatile void __iomem *addr)
{
    (*(volatile unsigned int *) (addr)) |= mask;
}

static inline void clr_bits(unsigned int mask, volatile void __iomem *addr)
{
    (*(volatile unsigned int *) (addr)) &= ~(mask);
}

static inline void usb3_rw_reg_writel(u32 val, void __iomem *base, unsigned long int offset)
{
	writel(val, (base + offset));
}

static inline u32 usb3_rw_reg_readl(void __iomem *base, unsigned long int offset)
{
	return readl(base + offset);
}

static inline void usb3_rw_reg_setbit(u32 bit, void __iomem *base, unsigned long int offset)
{
	volatile uint32_t temp;
	temp = readl(base + offset);
	temp |= (1u << bit);
	writel(temp, base + offset);
}

static inline void usb3_rw_reg_clrbit(u32 bit, void __iomem *base, unsigned long int offset)
{
	volatile uint32_t temp;
	temp = readl(base + offset);
	temp &= ~(1u << bit);
	writel(temp, base + offset);
}

static inline void usb3_rw_reg_setvalue(u32 val, void __iomem *base, unsigned long int offset)
{
	volatile uint32_t temp;
	temp = readl(base + offset);
	temp |= val;
	writel(temp, base + offset);
}

static inline void usb3_rw_reg_clrvalue(u32 val, void __iomem *base, unsigned long int offset)
{
	volatile uint32_t temp;
	temp = readl(base + offset);
	temp &= val;
	writel(temp, base + offset);
}

#endif

