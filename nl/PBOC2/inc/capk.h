#ifndef _CAPK_H_
#define _CAPK_H_

#define	FILE_CAPK		"capkfile.dat"

typedef struct
{
	char RID[5];				//Ӧ���ṩ�߱�ʶ
	char Index;					//��֤���Ĺ�Կ����
	char HashAlgorithm;			//��֤����HASH�㷨��ʶ(Ŀǰ�̶�Ϊ0x01)
	char PkAlgorithm;			//��Կ�㷨��ʶ(Ŀǰ�̶�Ϊ0x01)
	char Value[248];			//��Կģ			
	char Len;					//��Կģ����
	unsigned char Exponent[3];	//��֤���Ĺ�Կָ��
	char Hash[20];				//��֤���Ĺ�Կ��ֵ֤
}TCAPK;


#define		CAPKERR_BASE		(0)
#define		CAPKERR_NONE		(CAPKERR_BASE)
#define		CAPKERR_INIT		(CAPKERR_BASE-1)
#define		CAPKERR_LOAD		(CAPKERR_BASE-2)
#define		CAPKERR_SAVE		(CAPKERR_BASE-3)

extern int CAPK_Init(void);
extern int CAPK_Load(char RID[5], char Index, TCAPK *CAPK);
extern int CAPK_Save(const TCAPK *CAPK);

#endif
