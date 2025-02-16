#ifndef _SCHED_H
#define _SCHED_H

#define HZ 100

#define NR_TASKS	64
#define TASK_SIZE	0x04000000			// 64MB
#define LIBRARY_SIZE	0x00400000

#if (TASK_SIZE & 0x3fffff)
#error "TASK_SIZE must be multiple of 4M"
#endif

#if (LIBRARY_SIZE & 0x3fffff)
#error "LIBRARY_SIZE must be a multiple of 4M"
#endif

#if (LIBRARY_SIZE >= (TASK_SIZE/2))
#error "LIBRARY_SIZE too damn big!"
#endif

#if (((TASK_SIZE>>16)*NR_TASKS) != 0x10000)
#error "TASK_SIZE*NR_TASKS must be 4GB"
#endif

#define LIBRARY_OFFSET (TASK_SIZE - LIBRARY_SIZE)

#define CT_TO_SECS(x)	((x) / HZ)
#define CT_TO_USECS(x)	(((x) % HZ) * 1000000/HZ)

#define FIRST_TASK task[0]
#define LAST_TASK task[NR_TASKS-1]

#include <linux/head.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#if (NR_OPEN > 32)
#error "Currently the close-on-exec-flags and select masks are in one long, max 32 files/proc"
#endif

#define TASK_RUNNING			0		// 进程正在运行或已准备就绪
#define TASK_INTERRUPTIBLE		1		// 进程处于可中断等待状态
#define TASK_UNINTERRUPTIBLE	2		// 进程处于不可中断等待状态, 主要用于 I/O 操作
#define TASK_ZOMBIE				3		// 进程处于僵死状态, 已经停止运行, 但是父进程还没发信号
#define TASK_STOPPED			4		// 进程已停止

#ifndef NULL
#define NULL ((void *) 0)
#endif

extern int copy_page_tables(unsigned long from, unsigned long to, long size);
extern int free_page_tables(unsigned long from, unsigned long size);

extern void sched_init(void);
extern void schedule(void);
extern void trap_init(void);
extern void panic(const char * str);
extern int tty_write(unsigned minor,char * buf,int count);

typedef int (*fn_ptr)();

struct i387_struct {
	long	cwd;
	long	swd;
	long	twd;
	long	fip;
	long	fcs;
	long	foo;
	long	fos;
	long	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
};

struct tss_struct {
	long	back_link;	/* 16 high bits zero */
	long	esp0;
	long	ss0;		/* 16 high bits zero */
	long	esp1;
	long	ss1;		/* 16 high bits zero */
	long	esp2;
	long	ss2;		/* 16 high bits zero */
	long	cr3;
	long	eip;
	long	eflags;
	long	eax,ecx,edx,ebx;
	long	esp;
	long	ebp;
	long	esi;
	long	edi;
	long	es;		/* 16 high bits zero */
	long	cs;		/* 16 high bits zero */
	long	ss;		/* 16 high bits zero */
	long	ds;		/* 16 high bits zero */
	long	fs;		/* 16 high bits zero */
	long	gs;		/* 16 high bits zero */
	long	ldt;		/* 16 high bits zero */
	long	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
	struct i387_struct i387;
};

struct task_struct {
/* these are hardcoded - don't touch */
	long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
					// 任务的运行状态 -1 -> 不可运行, 0 -> 可运行,  >0 已停止
	long counter;	// 任务运行时间计数, 时间片
	long priority;	// 优先数, 任务开始运行时 counter=priority, 越大运行时间越长
	long signal;	// 信号位图, 每个bit位代表一种信号, 信号值=位偏移值+1, 最多有32个信号
	struct sigaction sigaction[32];	// 信号执行属性结构, 对应信号将要执行的操作和标志信息
	long blocked;	/* bitmap of masked signals */
					// 进程信号屏蔽码, 对应信号位图
/* various fields */
	int exit_code;	// 任务停止后的推出码, 其父进程会来取
	unsigned long start_code,end_code,end_data,brk,start_stack;
					// start_code 	代码段地址
					// end_code 	代码长度, 单位: 字节
					// end_data 	代码长度+数据长度, 单位: 字节
					// brk 			总长度
					// start_stack	堆栈段地址
	long pid,pgrp,session,leader;
					// pid 			进程标识号
					// pgrp			进程组号
					// session		会话号
					// leader		会话首领
	int	groups[NGROUPS];	// 进程所属组号, 一个进程可以属于多个组
	/* 
	 * pointers to parent process, youngest child, younger sibling,
	 * older sibling, respectively.  (p->father can be replaced with 
	 * p->p_pptr->pid)
	 */
	struct task_struct	*p_pptr, *p_cptr, *p_ysptr, *p_osptr;
					// p_pptr 		指向父进程的指针
					// p_cptr 		指向最新子进程的指针
					// p_ysptr 		指向比自己后创建的相邻进程的指针
					// p_osptr		指向比自己早创建的相邻进程的指针
	unsigned short uid,euid,suid;
					// uid			拥有该进程用户标识号ID
					// euid			有效用户ID, 用于指明访问文件的权力
					// suid			保存的用户ID
	unsigned short gid,egid,sgid;
					// gid			组标识号, 组ID
					// egid			有效组ID
					// sgid			保存的组ID
	unsigned long timeout,alarm;
					// timeout 		内核定时器的超时值
					// alarm		报警定时值, 滴答数
	long utime,stime,cutime,cstime,start_time;
					// utime		用户态运行时间, 滴答数
					// stime		内核态运行时间, 滴答数
					// cutime		子进程用户态运行时间, 滴答数
					// cstime		子进程内核态运行时间, 滴答数
					// start_time	进程开始运行的时刻
	struct rlimit rlim[RLIM_NLIMITS];	// 进程资源资源使用统计数组
	unsigned int flags;	/* per process flags, defined below */
					// 各进程的标志, 0.12还未使用
	unsigned short used_math;
					// 标志: 是否使用了协处理器
/* file system info */
	int tty;		/* -1 if no tty, so it must be signed */
					// 进程使用的tty终端的子设备号, -1表示没有使用
	unsigned short umask;			// 文件创建属性屏蔽位
	struct m_inode * pwd;			// 当期工作目录i节点结构指针
	struct m_inode * root;			// 根目录i节点结构指针
	struct m_inode * executable;	// 执行文件i节点结构指针
	struct m_inode * library;		// 被加载库文件i节点结构指针
	unsigned long close_on_exec;	// 执行时关闭文件句柄位图标志, 每个位代表一个文件描述符
	struct file * filp[NR_OPEN];	// 文件结构指针表, 表项号即是文件描述符的值
/* ldt for this task 0 - zero 1 - cs 2 - ds&ss */
	struct desc_struct ldt[3];		// 局部描述符, 0-空, 1-代码段cs, 2-数据段ds和堆栈段ss
/* tss for this task */
	struct tss_struct tss;			// 进程的任务状态信息段信息结构
};

/*
 * Per process flags
 */
#define PF_ALIGNWARN	0x00000001	/* Print alignment warning msgs */
					/* Not implemented yet, only for 486*/

/*
 *  INIT_TASK is used to set up the first task table, touch at
 * your own risk!. Base=0, limit=0x9ffff (=640kB)
 */
#define INIT_TASK \
/* state etc */	{ 0,15,15, \
/* signals */	0,{{},},0, \
/* ec,brk... */	0,0,0,0,0,0, \
/* pid etc.. */	0,0,0,0, \
/* suppl grps*/ {NOGROUP,}, \
/* proc links*/ &init_task.task,0,0,0, \
/* uid etc */	0,0,0,0,0,0, \
/* timeout */	0,0,0,0,0,0,0, \
/* rlimits */   { {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff},  \
		  {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff}, \
		  {0x7fffffff, 0x7fffffff}, {0x7fffffff, 0x7fffffff}}, \
/* flags */	0, \
/* math */	0, \
/* fs info */	-1,0022,NULL,NULL,NULL,NULL,0, \
/* filp */	{NULL,}, \
	{ \
		{0,0}, \
/* ldt */	{0x9f,0xc0fa00}, \
		{0x9f,0xc0f200}, \
	}, \
/*tss*/	{0,PAGE_SIZE+(long)&init_task,0x10,0,0,0,0,(long)&pg_dir,\
	 0,0,0,0,0,0,0,0, \
	 0,0,0x17,0x17,0x17,0x17,0x17,0x17, \
	 _LDT(0),0x80000000, \
		{} \
	}, \
}

extern struct task_struct *task[NR_TASKS];
extern struct task_struct *last_task_used_math;
extern struct task_struct *current;
extern unsigned long volatile jiffies;
extern unsigned long startup_time;
extern int jiffies_offset;

#define CURRENT_TIME (startup_time+(jiffies+jiffies_offset)/HZ)		// 系统的当前时间

extern void add_timer(long jiffies, void (*fn)(void));
extern void sleep_on(struct task_struct ** p);
extern void interruptible_sleep_on(struct task_struct ** p);
extern void wake_up(struct task_struct ** p);
extern int in_group_p(gid_t grp);

/*
 * Entry into gdt where to find first TSS. 0-nul, 1-cs, 2-ds, 3-syscall
 * 4-TSS0, 5-LDT0, 6-TSS1 etc ...
 */
#define FIRST_TSS_ENTRY 4
#define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY+1)
#define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))	// 任务号n对应的任务状态段选择符
#define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))	// 任务号n对应的局部描述符号表段选择符
#define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))				// 加载任务号n的任务状态段选择符到任务寄存器TR
#define lldt(n) __asm__("lldt %%ax"::"a" (_LDT(n)))				// 加载任务号n对局部描述符号表段选择符到局部描述符表寄存器LDTR
#define str(n) \
__asm__("str %%ax\n\t" \
	"subl %2,%%eax\n\t" \
	"shrl $4,%%eax" \
	:"=a" (n) \
	:"a" (0),"i" (FIRST_TSS_ENTRY<<3))
/*
 * switch_to(n) should switch tasks to task nr n, first
 * checking that n isn't the current task, in which case it does nothing.
 * This also clears the TS-flag if the task we switched to has used
 * tha math co-processor latest.
 * 1. 判断切换到的taskn是否为当前任务, 如果是则什么都不做
 * 2. ljmp到taskn的tss段, 实现任务切换
 */
#define switch_to(n) {\
struct {long a,b;} __tmp; \
__asm__("cmpl %%ecx,_current\n\t" \
	"je 1f\n\t" \
	"movw %%dx,%1\n\t" \
	"xchgl %%ecx,_current\n\t" \
	"ljmp %0\n\t" \
	"cmpl %%ecx,_last_task_used_math\n\t" \
	"jne 1f\n\t" \
	"clts\n" \
	"1:" \
	::"m" (*&__tmp.a),"m" (*&__tmp.b), \
	"d" (_TSS(n)),"c" ((long) task[n])); \
}

#define PAGE_ALIGN(n) (((n)+0xfff)&0xfffff000)

#define _set_base(addr,base) \
__asm__("movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \
	"movb %%dl,%1\n\t" \
	"movb %%dh,%2" \
	::"m" (*((addr)+2)), \
	  "m" (*((addr)+4)), \
	  "m" (*((addr)+7)), \
	  "d" (base) \
	:"dx")

#define _set_limit(addr,limit) \
__asm__("movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \
	"movb %1,%%dh\n\t" \
	"andb $0xf0,%%dh\n\t" \
	"orb %%dh,%%dl\n\t" \
	"movb %%dl,%1" \
	::"m" (*(addr)), \
	  "m" (*((addr)+6)), \
	  "d" (limit) \
	:"dx")

#define set_base(ldt,base) _set_base( ((char *)&(ldt)) , base )
#define set_limit(ldt,limit) _set_limit( ((char *)&(ldt)) , (limit-1)>>12 )

#define _get_base(addr) ({\
unsigned long __base; \
__asm__("movb %3,%%dh\n\t" \
	"movb %2,%%dl\n\t" \
	"shll $16,%%edx\n\t" \
	"movw %1,%%dx" \
	:"=d" (__base) \
	:"m" (*((addr)+2)), \
	 "m" (*((addr)+4)), \
	 "m" (*((addr)+7))); \
__base;})

#define get_base(ldt) _get_base( ((char *)&(ldt)) )

#define get_limit(segment) ({ \
unsigned long __limit; \
__asm__("lsll %1,%0\n\tincl %0":"=r" (__limit):"r" (segment)); \
__limit;})

#endif
