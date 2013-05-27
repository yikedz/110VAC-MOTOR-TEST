#include<reg51.h>
#include"uart.h"
//----------------------------
void init_uart(void)
{
	//*
	//该部分适用于普通51 如AT89S51系列单片机
	//波特率为9600@11.0592M
	TMOD|=0x20;  //TMOD=0010 0000B，定时器T1工作于方式2 
	SCON|=0x50;  //SCON=0101 0000B，串口工作方式1
	TH1=0xfd;    //根据规定给定时器T1赋初值
	TL1=0xfd;    //根据规定给定时器T1赋初值
	TR1=1;      //启动定时器T1
	ES = 1;		//允许串口中断 
	//EA = 1; 	
	//*/

  //------------------------------------

   /*
   										  
   //该部分适用于12C5A60S2双串口单片机 
   //使用串口2 TXD RXD从P1口映射到P4.2 P4.3端口上  
   //独立比特率发生器 波特率为9600@11.0592M

	AUXR1 = AUXR1|0X10;//通讯口由P1切换到P4口

	S2CON    =   0x50;   //0101,0000 8位可变波特率，无奇偶校验位,允许接收
 
	BRT	=	0xfd;

	AUXR=0x10; // T0x12,T1x12,UART_M0x6,BRTR,S2SMOD,BRTx12,EXTRAM,S1BRS	

	IE2	= 0x01;	//允许串口2中断,ES2=1

  
  	PCON &= 0x7f;		//波特率不倍速
	SCON = 0x50;		//8位数据,可变波特率
	BRT = 0xDC;		//设定独立波特率发生器重装值
	AUXR |= 0x04;		//独立波特率发生器时钟为Fosc,即1T
	AUXR |= 0x01;		//串口1选择独立波特率发生器为波特率发生器
	AUXR |= 0x10;		//启动独立波特率发生器
	ES = 1;		//允许串口中断  */
  //------------------------------------
}

void uart_send_char(unsigned char i)
{
	///*
	//该部分适用于普通51 如AT89S51系列单片机

	SBUF = i;
	while(!TI);
	TI = 0;
	//*/

   /*	
   									  
   //该部分适用于12C5A60S2双串口单片机 
   //使用串口2 TXD RXD引脚由init_uart()函数确定P1/P4 
    
	unsigned char temp = 0;

	IE2	=	0x00;	//关串口2中断,es2=0

    S2CON = S2CON&0xFD; //B'11111101,清零串口2发送完成中断请求标志
    S2BUF   =   i;
    do
	{
		temp = S2CON;
		temp = temp & 0x02;
	}while(temp==0);

	
    S2CON	=	S2CON & 0xFD; //B'11111101,清零串口2发送完成中断请求标志

	IE2	=0x01;	//允许串口2中断,ES2=1

	*/
	
}

