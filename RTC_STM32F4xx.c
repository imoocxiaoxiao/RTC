/************************************************************************

                 实时时钟模块-在STM32F4x中,使用STM32库函数的实现
使用外部32768时钟
************************************************************************/

#include "RTC.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_pwr.h"
#include "Delay.h"
#include <string.h>

/************************************************************************
          原函数(代码来自正点原子，修改RTC_Init1接口及初始化部分)
************************************************************************/
 
//RTC时间设置
//hour,min,sec:小时,分钟,秒钟
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
}
//RTC日期设置
//year,month,date:年(0~99),月(1~12),日(0~31)
//week:星期(1~7,0,非法!)
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;
	RTC_DateTypeInitStructure.RTC_Year=year;
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
}
 
//RTC初始化
//返回值:0,初始化成功;
//       1,LSE开启失败;
//       2,进入初始化模式失败;
u8 RTC_Init1(signed char IsInited)
{
	RTC_InitTypeDef RTC_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 
	
	//if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050)		//是否第一次配置?->开机重配置一次但时间不变
	//{

    /*/LSE 开启 
		RCC_LSEConfig(RCC_LSE_ON);   
    u16 retry=0X1FFF; 
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
	  {
			retry++;
			DelayMs(10);
      if(retry==0)return 1;		//LSE 开启失败. 
		}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟*/
    
    //LSI开启
    RCC_LSICmd(ENABLE);// Enable the LSI OSC           
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET); // Wait till LSI is ready 
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);//注：只有去掉电池掉电，才能切回时钟源
    
    //使能RTC时钟 
		RCC_RTCCLKCmd(ENABLE);	
    RTC_WaitForSynchro();//->后加入
    //LSE时
    //RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC异步分频系数(1~0X7F)-
    //RTC_InitStructure.RTC_SynchPrediv  = 0x0F;//RTC同步分频系数(0~7FFF)>原FF
    //LSI时
    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC异步分频系数(1~0X7F)-
    RTC_InitStructure.RTC_SynchPrediv  = 0xFF;//RTC同步分频系数(0~7FFF)   
    
    
	if(!IsInited || ((RTC->DR & RTC_DR_YT) == 0))		//第一次配置或校验日期复位时置时间
	{   
    RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//RTC设置为,24小时格式
    RTC_Init(&RTC_InitStructure);

		RTC_Set_Time(23,59,56,RTC_H12_AM);	//设置时间
		RTC_Set_Date(18,12,31,1);		        //设置日期
    //RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//标记已经初始化过了
	} 
 
	return 0;
}

/************************************************************************
                          接口函数实现
************************************************************************/
//---------------------------------初始化函数实现----------------------------
void RTC_Init0(signed char IsInited)
{
  RTC_Init1(IsInited);
  RTC_Task();//强制更新一次
}

//--------------------------------任务函数实现-------------------------------
//建议256ms更新一次以更新实时时间
void RTC_Task(void)
{
  RTC_TimeTypeDef Time;
  RTC_GetTime(RTC_Format_BIN, &Time);
  RTC_DateTypeDef Date;
  RTC_GetDate(RTC_Format_BIN, &Date);
  
  //更新RTC
  RTC_cbUpdate(Date.RTC_Year, Date.RTC_Month, Date.RTC_Date,
               Time.RTC_Hours,Time.RTC_Minutes, Time.RTC_Seconds);              
}

//-------------------------------设置RTC信息-----------------------------
//Pos定义为:相对年0,月1,日2,时3,分4,秒5
void RTC_SetInfo(unsigned char Pos,
                 unsigned char Data)
{
  if(Pos <= 2){//修改日期
    RTC_DateTypeDef Date;
    RTC_GetDate(RTC_Format_BIN, &Date);
    if(Pos == 0) Date.RTC_Year = Data;
    else if(Pos == 1) Date.RTC_Month = Data;
    else Date.RTC_Date = Data;
    RTC_SetDate(RTC_Format_BIN, &Date);
  }
  else{//修改时间
    RTC_TimeTypeDef Time;
    RTC_GetTime(RTC_Format_BIN, &Time);    
    if(Pos == 3) Time.RTC_Hours = Data;
    else if(Pos == 4) Time.RTC_Minutes = Data;
    else Time.RTC_Seconds = Data;
    RTC_SetTime(RTC_Format_BIN, &Time);    
  }
}

//------------------------------写备份数据------------------------------
//用于保存掉电不丢失的数据，为保证与32位硬件兼容，为32Bit模式
void RTC_WrBakData(unsigned char Adr,         //保存位置
                   const unsigned long *pData,//要保存的数据
                   unsigned char Len)          //数据长度
{
  unsigned char EndPos = Adr + Len;
  if(EndPos >= 20) return; //超过BKPxR范围了
  //BKPxR为寄存器，不能memcpy
  for(; Adr < EndPos; Adr++){
    RTC->BKPR[Adr] = *pData++;
  }
}

//------------------------------读备份数据---------------------------------
//用于保存掉电不丢失的数据，为保证与32位硬件兼容，为32Bit模式
void RTC_RdBakData(unsigned char Adr,     //保存位置
                   unsigned long *pData,//要保存的数据
                   unsigned char Len)   //数据长度
{
  unsigned char EndPos = Adr + Len;
  if(EndPos >= 20) EndPos = 20; //超过BKPxR范围了
  //BKPxR为寄存器，不能memcpy
  for(; Adr < EndPos; Adr++){
    *pData++ = RTC->BKPR[Adr];
  }
}


