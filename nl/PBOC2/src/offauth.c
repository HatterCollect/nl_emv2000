#include <string.h>
#include <stdlib.h>
#include "posapi.h"
#include "define.h"
#include "posdef.h"
#include "ic.h"
#include "capk.h"
#include "rsa.h"
#include "tools.h"
#include "tvrtsi.h"
#include "appdata.h"
#include "dolprocess.h"
#include "tlv.h"


//��ʲ�㷨��ʶ
#define HASH_ALGORITHM	0x01

char m_isspk_modules[MAX_RSA_MODULUS_LEN];	//�����й�Կģ
char m_isspk_exp[3];						//�����й�Կָ��
int  m_isspk_modules_len;					//�����й�Կģ��

char m_icpk_modules[MAX_RSA_MODULUS_LEN];	//IC����Կģ
char m_icpk_exp[3];							//IC����Կģָ��
int  m_icpk_modules_len;					//IC����Կģ��

char m_pinpk_modules[MAX_RSA_MODULUS_LEN];	//PIN��Կģ
char m_pinpk_exp[3];						//PIN��Կģָ��
int  m_pinpk_modules_len;					//PIN��Կģ��

char m_RID[5];
char m_staticdata[MAX_RSA_MODULUS_LEN]; 

//SDA�������
typedef struct
{
	char 	index; 				//ע�����Ĺ�Կ����
 	char 	*isscert;			//�����й�Կ֤��
 	int	 	isscert_len;		//�����й�Կ֤�鳤��
 	char 	*isspk_remainder;	//�����й�Կ����
 	int		isspk_remainder_len;//�����й�Կ�����
 	char	*isspk_exp;			//�����й�Կָ��
 	int		isspk_exp_len;		//�����й�Կָ������
 	char	*signeddata;		//ǩ����ľ�̬Ӧ������
 	int		signeddata_len;		//ǩ����ľ�̬Ӧ�����ݳ���
 	char	*pan;				//IC�����ʺ�
 	int		pan_len;			//���ʺų���
}TSDAData;


//DDA�������
typedef struct
{
	TSDAData sdadata;
 	char 	*iccert;			//IC����Կ֤��
 	int	 	iccert_len;			//IC����Կ֤�鳤��
 	char 	*icpk_remainder;	//IC����Կ����
 	int		icpk_remainder_len; //IC����Կ�����
 	char	*icpk_exp;			//IC����Կָ��
 	int		icpk_exp_len;		//IC����Կָ������
}TDDAData;


/**
* @fn _JudgeExpdate
* @brief �ж�RID, ��Կ������֤�����к��Ƿ���Ч

* @param (in)	char RID[5]	 
* @param (in)	char index ��Կ����	 
* @param (in)	char serialnum[3]	֤�����к�	 
* @return  �ɹ�: SUCC(��Ч)
* @return  ʧ��: FAIL(��Ч)
*/
int _JudgeBlackCert(char RID[5], char index, char serialnum[3])
{
	int fp;
	int len;
	char curvalue[16];
	char buf[16];

	memset(curvalue, 0, sizeof(curvalue));
	memcpy(curvalue, RID, 5);
	memcpy(curvalue+5, &index, 1);
	memcpy(curvalue+6, serialnum, 3);
	
	//�򿪷�����֤��������ļ�
	fp = fopen(BlackCertFile, "r");
	if(fp < 0)
	{
		return SUCC;
	}

	while(1)
	{
		len = fread(buf, sizeof(TBlackCert), fp);
		if(len != sizeof(TBlackCert))
		{
			break;			
		}

		if(memcmp(curvalue, buf, sizeof(TBlackCert))==0)
		{
			fclose(fp);
			return FAIL;
		}
	}
	
	fclose(fp);
	return SUCC;
}


/**
* @fn _JudgeExpdate
* @brief �ж��Ƿ񳬹���Ч��

* @param (in)	char *expdate	��Ч��
* @return  �ɹ�: SUCC(����Ч����)
* @return  ʧ��: FAIL(������Ч����)
*/
int _JudgeExpdate(char *expdate)
{
	char date[9];
	char time[7];
	char buf[9];
	int curday;
	int expday;
	char expiredate[5];

	memset(date, 0, sizeof(date));
	memset(time, 0, sizeof(time));
	GetTime(date, time);

	curday = 0;
	expday = 0;

	memset(buf, 0, sizeof(buf));
	memcpy(buf, date, 6);
	curday = atoi(buf);

	
	memset(expiredate, 0, sizeof(expiredate));
	BcdToAscii((unsigned char *)expiredate, (unsigned char *)expdate, 4, 0);
	memset(buf, 0, sizeof(buf));
	if(memcmp(expiredate+2, "50", 2)>0)	//�����ݴ���50����Ϊ��20����
	{
		memcpy(buf, "19", 2);
	}
	else
	{
		memcpy(buf, "20", 2);
	}
	
	memcpy(buf+2, expiredate+2, 2);
	memcpy(buf+4, expiredate, 2);
	expday = atoi(buf);

	if(curday <= expday)
	{
		return SUCC;
	}
	else
	{
		return FAIL;
	}
}


/**
* @fn _PreSDA
* @brief SDA(��̬������֤)Ԥ������ý���SDA���������

* @param (out)	TSDAData sdadata	SDA������������
* @return  �ɹ�: SUCC
* @return  ʧ��: FAIL
*/
int _PreSDA(TSDAData *sdadata)
{
	char *pdata;
	int len;

	len = 0;
	pdata = GetAppData("\x8f", &len);
	if(len !=1)
	{
		return FAIL;
	}
	sdadata->index = pdata[0];
	
	sdadata->isscert = GetAppData("\x90", &sdadata->isscert_len);
	if(sdadata->isscert == NULL)
	{
		return FAIL;
	}

	sdadata->isspk_remainder = GetAppData("\x92", &sdadata->isspk_remainder_len);

	sdadata->isspk_exp = GetAppData("\x9f\x32", &sdadata->isspk_exp_len);
	if(sdadata->isspk_exp == NULL)
	{
		return FAIL;
	}

	sdadata->pan = GetAppData("\x5a", &sdadata->pan_len);
	if(sdadata->pan == NULL)
	{
		return FAIL;
	}

	return SUCC;
}


/**
* @fn GetIssuerPK
* @brief ��÷����й�Կ

* @param (in)	char RID[5]			//Ӧ���ṩ�߱�ʶ  		
* @param (in)	TSDAData *sdadata	//��̬������֤��������
* @param (out)	unsigned char *isspk_modules	//�����й�Կģ
* @param (out)	unsigned char *isspk_exp		//�����й�Կָ��
* @param (out)	int *isspk_len		//�����й�Կָ��ģ��
* @return  �ɹ�: SUCC
* @return  ʧ��: FAIL
*/
int GetIssuerPK(char RID[5], TSDAData *sdadata, char *isspk_modules, char *isspk_exp, int *isspk_len)
{
	int ret;
	int i;
	TCAPK capk;
	char res[MAX_RSA_MODULUS_LEN];
	char buf[MAX_RSA_MODULUS_LEN];
	char digest[20];

	//���¶���Ϊ�ӷ����й�Կ֤��ָ���������Ԫ��
	char DataHeader;	//����ͷ
	char DataTrailer;	//����β
	char CertFormat;	//֤���ʽ
	char IssIdNum[4];	//�����б�ʶ
	char ExpDate[2];	//֤����Ч��
	char SerialNum[3];	//֤�����к�
	char HashAlg;		//��ʲ�㷨
	char IssPkAlg;		//�����й�Կ��ʲ�㷨
	char IssPkLen;		//�����й�Կ����
	char IssPkExpLen;	//�����й�Կָ������
	char IssPkModules[MAX_RSA_MODULUS_LEN];	//�����й�Կ����ģ
	char HashValue[20];		//��ʲֵ
	
	//������֤����������RID��ȡCA��Կ
	ret = CAPK_Load(RID, sdadata->index, &capk);

	//��ȡCA��Կʧ�ܣ��˳�
	if(ret != CAPKERR_NONE)
	{
//		ERR_MSG("  ָ����CA��Կ\n    ������");
		return FAIL;
	}

	//�жϷ�����֤�鳤���Ƿ����CA��Կ����
	if(sdadata->isscert_len != capk.Len)
	{
//		ERR_MSG("   ������֤��\n    ���ȴ���");
		return FAIL;
	}

	//�ָ�������֤��
	memset(res, 0, sizeof(res));
	ret = Recover((unsigned char *)sdadata->isscert, (unsigned char *)capk.Value, 
		(unsigned char *)capk.Exponent, capk.Len, (unsigned char *)res);
	if(ret != SUCC)
	{
//		ERR_MSG("   ������֤��\n    �ָ�ʧ��");
		return FAIL;
	}

	//�ֽⷢ����֤������
	DataHeader = res[0];
	CertFormat = res[1];
	memcpy(IssIdNum, res+2, 4);
	memcpy(ExpDate, res+6, 2);
	memcpy(SerialNum, res+8, 3);
	HashAlg = res[11];
	IssPkAlg = res[12];
	IssPkLen = res[13];
	IssPkExpLen = res[14];
	memcpy(IssPkModules, res+15, capk.Len-36);
	memcpy(HashValue, res + capk.Len-21, 20);
	DataTrailer = res[capk.Len-1];
	

	//���֤���ͷ�Ƿ�Ϸ�
	if(DataHeader != 0x6a)	
	{
//		ERR_MSG("   ������֤��\n   ����ͷ����");
		return FAIL;
	}

	//���֤���ʽ�Ƿ�Ϸ�
	if(CertFormat != 0x02)	
	{
//		ERR_MSG("   ������֤��\n    ��ʽ����");
		return FAIL;
	}

	//���֤��β�Ƿ�Ϸ�
	if(DataTrailer != 0xbc)
	{
//		ERR_MSG("   ������֤��\n   ����β����");
		return FAIL;
	}

	//��鷢���й�Կģ�����Ƿ����
	if(IssPkLen > capk.Len - 36 && sdadata->isspk_remainder == NULL)
	{
		SetTVR(ICC_DATA_LOST, 1);
		return FAIL;
	}	

	//���ɲ����ʲ���������
	memset(buf, 0, sizeof(buf));
	memcpy(buf, res+1, capk.Len-22);
	memcpy(buf + capk.Len-22, sdadata->isspk_remainder,
		sdadata->isspk_remainder_len);
	memcpy(buf + capk.Len-22+sdadata->isspk_remainder_len,
		sdadata->isspk_exp, IssPkExpLen);

	//�����ʲ���
	memset(digest, 0, sizeof(digest));
	CalcMsgDigest(buf, capk.Len-22+sdadata->isspk_remainder_len+
		IssPkExpLen, digest);
	
	//�ȽϹ�ʲ����Ƿ���ȷ
	if(memcmp(digest, HashValue, 20) != 0)
	{
//		ERR_MSG(" ������֤��\n ��ʲ�������ȷ");
		return FAIL;
	}

	//�жϷ����б�ʶ�Ƿ���ȷ
	memset(buf, 0, sizeof(buf));
	BcdToAscii((unsigned char *)buf, (unsigned char *)IssIdNum, 8, 0);
	BcdToAscii((unsigned char *)buf+8, (unsigned char *)(sdadata->pan), 8, 0);
	for(i=0; i<8; i++)
	{
		if(buf[i]=='F')
		{
			break;
		}

		if(buf[i] != buf[8+i])
		{
//			ERR_MSG(" �����б�ʶ����");
			return FAIL;
		}
	}

	//�ж�֤���Ƿ�ʧЧ
	ret = _JudgeExpdate(ExpDate);
	if(ret != SUCC)
	{
//		ERR_MSG("  ֤���Ѿ�����");
		return FAIL;
	}
	

	//��֤ rid, ��Կ������֤�����к��Ƿ���Ч
	if(_JudgeBlackCert(RID, sdadata->index, SerialNum)!=SUCC)
	{
//		ERR_MSG("������֤���ѳ���");
		return FAIL;
	}
		
	//�жϹ�Կ�㷨��ʶ�Ƿ���ȷ
	if(IssPkAlg != HASH_ALGORITHM)
	{
//		ERR_MSG("   �����й�Կ\n    �㷨����");
		return FAIL;
	}	

	//���ɷ����й�Կ
	*isspk_len = IssPkLen;
	if(*isspk_len <= capk.Len-36)
	{
		memcpy(isspk_modules, IssPkModules, IssPkLen);
	}
	else
	{
		memcpy(isspk_modules, IssPkModules, capk.Len-36);
		memcpy(isspk_modules + capk.Len - 36, sdadata->isspk_remainder, sdadata->isspk_remainder_len);
	}
	
	if(IssPkExpLen == 3)
	{
		memcpy(isspk_exp, sdadata->isspk_exp, 3);
	}
	else
	{
		memset(isspk_exp, 0x00, 3);
		isspk_exp[2] = sdadata->isspk_exp[0];
	}

	return SUCC;
}


/**
* @fn GetIcPK
* @brief ���IC����Կ

* @param (in)	char RID[5]			//Ӧ���ṩ�߱�ʶ  		
* @param (in)	TDDAData *ddadata	//��̬������֤��������
* @param (out)	unsigned char *icpk_modules	//IC����Կģ
* @param (out)	unsigned char *icpk_exp		//IC����Կָ��
* @param (out)	int *icpk_len		//IC����Կָ��ģ��
* @return  �ɹ�: SUCC
* @return  ʧ��: FAIL
*/
int GetIcPK(TDDAData *ddadata, char *staticdata, 
	char *isspk_modules, char *isspk_exp, int isspk_len,
	char *icpk_modules, char *icpk_exp, int *icpk_len)
{
	int ret;
	int i;
	int len;
	char res[MAX_RSA_MODULUS_LEN];
	char buf[MAX_RSA_MODULUS_LEN];
	char digest[20];
	char *pAip;
	char *pdata;

	//���¶���Ϊ��IC����Կ֤��ָ���������Ԫ��
	char DataHeader;	//����ͷ
	char DataTrailer;	//����β
	char CertFormat;	//֤���ʽ
	char pan[10];		//�����б�ʶ
	char ExpDate[2];	//֤����Ч��
	char SerialNum[3];	//֤�����к�
	char HashAlg;		//��ʲ�㷨
	char IcPkAlg;		//IC����Կ��ʲ�㷨
	char IcPkLen;		//IC����Կ����
	char IcPkExpLen;	//IC����Կָ������
	char IcPkModules[MAX_RSA_MODULUS_LEN];	//IC����Կ����ģ
	char HashValue[20];		//��ʲֵ
	

	//�ж�IC����Կ֤�鳤���Ƿ���ڷ����й�Կģ����
	if(ddadata->iccert_len != isspk_len)
	{
//		ERR_MSG("   IC��֤��\n    ���ȴ���");
		return FAIL;
	}

	//�ָ�IC��֤��
	memset(res, 0, sizeof(res));
	ret = Recover((unsigned char *)ddadata->iccert, (unsigned char *)isspk_modules, 
		(unsigned char *)isspk_exp, isspk_len, (unsigned char *)res);
	if(ret != SUCC)
	{
//		ERR_MSG("   IC��֤��\n    �ָ�ʧ��");
		return FAIL;
	}

	//�ֽ�IC��֤������
	DataHeader = res[0];
	CertFormat = res[1];
	memcpy(pan, res+2, 10);
	memcpy(ExpDate, res+12, 2);
	memcpy(SerialNum, res+14, 3);
	HashAlg = res[17];
	IcPkAlg = res[18];
	IcPkLen = res[19];
	IcPkExpLen = res[20];
	memcpy(IcPkModules, res+21, isspk_len - 42);
	memcpy(HashValue, res + isspk_len-21, 20);
	DataTrailer = res[isspk_len-1];
	

	//���֤���ͷ�Ƿ�Ϸ�
	if(DataHeader != 0x6a)	
	{
//		ERR_MSG("   IC��֤��\n   ����ͷ����");
		return FAIL;
	}

	//���֤���ʽ�Ƿ�Ϸ�
	if(CertFormat != 0x04)	
	{
//		ERR_MSG("   IC��֤��\n    ��ʽ����");
		return FAIL;
	}

	//���֤��β�Ƿ�Ϸ�
	if(DataTrailer != 0xbc)
	{
//		ERR_MSG("   ������֤��\n   ����β����");
		return FAIL;
	}

	//���IC����Կģ�����Ƿ����
	if(IcPkLen > isspk_len - 42 && ddadata->icpk_remainder == NULL)
	{
		SetTVR(ICC_DATA_LOST, 1);
		return FAIL;
	}	

	//���ɲ����ʲ���������
	memset(buf, 0, sizeof(buf));
	memcpy(buf, res+1, isspk_len-22);
	len = isspk_len - 22;
	memcpy(buf + len, ddadata->icpk_remainder, ddadata->icpk_remainder_len);
	len = len + ddadata->icpk_remainder_len;
	memcpy(buf + len, ddadata->icpk_exp, IcPkExpLen);
	len = len + IcPkExpLen;
	memcpy(buf + len, staticdata+1, staticdata[0]);
	len = len + staticdata[0];

	pdata = GetAppData("\x9f\x4a", &i);
	if(pdata != NULL) 	//������̬������֤��ǩ�б�
	{
		if( i == 1 && pdata[0] == 0x82) //��ǩ�б�ֻ����82��ǩ
		{
			pAip = GetAppData("\x82", &ret);
			memcpy(buf + len, pAip, 2);
			len = len + 2;
		}
		else
		{
//			ERR_MSG("  ��̬������֤\n  ��ǩ�б����");
			return FAIL;
		}
	}

	
	//�����ʲ���
	memset(digest, 0, sizeof(digest));
	CalcMsgDigest(buf, len, digest);
	
	//�ȽϹ�ʲ����Ƿ���ȷ
	if(memcmp(digest, HashValue, 20) != 0)
	{
//		ERR_MSG(" IC��֤��\n ��ʲ�������ȷ");
		return FAIL;
	}

	//�ж����ʺ��Ƿ���ȷ
	memset(buf, 0, sizeof(buf));
	BcdToAscii((unsigned char *)buf, (unsigned char *)pan, 20, 0);
	BcdToAscii((unsigned char *)buf+20, (unsigned char *)ddadata->sdadata.pan, 20, 0);
	for(i=0; i<20; i++)
	{
		if(buf[i]=='F')
		{
			break;
		}

		if(buf[i] != buf[20+i])
		{
//			ERR_MSG("   ���ʺŴ���");
			return FAIL;
		}
	}

	//�ж�֤���Ƿ�ʧЧ
	ret = _JudgeExpdate(ExpDate);
	if(ret != SUCC)
	{
//		ERR_MSG("  ֤���Ѿ�����");
		return FAIL;
	}
	
	
	//�жϹ�Կ�㷨��ʶ�Ƿ���ȷ
	if(IcPkAlg != HASH_ALGORITHM)
	{
//		ERR_MSG("   �����й�Կ\n    �㷨����");
		return FAIL;
	}	

	//���ɷ����й�Կ
	*icpk_len = IcPkLen;
	if(*icpk_len <= isspk_len - 42)
	{
		memcpy(icpk_modules, IcPkModules, IcPkLen);
	}
	else
	{
		memcpy(icpk_modules, IcPkModules, isspk_len-42);
		memcpy(icpk_modules + isspk_len-42, ddadata->icpk_remainder, ddadata->icpk_remainder_len);
	}
	
	if(IcPkExpLen == 3)
	{
		memcpy(icpk_exp, ddadata->icpk_exp, 3);
	}
	else
	{
		memset(icpk_exp, 0x00, 3);
		icpk_exp[2] = ddadata->icpk_exp[0];
	}

	return SUCC;
}


int VerifyStaticData(TSDAData *sdadata, char *staticdata, char *isspk_modules, char *isspk_exp, int isspk_len)
{
	int ret;
	int i;
	char res[MAX_RSA_MODULUS_LEN];
	char buf[MAX_RSA_MODULUS_LEN];
	char digest[20];
	char *pdata;
	char *pAip;
	int len;

	//���¶���Ϊ��ǩ���ľ�̬Ӧ�����ݻָ���������Ԫ��
	char DataHeader;	//����ͷ
	char DataTrailer;	//����β
	char CertFormat;	//ǩ�����ݸ�ʽ
	char HashAlg;		//��ʲ�㷨
	char DataAuthCode[2];	//������֤����
	char HashValue[20];	//��ʲֵ
	
	//�ж�ǩ����̬Ӧ�����ݳ����Ƿ���ڷ����й�Կģ����
	if(sdadata->signeddata_len != isspk_len)
	{
//		ERR_MSG("  ǩ����̬Ӧ��\n  ���ݳ��ȴ���");
		return FAIL;
	}

	//�ָ���̬Ӧ������
	memset(res, 0, sizeof(res));
	ret = Recover((unsigned char *)sdadata->signeddata, (unsigned char *)isspk_modules, 
		(unsigned char *)isspk_exp, isspk_len, (unsigned char *)res);
	if(ret != SUCC)
	{
//		ERR_MSG("  ��̬Ӧ������\n    �ָ�ʧ��");
		return FAIL;
	}

	//�ֽ⾲̬Ӧ������
	DataHeader = res[0];
	CertFormat = res[1];
	HashAlg = res[2];
	memcpy(DataAuthCode, res+3, 2);
	memcpy(HashValue, res + isspk_len-21, 20);
	DataTrailer = res[isspk_len-1];
	

	//�������ͷ�Ƿ�Ϸ�
	if(DataHeader != 0x6a)	
	{
//		ERR_MSG("  ��̬Ӧ������\n �ָ�����ͷ����");
		return FAIL;
	}

	//���֤���ʽ�Ƿ�Ϸ�
	if(CertFormat != 0x03)	
	{
//		ERR_MSG("  ��̬Ӧ������\n  ���ݸ�ʽ����");
		return FAIL;
	}

	//���֤��β�Ƿ�Ϸ�
	if(DataTrailer != 0xbc)
	{
//		ERR_MSG("  ��̬Ӧ������\n  �ָ�����β����");
		return FAIL;
	}

	//���ɲ����ʲ���������
	memset(buf, 0, sizeof(buf));
	memcpy(buf, res+1, isspk_len-22);
	memcpy(buf + isspk_len-22, staticdata+1, staticdata[0]); 
	
	len = isspk_len - 22 + staticdata[0];

	
	pdata = GetAppData("\x9f\x4a", &i);
	if(pdata != NULL) 	//������̬������֤��ǩ�б�
	{
		if( i == 1 && pdata[0] == 0x82) //��ǩ�б�ֻ����82��ǩ
		{
			pAip = GetAppData("\x82", &ret);
			memcpy(buf + len, pAip, 2);
			len = len + 2;
		}
		else
		{
//			ERR_MSG("  ��̬������֤\n  ��ǩ�б����");
			return FAIL;
		}
	}
	
	//�����ʲ���
	memset(digest, 0, sizeof(digest));
	CalcMsgDigest(buf, len, digest);
	
	//�ȽϹ�ʲ����Ƿ���ȷ
	if(memcmp(digest, HashValue, 20) != 0)
	{
//		ERR_MSG(" ��̬������֤\n ��ʲ�������ȷ");
		return FAIL;
	}

	ret = SetAppData("\x9f\x45", DataAuthCode, 2);
	if(ret < 0)
	{
//		ERR_MSG("  ����������֤\n  ����ʧ��!!");
		return FAIL;
	}
	else
	{
		return SUCC;
	}

}


/**
* @fn _PreDDA
* @brief DDA(��̬������֤)Ԥ������ý���DDA���������

* @param (out)	TSDAData ddadata	DDA������������
* @return  �ɹ�: SUCC
* @return  ʧ��: FAIL
*/
int _PreDDA(TDDAData *ddadata)
{
	int ret;
	
	ret = _PreSDA(&ddadata->sdadata);
	if(ret < 0)
	{
		return FAIL;
	}
	
	ddadata->iccert = GetAppData("\x9f\x46", &ddadata->iccert_len);
	if(ddadata->iccert == NULL)
	{
		return FAIL;
	}

	ddadata->icpk_remainder = GetAppData("\x9f\x48", &ddadata->icpk_remainder_len);

	ddadata->icpk_exp = GetAppData("\x9f\x47", &ddadata->icpk_exp_len);
	if(ddadata->icpk_exp == NULL)
	{
		return FAIL;
	}

	return SUCC;	
}


int VerifyDynamicData(char *icpk_modules, char *icpk_exp, int icpk_len)
{
	int ret;
	char res[MAX_RSA_MODULUS_LEN];
	char buf[MAX_RSA_MODULUS_LEN];
	char ddol[MAX_RSA_MODULUS_LEN];
	char digest[20];
	char *pdata;
	int len;
	char dynadata[MAX_RSA_MODULUS_LEN];
	int length;

	//���¶���Ϊ��ǩ���ľ�̬Ӧ�����ݻָ���������Ԫ��
	char DataHeader;	//����ͷ
	char DataTrailer;	//����β
	char CertFormat;	//ǩ�����ݸ�ʽ
	char HashAlg;		//HASH�㷨
	char DynaDataLen;	//��̬���ݳ���
	char DynamicData[256]; //IC����̬����
	char HashValue[20];	//��ʲֵ
	
	//���IC����û��DDOL,ʹ���ն�ȱʡ��DDOL
	pdata = GetAppData(DDOL, NULL);
	if(pdata == NULL)
	{
		SetAppData(DDOL, g_termparam.DefaultDDOL, g_termparam.DefaultDDOLen);
	}
	
	//ȡDDOL
	memset(ddol, 0, sizeof(ddol));
	ret = DOLProcess(DDOL, ddol);
	if(ret <0)
	{
//		ERR_MSG("DDOL�����������");
		return FAIL;
	}

	//�ڲ���֤
	memset(buf, 0, sizeof(buf));
	length = IC_InternalAuthenticate(ddol+1, ddol[0], buf);
	if(length < 0)
	{
//		ERR_MSG("  �ڲ���֤ʧ��");
		return QUIT;
	}

	TLV_Init(NULL) ;
	if (TLV_Decode(buf, length) != TLVERR_NONE)
	{
		return  QUIT;
	}	

	//ȡ��̬ǩ������
	memset(dynadata, 0x00, sizeof(dynadata)) ;
	if (TLV_GetValue("\x80", dynadata, &len, 1) == TLVERR_TAGNOTFOUND)
	{
		if(TLV_GetValue("\x9f\x4b", dynadata, &len, 1) == TLVERR_TAGNOTFOUND)
		{
//			ERR_MSG(" �޶�̬Ӧ������\n");
			return FAIL;
		}
	}
	else
	{
		//�ж�80ģ���Ƿ���ȷ
		if(len + 2 != length)
		{
			return QUIT;
		}
	}
	
	//�ж�ǩ����̬Ӧ�����ݳ����Ƿ���ڷ����й�Կģ����
	if(len != icpk_len)
	{
//		ERR_MSG("  ǩ����̬Ӧ��\n  ���ݳ��ȴ���");
		return FAIL;
	}

	//�ָ���̬Ӧ������
	memset(res, 0, sizeof(res));
	ret = Recover((unsigned char *)dynadata, (unsigned char *)icpk_modules, 
		(unsigned char *)icpk_exp, icpk_len, (unsigned char *)res);
	if(ret != SUCC)
	{
//		ERR_MSG("  ��̬Ӧ������\n    �ָ�ʧ��");
		return FAIL;
	}

	//�ֽ⶯̬Ӧ������
	DataHeader = res[0];
	CertFormat = res[1];
	HashAlg = res[2];
	DynaDataLen = res[3];
	memcpy(DynamicData, res+4, DynaDataLen);
	memcpy(HashValue, res + icpk_len-21, 20);
	DataTrailer = res[icpk_len-1];
	

	//�������ͷ�Ƿ�Ϸ�
	if(DataHeader != 0x6a)	
	{
//		ERR_MSG("  ��̬Ӧ������\n �ָ�����ͷ����");
		return FAIL;
	}

	//���֤���ʽ�Ƿ�Ϸ�
	if(CertFormat != 0x05)	
	{
//		ERR_MSG("  ��̬Ӧ������\n  ���ݸ�ʽ����");
		return FAIL;
	}

	//���֤��β�Ƿ�Ϸ�
	if(DataTrailer != 0xbc)
	{
//		ERR_MSG("  ��̬Ӧ������\n  �ָ�����β����");
		return FAIL;
	}

	//���ɲ����ʲ���������
	memset(buf, 0, sizeof(buf));
	memcpy(buf, res+1, icpk_len-22);
	memcpy(buf + icpk_len-22, ddol+1, ddol[0]); 
	
	len = icpk_len - 22 + ddol[0];

	//�����ʲ���
	memset(digest, 0, sizeof(digest));
	CalcMsgDigest(buf, len, digest);
	
	//�ȽϹ�ʲ����Ƿ���ȷ
	if(memcmp(digest, HashValue, 20) != 0)
	{
//		ERR_MSG(" ��̬������֤\n ��ʲ�������ȷ");
		return FAIL;
	}

	ret = SetAppData("\x9f\x4C", DynamicData+1, DynamicData[0]);
	if(ret < 0)
	{
//		ERR_MSG("  ����IC��̬\n  ����ʧ��!!");
		return FAIL;
	}
	else
	{
		return SUCC;
	}

}



/**
* @fn GetPinPK
* @brief ���PIN��Կ

* @param (out)	unsigned char *pinpk_modules	//PIN��Կģ
* @param (out)	unsigned char *pinpk_exp		//PIN��Կָ��
* @param (out)	int *pinpk_len		//PIN��Կָ��ģ��
* @return  �ɹ�: SUCC
* @return  ʧ��: FAIL
* @return  ֤�鲻����: QUIT
*/
int GetPinPK(char *isspk_modules, char *isspk_exp, int isspk_len,
	char *pinpk_modules, char *pinpk_exp, int *pinpk_len)
{
	int ret;
	int len;
	char res[MAX_RSA_MODULUS_LEN];
	char buf[MAX_RSA_MODULUS_LEN];
	char digest[20];

	//���¶���Ϊ��Pin����Կ֤��ָ���������Ԫ��
	char DataHeader;	//����ͷ
	char DataTrailer;	//����β
	char CertFormat;	//֤���ʽ
	char pan[10];		//PIN��ʶ
	char ExpDate[2];	//֤����Ч��
	char SerialNum[3];	//֤�����к�
	char HashAlg;		//��ʲ�㷨
	char PinPkAlg;		//Pin����Կ��ʲ�㷨
	char PinPkLen;		//Pin����Կ����
	char PinPkExpLen;	//Pin����Կָ������
	char PinPkModules[MAX_RSA_MODULUS_LEN];	//Pin����Կ����ģ
	char HashValue[20];		//��ʲֵ

	char *pPincert; 			//PIN֤��ָ��
	int pincert_len;			//PIN֤�鳤��
 	char *pPinpk_remainder;		//PIN��Կ����
 	int	 pinpk_remainder_len;	//PIN��Կ�����
 	char *pPinpk_exp;			//PIN��Կָ��
 	int	 pinpk_exp_len;			//PIN��Կָ������

	pincert_len = 0;
	pinpk_exp_len = 0;
	pPincert = GetAppData("\x9f\x2d", &pincert_len);
	if(pPincert == NULL)
	{
		return QUIT;
	}

	pPinpk_remainder = GetAppData("\x9f\x2f", &pinpk_remainder_len);
	pPinpk_exp = GetAppData("\x9f\x2e", &pinpk_exp_len);
	if(pPinpk_exp == NULL)
	{
		return QUIT;
	}

	//�ж�Pin����Կ֤�鳤���Ƿ����PIN��Կģ����
	if(pincert_len != isspk_len)
	{
//		ERR_MSG("   Pin��֤��\n    ���ȴ���");
		return QUIT;
	}

	//�ָ�Pin��֤��
	memset(res, 0, sizeof(res));
	ret = Recover((unsigned char *)pPincert, (unsigned char *)isspk_modules, 
		(unsigned char *)isspk_exp, isspk_len, (unsigned char *)res);
	if(ret != SUCC)
	{
//		ERR_MSG("   Pin��֤��\n    �ָ�ʧ��");
		return FAIL;
	}

	//�ֽ�Pin��֤������
	DataHeader = res[0];
	CertFormat = res[1];
	memcpy(pan, res+2, 10);
	memcpy(ExpDate, res+12, 2);
	memcpy(SerialNum, res+14, 3);
	HashAlg = res[17];
	PinPkAlg = res[18];
	PinPkLen = res[19];
	PinPkExpLen = res[20];
	memcpy(PinPkModules, res+21, isspk_len - 42);
	memcpy(HashValue, res + isspk_len-21, 20);
	DataTrailer = res[isspk_len-1];
	

	//���֤���ͷ�Ƿ�Ϸ�
	if(DataHeader != 0x6a)	
	{
//		ERR_MSG("   Pin��֤��\n   ����ͷ����");
		return FAIL;
	}

	//���֤���ʽ�Ƿ�Ϸ�
	if(CertFormat != 0x04)	
	{
//		ERR_MSG("   Pin��֤��\n    ��ʽ����");
		return FAIL;
	}

	//���֤��β�Ƿ�Ϸ�
	if(DataTrailer != 0xbc)
	{
//		ERR_MSG("   PIN֤��\n   ����β����");
		return FAIL;
	}

	//���ɲ����ʲ���������
	memset(buf, 0, sizeof(buf));
	memcpy(buf, res+1, isspk_len-22);
	len = isspk_len - 22;
	memcpy(buf + len, pPinpk_remainder, pinpk_remainder_len);
	len = len + pinpk_remainder_len;
	memcpy(buf + len, pPinpk_exp, PinPkExpLen);
	len = len + PinPkExpLen;

	//�����ʲ���
	memset(digest, 0, sizeof(digest));
	CalcMsgDigest(buf, len, digest);

	//�ȽϹ�ʲ����Ƿ���ȷ
	if(memcmp(digest, HashValue, 20) != 0)
	{
//		ERR_MSG(" Pin֤��\n ��ʲ�������ȷ");
		return FAIL;
	}

	//�ж�֤���Ƿ�ʧЧ
	ret = _JudgeExpdate(ExpDate);
	if(ret != SUCC)
	{
//		ERR_MSG("  ֤���Ѿ�����");
		return FAIL;
	}
	
	
	//�жϹ�Կ�㷨��ʶ�Ƿ���ȷ
	if(PinPkAlg != HASH_ALGORITHM)
	{
//		ERR_MSG("   PIN��Կ\n    �㷨����");
		return FAIL;
	}	

	//����PIN��Կ
	*pinpk_len = PinPkLen;
	if(*pinpk_len <= isspk_len - 42)
	{
		memcpy(pinpk_modules, PinPkModules, PinPkLen);
	}
	else
	{
		memcpy(pinpk_modules, PinPkModules, isspk_len-42);
		memcpy(pinpk_modules + isspk_len-42, pPinpk_remainder, pinpk_remainder_len);
	}
	
	if(PinPkExpLen == 3)
	{
		memcpy(pinpk_exp, pPinpk_exp, 3);
	}
	else
	{
		memset(pinpk_exp, 0x00, 3);
		pinpk_exp[2] = pPinpk_exp[0];
	}

	return SUCC;
}



int EncryptPin(char *pin, char *encpin)
{
	int ret;
	int i;
	char ic_rand[8];
	char randnum[MAX_RSA_MODULUS_LEN];
	char buf[MAX_RSA_MODULUS_LEN];
	TDDAData ddadata;
	
	//���PIN��Կ
	ret = GetPinPK(m_isspk_modules, m_isspk_exp, m_isspk_modules_len, 
		m_pinpk_modules, m_pinpk_exp, &m_pinpk_modules_len);
	if( ret == FAIL )
	{
//ERR_MSG("pin ֤�����");
		return FAIL;
	}
	else if(ret == QUIT)
	{
//ERR_MSG("None PIN֤��");
		if(m_icpk_modules_len == 0) //û��IC����Կ�������
		{
			memset(&ddadata, 0, sizeof(ddadata));
			_PreDDA(&ddadata);
	
			//��÷����й�Կ
			memset(m_isspk_modules, 0x00, sizeof(m_isspk_modules));
			memset(m_isspk_exp, 0x00, sizeof(m_isspk_exp));
			m_isspk_modules_len = 0;
	
			ret = GetIssuerPK(m_RID, &(ddadata.sdadata), m_isspk_modules, m_isspk_exp, &m_isspk_modules_len);
			if(ret != SUCC)
			{
				return FAIL;
			}

			//���IC���й�Կ
			memset(m_icpk_modules, 0x00, sizeof(m_icpk_modules));
			memset(m_icpk_exp, 0x00, sizeof(m_icpk_exp));
			m_icpk_modules_len = 0;
	
			ret = GetIcPK(&ddadata, m_staticdata, m_isspk_modules,
				m_isspk_exp, m_isspk_modules_len, m_icpk_modules, m_icpk_exp, &m_icpk_modules_len);
			if(ret != SUCC)
			{
				return FAIL;
			}
		}
		memcpy(m_pinpk_modules, m_icpk_modules, m_icpk_modules_len);
		memcpy(m_pinpk_exp, m_icpk_exp, 3);
		m_pinpk_modules_len = m_icpk_modules_len;
	}

	//����IC�������
	memset(ic_rand, 0, sizeof(ic_rand));
	ret = IC_GetChallenge(ic_rand);
	if(ret <0 )
	{
//ERR_MSG("ȡ���������");
//		return QUIT;  //ԭCASEҪ���˳�����
		return FAIL ;  //����CASE CI032.00 �������˳�
	}

	//�����ն������
	memset(randnum, 0, sizeof(randnum));
	for (i=0 ; i<= (m_pinpk_modules_len-17) / 4 ; i++)
	{
		ret = rand();
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%d", ret);
		AsciiToBcd((unsigned char *)randnum+i*4, (unsigned char *)buf, 8, 0);
	}	

	memset(buf, 0, sizeof(buf));
	buf[0] = 0x7f;
	memcpy(buf + 1, pin, 8);
	memcpy(buf + 9, ic_rand, 8);
	memcpy(buf + 17, randnum, m_pinpk_modules_len-17);

	Recover((unsigned char * )buf, (unsigned char *)m_pinpk_modules,
		(unsigned char *)m_pinpk_exp, m_pinpk_modules_len, (unsigned char *)encpin);

	return m_pinpk_modules_len;
}


int OffAuth_SDA(char RID[5], char *staticdata)
{
	TSDAData sdadata;
//	char isspk_modules[MAX_RSA_MODULUS_LEN];	//�����й�Կģ
//	char isspk_exp[3];							//�����й�Կָ��
//	int isspk_len;								//�����й�Կģ����
	int ret;

	//��ʼ�����о�̬������֤���������
	memset(&sdadata, 0, sizeof(sdadata));
	ret = _PreSDA(&sdadata);
	sdadata.signeddata = GetAppData("\x93", &sdadata.signeddata_len);
	if(ret != SUCC || sdadata.signeddata == NULL )
	{
		SetTVR(ICC_DATA_LOST, 1);
		SetTVR(OFFLINE_SDA_FAIL, 1);
		SetTSI(OFFLINE_DA_COMPLETION, 1);
//		ERR_MSG("��̬��֤���ݲ�ȫ");
		return FAIL;
	}

	//��÷����й�Կ
	memset(m_isspk_modules, 0x00, sizeof(m_isspk_modules));
	memset(m_isspk_exp, 0x00, sizeof(m_isspk_exp));
	m_isspk_modules_len = 0;
	ret = GetIssuerPK(RID, &sdadata, m_isspk_modules, m_isspk_exp, &m_isspk_modules_len);
	if(ret != SUCC)
	{
		SetTVR(OFFLINE_SDA_FAIL, 1);
		SetTSI(OFFLINE_DA_COMPLETION, 1);
		return FAIL;
	}

	//��֤ǩ���ľ�̬Ӧ������
	ret = VerifyStaticData(&sdadata, staticdata, m_isspk_modules, m_isspk_exp, m_isspk_modules_len);
	if(ret != SUCC)
	{
		SetTVR(OFFLINE_SDA_FAIL, 1);
		SetTSI(OFFLINE_DA_COMPLETION, 1);
		return FAIL;
	}

	SetTSI(OFFLINE_DA_COMPLETION, 1);
	return SUCC;
}


int OffAuth_DDA(char RID[5], char *staticdata)
{
	TDDAData ddadata;
//	char isspk_modules[MAX_RSA_MODULUS_LEN];	//�����й�Կģ
//	char isspk_exp[3];							//�����й�Կָ��
//	int isspk_len;								//�����й�Կģ����
//	char icpk_modules[MAX_RSA_MODULUS_LEN];		//IC����Կģ
//	char icpk_exp[3];							//IC����Կָ��
//	int icpk_len;								//IC����Կģ����
	int ret;

	//��ʼ�����о�̬������֤���������
	memset(&ddadata, 0, sizeof(ddadata));
	ret = _PreDDA(&ddadata);
	if(ret != SUCC)
	{
		SetTVR(ICC_DATA_LOST, 1);
		SetTVR(OFFLINE_DDA_FAIL, 1);
		SetTSI(OFFLINE_DA_COMPLETION, 1);
//		ERR_MSG("��̬��֤���ݲ�ȫ");
		return FAIL;
	}
	
	//��÷����й�Կ
	memset(m_isspk_modules, 0x00, sizeof(m_isspk_modules));
	memset(m_isspk_exp, 0x00, sizeof(m_isspk_exp));
	m_isspk_modules_len = 0;
	
	ret = GetIssuerPK(RID, &(ddadata.sdadata), m_isspk_modules, m_isspk_exp, &m_isspk_modules_len);
	if(ret != SUCC)
	{
		SetTVR(OFFLINE_DDA_FAIL, 1);
		SetTSI(OFFLINE_DA_COMPLETION, 1);
		return FAIL;
	}

	//���IC���й�Կ
	memset(m_icpk_modules, 0x00, sizeof(m_icpk_modules));
	memset(m_icpk_exp, 0x00, sizeof(m_icpk_exp));
	m_icpk_modules_len = 0;
	
	ret = GetIcPK(&ddadata, staticdata, m_isspk_modules,
		m_isspk_exp, m_isspk_modules_len, m_icpk_modules, m_icpk_exp, &m_icpk_modules_len);
	if(ret != SUCC)
	{
		SetTVR(OFFLINE_DDA_FAIL, 1);
		SetTSI(OFFLINE_DA_COMPLETION, 1);
		return FAIL;
	}

	//��֤��̬ǩ������
	ret = VerifyDynamicData(m_icpk_modules, m_icpk_exp, m_icpk_modules_len);
	if(ret != SUCC)
	{
		SetTVR(OFFLINE_DDA_FAIL, 1);
		SetTSI(OFFLINE_DA_COMPLETION, 1);
		return ret;
	}
		
	SetTSI(OFFLINE_DA_COMPLETION, 1);
	
	return SUCC;
}


int OffAuth(char RID[5], char *staticdata)
{
	char *pAIP;
	int len;

	memset(m_RID, 0, sizeof(m_RID));
	memset(m_staticdata, 0, sizeof(m_staticdata));
	memcpy(m_RID, RID, 5);
	memcpy(m_staticdata, staticdata, staticdata[0]+1);
	
	//�ӻ�������ȡAIP
	pAIP = GetAppData("\x82", &len);
	if(pAIP == NULL)
	{
//		ERR_MSG("   AIP������!");
		return FAIL;
	}

	if(pAIP[0]&0x02 && g_termconfig.TermCap[2]&0x10)	//CDA
	{
//		ERR_MSG("�ݲ�֧��CDA");
		return FAIL;
	}
	else if(pAIP[0]&0x20 && g_termconfig.TermCap[2]&0x40)		//DDA
	{
//		SUCC_MSG("DDA");
		if(OffAuth_DDA(RID, staticdata)==QUIT)
		{
			return QUIT;
		}
		else
		{
			return SUCC;
		}
	}
	else if(pAIP[0]&0x40 && (g_termconfig.TermCap[2]&0x80 || g_termconfig.TermCap[2]&0x40)) //SDA
	{
//		SUCC_MSG("SDA");	
		OffAuth_SDA(RID, staticdata);
		return SUCC;
		
	}
	else 			//�����ѻ���֤
	{
//		SUCC_MSG("NO OFFAUTH");	
		SetTVR(OFFLINE_AUTH_UNDO, 1);
		return SUCC;
	}
	
}

int CAPKLoad(char RID[5], char Index, TCAPK *CAPK)
{
	int fp;
	TCAPK TmpCAPK;
	unsigned char keycount = 0 ;
	int i = 0;

	if ((fp = fopen(FILE_CAPK, "r")) < 0)
		return FAIL ;
	fread((char *)&keycount, 1, fp) ;

	for (i = 0; i < keycount ; i++)
	{
		if (fread((char *)&TmpCAPK, sizeof(TCAPK), fp) != sizeof(TCAPK))
		{
			break ;
		}
		if (TmpCAPK.Index == Index)
		{
			memcpy((char *)CAPK, (char *)&TmpCAPK, sizeof(TCAPK));
			fclose(fp) ;
			return SUCC ;
		}
	}
	fclose(fp);
	return FAIL ;
}


