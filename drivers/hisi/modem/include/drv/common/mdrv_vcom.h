#ifndef __MDRV_VCOM_H__
#define __MDRV_VCOM_H__

#define com_id(n,i)     com_id_##n##_##i
#define com_name(n, i)  n#i
#define com_map(n,i)    com_name(#n,i),com_id(n,i)
#define MEXMSGLEN   (4096)
#define SYNCCOM     0
#define ASYNCOM     1
#define THROUGH     2
#define BINDPID     3
#define HOTPLUG     4

enum com_id_e {
    com_id_appvcom_0,
    com_id_appvcom_1,
    com_id_appvcom_2,
    com_id_appvcom_3,
    com_id_appvcom_4,
    com_id_appvcom_5,
    com_id_appvcom_6,
    com_id_appvcom_7,
    com_id_appvcom_8,
    com_id_appvcom_9,
    com_id_appvcom_10,
    com_id_appvcom_11,
    com_id_appvcom_12,
    com_id_appvcom_13,
    com_id_appvcom_14,
    com_id_appvcom_15,
    com_id_appvcom_16,
    com_id_appvcom_17,
    com_id_appvcom_18,
    com_id_appvcom_19,
    com_id_appvcom_20,
    com_id_appvcom_21,
    com_id_appvcom_22,
    com_id_appvcom_23,
    com_id_appvcom_24,
    com_id_appvcom_25,
    com_id_appvcom_26,
    com_id_appvcom_27,
    com_id_appvcom_28,
    com_id_appvcom_29,
    com_id_appvcom_30,
    com_id_appvcom_31,
    com_id_appvcom_32,
    com_id_appvcom_33,
    com_id_appvcom_34,
    com_id_appvcom_35,
    com_id_appvcom_36,
    com_id_appvcom_37,
    com_id_appvcom_38,
    com_id_appvcom_39,
    com_id_appvcom_40,
    com_id_appvcom_41,
    com_id_appvcom_42,
    com_id_appvcom_43,
    com_id_appvcom_44,
    com_id_appvcom_45,
    com_id_appvcom_46,
    com_id_appvcom_47,
    com_id_appvcom_48,
    com_id_appvcom_49,
    com_id_appvcom_50,
    com_id_appvcom_51,
    com_id_appvcom_52,
    com_id_appvcom_53,  //com_id_appvcom_ERRLOG
    com_id_appvcom_54,  //com_id_appvcom_TLLOG,
    com_id_appvcom_55,  //com_id_appvcom_LOG,
    com_id_appvcom_56,  //com_id_appvcom_LOG1
    com_id_appvcom_57,
    com_id_appvcom_58,
    com_id_appvcom_59,
    com_id_appvcom_60,
    com_id_appvcom_61,
    com_id_appvcom_62,
    com_id_appvcom_63,
    com_id_nmctrlvcom,
    com_id_bastet,
    com_id_simhotplug_1,
    com_id_modemstatus,
    com_id_modemstatus_w,
    com_id_getslice,
    com_id_getslice_w,
    com_id_portcfg_r,
    com_id_portcfg_w,
    com_id_act,
    bind_pid,
    com_id_rnic,
    com_id_bottom,
};

enum com_id_ex_e {
    com_ex_id_power_state = com_id_bottom,
};

#endif
