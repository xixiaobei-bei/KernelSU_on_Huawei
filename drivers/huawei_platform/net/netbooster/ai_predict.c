/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Do prediction for a new PS epoch sample using the generated
 *              the classifier module parameters.
 * Author: liubinjun@huawei.com
 * Create: 2018-05-15
 */

#include "ai_predict.h"
#include <huawei_platform/log/hw_log.h>

#ifdef CONFIG_APP_QOE_AI_PREDICT
#define INT_MAX 2147483647
#define NETWORK_STATUS_APP_QOE_NORMAL 4
#define NETWORK_STATUS_APP_QOE_GENERAL_SLOW 5
#define ONE_THOUSAND 1000

int g_slow_proba_threshold_normal_state = 540;
int g_slow_proba_threshold_slow_state = 500;
int g_slow_proba_threshold_init_state = 510;

static int judge_clf_info_int_exp(long* fake_bts_agg_value,
	long* true_bts_agg_value, long* final_clf_value, int* vote_positvie,
	int* vote_negative, classifier_info_int_exp_type* clf_info,
	const int x_smp[])
{
	int i;
	int sub_clfs_num; /* sub-clfs number */
	sub_clfs_num = clf_info->sub_clfs_num;
	for (i = 0; i < sub_clfs_num; i++) {
		/* refer: json_parse.c: parse_json_string_for_adaboost() */
		int cur_thres = clf_info->pp_param[i][PP_INDEX_0];
		int weight = clf_info->pp_param[i][PP_INDEX_1];
		int sel_feat_idx = clf_info->pp_param[i][PP_INDEX_2];
		int left_child_is_fake = clf_info->pp_param[i][PP_INDEX_3];
		int cur_judge_value; /* value: 1 or -1 */

		/* prevent overflow */
		if (x_smp[sel_feat_idx] > (INT_MAX / FLOAT_TO_INT_ENLARGE_10E3))
			return FALSE;

		/* compute final_clf_value */
		if (x_smp[sel_feat_idx] * FLOAT_TO_INT_ENLARGE_10E3 <= cur_thres)
			/* left/right child class: <= or >=   [0, 65535] */
			cur_judge_value = 1 * left_child_is_fake;
		else
			cur_judge_value = -1 * left_child_is_fake;

		*final_clf_value += (long)(cur_judge_value * weight);

		/* get clf vote result */
		if (cur_judge_value == 1) {
			/* + positive */
			*fake_bts_agg_value += (long)(cur_judge_value * weight);
			*vote_positvie += 1;
		} else {
			/* - negative */
			*true_bts_agg_value += (long)(cur_judge_value * weight);
			*vote_negative += 1;
		}
	}
	return TRUE;
}

judge_rlt_info_int_calc_type judge_single_smp_using_clf_info_int_exp(
	classifier_info_int_exp_type clf_info, const int x_smp[], int app_qoe_level)
{
	/* meaning: FALSE: not psSlow; 1.0: all as psNormal */
	judge_rlt_info_int_calc_type rlt_info1 = {
		FALSE, 0, 0.0, 1.0, 0, ADA_SUB_CLFS_NUM };

	int is_ps_slow; /* default is ps normal */
	long final_clf_value = 0;
	long fake_bts_agg_value = 0;
	long true_bts_agg_value = 0;
	int vote_negative = 0; /* ps normal: 0 */
	int vote_positvie = 0; /* ps slow: 1 */
	long tmp_int;
	int slow_proba_threshold;

	if (judge_clf_info_int_exp(&fake_bts_agg_value, &true_bts_agg_value,
		&final_clf_value, &vote_positvie, &vote_negative,
		&clf_info, x_smp) == FALSE) {
		return rlt_info1;
	}

	/* get final judge result */
	tmp_int = fake_bts_agg_value - true_bts_agg_value;
	if (tmp_int == 0)
		tmp_int = 1;
	rlt_info1.ps_slow_proba = fake_bts_agg_value * ONE_THOUSAND / tmp_int;
	rlt_info1.final_clf_value = final_clf_value;
	rlt_info1.ps_norm_proba = -true_bts_agg_value * ONE_THOUSAND / tmp_int;
	rlt_info1.vote_positvie = vote_positvie;
	rlt_info1.vote_negative = vote_negative;

	if (app_qoe_level == NETWORK_STATUS_APP_QOE_NORMAL)
		slow_proba_threshold = g_slow_proba_threshold_normal_state;
	else if (app_qoe_level == NETWORK_STATUS_APP_QOE_GENERAL_SLOW)
		slow_proba_threshold = g_slow_proba_threshold_slow_state;
	else
		slow_proba_threshold = g_slow_proba_threshold_init_state;

	if (final_clf_value > 0 && rlt_info1.ps_slow_proba > slow_proba_threshold)
		is_ps_slow = TRUE; /* ps slow */
	else
		is_ps_slow = FALSE; /* ps normal */
	rlt_info1.is_ps_slow = is_ps_slow; /* get final judge result */

	return rlt_info1;
}

void set_slow_proba_threshold(int threshold_normal, int threshold_slow,
	int threshold_init)
{
	if (threshold_normal > 0 && threshold_slow > 0 && threshold_init > 0) {
		pr_info("[AppQoe]set_slow_proba_threshold %d,%d,%d\n",
			threshold_normal, threshold_slow, threshold_init);
		g_slow_proba_threshold_normal_state = threshold_normal;
		g_slow_proba_threshold_slow_state = threshold_slow;
		g_slow_proba_threshold_init_state = threshold_init;
	}
}
#endif
