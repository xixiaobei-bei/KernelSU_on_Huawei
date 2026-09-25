

/* 头文件包含 */
#include "platform_common_clk.h"

#include <linux/mtd/hisi_nve_interface.h>

#include "hisi_ini.h"
#include "plat_debug.h"
#include "chr_user.h"
#include "plat_firmware.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLATFORM_COMMON_CLK_C

typedef struct {
    int8 *pc_name;         // ini 文件中对应的名字
    int8 c_index;          // 使用的是第几组参数a0，第一组， a1,第二组...
    oal_int32 l_type;      // 数组最大值/最小值的index
    oal_int32 l_prot_min;  // 读取值的下限保护
    oal_int32 l_prot_max;  // 读取值的上限保护
    oal_int32 l_def_val;   // 存放默认数据的结果
} ini_file_str;

typedef struct {
    oal_int32 part_id;     // nv号
    oal_int32 total_size;  // nv项大小
    oal_int32 read_size;   // 存储实际有效内容的大小
    char *name;
    oal_int32 (*process_cali)(oal_uint8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para);  // 处理函数
} nv_part_str;

/* 函数声明 */
oal_int32 process_plat_cali(oal_uint8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para);
oal_int32 process_factory_cali(oal_uint8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para);
oal_int32 process_gnss_self_study(oal_uint8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para);

/* 全局变量定义 */
oal_uint8 *pucDcxoDataBuf = NULL;  // 存储从NV中读取的参数信息，解析cfg文件时赋值一次，后续GNSS业务需要，也会更新该变量
dcxo_manage_stru dcxo_info = {0};
dcxo_nv_info_stru factory = {0};

oal_int32 default_coeff[COEFF_NUM] = {
    DEFAULT_A0,
    DEFAULT_A1,
    DEFAULT_A2,
    DEFAULT_A3,
    DEFAULT_T0
};

dcxo_pmu_addr_stru st_pmu_addr[PMU_TYPE_BUFF] = {

    {   PMU_6555V300_CALI_END, PMU_6555V300_CALI_AVE0, PMU_6555V300_CALI_AVE1,
        PMU_6555V300_CALI_AVE2, PMU_6555V300_CALI_AVE3, PMU_6555V300_ANA_EN,
        PMU_6555V300_STATE, PMU_6555V300_LOW_BYTE, PMU_6555V300_HIGH_BYTE,
        PMU_HI655V300
    },

    {   PMU_6421V700_CALI_END, PMU_6421V700_CALI_AVE0, PMU_6421V700_CALI_AVE1,
        PMU_6421V700_CALI_AVE2, PMU_6421V700_CALI_AVE3, PMU_6421V700_ANA_EN,
        PMU_6421V700_STATE, PMU_6421V700_LOW_BYTE, PMU_6421V700_HIGH_BYTE,
        PMU_HI6421V700
    }
};

/* 先读平台默认值，其次工厂刷入的nv，最后读自学习的nv，顺序不能反 */
nv_part_str gst_dcxo_nv_part[] = {
    {   PLAT_DEF_ID, OAL_SIZEOF(default_coeff), OAL_SIZEOF(default_coeff),
        PLAT_DEF_NAME, process_plat_cali
    },

    {   NV_FAC_ID, NV_FAC_SIZE, OAL_SIZEOF(dcxo_nv_info_stru),
        NV_FAC_NAME, process_factory_cali
    },
    /* GNSS 自学习分区的大小是valid标志位 + a0~t0五个系数 */
    {   NV_GNSS_ID, NV_GNSS_SIZE, OAL_SIZEOF(oal_int32) + OAL_SIZEOF(oal_uint32)*COEFF_NUM,
        NV_GNSS_NAME, process_gnss_self_study
    }
};

/* 通过该顺序来读取ini文件， 其中的顺序不能打乱，原因看deal_min_max_val 注释的策略1 */
ini_file_str gst_dcxo_ini_para[] = {
    { "a0_min", COEFF_A0, INIT_TYPE_MIN, A0_EXCEP_PROT_MIN, A0_EXCEP_PROT_MAX, DEF_A0_MIN },
    { "a0_max", COEFF_A0, INIT_TYPE_MAX, A0_EXCEP_PROT_MIN, A0_EXCEP_PROT_MAX, DEF_A0_MAX },
    { "a1_min", COEFF_A1, INIT_TYPE_MIN, A1_EXCEP_PROT_MIN, A1_EXCEP_PROT_MAX, DEF_A1_MIN },
    { "a1_max", COEFF_A1, INIT_TYPE_MAX, A1_EXCEP_PROT_MIN, A1_EXCEP_PROT_MAX, DEF_A1_MAX },
    { "a2_min", COEFF_A2, INIT_TYPE_MIN, A2_EXCEP_PROT_MIN, A2_EXCEP_PROT_MAX, DEF_A2_MIN },
    { "a2_max", COEFF_A2, INIT_TYPE_MAX, A2_EXCEP_PROT_MIN, A2_EXCEP_PROT_MAX, DEF_A2_MAX },
    { "a3_min", COEFF_A3, INIT_TYPE_MIN, A3_EXCEP_PROT_MIN, A3_EXCEP_PROT_MAX, DEF_A3_MIN },
    { "a3_max", COEFF_A3, INIT_TYPE_MAX, A3_EXCEP_PROT_MIN, A3_EXCEP_PROT_MAX, DEF_A3_MAX },
    { "t0_min", COEFF_T0, INIT_TYPE_MIN, T0_EXCEP_PROT_MIN, T0_EXCEP_PROT_MAX, DEF_T0_MIN },
    { "t0_max", COEFF_T0, INIT_TYPE_MAX, T0_EXCEP_PROT_MIN, T0_EXCEP_PROT_MAX, DEF_T0_MAX }
};

/* 函数实现 */
oal_int32 dcxo_data_buf_malloc(oal_void)
{
    oal_uint8 *buffer = NULL;

    buffer = OS_KZALLOC_GFP(DCXO_CALI_DATA_BUF_LEN);
    if (buffer == NULL) {
        return -EFAIL;
    }
    pucDcxoDataBuf = buffer;

    return SUCC;
}

void dcxo_data_buf_free(void)
{
    if (pucDcxoDataBuf != NULL) {
        OS_MEM_KFREE(pucDcxoDataBuf);
        pucDcxoDataBuf = NULL;
    }
}

oal_void *get_dcxo_data_buf_addr(oal_void)
{
    return pucDcxoDataBuf;
}

nv_part_str *get_nv_part_from_id(oal_int32 part_id)
{
    oal_int32 loop;
    for (loop = 0; loop < OAL_ARRAY_SIZE(gst_dcxo_nv_part); loop++) {
        if (gst_dcxo_nv_part[loop].part_id == part_id) {
            return &(gst_dcxo_nv_part[loop]);
        }
    }

    return NULL;
}

/*
 * 函 数 名  : deal_min_max_val
 * 功能描述  : 判断从ini文件中读取的数据是否有效，并将数据存到g_st_dcxo_info.check_value中
 *             判断数据有效的判断策略:
 *             1 从ini中读到的max 一定要比min要大，比如t0_max要比t0_min要大，否则t0_min, t0_max都视为无效数据
 *             2 从ini中读到的每一个值，都会做一次异常值判断，不能超过软件定义的最大范围。
 *
 *             无效数据的处理:
 *             使用软件定义的默认参数。
 *
 *             由于数组每次是读取一个项，所以实现策略1，需要先保存上一个值，再做判断。
 *             具体的实现方法:
 *             按照gst_dcxo_ini_para数组定义的数据结构，每次都是先读min值，再读max值，最终结果存到check_value数组中
 *             此处以t0举例:
 *             当读t0_min时，将读取到的值存放到内存中check_value[0][INIT_TYPE_MIN](第一个0表明是第一组T0),同时将
 *             t0的默认值，存放到check_value[0][INIT_TYPE_MAX]
 *
 *             当读t0_max时, 先判断该值是否大于check_value[0][INIT_TYPE_MIN],如果不是，表明出现异常，
 *             先用check_value[0][INIT_TYPE_MAX]值，恢复t0_min的默认值，然后将INIT_TYPE_MAX位置赋值t0_max初始值
 *             如果是， 将INIT_TYPE_MAX位置，赋值为ini中读取的值。
 * 输入参数  : c_para_pos表明在gst_dcxo_ini_para数组中第几个项的index
 *             l_val   从ini文件中读取到值
 * 返 回 值  : 读取内容有效，返回INI_SUCC，否则返回INI_FAILED
 */
oal_int32 deal_min_max_val(oal_int8 c_para_pos, oal_int32 l_val)
{
    oal_int8 c_chk_arr_pos;
    oal_int32 l_pair_pos;
    oal_int32 *al_check_coeff;

    c_chk_arr_pos = gst_dcxo_ini_para[c_para_pos].c_index;
    l_pair_pos = gst_dcxo_ini_para[c_para_pos].l_type;
    al_check_coeff = dcxo_info.check_value[c_chk_arr_pos];

    PS_PRINT_INFO("[dcxo] read ini file [index,pos] [%d,%d] value %d\n", c_chk_arr_pos, l_pair_pos, l_val);
    // 对min值做处理，将min的默认值保存在check_value[INIT_TYPE_MAX]位置
    if (l_pair_pos == INIT_TYPE_MIN) {
        al_check_coeff[INIT_TYPE_MIN] = l_val;
        al_check_coeff[INIT_TYPE_MAX] = gst_dcxo_ini_para[c_para_pos].l_def_val;
    }

    // 如果此时读的是max，并且不满足min < max, 需要将check_value中min的位置，用默认值替换
    if ((l_pair_pos == INIT_TYPE_MAX) && (l_val <= al_check_coeff[INIT_TYPE_MIN])) {
        al_check_coeff[INIT_TYPE_MIN] = al_check_coeff[INIT_TYPE_MAX];
        al_check_coeff[INIT_TYPE_MAX] = gst_dcxo_ini_para[c_para_pos].l_def_val;
        PS_PRINT_ERR("[dcxo][ini] get par array id [%d] val[%d] min >= max \n", c_para_pos, l_val);
        return INI_FAILED;
    }

    // min max的异常范围检测
    if ((l_val > gst_dcxo_ini_para[c_para_pos].l_prot_max) || (l_val < gst_dcxo_ini_para[c_para_pos].l_prot_min)) {
        PS_PRINT_ERR("[dcxo][ini] get id [%d] val[%d], not limit between [%d]<->[%d]\n",
                     c_para_pos, l_val, gst_dcxo_ini_para[c_para_pos].l_prot_min,
                     gst_dcxo_ini_para[c_para_pos].l_prot_max);

        al_check_coeff[l_pair_pos] = gst_dcxo_ini_para[c_para_pos].l_def_val;
        return INI_FAILED;
    }

    al_check_coeff[l_pair_pos] = l_val;

    return INI_SUCC;
}

/*
 * 函 数 名  : read_tcxo_dcxo_mode_from_ini_file
 * 功能描述  : 从ini文件中读取 tcxo or dcxo的模式
 * 返 回 值  : 当前使用的模式
 */
oal_int32 read_tcxo_dcxo_mode_from_ini_file(void)
{
    int8 ac_mode_buff[INI_BUF_SIZE] = {0};
    oal_int32 l_mode;

    // 从ini文件中读取 "tcxo_dcxo_mode" label
    if (get_cust_conf_string(INI_MODU_PLAT, TCXO_DCXO_MODE, ac_mode_buff, OAL_SIZEOF(ac_mode_buff)) != INI_SUCC) {
        PS_PRINT_ERR("[dcxo] read dcxo_tcxo_mode from ini file failed, use default tcxo mode\n");
        dcxo_info.dcxo_tcxo_mode = INI_TCXO_MODE;
        return INI_TCXO_MODE;
    }

    if (OAL_STRNCMP(INI_DCXO_STR, ac_mode_buff, OAL_STRLEN(INI_DCXO_STR)) == 0) {
        l_mode = INI_DCXO_MODE;
    } else {
        l_mode = INI_TCXO_MODE;
    }

    dcxo_info.dcxo_tcxo_mode = l_mode;

    return l_mode;
}

/*
 * 函 数 名  : read_min_max_val_from_ini_file
 * 功能描述  : 从ini文件中读取 min/max参数的值
 */
void read_min_max_val_from_ini_file(void)
{
    oal_int8 loop;
    oal_int8 c_index;
    oal_int32 l_val;
    oal_int32 l_type;

    for (loop = 0; loop < OAL_ARRAY_SIZE(gst_dcxo_ini_para); loop++) {
        if (get_cust_conf_int32(INI_MODU_PLAT, gst_dcxo_ini_para[loop].pc_name, &l_val) != INI_SUCC) {
            // 如果读取ini文件失败, 使用默认配置
            c_index = gst_dcxo_ini_para[loop].c_index;
            l_type = gst_dcxo_ini_para[loop].l_type;
            dcxo_info.check_value[c_index][l_type] = gst_dcxo_ini_para[loop].l_def_val;
            PS_PRINT_ERR("[dcxo] [index,pos][%d,%d] read from ini failed, use default value %d\n",
                         c_index, l_type, gst_dcxo_ini_para[loop].l_def_val);
            continue;
        }

        if (deal_min_max_val(loop, l_val) == INI_FAILED) {
            PS_PRINT_ERR("[dcxo] deal para arry id[%d], min-max error\n", loop);
        }
    }
}

void read_pmu_type_from_ini_file(void)
{
    int8 ac_type_buff[INI_BUF_SIZE] = {0};
    dcxo_info.pmu_chip_type = PMU_HI655V300;

    if (get_cust_conf_string(INI_MODU_PLAT, INI_PMU_CHIP_TYPE, ac_type_buff, OAL_SIZEOF(ac_type_buff)) != INI_SUCC) {
        PS_PRINT_ERR("[dcxo] read pmu type from ini failed, use default HI655V300\n");
        return;
    }

    if (OAL_STRNCMP(INI_6555V300_STR, ac_type_buff, OAL_STRLEN(INI_6555V300_STR)) == 0) {
        dcxo_info.pmu_chip_type = PMU_HI655V300;
        PS_PRINT_INFO("[dcxo] use pmu %s", INI_6555V300_STR);
    } else if (OAL_STRNCMP(INI_6421V700_STR, ac_type_buff, OAL_STRLEN(INI_6421V700_STR)) == 0) {
        dcxo_info.pmu_chip_type = PMU_HI6421V700;
        PS_PRINT_INFO("[dcxo] use pmu %s", INI_6421V700_STR);
    } else {
        PS_PRINT_ERR("[dcxo] read pmu info %s not vld, use default HI655V300\n", ac_type_buff);
    }
}

void read_tcxo_dcxo_ini_file(void)
{
    oal_int32 l_ret;

    // 先判断如果是tcxo 模式，不需要继续min-max参数
    l_ret = read_tcxo_dcxo_mode_from_ini_file();
    if (l_ret == INI_TCXO_MODE) {
        return;
    }

    read_min_max_val_from_ini_file();
    read_pmu_type_from_ini_file();
}

int32 get_tcxo_dcxo_mode(void)
{
    return dcxo_info.dcxo_tcxo_mode;
}

/*
 * 函 数 名  : report_chr_err
 * 功能描述  : 上报CHR事件
 * 输入参数  : chr_errno 上报的事件id,
 *             chr_ptr 上报内容的首地址，
 *             chr_len 上报内容的长度
 */
void report_chr_err(oal_int32 chr_errno, oal_uint8 *chr_ptr, oal_uint16 chr_len)
{
    switch (chr_errno) {
        case PLAT_CHR_ERRNO_DCXO_NV_RD:
            if (chr_len != OAL_SIZEOF(plat_chr_nv_rd_excep_stru)) {
                PS_PRINT_ERR("[dcxo] [chr] id [%d] input len [%d] not match [%ld]\n",
                             chr_errno, chr_len, OAL_SIZEOF(plat_chr_nv_rd_excep_stru));

                return;
            }

            if (dcxo_info.chr_nv_read_err > PLAT_MAX_REPORT_CNT) {
                PS_PRINT_ERR("[dcxo] [chr] id [%d] exceed max report\n", chr_errno);
                return;
            }

            dcxo_info.chr_nv_read_err++;

            CHR_EXCEPTION_P(chr_errno, chr_ptr, chr_len);
            break;

        default:
            PS_PRINT_ERR("[dcxo] [chr] id [%d] not support\n", chr_errno);
            return;
    }

    PS_PRINT_INFO("[dcxo] [chr] id [%d] repot, report cnt %d\n", chr_errno, dcxo_info.chr_nv_read_err);
}

oal_int32 read_coeff_from_nvram(oal_uint8 *pc_out, nv_part_str *nv_para)
{
    struct hisi_nve_info_user info;
    oal_int32 ret = INI_FAILED;

    if (nv_para->part_id < 0) {  // 对于小于0的nv id为虚拟配置，目前用来配置dcxo默认值
        ret = memcpy_s(pc_out, MALLOC_LEN, default_coeff, nv_para->read_size);
        if (ret != EOK) {
            PS_PRINT_ERR("read_coeff_from_nvram: memcpy_s failed, ret = %d", ret);
            return INI_FAILED;
        }
        return INI_SUCC;
    } else {
        memset_s(&info, OAL_SIZEOF(info), 0, OAL_SIZEOF(info));
        ret = memcpy_s(info.nv_name, sizeof(info.nv_name), nv_para->name, OAL_STRLEN(nv_para->name));
        if (ret != EOK) {
            PS_PRINT_ERR("read_coeff_from_nvram: memcpy_s failed, ret = %d", ret);
            return INI_FAILED;
        }
        info.nv_operation = NVRAM_READ;
        info.valid_size = nv_para->total_size;
        info.nv_number = nv_para->part_id;

        ret = hisi_nve_direct_access(&info);

        if (ret == INI_SUCC) {
            ret = memcpy_s(pc_out, MALLOC_LEN, info.nv_data, nv_para->read_size);
            if (ret != EOK) {
                PS_PRINT_ERR("read_coeff_from_nvram: memcpy_s failed, ret = %d", ret);
                return INI_FAILED;
            }
            PS_PRINT_INFO("[dcxo] read_conf_from_nvram::nvram id[%d], nv name[%s], size[%d]\r\n!",
                          info.nv_number, info.nv_name, info.valid_size);
        } else {
            PS_PRINT_ERR("[dcxo] read nvm [%d] %s failed", info.nv_number, info.nv_name);
            return INI_FAILED;
        }
    }

    return INI_SUCC;
}

oal_int32 nv_coeff_vld(oal_int32 *pl_coeff, oal_uint32 size, char *nv_name)
{
    oal_int32 loop;
    oal_int32 l_invld_cnt = 0;
    for (loop = 0; loop < size; loop++) {
        if ((*pl_coeff < dcxo_info.check_value[loop][INIT_TYPE_MIN]) ||
            (*pl_coeff > dcxo_info.check_value[loop][INIT_TYPE_MAX])) {
            PS_PRINT_ERR("[dcxo] get nv[%s] coeff [%d], value %d, check exceed min[%d] <-> max[%d] scope \n",
                         nv_name, loop, *pl_coeff, dcxo_info.check_value[loop][INIT_TYPE_MIN],
                         dcxo_info.check_value[loop][INIT_TYPE_MAX]);

            l_invld_cnt++;
        }

        PS_PRINT_INFO("[dcxo] read nv[%s] nv a[%d] = Dec(%d) Hex(0x%x)\n", nv_name, loop, *pl_coeff, *pl_coeff);

        pl_coeff++;
    }

    if (l_invld_cnt > 0) {
        return INI_FAILED;
    }

    return INI_SUCC;
}


/*
 * 函 数 名  : update_dcxo_coeff
 * 功能描述  : 当系统起来之后，动态更新内存中三项式系数a0~a3
 * 输入参数  : 传入需要更新参数的buffer地址
 */
oal_void update_dcxo_coeff(oal_int32 *coeff, oal_uint32 coeff_cnt)
{
    dcxo_dl_para_stru *pst_para = NULL;
    oal_int32 ret;

    if (coeff == NULL) {
        return;
    }

    pst_para = get_dcxo_data_buf_addr();
    if (pst_para == NULL) {
        PS_PRINT_ERR("[dcxo] update coeff input pst_para is NULL\n");
        return;
    }

    if (nv_coeff_vld(coeff, COEFF_NUM, "update_dcxo_coeff") != INI_SUCC) {
        PS_PRINT_ERR("[dcxo] update coeff invlid\n");
        return;
    }

    ret = memcpy_s(pst_para->coeff_para, sizeof(pst_para->coeff_para), coeff, coeff_cnt);
    if (ret != EOK) {
        PS_PRINT_ERR("[dcxo] memcpy_s failed, ret = %d\n", ret);
        return;
    }
}

oal_int32 process_plat_cali(oal_uint8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para)
{
    oal_int32 *pl_para;
    oal_uint32 pmu_type;
    oal_int32 ret;

    pst_dl_para->check_data = DCXO_CHECK_MAGIC;
    pst_dl_para->tcxo_dcxo_flag = INI_DCXO_MODE;

    // 初始化成默认的三项式系数
    pl_para = pst_dl_para->coeff_para;
    ret = memcpy_s(pl_para, sizeof(pst_dl_para->coeff_para), pc_buffer, sizeof(default_coeff));
    if (ret != EOK) {
        PS_PRINT_ERR("[dcxo] process_plat_cali, memcpy_s failed, ret = %d \n", ret);
        return INI_FAILED;
    }

    // 根据ini获取到当前的PMU 偏移地址
    pmu_type = dcxo_info.pmu_chip_type;
    ret = memcpy_s(&(pst_dl_para->pmu_addr), sizeof(dcxo_pmu_addr_stru),
                   &st_pmu_addr[pmu_type], sizeof(dcxo_pmu_addr_stru));
    if (ret != EOK) {
        PS_PRINT_ERR("[dcxo] process_plat_cali, memcpy_s failed, ret = %d \n", ret);
        return INI_FAILED;
    }

    return INI_SUCC;
}

oal_int32 process_factory_cali(oal_uint8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para)
{
    oal_int32 *pl_coeff = NULL;
    plat_chr_nv_rd_excep_stru chr_nv_rd;
    oal_int32 ret;
    oal_uint16 crc_relt;

    ret = memcpy_s(&factory, OAL_SIZEOF(dcxo_nv_info_stru), pc_buffer, OAL_SIZEOF(dcxo_nv_info_stru));
    if (ret != EOK) {
        PS_PRINT_ERR("[dcxo] process_factory_cali: memcpy_s faild, ret = %d\n", ret);
        return INI_FAILED;
    }

    pl_coeff = factory.coeff_para;

    if (factory.valid != DCXO_NV_CHK_OK) {
        PS_PRINT_ERR("[dcxo] factory nv check not valid\n");
        goto report_chr;
    }

    // 此处检测到工厂分区t0使用的是10位，需要修正到16位
    if (factory.t0_bit_width_fix == T0_BIT_WIDTH_10) {
        pl_coeff[COEFF_T0] *= T0_WIDTH10_TO_WIDTH16;
    }

    /* 判断工厂分区读出的参数，是否在有效的min-max区间 */
    if (nv_coeff_vld(pl_coeff, COEFF_NUM, NV_FAC_NAME) == INI_FAILED) {
        PS_PRINT_ERR("[dcxo] factory coeff data check failed\n");
        goto report_chr;
    }

    // dcxo_nv_info_stru结构体最后一个4字节用作crc校验，所以长度需要减去该部分
    crc_relt = cal_crc_16((oal_uint8 *)&factory, sizeof(dcxo_nv_info_stru) - sizeof(oal_int32));
    if (crc_relt != (factory.crc_relt & FAC_NV_CRC_MASK)) {
        PS_PRINT_ERR("[dcxo] crc calc failed, calc %d, read %d\n",
                     crc_relt, factory.crc_relt);

        // 老版本的nv配置，没有使用crc校验, 直接返回，不需要上报chr
        if ((factory.crc_relt == 0) && (pl_coeff[COEFF_T0] == DCXO_PARA_T0_30)) {
            PS_PRINT_INFO("[dcxo] old nv version don't report chr");
            return INI_FAILED;
        }

        goto report_chr;
    }

    PS_PRINT_INFO("[dcxo] prcess_factory crc calc succ, calc 0x%x, read 0x%x\n",
                     crc_relt, factory.crc_relt);

    ret = memcpy_s(pst_dl_para->coeff_para, sizeof(pst_dl_para->coeff_para),
                   pl_coeff, COEFF_NUM * sizeof(oal_int32));
    if (ret != EOK) {
        PS_PRINT_ERR("[dcxo] process_factory_cali: memcpy_s faild, ret = %d", ret);
        goto report_chr;
    }

    return INI_SUCC;

report_chr:
    chr_nv_rd.l_struct_len = OAL_SIZEOF(plat_chr_nv_rd_excep_stru);
    ret = memcpy_s(chr_nv_rd.l_para, sizeof(chr_nv_rd.l_para), pl_coeff, COEFF_NUM * sizeof(oal_int32));
    if (ret == EOK) {
        report_chr_err(PLAT_CHR_ERRNO_DCXO_NV_RD, (oal_uint8 *)&chr_nv_rd, OAL_SIZEOF(chr_nv_rd));
    }
    return INI_FAILED;
}

oal_int32 process_gnss_self_study(oal_uint8 *pc_buffer, dcxo_dl_para_stru *pst_dl_para)
{
    oal_int32 *pl_coeff;
    dcxo_nv_info_stru *pst_info;
    plat_chr_nv_rd_excep_stru chr_nv_rd;
    oal_int32 ret;

    pst_info = (dcxo_nv_info_stru *)pc_buffer;
    pl_coeff = pst_info->coeff_para;

    if (pst_info->valid != DCXO_NV_CHK_OK) {
        PS_PRINT_ERR("[dcxo] gnss nv check not valid \n");
        goto report_chr;
    }

    /* 判断gnss自学习分区读出的参数，是否在有效的min-max区间 */
    if (nv_coeff_vld(pl_coeff, COEFF_NUM, NV_GNSS_NAME) == INI_FAILED) {
        PS_PRINT_ERR("[dcxo] factory coeff data check failed\n");
        goto report_chr;
    }

    ret = memcpy_s(pst_dl_para->coeff_para, sizeof(pst_dl_para->coeff_para),
                   pl_coeff, COEFF_NUM * OAL_SIZEOF(oal_int32));
    if (ret != EOK) {
        PS_PRINT_ERR("[dcxo] process_gnss_self_study, memcpy_s failed, ret = %d\n", ret);
        goto report_chr;
    }

    return INI_SUCC;

report_chr:
    memset_s(&chr_nv_rd, OAL_SIZEOF(plat_chr_nv_rd_excep_stru), 0, OAL_SIZEOF(plat_chr_nv_rd_excep_stru));
    chr_nv_rd.l_struct_len = OAL_SIZEOF(plat_chr_nv_rd_excep_stru);
    ret = memcpy_s(chr_nv_rd.l_para, sizeof(chr_nv_rd.l_para), pl_coeff,
                   COEFF_NUM * OAL_SIZEOF(oal_int32));
    if (ret == EOK) {
        report_chr_err(PLAT_CHR_ERRNO_DCXO_NV_RD, (oal_uint8 *)&chr_nv_rd, OAL_SIZEOF(chr_nv_rd));
    }
    return INI_FAILED;
}

oal_int32 get_dcxo_coeff(dcxo_dl_para_stru *pst_dl_para)
{
    oal_uint8 *buffer = NULL;
    oal_int32 ret = INI_FAILED;
    oal_int32 loop;
    oal_int32 nv_size;

    if (pst_dl_para == NULL) {
        PS_PRINT_ERR("[dcxo] pst_dl_para input is NULL \n");
        return INI_FAILED;
    }

    buffer = OS_KZALLOC_GFP(MALLOC_LEN);
    if (buffer == NULL) {
        PS_PRINT_ERR("[dcxo] alloc coeff mem failed \n");
        return INI_FAILED;
    }

    nv_size = OAL_ARRAY_SIZE(gst_dcxo_nv_part);
    // 依次遍历平台分区，工厂分区，自学习分区，读取nv内容并进行处理
    for (loop = 0; loop < nv_size; loop++) {

        // 判断需要读取的buffer大小是否会越界
        if (gst_dcxo_nv_part[loop].read_size > MALLOC_LEN) {
            PS_PRINT_ERR("[dcxo] read nv %s size exceed malloc len\n",
                         gst_dcxo_nv_part[loop].name);
            ret = INI_FAILED;
            continue;
        }

        // 读取nv中的数据
        ret = read_coeff_from_nvram(buffer, &gst_dcxo_nv_part[loop]);
        if (ret == INI_FAILED) {
            memset_s(buffer, MALLOC_LEN, 0, MALLOC_LEN);
            continue;
        }

        // 对nv项内容进行处理
        if (gst_dcxo_nv_part[loop].process_cali != NULL) {
            ret = gst_dcxo_nv_part[loop].process_cali(buffer, pst_dl_para);
            if (ret != INI_SUCC) {
                memset_s(buffer, MALLOC_LEN, 0, MALLOC_LEN);
                continue;
            }

            // 记录成功写入到下载buffer中的nv号
            dcxo_info.use_part_id = gst_dcxo_nv_part[loop].part_id;
        }

        memset_s(buffer, MALLOC_LEN, 0, MALLOC_LEN);
    }

    OS_MEM_KFREE(buffer);
    return ret;
}

oal_void show_dcxo_conf_info(dcxo_dl_para_stru *pst_dl_para)
{
    oal_int32 loop;
    nv_part_str *nv_part;
    dcxo_pmu_addr_stru *pmu_addr = NULL;

    nv_part = get_nv_part_from_id(dcxo_info.use_part_id);

    PS_PRINT_INFO("[dcxo] check data = 0x%x use dcxo mode, nv id = %d, name = %s cali buffer len %ld\n",
                  pst_dl_para->check_data, dcxo_info.use_part_id, (nv_part == NULL) ? NULL : nv_part->name,
                  DCXO_CALI_DATA_BUF_LEN);

    for (loop = 0; loop < COEFF_NUM; loop++) {
        PS_PRINT_INFO("[dcxo] a[%d] = 0x%x \n", loop, pst_dl_para->coeff_para[loop]);
    }

    pmu_addr = &(pst_dl_para->pmu_addr);
    PS_PRINT_INFO("[dcxo] pmu info  cali 0x%x, av0 0x%x av1 0x%x av2 0x%x av3 0x%x\n",
                  pmu_addr->cali_end, pmu_addr->auto_cali_ave0, pmu_addr->auto_cali_ave1,
                  pmu_addr->auto_cali_ave2, pmu_addr->auto_cali_ave3);

    PS_PRINT_INFO("[dcxo] pmu info  ana 0x%x, state 0x%x low byte 0x%x high bytpe 0x%x type %d\n",
                  pmu_addr->wifi_ana_en, pmu_addr->xoadc_state, pmu_addr->reg_addr_low,
                  pmu_addr->reg_addr_high, pmu_addr->pmu_type);
}

oal_int32 read_dcxo_cali_data(oal_void)
{
    dcxo_dl_para_stru *pst_dl_para;

    pst_dl_para = get_dcxo_data_buf_addr();
    if (pst_dl_para == NULL) {
        PS_PRINT_ERR("[dcxo][read_dcxo_cali_data] pst_para is NULL\n");
        return INI_FAILED;
    }

    if (get_tcxo_dcxo_mode() == INI_TCXO_MODE) {
        PS_PRINT_INFO("[dcxo] use tcxo mode\n");
        return INI_SUCC;
    }

    if (get_dcxo_coeff(pst_dl_para) == INI_FAILED) {
        PS_PRINT_ERR("[dcxo] not read coeff from gnss part\n");
    }

    show_dcxo_conf_info(pst_dl_para);

    return INI_SUCC;
}
