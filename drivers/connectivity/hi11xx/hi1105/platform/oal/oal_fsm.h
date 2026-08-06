

#ifndef __OAL_FSM_H__
#define __OAL_FSM_H__

/* 宏定义 */
#define OAL_FSM_MAX_NAME   8 /* 状态机名最大长度 */
#define OAL_FSM_MAX_STATES 100
#define OAL_FSM_MAX_EVENTS 100
#define OAL_FSM_STATE_NONE 255 /* invalid state */
#define OAL_FSM_EVENT_NONE 255 /* invalid event */

/* 状态信息结构定义 */
typedef struct __oal_fsm_state_info {
    uint32_t state;                                /* 状态ID */
    const char *name;                            /* 状态名 */
    void (*oal_fsm_entry)(void *p_ctx); /* 进入本状态的处理回调函数指针 */
    void (*oal_fsm_exit)(void *p_ctx);  /* 退出本状态的处理回调函数指针 */
    /* 本状态下的事件处理回调函数指针 */
    uint32_t (*oal_fsm_event)(void *p_ctx, uint16_t event, uint16_t event_data_len, void *event_data);
} oal_fsm_state_info;

/* 状态机结构定义 */
typedef struct __oal_fsm {
    uint8_t uc_name[OAL_FSM_MAX_NAME]; /* 状态机名字 */
    uint8_t uc_cur_state;              /* 当前状态 */
    uint8_t uc_prev_state;             /* 前一状态，发出状态切换事件的状态 */
    uint8_t uc_num_states;             /* 状态机的状态个数 */
    uint8_t uc_rsv[1];
    const oal_fsm_state_info *p_state_info;
    void *p_ctx;          /* 上下文，指向状态机实例拥有者 */
    void *p_oshandler;    /* owner指针，指向VAP或者device,由具体的状态机决定 */
    uint16_t us_last_event; /* 最后处理的事件 */
    uint8_t uc_rsv1[2];
} oal_fsm_stru;

/* 函数声明 */
extern uint32_t oal_fsm_create(void *p_oshandle, /* 状态机owner的指针，对低功耗状态机，指向VAP结构 */
                               const uint8_t *p_name,                           /* 状态机的名字 */
                               void *p_ctx,                                   /* 状态机context */
                               oal_fsm_stru *pst_oal_fsm,                         /* oal状态机内容 */
                               uint8_t uc_init_state,                           /* 初始状态 */
                               const oal_fsm_state_info *p_state_info,            /* 状态机实例指针 */
                               uint8_t uc_num_states);                          /* 本状态机的状态个数 */

extern uint32_t oal_fsm_trans_to_state(oal_fsm_stru *p_fsm, uint8_t uc_state);

extern uint32_t oal_fsm_event_dispatch(oal_fsm_stru *p_fsm, uint16_t us_event,
                                       uint16_t us_event_data_len, void *p_event_data);

#endif
