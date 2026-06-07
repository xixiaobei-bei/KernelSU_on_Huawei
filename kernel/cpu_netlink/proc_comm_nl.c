#include <cpu_netlink/cpu_netlink.h>
#define BINDER_THREAD_NAME "Binder:"

#ifdef CONFIG_HW_RTG_SCHED
#include "hwrtg/proc_state.h"
#endif

#define IAWARE_COMM_MSG_LEN 2

static void send_thread_comm_msg(int num, int pid, int tgid)
{
    int dt[IAWARE_COMM_MSG_LEN];
    dt[0] = pid;
    dt[1] = tgid;
    send_to_user(num, IAWARE_COMM_MSG_LEN, dt);
}

void iaware_proc_comm_connector(struct task_struct *task, const char *comm)
{
    int sock_num = 0;
    int pid;
    int tgid;

    if (!task || !comm)
        return;

    if (strstr(comm, BINDER_THREAD_NAME))
        sock_num = PROC_COMM;
#ifdef CONFIG_HW_RTG_SCHED
    else if (is_key_aux_comm(task, comm))
        sock_num = PROC_COMM2;
#endif

    if (!sock_num)
        return;

    get_task_struct(task);
    pid = task->pid;
    tgid = task->tgid;
    put_task_struct(task);
    send_thread_comm_msg(sock_num, pid, tgid);
}
