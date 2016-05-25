#include	<string.h>
#include	<stdlib.h>

#include "../8200API/posapi.h"
#include "../8200API/define.h"

#include	"../inc/pboc_pack.h"
#include "../inc/pboc_comm.h"

static PBOC_TLV xTLVList[TOTAL_TAG_NUMBER];///<���ڴ�������TLV �б� 

//static char xDataBuf[PACK_BUF_LEN];///<���ڴ����������ݻ�����

//BITMAP��TAG�Ĺ�ϵӳ��,��������TAG��BITMAP�е�λ��
static const char TAG_MAP[TOTAL_TAG_NUMBER] = 
{
	TAG_TRANS_TYPE			,	//Transaction Type
	TAG_TRANS_COUNTER		,	//Application Transaction Counter
	TAG_ARQC				,	//ARQC
	TAG_CRYPT_INFO_DATA	,	//Cryption Information Data
	TAG_AMOUNT_AUTH		,	//Amount Authorized
	TAG_AMOUNT_OTHER		,	//Amount Other
	TAG_ENCIPHERED_PIN		,	//Enciphered PIN
	TAG_AUTH_RESP_CODE	,	//Authorization Response Code
	TAG_ISS_AUTH_DATA		,	//Issuer Authorized Data
	TAG_ISS_SCRIPT			,	//Issuer Script
	TAG_ISS_RESULT			,	//Issuer Script Results
	TAG_TERM_VERIFI_RESULT	,	//Terminal Verification Result
	TAG_TRANS_STATUS_INFO	,	//Transaction Status Information
	TAG_APP_PAN				,	//Application PAN
	TAG_TRANS_DATE			,	//Transaction Date
	TAG_TRANS_TIME			,	//Transaction Time
	TAG_CAPK_UPDATE		,	//CAPK Update Data
	TAG_TRANS_RESULT		,	//transaction result
	TAG_POS_ENTRY_MODE	,	//pos entry mode
	TAG_ISS_DATA			,// 9f10 
	TAG_TRANS_FORCEACCEPT	,//����ǿ�Ƚ��ܱ�־ 4FAA
};

//����������BITMAP�Ĺ�ϵӳ�䣬���������ض����׵�BITMAP
static const PBOC_BITMAP xBitmapList[TOTAL_TRANS_NUMBER] =
{
	{TRANS_AUTH,		"111111100000010000011"},	//Authorization
	{TRANS_BATCH,		"110111010011111101011"},	//Batch Data Capture
	{TRANS_REVERSAL,	"110010010011111100011"},	//Reversal
	{TRANS_UPDATE_CAK,	"100000000000000000000"},	//Update CA key
	{TRANS_ENTRY_MODE,	"100010100000011100100"},	//Pos Entry Mode
};

/**
 *	@sn	int AssociateTLVList(PBOC_TRANS_DATA * pxData)
 *	@brief ����TAG_MAP�ж����TAGλ�ð�PBOC_TRANS_DATA�ṹ�е����ݹ�����xTLVList
 *	@param	pxData :��������
 *	@return @li < 0 ����
 *		@li = 0 �ɹ�
 */
static int AssociateTLVList(PBOC_TRANS_DATA * pxData)
{
	int i = 0;
	
	memset((char *)&xTLVList[0], 0, sizeof(xTLVList));
	for(i = 0; i < TOTAL_TAG_NUMBER; i++)
	{
		switch(TAG_MAP[i])
		{
			case TAG_TRANS_TYPE			:	//Transaction Type	
				memcpy(xTLVList[i].cLabel, "\x9C", 1);
				xTLVList[i].pxValue = &(pxData->TransType);
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = 1;
				break;
				
			case TAG_TRANS_COUNTER		:	//Application Transaction Counter
				memcpy(xTLVList[i].cLabel, "\x9f\x36", 2);
				xTLVList[i].pxValue = pxData->ATC;
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = 2;
				break;
				
			case TAG_ARQC				:	//ARQC
				memcpy(xTLVList[i].cLabel, "\x9f\x26", 2);
				xTLVList[i].pxValue = pxData->ARQC;
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = 8;
				break;
				
			case TAG_CRYPT_INFO_DATA	:	//Cryption Information Data
				memcpy(xTLVList[i].cLabel, "\x9f\x27", 2);
				xTLVList[i].pxValue = &(pxData->CID);
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = 1;
				break;
				
			case TAG_AMOUNT_AUTH		:	//Amount Authorized
				memcpy(xTLVList[i].cLabel, "\x9f\x02", 2);
				xTLVList[i].pxValue = pxData->gAmount;
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = 6;
				break;
				
			case TAG_AMOUNT_OTHER		:	//Amount Other
				memcpy(xTLVList[i].cLabel, "\x9f\x03", 2);
				xTLVList[i].pxValue = pxData->gAmount_O;
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = 6;
				 break;
				 
			case TAG_ENCIPHERED_PIN		:	//Enciphered PIN
				memcpy(xTLVList[i].cLabel, "\x9f\x50", 2);
				xTLVList[i].pxValue = pxData->E_PIN;
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = 12;
				break;
				
			case TAG_AUTH_RESP_CODE	:	//Authorization Response Code						
				memcpy(xTLVList[i].cLabel, "\x8a", 1);
				xTLVList[i].pxValue = pxData->Auth_Res_Code;
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = 2;
				break;
				
			case TAG_ISS_AUTH_DATA		:	//Issuer Authorized Data
				memcpy(xTLVList[i].cLabel, "\x91", 1);
				xTLVList[i].pxValue = pxData->IAD;
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = pxData->IADLen;
				break;
				
			case TAG_ISS_SCRIPT			:	//Issuer Script	
				memcpy(xTLVList[i].cLabel, "\x72", 1);
				xTLVList[i].pxValue = pxData->IS;
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = pxData->ISLen;
				break;

			case TAG_ISS_RESULT			:	//Issuer Script Results
				memcpy(xTLVList[i].cLabel, "\x9f\x51", 2);
				xTLVList[i].pxValue = pxData->ISR;
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = pxData->ISRLen;
				break;
				
			case TAG_TERM_VERIFI_RESULT	:	//Terminal Verification Result
				memcpy(xTLVList[i].cLabel, "\x95", 1);
				xTLVList[i].pxValue = pxData->TVR;
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = 5;
				break;
				
			case TAG_TRANS_STATUS_INFO	:	//Transaction Status Information
				memcpy(xTLVList[i].cLabel, "\x9b", 1);
				xTLVList[i].pxValue = pxData->TSI;
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = 2;
				break;

			case TAG_APP_PAN			:	//Application PAN
				memcpy(xTLVList[i].cLabel, "\x5A", 1);
				xTLVList[i].pxValue = pxData->Pan;
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = pxData->PanLen;
				break;
				
			case TAG_TRANS_DATE		:	//Transaction Date
				memcpy(xTLVList[i].cLabel, "\x9A", 1);
				xTLVList[i].pxValue = pxData->POS_date;
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = 3;
				break;
				
			case TAG_TRANS_TIME			:	//Transaction Time
				memcpy(xTLVList[i].cLabel, "\x9F\x21", 2);
				xTLVList[i].pxValue = pxData->POS_time;
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = 3;
				break;

			case TAG_CAPK_UPDATE		:	//CAPK Update Data
				memcpy(xTLVList[i].cLabel, "\xDC", 1);
				xTLVList[i].pxValue = pxData->UpdateKey;
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = pxData->UpdateKeyLen;
				break;
				
			case TAG_TRANS_RESULT		:	//transaction result
				memcpy(xTLVList[i].cLabel, "\xDE", 1);
				xTLVList[i].pxValue = &(pxData->TransResult);
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = 1;
				break;
				
			case TAG_POS_ENTRY_MODE	:	//pos entry mode
				memcpy(xTLVList[i].cLabel, "\xEE", 1);
				xTLVList[i].pxValue = &(pxData->POS_Enter_Mode);
				xTLVList[i].iLabelLen = 1;
				xTLVList[i].iValueLen = 1;
				break;
			case TAG_ISS_DATA:
				memcpy(xTLVList[i].cLabel, "\x9F\x10", 2);
				xTLVList[i].pxValue = pxData->ISS_APP_DATA;
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = pxData->IssAppDataLen ;
				break;
			case TAG_TRANS_FORCEACCEPT:
				memcpy(xTLVList[i].cLabel, "\x4F\xAA", 2);
				xTLVList[i].pxValue = &(pxData->iAcceptForced);
				xTLVList[i].iLabelLen = 2;
				xTLVList[i].iValueLen = 1 ;
				break;			
			default:
				return PACKTLV_ERR_TAGMAP;
		}
	}

	return 0;
}

/**
 *	@sn	int PackTLV(PBOC_TLV *pxInTLV, char * pxOutTLV)
 *	@brief	��PBOC_TLV�ṹ���ݴ����TLV
 *	@param	pxInTLV :PBOC_TLV�ṹ����ָ��
 *	@param	pxOutTLV:���TLV�ֽڴ�
 *	@return	@li <= 0����
 *		@li  > 0 TLV������
 */
static int PackTLV(PBOC_TLV *pxInTLV, char * pxOutTLV)
{
	int iTmp = 0, iTLVLength = 0, i =0;
	char cTmp[10];
	
	iTLVLength = 0;

	//get Tag label
	memcpy(pxOutTLV, pxInTLV->cLabel, pxInTLV->iLabelLen);
	iTLVLength += pxInTLV->iLabelLen;

	//add value length
	iTmp = pxInTLV->iValueLen;
	memset(cTmp, 0, sizeof(cTmp));
	i = 0;///<���㳤��ֵ��ռ�ֽ���
	while((iTmp & 0xFF) > 0)
	{
		cTmp[i] = iTmp & 0xFF;
		iTmp = iTmp >> 8;
		i++;
	}
	iTmp = pxInTLV->iValueLen;
	if((i > 3) || (iTmp < 0))
	{
		return PACKTLV_ERR_LENGTH;
	}
	if(iTmp <= 127)
	{
		//1-127��������ĸ�ʽ�ǣ�����ֽڵ�b8λΪ0��b7��b1λ��ʾֵ�򳤶�
		pxOutTLV[iTLVLength] = iTmp & 0xFF;
		iTLVLength += 1;
	}
	else
	{
		//����127��������ĸ�ʽ�ǣ�����ֽڵ�b8λΪ1��b7��b1λ��ʾ����������ֽڵĺ����ֽڵ��ֽ�����
		//����ֽڵĺ����ֽ�Ϊֵ��ĳ���
		pxOutTLV[iTLVLength] =	i | 0x80;
		iTLVLength += 1;
		while(i > 0)
		{
			pxOutTLV[iTLVLength] = cTmp[i - 1];
			iTLVLength += 1;
			i--;
		}
	}

	//add value
	memcpy(&pxOutTLV[iTLVLength], pxInTLV->pxValue, pxInTLV->iValueLen);
	iTLVLength += pxInTLV->iValueLen;
	
	return iTLVLength;
}

/**
 *	@sn	int PBOC_PACK(char cTransType, PBOC_TRANS_DATA * pxData, char * pxOutData)
 *	@brief	����xBitmapList��BITMAP���壬�ѽ������ݴ��
 *	@param	cTransType : ��������
 *	@param	pxData : ��������
 *	@param	pxOutData : �����Ľ��
 *	@return	@li	<= 0 ����
 *		@li > 0 �ɹ�
 */
static int PBOC_PACK(char cTransType, PBOC_TRANS_DATA * pxData, char * pxOutData)
{
	int i = 0, iDataLen = 0, iRet = 0;
	char bitmap[TOTAL_TAG_NUMBER+1] ;

	//�����������ݵ�xTLVList��������
	if((iRet = AssociateTLVList(pxData)) != 0)
	{
		return iRet;
	}
	
	memset(bitmap, 0, sizeof(bitmap));
	//��ȡ���׵�BitMap
	iRet = -1;
	for(i = 0; i < TOTAL_TRANS_NUMBER; i++)
	{
		if(xBitmapList[i].TransType == cTransType)
		{	
			memcpy(bitmap, xBitmapList[i].bitmap, TOTAL_TAG_NUMBER);
			iRet = 0;
			break;
		}
	}		

	if(iRet != 0)
	{
		return PACKTLV_ERR_LENGTH;
	}
	//����bitmap���
	for( i = 0; i < TOTAL_TAG_NUMBER; i ++)
	{
		if(bitmap[i] == '1')
		{
			if((iRet = PackTLV(&xTLVList[i], &pxOutData[iDataLen])) <= 0)
			{
				return iRet;
			}
			else
			{
				iDataLen += iRet;
			}
		}
	}

	return iDataLen;
}
#if 0
/**
 *	@sn	static int GetTLVTagInfo(int iOff, PBOC_TLV *pxTag)	
 *	@brief	��iOffλ�ÿ�ʼ��ȡ1��TAG����Ϣ
 *	@param iOff :tag��xDataBuf�еĿ�ʼλ��
 *	@param pxTag:��ȡ�ĸ�TAG����Ϣ
 *	@return @li <= 0����
 *	@li > 0��ǰָ��ƫ��λ��,������һ��TAGʹ��
 */
static int GetTLVTagInfo(int iOff, PBOC_TLV *pxTag)	
{
	int i = 0, iLenLen = 0, iDataLen = 0, j = 0;
	char tmp[5];

	i = iOff;
//ȡTag
	if((xDataBuf[iOff] & 0x1f) == 0x1f)
	{	
		i += 1;

		while(i < MAX_LABEL_LEN)
		{
			i += 1; 
			if((xDataBuf[i] & 0x80) == 0)
			{
				break;
			}
		}
	}
	else
	{
		i += 1;
	}
	pxTag->iLabelLen = i - iOff;
	if(pxTag->iLabelLen > MAX_LABEL_LEN)
	{
		return -1;
	}
	memcpy(pxTag->cLabel, &xDataBuf[iOff], pxTag->iLabelLen);
	
	//ȡ����
	if(xDataBuf[i] & 0x80)
	{
		iLenLen = xDataBuf[i] & 0x7f;
		i += 1;
		memcpy(tmp, &xDataBuf[i], iLenLen);
		for(j = 0; j < iLenLen; j++)
		{
			iDataLen = (iDataLen << 8) + tmp[j];
		}
		i += iLenLen;
	}
	else
	{
		iDataLen = xDataBuf[i] & 0x7f;
		i += 1;
	}
	pxTag->iValueLen = iDataLen;
	
	//ȡ����
	pxTag->pxValue = &xDataBuf[i]; 
	i += iDataLen;
	
	return i;
}
/**
 *	@sn	static int PBOC_UNPACK(int iDataLen, char * pxInData, PBOC_TRANS_DATA * pxOutData)
 *	@brief	��һ���ԣִ̣����ݽ⵽PBOC_TRANS_DATA�ṹ��
 *	@param	iDataLen	:����ԣִ̣��ĳ���
 *	@param	pxInData	:����ԣִ̣�
 *	@param	pxOutData:�������Ľ��
 *	@return	@li <0 ����
 *	@li = 0 �ɹ�
 */
static int PBOC_UNPACK(int iDataLen, char * pxInData, PBOC_TRANS_DATA * pxOutData)
{
	int  iRet = 0;
	PBOC_TLV tmp;
	char iOff = 0;

	//�����������ݵ�xTLVList��������
	if((iRet = AssociateTLVList(pxOutData)) != 0)
	{
		return iRet;
	}
	if(iDataLen > PACK_BUF_LEN)
	{
		return -1;
	}
	memcpy(xDataBuf, pxInData, iDataLen);
	iOff = 0;
	while(iOff < iDataLen)///<���Off == iDataLen��������
	{			
		memset((char *)&tmp, 0, sizeof(PBOC_TLV));
		iRet = GetTLVTagInfo(iOff, &tmp);
		
		if(iRet > 0)
		{
			iOff = iRet;//Ϊ���¸�TAG׼��
		}
		else
		{
			return -1;
		}
		switch(tmp.pxValue[0])
		{
			case '\x9C':	//transtype
				memcpy(&pxOutData->TransType, tmp.pxValue, 1);
				break;
			case '\x8a':	//ARC
				memcpy(pxOutData->Auth_Res_Code, tmp.pxValue, 2);
				break;
			case '\x91':	//Issuer Auth Data
				pxOutData->IADLen = tmp.iValueLen;
				memcpy(pxOutData->IAD, tmp.pxValue, pxOutData->IADLen);
				break;
			case '\x72':	//Issuer Script, Var length
				pxOutData->ISLen = tmp.iValueLen;
				pxOutData->ISFlag = 0x72;
				memcpy(pxOutData->IS, tmp.pxValue, pxOutData->ISLen);
				break;
			case '\x71':	//Issuer Script, Var length
				pxOutData->ISLen = tmp.iValueLen;
				pxOutData->ISFlag = 0x71;
				memcpy(pxOutData->IS, tmp.pxValue, pxOutData->ISLen);
				break;
			case '\xdc':
				pxOutData->UpdateKeyLen = tmp.iValueLen;
				memcpy(pxOutData->UpdateKey, tmp.pxValue, pxOutData->UpdateKeyLen);
				break;
			default:		//Unknown Tag
				break;
		}
	
	}
	if(iOff != iDataLen)///<�����Ȳ�һ��
	{
		return -1;
	}
	
	return 0;
}
#endif
/**
 *	@sn	int CommWithHost(char cTransType, PBOC_TRANS_DATA * pxInData,  char * pxOutData)
 *	@brief	������ͨѶ
 *	@param	cTransType :��������
 *	@param	pxInData:�ն˵õ��Ľ�������
 *	@param	pxOutData:�������ؽ�����Ӧ����
 *	@return < 0����
 *	@li == 0�ɹ�
 */
int CommWithHost(char cTransType, PBOC_TRANS_DATA * pxInData, char * pxOutData)
{
	int iRet = 0, iDataLen = 0;
	char xDataBuf[PACK_BUF_LEN];
	
	InitComm();
	memset(xDataBuf, 0, sizeof(xDataBuf));
	if((iRet = PBOC_PACK(cTransType, pxInData, xDataBuf)) <= 0)
	{
		return iRet;
	}
	if((iRet = SendData(xDataBuf, iRet)) != SUCC)
	{
		return iRet;
	}
	if((iRet = ReceiveData(pxOutData, &iDataLen)) != SUCC)
	{
		return iRet;
	}	
	
	return iDataLen;
}

