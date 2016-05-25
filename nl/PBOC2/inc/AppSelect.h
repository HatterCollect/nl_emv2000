#ifndef _APPSELECT_H_
#define _APPSELECT_H_

#define TermAIDFile		"termaid.dat"
#define MAXAIDLISTNUM	10

typedef struct
{
	char	Enable;
	char	AID[16];			//AID(Ӧ�ñ�ʶ��)
	char	AIDLen;				//AID����
	char	AppLabel[16];		//Ӧ�ñ�ǩ
	char	AppLabelLen;		//Ӧ�ñ�ǩ����
	char	AppPreferedName[16];	//Ӧ����������
	char	AppPreferedNameLen;	//Ӧ���������Ƴ���
	char	AppPriorityID;	//Ӧ������Ȩ��ʶ��
}TAIDList;
#define  AIDLISTLEN	53

//#define USEAIDLIST -2000
extern int CreateAppList(char AllowPartialAID, TAIDList oAIDList[], int *oAIDNum) ;
extern int GetTermAidList(void);

#endif
