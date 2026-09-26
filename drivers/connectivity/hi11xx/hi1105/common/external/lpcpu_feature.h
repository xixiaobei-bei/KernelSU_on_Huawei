

#ifndef __LPCPU_FEATURE_H__
#define __LPCPU_FEATURE_H__

#ifndef _PRE_PRODUCT_HI1620S_KUNPENG
#if defined(CONFIG_ARCH_HISI)
#if defined(CONFIG_ARCH_PLATFORM)
extern void get_slow_cpus(struct cpumask *cpumask);
extern void get_fast_cpus(struct cpumask *cpumask);

#define external_get_slow_cpus(cpumask)  get_slow_cpus(cpumask)
#define external_get_fast_cpus(cpumask)  get_fast_cpus(cpumask)
#else
extern void hisi_get_slow_cpus(struct cpumask *cpumask);
extern void hisi_get_fast_cpus(struct cpumask *cpumask);
#define external_get_slow_cpus(cpumask)  hisi_get_slow_cpus(cpumask)
#define external_get_fast_cpus(cpumask)  hisi_get_fast_cpus(cpumask)

#endif /* endif for  CONFIG_ARCH_PLATFORM */
#endif /* endif for CONFIG_ARCH_HISI */
#endif /* endif for _PRE_PRODUCT_HI1620S_KUNPENG */
/* º¯Êý¶¨Òå */
int32_t gps_ilde_sleep_vote(uint32_t val);
void wlan_pm_idle_sleep_vote(uint8_t uc_allow);

#endif // __EXTERNAL_FEATURE_H__