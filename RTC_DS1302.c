/************************************************************************

                 ʵʱʱ��ģ��-��DS1302�е�ʵ��

************************************************************************/

#include "RTC.h"
#include "Ds1302.h"     //����
#include "Ds1302Drv.h"  //����
#include "ZipTime.h"    //ֱ��ʹ��
#include <string.h>

unsigned char RTC_DS1302_Err;

//----------------------------------BCDת����-----------------------------------
unsigned char Bcd2Data(unsigned char Bcd)
{
  unsigned char Data = (Bcd >> 4) * 10;
  return Data + (Bcd & 0x0f);
}

//----------------------------------����תBCD-----------------------------------
unsigned char Data2Bcd(unsigned char Data)
{
  if(Data > 99) Data = 99;//��ֹ�쳣
  unsigned char Bcd = (Data / 10) << 4;
  return Bcd + (Data % 10);
}

//--------------------------------����ʱ�亯��ʵ��-------------------------------
static void _UpdateRTC(signed char IsUpdateAll)   //�����б�־
{
  //�ȸ�����
  unsigned char Bcd;
  DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_SEC ,&Bcd, 1);
  if(Bcd == 0xff){
    RTC_DS1302_Err = 0xff;//RTC���ϱ�־
    return;
  }
  
  RTC_DS1302_Err = 0;
  unsigned long ZipTime;
  
  if(Bcd & 0x80){//ʱ��û��ʱ,��������ʱ��
    ZipTime = ZipTime_GetRTC() & ZIP_TIME_SEC_MASK;
    RTC_SetInfo(5, ZipTime);
  }
  else ZipTime = Bcd2Data(Bcd); //���Ӹ�����

  //��Ϊ0-9ʱ��Ϊ��ֹͨѶ���ϣ����ڼ��θ��·��Ӽ�������Ϣ(����ģʽֻ�ܴ��뿪ʼ)
  if((!(Bcd & 0x70)) || IsUpdateAll){
    //��
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_MIN ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_MIN_SHIFT) & ZIP_TIME_MIN_MASK;
    //ʱ
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_HOUR ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_HOUR_SHIFT) & ZIP_TIME_HOUR_MASK;
    //��
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_DATE ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_DATE_SHIFT) & ZIP_TIME_DATE_MASK;    
    //��
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_MONTH ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_MOUTH_SHIFT) & ZIP_TIME_MOUTH_MASK;  
    //��
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_YEAR ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_YEAR_SHIFT) & ZIP_TIME_YEAR_MASK;      
  }
  else ZipTime |= (ZipTime_GetRTC() & ~ZIP_TIME_SEC_MASK);
  
  RTC_cbUpdateZipTime(ZipTime); //����RTC             
}

//------------------����RTC��Ϣʱ����λ�õ�ָ��----------------------------
static const unsigned char _PosToCmd[] = {
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_YEAR,  //��
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_MONTH, //��
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_DATE,  //��
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_HOUR,  //ʱ
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_MIN,   //��
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_SEC,   //��  
};

//-------------------------------����RTC��Ϣ-----------------------------
//Pos����Ϊ:�����0,��1,��2,ʱ3,��4,��5
void RTC_SetInfo(unsigned char Pos, unsigned char Data)
{
  //ȥ��д����
  unsigned char Bcd = 0;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CTR, &Bcd, 1);
  //дʱ��
  Bcd = Data2Bcd(Data);//ת��ΪBCD��
  DS1302_Wr(_PosToCmd[Pos], &Bcd, 1);  
  //��緽ʽͬʱд��
  Bcd = DS1302_CTR_R2k | DS1302_CTR_DS2 | DS1302_CTR_TCS;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CHRGE_CTR, &Bcd, 1);
  //����д����
  Bcd = DS1302_CTR_WP;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CTR, &Bcd, 1);
}

/***********************************************************************
                          �ӿں���ʵ��
************************************************************************/
//---------------------------��ʼ������ʵ��----------------------------
void RTC_Init0(signed char IsInited)
{
  DS1302_cbIOCfg();
 //����оƬ
  if(!IsInited){
    //��ʼ��ʱ��
    RTC_SetInfo(0, 20);
    RTC_SetInfo(1, 12);
    RTC_SetInfo(2, 31);    
    RTC_SetInfo(3, 23);
    RTC_SetInfo(4, 59);
    RTC_SetInfo(5, 56);   
  }
  _UpdateRTC(1);
  _UpdateRTC(1); //ǿ���ٸ���һ��
}

//--------------------------------������ʵ��-------------------------------
//����256ms����һ���Ը���ʵʱʱ��
void RTC_Task(void)
{
  _UpdateRTC(0);
}

//------------------------------д��������------------------------------
//���ڱ�����粻��ʧ������,��ַ+���Ȳ�Ӧ����RTC����,�ж���Ҫ��ʱ�����
void RTC_WrBakData(unsigned char Adr,         //����λ��
                   const unsigned char *pData,//Ҫ���������
                   unsigned char Len)          //���ݳ���
{
  //ȥ��д����
  unsigned char Bcd = 0;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CTR, &Bcd, 1);
  //д����
  if(Adr == 0)//ͻ����ʽдһ��
    DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RAM | DS1302_RA_BURST, pData, Len);
  else{//ֻ��һ����д��
    Adr = DS1302_EN | DS1302_WR | DS1302_RAM | (Adr << 1);
    for(; Len > 0; Len--, Adr += 2, pData++)//��д��־�����λ
      DS1302_Wr(Adr, pData, 1);    
  }
  //����д����
  Bcd = DS1302_CTR_WP;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CTR, &Bcd, 1);
}

//------------------------------����������---------------------------------
//���ڱ�����粻��ʧ������,��ַ+���Ȳ�Ӧ����RTC����,�ж���Ҫ��ʱ�����
void RTC_RdBakData(unsigned char Adr,     //����λ��
                   unsigned char *pData,//Ҫ���������
                   unsigned char Len)   //���ݳ���
{
  if(Adr == 0)//ͻ����ʽ�ɶ�һ��
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RAM | DS1302_RA_BURST, pData, Len);
  else{//ֻ��һ��������
    Adr = DS1302_EN | DS1302_RD | DS1302_RAM | (Adr << 1);
    for(; Len > 0; Len--, Adr += 2, pData++)//��д��־�����λ
      DS1302_Rd(Adr, pData, 1);    
  }
}


