
# Declare header File path
EXTRA_CFLAGS += \
    -Iinclude/drm \
    -Idrivers/gpu/drm/hisi


hisi-drm-heaps-y := \
         hisi_drm_page_pool_heap.o \
         hisi_drm_heap_ops_helper.o \
         hisi_drm_page_pool.o \
         hisi_drm_debug_heap.o \
         hisi_drm_heaps_helper.o \
         hisi_drm_heaps_manager.o \
         hisi_drm_heaps_defs.o
obj-$(CONFIG_DRM_GEM_HEAP) += hisi-drm-heaps.o

hisi-drm-heaps-tracer-y := \
	 hisi_drm_heaps_tracer.o
obj-$(CONFIG_DRM_GEM_TRACE) += hisi-drm-heaps-tracer.o

clean:
	rm *.o .*cmd
