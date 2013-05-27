#ifndef MODBUS_SLAVE_H_
	#define MODBUS_SLAVE_H_
	
	#define FLAG unsigned char 
	#define uchar unsigned char
	#define uint unsigned int
	#define STATE_REC_HEADER    			1
	#define STATE_REC_DATA			      	2
	#define STATE_REC_END       			3
	#define Read_Holding_Registers      0x03
	#define Write_Single_Register		0x06
	#define Broadcast_Add 32

	#define RS485 //用于选择232方式还是485方式	

	//extern uint xdata modbus_reg[];
	extern uint modbus_reg[];
	//-------------------------------------------
	void modbus_decode(void);
	void feedback_write_reg(void);//向上位机反馈写寄存器命令
	void feedback_read_reg(unsigned int reg_num,unsigned int reg_base);//读取的数据的数量 基址偏移
	unsigned char hex_to_ascii(unsigned char hex_data,unsigned char hign_low);
	unsigned char ascii_to_hex(unsigned char ascii_data);
	#ifdef RS485
		void delay_for_dir485(unsigned int delay_time);
	#endif
#endif