#ifndef _SOFT_TIMER__
#define _SOFT_TIMER__

	#define SOFT_TIMER_NUM 1
	#define SOFT_TIMER0 0
	#define SOFT_TIMER1 1
	#define SOFT_TIMER2 2


typedef struct SOFT_TIMER
	{
		unsigned char en_run;//��ʱ������ʱ��
		unsigned int value_current;//��ǰֵ
		unsigned int value_setting;//�趨ֵ
		unsigned char timeout_flg;//��ʱ������ʱ�䵽
	}SOFT_TIMER,*P_SOFT_TIMER;

extern SOFT_TIMER soft_timer[];

void start_soft_timer(unsigned char,unsigned int time_setting);
void update_all_timers(void);//���´������������ʱ��  ��Ӳ����ʱ���б�����
unsigned char check_soft_timer(unsigned char soft_timer_id);
void clear_soft_timer(unsigned char soft_timer_id);
#endif
