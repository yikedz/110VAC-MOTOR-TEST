#ifndef _SOFT_TIMER__
#define _SOFT_TIMER__

	#define SOFT_TIMER_NUM 1
	#define SOFT_TIMER0 0
	#define SOFT_TIMER1 1
	#define SOFT_TIMER2 2


typedef struct SOFT_TIMER
	{
		unsigned char en_run;//定时器运行时能
		unsigned int value_current;//当前值
		unsigned int value_setting;//设定值
		unsigned char timeout_flg;//定时器运行时间到
	}SOFT_TIMER,*P_SOFT_TIMER;

extern SOFT_TIMER soft_timer[];

void start_soft_timer(unsigned char,unsigned int time_setting);
void update_all_timers(void);//更新处理所有软件定时器  在硬件定时器中被调用
unsigned char check_soft_timer(unsigned char soft_timer_id);
void clear_soft_timer(unsigned char soft_timer_id);
#endif
