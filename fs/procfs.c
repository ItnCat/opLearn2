#include <asm/segment.h>
#include <linux/sched.h>
#include <stdarg.h>

#define  SIZE  1024		//缓冲区大小
char psbuffer[SIZE];	//定义缓冲区来存放结点信息
char membuffer[SIZE];
/* 将整型数据转换为字符数据并赋值给psbuffer */
void itoa(int num, int* j) 
{
	char buffer[16] = {};	//定义一个缓冲区来临时存放将整型num转换成的反向的字符串
	int i = 0;
 	if (num == 0)
 	{
		psbuffer[*j] = '0';
		(*j)++;
		psbuffer[*j] = '\t';
		(*j)++;
		return;
	}
	if (num<0)
	{
		psbuffer[*j] = '-';
		(*j)++;
		num = -num;
	}
	
	//将整型数据num转换成反向字符串，保存到buffer中
	//i同时表示num转换成的字符的个数
	for(i = 0; num != 0; i++)
	{
		buffer[i] = (char)(num % 10 + '0'); 	//将num的个位数转换成字符数据放到buffer中
		num = num / 10;		//去掉个位数
	}
	
	//反向将buffer中的字符放入到psbuffer中
	//数组下标是此0开始计数的，所以首先需要将i减1
	for(--i; i >= 0; i--)
	{
		psbuffer[*j] = buffer[i];
		(*j) ++;
	}
	psbuffer[*j] = '\t';		//此处加制表符是为来显示时将数据对齐，以便于查看
	(*j)++;
}

/* 向数组psbuffer中添加标题 */
/* 参数j为在psbuffer中写数据的偏移位置 */
void addTitle(char* title, int* j)
{
	//这里也可以使用strcat，但是由于*j的值还需要来记录偏移的位置，这样就
	//还需要使用strlen来计算*title的长度，效率可能会降低许多，
	//同时代码量也不会比下面使用的代码量少
	for( ; *title; (*j)++, *title++)
		psbuffer[*j] = *title;	//逐字节的将标题添加到psbuffer中
	
	psbuffer[*j] = '\t';		//此处加制表符是为来显示时将数据对齐，以便于查看
	(*j) ++;
}

static int sprintf(char *printbuf ,const char *fmt, ...)
{
	// 产生格式化信息并输出到标准输出设备stdout(1)，这里是指屏幕上显示。参数'*fmt'指定输出将
	// 采用的格式，参见各种标准C 语言书籍。该子程序正好是vsprintf 如何使用的一个例子。该程
	// 序使用vsprintf()将格式化的字符串放入printbuf 缓冲区，然后用write()将缓冲区的内容输出
	// 到标准设备（1--stdout）。
	va_list args;
	int i;
	va_start(args, fmt);
	write(1,printbuf,i=vsprintf(printbuf, fmt, args));
	va_end(args);
	return i;
}
/*  读取系统中的信息放到psbuffer数组中,并再读取psbuffer中的数据写入到用户
	空间的buf中。读取位置从f_pos指向的位置开始，每次最多读count个字节，并根据
	实际读到的字节数调整f_pos的值，最后返回实际读到的字节数*/
int memread(int dev, char * buf, int count, off_t * f_pos)
{
    int i, j = 0;
    int free = 0;
    
    // 第一次从meminfobuffer数组的起始位置0开始
    if(!(*f_pos))
    {
        memset(membuffer, 0, SIZE);
        
        // 扫描内存页面映射数组mem_map[]，统计空闲页面数
        for(i = 0; i < PAGING_PAGES; i++)
        {
            if(! mem_map[i]) 
                free++;
        }
        
        // 生成物理内存信息，格式参考实验八
        j += sprintf(membuffer + j, "Page Count : %d\n", PAGING_PAGES);
        j += sprintf(membuffer + j, "Memory Count : %d * 4096 = %d Byte\n\n", PAGING_PAGES, PAGING_PAGES * 4096);
        j += sprintf(membuffer + j, "Free Page Count : %d\n", free);
        j += sprintf(membuffer + j, "Used Page Count : %d\n", PAGING_PAGES - free);
    }
    
    // 将数组membuffer中的信息读取到用户空间的buf位置处
    for(i = 0; i < count; i++, (*f_pos)++)
    {
        if(membuffer[*f_pos] == '\0')
            break;
        // put_fs_byte函数是将一字节存放在fs段中指定内存地址处
        put_fs_byte(membuffer[*f_pos], &buf[i]);
    }
    
    return i;
}
