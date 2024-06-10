#
# if you want the ram-disk device, define this to be the
# size in blocks.
#
# 如果你要使用RAM盘设备的话就定义块的大小. 这里默认RAMDISK没有定义(注释掉了),
# 否则gcc编译时会带有选项"-DRAMDISK=512";
RAMDISK = #-DRAMDISK=512

AS86	=as86 -0 -a	# 8086汇编编译器和链接器, 后带的参数含义分别是:  
LD86	=ld86 -0	# -o 生成8086目标程序; -a 生成与gas和gld部分兼容的代码.

AS	=gas			# GNU汇编器和链接器.
LD	=gld
# 下面是GNU链接器gld运行时用到的选项. 含义是: -s 输出文件中省略所有的符号信息; 
# -x: 删除所有局部符号; -M: 表示需要在标准输出设备(显示器)上打印链接映像(link map), 是指由链接程序
# 产生的一种内存地址映像, 其中列出了程序段装入内存中的位置信息. 具体来讲有如下信息:
# 1. 目标文件和符号信息映射到内存中的位置;
# 2. 公共符号如何放置;
# 3. 链接中包含的所有文件成员及其引用的符号;
LDFLAGS	=-s -x -M
# gcc是GNU 程序的编译器. 对于UNIX类脚本(script)程序而言, 在引用定义的标识符时,
# 需要在前面加上$符号并使用括号()括住标识符.
CC	=gcc $(RAMDISK)
# 下面指定gcc使用的选项.选项的含义为: -Wall打印所有警告信息; -0 对代码进行优化.
# '-f标志'指定与机器无关的编译标志. 其中-fstrength-reduce 用于优化循环语句;
# -fcombine-regs 用于指明编译器在组合编译阶段把复制一个寄存器到另一个寄存器的指令组合在一起;
# -fomit-frame-pointer 指明对于无需帧指针(Frame pointer)的函数不要把帧指针保留在寄存器中,
# 这样在函数中可以避免对帧指针的操作和维护;
# -mstring-insns 是Linus在学习gcc编译器时为gcc增加的选项, 用于gcc-1.40在复制结构等操作时使用386CPU的字符串指令, 可以去掉.
CFLAGS	=-Wall -O -fstrength-reduce -fomit-frame-pointer \
-fcombine-regs -mstring-insns
# cpp是gcc的预处理程序. '-nostdinc -Iinclude'含义是不要搜索标准头文件目录下的文件, 即不用系统/usr/include/目录下的头文件, 
# 而是使用'-I'选项指定的目录或当前目录下搜索头文件;
CPP	=cpp -nostdinc -Iinclude

#
# ROOT_DEV specifies the default root-device when making the image.
# This can be either FLOPPY, /dev/xxxx or empty, in which case the
# default of /dev/hd6 is used by 'build'.
#
# ROOT_DEV 指定在创建内核映像文件(image)时所使用的默认根文件系统所在的设备,
# 可以是软盘(FLOPPY) /dev/xxxx或者干脆空着, 空着时buildu程序(在tools/目录下)就使用默认值/dev/hd6.
#
# 这里/dev/hd6对应第2个硬盘的第一个分区. 这是Linus开发Linux时自己机器上根文件系统所在的分区位置;
# /dev/hd2表示把第一个硬盘的第二分区用作交换分区.
ROOT_DEV=/dev/hd6
SWAP_DEV=/dev/hd2

# 下面是kernel目录 mm目录和fs目录所产生的目标代码文件.
# 为了方便引用, 将它们用ARCHIVES(归档文件)标识符表示.
ARCHIVES=kernel/kernel.o mm/mm.o fs/fs.o
DRIVERS =kernel/blk_drv/blk_drv.a kernel/chr_drv/chr_drv.a	# 块和字符设备库文件
MATH	=kernel/math/math.a									# 数学运算库文件
LIBS	=lib/lib.a											# 由lib/目录下的文件所编译生成的通用库文件

# 下面是make老式的隐式后缀规则
.c.s:
	$(CC) $(CFLAGS) \
	-nostdinc -Iinclude -S -o $*.s $<
.s.o:
	$(AS) -c -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) \
	-nostdinc -Iinclude -c -o $*.o $<
# 下面'all'表示创建 Makefile 所知的最顶层的目标。这里即是 Image 文件。这里生成的 Image 文件
# 即是引导启动盘映像文件 bootimage。若将其写入软盘就可以使用该软盘引导 Linux 系统了。在
# Linux 下将 Image 写入软盘的命令参见 46 行。 DOS 系统下可以使用软件 rawrite.exe
all:	Image

# 说明目标（ Image 文件）是由冒号后面的 4 个元素产生，分别是 boot/目录中的 bootsect 和 setup
# 文件、 tools/目录中的 system 和 build 文件。 43--44 行这是执行的命令。 43 行表示使用 tools 目
# 录下的 build 工具程序（下面会说明如何生成）将 bootsect、 setup 和 system 文件以$(ROOT_DEV)
# 为根文件系统设备组装成内核映像文件 Image。第 45 行的 sync 同步命令是迫使缓冲块数据立即写盘
# 并更新超级块。
Image: boot/bootsect boot/setup tools/system tools/build
	tools/build boot/bootsect boot/setup tools/system $(ROOT_DEV) \
		$(SWAP_DEV) > Image
	sync
# 表示 disk 这个目标要由 Image 产生。 dd 为 UNIX 标准命令：复制一个文件，根据选项进行转换和格
# 式化。 bs=表示一次读/写的字节数。 if=表示输入的文件， of=表示输出到的文件。这里/dev/PS0 是
# 指第一个软盘驱动器(设备文件)。在现代 Linux 系统下需使用/dev/fd0。
disk: Image
	dd bs=8192 if=Image of=/dev/PS0

tools/build: tools/build.c			# 由 tools 目录下的 build.c 程序生成执行程序 build。
	$(CC) $(CFLAGS) \
	-o tools/build tools/build.c	

boot/head.o: boot/head.s			# 利用上面给出的.s.o规则生成head.o目标文件

# 表示 tools 目录中的 system 文件要由冒号右边所列的元素生成。 57--62 行是生成 system 的命令。
# 最后的 > System.map 表示 gld 需要将连接映像重定向存放在 System.map 文件中。
# 关于 System.map 文件的用途参见注释后的说明。
tools/system:	boot/head.o init/main.o \
		$(ARCHIVES) $(DRIVERS) $(MATH) $(LIBS)
	$(LD) $(LDFLAGS) boot/head.o init/main.o \
	$(ARCHIVES) \
	$(DRIVERS) \
	$(MATH) \
	$(LIBS) \
	-o tools/system > System.map

# 数学协处理函数文件 math.a 由 64 行上的命令实现：进入 kernel/math/目录；运行 make 工具程序。
kernel/math/math.a:
	(cd kernel/math; make)

# 生成块设备库文件 blk_drv.a，其中含有可重定位目标文件。
kernel/blk_drv/blk_drv.a:
	(cd kernel/blk_drv; make)

# 生成字符设备函数文件 chr_drv.a。
kernel/chr_drv/chr_drv.a:
	(cd kernel/chr_drv; make)

# 内核目标模块 kernel.o
kernel/kernel.o:
	(cd kernel; make)

# 内存管理模块 mm.o
mm/mm.o:
	(cd mm; make)

# 文件系统目标模块 fs.o
fs/fs.o:
	(cd fs; make)

# 库函数 lib.a
lib/lib.a:
	(cd lib; make)

# 使用 8086 汇编和链接器对 setup.s 编译，生成 setup.o 文件。 -s 选项表示需去除目标中的符号信息。
boot/setup: boot/setup.s
	$(AS86) -o boot/setup.o boot/setup.s
	$(LD86) -s -o boot/setup boot/setup.o

# 执行预处理器程序，替换*.S 文件中的宏生成对应的*.s 文件。
boot/setup.s:	boot/setup.S include/linux/config.h
	$(CPP) -traditional boot/setup.S -o boot/setup.s


boot/bootsect.s:	boot/bootsect.S include/linux/config.h
	$(CPP) -traditional boot/bootsect.S -o boot/bootsect.s

# 同上。生成 bootsect.o 磁盘引导块。
boot/bootsect:	boot/bootsect.s
	$(AS86) -o boot/bootsect.o boot/bootsect.s
	$(LD86) -s -o boot/bootsect boot/bootsect.o
	
# 当执行'make clean'时，就会执行 101--107 行上的命令，去除所有编译连接生成的文件。
# 'rm'是文件删除命令，选项-f 含义是忽略不存在的文件，并且不显示删除信息。
clean:
	rm -f Image System.map tmp_make core boot/bootsect boot/setup \
		boot/bootsect.s boot/setup.s
	rm -f init/*.o tools/system tools/build boot/*.o
	(cd mm;make clean)
	(cd fs;make clean)
	(cd kernel;make clean)
	(cd lib;make clean)

# 该规则将首先执行上面的 clean 规则，然后对 linux/目录进行压缩，生成'backup.Z'压缩文件。
# 'cd .. '表示退到 linux/的上一级（父）目录； 'tar cf - linux'表示对 linux/目录执行 tar 归档
# 程序。 '-cf'表示需要创建新的归档文件 '| compress -'表示将 tar 程序的执行通过管道操作('|')
# 传递给压缩程序 compress，并将压缩程序的输出存成 backup.Z 文件。
backup: clean
	(cd .. ; tar cf - linux | compress - > backup.Z)
	sync

# 该目标或规则用于产生各文件之间的依赖关系。创建这些依赖关系是为了让 make 命令用它们来确定
# 是否需要重建一个目标对象。比如当某个头文件被改动过后， make 就能通过生成的依赖关系，重新
# 编译与该头文件有关的所有*.c 文件。具体方法如下：
# 使用字符串编辑程序 sed 对 Makefile 文件（这里即是本文件）进行处理，输出为删除了 Makefile
# 文件中'### Dependencies'行后面的所有行，即删除了下面从 122 开始到文件末的所有行，并生成
# 一个临时文件 tmp_make（也即 114 行的作用）。然后对指定目录下（ init/）的每一个 C 文件（其实
# 只有一个文件 main.c） 执行 gcc 预处理操作。标志'-M'告诉预处理程序 cpp 输出描述每个目标文件
# 相关性的规则，并且这些规则符合 make 语法。对于每一个源文件，预处理程序会输出一个规则，其
# 结果形式就是相应源程序文件的目标文件名加上其依赖关系，即该源文件中包含的所有头文件列表。
# 然后把预处理结果都添加到临时文件 tmp_make 中，最后将该临时文件复制成新的 Makefile 文件。
# 115 行上的'$$i'实际上是'$($i) '。这里'$i'是这句前面的 shell 变量'i'的值。
dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in init/*.c;do echo -n "init/";$(CPP) -M $$i;done) >> tmp_make
	cp tmp_make Makefile
	(cd fs; make dep)
	(cd kernel; make dep)
	(cd mm; make dep)

### Dependencies:
init/main.o : init/main.c include/unistd.h include/sys/stat.h \
  include/sys/types.h include/sys/time.h include/time.h include/sys/times.h \
  include/sys/utsname.h include/sys/param.h include/sys/resource.h \
  include/utime.h include/linux/tty.h include/termios.h include/linux/sched.h \
  include/linux/head.h include/linux/fs.h include/linux/mm.h \
  include/linux/kernel.h include/signal.h include/asm/system.h \
  include/asm/io.h include/stddef.h include/stdarg.h include/fcntl.h \
  include/string.h 
