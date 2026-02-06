
#include "hisi_lcd_drv.h"
#include "hisi_defs.h"


static int hisi_mipi_lcd_probe(struct mipi_dsi_device *dsi)
{
	UNUSED(dsi);

	return 0;
}

static int hisi_mipi_lcd_remove(struct mipi_dsi_device *dsi)
{
	UNUSED(dsi);

	return 0;
}

static void hisi_mipi_lcd_shutdown(struct mipi_dsi_device *dsi)
{
	UNUSED(dsi);
}

static const struct of_device_id hisi_lcd_panel_match_table[] = {
	{
		.compatible = "hisi,mipi_sharp_NT36870",
		.data = NULL,
	},
	{
		.compatible = "hisi,mipi_sharp_NT36860",
		.data = NULL,
	},
	{},
};

static struct mipi_dsi_driver hisi_mipi_lcd_driver = {
	.probe = hisi_mipi_lcd_probe,
	.remove = hisi_mipi_lcd_remove,
	.shutdown = hisi_mipi_lcd_shutdown,
	.driver = {
		.name = "hisi-mipi-lcd",
		.owner = THIS_MODULE,
		.of_match_table = hisi_lcd_panel_match_table,
	},
};


module_mipi_dsi_driver(hisi_mipi_lcd_driver);
