#ifndef UART_H
#define UART_H
#include     <stdio.h>		/*��׼����������� */
#include     <stdlib.h>		/*��׼�����ⶨ�� */
#include     <unistd.h>		/*Unix ��׼�������� */
#include     <sys/types.h>
#include     <sys/stat.h>
#include     <fcntl.h>		/*�ļ����ƶ��� */
#include     <termios.h>	/*PPSIX �ն˿��ƶ��� */
#include     <errno.h>		/*����Ŷ��� */
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
