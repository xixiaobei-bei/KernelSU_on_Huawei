/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Ai predict head file.
 * Author: liubinjun@huawei.com
 * Create: 2018-05-15
 */

#ifndef _AI_PREDICT_H
#define _AI_PREDICT_H

/* type definition for platform migration */
#define TRUE 1
#define FALSE 0

/* features numbers in train data sets */
#define FEATURE_NUM 10

#define ADA_SUB_CLFS_NUM 300
#define QUADRUPLES 4

enum pp_para_index_enum {
	PP_INDEX_0 = 0,
	PP_INDEX_1 = 1,
	PP_INDEX_2 = 2,
	PP_INDEX_3 = 3,

	PP_INDEX_BUFF
};

/*
 * use for float to int  transformation process
 * attation: need avoid overflow. int is 32bit(4 billion) in Hisi
 */
/* for 'threshold' param. [0,65535] */
#define FLOAT_TO_INT_ENLARGE_10E3 1000
/* for 'weight' param. maybe a float number */
#define FLOAT_TO_INT_ENLARGE_10E6 1000000

/* The ClassifierInfo structure using int expression */
typedef struct classifier_info_int_exp {
	int (*pp_param)[QUADRUPLES];
	int sub_clfs_num; /* sub-clfs number */
} classifier_info_int_exp_type;

/* The ClassifierInfo structure: */
typedef struct judge_rlt_info_int_calc {
	/*
	 * if it's a ps slow yes or not
	 * when TRUE, it's a psSlow. when FALSE, it's a psNormal
	 */
	int is_ps_slow;
	/* the total judge value. Generally, when > 0, regard as a ps slow */
	int final_clf_value;

	long ps_slow_proba; /* the probability if if it's a ps slow */
	long ps_norm_proba; /* the probability if if it's a ps normal */
	int vote_positvie; /* the sub-clfs numbers vote as ps slow */
	int vote_negative; /* the sub-clfs numbers vote as ps normal */
} judge_rlt_info_int_calc_type;

/* Returns the AI(algorithm module) judgement result with Adaboost Algorithm */
judge_rlt_info_int_calc_type judge_single_smp_using_clf_info_int_exp(
	classifier_info_int_exp_type clf_info, const int x_smp[], int app_qoe_level);

#endif
