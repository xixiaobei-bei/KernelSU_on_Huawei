/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/hisi-spmi.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_hisi_spmi.h>

struct of_spmi_dev_info {
	struct spmi_controller *ctrl;
	struct spmi_boardinfo b_info;
};
struct of_spmi_res_info {
	struct device_node *node;
	uint32_t num_reg;
	uint32_t num_irq;
};

/**
 * spmi_get_resource - get a resource for a device
 * @dev: spmi device
 * @node: device node resource
 * @type: resource type
 * @res_num: resource index
 *
 * If 'node' is specified as NULL, then the API treats this as a special
 * case to assume the first devnode. For configurations that do not use
 * spmi-dev-container, there is only one node to begin with, so NULL should
 * be passed in this case.
 *
 * Returns
 *  NULL on failure.
 */
struct resource *spmi_get_resource(struct spmi_device *dev,
				   struct spmi_resource *node,
				   unsigned int type, unsigned int res_num)
{
	unsigned int i;

	/* if a node is not specified, default to the first node */
	if (!node)
		node = &dev->res;

	for (i = 0; i < node->num_resources; i++){
		struct resource *r = &node->resource[i];

		if (type == resource_type(r) && res_num-- == 0)
			return r;
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(spmi_get_resource);

#define SPMI_MAX_RES_NAME 256

/**
 * spmi_get_resource_byname - get a resource for a device given a name
 * @dev: spmi device handle
 * @node: device node resource
 * @type: resource type
 * @name: resource name to lookup
 */
struct resource *spmi_get_resource_byname(struct spmi_device *dev,
					  struct spmi_resource *node,
					  unsigned int type,
					  const char *name)
{
	unsigned int i;

	/* if a node is not specified, default to the first node */
	if (!node)
		node = &dev->res;

	for (i = 0; i < node->num_resources; i++){
		struct resource *r = &node->resource[i];

		if (type == resource_type(r) && r->name &&
				!strncmp(r->name, name, SPMI_MAX_RES_NAME))
			return r;
	}
	return NULL;
}
EXPORT_SYMBOL_GPL(spmi_get_resource_byname);

/**
 * spmi_get_irq - get an IRQ for a device
 * @dev: spmi device
 * @node: device node resource
 * @res_num: IRQ number index
 *
 * Returns
 *  -ENXIO on failure.
 */
int spmi_get_irq(struct spmi_device *dev, struct spmi_resource *node,
					  unsigned int res_num)
{
	/* if a node is not specified, default to the first node */
	if (!node)
		node = &dev->res;

	return of_irq_get(node->of_node, res_num);
}
EXPORT_SYMBOL_GPL(spmi_get_irq);

/**
 * spmi_get_irq_byname - get an IRQ for a device given a name
 * @dev: spmi device handle
 * @node: device node resource
 * @name: resource name to lookup
 *
 * Returns -ENXIO on failure
 */
int spmi_get_irq_byname(struct spmi_device *dev,
			struct spmi_resource *node, const char *name)
{
	/* if a node is not specified, default to the first node */
	if (!node)
		node = &dev->res;

	return of_irq_get_byname(node->of_node, name);
}
EXPORT_SYMBOL_GPL(spmi_get_irq_byname);

/*
 * Initialize r_info structure for safe usage
 */
static inline void of_spmi_init_resource(struct of_spmi_res_info *r_info,
					 struct device_node *node)
{
	r_info->node = node;
	r_info->num_reg = 0;
	r_info->num_irq = 0;
}

/*
 * Calculate the number of resources to allocate
 *
 * The caller is responsible for initializing the of_spmi_res_info structure.
 */
static void of_spmi_sum_resources(struct of_spmi_res_info *r_info,
				  bool has_reg)
{
	struct of_phandle_args oirq;
	uint64_t size;
	uint32_t flags;
	int i = 0;

	while (of_irq_parse_one(r_info->node, i, &oirq) == 0)
		i++;

	r_info->num_irq += i;

	if (!has_reg)
		return;

	/*
	 * We can't use of_address_to_resource here since it includes
	 * address translation; and address translation assumes that no
	 * parent buses have a size-cell of 0. But SPMI does have a
	 * size-cell of 0.
	 */
	i = 0;
	while (of_get_address(r_info->node, i, &size, &flags) != NULL)
		i++;

	r_info->num_reg += i;
}

/*
 * Allocate enough memory to handle the resources associated with the
 * primary node.
 */
static int of_spmi_allocate_node_resources(struct of_spmi_dev_info *d_info,
					   struct of_spmi_res_info *r_info)
{
	uint32_t num_irq = r_info->num_irq, num_reg = r_info->num_reg;
	struct resource *res = NULL;

	if (num_irq || num_reg) {
		res = kzalloc(sizeof(*res) * (num_irq + num_reg), GFP_KERNEL);
		if (!res)
			return -ENOMEM;
	}
	d_info->b_info.res.num_resources = num_reg + num_irq;
	d_info->b_info.res.resource = res;

	return 0;
}

/*
 * free node resources - used with primary node
 */
static void of_spmi_free_node_resources(struct of_spmi_dev_info *d_info)
{
	kfree(d_info->b_info.res.resource);
}

static void of_spmi_populate_resources(struct of_spmi_dev_info *d_info,
				       struct of_spmi_res_info *r_info,
				       struct resource *res)

{
	uint32_t num_irq = r_info->num_irq, num_reg = r_info->num_reg;
	unsigned int i;
	const  __be32 *addrp;
	uint64_t size;
	uint32_t flags;

	if ((num_irq || num_reg) && (res != NULL)) {
		for (i = 0; i < num_reg; i++, res++){
			/* Addresses are always 16 bits */
			addrp = of_get_address(r_info->node, i, &size, &flags);
			BUG_ON(!addrp);
			res->start = be32_to_cpup(addrp);
			res->end = res->start + size - 1;
			res->flags = flags;
			(void)of_property_read_string_index(r_info->node, "reg-names",i, &res->name);
		}
	}
}

/*
 * Gather primary node resources and populate.
 */
static void of_spmi_populate_node_resources(struct of_spmi_dev_info *d_info,
					    struct of_spmi_res_info *r_info)

{
	struct resource *res;

	res = d_info->b_info.res.resource;
	d_info->b_info.res.of_node = r_info->node;
	(void)of_property_read_string(r_info->node, "label",&d_info->b_info.res.label);
	of_spmi_populate_resources(d_info, r_info, res);
}
/*
 * Allocate dev_node array for spmi_device - used with spmi-dev-container
 */
static inline int of_spmi_alloc_devnode_store(struct of_spmi_dev_info *d_info,
					      uint32_t num_dev_node)
{
	d_info->b_info.num_dev_node = num_dev_node;
	d_info->b_info.dev_node = kzalloc(sizeof(struct spmi_resource) *
						num_dev_node, GFP_KERNEL);
	if (!d_info->b_info.dev_node)
		return -ENOMEM;

	return 0;
}

/*
 * Allocate enough memory to handle the resources associated with the
 * spmi-dev-container nodes.
 */
static int of_spmi_allocate_devnode_resources(struct of_spmi_dev_info *d_info,
					      struct of_spmi_res_info *r_info,
					      uint32_t idx)
{
	uint32_t num_irq = r_info->num_irq, num_reg = r_info->num_reg;
	struct resource *res = NULL;

	if (num_irq || num_reg) {
		res = kzalloc(sizeof(*res) * (num_irq + num_reg), GFP_KERNEL);
		if (!res)
			return -ENOMEM;
	}
	d_info->b_info.dev_node[idx].num_resources = num_reg + num_irq;
	d_info->b_info.dev_node[idx].resource = res;

	return 0;
}

/*
 * create a single spmi_device
 */
static int of_spmi_create_device(struct of_spmi_dev_info *d_info,
				 struct device_node *node)
{
	struct spmi_controller *ctrl = d_info->ctrl;
	struct spmi_boardinfo *b_info = &d_info->b_info;
	void *result;
	int rc;

	rc = of_modalias_node(node, b_info->name, sizeof(b_info->name));
	if (rc < 0) {
		dev_err(&ctrl->dev, "of_spmi modalias failure on %s\n",
				node->full_name);
		return rc;
	}

	b_info->of_node = of_node_get(node);
	result = spmi_new_device(ctrl, b_info);

	if (result == NULL) {
		dev_err(&ctrl->dev, "of_spmi: Failure registering %s\n",
				node->full_name);
		of_node_put(node);
		return -ENODEV;
	}
	return 0;
}
/*
 * free devnode resources - used with spmi-dev-container
 */
static void of_spmi_free_devnode_resources(struct of_spmi_dev_info *d_info)
{
	unsigned int i;

	for (i = 0; i < d_info->b_info.num_dev_node; i++)
		kfree(d_info->b_info.dev_node[i].resource);

	kfree(d_info->b_info.dev_node);
}
/*
 * Gather node devnode resources and populate - used with spmi-dev-container.
 */
static void of_spmi_populate_devnode_resources(struct of_spmi_dev_info *d_info,
					       struct of_spmi_res_info *r_info,
					       int idx)

{
	struct resource *res;

	res = d_info->b_info.dev_node[idx].resource;
	d_info->b_info.dev_node[idx].of_node = r_info->node;
	(void)of_property_read_string(r_info->node, "label",&d_info->b_info.dev_node[idx].label);
	of_spmi_populate_resources(d_info, r_info, res);
}

/*
 * Walks all children of a node containing the spmi-dev-container
 * binding. This special type of spmi_device can include resources
 * from more than one device node.
 */
static void of_spmi_walk_dev_container(struct of_spmi_dev_info *d_info,
					struct device_node *container)
{
	struct of_spmi_res_info r_info = {};
	struct spmi_controller *ctrl = d_info->ctrl;
	struct device_node *node;
	int rc, i, num_dev_node = 0;

	if (!of_device_is_available(container))
		return;

	/*
	 * Count the total number of device_nodes so we know how much
	 * device_store to allocate.
	 */
	for_each_child_of_node(container, node) {
		if (!of_device_is_available(node))
			continue;
		num_dev_node++;
	}

	rc = of_spmi_alloc_devnode_store(d_info, num_dev_node);
	if (rc) {
		dev_err(&ctrl->dev, "%s: unable to allocate devnode resources\n",
								__func__);
		return;
	}

	i = 0;
	for_each_child_of_node(container, node) {
		if (!of_device_is_available(node))
			continue;
		of_spmi_init_resource(&r_info, node);
		of_spmi_sum_resources(&r_info, true);
		rc = of_spmi_allocate_devnode_resources(d_info, &r_info, i);
		if (rc) {
			dev_err(&ctrl->dev, "%s: unable to allocate"
					" resources\n", __func__);
			of_spmi_free_devnode_resources(d_info);
			return;
		}
		of_spmi_populate_devnode_resources(d_info, &r_info, i);
		i++;
	}

	of_spmi_init_resource(&r_info, container);
	of_spmi_sum_resources(&r_info, true);

	rc = of_spmi_allocate_node_resources(d_info, &r_info);
	if (rc) {
		dev_err(&ctrl->dev, "%s: unable to allocate resources\n",
								  __func__);
		of_spmi_free_node_resources(d_info);
	}

	of_spmi_populate_node_resources(d_info, &r_info);


	rc = of_spmi_create_device(d_info, container);
	if (rc) {
		dev_err(&ctrl->dev, "%s: unable to create device for"
				" node %s\n", __func__, container->full_name);
		of_spmi_free_devnode_resources(d_info);
		return;
	}
}

/*
 * Walks all children of a node containing the spmi-slave-container
 * binding. This indicates that all spmi_devices created from this
 * point all share the same slave_id.
 */
static void of_spmi_walk_slave_container(struct of_spmi_dev_info *d_info,
					 struct device_node *container)
{
	struct spmi_controller *ctrl = d_info->ctrl;
	struct device_node *node;
	int rc;

	for_each_child_of_node(container, node) {
		struct of_spmi_res_info r_info;

		if (!of_device_is_available(node))
			continue;

		/**
		 * Check to see if this node contains children which
		 * should be all created as the same spmi_device.
		 */
		if (of_get_property(node, "spmi-dev-container", NULL)) {
			of_spmi_walk_dev_container(d_info, node);
			continue;
		}

		of_spmi_init_resource(&r_info, node);
		of_spmi_sum_resources(&r_info, true);

		rc = of_spmi_allocate_node_resources(d_info, &r_info);
		if (rc) {
			dev_err(&ctrl->dev, "%s: unable to allocate"
						" resources\n", __func__);
			goto slave_err;
		}

		of_spmi_populate_node_resources(d_info, &r_info);

		rc = of_spmi_create_device(d_info, node);
		if (rc) {
			dev_err(&ctrl->dev, "%s: unable to create device for"
				     " node %s\n", __func__, node->full_name);
			goto slave_err;
		}
	}
	return;

slave_err:
	of_spmi_free_node_resources(d_info);
}

int of_spmi_register_devices(struct spmi_controller *ctrl)
{
	struct device_node *node = ctrl->dev.of_node;

	/* Only register child devices if the ctrl has a node pointer set */
	if (!node)
		return -ENODEV;

	for_each_child_of_node(ctrl->dev.of_node, node) {
		struct of_spmi_dev_info d_info = {};
		int rc, have_dev_container = 0;

		/* Get properties from the device tree */
		rc = of_property_read_u8(node, "slave_id", &d_info.b_info.slave_id);
		if (rc) {
			dev_err(&ctrl->dev, "slave_id \n");
			continue;
		}

		d_info.ctrl = ctrl;

		if (of_get_property(node, "spmi-dev-container", NULL))
			have_dev_container = 1;
		if (of_get_property(node, "spmi-slave-container", NULL)) {
			of_spmi_walk_dev_container(&d_info, node);
			if (!have_dev_container)
				of_spmi_walk_slave_container(&d_info, node);
		} else {
			struct of_spmi_res_info r_info;  /*lint !e578*/

			/**
			 * A dev container at the second level without a slave
			 * container is considered an error.
			 */
			if (have_dev_container) {
				dev_err(&ctrl->dev, "%s: structural error,"
				     " node %s has spmi-dev-container without"
				     " specifying spmi-slave-container\n",
				     __func__, node->full_name);
				continue;
			}

		if (!of_device_is_available(node))
			continue;

		of_spmi_init_resource(&r_info, node);
		of_spmi_sum_resources(&r_info, false);
		rc = of_spmi_allocate_node_resources(&d_info, &r_info);
		if (rc) {
			dev_err(&ctrl->dev, "%s: unable to allocate"
					" resources\n", __func__);
			of_spmi_free_node_resources(&d_info);
			continue;
		}

		of_spmi_populate_node_resources(&d_info, &r_info);

		rc = of_spmi_create_device(&d_info, node);
		if (rc) {
			dev_err(&ctrl->dev, "%s: unable to create"
					" device\n", __func__);
				of_spmi_free_node_resources(&d_info);
				continue;
			}
		}
	}

	return 0;
}
EXPORT_SYMBOL(of_spmi_register_devices);

MODULE_LICENSE("GPL v2");
