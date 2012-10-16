#include <stdio.h>              // printf
#include <fcntl.h>              // open
#include <string.h>             // bzero
#include <stdlib.h>             // exit
#include <sys/times.h>          // times
#include <sys/stat.h>           // lstat
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
//*********************************

#include <stdio.h>		/*��׼����������� */
#include <stdlib.h>		/*��׼�����ⶨ�� */
#include <unistd.h>		/*Unix ��׼�������� */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>		/*�ļ����ƶ��� */
#include <termios.h>	/*PPSIX �ն˿��ƶ��� */
#include <errno.h>		/*����Ŷ��� */
#include <string.h>
#include "uart.h"
#include "test485.h"
#define TRAN_LEN 2 //���ͳ���
#define TEST485INF "[test485]"
int main(int argc,char ** argv)
{
	if(argc < 3) {
		printusage();
		return 0;
	}
	if(argc == 3 ) {
		int offset=atoi(argv[1])-1; //COM1��ʼ ��ַ��ttyS0 ��ʼ
		int comnum=atoi(argv[2]);
		chkcom(offset,comnum);
		return 0;
	}
	if(argc == 4 && strcmp(argv[1],"-d")==0) {
		//printf("directlink\n");
		directlink(atoi(argv[2])-1,atoi(argv[3])-1);//com1 = ttyS0
		return 0;
	}
	//anything else, print usage
	printusage();
	return 0;
}
/*
int offset=0;//�˿���ʼƫ��
int comnum=0;//���ڸ���
*/
void chkcom(int offset,int comnum)
{
	int i;
	int j;
	int len;
	int recilen;
	char strdev[255];//device string
	unsigned char buf_send[TRAN_LEN];
	unsigned char buf_reci[TRAN_LEN];
	class Cuart com[comnum];//
	//��
	for(i=0; i<comnum; i++) {
		sprintf(strdev,"/dev/ttyS%d",i+offset);
		if(com[i].open_uart(strdev,O_RDWR)<0) {
			printf("open com%d err\n",i+offset+1);
			exit(1);
		}
	}
	//��ӡ��ͷ
	printf("\t");
	for(i=0; i<comnum; i++) {
		printf("com%d\t",i+offset+1);
	}
	printf("\n");
	//��������
	for(i=0; i<comnum; i++) {//��ѭ��
		printf("com%d\t",i+offset+1);//���
		memset(buf_send,i+offset+1,TRAN_LEN);//������Ϣ(���˿ں�)
		len=com[i].uart_write(buf_send,TRAN_LEN);
		for(j=0; j<comnum; j++) { //��ѭ��
			memset(buf_reci,0xFF,TRAN_LEN);//��ս�����,0�п�����COM1(ttyS0)����
			if(j==i) {
				printf("-\t");
				fflush(stdout);
				continue; //�����ö˿ڱ���
			}
			while(1) {
				recilen=com[j].uart_read(buf_reci,TRAN_LEN);
				if(recilen<=0) { break; }
			}
			//�ó�����, ͨ/��ͨ
			if(buf_reci[0]==buf_send[0]) {
				printf("O");
				fflush(stdout);
			} else {
				printf("X");
				fflush(stdout);
				if(buf_reci[0]!=0xFF) {
					printf("\nreci:[%02X],maybe other program "
					       "is using serial port now.\n",buf_reci[0]);
					goto stop;
				}
			}
			printf("\t");
			fflush(stdout);
		}//end one line for
		printf("\n");
	}//end for
stop:
	//�ر�
	for(i=0; i<comnum; i++) {
		com[i].close_uart();
	}
	return ;
}

//��������ֱ�������ļ��
void directlink(int a,int b)
{
	int i;
	int len;
	int recilen;
	char strdev[255];//dev string
	unsigned char buf_send[TRAN_LEN];
	unsigned char buf_reci[TRAN_LEN];
	class Cuart coma;
	class Cuart comb;
	//int offset=2;//�˿���ʼƫ��
	//�򵥵Ĳ��� �˿�3 �� 8
	sprintf(strdev,"/dev/ttyS%d",a);
	if(coma.open_uart(strdev, O_RDWR)<=0) {
		printf("open com%d err\n",a+1);
		exit(1);
	}
	sprintf(strdev,"/dev/ttyS%d",b);
	if(comb.open_uart(strdev, O_RDWR)<=0) {
		printf("open com%d err\n",b+1);
		exit(1);
	}
	//Ĭ������
	//���ͱ��˿�
	memset(buf_send,a,TRAN_LEN);
	len=coma.uart_write(buf_send,TRAN_LEN);
	if(len != TRAN_LEN) {
		printf("len write=%d \n",len);
	}
	printf("\tcom%d\n",a+1);
	//����
	memset(buf_reci,0xFF,TRAN_LEN);//��ս�����,0�п�����COM1(ttyS0)����
	//printf("buf_reci:");
	printf("com%d\t",b+1);
	fflush(stdout);
	while(1) {
		recilen=comb.uart_read(buf_reci,TRAN_LEN);
		for(i=0; i<recilen; i++) {
			//printf(" 0x%02X",buf_reci[i]);
		}
		if(recilen<=0) { break; }
	}
	if(buf_reci[0]==buf_send[0]) {
		printf("O");
		fflush(stdout);
	} else if(buf_reci[0]==0xFF) {
		printf("X");
		fflush(stdout);
	} else {
		printf("\nreci:[%02X],maybe other program"
		       "is using serial port now.\n",buf_reci[0]);
		goto stop;
	}
stop:
	printf("\n");
	coma.close_uart();
	comb.close_uart();
}

void printusage(void)
{
	printf("Usage: chkcom <Start number> <Number of ports>\n"
	       "   or: chkcom -d <comA> <comB>\n"
	       "Check the Serial Ports(comX).\n"
	       "\n"
	       "For example:\n"
	       "\tchkcom 3 4\n"
	       "\t> Check COM3 COM4 COM5 COM6,altogether 4 COM ports. \n"
	       "\tchkcom -d 3 4\n"
	       "\t> Direct link the COM3 and COM4,check them.\n"
	       "\n"
	       "The meaning of symbols in the table:\n"
	       "\tO: linked\n"
	       "\tX: unlinked.\n"
	       "\t-: Itself port\n");
}
