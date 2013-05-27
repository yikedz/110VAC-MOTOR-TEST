/*
	�����첽���pwm���� 
	mcu��stc89c52
	���Կ���
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
							
unsigned char time_getter[4];//�������ʱ������ 0--��תʱ�� 1 ֹͣʱ��1  2��תʱ�� 3ֹͣʱ��2

void init_timer0(void);

void run_motor(void);

void main(void)
{
	FZ = OFF;
 	ZZ = OFF;


	//�������ʱ���ʼֵ
	time_getter[0] = 45;//��ת

	time_getter[1] = 1; //ֹͣʱ��1

	time_getter[2] = 4; //��ת

	time_getter[3] = 65;//ֹͣʱ��2

	init_uart();

	init_timer0();


	 modbus_reg[0] = 45;
	 modbus_reg[1] = 1;
	 modbus_reg[2]  = 4;
	 modbus_reg[3] = 65;

	 modbus_reg[4]  = 1;//����

	 modbus_reg[5]  = 1;//����״̬��

	 modbus_reg[6] = 1;//������б�־

	 modbus_reg[7] = 0;//ѭ������


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
			//ֹͣ��ʱ���������ʱ��
			time_getter[0] = modbus_reg[0];
			time_getter[1] = modbus_reg[1];
			time_getter[2] = modbus_reg[2];
			time_getter[3] = modbus_reg[3];
			modbus_reg[5] = 0;//����״̬��
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
	TMOD |=0x01 ;//��ʱ��0�����ڷ�ʽһ
	TH0 = 0xdc;//11.0592
	TL0 = 0x00;

	ET0 = 1;  //ʹ�ܶ�ʱ��0�ж�
	TR0 =1;  //������ʱ��0

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
			ZZ = 0;//�����ת
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
			ZZ = 1;//���ֹͣʱ��1
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
			FZ = 0;//�����ת
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
			FZ = 1;//���ֹͣʱ��2
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





