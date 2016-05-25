/**
 * @file AppData.c
 * @brief �������ݴ�ȡ��ʽ

       Ϊ�˷���ģ��֮������ݽ���(����), �б�Ҫ����һ��ȫ�ֵĻ�����,
   ���ṩ���ʺ���,�������������,�������ڸ���ģ��ĺ������/����,
   ���㶨�Ʋ�ͬ�����ݷ���ģʽ----��������
       ��ģ���ṩ���ݵĳ�ʼ��,��ȡ,д��,ɾ������(���ȫ��)
 * @version Version 1.0
 * @author Ҷ��ͳ
 * @date 2005-8-31
 */

#include <string.h>
#include "posapi.h"
#include "AppData.h"

#ifndef MAXLEN
#define MAXLEN 4028		//ȫ�ֻ��������������
#endif 

#ifndef MAXTAG
#define MAXTAG 255 		//�����Դ�ŵı�ǩ��
#endif

static char 		_AppData[MAXLEN] ;			//ȫ�ֻ�����

static int		_DataManage[MAXTAG + 2] ;	//��������������


/**
 * @fn void InitAppData (void)
 * @brief ��ʼ��ȫ�ֻ�����Ϊȫ0x00

 * @return ��
 */
void InitAppData (void)
{
	memset (_AppData, 0x00, MAXLEN) ;
	memset (_DataManage, 0x00, MAXTAG) ;
}

/**
 * @fn int HashAppData( char *)
 * @brief  �ж��Ƿ����һ����ǩtag

 * @param char * tag Ҫ�жϵı�ǩ
 * @return int	@li  TRUE ����
 			    @li  FALSE ������
 */			     	
int HasAppData ( char * tag )
{
	int i ;
	int nID = 0 ;
	
	nID = * ( (unsigned char *)tag) ;
	if ( (*tag & 0x1F) == 0x1f ) //�и����ֽ�
	{
		nID = (nID << 8) | * ( (unsigned char *)(tag + 1)) ;
	}
	if (!nID)
		return FALSE;
	for (i = 1 ; i <= _DataManage[0] ; i++) 
	{
		if (nID == (_DataManage[i] & 0x0000ffff))
		{
			return i ;
		}
	}
	return FALSE ;	
}

/**
 * @fn char  * GetAppData(unsigned char  *, int *)
 * @brief ��ȫ�ֻ�����������
 
 * @param char  * tag 	Ҫ��ȡ���ݵı�ǩ,��"\x9F\x07"
 * @param int  * length 	���������ݵĳ���
 * @return char  * ָ�������ڻ������е�λ�� 
 			 NULL û���ҵ�Ҫ�������
 */
char * GetAppData (char * tag, int * length)
{
	int i ;
	
	if ( (i = HasAppData(tag)) == FALSE)
	{
		return NULL ;
	}

	if( length != NULL)
	{
		*length =  (_DataManage[i + 1] >> 16) - (_DataManage[i] >> 16) ;
	}
	return  _AppData + (_DataManage[i] >> 16) ;
	
}

/**
 */
void DeleteAppData (int nIndex)
{
	int offset , len1, len2 ;
	
	offset = _DataManage[nIndex] >> 16 ; //Ҫɾ������ʼ��ַ
	len1 =  (_DataManage[nIndex + 1] >> 16) - offset ;//Ҫɾ���ĳ���
	len2 =  (_DataManage[_DataManage[0] + 1] >> 16) - offset - len1 ;//

	memcpy (_AppData + offset, _AppData + offset + len1, len2) ;
	len1 <<= 16 ;
	for (offset = nIndex + 1 ; offset <= _DataManage[0] +1 ; offset++)
	{
		_DataManage[offset] -= len1 ;
		_DataManage[offset - 1] = _DataManage[offset] ;
	}
	_DataManage[0]-- ;
}
/**
 * @fn int SetAppData(unsigned char  *, unsigned char  * content, int length)
 * @brief ��ȫ�ֻ�����д����

 * @param char  * tag Ҫд������ݵı�ǩ
 * @param char  * content д�������
 * @param int	   length д�����ݵĳ���
 * @return	SUCC	�ɹ�
 			ADERR_INVALIDTAG ��Ч�ı�ǩ
 			ADERR_DATADUP	 ��ǩ�Ѿ�����
 			ADERR_TAGOVER	 ��ǩ������
 			ADERR_LENOVER	 ��������
 */
int SetAppData (char  * tag, char  * content, int length)
{
	int i = 0 ;
	int nID = 0 ;
	int len, offset ;
	
	nID = * ( (unsigned char *)tag) ;
	if ( (*tag & 0x1F) == 0x1f ) //�и����ֽ�
	{
		nID = (nID << 8) | * ( (unsigned char *)(tag+1)) ;
	}
	if (!nID)
		return ADERR_INVALIDTAG ;	
	for (i = 1 ; i <= _DataManage[0] ; i++) 
	{
		if (nID == (_DataManage[i] & 0x0000ffff))
		{
			offset = _DataManage[i ] >> 16 ;
			len =  (_DataManage[i + 1] >> 16) - offset ;
			if (len != length)
			{
				DeleteAppData(i) ;
				break ;
			}
			memcpy(_AppData + offset, content, length) ;
			return SUCC ;
		}
	}
	
	if (_DataManage[0] == MAXTAG) //��ǩ��������
		return ADERR_TAGOVER ;
	offset = _DataManage[_DataManage[0] + 1] >> 16 ; //��һ�����Դ�ŵ�λ��
	len =MAXLEN - offset ; //��һ�����Դ�ŵĳ���
	if (len <length) //���Ȳ���
		return ADERR_LENOVER ;
	
	memcpy (_AppData +offset, content, length) ; //д�뻺����
	_DataManage[0] ++ ; //���ӱ�ǩ��
	_DataManage[_DataManage[0]] = ((offset << 16) | nID) ; //���浱ǰ��ǩ����Ϣ
	_DataManage[_DataManage[0] + 1] = ((offset + length) << 16) ;//��¼����һ�����õ�λ��
	return SUCC ;
}

/**
 * @fn int DelAppData(unsigned char  * tag)
 * @�ӻ�������ɾ��ָ����ǩ��ֵ

 * @param char  * tag Ҫɾ���ı�ǩ
 * @return SUCC
 		   ADERR_NOTFOUND �����ڸñ�ǩ
 */
int DelAppData (char  * tag)
{
	int i = 0 ;
	
	if ( (i = HasAppData(tag)) == FALSE)
	{
		return ADERR_NOTFOUND ;
	}
	
	DeleteAppData(i) ;
	return SUCC ;
}

void GetTags(int * tmp, int * num)
{
	memcpy( tmp, _DataManage + 1, _DataManage[0] * 4) ;
	*num = _DataManage[0] ;
}