#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "usmart.h" 
#include "rs485.h"
#include "jy901.h"
//ALIENTEK ̽����STM32F407������ ʵ��26
//485 ʵ��  
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾
    
int main(void)
{   
	Stm32_Clock_Init(336,8,2,7);//����ʱ��,168Mhz 
	delay_init(168);			//��ʱ��ʼ��  
	uart_init(84,115200);		//��ʼ�����ڲ�����Ϊ115200  	 
	uart3_init(42,115200);
//	usmart_dev.init(84);		//��ʼ��usmart
	LED_Init();					//��ʼ��LED 
// 	LCD_Init();					//LCD��ʼ�� 
//	KEY_Init(); 				//������ʼ��  
	RS485_Init(42,115200);		//��ʼ��RS485
	
	while(1)
	{
ModbusRWReg(allowAddeId[3],imuRead,Roll,3);
delay_ms(3);
ModbusRWReg(allowAddeId[4],imuRead,Roll,3);
delay_ms(3);
ModbusRWReg(allowAddeId[5],imuRead,Roll,3);
delay_ms(3);
		
//ModbusRWReg(allowAddeId[3],imuRead,AX,3);
//delay_ms(3);
//ModbusRWReg(allowAddeId[4],imuRead,AX,3);
//delay_ms(3);
//ModbusRWReg(allowAddeId[5],imuRead,AX,3);
//delay_ms(3);
		
ModbusRWReg(allowAddeId[3],imuRead,GX,3);
delay_ms(3);
ModbusRWReg(allowAddeId[4],imuRead,GX,3);
delay_ms(3);
ModbusRWReg(allowAddeId[5],imuRead,GX,3);
delay_ms(3);
//		if(stcIMU[4].x_Angle>stcIMU[5].x_Angle){
//			if(stcIMU[4].x_w>10&&(stcIMU[4].x_Angle>100&&stcIMU[4].x_Angle<130)){
//				printf("left start\r\n");
//			}
//		}else{
//			if(stcIMU[5].x_w>10&&(stcIMU[5].x_Angle>100&&stcIMU[5].x_Angle<130)){
//				printf("rigt start\r\n");
//			}
//		}
	}   
}

















