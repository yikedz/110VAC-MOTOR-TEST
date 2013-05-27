#include<reg51.h>
#include "modbus_slave.h"
#include"uart.h"

//-------------------------------------------
#ifdef RS485 
	sbit DIR485 = P1^5;//485通信控制位
#endif	
//-------------------------------------------
FLAG rec_one_frame_ok = 0;//接收一帧数据成功标志
FLAG modbus_rec_data_valid = 0;//接收到有效数据
//------------------------------------------- 
uchar HOST_ID = 1;//本机地址为1
uchar state = STATE_REC_HEADER;
uchar modbus_rec_count = 0;
//uchar xdata RX_BUF[40];//帧寄存器
//uint xdata modbus_reg[16];//modbus 寄存器
uchar RX_BUF[40];//帧寄存器
uint modbus_reg[16];//modbus 寄存器
//------------------------------------------- 
void uart_intterrupt(void) interrupt 4
{
	static unsigned char rec_count = 0;
	static unsigned char rec_temp_count = 0;
	static 	unsigned char rec_temp_buf[2];
	unsigned char rec_buf;

    if(RI)//查询是否为接受完成 S2RI = 1
    {
		RI = 0;
		rec_buf=SBUF;
		//上位机发送时高字节在前
		switch(state)
		{
			case STATE_REC_HEADER://帧开始标志
				if(rec_buf==':')
				{
					state = STATE_REC_DATA;
					rec_count = 0;
					rec_temp_count = 0;
				}
			break;
			
			case STATE_REC_DATA://开始接收数据
				if(rec_buf==0x0d)//如果接收到回车
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
						
						//在此判断地址有效性
						if(rec_count==1)
						{
							if((RX_BUF[0]!=HOST_ID)&&(RX_BUF[0]!=Broadcast_Add))//判断是否为本机 或是广播地址
							{
								state = STATE_REC_HEADER;//状态机复位
							}							
						}		
					}
					
				}
			break;

			case STATE_REC_END://接收结束标志位 回车换行
				if(rec_buf==0x0A)//接收到换行符
				{
					rec_one_frame_ok = 1;//接收到一有效帧
					modbus_rec_count = rec_count;
					state = STATE_REC_HEADER;//状态机复位					
				}
				else
				{
					state = STATE_REC_HEADER;//状态机复位					
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
	unsigned char modbus_lrc_check = 0;//临时检验用
	unsigned char modbus_rec_function_code = 0;
	unsigned int  modbus_rec_rw_base = 0;//读写开始地址
	//-----------------------------------------------
	if(rec_one_frame_ok)//开始进行数据校验
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
				DIR485 = 1;//转为发送状态
				delay_for_dir485(120);//转为发送状态切换时间
			#endif

		}  
		else
		{
			#ifdef RS485
				DIR485 = 0;//转为接收状态	
			#endif					
		}

		rec_one_frame_ok = 0;
	}
	//-----------------------------------------------
	
	if(modbus_rec_data_valid)//数据校验成功 开始解码
	{
		modbus_rec_function_code = RX_BUF[1];

		switch(modbus_rec_function_code)
		{
			case Read_Holding_Registers://0x03

				modbus_rec_rw_base = (RX_BUF[2]<<8)+RX_BUF[3];

 				feedback_read_reg(((RX_BUF[4]<<8)+RX_BUF[5]),modbus_rec_rw_base);

			break;

			case Write_Single_Register://0x06

				modbus_rec_rw_base = (RX_BUF[2]<<8)+RX_BUF[3];//获取将要写入的寄存器地址

				if((modbus_rec_rw_base>=0)&&(modbus_rec_rw_base<=15))//防止数组越界造成单片机复位
				{
					modbus_reg[modbus_rec_rw_base] = ((RX_BUF[4]<<8)+RX_BUF[5]);//将数据写入

					feedback_write_reg();
				}
				
			break;

			default:
			break;
		}
		modbus_rec_data_valid = 0;

		#ifdef RS485
			DIR485 = 0;//转为接收状态
			delay_for_dir485(50);//转为发送状态切换时间	
		#endif		


	}
	//-----------------------------------------------

}
void feedback_write_reg(void)//向上位机反馈写寄存器命令
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
void feedback_read_reg(unsigned int reg_num,unsigned int reg_base)//读取的数据的数量 基址偏移
{
	unsigned char send_lrc_check = 0;
	unsigned char i = 0;
	uart_send_char(':');

	if(RX_BUF[0]==HOST_ID)//主机ID
 	{
		uart_send_char(hex_to_ascii(HOST_ID,1));
		uart_send_char(hex_to_ascii(HOST_ID,0));		
	}
	else
	{
		uart_send_char(hex_to_ascii(Broadcast_Add,1));
		uart_send_char(hex_to_ascii(Broadcast_Add,0));			
	}

	//功能码
	uart_send_char(hex_to_ascii(Read_Holding_Registers,1));
	uart_send_char(hex_to_ascii(Read_Holding_Registers,0));

	//发送数据数量--字节数
	uart_send_char(hex_to_ascii((2*reg_num),1)); //2个字节
	uart_send_char(hex_to_ascii((2*reg_num),0));


	for(i=0;i<reg_num;i++)
	   {
			uart_send_char(hex_to_ascii((modbus_reg[reg_base+i]>>8),1));
			uart_send_char(hex_to_ascii((modbus_reg[reg_base+i]>>8),0));
			uart_send_char(hex_to_ascii(modbus_reg[reg_base+i],1));
			uart_send_char(hex_to_ascii(modbus_reg[reg_base+i],0));
			send_lrc_check = send_lrc_check+(modbus_reg[reg_base+i]>>8)+modbus_reg[reg_base+i];   	
	   }

	if(RX_BUF[0]==HOST_ID)//主机ID
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

	//回车 换行
	uart_send_char(0x0d);
	uart_send_char(0x0a);
}
unsigned char hex_to_ascii(unsigned char hex_data,unsigned char hign_low)
{
	unsigned char temp_data;
	if(hign_low==0)//转化低位为ascii
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