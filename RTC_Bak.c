/************************************************************************

                          保存实时RTC功能模块
在开机后实时保存RTC时钟至RTC模块提供的备份区，在开机后读取可获得关机赶时间
************************************************************************/

#include "RTC_Bak.h"
#include "RTC.h"
#include "ZipTime.h"

/************************************************************************
                           相关函数
************************************************************************/

//---------------------------更新函数-------------------------------
//在更新RTC时后立即调用
void RTC_Bak_Update(void)
{
  if(RTC_Bak_cbIsFinalRdRTC())//读取过才更新防止覆盖
    RTC_WrBakData(0, (unsigned char*)(&ZipTime_GetRTC()), 4);
}

//---------------------------获得函数-------------------------------
//在开机更新前调用,返回0表示无效
unsigned long RTC_Bak_Get(void)
{
  unsigned long ZipTime;
  RTC_RdBakData(0, (unsigned char*)(&ZipTime), 4);
  RTC_Bak_cbSetFinalRdRTC(); //置读取过标志
  
  if(ZipTime_Check(ZipTime) != 0) return 0;//校验错误
  return ZipTime;
}
