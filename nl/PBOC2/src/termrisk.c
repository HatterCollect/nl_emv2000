/**
* @file TermRisk.c
* @brief �ն˷��չ���

* @version Version 1.0
* @author Ҷ��ͳ
* @date 2005-08-05
*/

#include <string.h>
#include <stdlib.h>
#include "posapi.h"
#include "posdef.h"
#include "define.h"
#include "tvrtsi.h"
#include "ic.h"
#include "AppData.h"
#include "tools.h"
#include "TermRisk.h"

extern TTermParam g_termparam ;

void _PreTermRiskManage(TTermRisk * pTermRisk)
{
	pTermRisk->pPAN = GetAppData("\x5a", NULL);
	pTermRisk->pAIP = GetAppData("\x82", NULL);
	pTermRisk->Lower_COL= GetAppData("\x9f\x14", NULL);
	pTermRisk->Upper_COL= GetAppData("\x9f\x23", NULL);
	pTermRisk->MaxTargetPercent = g_termparam.MaxTargetPercent ;
	pTermRisk->TargetPercent = g_termparam.TargetPercent ;
	pTermRisk->FloorLimt = g_termparam.FloorLimit ;
	pTermRisk->ThresholdValue = g_termparam.ThresholdValue ;

}

//��ѯ�����Ƿ��ں�������
// SUCC ���ں�������  FAIL �ں�������
int	BlackListExcepFile(char *pPAN)
{
	int fp;
	int len;
	TBlackCard blackcard;
	
	//�򿪺������ļ�
	fp = fopen(BlackListFile, "r");
	if(fp < 0)
	{
		return SUCC;
	}

	while(1)
	{
		len = fread((char *)&blackcard, sizeof(TBlackCard), fp);
		if(len != sizeof(TBlackCard))
		{
			break;			
		}

		if(memcmp(pPAN, blackcard.cardno, blackcard.len)==0)
		{
			fclose(fp);
			SetTVR(CARD_IN_EXCEP_FILE, 1);
			return FAIL;
		}
	}
	
	fclose(fp);
	return SUCC;
}

/**
* @fn Floor_Limit
* @brief ����޶�
  
  ���㽻�׽�������ǰ�������ʹ�õģ����ж��Ƿ񳬹�
  �����޶������Ӧ��TVRλ
* @param (in) const char * FL_TERM �ն˽����޶�
* @param (in) int iAmount ��ǰ���׽���Ȩ��
* @return SUCC <����޶�
          FAIL >=����޶�
*/
int FloorLimit(const char * FL_TERM, int iAmount)
{
	int amount = 0;
	int termlimit ;
	
	C4ToInt( (unsigned int *)&termlimit, (unsigned char *)FL_TERM ) ;
//	amount = getlatestamount() ;
	if ( amount + iAmount >= termlimit )
	{
		SetTVR(TRADE_EXCEED_FLOOR_LIMIT, 1) ;
		return FAIL ;
	}
	return SUCC ;
}

/**
* @fn RandTransSelect
* @brief �������ѡ��
  
* @param (in) const TTermRisk *  pTermRisk ָ���ն˷��չ����õ������ݬ�������β���Ҫʹ�õ���
* @param (in) int iAmount ��ǰ���׽��
* @return void
*/
void RandTransSelect( const TTermRisk * pTermRisk, int iAmount )
{
	int termfl ;				// �ն�����޶� 
	int ThresValue ;			// ƫ���漴��ֵ
	float TransTargPercent ;	// ����Ŀ��ٷֱ� 
	float InterpolationFactor ; // ��ֵ���� 
	int RandomPercent ;			// ����ٷ��� 
	struct postime pt ;

	C4ToInt((unsigned int *)&termfl, (unsigned char *)pTermRisk->FloorLimt) ;
	C4ToInt((unsigned int *)&ThresValue, (unsigned char *)pTermRisk->ThresholdValue) ;

	getpostime(&pt) ;
	srand(pt.second + pt.minute * 60 + pt.hour * 60 * 60) ;
	RandomPercent = rand() % 99 + 1 ;

	if (iAmount < ThresValue) //���׽��С�����ƫ����ֵ
	{
		if (RandomPercent <= pTermRisk->TargetPercent) //����ٷ���С��Ŀ��ٷ���
		{
			SetTVR(SELECT_ONLILNE_RANDOM, 1) ;
		}
	}
	else if (iAmount < termfl) // ���׽��������ƫ����ֵ��С������޶�
	{
		InterpolationFactor = (float)(iAmount - ThresValue) /    //��ֵ����
			(float)(termfl - ThresValue) ;
		TransTargPercent = (float)(pTermRisk->MaxTargetPercent -  //����Ŀ��׷���
			pTermRisk->TargetPercent) * InterpolationFactor + 
			pTermRisk->TargetPercent ;
		if ( (float)RandomPercent <= TransTargPercent )
		{
			SetTVR(SELECT_ONLILNE_RANDOM, 1) ;
		}
	}
}

/**
* @fn GetATC
* @brief ��ȡӦ�ý�����ţ�������ǰ�ģ������������
 
* @param (out) char * atc ���ص�ǰATC 
* @param (out) char * onlineatc �����������ATC
* @return 0 ���߶�û��ȡ��
          1 ȡ��atc
		  2 ȡ��onlineatc
		  3 ͬʱȡ������
*/
int GetATC( char * atc, char * onlineatc )
{
	int ret ;
	char oData[256] ;
	int iExist = 0 ;
    
	/* ȡATC */
	memset(oData, 0, 10) ;
	ret = IC_GetData( GETDATAMODE_ATC, oData ) ;
	if ( ret > 0)
	{
		iExist |= 0x01 ;
		memcpy( atc, oData + 3, 2 ) ;
	}
    /* ȡLatest Online ATC */
	memset(oData, 0, 10) ;
	ret = IC_GetData( GETDATAMODE_ONLINEATC, oData ) ;
	if ( ret > 0)
	{
		iExist |= 0x02 ;
		memcpy( onlineatc, oData + 3, 2 ) ;
	}
	return iExist ;
}
/**
* @fn CheckVelocity
* @brief Ƶ�ȼ��
 
* @param (in) const TTermRisk *  pTermRisk ָ���ն˷��չ����õ�������
* @return SUCC �Ѽ��
          FAIL û�м��
*/
int CheckVelocity( const TTermRisk *  pTermRisk ) 
{
	int ret ;
	char atc[2] ; // Ӧ�ý������ 
	char lastonlineatc[2] ; // �������Ӧ�ý������ 
	int atctmp, loatctmp ;

	// �����������ѻ��������޻�����
	if ((pTermRisk->Lower_COL == NULL)||
		(pTermRisk->Upper_COL == NULL))
	{
		return FAIL ; //�����˽�
	}
	
	ret = GetATC(atc, lastonlineatc) ;
	if ( ret & 0x02 )
	{
		loatctmp = (((int)lastonlineatc[0]) << 8) + lastonlineatc[1] ;
		if ( loatctmp == 0 )		//�ϴ�����������żĴ���Ϊ0
		{
			SetTVR(NEW_CARD, 1) ;	// �����¿���־
		}
	}
	if ( ret != 3) // �޷���ICCͬʱȡ��atc �� lastonlineatc 
	{
		SetTVR(EXCEED_CON_OFFLINE_FLOOR_LIMIT, 1) ; //���������ѻ�����
		SetTVR(EXCEED_CON_OFFLINE_UPPER_LIMIT, 1) ; //���������ѻ�����
		SetTVR(ICC_DATA_LOST, 1) ; //  IC������ȱʧ 
	}
	else
	{
		atctmp = (((int)atc[0]) << 8) + atc[1] ;
		if (atctmp <= loatctmp)
		{
			SetTVR(EXCEED_CON_OFFLINE_FLOOR_LIMIT, 1) ;
			SetTVR(EXCEED_CON_OFFLINE_UPPER_LIMIT, 1) ;
		}
		if ( atctmp - loatctmp > *(pTermRisk->Lower_COL) ) //������������ѻ�����
		{
			SetTVR(EXCEED_CON_OFFLINE_FLOOR_LIMIT, 1) ;
		}
		if ( atctmp - loatctmp > *(pTermRisk->Upper_COL) ) //������������ѻ�����
		{
			SetTVR(EXCEED_CON_OFFLINE_UPPER_LIMIT, 1) ;
		}
	}
	return SUCC ;
}

/**
* @fn TermRisk_Manage
* @brief Ƶ�ȼ��
 
* @param (in) TTermRisk * pTermRisk  �ն˷��չ����õ�������
* @param (in) int iAmount ��ǰ���׽��(��Ȩ���)
* @return SUCC �Ѽ��
          FAIL û�м��
*/
int TermRiskManage(const TTermRisk * pTermRisk , int iAmount )
{
	int ret ;
	if ( *(pTermRisk->pAIP) & 0x08 )
	{
		BlackListExcepFile(pTermRisk->pPAN) ; //�ն˺��������쳣�ļ�����
		if (g_termparam.cAllowForceOnline)
		{
			clrscr() ;
			printf("�Ƿ�ǿ������?\n") ;
			printf("��(ȷ��). \n��(ȡ��).") ;
			while(1) 
			{
				ret = getkeycode(0) ;
				if (ret == ENTER || ret == ESC)
					break ;
			}
			if (ret == ENTER)
				SetTVR(MERCHANT_REQ_ONLINE, 1) ;
		}
		FloorLimit(pTermRisk->FloorLimt, iAmount) ; //����޶���
		RandTransSelect(pTermRisk, iAmount) ; //�漴����ѡ��
		CheckVelocity(pTermRisk) ; //Ƶ�ȼ��
		SetTSI(TERM_RISK_MANA_COMPLETION, 1) ;
	}
	return SUCC ;
}
