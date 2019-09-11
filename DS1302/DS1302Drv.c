/***********************************************************************
//
//					 DS1302_底层驱动--
//采用GPIO口模拟的方法实现与DS1302_的通讯连接
***********************************************************************/

#include "DS1302Drv.h"

//给DS1302_数据移位函数
static void SetDS1302_ShiftData(unsigned char ShiftData);		
//得到DS1302_数据移位函数	
static unsigned char GetDS1302_ShiftData(void);

//--------------------------DS1302写数据函数---------------------
void DS1302_Wr(unsigned char Adr,//数据所在地址,只包含RTC/RAM选择位+地址位
               const unsigned char *pData,//需发送的数据指针
               unsigned char Len)//需发送的数据个数
{
	Ds1302_cbEnterCritical();
	DS1302_cbSclkClr();
	Adr |= 0x80;//WR_EN; 		//填充地址项
	DS1302_cbRstSet();	//先拉低时钟,再高电平选中
	
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

//--------------------------DS1302读数据函数---------------------
//返回多收的一个数
unsigned char DS1302_Rd(unsigned char Adr,//数据所在地址,只包含RTC/RAM选择位+地址位
                        unsigned char *pData,     //接收的数据指针
                        unsigned char Len)       //需接收的数据个数
{

	Ds1302_cbEnterCritical();
	DS1302_cbSclkClr();
	Adr |= 0x81;//WR_EN | WR0_RD1; 		//填充地址项,为读状态
	DS1302_cbRstSet();	//高电平选中
	Ds1302_cbDelayUs(DS1302_cbCsRdyUs());
	SetDS1302_ShiftData(Adr);
	for(unsigned char i = 0; i < Len; i++)
	{
		Ds1302_cbDelayUs(DS1302_cbByteRdyUs());
		*(pData + i) = GetDS1302_ShiftData();
	}
  //多收一个数时，数据才能对
	unsigned char uData;
	uData = GetDS1302_ShiftData();
	DS1302_cbRstClr();
	Ds1302_cbDelayUs(DS1302_cbCsRdyUs() >> 1);
	Ds1302_cbExitCritical();
 	return uData;
}

//-----------------------------------给DS1302_数据移位函数---------------------------------
//将数据移入DS1302_
//此函数只负责控制SCLK与SIO
//调用此函数前后SCLK为高电平,SIO不确定
//形参：需移数据的数
static void SetDS1302_ShiftData(unsigned char ShiftData)
{
	unsigned char i;

	DS1302_cbIoSetOut();								//置为输出状态
	for(i = 0;i < 8; i++)
	{
		DS1302_cbSclkClr();
		if (ShiftData & 0x01)						//得到数据,下降沿主机LSB移出
			DS1302_cbIoSet();
		else DS1302_cbIoClr();
		Ds1302_cbDelayUs((DS1302_cbRateUs() >> 1) - 1);
		DS1302_cbSclkSet();							//上升沿DS1302_锁存
		Ds1302_cbDelayUs(DS1302_cbRateUs() >> 1);	
		ShiftData = ShiftData >> 1;			//LSB（最后第7次移位无效）
	}
}	

//-----------------------------------得到DS1302_数据移位函数---------------------------------
//从DS1302_得到数据
//此函数只负责数据移控制SCLK与SIO
//调用此函数前后SCLK为高电平,SIO不确定
//输出：得到的8bit数据
static unsigned char GetDS1302_ShiftData(void)
{
	unsigned char i,ShiftData;

	DS1302_cbIoSetIn();
	ShiftData = 0;
	for(i = 0;i < 8; i++)
	{
		ShiftData = ShiftData >> 1;
		DS1302_cbSclkClr();								//下降沿得到数据
		Ds1302_cbDelayUs(DS1302_cbRateUs() >> 1);	
		if(IsDS1302_cbIoHi()) ShiftData |= 0x80;	//LSB数据
		else ShiftData &= ~0x80;					//此句无意义但可让高低数据时时间相同
		DS1302_cbSclkSet();							//上升沿锁存(下降沿DS1302_数据移入)
		Ds1302_cbDelayUs((DS1302_cbRateUs() >> 1) - 1);	
	}
	return ShiftData;
}

