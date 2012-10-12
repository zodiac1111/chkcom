#ifndef UART_H
#define UART_H
#include     <stdio.h>		/*标准输入输出定义 */
#include     <stdlib.h>		/*标准函数库定义 */
#include     <unistd.h>		/*Unix 标准函数定义 */
#include     <sys/types.h>
#include     <sys/stat.h>
#include     <fcntl.h>		/*文件控制定义 */
#include     <termios.h>	/*PPSIX 终端控制定义 */
#include     <errno.h>		/*错误号定义 */
#include<string.h>

class Cuart
{

public:
	Cuart();
	~Cuart();
	int open_uart(const char *file, int oflag);
	int close_uart(void);
	int set_speed(int speed);
	int set_Parity(int databits, int stopbits, int parity);
	int uart_write(const void *buf, size_t n);
	int uart_read( void *buf, size_t nbytes);
public:
	int fd;
	int speed_arr[7] ;
	int name_arr[7] ;
};


#endif // UART_H
