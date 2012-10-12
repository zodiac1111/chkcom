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

#include     <stdio.h>		/*标准输入输出定义 */
#include     <stdlib.h>		/*标准函数库定义 */
#include     <unistd.h>		/*Unix 标准函数定义 */
#include     <sys/types.h>
#include     <sys/stat.h>
#include     <fcntl.h>		/*文件控制定义 */
#include     <termios.h>	/*PPSIX 终端控制定义 */
#include <errno.h>		/*错误号定义 */
#include <string.h>
#include "uart.h"
#include "test485.h"
#define TRAN_LEN 2 //发送长度
#define TEST485INF "[test485]"
int main(int argc,char ** argv)
{
	if(argc < 3){
		printusage();
		return 0;
	}
	if(argc == 3 ){
		int offset=atoi(argv[1])-1; //COM1开始 地址则ttyS0 开始
		int comnum=atoi(argv[2]);
		chkcom(offset,comnum);
		return 0;
	}
	if(argc == 4 && strcmp(argv[1],"-d")==0){
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
	int i;int j;
	int len;
	int recilen;
	char strdev[255];//device string
	unsigned char buf_send[TRAN_LEN];
	unsigned char buf_reci[TRAN_LEN];
	class Cuart com[comnum];//
	//打开
	for(i=0;i<comnum;i++){
		sprintf(strdev,"/dev/ttyS%d",i+offset);
		com[i].open_uart(strdev,O_RDWR);
	}
	//打印表头
	printf("\t");
	for(i=0;i<comnum;i++){
		printf("com%d\t",i+offset+1);
	}
	printf("\n");
	//数据
	for(i=0;i<comnum;i++){
		printf("com%d\t",i+offset+1);//序号
		memset(buf_send,i,TRAN_LEN);//发送信息(本端口号)
		len=com[i].uart_write(buf_send,TRAN_LEN);
		//
		for(j=0;j<comnum;j++){ //一行内
			//接收
			memset(buf_reci,0xFF,TRAN_LEN);//清空接收区,0有可能与COM1(ttyS0)混淆
			if(j==i){
				printf("-\t");fflush(stdout);
				continue; //跳过该端口本身
			}

			while(1) {
				recilen=com[j].uart_read(buf_reci,TRAN_LEN);
				//				for(k=0;k<recilen;k++){
				//					//printf(" 0x%02X",buf_reci[i]);
				//				}
				if(recilen<=0) break;
			}
			//得出结论, 通/不通
			if(buf_reci[0]==buf_send[0]){
				printf("O");
			}else{
				printf("X");
			}
			printf("\t");
			fflush(stdout);
		}
		printf("\n");
		//i++;
	}
	//关闭
	for(i=0;i<comnum;i++){
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
	class Cuart coma;class Cuart comb;
	//int offset=2;//端口起始偏移
	//简单的测试 端口3 和 8
	sprintf(strdev,"/dev/ttyS%d",a);
	coma.open_uart(strdev, O_RDWR);
	sprintf(strdev,"/dev/ttyS%d",b);
	comb.open_uart(strdev, O_RDWR);
	//默认设置
	//发送本端口
	memset(buf_send,a,TRAN_LEN);
	len=coma.uart_write(buf_send,TRAN_LEN);
	if(len != TRAN_LEN){
		printf("len write=%d \n",len);
	}
	printf("\tcom%d\n",a+1);
	//	printf("buf_send:");
	//	for(i=0;i<len;i++){
	//		printf(" 0x%02X",buf_send[i]);
	//	}
	//	printf("\n");

	//接收
	memset(buf_reci,0x00,TRAN_LEN);//清
	//printf("buf_reci:");
	printf("com%d\t",b+1);fflush(stdout);
	while(1) {
		recilen=comb.uart_read(buf_reci,TRAN_LEN);
		for(i=0;i<recilen;i++){
			//printf(" 0x%02X",buf_reci[i]);
		}
		if(recilen<=0) break;
	}
	//printf("\n");
	if(buf_reci[0]==buf_send[0]){
		printf("O");
	}else{
		printf("X");
	}
	printf("\n");
	coma.close_uart();
	comb.close_uart();
}

void printusage(void)
{
	printf("Usage1: testcom <start number> <com number>\n"
	       "\tFor example: testcom 2 4\n"
	       "\t> then will check COM2 COM3 COM4 COM5 ,4 COM ports. \n"
	       "Usage2: testcom -d <comA> <comB>\n"
	       "\tFor example: testcom -d 3 4\n"
	       "\t> Direct link the COM3 and COM4,check them.\n\n"
	       "The meaning of symbols in the table:\n"
	       "\tO:OK\n"
	       "\tX:Not OK\n"
	       "\t-:Itself port\n");
}
