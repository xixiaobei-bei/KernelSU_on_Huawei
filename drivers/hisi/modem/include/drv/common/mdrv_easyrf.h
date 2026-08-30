#ifndef __MDRV_EASYRF_H__
#define __MDRV_EASYRF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* easyrf dump头结构 */
typedef struct
{
    unsigned char                       aucDescription[4];                      /* dump文件信息描述，填写RFIC对应的ASCII码 */
    unsigned int                        uwDumpSize;                             /* dump文件大小，包含头，单位byte */
}EASYRF_DUMP_HEAD_STRU;

#ifdef __cplusplus
}
#endif

#endif
