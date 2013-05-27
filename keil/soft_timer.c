#include "soft_timer.h"

SOFT_TIMER soft_timer[SOFT_TIMER_NUM];

//启动软件定时器 并将定时器置为初始状态 soft_timer_id为软件定时器数组索引
void start_soft_timer(unsigned char soft_timer_id,unsigned int time_setting)
{
	soft_timer[soft_timer_id].en_run = 1;
	soft_timer[soft_timer_id].value_current = 0;
	soft_timer[soft_timer_id].value_setting = time_setting;
	soft_timer[soft_timer_id].timeout_flg = 0;
}

//调度更新所有定时器 --只能在定时器中断中使用
void update_all_timers(void)
{
	unsigned char i;

		for(i=0;i<SOFT_TIMER_NUM;i++)
		{
			if(soft_timer[i].en_run)//使能该软件定时器运行
			{
				if((soft_timer[i].value_current)<(soft_timer[i].value_setting))//检测时间是否到
				{
					soft_timer[i].value_current++;					
				}
				else//时间到
				{
					soft_timer[i].timeout_flg = 1;//软件定时器到标志置位
					soft_timer[i].en_run = 0;//停止该软件定时器	
				}
			}
		}	
}
/*	
	检查指定的软件定时器定时时间到了没 到了返回1
*/
unsigned char check_soft_timer(unsigned char soft_timer_id)
{	
	return soft_timer[soft_timer_id].timeout_flg;
}

/*	
	清除软件定时器时间到标志 方便下一次进行定时
*/
void clear_soft_timer(unsigned char soft_timer_id)
{	
	soft_timer[soft_timer_id].timeout_flg = 0;
}
