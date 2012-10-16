#include "uart.h"
Cuart::Cuart()
{
	speed_arr[0] = B38400;
	speed_arr[1] = B19200;
	speed_arr[2] = B9600;
	speed_arr[3] = B4800;
	speed_arr[4] = B2400;
	speed_arr[5] = B1200;
	speed_arr[6] = B300;
	name_arr[0] = 38400;
	name_arr[1] = 19200;
	name_arr[2] = 9600;
	name_arr[3] = 4800;
	name_arr[4] = 2400;
	name_arr[5] = 1200;
	name_arr[6] = 300;
}

Cuart::~Cuart()
{
}

int Cuart::open_uart(const char *file, int oflag)
{
	fd=open(file,oflag);
	if(fd>0) {
		if(set_speed(1200)<0) {
			return -1;
		}
		if(set_Parity(8, 1, 'N')<0) {
			return -1;
		}
	}
	return fd;
}

int Cuart::set_speed(int speed)
{
	int i;
	int status;
	struct termios Opt;
	tcgetattr(fd, &Opt);
	for (i = 0; i < int(sizeof(speed_arr) / sizeof(int)); i++) {
		if (speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0) {
				perror("set com speed:");
				return -1;
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
	return 0;
}
/**

  *@brief   ���ô�������λ��ֹͣλ��Ч��λ
  *@param  fd     ����  int  �򿪵Ĵ����ļ����
  *@param  databits ����  int ����λ   ȡֵ Ϊ 7 ����8
  *@param  stopbits ����  int ֹͣλ   ȡֵΪ 1 ����2
  *@param  parity  ����  int  Ч������ ȡֵΪN,E,O,,S
  */
int Cuart::set_Parity(int databits, int stopbits, int parity)
{
	struct termios options;
	if (tcgetattr(fd, &options) != 0) {
		perror("set_Parity1");
		return -1;
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) {	/*��������λ�� */
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
		return (-1);
	}
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;	/* Clear parity enable */
		options.c_iflag &= ~INPCK;	/* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);	/* ����Ϊ��Ч�� */
		options.c_iflag |= INPCK;	/* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;	/* Enable parity */
		options.c_cflag &= ~PARODD;	/* ת��ΪżЧ�� */
		options.c_iflag |= INPCK;	/* Disnable parity checking */
		break;
	case 'S':
	case 's':		/*as no parity */
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return (-1);
	}
	/* ����ֹͣλ */
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return (-1);
	}
	/* Set input parity option */
	if (parity != 'n')
	{ options.c_iflag |= INPCK; }
	tcflush(fd, TCIFLUSH);
	options.c_cc[VTIME] = 5;	/* ���ó�ʱ15 seconds */
	options.c_cc[VMIN] = 0;	/* Update the options and do it NOW */
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("tcsetattr\n");
		return (-1);
	}
	return (0);
}

int Cuart::close_uart(void)
{
	return close(this->fd);
}

int Cuart::uart_write(const void *buf, size_t n)
{
	return write(fd,buf,n);
}

int Cuart::uart_read( void *buf, size_t nbytes)
{
	return read(fd, buf,nbytes);
}
