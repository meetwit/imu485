#include <string.h>
#include "JY901.h"
#include "stdio.h"
#include "rs485.h"
#include "led.h"
#include "ANO.h"
#include "usart.h"	 

/**********************************************************************************************
jy901 modbus 协议解析
包含文件：jy901.h,jy901.c
移植①：修改模块波特率和程序485波特率，使之相等
		②：完善RS485_Send_Data函数实体，参数为要发送的字节
		③：在串口中断调用CopeSerialData，参数位收到的字节
		④：根据你的modbus节点数量填写imuNum的宏定义
		⑤：根据你节点填写allowAddeId数组内容，毋须按照顺序，填进去即可
		⑥：解析后的调用，使用stcIMU[]结构体数组调用相应数据
		
日期：	2019年1月23日
作者：	meetwit
***********************************************************************************************/

struct STime		stcTime={0};
struct SAcc 		stcAcc={0};
struct SGyro 		stcGyro={0};
struct SAngle 		stcAngle={0};
struct SMag 		stcMag={0};
struct SDStatus 	stcDStatus={0};
struct SPress 		stcPress={0};
struct SLonLat 		stcLonLat={0};
struct SGPSV 		stcGPSV={0};

struct IMU			stcIMU[imuNum]={0};												//对外调用该结构体即可

unsigned char allowAddeId[imuNum]={0x50,0x51,0x52,0x53,0x54,0x55};				//系统中modbus节点地址

unsigned char auchCRCHi[256] ={
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
	0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
	0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
	0x40} ;
unsigned char auchCRCLo[256] ={
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
	0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 
	0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 
	0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 
	0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 
	0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 
	0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91, 
	0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 
	0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 
	0x40 };
	
	//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 

unsigned short CRC16(unsigned char *puchMsg,unsigned short usDataLen) //CRC校验
		{ 
				char uchCRCHi = 0xFF ; /* ? CRC 高位 */
				char uchCRCLo = 0xFF ; /* ? CRC低位*/
				int uIndex ; /* CRC 索引 */
				int i = 0;
				for (;i<usDataLen;i++)
				{
						uIndex = (int)(uchCRCHi ^ puchMsg[i]) ; /* ?? CRC */
						uchCRCHi = (char)(uchCRCLo ^ auchCRCHi[uIndex]) ;
						uchCRCLo = auchCRCLo[uIndex] ;
				}
				return (unsigned short)(((short)uchCRCHi << 8) | (short)uchCRCLo) ;
		}

/*
函数名：	send485
传	参：	data
返回值：	void
		作	用：	485发送实体函数
日	期：	2019年1月22日
作	者：	meetwit		
*/		
void send485(unsigned char data){
		RS485_Send_Data(&data,sizeof(unsigned char));	
}


/*
函数名：	CopeSerialData
传	参：	ucData 串口收到的字节
返回值：	void
作	用：	解析jy901接收到的数据
日	期：	2019年1月22日
作	者：	meetwit
*/
void CopeSerialData(unsigned char ucData)
{
	static unsigned char ucRxBuffer[11];
	static unsigned char ucRxCnt = 0;	
	static unsigned char ucRxLen = 0;	
	static float temp1 = 208.98;
	static float temp2 = 16.384;
	static float temp3 = 182.04;
	static unsigned char  index = 0,curId[imuNum];
	
	
	static  float ano_out[10] = {0},angle_change_history[3] = {0},angle_change_time[2] = {0},assi_time[2] = {0},GX_history[2][3]={0},P_flag = 0;
	
	ucRxBuffer[ucRxCnt++]=ucData;
	if(ucRxCnt==1){
		return;
	}
	if(ucRxCnt==2){
		if (ucRxBuffer[1]!=0x03)
		{
			ucRxCnt=0;																															  
		}
			return;		
	}
	if(ucRxCnt==3){
			ucRxLen = ucRxBuffer[2]+5;
			return;		
	}
	if (ucRxCnt<ucRxLen) {
		return;
		}
	else
	{
			for(index=0;index<imuNum;index++){
				if(stcIMU[index].addrId==ucRxBuffer[0]){
					break;
				}
			}
		if(stcIMU[index].addrReg==Roll){				//角度
			stcAngle.Angle[0] = ((unsigned short)ucRxBuffer[3]<<8)|ucRxBuffer[4];
			stcAngle.Angle[1] = ((unsigned short)ucRxBuffer[5]<<8)|ucRxBuffer[6];
			stcAngle.Angle[2] = ((unsigned short)ucRxBuffer[7]<<8)|ucRxBuffer[8];
			stcIMU[index].x_Angle = stcAngle.Angle[0]/temp3;		//单位：°
			stcIMU[index].y_Angle = stcAngle.Angle[1]/temp3;
			stcIMU[index].z_Angle = stcAngle.Angle[2]/temp3;
			
			/*年前触发部分开始*/
//	if(stcIMU[index].x_Angle>110&&curId[index]){
//		//printf("%d",stcIMU[index].addrId);
//		while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
//		USART1->DR =  stcIMU[index].addrId;  
//		curId[index] = 0;
//		if(index==1) LED0=0;
//		if(index==2) LED1=0;
//	}else if(stcIMU[index].x_Angle<=110){
//		curId[index] = 1;
//		if(index==1) LED0=1;
//		if(index==2) LED1=1;
//	}
			/*年前触发部分结束*/
			
			/*ano_out[8] 为助力输出 ，限制峰值为正负100*/
		if(stcIMU[4].x_Angle>stcIMU[5].x_Angle){
			if(stcIMU[4].x_w>20&&(stcIMU[4].x_Angle>100&&stcIMU[4].x_Angle<130)){
				ano_out[8] = 100;
					assi_time[0]++;
				assi_time[1] = 0;
			}else{
				ano_out[8] = 0;
					assi_time[1] ++;
				assi_time[0] = 0;
			}
			if(assi_time[0]==2){
				PcTx_Byte3('s');
				ano_out[9] = 150;
			}else if(assi_time[1]==2){
				PcTx_Byte3('e');
				ano_out[9] = 300;
			}
			
		}
//		else{
//			if(stcIMU[5].x_w>10&&(stcIMU[5].x_Angle>100&&stcIMU[5].x_Angle<130)){
//				printf("rigt start\r\n");
//			}
//		}	
			
//printf("%d,%d,%d,%d,%7.2f,%7.2f,%7.2f\r\n",Roll,ucRxBuffer[0],stcIMU[index].addrId,ucRxCnt,stcIMU[index].x_Angle,stcIMU[index].y_Angle,stcIMU[index].z_Angle);
//printf("%d,%7.2f\r\n",stcIMU[index].addrId,stcIMU[index].x_Angle);
		/*ano_out[0 1] 输出的是角度*/
ano_out[stcIMU[index].addrId%2] = stcIMU[index].x_Angle;
		/*ano_out[4] 输出的是左右脚角度的差*/
ano_out[4] = ano_out[0]-ano_out[1];
ano_out[5] = -ano_out[4];

		/*ano_out[6] 输出到匿名 角度的差的变化量 的 互补滤波*/
ano_out[6] = (ano_out[4]-angle_change_history[0])*5+(angle_change_history[0]-angle_change_history[1])*3+(angle_change_history[1]-angle_change_history[2])*2;
		/*角度的差的变化量 转化为 方波 峰值为+-200*/
		if(ano_out[6]>10){
//			ano_out[6] = 200;
			angle_change_time[1]++;
		}else if(ano_out[6]<-10){
//			ano_out[6] = -200;
			angle_change_time[0]++;
			P_flag = 1;
		}else{
//			ano_out[6] = 0;
			angle_change_time[1] = 0;
			angle_change_time[0] = 0;
		}
		if(angle_change_time[1] >3){
			ano_out[6] = 200;
		}else if(angle_change_time[0] >3){
			ano_out[6] = -200;
		}else{
			ano_out[6] = 0;
		}
		
		if(angle_change_time[1]==3&&P_flag){
			PcTx_Byte3('m');
			ano_out[9] = 50;
			P_flag = 0;
		}

ano_out[7] = -ano_out[6];

angle_change_history[2] = angle_change_history[1];
angle_change_history[1] = angle_change_history[0];
angle_change_history[0] = ano_out[4];

ANO_send(0xf2,(unsigned char *)ano_out,sizeof(float),sizeof(ano_out));
		}else if(stcIMU[index].addrReg==AX){		//加速度
			stcAcc.a[0] = ((unsigned short)ucRxBuffer[3]<<8)|ucRxBuffer[4];
			stcAcc.a[1] = ((unsigned short)ucRxBuffer[5]<<8)|ucRxBuffer[6];
			stcAcc.a[2] = ((unsigned short)ucRxBuffer[7]<<8)|ucRxBuffer[8];
			stcIMU[index].x_a = stcAcc.a[0]/temp1;			//单位：m/s^2
			stcIMU[index].y_a = stcAcc.a[1]/temp1;
			stcIMU[index].z_a = stcAcc.a[2]/temp1;
//printf("%d,%d,%d,%d,%7.2f,%7.2f,%7.2f\r\n",AX,ucRxBuffer[0],stcIMU[index].addrId,ucRxCnt,stcIMU[index].x_a,stcIMU[index].y_a,stcIMU[index].z_a);
		}else if(stcIMU[index].addrReg==GX){		//角速度
			stcGyro.w[0] = ((unsigned short)ucRxBuffer[3]<<8)|ucRxBuffer[4];
			stcGyro.w[1] = ((unsigned short)ucRxBuffer[5]<<8)|ucRxBuffer[6];
			stcGyro.w[2] = ((unsigned short)ucRxBuffer[7]<<8)|ucRxBuffer[8];
			stcIMU[index].x_w = stcGyro.w[0]/temp2;		//单位：°/s
			stcIMU[index].y_w = stcGyro.w[1]/temp2;
			stcIMU[index].z_w = stcGyro.w[2]/temp2;
			/*ano_out[2 3] 输出的是角速度。互补滤波*/
			ano_out[stcIMU[index].addrId%2+2] = (stcIMU[index].x_w)*0.5+GX_history[stcIMU[index].addrId%2][0]*0.3+GX_history[stcIMU[index].addrId%2][1]*0.2;
			GX_history[stcIMU[index].addrId%2][2] = GX_history[stcIMU[index].addrId%2][1];
			GX_history[stcIMU[index].addrId%2][1] = GX_history[stcIMU[index].addrId%2][0];
			GX_history[stcIMU[index].addrId%2][0] = stcIMU[index].x_w;
//printf("%d,%d,%d,%d,%7.2f,%7.2f,%7.2f\r\n",GX,ucRxBuffer[0],stcIMU[index].addrId,ucRxCnt,stcIMU[index].x_w,stcIMU[index].y_w,stcIMU[index].z_w);
		}
		ucRxCnt=0;
	}
}

/*
函数名：	ModbusRWReg
参	数：	Addr					：从机地址		
					R_W						：==0x03/0x06		读命令/写命令
					usReg					：读/写	寄存器地址			
					usRegNumDate	：写的内容/读的个数
返回值：	void
作	用：	modbus读写的协议
时	间：	2019年1月22日
作	者：	meetwit
*/
void 	ModbusRWReg(unsigned short Addr, unsigned short R_W, unsigned short usReg, unsigned short usRegNumDate){
			char i=0,j=0;
      char cIndex=0;
	    unsigned char record[8];
	    short checkCRC;
			for(j=0;j<imuNum;j++){
				if(allowAddeId[j]==Addr){
					stcIMU[j].addrId = Addr;
					stcIMU[j].addrReg = usReg;
					break;
				}
			}
	    record[cIndex++]=Addr;
	    record[cIndex++]=R_W;
	    record[cIndex++]=usReg<<8;
	    record[cIndex++]=usReg&0xff;
	    record[cIndex++]=usRegNumDate<<8;
	    record[cIndex++]=usRegNumDate&0xff;
	    checkCRC=CRC16(record,cIndex);
	    record[cIndex++]=checkCRC&0xff; 
	    record[cIndex++]=(checkCRC>>8)&0xff;
		  for(i=0;i<=cIndex;i++)
			send485(record[i]);
}
