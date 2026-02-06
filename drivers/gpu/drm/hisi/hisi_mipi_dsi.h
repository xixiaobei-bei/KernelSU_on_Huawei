
#ifndef _HISI_MIPI_DSI_H_
#define _HISI_MIPI_DSI_H_

#include "hisi_drm_drv.h"
#include "hisi_drm_dsi.h"
#include "hisi_panel.h"
#include "hisi_dsi_reg.h"

void mipi_cdphy_init_config(char __iomem *mipi_dsi_base, struct hisi_dsi *dsi);
void mipi_config_phy_test_code(char __iomem *mipi_dsi_base,
	uint32_t test_code_addr, uint32_t test_code_parameter);
void mipi_config_cphy_spec1v0_parameter(char __iomem *mipi_dsi_base,
	struct hisi_dsi *dsi);
void mipi_config_dphy_spec1v2_parameter(char __iomem *mipi_dsi_base,
	struct hisi_dsi *dsi);
uint32_t mipi_pixel_clk(struct hisi_dsi *dsi);

void mipi_ldi_init(struct hisi_dsi *dsi, char __iomem *mipi_dsi_base);
void init_dual_mipi_ctrl(struct hisi_dsi *dsi);

void get_dsi_dphy_ctrl(struct hisi_dsi *dsi,
	struct mipi_dsi_phy_ctrl *phy_ctrl);

void enable_dsi_ldi(struct hisi_dsi *dsi);
void disable_dsi_ldi(struct hisi_dsi *dsi);
void ldi_dsi_data_gate(struct hisi_dsi *dsi, bool enable);
void single_frame_update(struct hisi_dsi *dsi);

#endif
