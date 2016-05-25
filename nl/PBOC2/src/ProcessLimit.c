/**
 * @file ProcessLimit.c
 * @brief ��������

   ����IC���е�Ӧ�����ն��е�Ӧ�õļ��ݳ̶�
   ��������ĵ����������ܾ�����
   �������ݰ���Ӧ����Ч���ڣ�Ӧ��ʧЧ����
   Ӧ�ð汾���Լ����������ж������������(Ӧ����;����AUC)
 * @version Version 1.0
 * @author Ҷ��ͳ
 * @date 2005-8-25 
 */
#include <string.h>
#include "posapi.h"
#include "AppData.h"
#include "tvrtsi.h"

#define DATE(yearh, yearl, month, day) \
	(int)(( (unsigned int) yearh << 24) | ( (unsigned int) yearl << 16) | \
	( (unsigned int) month << 8) | ( (unsigned int) day ))
/**
 * @fn ProcessLimit

 * @param int nIsATM ��ǰ�Ƿ���ATM�豸
 * @return FAIL
 			SUCC
 */
int ProcessLimit(int nIsATM)
{
	char * ptrData1, *ptrData2, *ptrData3, *ptrData4 ;
	struct postime pt ;
	char yearh ;
	int invalid = 1 ;
	//Ӧ�ð汾�ż��
	if ( (ptrData1 = GetAppData("\x9F\x08", NULL)) &&  	//IC ��Ӧ�ð汾��
		(ptrData2 = GetAppData("\x9F\x09", NULL) ) )	//�ն�Ӧ�ð汾��
	{
		if (memcmp(ptrData1, ptrData2, 2) !=0)
		{
			SetTVR(VERSION_NOT_MATCHED, 1) ;
		}
	}

	//Ӧ����;���Ƽ��
	if ( ptrData1 = GetAppData("\x9F\x07", NULL))		//Ӧ����;���ƴ���
	{
		if (ptrData2 = GetAppData("\x5F\x28", NULL))	//�����й��Ҵ������
		{
			ptrData3 = GetAppData("\x9F\x1A",NULL) ; 	//�ն˹��Ҵ���,һ������
			if (!ptrData3)
				return FAIL ;
			ptrData4 = GetAppData("\x9C", NULL) ; 		//�������ͣ�һ������
			if (!ptrData4)
				return FAIL ;	
			if (memcmp(ptrData2, ptrData3, 2) == 0) 	// ���ڽ���
			{
				switch(*ptrData4)
				{
				case 0x00:		//��Ʒ�ͷ���
					if ( *ptrData1 & 0x20 ) //��Ч
						invalid = 0 ;
					break ;
				case 0x01:		//�ֽ���
					if ( *ptrData1 & 0x80 ) //��Ч
						invalid = 0 ;
					break ;
				default :
					break ;
				}
			}
			else //���ʽ���
			{
				switch(*ptrData4)
				{
				case 0x00:		//��Ʒ�ͷ���
					if ( *ptrData1 & 0x10) //��Ч
						invalid = 0 ;
					break ;
				case 0x01:		//�ֽ���
					if ( *ptrData1 & 0x40 ) //��Ч
						invalid = 0 ;
					break ;
				default :
					break ;
				}
			}
		}
		else
		{
			invalid = 0;
		}
		
		if (nIsATM) //��ATM�豸
		{
			if ( !(*ptrData1 & 0x02) && !invalid ) //ATM ��Ч
				invalid = 1 ;
		}
		else //��ATM�豸
		{
			if ( !(*ptrData1 & 0x01) && !invalid) //��ATM ����ն���Ч
				invalid = 1 ;
		}
	}	
	else
	{
		invalid = 0;
	}
	
	if (invalid)
		SetTVR(SERV_REFUSE, 1) ;

	//Ӧ����Ч��/ʧЧ�ڼ��
	getpostime(&pt) ;
	if (ptrData1 = GetAppData("\x5F\x25", NULL) ) //���������Ӧ����Ч����
	{
		yearh = *ptrData1 > 0x50 ? 0x19 : 0x20 ;		
		if (DATE(pt.yearh, pt.yearl, pt.month, pt.day) <=
			DATE(yearh, *ptrData1, *(ptrData1 + 1), *(ptrData1 + 2)) )
		{
			SetTVR(APP_NOT_EFFECT, 1) ;
		}
	}

	ptrData1 = GetAppData("\x5F\x24", NULL) ;//��ȡӦ��ʧЧ����
	yearh = *ptrData1 > 0x50 ? 0x19 : 0x20 ;
	if (DATE(pt.yearh, pt.yearl, pt.month, pt.day) >
			DATE(yearh, *ptrData1, *(ptrData1 + 1), *(ptrData1 + 2)) )
		SetTVR(APP_EXPIRE, 1) ;

	return SUCC ;	
}
