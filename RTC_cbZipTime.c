/************************************************************************

                   实时时钟模块-对接ZipTime模块的回调函数实现

************************************************************************/

#include "RTC.h"
#include "RTC_Bak.h"
#include "ZipTime.h"

//------------------------------更新实时时间----------------------------
void RTC_cbUpdate(unsigned char RelativeYear, //相对起始年,0-63
                  unsigned char Mouth,
                  unsigned char Date,
                  unsigned char Hour,
                  unsigned char Minute,
                  unsigned char Sec)
{
  //更新实时时间
  ZipTime_UdateRTC(ZipTime_FromTime(RelativeYear,
                                    Mouth,
                                    Date,
                                    Hour,
                                    Minute,
                                    Sec));
  if((Sec % 10) == 0)//间隔更新一次
    RTC_Bak_Update(); //更新备份时间
}

//-----------------------更新实时时间用ZipTime格式----------------------------
void RTC_cbUpdateZipTime(unsigned long ZipTime)
{
  ZipTime_UdateRTC(ZipTime);
  if((ZipTime & 0x07) == 0)//间隔更新一次
    RTC_Bak_Update(); //更新备份时间  
}




