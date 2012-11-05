/* һ�δ� /dev/ttyS* �豸���ж�д������
  �����ж��豸�Ƿ�������
  ���ڻ��� 485-1 = ttyS0 ==> 485-8 = ttyS7	��8·485��
  �ڹ�	  485-1��6 = com3��8 = ttyS2��7		��6·485��
  �������޷��ж� ���ܻ��Ǳڹ�
  ʹ��ǰӦ�ر�����ʹ�ô��ڵĽ��̡� fuser /dev/ttyS* ָ����Բ鿴ʹ�ô��ڵĽ���
*/
//*********************************
#include <ctype.h>
#include <errno.h>		/*����Ŷ��� */
#include <fcntl.h>              // open
#include <iostream>
#include <pthread.h>
#include <stdio.h>              // printf
#include <stdlib.h>		/*��׼�����ⶨ�� */
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>           // lstat
#include <sys/times.h>          // times
#include <sys/types.h>
#include <termios.h>	/*PPSIX �ն˿��ƶ��� */
#include <unistd.h>		/*Unix ��׼�������� */
#include "test485.h"
#include "uart.h"
#define TRAN_LEN 1 //���ͳ���
#define TEST485INF "[chkcom]"
//#define DEBUG
int main(int argc,char ** argv)
{

	if(argc < 3) {
		printusage();
		return 0;
	}
	if(argc == 3 ) {
		int start=atoi(argv[1])-1; //COM1��ʼ ��ַ��ttyS0 ��ʼ
		int end=atoi(argv[2])-1;
		if(end<=start){
			printf(TEST485INF"Start port must less than end port.\n");
			printusage();
			return 0;
		}
		//while(1)
		chkcom(start,(end-start)+1);

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
	if(comnum<=1){
		return;
	}
	class Cuart com[comnum];//
	//��
	for(i=0; i<comnum; i++) {
		sprintf(strdev,"/dev/ttyS%d",i+offset);
		if(com[i].open_uart(strdev,O_RDWR)<0) {
			printf(TEST485INF"open com%d err\n",i+offset+1);
			exit(-2);
		}
	}
	//��ӡ��ͷ
	printf("Tx\\Rx\t");
	for(i=0; i<comnum; i++) {
		printf("com%d\t",i+offset+1);
	}
	printf("\n");
	//��������
	for(i=0; i<comnum; i++) {//��ѭ��
		printf("com%d\t",i+offset+1);//���
		memset(buf_send,i+offset+1,TRAN_LEN);//������Ϣ(���˿ں�)
#ifdef DEBUG
		printf("write begin \n");
#endif
		len=com[i].uart_write(buf_send,TRAN_LEN);
		if(len!=TRAN_LEN){
			printf("write error len=%d\n",len);
		}
#ifdef DEBUG
		printf("write ok \n");
#endif
		for(j=0; j<comnum; j++) { //��ѭ��
			memset(buf_reci,0xFF,TRAN_LEN);//��ս�����,0�п�����COM1(ttyS0)����
			if(j==i) {
				printf("-\t");
				fflush(stdout);
				continue; //�����ö˿ڱ���
			}
			//printf("out of while \n");
			while(1) {
#ifdef DEBUG
				printf("reading \n");
#endif
				recilen=com[j].uart_read(buf_reci,TRAN_LEN);
#ifdef DEBUG
				printf("read over recilen=%d buf_reci=%02x \n",recilen,buf_reci[0]);
#endif
				if(recilen !=TRAN_LEN || recilen<=0)
				{ break; }
			}
			//�ó�����, ͨ/��ͨ
			if(adjust(buf_send[0],buf_reci[0]) < 0) {
				goto stop;
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
		exit(-2);
	}
	sprintf(strdev,"/dev/ttyS%d",b);
	if(comb.open_uart(strdev, O_RDWR)<=0) {
		printf("open com%d err\n",b+1);
		exit(-3);
	}
	//Ĭ������
	//���ͱ��˿�
	memset(buf_send,a,TRAN_LEN);
	len=coma.uart_write(buf_send,TRAN_LEN);
	if(len != TRAN_LEN) {
		printf("len write=%d \n",len);
	}
	printf("Tx\\Rx\t");
	printf("com%d\n",b+1);
	//����
	memset(buf_reci,0xFF,TRAN_LEN);//��ս�����,0�п�����COM1(ttyS0)����
	//printf("buf_reci:");

	printf("com%d\t",a+1);
	fflush(stdout);
	while(1) {
		recilen=comb.uart_read(buf_reci,TRAN_LEN);
		for(i=0; i<recilen; i++) {
			//printf(" 0x%02X",buf_reci[i]);
		}
		if(recilen<=0) { break; }
	}
	adjust(buf_send[0],buf_reci[0]);
	printf("\n");
	coma.close_uart();
	comb.close_uart();
}

void printusage(void)
{
	printf("Usage: chkcom <Start port> <End port>\n"
	       "   or: chkcom -d <comA> <comB>\n"
	       "Check the Serial Ports(comX).\n"
	       "\n"
	       "For example:\n"
	       "\tchkcom 3 6\n"
	       "\t> Check COM3 COM4 COM5 COM6 ports. \n"
	       "\tchkcom -d 3 4\n"
	       "\t> Direct link the COM3 and COM4,check them.\n"
	       "\n"
	       "The meaning of symbols in the table:\n"
	       "\tO: linked\n"
	       "\tX: unlinked.\n"
	       "\t-: Itself port\n"
	       "\t?[num]: Maybe some other program is using serial port,"
		"<NUM> is received\n");
}
int adjust(unsigned char send,unsigned char reci)
{
	if(reci==send) {
		printf("O");
		fflush(stdout);
		return 0;
	} else if(reci==0xFF) {
		printf("X");
		fflush(stdout);
		return 0;
	} else {
		printf("?[%02X]",reci);
		fflush(stdout);
		return 0;
		//		printf("\n");
		//		printf(TEST485INF"Reci:[0x%02X],maybe other program"
		//		       " is using serial port now.\n"
		//		       "Please stop other programs.\n",reci);
		//		return -1;
	}
	return 0;
}
