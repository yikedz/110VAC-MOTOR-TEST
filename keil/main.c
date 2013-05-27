/*
	单向异步电机pwm调速 
	mcu：stc89c52
	电脑控制
*/
#include<reg51.h>
#include "modbus_slave.h"
#include"uart.h"
#include "soft_timer.h"

#define ON 0
#define OFF 1


sbit FZ = P2^7;
sbit ZZ = P2^4;	

static unsigned char started_flg = 0;

static unsigned char step = 1;					 
							
unsigned char time_getter[4];//电机运行时间数组 0--正转时间 1 停止时间1  2反转时间 3停止时间2

void init_timer0(void);

void run_motor(void);

void main(void)
{
	FZ = OFF;
 	ZZ = OFF;


	//电机运行时间初始值
	time_getter[0] = 45;//正转

	time_getter[1] = 1; //停止时间1

	time_getter[2] = 4; //反转

	time_getter[3] = 65;//停止时间2

	init_uart();

	init_timer0();


	 modbus_reg[0] = 45;
	 modbus_reg[1] = 1;
	 modbus_reg[2]  = 4;
	 modbus_reg[3] = 65;

	 modbus_reg[4]  = 1;//启动

	 modbus_reg[5]  = 1;//动作状态步

	 modbus_reg[6] = 1;//电机运行标志

	 modbus_reg[7] = 0;//循环次数


	EA = 1;
	
	while(1)
	{
		if( modbus_reg[4]==1)
		{
			run_motor();
			modbus_reg[5] = step;	
		}
		else
		{
			//停止的时候可以设置时间
			time_getter[0] = modbus_reg[0];
			time_getter[1] = modbus_reg[1];
			time_getter[2] = modbus_reg[2];
			time_getter[3] = modbus_reg[3];
			modbus_reg[5] = 0;//动作状态步
			FZ  = 1;
			ZZ = 1;
			started_flg = 0;
			step = 1;
			//modbus_reg[7] = 0;
		}

		modbus_decode(); 
	}
}	
void init_timer0(void)
{
	TMOD |=0x01 ;//定时器0工作在方式一
	TH0 = 0xdc;//11.0592
	TL0 = 0x00;

	ET0 = 1;  //使能定时器0中断
	TR0 =1;  //启动定时器0

}

void timer0_isr(void) interrupt 1
{

	TH0 = 0xdc;//11.0592
	TL0 = 0x00;

	update_all_timers();

}

void run_motor(void)
{


	switch(step)
	{
		case 1:
		modbus_reg[6] = 1;	
		
		if(started_flg==0)
		{
			start_soft_timer(SOFT_TIMER0,time_getter[0]*100);
			ZZ = 0;//电机正转
			started_flg = 1;			 
		}
		else
		{
			if(check_soft_timer(SOFT_TIMER0))
			{
				step = 2;
				started_flg = 0;		
			}	
		}
		break;
		case 2:
		modbus_reg[6] = 0;	
		if(started_flg==0)
		{
			start_soft_timer(SOFT_TIMER0,time_getter[1]*100);
			ZZ = 1;//电机停止时间1
			started_flg = 1;	
		}
		else
	
		{
			if(check_soft_timer(SOFT_TIMER0))
			{
				step = 3;
				started_flg = 0;		
			}	
		}
		break;
		case 3:
		modbus_reg[6] = 1;	
		if(started_flg==0)
		{
			start_soft_timer(SOFT_TIMER0,time_getter[2]*100);
			FZ = 0;//电机反转
			started_flg = 1;	
		}
		else
		{
			if(check_soft_timer(SOFT_TIMER0))
			{
				step = 4;
				started_flg = 0;		
			}	
		}
		break;
		case 4:
		modbus_reg[6] = 0;	
		if(started_flg==0)
		{
			start_soft_timer(SOFT_TIMER0,time_getter[3]*100);
			FZ = 1;//电机停止时间2
			started_flg = 1;	
		}
		else
		{
			if(check_soft_timer(SOFT_TIMER0))
			{
				step = 1;
				started_flg = 0;	
				modbus_reg[7]++;	
			}	
		}
		break;
		default:
			step = 1;
			started_flg = 0;
		break;	
	}
}





