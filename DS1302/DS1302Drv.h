/*******************************************************************************

		               DS1302_ֱ��IO������ʱ��ʵ��ͷ�ļ�

*******************************************************************************/

#ifndef _DS1302_DRV_H
#define _DS1302_DRV_H

/***********************************************************************
                        ��غ���
***********************************************************************/

//--------------------------DS1302д���ݺ���---------------------
void DS1302_Wr(unsigned char Adr,//�������ڵ�ַ,ֻ����RTC/RAMѡ��λ+��ַλ
               const unsigned char *pData,//�跢�͵�����ָ��
               unsigned char Len);//�跢�͵����ݸ���

//--------------------------DS1302�����ݺ���---------------------
//���ض��յ�һ����
unsigned char DS1302_Rd(unsigned char Adr,//�������ڵ�ַ,ֻ����RTC/RAMѡ��λ+��ַλ
                        unsigned char *pData,     //���յ�����ָ��
                        unsigned char Len);       //����յ����ݸ���


/***********************************************************************
                        ��غ���
***********************************************************************/
//-------------------------ģ��ӿڶ���-----------------------------
#include "IOCtrl.h"

//��ʼ�����������ⲿ����
#define DS1302_cbIOCfg()       do{CfgRTC();}while(0)
//ʱ����:
#define		DS1302_cbSclkSet()	 do{SetRTC_SCLK();}while(0)
#define		DS1302_cbSclkClr()	 do{ClrRTC_SCLK();}while(0)

//��λ��:(����ʱ���߷���)
#define		DS1302_cbRstSet()    do{SetRTC_EN();}while(0)
#define		DS1302_cbRstClr()    do{ClrRTC_EN();}while(0)

//SIO��:
#define		DS1302_cbIoSet()		 do{SetRTC_IO();}while(0)
#define		DS1302_cbIoClr()		 do{ClrRTC_IO();}while(0)
#define		DS1302_cbIoSetIn()	 do{InRTC_IO();}while(0)
#define		DS1302_cbIoSetOut()  do{OutRTC_IO();}while(0)
#define		IsDS1302_cbIoHi()	   IsRTC_IO()
#define		IsDS1302_cbIoLow()	 (!IsRTC_IO())

//-----------------------�������---------------------------
//DS1302_��SPI����,��usΪ��λ,ע���ֵ��ϵͳcpuʱ���й�
#define		DS1302_cbRateUs()		  (4)	//ʱ������250k
#define		DS1302_cbCsRdyUs()		(4)	//Ƭѡ��׼��ʱ��
#define		DS1302_cbByteRdyUs()	(4)	//�����ֽ�֮��ļ��ʱ��
//��ʱ����
#include "Delay.h"
#define		Ds1302_cbDelayUs(Us)         do{DelayUs(Us);}while(0)

#define		Ds1302_cbEnterCritical()     do{}while(0)
#define		Ds1302_cbExitCritical()      do{}while(0)




#endif
