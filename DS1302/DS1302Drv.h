/*******************************************************************************

		               DS1302_直接IO口驱动时的实现头文件

*******************************************************************************/

#ifndef _DS1302_DRV_H
#define _DS1302_DRV_H

/***********************************************************************
                        相关函数
***********************************************************************/

//--------------------------DS1302写数据函数---------------------
void DS1302_Wr(unsigned char Adr,//数据所在地址,只包含RTC/RAM选择位+地址位
               const unsigned char *pData,//需发送的数据指针
               unsigned char Len);//需发送的数据个数

//--------------------------DS1302读数据函数---------------------
//返回多收的一个数
unsigned char DS1302_Rd(unsigned char Adr,//数据所在地址,只包含RTC/RAM选择位+地址位
                        unsigned char *pData,     //接收的数据指针
                        unsigned char Len);       //需接收的数据个数


/***********************************************************************
                        相关函数
***********************************************************************/
//-------------------------模拟接口定义-----------------------------
#include "IOCtrl.h"

//初始化函数需由外部调用
#define DS1302_cbIOCfg()       do{CfgRTC();}while(0)
//时钟线:
#define		DS1302_cbSclkSet()	 do{SetRTC_SCLK();}while(0)
#define		DS1302_cbSclkClr()	 do{ClrRTC_SCLK();}while(0)

//复位线:(兼置时钟线方向)
#define		DS1302_cbRstSet()    do{SetRTC_EN();}while(0)
#define		DS1302_cbRstClr()    do{ClrRTC_EN();}while(0)

//SIO线:
#define		DS1302_cbIoSet()		 do{SetRTC_IO();}while(0)
#define		DS1302_cbIoClr()		 do{ClrRTC_IO();}while(0)
#define		DS1302_cbIoSetIn()	 do{InRTC_IO();}while(0)
#define		DS1302_cbIoSetOut()  do{OutRTC_IO();}while(0)
#define		IsDS1302_cbIoHi()	   IsRTC_IO()
#define		IsDS1302_cbIoLow()	 (!IsRTC_IO())

//-----------------------相关配置---------------------------
//DS1302_的SPI速率,以us为单位,注意此值与系统cpu时钟有关
#define		DS1302_cbRateUs()		  (4)	//时间速率250k
#define		DS1302_cbCsRdyUs()		(4)	//片选后准备时间
#define		DS1302_cbByteRdyUs()	(4)	//两个字节之间的间隔时间
//延时程序
#include "Delay.h"
#define		Ds1302_cbDelayUs(Us)         do{DelayUs(Us);}while(0)

#define		Ds1302_cbEnterCritical()     do{}while(0)
#define		Ds1302_cbExitCritical()      do{}while(0)




#endif
