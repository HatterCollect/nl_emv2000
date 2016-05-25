/**
* @file tvrtsi.c
* @brief ����TVR,TSI

  ��ģ����EMV2000��ʹ�ã����������������
       @li ���ݲ�������TVR�ֽڣ��ն���֤�����
	   @li ���ݲ�������TSI�ֽڣ�����״̬��Ϣ��
* @version Version 1.0
* @author Ҷ��ͳ
* @date 2005-08-02
*/
#include "AppData.h"
#include "tvrtsi.h"
#include <string.h>

///ȫ��TVR
static char *  _tvrvalue ; 
///ȫ��TSI
static char *  _tsivalue ;

/**
* @fn InitTVR
* @brief ��ʼ��TVR
*/
void InitTVR(void)
{
	char tmp[5] ;
	memset(tmp, 0x00, 5) ;
	SetAppData("\x95", tmp, 5) ;
}

/**
* @fn SetTVR
* @brief ����iSetMask����TVR��Ӧ��λ

* @param (in) int iSetMask Ҫ��TVR�ֽ������õ�λ������
* @param (in) OnOff  ��(1)��ر�(0)ĳλ��
* @return void
*/
void SetTVR( int iSetMask, int OnOff )
{
	_tvrvalue = GetAppData("\x95", NULL) ;
	if( OnOff )
	{
		_tvrvalue[iSetMask >> 8]  |= (unsigned char)iSetMask ;
	}
	else
	{
		_tvrvalue[iSetMask >> 8]  &= ~((unsigned char)iSetMask) ;
	}

}

/**
* @fn InitTSI
* @brief ��ʼ��TSI
*/
void InitTSI(void)
{
	char tmp[2] ;
	memset(tmp, 0x00, 2) ;
	SetAppData("\x9B", tmp, 2) ;
}

/**
* @fn SetTSI
* @brief ����iSetMask����TSI��Ӧ��λ

* @param (in) int iSetMask Ҫ��TSI�ֽ������õ�λ������
* @param (in) int OnOff  ��(1)��ر�(0)ĳλ
* @return void
*/
void SetTSI( int iSetMask, int OnOff )
{
	_tsivalue = GetAppData("\x9B", NULL) ;
	if ( OnOff )
	{
		_tsivalue[iSetMask >> 8] |= (unsigned char)iSetMask ;
	}
	else
	{
		_tsivalue[iSetMask >> 8] &= ~((unsigned char)iSetMask) ;
	}
}

/**
* @fn GetTVR
* @brief ����TVR�ֽ�

* @param (out) char * tvrtmp ����TVR�ֽ�
* @return void
*/
void GetTVR( char * tvrtmp )
{

	memcpy(tvrtmp, _tvrvalue, 5) ;

}

/**
* @fn GetTSI
* @brief ����TVR�ֽ�

* @param (out) char * tsitmp ����TVR�ֽ�
* @return void
*/
void GetTSI( char * tsitmp )
{
	memcpy(tsitmp, _tsivalue, 2) ;
}
