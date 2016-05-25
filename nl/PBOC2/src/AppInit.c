/**
 * @file AppInit.c
 * @brief Ӧ�ó�ʼ��

	�Ե�ǰѡ����Ӧ�ý��г�ʼ��,ͨ����װPDOLָ��������,
	��IC������ȡ������Ӧ����,�����ݷ��ص������ж��Ƿ�
	�����ǰӦ�ö����ص�Ӧ��ѡ��,�����������Ķ�Ӧ������
 * @version Version 1.0
 * @author Ҷ��ͳ
 * @date 2005-8-25
 */

#include <string.h>
#include "posapi.h"
#include "posdef.h"
#include "ic.h"
#include "tvrtsi.h"
#include "DOLProcess.h"
#include "AppData.h"
#include "AppInit.h" 
#include "tlv.h"



/**
 * @fn int AnalyzeProcOp(char *, int)
 * @brief ����ȡ����ѡ��ķ���

 * @param char * resp ȡ����ѡ��ķ���
 * @param int	   ret	 resp�ĳ���
 * @return 
 */
int AnalyzeProcOp (char * resp, int ret)
{
	TTagRestriction tr[] = {
			{"\x82", 2, 2}, {"\x94", 0, 252}, {"\x77", 0, 0}} ;
	char value[256] ;
	int len ;
	
	//��Ӧ�����е����ݶ�����һ����ǩΪ'80'�Ļ������ݶ���
	if (*resp == 0x80)
	{
		if (*(resp + 1) == 0 || *(resp + 1) == 2) //����Ϊ0��Ϊ����
			return FAIL ;
		if (*(resp+1) % 4 != 2) //Ӧ�ý�������2���ֽڣ�AFL 4�ֽڱ߽�
			return FAIL ;
		SetAppData("\x82", resp + 2, 2) ; //AIP�ı�ǩΪ'83'
		SetAppData("\x94", resp + 4, *(resp + 1) - 2) ; //AFL ��ǩΪ'94'
		return SUCC ;
	}
	else if (*resp = 0x77)
	{
		TLV_Init(tr) ;
		if (TLV_Decode(resp, ret) != TLVERR_NONE)
			return FAIL ;
		
		memset(value, 0x00, sizeof(value)) ;
		if (TLV_GetValue("\x82", value, &len, 1) == TLVERR_TAGNOTFOUND)
			return FAIL ;
		SetAppData("\x82", value, len) ;
		if (TLV_GetValue("\x94", value, &len, 1) == TLVERR_TAGNOTFOUND)
			return FAIL ;
		SetAppData("\x94", value, len) ;
		return SUCC ;
	}
	else 
		return FAIL ;
}
/**
 * @fn int AppInit(void)

 * @return SUCC �ɹ�
 			 FAIL ʧ��
 */
int AppInit (void)
{
	char PDOL_Value[256] ;//APDU����������ܳ���255�ֽ�(LC����)
	char resp[256] ;//APDU����Ӧ,���Ϊ256�ֽ�,��(LE)����
	int ret ;
	char trace[4];

	InitTVR() ; //��ʼ��TVR
	InitTSI() ; //��ʼ��TSI

	memset (PDOL_Value, 0, 256) ;
	if (DOLProcess (PDOL, PDOL_Value) == FAIL)//��������Ԫ�б�ʧ��
		return FAIL ;

	memset(resp, 0x00, 256) ;
	//PDOL_Value��һ�ֽ�Ϊ�����ֽڵĳ���
	if( (ret = IC_GetProcessOptions(PDOL_Value + 1, PDOL_Value[0], resp)) < 0 )
	{	
		IC_GetLastSW (PDOL_Value, PDOL_Value+1) ;
		if (*PDOL_Value == 0x69 && *(PDOL_Value + 1) ==0x85)
		{
			//�迼�������ǰӦ�ã����ص�Ӧ��ѡ��
			return QUIT ;
		}
		return FAIL ; 	
	}
	
	//��������ѡ���������Ӧ
	if(AnalyzeProcOp( resp,  ret) == FAIL)
 		return FAIL ;

	getvar((char * )&ret, TSC_OFF, TSC_LEN) ;
	ret ++ ;
	if (ret > 99999999)
		ret = 0 ;

	memset(trace, 0, sizeof(trace));
	IntToC4((unsigned char *)trace, (unsigned int)ret) ;
	SetAppData("\x9F\x41", trace, 4) ;
	savevar((char * )&ret, TSC_OFF, TSC_LEN) ;
	return SUCC ;
}

