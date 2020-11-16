/************************************************************************

                 实时时钟模块-在DS1302中的实现

************************************************************************/

#include "RTC.h"
#include "Ds1302.h"     //定义
#include "Ds1302Drv.h"  //驱动
#include "ZipTime.h"    //直接使用
#include <string.h>

unsigned char RTC_DS1302_Err;

//----------------------------------BCD转数据-----------------------------------
unsigned char Bcd2Data(unsigned char Bcd)
{
  unsigned char Data = (Bcd >> 4) * 10;
  return Data + (Bcd & 0x0f);
}

//----------------------------------数据转BCD-----------------------------------
unsigned char Data2Bcd(unsigned char Data)
{
  if(Data > 99) Data = 99;//防止异常
  unsigned char Bcd = (Data / 10) << 4;
  return Bcd + (Data % 10);
}

//--------------------------------更新时间函数实现-------------------------------
static void _UpdateRTC(signed char IsUpdateAll)   //读所有标志
{
  //先更新秒
  unsigned char Bcd;
  DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_SEC ,&Bcd, 1);
  if(Bcd == 0xff){
    RTC_DS1302_Err = 0xff;//RTC故障标志
    return;
  }
  
  RTC_DS1302_Err = 0;
  unsigned long ZipTime;
  
  if(Bcd & 0x80){//时间没走时,先让其走时间
    ZipTime = ZipTime_GetRTC() & ZIP_TIME_SEC_MASK;
    RTC_SetInfo(5, ZipTime);
  }
  else ZipTime = Bcd2Data(Bcd); //秒钟更新了

  //秒为0-9时，为防止通讯不上，此期间多次更新分钟及以上信息(触发模式只能从秒开始)
  if((!(Bcd & 0x70)) || IsUpdateAll){
    //分
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_MIN ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_MIN_SHIFT) & ZIP_TIME_MIN_MASK;
    //时
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_HOUR ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_HOUR_SHIFT) & ZIP_TIME_HOUR_MASK;
    //日
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_DATE ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_DATE_SHIFT) & ZIP_TIME_DATE_MASK;    
    //月
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_MONTH ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_MOUTH_SHIFT) & ZIP_TIME_MOUTH_MASK;  
    //年
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RTC | DS1302_RA_YEAR ,&Bcd, 1);
    ZipTime |= ((unsigned long)Bcd2Data(Bcd) << ZIP_TIME_YEAR_SHIFT) & ZIP_TIME_YEAR_MASK;      
  }
  else ZipTime |= (ZipTime_GetRTC() & ~ZIP_TIME_SEC_MASK);
  
  RTC_cbUpdateZipTime(ZipTime); //更新RTC             
}

//------------------设置RTC信息时，由位置到指令----------------------------
static const unsigned char _PosToCmd[] = {
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_YEAR,  //年
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_MONTH, //月
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_DATE,  //日
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_HOUR,  //时
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_MIN,   //分
  DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_SEC,   //秒  
};

//-------------------------------设置RTC信息-----------------------------
//Pos定义为:相对年0,月1,日2,时3,分4,秒5
void RTC_SetInfo(unsigned char Pos, unsigned char Data)
{
  //去除写保护
  unsigned char Bcd = 0;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CTR, &Bcd, 1);
  //写时间
  Bcd = Data2Bcd(Data);//转换为BCD码
  DS1302_Wr(_PosToCmd[Pos], &Bcd, 1);  
  //充电方式同时写入
  Bcd = DS1302_CTR_R2k | DS1302_CTR_DS2 | DS1302_CTR_TCS;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CHRGE_CTR, &Bcd, 1);
  //加上写保护
  Bcd = DS1302_CTR_WP;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CTR, &Bcd, 1);
}

/***********************************************************************
                          接口函数实现
************************************************************************/
//---------------------------初始化函数实现----------------------------
void RTC_Init0(signed char IsInited)
{
  DS1302_cbIOCfg();
 //配置芯片
  if(!IsInited){
    //初始化时间
    RTC_SetInfo(0, 20);
    RTC_SetInfo(1, 12);
    RTC_SetInfo(2, 31);    
    RTC_SetInfo(3, 23);
    RTC_SetInfo(4, 59);
    RTC_SetInfo(5, 56);   
  }
  _UpdateRTC(1);
  _UpdateRTC(1); //强制再更新一次
}

//--------------------------------任务函数实现-------------------------------
//建议256ms更新一次以更新实时时间
void RTC_Task(void)
{
  _UpdateRTC(0);
}

//------------------------------写备份数据------------------------------
//用于保存掉电不丢失的数据,地址+长度不应超过RTC容量,有对齐要求时需对齐
void RTC_WrBakData(unsigned char Adr,         //保存位置
                   const unsigned char *pData,//要保存的数据
                   unsigned char Len)          //数据长度
{
  //去除写保护
  unsigned char Bcd = 0;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CTR, &Bcd, 1);
  //写数据
  if(Adr == 0)//突发方式写一组
    DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RAM | DS1302_RA_BURST, pData, Len);
  else{//只能一个个写了
    Adr = DS1302_EN | DS1302_WR | DS1302_RAM | (Adr << 1);
    for(; Len > 0; Len--, Adr += 2, pData++)//读写标志在最低位
      DS1302_Wr(Adr, pData, 1);    
  }
  //加上写保护
  Bcd = DS1302_CTR_WP;
  DS1302_Wr(DS1302_EN | DS1302_WR | DS1302_RTC | DS1302_RA_CTR, &Bcd, 1);
}

//------------------------------读备份数据---------------------------------
//用于保存掉电不丢失的数据,地址+长度不应超过RTC容量,有对齐要求时需对齐
void RTC_RdBakData(unsigned char Adr,     //保存位置
                   unsigned char *pData,//要保存的数据
                   unsigned char Len)   //数据长度
{
  if(Adr == 0)//突发方式可读一组
    DS1302_Rd(DS1302_EN | DS1302_RD | DS1302_RAM | DS1302_RA_BURST, pData, Len);
  else{//只能一个个读了
    Adr = DS1302_EN | DS1302_RD | DS1302_RAM | (Adr << 1);
    for(; Len > 0; Len--, Adr += 2, pData++)//读写标志在最低位
      DS1302_Rd(Adr, pData, 1);    
  }
}


