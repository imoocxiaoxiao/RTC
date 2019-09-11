/************************************************************************

                 ʵʱʱ��ģ��-��STM32F4x��,ʹ��STM32�⺯����ʵ��
ʹ���ⲿ32768ʱ��
************************************************************************/

#include "RTC.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include "Delay.h"
#include <string.h>

/************************************************************************
          ԭ����(������������ԭ�ӣ��޸�RTC_Init1�ӿڼ���ʼ������)
************************************************************************/
 
//RTCʱ������
//hour,min,sec:Сʱ,����,����
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ�� 
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
}
//RTC��������
//year,month,date:��(0~99),��(1~12),��(0~31)
//week:����(1~7,0,�Ƿ�!)
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ�� 
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;
	RTC_DateTypeInitStructure.RTC_Year=year;
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
}
 
//RTC��ʼ��
//����ֵ:0,��ʼ���ɹ�;
//       1,LSE����ʧ��;
//       2,�����ʼ��ģʽʧ��;
u8 RTC_Init1(signed char IsInited)
{
	RTC_InitTypeDef RTC_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//ʹ��PWRʱ��
	PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ������� 
	
	//if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050)		//�Ƿ��һ������?->����������һ�ε�ʱ�䲻��
	//{

    /*/LSE ���� 
		RCC_LSEConfig(RCC_LSE_ON);   
    u16 retry=0X1FFF; 
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//���ָ����RCC��־λ�������,�ȴ����پ������
	  {
			retry++;
			DelayMs(10);
      if(retry==0)return 1;		//LSE ����ʧ��. 
		}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��*/
    
    //LSI����
    RCC_LSICmd(ENABLE);// Enable the LSI OSC           
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET); // Wait till LSI is ready 
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);//ע��ֻ��ȥ����ص��磬�����л�ʱ��Դ
    
    //ʹ��RTCʱ�� 
		RCC_RTCCLKCmd(ENABLE);	
    RTC_WaitForSynchro();//->�����
    //LSEʱ
    //RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC�첽��Ƶϵ��(1~0X7F)-
    //RTC_InitStructure.RTC_SynchPrediv  = 0x0F;//RTCͬ����Ƶϵ��(0~7FFF)>ԭFF
    //LSIʱ
    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC�첽��Ƶϵ��(1~0X7F)-
    RTC_InitStructure.RTC_SynchPrediv  = 0xFF;//RTCͬ����Ƶϵ��(0~7FFF)   
    
    
	if(!IsInited || ((RTC->DR & RTC_DR_YT) == 0))		//��һ�����û�У�����ڸ�λʱ��ʱ��
	{   
    RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//RTC����Ϊ,24Сʱ��ʽ
    RTC_Init(&RTC_InitStructure);

		RTC_Set_Time(23,59,56,RTC_H12_AM);	//����ʱ��
		RTC_Set_Date(18,12,31,1);		        //��������
    //RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//����Ѿ���ʼ������
	} 
 
	return 0;
}

/************************************************************************
                          �ӿں���ʵ��
************************************************************************/
//---------------------------------��ʼ������ʵ��----------------------------
void RTC_Init0(signed char IsInited)
{
  RTC_Init1(IsInited);
  RTC_Task();//ǿ�Ƹ���һ��
}

//--------------------------------������ʵ��-------------------------------
//����256ms����һ���Ը���ʵʱʱ��
void RTC_Task(void)
{
  RTC_TimeTypeDef Time;
  RTC_GetTime(RTC_Format_BIN, &Time);
  RTC_DateTypeDef Date;
  RTC_GetDate(RTC_Format_BIN, &Date);
  
  //����RTC
  RTC_cbUpdate(Date.RTC_Year, Date.RTC_Month, Date.RTC_Date,
               Time.RTC_Hours,Time.RTC_Minutes, Time.RTC_Seconds);              
}

//-------------------------------����RTC��Ϣ-----------------------------
//Pos����Ϊ:�����0,��1,��2,ʱ3,��4,��5
void RTC_SetInfo(unsigned char Pos,
                 unsigned char Data)
{
  if(Pos <= 2){//�޸�����
    RTC_DateTypeDef Date;
    RTC_GetDate(RTC_Format_BIN, &Date);
    if(Pos == 0) Date.RTC_Year = Data;
    else if(Pos == 1) Date.RTC_Month = Data;
    else Date.RTC_Date = Data;
    RTC_SetDate(RTC_Format_BIN, &Date);
  }
  else{//�޸�ʱ��
    RTC_TimeTypeDef Time;
    RTC_GetTime(RTC_Format_BIN, &Time);    
    if(Pos == 3) Time.RTC_Hours = Data;
    else if(Pos == 4) Time.RTC_Minutes = Data;
    else Time.RTC_Seconds = Data;
    RTC_SetTime(RTC_Format_BIN, &Time);    
  }
}

//------------------------------д��������------------------------------
//���ڱ�����粻��ʧ�����ݣ�Ϊ��֤��32λӲ�����ݣ�Ϊ32Bitģʽ
void RTC_WrBakData(unsigned char Adr,         //����λ��
                   const unsigned long *pData,//Ҫ���������
                   unsigned char Len)          //���ݳ���
{
  unsigned char EndPos = Adr + Len;
  if(EndPos >= 20) return; //����BKPxR��Χ��
  //BKPxRΪ�Ĵ���������memcpy
  for(; Adr < EndPos; Adr++){
    RTC->BKPR[Adr] = *pData++;
  }
}

//------------------------------����������---------------------------------
//���ڱ�����粻��ʧ�����ݣ�Ϊ��֤��32λӲ�����ݣ�Ϊ32Bitģʽ
void RTC_RdBakData(unsigned char Adr,     //����λ��
                   unsigned long *pData,//Ҫ���������
                   unsigned char Len)   //���ݳ���
{
  unsigned char EndPos = Adr + Len;
  if(EndPos >= 20) EndPos = 20; //����BKPxR��Χ��
  //BKPxRΪ�Ĵ���������memcpy
  for(; Adr < EndPos; Adr++){
    *pData++ = RTC->BKPR[Adr];
  }
}


