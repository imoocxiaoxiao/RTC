/************************************************************************

                   实时时钟模块-在STM32F4x中的实现(直接操作寄存器,未调试完成)
使用外部32768时钟
************************************************************************/

#include "RTC.h"
#include "stm32f4xx.h"
#include <string.h>

/************************************************************************
                          内部函数
************************************************************************/
//-----------------------------写保护相关----------------------------
//除访问RTC的RTC_ISR[13:8], RTC_TAFCR, and RTC_BKPxR外，其它均需解除保护
#define _UnLock() do{RTC->WPR = 0xCA; RTC->WPR = 0x53;}while(0)      //写保护
#define _Lock() do{PWR->CR |= PWR_CR_DBP; RTC->WPR = 0xFF; }while(0)  //去写保护

//-----------------------------进出配置模式相关----------------------------
//返回0未进入
static signed char _EnterInitMode(void)//进入配置模式
{
  RTC->ISR |= RTC_ISR_INIT; //置进入标志
  //等待进入
  for(unsigned short Wait = 0xfff; Wait > 0; Wait--){
    if(RTC->ISR & RTC_ISR_INITF) return -1; 
  }
  return 0;
}
#define _ExitInitMode() do{RTC->ISR &= ~RTC_ISR_INIT;}while(0)//退出配置模式

//------------------------------等待RTC同步------------------------------
//返回是否完成
static signed char _WaitForSynchro(void)
{
  RTC->ISR  &= ~(RTC_ISR_RSF | RTC_ISR_INIT);
  //等待
  for(unsigned long Wait = 0x00020000; Wait > 0; Wait--){
    if(RTC->ISR & RTC_ISR_RSF) return -1; 
  }
  return 0;
}

//---------------------------------走时间-------------------------------
static void _StartRTC(void)
{
   _UnLock(); //解锁 
  _EnterInitMode();//进入配置模式
  RTC->CR &= ~RTC_CR_FMT; //24小时制
  RTC->PRER = 32767; //写预分频器以在设置后走时间->假定为外部32768HZ
  _ExitInitMode(); //退出配置模式
  _Lock();   //加锁
}

//-------------------------------设置RTC时间-----------------------------
//Pos: RTC_SetInfo设置的信息,0xff置默认时间,其它不处理时间
static void _UpdateDateTime(unsigned char Pos,
                        unsigned long DateTime)//待写入的数据
{
  _UnLock();  //去除写保护 
  
  //更新时间
  if(Pos < 3) RTC->DR = DateTime; //年月日时
  else if(Pos < 6) RTC->TR = DateTime;//时分秒时
  else if(Pos == 0xff){//默认时间
    RTC->DR = 0x00192101; //19年1月1日星期2
    RTC->TR = 0x00000000; //0点开始   
  }
  //else 其它不默认
  _Lock(); //加上保护
  
  _StartRTC(); //设置后强制走时间
}

//------------------------------BCD转换-----------------------------
static unsigned char _Bcd2Data(unsigned char BCD)
{
  return (BCD >> 4) * 10 + BCD & 0x0f;
}
static unsigned char _Data2Bcd(unsigned char Data)
{
  return ((Data / 10) << 4) | (Data % 10);
}

/************************************************************************
                           相关函数实现
************************************************************************/

//---------------------------------初始化函数----------------------------
void RTC_Init0(signed char IsInited)
{
  RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_AHB1ENR_BKPSRAMEN; //先开启电源与备份SRAM 
  //=========================外部基础配置===========================
  PWR->CR |= PWR_CR_DBP;  //先去除写保护   CR_DBP_BB = 1; //
  RCC->BDCR |= RCC_BDCR_LSEON; //开启外部时钟
  //检查外部时钟时否开启
  for(unsigned long Wait = 0xfffff; Wait > 0; Wait--){
    if(RCC->BDCR & RCC_BDCR_LSERDY) break; 
  }
  //配置时钟源为外部LSE(0无1外2内3快)
  RCC->BDCR |= RCC_BDCR_RTCSEL_1;
  RCC->BDCR &= ~RCC_BDCR_RTCSEL_0;
  //使允RTC时钟
  RCC->BDCR |= RCC_BDCR_RTCEN;  
  _WaitForSynchro();//等待RTC寄存器同步才能写
  //=========================内部基础配置===========================
  _StartRTC(); //强制走时间

  if(!IsInited)//未初始化时
    _UpdateDateTime(0xff, 0); //置默认时间(时间为默认)
  else _UpdateDateTime(6, 0); //配置为走时间(不改日期)
}

//--------------------------------任务函数-------------------------------
//建议256ms更新一次以更新实时时间
void RTC_Task(void)
{
  //一次性读出,要求读两次，不对第三次就对了
  unsigned long DR = RTC->DR;
  if(DR != RTC->DR) DR = RTC->DR;
  unsigned long TR = RTC->TR;
  if(TR != RTC->TR) TR = RTC->TR;
  
  //更新RTC
  RTC_cbUpdate(_Bcd2Data(_Bcd2Data((DR & (RTC_DR_YU | RTC_DR_YT)) >> RTC_DR_YU_SHIFT)),
               _Bcd2Data(_Bcd2Data((DR & (RTC_DR_MU | RTC_DR_MT)) >> RTC_DR_MU_SHIFT)),
               _Bcd2Data(_Bcd2Data((DR & (RTC_DR_DU | RTC_DR_DT)) >> RTC_DR_DU_SHIFT)),               
               _Bcd2Data(_Bcd2Data((TR & (RTC_TR_HT | RTC_TR_HU)) >> RTC_TR_HU_SHIFT)),
               _Bcd2Data(_Bcd2Data((TR & (RTC_TR_MNT | RTC_TR_MNU)) >> RTC_TR_MNU_SHIFT)),
               _Bcd2Data(_Bcd2Data((TR & (RTC_TR_ST | RTC_TR_SU)) >> RTC_TR_SU_SHIFT)));              
}

//-------------------------------设置RTC信息-----------------------------
//Pos定义为:相对年0,月1,日2,时3,分4,秒5
void RTC_SetInfo(unsigned char Pos,
                 unsigned char Data)
{
  //获得写入数据
  unsigned long DateTime;
  if(Pos < 3){//年月日时 
    DateTime = RTC->DR;
    if(Pos == 0){//年时
      DateTime &= ~(RTC_DR_YU | RTC_DR_YT);
      DateTime |= _Data2Bcd(Data) << RTC_DR_YU_SHIFT;
    }
    else if(Pos == 1){//月时
      DateTime &= ~(RTC_DR_MU | RTC_DR_MT);
      DateTime |= _Data2Bcd(Data) << RTC_DR_MU_SHIFT;
    }
    else{//日时
      DateTime &= ~(RTC_DR_DU | RTC_DR_DT);
      DateTime |= _Data2Bcd(Data) << RTC_DR_DU_SHIFT;
    }    
  }
  else{//时分秒时
    DateTime = RTC->TR;
    if(Pos == 3){//时时
      DateTime &= ~(RTC_TR_HT | RTC_TR_HU);
      DateTime |= _Data2Bcd(Data) << RTC_TR_HU_SHIFT;
    }
    else if(Pos == 4){//分时
      DateTime &= ~(RTC_TR_MNT | RTC_TR_MNU);
      DateTime |= _Data2Bcd(Data) << RTC_TR_MNU_SHIFT;
    }
    else{//秒时
      DateTime &= ~(RTC_TR_ST | RTC_TR_SU);
      DateTime |= _Data2Bcd(Data) << RTC_TR_SU_SHIFT;
    } 
  }
  _UpdateDateTime(Pos, DateTime); //更新
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


