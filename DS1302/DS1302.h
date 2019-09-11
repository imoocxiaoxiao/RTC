/*******************************************************************************

		               DS1302内部寄存器等定义

*******************************************************************************/
#ifndef __DS1302_H
#define __DS1302_H

//注：低位在前，高位在后
//--------------------------DS1302命令字节位定义------------------------
//DS1302命令字节位定义,该字节为发送的第一个数据
#define		DS1302_EN		      0x80	//为1时允许写DS1302
#define		DS1302_RTC		    0x00	//寄存器选择:为时钟日历
#define		DS1302_RAM		    0x40	//寄存器选择:RAM区
#define		DS1302_RDWR_FLAG	0x01	//读写标志
#define		DS1302_WR		      0x00	//写
#define		DS1302_RD		      0x01	//读
#define		DS1302_ADR_MASK		0x3e	//第1-6位为数据对应地址位置,见寄存器定义

//---------------------DS1302时钟,日历区地址及位定义----------------------
//RA = RTC_ADR,地址,即当命令字节为RTC时ADR_MASK位定义

//猝发模式控制位,此地址时从地址0,第0bit开始连续读写
#define		DS1302_RA_BURST		(31 << 1)
//秒 
#define		DS1302_RA_SEC			(0 << 1)
#define		DS1302_CLK_HALT		 0x80		//时钟保存标志,该位置1时,时间不走
#define		DS1302_SEC_BCD		0x7f		//秒BCD码	
//分钟	
#define		DS1302_RA_MIN			 (1 << 1)
#define		DS1302_MIN_BCD		 0xff		//分钟BCD码,无效高位为0
//小时
#define		DS1302_RA_HOUR			(2 << 1)	//小时,位定义为:
#define		DS1302_H12H24MOD		0x80		//指定小时模式,为1时为12小时模式
#define		DS1302_HOUR_BCD		0x7f		//小时BCD码
//天数	
#define		DS1302_RA_DATE			(3 << 1)
#define		DS1302_DATE_BCD		0xff		//天数BCD码,无效高位为0
//月份	
#define		DS1302_RA_MONTH		(4 << 1)
#define		DS1302_MONTH_BCD		0xff		//月份BCD码,无效高位为0
//星期
#define		DS1302_RA_DAY			(5 << 1)
#define		DS1302_DAY_BCD			0xff		//星期BCD码,无效高位为0
//年,范围为0-99
#define		DS1302_RA_YEAR			(6 << 1)
#define		DS1302_YEAR_BCD		0xff		//年BCD码,无效高位为0
//控制字
#define		DS1302_RA_CTR			(7 << 1)
#define		DS1302_CTR_WP			0x80		//1302写保护标志,其它位无效为0

//充电控制
#define		DS1302_RA_CHRGE_CTR	(8 << 1)
//数据位定义为:
#define		DS1302_CTR_RS			  0x03		//充电电阻选择，可定为:
#define		DS1302_CTR_RNO			0x00		//无电阻,即不可充电
#define		DS1302_CTR_R2k			0x01		//2k电阻
#define		DS1302_CTR_R4k			0x10		//4k电阻
#define		DS1302_CTR_R8k			0x11		//8k电阻
#define		DS1302_CTR_DS			  0x0C		//二极管选择位,接入一个0.7V,只可定义为:
#define		DS1302_CTR_DS1			0x04		//一个二极管限压
#define		DS1302_CTR_DS2			0x08		//两个二极管限压,其它选项不充电
#define		DS1302_CTR_TCS			0xa0		//充电控制只有高4位为A时才允许充电

//---------------------------------用户区地址及位定义-------------------------
//此段由用户区程序自定义
//UA = USER_ADR,地址,即当命令字节为RAM时ADR_MASK位定义



#endif // #ifndef DS1302_H



