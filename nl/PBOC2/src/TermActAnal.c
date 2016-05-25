/**
* @file TermActAnal.c
* @brief �ն���Ϊ����

* @version Version 1.0
* @author Ҷ��ͳ
* @date 2005-08-05
*/
#include "posapi.h"
#include "posdef.h"
#include "tvrtsi.h"
#include "ic.h"
#include "tlv.h"
#include "dolprocess.h"
#include "AppData.h"
#include <string.h>

extern TTermParam g_termparam ;
extern void CalcMsgDigest(char *message, int len, char *digest) ;

void _PreTermActAna(TTermActAna * pTermActAna)
{
	char * pData ;
	char buff[256] ;
	char hash[256] ;
	
	pData = GetAppData("\x9f\x0d", NULL) ;
	if (pData == NULL)
	{
		SetAppData("\x9f\x0d", "\xFF\xFF\xFF\xFF\xFF", 5) ;
		pData = GetAppData("\x9f\x0d", NULL) ;
	}
	pTermActAna->IAC_Default = pData ;

	pData = GetAppData("\x9f\x0e", NULL) ;
	if (pData == NULL)
	{
		SetAppData("\x9f\x0e", "\x00\x00\x00\x00\x00", 5) ;
		pData = GetAppData("\x9f\x0e", NULL) ;
	}
	pTermActAna->IAC_Decline = pData ;

	pData = GetAppData("\x9f\x0f", NULL) ;
	if (pData == NULL)
	{
		SetAppData("\x9f\x0f", "\xFF\xFF\xFF\xFF\xFF", 5) ;
		pData = GetAppData("\x9f\x0f", NULL) ;
	}
	pTermActAna->IAC_Online = pData ;

	pTermActAna->TAC_Decline = g_termparam.TAC_Decline ;
	pTermActAna->TAC_Default = g_termparam.TAC_Default;
	pTermActAna->TAC_Online = g_termparam.TAC_Online ;

	if (pData = GetAppData("\x9F\x35", NULL))
		pTermActAna->nCanOnline = (((pData[0] & 0x0F) == 0x03 ||
				(pData[0] & 0x0F) == 0x06) ? 0 : 1) ; 
	else
		pTermActAna->nCanOnline = 1 ;
	pTermActAna->nNeedCDA = 0 ;

	memset(buff, 0, sizeof(buff));
	memset(hash, 0, sizeof(hash));
	pData = GetAppData("\x97", NULL) ;
	if (pData == NULL)
	{
		SetAppData("\x97", g_termparam.DefaultTDOL, g_termparam.DefaultTDOLen) ;
	}
	DOLProcess(TDOL, buff) ;
	if (!pData)
	{
		DelAppData("\x97") ;
	}
	CalcMsgDigest(buff + 1, buff[0], hash ) ;
	SetAppData("\x98", hash, 20) ;

}
/**
* @fn CheckTVR
* @brief ���TVR
  
  ��TVR�뷢������Ϊ���뼰�ն���Ϊ������бȽϣ���ȷ������������Ϊ
* @param (in) const TTermActAna * pTermActAna
* @param (out) char * cAuthResCode ��Ȩ��Ӧ����
* @return @li ACTYPE_ACC	�ն˽���Ƭ����ACC
		  @li ACTYPE_ARQC	�ն˽���Ƭ����ARQC
		  @li ACTYPE_TC		�ն˽���Ƭ����TC
*/
int CheckTVR( const TTermActAna * pTermActAna, char *cAuthResCode )
{
	char cTVR[5] ;
	int i ;

	GetTVR(cTVR) ; //���TVR���������±Ƚ�
	for (i = 0; i < 5; i++) //�ܾ�
	{
		if( (pTermActAna->TAC_Decline[i] & cTVR[i]) ||
			(pTermActAna->IAC_Decline[i] & cTVR[i]) )
		{
			memcpy(cAuthResCode, "\x5A\x31", 2) ; //�ѻ��ܾ�
			return ACTYPE_AAC ; //����AAC
		}
	}

	memcpy(cAuthResCode, "\x59\x31",2) ; //�ѻ���׼
	if (pTermActAna->nCanOnline) //�ն˿�������
	{
		for (i = 0; i < 5; i++)
		{
			if( (pTermActAna->TAC_Online[i] & cTVR[i]) || 
				(pTermActAna->IAC_Online[i] & cTVR[i]) )
			{
				memcpy(cAuthResCode, "\x00\x00", 2) ; //����
				return ACTYPE_ARQC ; //����AAC
			}
		}
	}
	else //ȱʡ
	{
		for (i = 0; i < 5; i++)
		{
			if( (pTermActAna->TAC_Default[i] & cTVR[i]) ||
				(pTermActAna->IAC_Default[i] & cTVR[i]) )
			{
				memcpy(cAuthResCode, "\x5A\x33", 2) ; //�޷��������ѻ����ܾ�
				return ACTYPE_AAC ; //����AAC
			}
		}
		memcpy(cAuthResCode, "\x59\x33",2) ; //�޷��������ѻ���׼
	}
	return ACTYPE_TC ;
}

/**
* @fn AnalyzeACData
* @brief ����Generate AC����صĴ���
  
  ����ȷ����������µıر��������ڣ�����ʧ��
* @param (in) char * response ���Generate AC����Ӧ���룬������SW1SW2 
* @param (in) int len response�ĳ���
* @param (in) int NeedCDA �Ƿ���CDA��������Ҫ9f4b�����
* @return SUCC �����ɹ�
		  FAIL ʧ�ܣ�����������
*/
int AnalyzeACData(char * response, int len, int NeedCDA)
{
	char buff[255] ;
	int len0, ret ;
	TTagRestriction tr[7] = {   //��Ҫ��ȡ��TLV
		{"\x9f\x27", 1, 1}, {"\x9f\x36", 2, 2}, {"\x9f\x26", 8, 8},
		{"\x9f\x4b", 1,255},{"\x9f\x10",1, 32}, {"\x80",11,43} }; 
	
	memset(buff, 0, 255) ;
	TLV_Init(tr); //��ʼ��TLV����ģ�� 	
	if( TLV_Decode(response, len) != TLVERR_NONE )//����TLV��ʧ���򷵻�
		return FAIL ;	
	if (*response == 0x80 )//�����80��ǩ
	{
		ret = TLV_GetValue("\x80", buff, &len0, 1) ;
		if(len0 == 0) //TLV��ʽ���󣬱ر��򣬳��Ȳ���Ϊ0 
			return FAIL ;
		if( ret == TLVERR_NONE )
		{
			SetAppData("\x9f\x27", buff, 1) ;
			SetAppData("\x9f\x36", buff + 1, 2) ;
			SetAppData("\x9f\x26", buff + 3, 8) ;
			if (len0 > 11) 
				SetAppData("\x9f\x10", buff + 11, len0 - 11) ;
			return SUCC ;
		}
		else
			return FAIL ;
	}
	else if (*response == 0x77 )//77��ǩ
	{	
		// �ر���������
		if (TLV_GetValue("\x9f\x27", buff, &len0,1)  != TLVERR_NONE)
			return FAIL ;
		SetAppData("\x9f\x27", buff, len0) ;
		
		if (TLV_GetValue("\x9f\x36", buff, &len0,1)  != TLVERR_NONE)
			return FAIL ;
		SetAppData("\x9f\x36", buff, len0) ;	
		if (NeedCDA)
		{
			if (TLV_GetValue("\x9f\x4B", buff, &len0,1)  != TLVERR_NONE)
				return FAIL ;
			SetAppData("\x9f\x48", buff, len0) ;
		}
		else
		{
			if (TLV_GetValue("\x9f\x26", buff, &len0,1)  != TLVERR_NONE)
				return FAIL ;
			SetAppData("\x9f\x26", buff, len0) ;
		}
		//��ѡ������
		if (TLV_GetValue("\x9f\x10", buff, &len0,1)  == TLVERR_NONE)
			SetAppData("\x9f\x10", buff, len0) ;
		return SUCC ;
	}
	else //��ǩ����
		return FAIL ;
}

/**
* @fn TermAct_Analyze
* @brief ִ���ն���Ϊ����
  
  ��ִ�й��̣�ͬʱ���п�Ƭ�ڵķ��չ���/��Ϊ����������������ظ��ն�
* @param (in) const TTermActAna * pTermActAna �ն���Ϊ�����õ�������
* @return AAC 	�ն���Ϊ�����Ϳ�Ƭ��Ϊ������Ľ��׽��
		  TC	
		  ARQC		
		  FAIL ʧ��
*/
int TermActAnalyze( const TTermActAna * pTermActAna)
{
	int ret ;
	TACType iACType ;
	char response[256] ;
	char cdolvalue[256] ;
	char cAuthResCode[2] = { 0x00, 0x00 } ;
	char *cid ;
	
	iACType = (TACType)CheckTVR(pTermActAna, cAuthResCode) ;
	SetAppData("\x8A",cAuthResCode, 2) ;


	memset(cdolvalue, 0x00, sizeof(cdolvalue)) ;
	if (DOLProcess(CDOL1, cdolvalue) == FAIL)
		return FAIL ;

	memset(response, 0, sizeof(response)) ;
	ret = IC_GenerateAC(iACType, pTermActAna->nNeedCDA, 
		cdolvalue + 1,  cdolvalue[0], response) ;

	if( ret < 0 ) //��IC����
		return FAIL ;

	if( AnalyzeACData(response, ret, pTermActAna->nNeedCDA) == FAIL )
		return FAIL ;

	SetTSI(CARD_RISK_MANA_COMPLETION, 1) ;
	
	cid = GetAppData("\x9f\x27", NULL) ;
	if(cid == NULL)
		return FAIL ;

	if (((*cid & 0xC0) == 0x40) && (iACType < ACTYPE_TC))
		return FAIL ;
	if ((*cid & 0xC0) && (iACType == ACTYPE_AAC))
		return FAIL ;
	
	return SUCC;
}
