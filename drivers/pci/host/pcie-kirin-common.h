#ifndef _PCIE_KIRIN_COMMON_H
#define _PCIE_KIRIN_COMMON_H

#include "pcie-kirin.h"
#include <hw_cmdline_parse.h>

#define INBOUNT_OFFSET 0x100

/* noc power domain */
#define NOC_POWER_IDLEREQ_1		0x38c
#define NOC_POWER_IDLE_1		0x394
#define NOC_PW_MASK				0x10000
#define NOC_PW_SET_BIT			0x1

#define PCIEPHY_RESET_BIT		(0x1 << 17)
#define PCIEPHY_PIPE_LINE0_RESET_BIT	(0x1 << 19)

int config_enable_dbi(u32 rc_id, int flag);
int ltssm_enable(u32 rc_id, int yes);
int set_bme(u32 rc_id, int flag);
int set_mse(u32 rc_id, int flag);
int kirin_pcie_noc_power(struct kirin_pcie *pcie, int enable);
int kirin_pcie_phy_init(struct kirin_pcie *pcie);
void kirin_pcie_natural_cfg(struct kirin_pcie *pcie);
void kirin_pcie_reset_phy(struct kirin_pcie *pcie);
void kirin_pcie_config_l0sl1(u32 rc_id, enum link_aspm_state aspm_state);
void kirin_pcie_config_l1ss(u32 rc_id, enum l1ss_ctrl_state enable);
void kirin_pcie_outbound_atu(u32 rc_id, int index,
		int type, u64 cpu_addr, u64 pci_addr, u32 size);
void kirin_pcie_inbound_atu(u32 rc_id, int index,
		int type, u64 cpu_addr, u64 pci_addr, u32 size);
void kirin_pcie_select_eco(struct kirin_pcie *pcie);
int kirin_pcie_cfg_eco(struct kirin_pcie *pcie);
int wlan_on(u32 rc_id, int on);
void pcie_io_adjust(struct kirin_pcie *pcie);
void kirin_pcie_generate_msg(u32 rc_id, int index, u32 iatu_offset, int msg_type, u32 msg_code);

int pcie_memcpy(ulong dts, ulong src, uint32_t size);


u32 show_link_state(u32 rc_id);


static inline void pcie_wr_8(uint8_t val, char *addr)
{
	(*(volatile uint8_t *)(addr)) = (val);
}

static inline void pcie_wr_32(uint32_t val, char *addr)
{
	(*(volatile uint32_t *)(addr)) = (val);
}

static inline void pcie_wr_64(uint64_t val, char *addr)
{
	(*(volatile uint64_t *)(addr)) = (val);
}

static inline uint8_t pcie_rd_8(char *addr)
{
	return (*(volatile uint8_t *)(addr));
}

static inline uint32_t pcie_rd_32(char *addr)
{
	return (*(volatile uint32_t *)(addr));
}

static inline uint64_t pcie_rd_64(char *addr)
{
	return (*(volatile uint64_t *)(addr));
}

#ifdef CONFIG_KIRIN_PCIE_TEST
int retrain_link(u32 rc_id);
int set_link_speed(u32 rc_id, enum link_speed gen);
int show_link_speed(u32 rc_id);
u32 kirin_pcie_find_capability(struct pcie_port *pp, int cap);
#endif

#endif

