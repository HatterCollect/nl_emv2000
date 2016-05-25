#ifndef _POSDEF_H_
#define _POSDEF_H_

#include "error.h"

#define TermParaFile	"termpara.dat"
#define TermConfigFile	"termconf.dat"
#define BlackListFile	"black.dat"		//��������
#define	BlackCertFile	"blkcert.dat"	//������֤�������

//���������ṹ
typedef struct
{
	char len;			//ƥ�䳤��
	char cardno[10];	//����
	char rfu;
}TBlackCard;

//������֤�������
typedef struct
{
	char RID[5];		
	char index;			//��Կ����
	char serialnum[3];	//֤�����к�
	char rfu[3];		
}TBlackCert;

//AID�ṹ
typedef struct
{
	char len;			//AID����
	char aid[16];		//Ӧ�ñ�ʶ
	char name[32];		//Ӧ������
	char priority;		//���ȼ�
}TAID;

//�ն����ã���POS�·�ʱһ�������ã��������޸�
typedef __packed struct
{
	char TermCap[3] ;			//�ն����� '9F33'
	char TermCapAppend[5] ;		//�ն˸������� '9F40'  
	char IFDSerialNum[9] ;		//IFD���к� '9F1E', '\0'��β
	char TermCountryCode[2] ;	//�ն˹��Ҵ��� '9F1A'
	char TermID[9] ;			//�ն˱�ʶ '9F1C'��'\0'��β
	char TermType ;				//�ն����� '9F35'
	char TransCurrCode[2] ;		//���׻��� '5F2A'
	char TransCurrExp;			//���׻���ָ�� '5F36'
}TTermConfig ;

//�ն˲����������޸�
typedef __packed struct
{
	char TAC_Default[5] ;	 	//�ն���Ϊ����-ȱʡ 
	char TAC_Decline[5] ;	 	//�ն���Ϊ����-�ܾ�
	char TAC_Online[5] ;	 	//�ն���Ϊ����-����

	char FloorLimit[4];			//�ն�����޶� '9F1B'
	char TargetPercent;			//���ѡ��Ŀ��ٷ���  
	char MaxTargetPercent;		//ƫ��ѡ������Ŀ��ٷ���
	char ThresholdValue[4];		//ƫ�����ѡ��ֵ
	
	char MerchantTypeCode[2] ; 	// �̻������� '9F15'
	char MerchantID[16] ;	   	// �̻���ʶ '9F16'��'\0'��β

	char DefaultDDOL[128] ; 	//ȱʡDDOL
	char DefaultDDOLen ;
	char DefaultTDOL[128] ; 	//ȱʡTDOL
	char DefaultTDOLen ;
	char PartAIDFlag;			//����AIDѡ���־(TRUE/FALSE)
	char AcqID[6];				//�յ��б�ʶ '9F01'
	char AcqLen;				//�յ��б�ʶ����

	char AppVersion[2] ;		//Ӧ�ð汾�� '9F09'
	char PosEntry ;				//���۵����뷽ʽ '9F39'
	char TranRefCurrCode[2] ;	//���ײο����Ҵ��� '9F3C'
	char TranRefCurrExp ;		//���ײο�����ָ�� '9F3D'

	char cAllowForceOnline ;
	char cAllowForceAccept ;
}TTermParam ;

typedef struct
{
	//��ǩ'8E',�ֿ�����֤�����б�
	int CVMx ;		//X�����
	int CVMy ;		//Y�����
	char * CVRList ; 	//�ֿ�����֤�����б�
	char nCVRCount ;  //�ֿ�����֤�����б� �ֽ���
	//Ӧ�ý������� '82'
	char * pAIP ;
	//Ӧ�û��Ҵ��� '9F42'
	char * pAppCurrCode ;	
	//���׻��Ҵ��� '5F2A'
	char * pTransCurrCode ;	
	//�ֿ���֤������ '9f62'
	char CardIDType ;
	//�ֿ���֤���� '9F61'
	char * CardID ;
	char CardIDLen ;
	int  iAmount ; //���׽��
	//�Ƿ��ֽ���(ATM0x01, ����ֵ���ֽ�0x02,����0x04)
	char IsCash ;	
	char cvmcap ;	//CVM ���� '9F33'
}TCVML ;

typedef struct
{ 
	char nCanOnline ;
	char nNeedCDA ;
	char * TAC_Default ;
	char * TAC_Decline ;
	char * TAC_Online ;

	char * IAC_Default ;
	char * IAC_Decline ;
	char * IAC_Online ;
}TTermActAna ;

typedef struct
{
	char * pPAN ;
	char * Lower_COL ;		//�����ѻ���������
	char * Upper_COL ;		//�����ѻ���������
	char * pAIP ;
	char * FloorLimt ;
	char * ThresholdValue ;
	char    TargetPercent;			
	char    MaxTargetPercent;	
}TTermRisk ;

typedef struct 
{
	char iCanOnline ;
	char iNeedCDA ;
	char iTransType ;

	char * E_PIN ;
}TTransProc ;

typedef struct
{
	char * IAC_Default ;
	char * TAC_Default ;
	char * pAutheResCode ;
	char  TransProcR ;
	char  iCDAFail ;
	char  iNeedCDA ;
	char TVR[5] ;
}TCompletion ;

#define DEBUGMODE				//����ģʽ

extern TTermConfig g_termconfig;		//�ն�����
extern TTermParam  g_termparam;		//�ն˲���
extern int iNeedSignature ;

#define TSC_OFF 0
#define TSC_LEN 4
#define TERM_PARAMNUM_OFF 4 
#define TERM_PARAMNUM_LEN 1
#define USEAIDLIST -9999

#define MAX_SCRIPT 4
extern char  _ISResult[5 * MAX_SCRIPT]  ;
extern char ScriptNum ;
extern char iAcceptForced ;

#endif
