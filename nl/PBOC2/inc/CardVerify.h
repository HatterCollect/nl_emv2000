/**
* @file CardVerify.h
* @brief �ֿ�����֤ͷ�ļ�
  
* @version Version 2.0
* @author Ҷ��ͳ
* @date 2005-08-05
*/

#ifndef _CARDHOLDER_VERIFY_
#define _CARDHOLDER_VERIFY_

#define SUCC_SIG 1      ///�ɹ�����Ҫǩ��
#define SUCC_ONLINE 2   ///�ɹ�����Ҫ����

#define OUTLIMIT -6 	//����

void _PreCardVerify(TCVML * pCVML, int iAmount) ;
///�ֿ�����֤����
int CardVerify(const TCVML * pcvml, char * pPIN) ;

#endif