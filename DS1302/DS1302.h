/*******************************************************************************

		               DS1302�ڲ��Ĵ����ȶ���

*******************************************************************************/
#ifndef __DS1302_H
#define __DS1302_H

//ע����λ��ǰ����λ�ں�
//--------------------------DS1302�����ֽ�λ����------------------------
//DS1302�����ֽ�λ����,���ֽ�Ϊ���͵ĵ�һ������
#define		DS1302_EN		      0x80	//Ϊ1ʱ����дDS1302
#define		DS1302_RTC		    0x00	//�Ĵ���ѡ��:Ϊʱ������
#define		DS1302_RAM		    0x40	//�Ĵ���ѡ��:RAM��
#define		DS1302_RDWR_FLAG	0x01	//��д��־
#define		DS1302_WR		      0x00	//д
#define		DS1302_RD		      0x01	//��
#define		DS1302_ADR_MASK		0x3e	//��1-6λΪ���ݶ�Ӧ��ַλ��,���Ĵ�������

//---------------------DS1302ʱ��,��������ַ��λ����----------------------
//RA = RTC_ADR,��ַ,���������ֽ�ΪRTCʱADR_MASKλ����

//⧷�ģʽ����λ,�˵�ַʱ�ӵ�ַ0,��0bit��ʼ������д
#define		DS1302_RA_BURST		(31 << 1)
//�� 
#define		DS1302_RA_SEC			(0 << 1)
#define		DS1302_CLK_HALT		 0x80		//ʱ�ӱ����־,��λ��1ʱ,ʱ�䲻��
#define		DS1302_SEC_BCD		0x7f		//��BCD��	
//����	
#define		DS1302_RA_MIN			 (1 << 1)
#define		DS1302_MIN_BCD		 0xff		//����BCD��,��Ч��λΪ0
//Сʱ
#define		DS1302_RA_HOUR			(2 << 1)	//Сʱ,λ����Ϊ:
#define		DS1302_H12H24MOD		0x80		//ָ��Сʱģʽ,Ϊ1ʱΪ12Сʱģʽ
#define		DS1302_HOUR_BCD		0x7f		//СʱBCD��
//����	
#define		DS1302_RA_DATE			(3 << 1)
#define		DS1302_DATE_BCD		0xff		//����BCD��,��Ч��λΪ0
//�·�	
#define		DS1302_RA_MONTH		(4 << 1)
#define		DS1302_MONTH_BCD		0xff		//�·�BCD��,��Ч��λΪ0
//����
#define		DS1302_RA_DAY			(5 << 1)
#define		DS1302_DAY_BCD			0xff		//����BCD��,��Ч��λΪ0
//��,��ΧΪ0-99
#define		DS1302_RA_YEAR			(6 << 1)
#define		DS1302_YEAR_BCD		0xff		//��BCD��,��Ч��λΪ0
//������
#define		DS1302_RA_CTR			(7 << 1)
#define		DS1302_CTR_WP			0x80		//1302д������־,����λ��ЧΪ0

//������
#define		DS1302_RA_CHRGE_CTR	(8 << 1)
//����λ����Ϊ:
#define		DS1302_CTR_RS			  0x03		//������ѡ�񣬿ɶ�Ϊ:
#define		DS1302_CTR_RNO			0x00		//�޵���,�����ɳ��
#define		DS1302_CTR_R2k			0x01		//2k����
#define		DS1302_CTR_R4k			0x10		//4k����
#define		DS1302_CTR_R8k			0x11		//8k����
#define		DS1302_CTR_DS			  0x0C		//������ѡ��λ,����һ��0.7V,ֻ�ɶ���Ϊ:
#define		DS1302_CTR_DS1			0x04		//һ����������ѹ
#define		DS1302_CTR_DS2			0x08		//������������ѹ,����ѡ����
#define		DS1302_CTR_TCS			0xa0		//������ֻ�и�4λΪAʱ��������

//---------------------------------�û�����ַ��λ����-------------------------
//�˶����û��������Զ���
//UA = USER_ADR,��ַ,���������ֽ�ΪRAMʱADR_MASKλ����



#endif // #ifndef DS1302_H



