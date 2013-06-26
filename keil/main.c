/*
	mcu：stc89c52
*/
#include<reg51.h>

#define ON 0
#define OFF 1
sbit key = P1^5;
sbit sensor1 = P1^6;
sbit sensor2 = P1^7;

sbit FZ = P2^7;
sbit ZZ = P2^4;	
			 

unsigned char key_pressed = 0;//保存按键按下
unsigned char sensor1_flg = 0;//保存sensor1信号状态 
unsigned char sensor2_flg = 0;//保存sensor1信号状态
unsigned char  flg_10ms = 0;

void init_timer0(void);
void get_io_status(void);//获取io状态
void system_handle(void);

void main(void)
{
	FZ = OFF;
 	ZZ = OFF;
	init_timer0();
	EA = 1;
	while(1)
	{
		if(flg_10ms)
		{
			flg_10ms = 0;
			get_io_status();
		}
		
		system_handle();
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

    flg_10ms = 1;
	//update_all_timers();

}
void get_io_status(void)//获取io状态
{
static unsigned char key_state = 1;
static unsigned char key_pressed_count = 0;
static unsigned char key_release_count = 0;

static unsigned char sensor1_state = 1;
static unsigned char sensor1_pressed_count = 0;
static unsigned char sensor1_release_count = 0;

static unsigned char sensor2_state = 1;
static unsigned char sensor2_pressed_count = 0;
static unsigned char sensor2_release_count = 0;
 
//key button =================================
	switch(key_state)
	{
		case 1://检查低电平
			key = 1;
			if(key==0)//按键按下
			{
				key_pressed_count++;
				if(key_pressed_count==10)
				{
					key_pressed = 1;
					key_pressed_count = 0;
				}	
			}
			else
			{
				key_state = 2;
				key_pressed_count = 0;
				key_release_count = 0;
	
			}	
		break;
		case 2:
			key = 1;
			if(key)//按键按下
			{
				key_release_count++;
				if(key_release_count==10)
				{
					key_pressed = 0;
					key_release_count = 0;
				}	
			}
			else
			{
				key_state = 1;
				key_pressed_count = 0;
				key_release_count = 0;
	
			}
		break;
		default:
			key_state = 1;
		break;	
	}
//key button =================================

//sensor1=====================================
	switch(sensor1_state)
	{
		case 1:
			sensor1 =1;
			if(sensor1==0)//低电平信号有效
			{
				sensor1_pressed_count++;
				if(sensor1_pressed_count==5)
				{
					sensor1_flg = 1;
					sensor1_pressed_count = 0;	
				}
			}
			else
			{
				sensor1_state = 2;
				sensor1_pressed_count = 0;	
				sensor1_release_count = 0;	
			}
		break;
		case 2:
			sensor1 =1;
			if(sensor1)
			{
				sensor1_release_count++;
				if(sensor1_release_count==5)
				{
					sensor1_flg = 0;
					sensor1_release_count = 0;	
				}
			}
			else
			{
				sensor1_state = 1;
				sensor1_pressed_count = 0;	
				sensor1_release_count = 0;				
			}
		break;
		default:
			sensor1_state = 1;
		break;
	}
//sensor1=====================================

//sensor2=====================================
	switch(sensor2_state)
	{
		case 1:
			sensor2 =1;
			if(sensor2==0)//低电平信号有效
			{
				sensor2_pressed_count++;
				if(sensor2_pressed_count==5)
				{
					sensor2_flg = 1;
					sensor2_pressed_count = 0;	
				}
			}
			else
			{
				sensor2_state = 2;
				sensor2_pressed_count = 0;	
				sensor2_release_count = 0;	
			}
		break;
		case 2:
			sensor2 =1;
			if(sensor2)
			{
				sensor2_release_count++;
				if(sensor2_release_count==5)
				{
					sensor2_flg = 0;
					sensor2_release_count = 0;	
				}
			}
			else
			{
				sensor2_state = 1;
				sensor2_pressed_count = 0;	
				sensor2_release_count = 0;				
			}
		break;
		default:
			sensor2_state = 1;
		break;
	}
//sensor2=====================================	
}

void system_handle(void)
{
	static unsigned char key_system_state = 1;
	static unsigned char pos = 1;
	static unsigned char  en_motor_run = 0;
	switch(key_system_state)
	{
		case 1://等待按键按下
			if(key_pressed)//按键按下了
			{
				key_system_state = 2;
			}		
		break;
		case 2:
			if(key_pressed==0)//等待按键松开
			{
				en_motor_run = 1;
				key_system_state = 1;
			}
		break;
		default:
			key_system_state = 1;	
		break;
		
	}
	if(en_motor_run)//允许电机转动
	{
		switch(pos)
		{
			case 1://此时默认电机停止在传感器sensor1位置处 正转到sensor2处
				ZZ = ON;//电机开始正转
				if(sensor1_flg)//正转到sensor2处
				{
					ZZ = OFF;//正转停止
					en_motor_run = 0;
					pos = 2;
				}
			break;
			case 2://此时默认电机停止在传感器sensor2位置处 反转到sensor1处
				FZ = ON;//电机开始反转
				if(sensor2_flg)//反转到sensor1处
				{
					FZ = OFF;//反转停止
					en_motor_run = 0;
					pos = 1;
				}			
			break;
			default:
				pos = 1;			
			break;
		}		
	}

}






