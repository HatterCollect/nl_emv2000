/**
* @file CardVerify.c
* @brief �ֿ�����֤
  
* @version Version 2.0
* @author Ҷ��ͳ
* @date 2005-08-05
*/

#include "posapi.h"
#include "posdef.h"
#include "tvrtsi.h"
#include "tools.h"
#include "ic.h"
#include "AppData.h"
#include "CardVerify.h"
#include "offauth.h"
#include "des.h"
#include <string.h>

void _PreCardVerify(TCVML * pCVML, int iAmount)
{
	char * p ;
	int len ;
	
	pCVML->pAIP = GetAppData("\x82", NULL);
	pCVML->pAppCurrCode = GetAppData("\x9f\x42", NULL);
	pCVML->pTransCurrCode = GetAppData("\x5f\x2a", NULL);
		
	if (p = GetAppData("\x9f\x62", NULL))
		pCVML->CardIDType = *p ;
	pCVML->CardID = GetAppData("\x9f\x61", &len);
	pCVML->CardIDLen = len ;
	pCVML->iAmount = iAmount;
	p = GetAppData("\x9C", NULL) ;
	pCVML->IsCash = (p[0] == 0x01?1:0) ;
	
	p = GetAppData("\x9f\x33", NULL) ;
	pCVML->cvmcap = p[1] ;

	if (p = GetAppData("\x8e", &len) )
	{
		C4ToInt((unsigned int *)&(pCVML->CVMx), (unsigned char *)p);	
		C4ToInt((unsigned int *)&(pCVML->CVMy), (unsigned char *)p+4);
		
		pCVML->CVRList = p+8;
		pCVML->nCVRCount = len - 8;
	}
	else
		pCVML->nCVRCount = 0 ;
}

/**
* @fn cvrconditon
* @brief CVR���������жϣ���Card_Verify����

* @param (in) const TCVML * pCVML �ֿ�����֤Ҫ�õ�������
* @param (in) int i ָʾ��ǰ��CVM
* @return 0 ��ִ�и�CVR
          1 ִ��CVR
*/
int cvrconditon(const TCVML * pCVML, int i)
{
	char con = pCVML->CVRList[2 * i + 1] ;

	if (con >= 0x06)
	{
		if (pCVML->pAppCurrCode == NULL) //Ӧ�û��Ҳ�����
			return 0 ;
		if (memcmp(pCVML->pTransCurrCode, pCVML->pAppCurrCode, 2) != 0)
			return 0 ;
	}
	switch( con ) //CVM��������
	{
	case 0x00:				//����ִ��
		return 1 ;
	case 0x01:				//�����ATM�ֽ�
		return pCVML->IsCash  ;
	case 0x02:				//��������ֽ����
		return !pCVML->IsCash ;
	case 0x03:				//����ն�֧��CVM
		switch (0x3f & pCVML->CVRList[i * 2]) // ��ǰ��CVM
		{
		case 0x01:	//IC ������PIN�˶�
			return pCVML->cvmcap & 0x80 ;
		case 0x02:	//��������PIN
			return pCVML->cvmcap & 0x40 ;
		case 0x03:	//IC ������PIN + ǩ��
			return (pCVML->cvmcap & 0x80) && (pCVML->cvmcap & 0x20) ;
		case 0x04:
			return (pCVML->cvmcap & 0x10) && (pCVML->cvmcap & 0x20) ;
		case 0x05:
			return (pCVML->cvmcap & 0x10) ;
		case 0x1E:
			return pCVML->cvmcap & 0x20 ;
		case 0x1F: 	//����CVM
			return pCVML->cvmcap & 0x08 ;
		case 0x20: 	//�ֿ���֤����ʾ
		 	return pCVML->cvmcap & 0x01 ;
		default:
			return 0 ;//���б���ֵ������,������
		}
	case 0x04: 				//���������ֵ���ֽ�
		return pCVML->IsCash & 0x02 ;
	case 0x05:				//����Ƿ���
		return pCVML->IsCash & 0x04 ;
	case 0x06:				//���������Ӧ�û��ҽ��в��ҽ��С��x 
		if (pCVML->iAmount < pCVML->CVMx)
			return 1 ;
		break ;
	case 0x07:				// ���������Ӧ�û��ҽ��в��ҽ�����x 
		if (pCVML->iAmount > pCVML->CVMx)
			return 1 ;
		break ;
	case 0x08:				// ���������Ӧ�û��ҽ��в��ҽ��С��y 
		if (pCVML->iAmount < pCVML->CVMy)
			return 1 ;
		break ;
	case 0x09:				// ���������Ӧ�û��ҽ��в��ҽ�����y 
		if (pCVML->iAmount > pCVML->CVMy)
			return 1 ;
		break;
	default:
		break ;
	}
	return 0 ;
}

/**
* @fn pinwordcheck
* @brief ��������������Ч��

* @param (in) char * pPin �û����������PIN
* @param (in) int len pPin�ĳ���
* @return FAIL ��Ч
		  SUCC ��Ч
*/
int pinwordcheck(char *pin, int len)
{
	int i;

	for(i = 0; i < len ; i++ )
	{
		if( *(pin + i) < 0x30 || *(pin + i) > 0x39)	//��Ч�������ַ�
			return FAIL;
	}
	return SUCC;
}

/**
* @fn verifypin
* @brief ��װPIN��ָ֤�����ݣ�������ָ��

* @param (in) char * pPin �û����������PIN
* @param (in) int style ����1/����0
* @return ICERR_xxx IC�����ش���ָ��������ޣ�ʧ�ܣ��޷���֤����PIN����
		  FAIL ���ش���,�����˳�����
*/
int verifypin( char * pin, int style )
{
	int pinlen, ret ;
	char bcdpin[7] ;
	char buff[256] ;
	char pinblock[256];

	pinlen = strlen( pin ) ;//4-12�ֽ�
	AsciiToBcd((unsigned char*) bcdpin, (unsigned char*)pin, pinlen, 0 ) ; //�����
	if( pinlen & 0x01 )
		bcdpin[pinlen / 2] |= 0x0f ; //����������Ҫ���Ϊ1111
	memset( buff, 0xff, 256 ) ;
	buff[0] = 0x20 | (pinlen & 0x0f) ;
	memcpy( buff + 1, bcdpin, (pinlen + 1) / 2 ) ;

	if( style ) // ����PIN
	{
		pinlen = EncryptPin(buff, pinblock);
		if(pinlen < 0)
		{
			return pinlen;
		}
	}
	else	//����PIN
	{
		memcpy(pinblock, buff, 8);
		pinlen = 8 ; //����PIN���ݿ鳤��
	}

	ret = IC_Verify(pinblock, pinlen, (TVerifyMode)style) ;//Verify PINָ��
	return ret ;
}

/**
* @fn pinprocess
* @brief ִ��PIN��֤

* @param (in) int style ��֤��ʽ��VERIFYMODE_NORMAL���ģ�VERIFYMODE_ENCRYPT����
* return OUTLIMIT ���ޣ���֤ʧ��
		 QUIT	  δ��������
		 EXIT	  �������ʧЧ
		 SUCC	  ��֤ͨ��
		 FAIL     ����
*/
int pinprocess( int style )
{
	int ret ;
	int retrycount ; //���Լ���
	char resp[8] ;
	char pin[16] ;
	int maxretrycount;
	
	memset(resp, 0, 8) ;
	ret = IC_GetData(GETDATAMODE_PASSWORDRETRY, resp) ;
	if( (ret < 0) || (resp[0] != 0x9F && resp[1] != 0x17) || (resp[2] != 1) )
		retrycount = 3 ;//ȱʡֵ
	else
		retrycount = resp[3] ; //TLV��ʽ

	if ( !retrycount )
	{
		return OUTLIMIT ;
	}
	maxretrycount = retrycount;
	while( retrycount ) //��֤����
	{
		clrscr();
		memset(pin,0,16);
		printf("����������:\n");
		printf("(����%02d�λ���)\n", retrycount) ; 
		ret = getnumstr(pin,12,PASSWD,60);
		if(ret == QUIT )  // �˳�,δ���� 
		{
			if(retrycount == maxretrycount) //���δ����������˳�
			{
				return QUIT;
			}
			else
			{
				return -1000;
			}
		}
		if(ret == TIMEOUT)
			return ret ; //���̲�����
		if(ret < 4)
		{
			ERR_MSG("  ���볤�ȴ���!\n  ����������...");
			continue;
		}
		if( pinwordcheck(pin, ret) == FAIL ) //�Ƿ������ַ�
		{
			ERR_MSG("����������Ч�ַ�\n����������...");
			continue;
		}

		pin[12]=0;
		ret = verifypin( pin, style ) ; //��֤
		if ( ret == ICERR_NONE )
		{
			return SUCC; //��֤�ɹ�
		}
		else
		{
			if (ret == QUIT) //�ڶ�̬������֤��, ȡ�����ʧ��
				return FAIL ;
			if (ret == FAIL)	//verifyʧ��
				return -1000;
			IC_GetLastSW(resp, resp + 1) ;
			if ( (*resp == 0x69) && 
				( ( *(resp + 1) == 0x83) ||( *(resp + 1) == 0x84) ) )
			{
				return OUTLIMIT ;	//�ϴν��ף�PIN����ס
			}
			else if( *resp == 0x63 )  //PIN���Գ���
			{
				if ( (*( resp +1)  & 0x0F) == 0)
				{
					return OUTLIMIT; //����
				}
				retrycount = *( resp +1)  & 0x0F ; //���Լ���
				continue ;
			}
			else 
				return FAIL ;
		}
		retrycount -- ;
	}//End of " while( retrycount ) "
	return OUTLIMIT ; 
}

/**
* @fn Offline_PIN
* @brief �ѻ�PIN����

* @param (in) char cvrtype CVR��һ�ֽ�ָ�����ѻ�PIN��֤����
* @return 	-1 ����
			1 ʧ��
		  	2 �ɹ�
*/
int OffLinePIN( char cvrtype )
{
	int ret ;

	if( cvrtype == 0x01 || cvrtype == 0x03 )
		ret = pinprocess(VERIFYMODE_NORMAL) ;
	else
		ret = pinprocess(VERIFYMODE_ENCRYPT) ;

	switch( ret )
	{
	case OUTLIMIT:  // ���ޣ���֤ʧ��
		SetTVR(PIN_RETRY_EXCEED, 1) ;
		return 1 ;
	case QUIT:	   //δ��������
		SetTVR(REQ_PIN_NOT_INPUT, 1) ;
		return 1 ;
	case TIMEOUT:	   //�������ʧЧ
		SetTVR(REQ_PIN_PAD_FAIL, 1) ;
		return 1 ;
	case SUCC:
		SetTVR(UNKNOW_CVM, 0) ;
	   	SetTVR(PIN_RETRY_EXCEED, 0) ;
		SetTVR(REQ_PIN_PAD_FAIL, 0) ;
		SetTVR(REQ_PIN_NOT_INPUT, 0) ;
		SetTVR(INPUT_ONLINE_PIN, 0) ;
		return 2 ;
	case FAIL:
		return -1 ;
	default:
		return 1 ;
	}
	return 1 ;
}
/**
* @fn Online_PIN
* @brief ����PIN����

* @param (out) char * pPin �����û����������PIN
* @return 1 ʧ��
		  4 �ɹ�����Ҫ����
*/

int OnLinePIN( char * pPIN ) 
{
	int ret ;
	char pin[16] ;
	int pinlen;

	while(1) 
	{
		clrscr();
		memset(pin,0,16);
		printf("����������(����):\n");
		ret = getnumstr(pin,12,PASSWD,60);
		if(ret == QUIT)  // �˳�,δ���� 
		{
			SetTVR(REQ_PIN_NOT_INPUT, 1) ;
			return 1;
		}
		if(ret == TIMEOUT)
		{
			SetTVR(REQ_PIN_PAD_FAIL, 1) ;
			return 1 ; //���̲�����
		}
		if(ret < 4)
		{
			ERR_MSG("  ���볤�ȴ���!\n  ����������...");
			continue;
		}
		if( pinwordcheck(pin, ret) == FAIL ) //�Ƿ������ַ�
		{
			ERR_MSG("����������Ч�ַ�\n����������...");
			continue;
		}
		else
			break ;
	}
	pin[12] = 0 ;
	AsciiToBcd((unsigned char*)pPIN, (unsigned char*)pin, ret, 0);
	pinlen = ret;
	if( pinlen & 0x01 )
		pPIN[pinlen / 2] |= 0x0f ; //����������Ҫ���Ϊ1111
	Des((uchar *)pPIN, (uchar *)pPIN, DESKEY);
	SetTVR(INPUT_ONLINE_PIN, 1) ; /* ����PIN���� */
	return 4 ;	
}

/**
* @fn processcvr
* @brief ����һ��CVM

* @param (in) const TCVML * pCVML �ֿ�����֤Ҫ�õ�������
* @param (in) int i ָʾ��ǰ��CVM
* @param (out) char * pPin �����û����������PIN
* @return -1 ����
		  0 δִ��
		  1 ʧ��
		  2 �ɹ�
		  3 �ɹ�����Ҫǩ��
		  4 �ɹ�����Ҫ����
*/
int processcvr(const TCVML * pCVML, int i, char *pPIN )
{
	unsigned char cvrcode = 0x3f & pCVML->CVRList[2 * i] ;
	int ret ;
	switch ( cvrcode ) //CVM����
	{
	case 0x00:			//CVM����ʧ��
		return 1 ;
	case 0x01:			//����PIN��֤ 
	case 0x03:			//����PIN��֤��ǩ����ֽ�ţ�
	case 0x04:			//����PIN��֤ 
	case 0x05:			//����PIN��֤��ǩ����ֽ�ţ�
		if ((cvrcode <= 0x03 && (pCVML->cvmcap & 0x80)) ||
			( cvrcode >= 0x04 && ( pCVML->cvmcap & 0x10) ))
		{
			ret = OffLinePIN( cvrcode ) ;
			if (ret == 2) //�ѻ�PIN�ɹ�
			{
				if (cvrcode == 0x03 || cvrcode == 0x05) //��Ҫǩ��
				{
					if (pCVML->cvmcap & 0x20) //�ն�֧��ǩ��
						return 3 ;
					else
						return 1 ;
				}
				return 2 ;
			}
			else if (ret == -1) 
				return -1 ;
		}
		else
			SetTVR(REQ_PIN_PAD_FAIL, 1) ; //�ն˲�֧���ѻ�pin
		return 1 ;
	case 0x02:			//��������PIN��֤
		if (pCVML->cvmcap & 0x40) 
			return OnLinePIN( pPIN ) ;
		else
			SetTVR(REQ_PIN_PAD_FAIL, 1) ;//�ն˲�֧������pin
		return 1 ;
	case 0x1E:			//ǩ�� 
		if (pCVML->cvmcap & 0x20) 
			return 3 ; 
		return 1 ; //�ն˲�֧��ǩ��
	case 0x1F:			//����CVM
		if (pCVML->cvmcap & 0x08)
			return 2 ;
		return 1 ;
	case 0x20:			//�ֿ���֤����ʾ
		clrscr() ;
		printf("����: ") ;
		switch(pCVML->CardIDType)
		{
		case 0x00:
			printf("���֤\n") ;
			break ;
		case 0x01:
			printf("����֤\n") ;
			break ;
		case 0x02:
			printf("����\n") ;
			break ;
		case 0x03:
			printf("�뾳֤\n") ;
			break ;
		case 0x04:
			printf("��ʱ���֤\n") ;
			break ;
		case 0x05:
			printf(" ����\n") ;
			break ;
		default:
			break;
		}
		printf("֤����: ") ;
		for (ret = 0 ; ret < pCVML->CardIDLen ; ret++)
			printf("%c", pCVML->CardID[ret]) ;
		printf("\n��ȷ�밴\"ȷ��\" ") ;
		if (getkeycode(0) == ENTER)
			return 2 ;
		else
			return 1 ;
	default:				//ϵͳͳ������ʹ��,��EMV����������
		SetTVR(UNKNOW_CVM, 1) ; //δ֪��CVM
		return 1 ;
	}//"switch( cvrcode )" ends here
}

/**
* @fn Card_Verify
* @brief �ֿ�����֤

* @param (in) const TCVML * pcvml �ֿ�����֤�����б��Լ����������
* @param (out) char * pPin �����û����������PIN
* @return FAIL ��������ֹ
		  SUCC �ɹ����
		  SUCC_SIG �ɹ�����Ҫǩ��
		  SUCC_ONLINE �ɹ�����Ҫ����
*/
int CardVerify (const TCVML *pcvml,  char *pPIN)
{
	int iCVRTotal ; 
	int nextflag ; //�����ǰCVRʧ���Ƿ�Ӧ�ú�����CVR��1�ǣ�0��
	int ret = 0 ;
	int i ;
	char cvmr[3] = {0x00,0x00,0x00} ;
	
	if ( !(pcvml->pAIP[0] & 0x10) ) // ��Ƭ��֧�ֳֿ�����֤
	{
		cvmr[0] = 0x3F ; //δִ��CVM,����Ҫ���д�����CVM����
		SetAppData("\x9F\x34", cvmr, 3) ;
		return SUCC;
	}
	if( pcvml->nCVRCount == 0 ) //û��CVM�б�
	{
		cvmr[0] = 0x3F ;  
		SetAppData("\x9F\x34", cvmr, 3) ;
		SetTVR(ICC_DATA_LOST, 1) ; 
		SetTVR(CV_NOT_SUCCESS,1) ;
		return SUCC ;
	}
	if( pcvml->nCVRCount % 2 ) // ��ʽ����ʧ��
		return FAIL ;
	iNeedSignature = 0 ;
	iCVRTotal = pcvml->nCVRCount / 2 ; 
	//CVM������֤
	for (i = 0; i < iCVRTotal; i++) 
	{		
		//�ֿ�����������
		if( cvrconditon (pcvml, i) )
		{
			//cvmr��ǰ�����ֽ���Ϊcvm�ķ����������������
			cvmr[0] = pcvml->CVRList[i*2] ;
			cvmr[1] = pcvml->CVRList[2*i+1] ;
			nextflag = pcvml->CVRList[i*2] & 0x40 ; //�Ƿ�ִ�к����CVM

			//ִ�е�ǰCVM
			ret = processcvr(pcvml, i, pPIN) ;
			if( ret == 1 )       //��ǰʧ��
			{
				cvmr[2] = 0x01 ; //ʧ��
				if( nextflag ) //��һ��CVM���������
					continue ;
			}
			else if(ret == 2) //�ɹ�
				cvmr[2] = 0x02  ;
			else	 //δ֪,��ǩ��,�����
			{
				cvmr[2] = 0x00 ;
				if (ret == 3)
					iNeedSignature = 1 ;
			}
			if( ret != 0 ) // ���۳ɹ�ʧ�ܶ����������
				break ;
		}
		cvmr[0] = 0x3F ; //δִ��
	}
	SetTSI(CV_COMPLETION, 1) ;		//����TSI
	if ( cvmr[0] == 0x3f )			// ���һ��CVRҲδִ��
	{
		memcpy(cvmr,"\x3f\x00\x00",3);
	}
	SetAppData("\x9F\x34", cvmr, 3) ;
	if( ret >= 2 )			/* �ɹ� */
	{
		SetTVR(CV_NOT_SUCCESS, 0) ;
		return ret - 2 ;		/* 0 SUCC,1 SIGNATURE, 2 ONLINEPIN */ 
	}
	else if (ret >= 0) 
	{
		SetTVR(CV_NOT_SUCCESS, 1) ; /* δ�ɹ� */
		return SUCC;
	}
	else
		return FAIL ;
}

