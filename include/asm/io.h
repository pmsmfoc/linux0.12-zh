/* 往 I/O 端口写1字节数据 */
#define outb(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))

/* 从 I/O 端口读1字节数据 */
#define inb(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
_v; \
})

/**
 * 	1.将al寄存器中的数据写入 端口号=dx 的端口
 * 	2.引入两次跳转，增加延迟
 * 	功能: 往 I/O 端口写1字节数据，并引入延时
 */
#define outb_p(value,port) \
__asm__ ("outb %%al,%%dx\n" \
		"\tjmp 1f\n" \
		"1:\tjmp 1f\n" \
		"1:"::"a" (value),"d" (port))
/**
 * 	功能: 从 I/O 端口读1字节数据, 并引入延时
 */
#define inb_p(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al\n" \
	"\tjmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:":"=a" (_v):"d" (port)); \
_v; \
})
