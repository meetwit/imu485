#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "usmart.h" 
#include "rs485.h"
#include "jy901.h"
//ALIENTEK 探索者STM32F407开发板 实验26
//485 实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司
    
int main(void)
{   
	Stm32_Clock_Init(336,8,2,7);//设置时钟,168Mhz 
	delay_init(168);			//延时初始化  
	uart_init(84,115200);		//初始化串口波特率为115200  	 
	uart3_init(42,115200);
//	usmart_dev.init(84);		//初始化usmart
	LED_Init();					//初始化LED 
// 	LCD_Init();					//LCD初始化 
//	KEY_Init(); 				//按键初始化  
	RS485_Init(42,115200);		//初始化RS485
	
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

















