#undef TRACE_SYSTEM
#define TRACE_SYSTEM render_rt

#if !defined(_TRACE_RENDER_RT_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_RENDER_RT_H

#include <linux/tracepoint.h>
#include <securec.h>

DECLARE_EVENT_CLASS(sched_process_template,/* [false alarm]:原生宏定义 */

	TP_PROTO(struct task_struct *p),

	TP_ARGS(p),

	TP_STRUCT__entry(
		__array(	char,	comm,	TASK_COMM_LEN	)
		__field(	pid_t,	pid			)
	),

	TP_fast_assign(
		memcpy_s(__entry->comm, TASK_COMM_LEN, p->comm, TASK_COMM_LEN);
		__entry->pid		= p->pid;
	),

	TP_printk("comm=%s pid=%d",
		  __entry->comm, __entry->pid)
);

DEFINE_EVENT(sched_process_template, add_render_rthread,
         TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));

DEFINE_EVENT(sched_process_template, remove_render_rthread,
         TP_PROTO(struct task_struct *p),
	     TP_ARGS(p));


TRACE_EVENT(add_waker_to_render_rthread,/* [false alarm]:原生宏定义 */

     TP_PROTO(struct task_struct *waker_task, int waker_index,
	      struct task_struct *wakee_task, int wakee_index,
	      int waker_cnt),

     TP_ARGS(waker_task, waker_index, wakee_task, wakee_index, waker_cnt),

     TP_STRUCT__entry(
	     __array(	     char,   waker_comm,      TASK_COMM_LEN   )
	     __field(	     pid_t,  waker_pid			    )
	     __array(	     char,   wakee_comm,      TASK_COMM_LEN   )
	     __field(	     pid_t,  wakee_pid			     )
	     __field(	     int,    waker_index		)
	     __field(	     int,    wakee_index		)
	     __field(	     int,    waker_cnt		     )
     ),

     TP_fast_assign(
	     memcpy_s(__entry->waker_comm, TASK_COMM_LEN, waker_task->comm, TASK_COMM_LEN);
	     __entry->waker_pid	     = waker_task->pid;
	     memcpy_s(__entry->wakee_comm, TASK_COMM_LEN, wakee_task->comm, TASK_COMM_LEN);
	     __entry->wakee_pid	     = wakee_task->pid;
	     __entry->waker_index    = waker_index;
	     __entry->wakee_index    = wakee_index;
	     __entry->waker_cnt      = waker_cnt;
     ),

     TP_printk("waker_comm=%s waker_pid=%d wakee_comm=%s wakee_pid=%d waker_index=%d wakee_index=%d waker_cnt=%d",
	    __entry->waker_comm, __entry->waker_pid, __entry->wakee_comm, __entry->wakee_pid, __entry->waker_index,
	    __entry->wakee_index, __entry->waker_cnt)
);

TRACE_EVENT(update_rthread_util,/* [false alarm]:原生宏定义 */

     TP_PROTO(struct task_struct *task, u64 load, u64 util),

     TP_ARGS(task, load, util),

     TP_STRUCT__entry(
	     __array(	     char,   comm,      TASK_COMM_LEN	)
	     __field(	     pid_t,  pid			)
	     __field(	     u64,    load			)
	     __field(	     u64,    util			)
     ),

     TP_fast_assign(
	     memcpy_s(__entry->comm, TASK_COMM_LEN, task->comm, TASK_COMM_LEN);
	     __entry->pid	     = task->pid;
	     __entry->load    	     = load;
	     __entry->util           = util;
     ),

     TP_printk("comm=%s pid=%d load=%llu util=%llu",
	    __entry->comm, __entry->pid, __entry->load, __entry->util)
);

#endif /* _TRACE_RENDER_RT_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
