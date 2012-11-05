/* 一次打开 /dev/ttyS* 设备进行读写操作。
  用来判断设备是否正常。
  对于机架 485-1 = ttyS0 ==> 485-8 = ttyS7	共8路485口
  壁挂	  485-1～6 = com3～8 = ttyS2～7		共6路485口
  程序中无法判断 机架还是壁挂
  使用前应关闭其他使用串口的进程。 fuser /dev/ttyS* 指令可以查看使用串口的进程
*/
//*********************************
#include <ctype.h>
#include <errno.h>		/*错误号定义 */
#include <fcntl.h>              // open
#include <iostream>
#include <pthread.h>
#include <stdio.h>              // printf
#include <stdlib.h>		/*标准函数库定义 */
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>           // lstat
#include <sys/times.h>          // times
#include <sys/types.h>
#include <termios.h>	/*PPSIX 终端控制定义 */
#include <unistd.h>		/*Unix 标准函数定义 */
#include "test485.h"
#include "uart.h"
#define TRAN_LEN 1 //发送长度
#define TEST485INF "[chkcom]"
//#define DEBUG
int main(int argc,char ** argv)
{

	if(argc < 3) {
		printusage();
		return 0;
	}
	if(argc == 3 ) {
		int start=atoi(argv[1])-1; //COM1开始 地址则ttyS0 开始
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
int offset=0;//端口起始偏移
int comnum=0;//串口个数
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
	//打开
	for(i=0; i<comnum; i++) {
		sprintf(strdev,"/dev/ttyS%d",i+offset);
		if(com[i].open_uart(strdev,O_RDWR)<0) {
			printf(TEST485INF"open com%d err\n",i+offset+1);
			exit(-2);
		}
	}
	//打印表头
	printf("Tx\\Rx\t");
	for(i=0; i<comnum; i++) {
		printf("com%d\t",i+offset+1);
	}
	printf("\n");
	//表内数据
	for(i=0; i<comnum; i++) {//行循环
		printf("com%d\t",i+offset+1);//序号
		memset(buf_send,i+offset+1,TRAN_LEN);//发送信息(本端口号)
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
		for(j=0; j<comnum; j++) { //列循环
			memset(buf_reci,0xFF,TRAN_LEN);//清空接收区,0有可能与COM1(ttyS0)混淆
			if(j==i) {
				printf("-\t");
				fflush(stdout);
				continue; //跳过该端口本身
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
			//得出结论, 通/不通
			if(adjust(buf_send[0],buf_reci[0]) < 0) {
				goto stop;
			}
			printf("\t");
			fflush(stdout);
		}//end one line for
		printf("\n");
	}//end for
stop:
	//关闭
	for(i=0; i<comnum; i++) {
		com[i].close_uart();
	}
	return ;
}

//两个串口直接相连的检测
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
	//int offset=2;//端口起始偏移
	//简单的测试 端口3 和 8
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
	//默认设置
	//发送本端口
	memset(buf_send,a,TRAN_LEN);
	len=coma.uart_write(buf_send,TRAN_LEN);
	if(len != TRAN_LEN) {
		printf("len write=%d \n",len);
	}
	printf("Tx\\Rx\t");
	printf("com%d\n",b+1);
	//接收
	memset(buf_reci,0xFF,TRAN_LEN);//清空接收区,0有可能与COM1(ttyS0)混淆
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
