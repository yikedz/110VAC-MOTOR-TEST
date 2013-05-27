#include "soft_timer.h"

SOFT_TIMER soft_timer[SOFT_TIMER_NUM];

//���������ʱ�� ������ʱ����Ϊ��ʼ״̬ soft_timer_idΪ�����ʱ����������
void start_soft_timer(unsigned char soft_timer_id,unsigned int time_setting)
{
	soft_timer[soft_timer_id].en_run = 1;
	soft_timer[soft_timer_id].value_current = 0;
	soft_timer[soft_timer_id].value_setting = time_setting;
	soft_timer[soft_timer_id].timeout_flg = 0;
}

//���ȸ������ж�ʱ�� --ֻ���ڶ�ʱ���ж���ʹ��
void update_all_timers(void)
{
	unsigned char i;

		for(i=0;i<SOFT_TIMER_NUM;i++)
		{
			if(soft_timer[i].en_run)//ʹ�ܸ������ʱ������
			{
				if((soft_timer[i].value_current)<(soft_timer[i].value_setting))//���ʱ���Ƿ�
				{
					soft_timer[i].value_current++;					
				}
				else//ʱ�䵽
				{
					soft_timer[i].timeout_flg = 1;//�����ʱ������־��λ
					soft_timer[i].en_run = 0;//ֹͣ�������ʱ��	
				}
			}
		}	
}
/*	
	���ָ���������ʱ����ʱʱ�䵽��û ���˷���1
*/
unsigned char check_soft_timer(unsigned char soft_timer_id)
{	
	return soft_timer[soft_timer_id].timeout_flg;
}

/*	
	��������ʱ��ʱ�䵽��־ ������һ�ν��ж�ʱ
*/
void clear_soft_timer(unsigned char soft_timer_id)
{	
	soft_timer[soft_timer_id].timeout_flg = 0;
}
