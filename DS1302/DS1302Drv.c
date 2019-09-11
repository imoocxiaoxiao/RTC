/***********************************************************************
//
//					 DS1302_�ײ�����--
//����GPIO��ģ��ķ���ʵ����DS1302_��ͨѶ����
***********************************************************************/

#include "DS1302Drv.h"

//��DS1302_������λ����
static void SetDS1302_ShiftData(unsigned char ShiftData);		
//�õ�DS1302_������λ����	
static unsigned char GetDS1302_ShiftData(void);

//--------------------------DS1302д���ݺ���---------------------
void DS1302_Wr(unsigned char Adr,//�������ڵ�ַ,ֻ����RTC/RAMѡ��λ+��ַλ
               const unsigned char *pData,//�跢�͵�����ָ��
               unsigned char Len)//�跢�͵����ݸ���
{
	Ds1302_cbEnterCritical();
	DS1302_cbSclkClr();
	Adr |= 0x80;//WR_EN; 		//����ַ��
	DS1302_cbRstSet();	//������ʱ��,�ٸߵ�ƽѡ��
	
	SetDS1302_ShiftData(Adr);
	for(unsigned char i = 0; i < Len; i++)
	{
		Ds1302_cbDelayUs(DS1302_cbByteRdyUs());
		SetDS1302_ShiftData(*(pData + i));
	}
	DS1302_cbRstClr();
	Ds1302_cbExitCritical();
	Ds1302_cbDelayUs(DS1302_cbCsRdyUs() >> 1);
}

//--------------------------DS1302�����ݺ���---------------------
//���ض��յ�һ����
unsigned char DS1302_Rd(unsigned char Adr,//�������ڵ�ַ,ֻ����RTC/RAMѡ��λ+��ַλ
                        unsigned char *pData,     //���յ�����ָ��
                        unsigned char Len)       //����յ����ݸ���
{

	Ds1302_cbEnterCritical();
	DS1302_cbSclkClr();
	Adr |= 0x81;//WR_EN | WR0_RD1; 		//����ַ��,Ϊ��״̬
	DS1302_cbRstSet();	//�ߵ�ƽѡ��
	Ds1302_cbDelayUs(DS1302_cbCsRdyUs());
	SetDS1302_ShiftData(Adr);
	for(unsigned char i = 0; i < Len; i++)
	{
		Ds1302_cbDelayUs(DS1302_cbByteRdyUs());
		*(pData + i) = GetDS1302_ShiftData();
	}
  //����һ����ʱ�����ݲ��ܶ�
	unsigned char uData;
	uData = GetDS1302_ShiftData();
	DS1302_cbRstClr();
	Ds1302_cbDelayUs(DS1302_cbCsRdyUs() >> 1);
	Ds1302_cbExitCritical();
 	return uData;
}

//-----------------------------------��DS1302_������λ����---------------------------------
//����������DS1302_
//�˺���ֻ�������SCLK��SIO
//���ô˺���ǰ��SCLKΪ�ߵ�ƽ,SIO��ȷ��
//�βΣ��������ݵ���
static void SetDS1302_ShiftData(unsigned char ShiftData)
{
	unsigned char i;

	DS1302_cbIoSetOut();								//��Ϊ���״̬
	for(i = 0;i < 8; i++)
	{
		DS1302_cbSclkClr();
		if (ShiftData & 0x01)						//�õ�����,�½�������LSB�Ƴ�
			DS1302_cbIoSet();
		else DS1302_cbIoClr();
		Ds1302_cbDelayUs((DS1302_cbRateUs() >> 1) - 1);
		DS1302_cbSclkSet();							//������DS1302_����
		Ds1302_cbDelayUs(DS1302_cbRateUs() >> 1);	
		ShiftData = ShiftData >> 1;			//LSB������7����λ��Ч��
	}
}	

//-----------------------------------�õ�DS1302_������λ����---------------------------------
//��DS1302_�õ�����
//�˺���ֻ���������ƿ���SCLK��SIO
//���ô˺���ǰ��SCLKΪ�ߵ�ƽ,SIO��ȷ��
//������õ���8bit����
static unsigned char GetDS1302_ShiftData(void)
{
	unsigned char i,ShiftData;

	DS1302_cbIoSetIn();
	ShiftData = 0;
	for(i = 0;i < 8; i++)
	{
		ShiftData = ShiftData >> 1;
		DS1302_cbSclkClr();								//�½��صõ�����
		Ds1302_cbDelayUs(DS1302_cbRateUs() >> 1);	
		if(IsDS1302_cbIoHi()) ShiftData |= 0x80;	//LSB����
		else ShiftData &= ~0x80;					//�˾������嵫���øߵ�����ʱʱ����ͬ
		DS1302_cbSclkSet();							//����������(�½���DS1302_��������)
		Ds1302_cbDelayUs((DS1302_cbRateUs() >> 1) - 1);	
	}
	return ShiftData;
}

