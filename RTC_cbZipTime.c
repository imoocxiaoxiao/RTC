/************************************************************************

                   ʵʱʱ��ģ��-�Խ�ZipTimeģ��Ļص�����ʵ��

************************************************************************/

#include "RTC.h"
#include "RTC_Bak.h"
#include "ZipTime.h"

//------------------------------����ʵʱʱ��----------------------------
void RTC_cbUpdate(unsigned char RelativeYear, //�����ʼ��,0-63
                  unsigned char Mouth,
                  unsigned char Date,
                  unsigned char Hour,
                  unsigned char Minute,
                  unsigned char Sec)
{
  //����ʵʱʱ��
  ZipTime_UdateRTC(ZipTime_FromTime(RelativeYear,
                                    Mouth,
                                    Date,
                                    Hour,
                                    Minute,
                                    Sec));
  if((Sec % 10) == 0)//�������һ��
    RTC_Bak_Update(); //���±���ʱ��
}

//-----------------------����ʵʱʱ����ZipTime��ʽ----------------------------
void RTC_cbUpdateZipTime(unsigned long ZipTime)
{
  ZipTime_UdateRTC(ZipTime);
  if((ZipTime & 0x07) == 0)//�������һ��
    RTC_Bak_Update(); //���±���ʱ��  
}




