#define pciedev_debug(fmt, args...) printk(KERN_DEBUG "[pcie_misc]" fmt, ## args)
#define pciedev_info(fmt, args...) printk(KERN_INFO "[pcie_misc]" fmt, ## args)
#define pciedev_warn(fmt, args...) printk(KERN_WARNING "[pcie_misc]" fmt, ## args)
#define pciedev_err(fmt, args...) printk(KERN_ERR "[pcie_misc]" fmt, ## args)
