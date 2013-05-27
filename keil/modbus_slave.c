#include<reg51.h>
#include "modbus_slave.h"
#include"uart.h"

//-------------------------------------------
#ifdef RS485 
	sbit DIR485 = P1^5;//485ͨ�ſ���λ
#endif	
//-------------------------------------------
FLAG rec_one_frame_ok = 0;//����һ֡���ݳɹ���־
FLAG modbus_rec_data_valid = 0;//���յ���Ч����
//------------------------------------------- 
uchar HOST_ID = 1;//������ַΪ1
uchar state = STATE_REC_HEADER;
uchar modbus_rec_count = 0;
//uchar xdata RX_BUF[40];//֡�Ĵ���
//uint xdata modbus_reg[16];//modbus �Ĵ���
uchar RX_BUF[40];//֡�Ĵ���
uint modbus_reg[16];//modbus �Ĵ���
//------------------------------------------- 
void uart_intterrupt(void) interrupt 4
{
	static unsigned char rec_count = 0;
	static unsigned char rec_temp_count = 0;
	static 	unsigned char rec_temp_buf[2];
	unsigned char rec_buf;

    if(RI)//��ѯ�Ƿ�Ϊ������� S2RI = 1
    {
		RI = 0;
		rec_buf=SBUF;
		//��λ������ʱ���ֽ���ǰ
		switch(state)
		{
			case STATE_REC_HEADER://֡��ʼ��־
				if(rec_buf==':')
				{
					state = STATE_REC_DATA;
					rec_count = 0;
					rec_temp_count = 0;
				}
			break;
			
			case STATE_REC_DATA://��ʼ��������
				if(rec_buf==0x0d)//������յ��س�
				{
					state = STATE_REC_END;
				}
				else
				{
					rec_temp_buf[rec_temp_count++] = rec_buf;
				
					if(rec_temp_count==2)
					{
						rec_temp_count = 0;
						RX_BUF[rec_count++]=(ascii_to_hex(rec_temp_buf[0])<<4)+ascii_to_hex(rec_temp_buf[1]);
						
						//�ڴ��жϵ�ַ��Ч��
						if(rec_count==1)
						{
							if((RX_BUF[0]!=HOST_ID)&&(RX_BUF[0]!=Broadcast_Add))//�ж��Ƿ�Ϊ���� ���ǹ㲥��ַ
							{
								state = STATE_REC_HEADER;//״̬����λ
							}							
						}		
					}
					
				}
			break;

			case STATE_REC_END://���ս�����־λ �س�����
				if(rec_buf==0x0A)//���յ����з�
				{
					rec_one_frame_ok = 1;//���յ�һ��Ч֡
					modbus_rec_count = rec_count;
					state = STATE_REC_HEADER;//״̬����λ					
				}
				else
				{
					state = STATE_REC_HEADER;//״̬����λ					
				}
			break;	
			default:
				state  = STATE_REC_HEADER;
			break;
						
		}

	}
	else
	{
	}
	
}

void modbus_decode(void)
{
	unsigned char i;
	unsigned char modbus_lrc_check = 0;//��ʱ������
	unsigned char modbus_rec_function_code = 0;
	unsigned int  modbus_rec_rw_base = 0;//��д��ʼ��ַ
	//-----------------------------------------------
	if(rec_one_frame_ok)//��ʼ��������У��
	{
		modbus_lrc_check  = 0;	
		for(i=0;i<modbus_rec_count-1;i++)
		{														 
			modbus_lrc_check = modbus_lrc_check+RX_BUF[i];	
		}
		modbus_lrc_check = (~modbus_lrc_check)+1;	

			
		if(RX_BUF[modbus_rec_count-1]==modbus_lrc_check)
		{
			modbus_rec_data_valid = 1;
			#ifdef RS485
				DIR485 = 1;//תΪ����״̬
				delay_for_dir485(120);//תΪ����״̬�л�ʱ��
			#endif

		}  
		else
		{
			#ifdef RS485
				DIR485 = 0;//תΪ����״̬	
			#endif					
		}

		rec_one_frame_ok = 0;
	}
	//-----------------------------------------------
	
	if(modbus_rec_data_valid)//����У��ɹ� ��ʼ����
	{
		modbus_rec_function_code = RX_BUF[1];

		switch(modbus_rec_function_code)
		{
			case Read_Holding_Registers://0x03

				modbus_rec_rw_base = (RX_BUF[2]<<8)+RX_BUF[3];

 				feedback_read_reg(((RX_BUF[4]<<8)+RX_BUF[5]),modbus_rec_rw_base);

			break;

			case Write_Single_Register://0x06

				modbus_rec_rw_base = (RX_BUF[2]<<8)+RX_BUF[3];//��ȡ��Ҫд��ļĴ�����ַ

				if((modbus_rec_rw_base>=0)&&(modbus_rec_rw_base<=15))//��ֹ����Խ����ɵ�Ƭ����λ
				{
					modbus_reg[modbus_rec_rw_base] = ((RX_BUF[4]<<8)+RX_BUF[5]);//������д��

					feedback_write_reg();
				}
				
			break;

			default:
			break;
		}
		modbus_rec_data_valid = 0;

		#ifdef RS485
			DIR485 = 0;//תΪ����״̬
			delay_for_dir485(50);//תΪ����״̬�л�ʱ��	
		#endif		


	}
	//-----------------------------------------------

}
void feedback_write_reg(void)//����λ������д�Ĵ�������
{				
	unsigned char i;

	uart_send_char(':');

	for(i=0;i<modbus_rec_count;i++)
	{
		uart_send_char(hex_to_ascii(RX_BUF[i],1));
		uart_send_char(hex_to_ascii(RX_BUF[i],0));

	}
	uart_send_char(0x0d);
	uart_send_char(0x0a);
}
void feedback_read_reg(unsigned int reg_num,unsigned int reg_base)//��ȡ�����ݵ����� ��ַƫ��
{
	unsigned char send_lrc_check = 0;
	unsigned char i = 0;
	uart_send_char(':');

	if(RX_BUF[0]==HOST_ID)//����ID
 	{
		uart_send_char(hex_to_ascii(HOST_ID,1));
		uart_send_char(hex_to_ascii(HOST_ID,0));		
	}
	else
	{
		uart_send_char(hex_to_ascii(Broadcast_Add,1));
		uart_send_char(hex_to_ascii(Broadcast_Add,0));			
	}

	//������
	uart_send_char(hex_to_ascii(Read_Holding_Registers,1));
	uart_send_char(hex_to_ascii(Read_Holding_Registers,0));

	//������������--�ֽ���
	uart_send_char(hex_to_ascii((2*reg_num),1)); //2���ֽ�
	uart_send_char(hex_to_ascii((2*reg_num),0));


	for(i=0;i<reg_num;i++)
	   {
			uart_send_char(hex_to_ascii((modbus_reg[reg_base+i]>>8),1));
			uart_send_char(hex_to_ascii((modbus_reg[reg_base+i]>>8),0));
			uart_send_char(hex_to_ascii(modbus_reg[reg_base+i],1));
			uart_send_char(hex_to_ascii(modbus_reg[reg_base+i],0));
			send_lrc_check = send_lrc_check+(modbus_reg[reg_base+i]>>8)+modbus_reg[reg_base+i];   	
	   }

	if(RX_BUF[0]==HOST_ID)//����ID
 	{
		send_lrc_check =send_lrc_check+HOST_ID+Read_Holding_Registers+2*reg_num;		
	}
	else
	{
		send_lrc_check =send_lrc_check+Broadcast_Add+Read_Holding_Registers+2*reg_num;			
	}		
	send_lrc_check =  ~(send_lrc_check)+1;

	uart_send_char(hex_to_ascii(send_lrc_check,1));
	uart_send_char(hex_to_ascii(send_lrc_check,0));

	//�س� ����
	uart_send_char(0x0d);
	uart_send_char(0x0a);
}
unsigned char hex_to_ascii(unsigned char hex_data,unsigned char hign_low)
{
	unsigned char temp_data;
	if(hign_low==0)//ת����λΪascii
	{
		temp_data = hex_data&0x0f;
		if(temp_data>=0&&temp_data<=9)
		{
			temp_data = temp_data+0x30;	

		}
		else
		{
			temp_data = temp_data + 0x37;
		}
	}
	else
	{
		 temp_data = hex_data>>4;
		if(temp_data>=0&&temp_data<=9)
		{
			temp_data = temp_data+0x30;	
		}
		else
		{
			temp_data = temp_data + 0x37;
		}
	}
	return temp_data;	
}
unsigned char ascii_to_hex(unsigned char ascii_data)//'f' -->0x0f '8'-->0x08
{
	unsigned char hex_data = 0;
	if((ascii_data>='0')&&(ascii_data<='9'))
	{
		hex_data = ascii_data - 0x30;
	}
	else
	{
		hex_data = ascii_data - 0x37;
	}
	
	return hex_data;
}
#ifdef RS485
	void delay_for_dir485(unsigned int delay_time)
	{
		while(--delay_time);
	}
#endif