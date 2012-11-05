终端(arm平台，新终端arm-linux-gcc version 4.3.5)上运行
	用于检查com口(/dev/ttyS*设备文件).

对于壁挂和机架，ttyS*设备文件定义有所区别。
机架 458-1～458-8 对应 ttyS0～ttyS7			共8路
壁挂 458-1～486-6 对应 ttyS2～ttyS7 对应 com3～com8	共6路

硬件连接:对于485口,一般使用菊花链型连接,或者两两相连也可(A连A，B连B).

软件操作:关闭使用串口通讯的程序,例如主程序.否则可能影响检测结果.

运行效果大致如下(检测com3及之后的6个com口):

`
# ./chkcom 3 6
Tx\Rx  	com3    com4    com5    com6    com7    com8    
com3    -       O       O       X       X       X
com4    O       -       O       X       X       X
com5    O       O       -       X       X       X
com6    X       X       X       -       X       X
com7    X       X       X       X       -       O
com8    X       X       X       X       O       -
`

或者,直接检测 com3 和 com5

`
# ./chkcom -d 3  5
directlink
Tx\Rx   com3
com5    O
`

`
# ./chkcom 1 4
Tx\Rx   com1    com2    com3    com4    
com1    -       O*      X       X       
com2    O       -       X       X       
com3    X       ?[56]** -       O       
com4    X       X       O       - 
`

O 表示连通 
X 表示不连通
- 表示自身

*  表示从com1发送数据，被com2所接收。
**  ?[56]表示com2端口收到了其他程序发送的一个字节0x56。
通常是因为主程序没有关闭，造成对检测程序的干扰。此时应该结束主程序（例如hl3104）
