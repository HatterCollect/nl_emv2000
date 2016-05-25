

#include "posapi.h"
#include "posdef.h"
#include <string.h>
#include "ic.h"
#include "tvrtsi.h"
#include "AppData.h"
#include "DOLProcess.h"
#include "OnlineProc.h"
#include "Completion.h"
#include "issuerscript.h"
#include "tools.h"

#define LOGFILE "proc.log"
#define LOGLONG 64

extern int AnalyzeACData(char * response, int len, int NeedCDA) ;
extern int Online(int iTransType, char * pPIN) ;

void _PreCompletion(TCompletion * pCompletion, int TransR)
{
	char * pData ;

	pData = GetAppData("\x9f\x27", NULL) ;

	pCompletion->IAC_Default = GetAppData("\x9f\x0d",NULL) ;
	pCompletion->TAC_Default = g_termparam.TAC_Default ;
	pCompletion->pAutheResCode = GetAppData("\x8A", NULL) ;
	pCompletion->iCDAFail = 0 ;
	pCompletion->TransProcR = TransR ;
	GetTVR(pCompletion->TVR) ;
	pCompletion->iNeedCDA = 0 ;

	
}

TACType TermJudge(TCompletion * pCompletion)
{
	int i ;
	int len ;
	char * p ;
	char * pAuth = GetAppData("\x8A", NULL) ;
	switch(pCompletion->TransProcR)
	{
	case ONLINE_FAIL:
		for (i = 0 ; i < 5 ; i ++)
		{
			if ((pCompletion->IAC_Default[i] & pCompletion->TVR[i]) ||
				(pCompletion->TAC_Default[i] & pCompletion->TVR[i]))
				{
					memcpy(pAuth, "\x5A\x33",2) ;
					return ACTYPE_AAC ;
				}
		}
		if (i == 5)
		{
			memcpy(pAuth, "\x59\x33",2) ;
			return ACTYPE_TC ;
		}
		break ;
	case ONLINE_SUCC :
		if( (!memcmp(pCompletion->pAutheResCode, "\x30\x30",2)) ||
			(!memcmp(pCompletion->pAutheResCode, "\x31\x30",2)) ||
			(!memcmp(pCompletion->pAutheResCode, "\x31\x31",2)))
		{
			return ACTYPE_TC ;
		}
		else if ( (!memcmp(pCompletion->pAutheResCode, "\x30\x31",2)) ||
			(!memcmp(pCompletion->pAutheResCode, "\x30\x32",2)))
		{
			while(1)
			{
				clrscr() ;
				printf("����ϵ�������:\n") ;
				p = GetAppData("\x5A", &len) ;
				printf("---Ӧ�����ʺ�---\n") ;
				for (i = 0 ; i < len ; i++)
					printf("%02x", p[i]) ;
				getkeycode(0) ;
				clrscr() ;
				printf("��ѡ��:\n") ;
				printf("1.�ѻ���׼\n") ;
				printf("2.�ѻ��ܾ�\n") ;
				i = getkeycode(0) ;
				if (i == 0x31)
				{
					return  ACTYPE_TC ;
				}
				else if(i == 0x32)
				{
					return ACTYPE_AAC ;
				}
			}
		}
		else if (!memcmp(pCompletion->pAutheResCode, "\x30\x35",2))
		{
			return ACTYPE_AAC ;
		}
		else
		{
			pCompletion->TransProcR = ONLINE_FAIL ;
			i = TermJudge(pCompletion) ;
			Online(5, NULL) ;
			pCompletion->TransProcR = ONLINE_SUCC ;
			return (TACType)i ;
		}
	case AAR_OFFLINE_APPROVE :
		memcpy(pAuth, "\x59\x32", 2) ;
		return ACTYPE_TC ;
	case AAR_OFFLINE_REFUSE :
		memcpy(pAuth, "\x5A\x32", 2) ;
		return ACTYPE_AAC ;
	default:
		break ;
	}
	return ACTYPE_AAC ;
}
int _Completion(TCompletion * pCompletion)
{
	int i ;
	TACType iACType ;
	char dolvalue[256] ;
	char resp[256] ;
	char *p;
	char * pAuthRes ;

	//��һ��Generate AC���ؽ������
	pAuthRes = GetAppData("\x8A", NULL) ;
	//�ѻ�
	if (pCompletion->TransProcR == ACTYPE_TC) 
	{	
		if (pCompletion->iCDAFail)
		{
			memcpy(pAuthRes, "\x5A\x31",2) ;
			return REFUSE_OFFLINE;
		}
		else//TC��CDA��ִ�л�ɹ�
		{
			memcpy(pAuthRes, "\x59\x31",2) ;
			return APROVE_OFFLINE;
		}
	}
	
	if (pCompletion->TransProcR == ACTYPE_AAC)//AAC
	{
		memcpy(pAuthRes, "\x5A\x31",2) ;
		return REFUSE_OFFLINE ;
	}
	

	//�ű�����1
	if (pCompletion->TransProcR == ONLINE_SUCC)
	{
		clrscr() ;
		printf("\n�ű�����1...\n") ;
		msdelay(100);
		IssuerScript(FALSE) ;
	}

	if (pCompletion->iCDAFail) //CDA ʧ��
	{
		return REFUSE_OFFLINE ;
	}
	else
	{
		iACType = TermJudge(pCompletion);
	}

	memset(dolvalue, 0x00, sizeof(dolvalue)) ;
	DOLProcess(CDOL2, dolvalue);

	memset(resp, 0x00, sizeof(resp)) ;
	i = IC_GenerateAC(iACType, pCompletion->iNeedCDA, dolvalue + 1, dolvalue[0], resp);

	if (AnalyzeACData(resp, i, pCompletion->iNeedCDA) == FAIL)
		return FAIL ;

	//�Ų�����2
	if (pCompletion->TransProcR == ONLINE_SUCC)
	{
		clrscr() ;
		printf("\n�ű�����2...\n") ;
		msdelay(100);
		IssuerScript(TRUE) ;
	}
	
	p = GetAppData("\x9f\x27", NULL);
	
	if ( (p[0] & 0xC0) != 0x40 || iACType == ACTYPE_AAC)
	{
		if (iACType == ACTYPE_TC && pCompletion->TransProcR == ONLINE_SUCC)
			Online(5, NULL) ;
		return REFUSE_OFFLINE ;
	}
	
	if (pCompletion ->iNeedCDA)
	{
		return REFUSE_OFFLINE ;
		//ִ��CDA
	}
	else
		return APROVE_OFFLINE ;
}

int SaveLog(char iForceAccept)
{
	int fp ;
	char buff[256] ;
	char * p ;
	int len ;

	if ((fp = fopen(LOGFILE, "w")) == FAIL)
	{
		clrscr() ;
		printf("�޷��򿪽�����־�ļ�") ;
		return FAIL ;
	}
	memset(buff, 0x00, sizeof(buff)) ;
	fseek(fp, 0L, SEEK_END) ;

	p = GetAppData("\x5A", &len) ;
	buff[0] = len ;
	memcpy(buff + 1, p, len) ;

	if (p = GetAppData("\x9F\x36", NULL)) ;
		memcpy(buff + 11, p, 2) ;

	if (p = GetAppData("\x9F\x27", NULL)) ;
		buff[13] = *p ;

	if (p= GetAppData("\x9f\x02", NULL) )
		memcpy(buff + 14, p, 6) ;
	
	if (p= GetAppData("\x9f\x03", NULL) )
		memcpy(buff + 20, p, 6) ;
	
	GetTVR(buff + 26) ;
	GetTSI(buff + 31) ;

	if (p = GetAppData("\x8A", NULL) )
		memcpy(buff  + 34, p, 2) ;

	if (p = GetAppData("\x9A", NULL) )
		memcpy(buff + 36, p, 3) ;
	
	if (p = GetAppData("\x9f\x21", NULL) )
		memcpy(buff + 39, p, 3) ;

	buff[42] = ScriptNum * 5 ;
	memcpy(buff + 43, _ISResult, buff[42]) ;

	buff[63] = iForceAccept ;
	if (fwrite(buff, LOGLONG, fp) == FAIL)
	{
		clrscr() ;
		printf("��־д�����") ;
		fclose(fp) ;
		return FAIL ;
	}
	len = ftell(fp) ;
	len /=  LOGLONG ;
	fclose(fp) ;
	return len ;
}

void ParseLog(char * buf)
{
	char * p ;
	SetAppData("\x5A", buf + 1, buf[0]) ;
	SetAppData("\x9F\x36", buf + 11, 2) ;
	SetAppData("\x9F\x27", buf + 13, 1) ;
	SetAppData("\x9f\x02",  buf + 14, 6) ;
	SetAppData("\x9f\x03", buf + 20, 6) ;

	p = GetAppData("\x95", NULL) ;
	memcpy(p, buf + 26, 5) ;
	p = GetAppData("\x9b", NULL) ;
	memcpy(p, buf + 31, 3) ;

	SetAppData("\x8A", buf + 34, 2);
	SetAppData("\x9A", buf + 36, 3) ;
	SetAppData("\x9f\x21", buf + 39, 3) ;

	ScriptNum = buf[42] / 5 ;
	memcpy(_ISResult, buf+43, buf[42]);

	iAcceptForced = buf[63] ;
}

int GetLog(int i, char * buf)
{
	int fp ;
	int len ;
	
	if ((fp = fopen(LOGFILE, "r")) == FAIL)
		return FAIL ;

	fseek(fp, 0L, SEEK_END) ;
	len = ftell(fp) ;
	len /= LOGLONG ;

	if (i >= len)
		return FAIL ;
	
	fseek(fp, i * LOGLONG, SEEK_SET) ;
	fread(buf, LOGLONG, fp) ;
	fclose(fp) ;
	return SUCC ;
}

int sign(void)
{
	char *p;
	char print_buf[64];//���Ĵ�ӡ��Ϣ����û�����
	int tpstatus;
	char * pData ;
	int len, i ;

	while(1)
	{
		clrscr() ;
		printf("_____��ӡ_____\n\n���Ժ�...");
		//�ײ�����
		clrprintbuf();
		delay(10);
		
/*
		setprintmode(0,0);
		setprintfont(DOT8x16+(HZ12x16<<8));
*/

		setprintfont(DOT16x16 + (HZ32x16 << 8));		


		print("\r");

		memset(print_buf,0,sizeof(print_buf));
		sprintf(print_buf,"     NL8300 PBOC ����\r\r");
		print(print_buf);

		memset(print_buf,0,sizeof(print_buf));

		pData = GetAppData("\x5A", &len) ;
		sprintf(print_buf, "\r���˺�:") ;
		print(print_buf) ;
		p=print_buf ;
		for (i = 0 ;i < len; i++)
		{
			sprintf(p, "%02x", pData[i]) ;
			p += 2 ;
		}
		*p = 0x00 ;
		print(print_buf);

		memset(print_buf,0,sizeof(print_buf));
		pData = GetAppData("\x4F", &len) ;
		sprintf(print_buf, "\rӦ�ñ�ʶ:") ;
		print(print_buf);
		p=print_buf ;
		for (i = 0 ;i < len; i++)
		{
			sprintf(p, "%02x", pData[i]) ;
			if (p[0] >= 'a' && p[0] <= 'z')
				p[0] = p[0] - 0x20 ;
			if (p[1] >= 'a' && p[1] <= 'z')
				p[1] = p[1] - 0x20 ;
			p += 2 ;
		}
		*p = 0x00 ;
		print(print_buf);
		
		memset(print_buf,0,sizeof(print_buf));
		pData = GetAppData("\x9A", &len) ;
		sprintf(print_buf,
				"\r����: %02x%02x/%02x/%02x",
				pData[0] > 50? 0x19:0x20 ,pData[0],pData[1],pData[2]); 
		print(print_buf);

		memset(print_buf,0,sizeof(print_buf));
		pData = GetAppData("\x9F\x21", &len) ;
		sprintf(print_buf,
				"\rʱ��: %02x:%02x:%02x",
				 pData[0],pData[1],pData[2]); 
		print(print_buf);
		
		memset(print_buf,0,sizeof(print_buf));
		pData = GetAppData("\x9F\x36", NULL) ;
		sprintf(print_buf,"\r�������:%02x%02x",pData[0],pData[1]);
		print(print_buf);

		memset(print_buf,0,sizeof(print_buf));
		p = GetAppData("\x81",NULL) ;
		C4ToInt((unsigned int *)&len, (unsigned char *)p) ;
		sprintf(print_buf,"\r��Ȩ���: %10d.%02dԪ",
			len/100,len%100);
		print(print_buf) ;

		memset(print_buf,0,sizeof(print_buf));
		getvar((char * )&i, TSC_OFF, TSC_LEN) ;
		sprintf(print_buf, "\r�ն˼�����:%d", i) ;
		print(print_buf) ;
		
		if(iNeedSignature == 1)
		{
			memset(print_buf,0,sizeof(print_buf));
			sprintf(print_buf, "\r\r\r�ֿ���ǩ��: _____________\r\r\r" ) ;
			print(print_buf); 
		}
		
		memset(print_buf,0,sizeof(print_buf));
		p = GetAppData("\x95", NULL) ;
		sprintf(print_buf,"\r\rTVR:%02x %02x %02x %02x %02x",p[0],p[1],p[2],p[3],p[4]) ;
		print(print_buf) ;
		
		memset(print_buf,0,sizeof(print_buf));
		p = GetAppData("\x9B", NULL) ;
		sprintf(print_buf,"\rTSI:%02x %02x ",p[0],p[1]) ;
		print(print_buf) ;

		for(i = 0 ; i < ScriptNum ; i++)
		{
			memset(print_buf,0,sizeof(print_buf));
			sprintf(print_buf, "\r�ű�%d:%02x%02x%02x%02x%02x",
				i+1, _ISResult[5*i],_ISResult[5*i+1],_ISResult[5*i+2],
				_ISResult[5*i+3],_ISResult[5*i+4]) ;
			print(print_buf) ;
		}

		
		//֪ͨ�ײ㿪ʼ��ӡ
		kbhit();
		tpstatus=print("\f");
		if(tpstatus==TPNOPAPER )	continue;
		else if(tpstatus==TPQUIT)	return QUIT;
		// �ȴ���ӡ���
		while(1)
		{
			tpstatus=getprinterstatus();
			if(tpstatus!=TPNOTREADY)//����ӡ�����ڴ�ӡ
			{
				if(tpstatus==TPPRINTOVER)	break;
				else	return FAIL;
			}
		}
		//�ײ��ӡʱ,�û������ӳ������ӷ���������
		break;
	}
	return SUCC;
}
void Completion(TCompletion * pCompletion)
{
	int ret ;
	int state ;
	int len ;
	char iForcedAccept = 0 ;
	
	ret = _Completion( pCompletion) ;
	IC_PowerDown() ;
	clrscr();
	if (ret == FAIL)
	{
		printf("\n�������׹��̳���\n������ֹ���ɰο�") ;
		getkeycode(1) ;
	}
	else
	{
		if (ret == REFUSE_OFFLINE && g_termparam.cAllowForceAccept)
		{
			clrscr() ;
			printf("���ױ��ܾ����Ƿ�ǿ�Ƚ��ܽ���?\n") ;
			printf("1.�ǣ�2.��") ;
			while(1)
			{
				state = getkeycode(0) - 0x30;
				if (state == 1 || state == 2)
					break ;
			}
			if (state == 1)
			{
				ret = APROVE_OFFLINE ;
				iForcedAccept = 1 ;
			}
		}
		
		if (ret == APROVE_OFFLINE)
			len = SaveLog(iForcedAccept) ;
/*		
		if (pCompletion->TransProcR == ONLINE_SUCC)
		{	
			clrscr() ;
			printf(" ��%d�����׼�¼\n", len) ;
			printf("  �����ݲɼ�...") ;
			for (state = 0 ; state < len ; state++)
			{
				GetLog(state, buff) ;
				ParseLog(buff);
				Online(3, NULL) ;
			}
			fdel(LOGFILE) ;
		}
*/
		if (ret == APROVE_OFFLINE)
		{
			printf("\n  ��׼����\n���Ƴ�IC��") ;
			getkeycode(1) ;
			sign() ;
		}
		else 
		{
			printf("\n  �ܾ�����\n���Ƴ�IC��") ;
			getkeycode(1) ;
		}

	}
}
