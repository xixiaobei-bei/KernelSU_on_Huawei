# SPDX-License-Identifier: GPL-2.0
ccflags-y +=	-Idrivers/hisi/tzdriver
obj-$(CONFIG_ION) +=	ion.o ion-ioctl.o ion_heap.o
obj-$(CONFIG_ION_SYSTEM_HEAP) += ion_system_heap.o ion_page_pool.o
obj-$(CONFIG_ION_CARVEOUT_HEAP) += ion_carveout_heap.o
obj-$(CONFIG_ION_CHUNK_HEAP) += ion_chunk_heap.o
obj-$(CONFIG_ION_CMA_HEAP) += ion_cma_heap.o
obj-$(CONFIG_ION_HISI) += hisi/
obj-$(CONFIG_ION_HISI_SECSG) += ion_seccg_heap.o ion_sec_contig.o ion_secsg_heap.o
obj-$(CONFIG_ION_HISI_SECCM) += ion_seccm_heap.o
obj-$(CONFIG_ION_HISI_DMA_POOL) += ion_dma_pool_heap.o
obj-$(CONFIG_ION_HISI_CPA) += ion_cpa_heap.o
