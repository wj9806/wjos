#include "core/syscall.h"
#include "core/task.h"
#include "core/sys.h"
#include "tools/log.h"
#include "fs/fs.h"
#include "core/memory.h"
#include "dev/time.h"
#include "base/history.h"

typedef int (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg03);

void sys_print_msg(const char * fmt, int arg)
{
	log_printf(fmt, arg);
}

static const syscall_handler_t  sys_table[] = {
    [SYS_SLEEP] 	   =  (syscall_handler_t) sys_sleep,
	[SYS_GETPID] 	   =  (syscall_handler_t) sys_getpid,
	[SYS_FORK] 	       =  (syscall_handler_t) sys_fork,
	[SYS_PRINT_MSG]    =  (syscall_handler_t) sys_print_msg,
	[SYS_EXECVE]       =  (syscall_handler_t) sys_execve,
	[SYS_YEILD]        =  (syscall_handler_t) sys_yeild,
	[SYS_EXIT]		   =  (syscall_handler_t) sys_exit,
	[SYS_WAIT]		   =  (syscall_handler_t) sys_wait,
     
	[SYS_OPEN] 		   =  (syscall_handler_t) sys_open,
	[SYS_READ] 		   =  (syscall_handler_t) sys_read,
	[SYS_WRITE] 	   =  (syscall_handler_t) sys_write,
	[SYS_CLOSE] 	   =  (syscall_handler_t) sys_close,
	[SYS_LSEEK] 	   =  (syscall_handler_t) sys_lseek,
	[SYS_IOCTL]		   =  (syscall_handler_t) sys_ioctl,
   
	[SYS_ISATTY] 	   =  (syscall_handler_t) sys_isatty,
	[SYS_FSTAT] 	   =  (syscall_handler_t) sys_fstat,
	[SYS_SBRK] 	       =  (syscall_handler_t) sys_sbrk,
	[SYS_DUP] 	       =  (syscall_handler_t) sys_dup,
   
	[SYS_OPENDIR] 	   =  (syscall_handler_t) sys_opendir,
	[SYS_READDIR] 	   =  (syscall_handler_t) sys_readdir,
	[SYS_CLOSEDIR] 	   =  (syscall_handler_t) sys_closedir,
	[SYS_UNLINK]	   =  (syscall_handler_t) sys_unlink,

	[SYS_GETTIMEOFDAY] =  (syscall_handler_t) sys_gettimeofday,
	[SYS_GMTIME_R] 	   =  (syscall_handler_t) sys_gmtime_r,
	[SYS_POWER]		   =  (syscall_handler_t) sys_power,
	[SYS_SAVE_HISTORY] =  (syscall_handler_t) sys_save_history,
};

/**
 * 处理系统调用。该函数由系统调用函数调用
 */
void handle_syscall (syscall_frame_t * frame) {
	// 超出边界，返回错误
    if (frame->func_id < sizeof(sys_table) / sizeof(sys_table[0])) {
		syscall_handler_t handler = sys_table[frame->func_id];
		if (handler) {
			int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
			//eax可以用来放函数的返回值
			frame->eax = ret;
			return;
		}
	}

	// 不支持的系统调用，打印出错信息
	task_t * task = task_current();
	log_printf("task: %s, Unknown syscall: %d", task->name,  frame->func_id);
    // 设置系统调用的返回值，由eax传递
	frame->eax = -1;  
}