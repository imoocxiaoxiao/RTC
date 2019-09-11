/************************************************************************

                   ʵʱʱ��ģ��-��STM32F4x�е�ʵ��(ֱ�Ӳ����Ĵ���,δ�������)
ʹ���ⲿ32768ʱ��
************************************************************************/

#include "RTC.h"
#include "stm32f4xx.h"
#include <string.h>

/************************************************************************
                          �ڲ�����
************************************************************************/
//-----------------------------д�������----------------------------
//������RTC��RTC_ISR[13:8], RTC_TAFCR, and RTC_BKPxR�⣬��������������
#define _UnLock() do{RTC->WPR = 0xCA; RTC->WPR = 0x53;}while(0)      //д����
#define _Lock() do{PWR->CR |= PWR_CR_DBP; RTC->WPR = 0xFF; }while(0)  //ȥд����

//-----------------------------��������ģʽ���----------------------------
//����0δ����
static signed char _EnterInitMode(void)//��������ģʽ
{
  RTC->ISR |= RTC_ISR_INIT; //�ý����־
  //�ȴ�����
  for(unsigned short Wait = 0xfff; Wait > 0; Wait--){
    if(RTC->ISR & RTC_ISR_INITF) return -1; 
  }
  return 0;
}
#define _ExitInitMode() do{RTC->ISR &= ~RTC_ISR_INIT;}while(0)//�˳�����ģʽ

//------------------------------�ȴ�RTCͬ��------------------------------
//�����Ƿ����
static signed char _WaitForSynchro(void)
{
  RTC->ISR  &= ~(RTC_ISR_RSF | RTC_ISR_INIT);
  //�ȴ�
  for(unsigned long Wait = 0x00020000; Wait > 0; Wait--){
    if(RTC->ISR & RTC_ISR_RSF) return -1; 
  }
  return 0;
}

//---------------------------------��ʱ��-------------------------------
static void _StartRTC(void)
{
   _UnLock(); //���� 
  _EnterInitMode();//��������ģʽ
  RTC->CR &= ~RTC_CR_FMT; //24Сʱ��
  RTC->PRER = 32767; //дԤ��Ƶ���������ú���ʱ��->�ٶ�Ϊ�ⲿ32768HZ
  _ExitInitMode(); //�˳�����ģʽ
  _Lock();   //����
}

//-------------------------------����RTCʱ��-----------------------------
//Pos: RTC_SetInfo���õ���Ϣ,0xff��Ĭ��ʱ��,����������ʱ��
static void _UpdateDateTime(unsigned char Pos,
                        unsigned long DateTime)//��д�������
{
  _UnLock();  //ȥ��д���� 
  
  //����ʱ��
  if(Pos < 3) RTC->DR = DateTime; //������ʱ
  else if(Pos < 6) RTC->TR = DateTime;//ʱ����ʱ
  else if(Pos == 0xff){//Ĭ��ʱ��
    RTC->DR = 0x00192101; //19��1��1������2
    RTC->TR = 0x00000000; //0�㿪ʼ   
  }
  //else ������Ĭ��
  _Lock(); //���ϱ���
  
  _StartRTC(); //���ú�ǿ����ʱ��
}

//------------------------------BCDת��-----------------------------
static unsigned char _Bcd2Data(unsigned char BCD)
{
  return (BCD >> 4) * 10 + BCD & 0x0f;
}
static unsigned char _Data2Bcd(unsigned char Data)
{
  return ((Data / 10) << 4) | (Data % 10);
}

/************************************************************************
                           ��غ���ʵ��
************************************************************************/

//---------------------------------��ʼ������----------------------------
void RTC_Init0(signed char IsInited)
{
  RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_AHB1ENR_BKPSRAMEN; //�ȿ�����Դ�뱸��SRAM 
  //=========================�ⲿ��������===========================
  PWR->CR |= PWR_CR_DBP;  //��ȥ��д����   CR_DBP_BB = 1; //
  RCC->BDCR |= RCC_BDCR_LSEON; //�����ⲿʱ��
  //����ⲿʱ��ʱ����
  for(unsigned long Wait = 0xfffff; Wait > 0; Wait--){
    if(RCC->BDCR & RCC_BDCR_LSERDY) break; 
  }
  //����ʱ��ԴΪ�ⲿLSE(0��1��2��3��)
  RCC->BDCR |= RCC_BDCR_RTCSEL_1;
  RCC->BDCR &= ~RCC_BDCR_RTCSEL_0;
  //ʹ��RTCʱ��
  RCC->BDCR |= RCC_BDCR_RTCEN;  
  _WaitForSynchro();//�ȴ�RTC�Ĵ���ͬ������д
  //=========================�ڲ���������===========================
  _StartRTC(); //ǿ����ʱ��

  if(!IsInited)//δ��ʼ��ʱ
    _UpdateDateTime(0xff, 0); //��Ĭ��ʱ��(ʱ��ΪĬ��)
  else _UpdateDateTime(6, 0); //����Ϊ��ʱ��(��������)
}

//--------------------------------������-------------------------------
//����256ms����һ���Ը���ʵʱʱ��
void RTC_Task(void)
{
  //һ���Զ���,Ҫ������Σ����Ե����ξͶ���
  unsigned long DR = RTC->DR;
  if(DR != RTC->DR) DR = RTC->DR;
  unsigned long TR = RTC->TR;
  if(TR != RTC->TR) TR = RTC->TR;
  
  //����RTC
  RTC_cbUpdate(_Bcd2Data(_Bcd2Data((DR & (RTC_DR_YU | RTC_DR_YT)) >> RTC_DR_YU_SHIFT)),
               _Bcd2Data(_Bcd2Data((DR & (RTC_DR_MU | RTC_DR_MT)) >> RTC_DR_MU_SHIFT)),
               _Bcd2Data(_Bcd2Data((DR & (RTC_DR_DU | RTC_DR_DT)) >> RTC_DR_DU_SHIFT)),               
               _Bcd2Data(_Bcd2Data((TR & (RTC_TR_HT | RTC_TR_HU)) >> RTC_TR_HU_SHIFT)),
               _Bcd2Data(_Bcd2Data((TR & (RTC_TR_MNT | RTC_TR_MNU)) >> RTC_TR_MNU_SHIFT)),
               _Bcd2Data(_Bcd2Data((TR & (RTC_TR_ST | RTC_TR_SU)) >> RTC_TR_SU_SHIFT)));              
}

//-------------------------------����RTC��Ϣ-----------------------------
//Pos����Ϊ:�����0,��1,��2,ʱ3,��4,��5
void RTC_SetInfo(unsigned char Pos,
                 unsigned char Data)
{
  //���д������
  unsigned long DateTime;
  if(Pos < 3){//������ʱ 
    DateTime = RTC->DR;
    if(Pos == 0){//��ʱ
      DateTime &= ~(RTC_DR_YU | RTC_DR_YT);
      DateTime |= _Data2Bcd(Data) << RTC_DR_YU_SHIFT;
    }
    else if(Pos == 1){//��ʱ
      DateTime &= ~(RTC_DR_MU | RTC_DR_MT);
      DateTime |= _Data2Bcd(Data) << RTC_DR_MU_SHIFT;
    }
    else{//��ʱ
      DateTime &= ~(RTC_DR_DU | RTC_DR_DT);
      DateTime |= _Data2Bcd(Data) << RTC_DR_DU_SHIFT;
    }    
  }
  else{//ʱ����ʱ
    DateTime = RTC->TR;
    if(Pos == 3){//ʱʱ
      DateTime &= ~(RTC_TR_HT | RTC_TR_HU);
      DateTime |= _Data2Bcd(Data) << RTC_TR_HU_SHIFT;
    }
    else if(Pos == 4){//��ʱ
      DateTime &= ~(RTC_TR_MNT | RTC_TR_MNU);
      DateTime |= _Data2Bcd(Data) << RTC_TR_MNU_SHIFT;
    }
    else{//��ʱ
      DateTime &= ~(RTC_TR_ST | RTC_TR_SU);
      DateTime |= _Data2Bcd(Data) << RTC_TR_SU_SHIFT;
    } 
  }
  _UpdateDateTime(Pos, DateTime); //����
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


