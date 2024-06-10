/*
 *  linux/kernel/printk.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * When in kernel-mode, we cannot use printf, as fs is liable to
 * point to 'interesting' things. Make a printf with fs-saving, and
 * all is well.
 */
#include <stdarg.h>
#include <stddef.h>

#include <linux/kernel.h>

static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);

int printk(const char *fmt, ...)
{
	va_list args;				// 就是 char *args
	int i;

	va_start(args, fmt);		// 执行该语句后，args中存放着fmt参数后的第一个参数的地址
	i=vsprintf(buf,fmt,args);	// 格式化输出值 buf数组
	va_end(args);				// 空的宏定义
	console_print(buf);			// 把buf中的内容在屏幕上打印
	return i;
}
