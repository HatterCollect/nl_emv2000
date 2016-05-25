
#include <string.h>
#include "error.h"
#include "posapi.h"
#include "define.h"
#include "AppData.h"
#include "ic.h"
#include "tlv.h"
#include "tvrtsi.h"

#define MAX_CARDLOG_LENGTH 50
#define MAX_CARDLOGFORMAT_LEN 30

int DateValid(char * pDate)
{
	char month , day ;
	int i , year;
	const char mon[]={31,28,31,30,31,30,31,31,30,31,30,31} ;

	//�ж����������Ƿ���0-9֮��
	for (i = 0 ; i < 3 ; i ++)
	{
		if ((*(pDate + i) >> 4) > 0x09 || (*(pDate + i) & 0x0f) > 0x09)
			return FAIL ;	
	}
	//���
	year = ( *pDate  >> 4 ) * 10 + ( *pDate  & 0x0f ) ;
	year += ((*pDate > 0x50 ? 19 : 20) * 100) ;
	
	//�·�1-12
	month =( *(pDate + 1) >> 4 ) * 10 + ( *(pDate + 1) & 0x0f ) ;
	if (month > 12 || month < 1) 
		return FAIL ;		
	
	//����1-31
	day =( *(pDate + 2) >> 4 ) * 10 + ( *(pDate + 2) & 0x0f ) ;
	if (day > 31 || day < 1) 
		return FAIL ;

	if((month == 2) &&
		(((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0) ))
	{
		if (day > 29)
			return FAIL ;
	}
	else
		if (day > mon[month - 1])
			return FAIL ;
	return SUCC ;
}
/**
 * @fn ReadAppData
 * @brief ��IC��������

 * @param (out) char * StatDataAuth ����о�̬������֤������
 * @return SUCC �ɹ�
                FAIL   ʧ��
 */
int ReadAppData(char * StatDataAuth) 
{
	int i = 0 , j = 0, num, len  ;
	char sfi, frecord, lrecord, nrecord ; //һ��AFL��Ŀ��4���ֽ�
	int offset ;
	char buff[512] ;
	TTagList taglist[128] ;
	char * pAFL ;
	char *pAIP;
	//���ݱ�JR/T 0025.5--2004 ���������,�����ж����ݸ�ʽ(����)�Ƿ����
	TTagRestriction tr[] = { {"\x70",0,0},
		{"\x9F\x42",2,2}, {"\x9F\x44",1,1},{"\x9F\x05",1,32},{"\x5F\x25",3,3},
		{"\x5F\x24",3,3}, {"\x4F",5,16}, {"\x50",1,16}, {"\x9F\x12",1,16},
		{"\x5A",0,10}, {"\x5F\x34",1,1}, {"\x87",1,1}, {"\x9F\x07",2,2}, {"\x9F\x08",2,2},
		 {"\x8C",0,252}, {"\x8D",0,252},  {"\x5F\x20",2,26}, {"\x9F\x0B",1,19},
		 {"\x9F\x61",1,40}, {"\x9F\x62",1,1}, {"\x8E",0,252}, {"\x8F",1,1}, 
		 {"\x9F\x45",2,2}, {"\x9F\x49",0,252}, {"\x9F\x46",0,248}, {"\x9F\x47",1,3},
		 {"\x9F\x07",0,42}, {"\x9F\x0D",5,5}, {"\x9F\x0E",5,5}, {"\x9F\x0F",5,5},
		{"\x5F\x28",2,2}, {"\x90",0,252}, {"\x9F\x32",1,3}, {"\x92",0,36}, 
		{"\x9F\x14",1,1}, {"\x5F\x30",2,2}, {"\x93",0,248}, {"\x9F\x4A",0,0},
		{"\x9F\x1F",0,250}, {"\x57",0,19}, {"\x97",0,252}, {"\x9F\x23",1,1}
	} ;

	TLV_Init (tr) ;//����TLV ǰ��ʼ��
	pAIP = GetAppData("\x82", &len);	//�ӻ�������ȡAIP
	pAFL = GetAppData("\x94", &num) ;//��ȡӦ���ļ���λ��

	while (i < num) //һ������
	{
		sfi = ((*(pAFL + i)) >> 3) & 0x1F  ;	 //��5λ 
		frecord = *(pAFL + i + 1) ; 	 		//��sfi���Ա������ĵ�һ����¼
		lrecord = *(pAFL + i + 2) ;	 		//��sif���Ա����������һ����¼
		nrecord = *(pAFL + i + 3) ;   			//���ھ�̬��֤�Ķ��¼��
		i += 4 ; 							// 4 �ֽڱ߽�

		if ( sfi  == 0 || sfi == 31 ) //ȡֵ1-30
			return -2 ;
		if ( !frecord ) //�ڶ��ֽڲ���Ϊ0 
			return -3 ;
		if ( frecord > lrecord || nrecord > lrecord - frecord + 1 ) //��¼���ɴ�
			return -4 ;
		while (frecord <= lrecord) //ѭ��������Ŀ�����м�¼
		{
			memset(buff, 0x00, sizeof(buff)) ;
			len = IC_ReadRecord(sfi, frecord, buff); //����¼
			if (len < 0)  //����0�򷵻س���,С��0��ʧ��
				return -5 ;

			
			TLV_Init(NULL);
			if (TLV_Decode(buff, len) != TLVERR_NONE) //���벻��ʧ��
			{
				return -6 ;
			}
			//�����¼�����ݲ���70ģ��Ļ������ش���
			if ( buff[0] != 0x70 )
			{
				if (nrecord)
				{
					if(pAIP[0]&0x02)	//CDA
					{
						SetTVR(CDA_FAIL, 1);
					}
					else if(pAIP[0]&0x20)		//DDA
					{
						SetTVR(OFFLINE_DDA_FAIL, 1);
					}
					else if(pAIP[0]&0x40) //SDA
					{
						SetTVR(OFFLINE_SDA_FAIL, 1);
					}
					SetTSI(OFFLINE_DA_COMPLETION, 1);
				}
				if (sfi <= 10)
					return -7 ;
			}
			if (nrecord) //����Ǿ�̬��֤Ҫ�õ�����
			{				
				offset = 0 ;
				if (sfi >= 1 && sfi <= 10) //��¼���Ȳ������ѻ�������֤
				{
					if (TLV_GetTemplate("\x70", &offset, &len) == TLVERR_TAGNOTFOUND )
					{
						return -7 ;
					}
				}
				memcpy(StatDataAuth + 1 + StatDataAuth[0], buff + offset, len) ;
				StatDataAuth[0] += len ;
				nrecord -- ;
			}
			
			memset(taglist, 0x00, sizeof(taglist)) ; //��ʼ����ǩ�б�
			TLV_GetTagList(taglist,  &offset) ;	//��ȡ��ǩ�б�
			for (j = 0 ; j < offset ; j++)
			{

				if (taglist[j].Tag[0] & 0x20) //ģ��
					continue ;
				
				if ( !taglist[j].Restriction ) //���ݸ�ʽ����
					return -8 ;
				
				memset(buff, 0x00, 256) ;
				TLV_GetValue(taglist[j].Tag, buff, &len, 1) ;

				//һ���������ݶ������ڿ�Ƭ�г���һ��
				if ( HasAppData((char *)taglist[j].Tag) )
				{
					return -9 ;
				}
				else
					SetAppData((char *)taglist[j].Tag, buff, len) ;
			}
			frecord++ ;
		}
	}

	//�ж�CVM�б��Ƿ����
/*	len = 0;
	p = GetAppData("\x8e", &len);
	if(p != NULL && len < 10)
	{
		return FAIL;
	}
�޸ĵ�CASE CL044*/		
		
	//�ر�����ȱʧ��ʧ��
	if ( !HasAppData("\x5A") ||!HasAppData("\x8C") || !HasAppData("\x8D") ) 
	{
		return FAIL ;
	}
	if ( (pAFL =  GetAppData("\x5F\x24", NULL)) ) //�����pAFLָ��һ��,ȡӦ��ʧЧ����
		return DateValid(pAFL) ;
	else
		return FAIL ;
}

/*
int ParseCardLog(int recno, char * bufflog, int len1, char * logformat, int len2)
{
	char log[256] ;
	int i = 0, j= 0 , kk;
	int rightlen = 0;
	char * logs ;
//	logs = log ;
//Disp_deg_buf(bufflog, len1) ;
	char buff[256];
	int flag=0;

 	memset(log, 0x00, sizeof(logs)) ;

	gotoxy(0, 2);
	printf("  ���ڴ�ӡ  \n");
	printf("  ��%02d��¼", recno);
//	setprintmode(0,0);
	setprintfont(DOT8x16+(HZ12x16<<8));

	sprintf(buff, "\r\r��%d����¼:\r", recno) ;
	print(buff);
	
	while(i < len2)
	{	
		switch(logformat[i++])
		{
		case 0x9A:
			if (logformat[i] != 3)
				return -1 ;
			print("����:");
			break ;
		case 0x9C:
			if (logformat[i] != 1)
				return -2 ;
			print( "����: ") ;
			break ;
		case 0x9F:
			switch(logformat[i++])
			{
			case 0x21:
				if (logformat[i] != 3)
					return -3 ;
				print( "ʱ��:") ;
				break;
			case 0x02:
				if (logformat[i] != 6)
					return -4 ;
				print( "���:") ;
				flag=1;
				break;
			case 0x03:
				if (logformat[i] != 6)
					return -5 ;
				print("�������:") ;
				flag = 1;
				break;
			case 0x1A:
				if (logformat[i] != 2)
					return -6 ;
				print("���Ҵ���:") ;
				break;
			case 0x4E:
				if (logformat[i] > 20)
					return -7 ;
				clrscr() ;
				print( "�̻�:") ;
				break ;
			case 0x36:
				if (logformat[i] != 2)
					return -8 ;
				print("������:") ;
				break;
			default:
				return -9 ;
			}
			break ;
		case 0x5F:
			if (logformat[i++] == 0x2A)
			{
				if (logformat[i] != 2)
						return -10 ;
				print( "���Ҵ���:") ;
			}
			else
				return -11 ;
			break ;
		default:
			return FAIL ;
		}
//		logs += strlen(logs) ;
		memset(buff, 0, sizeof(buff));
		for (kk = 0 ; kk < logformat[i] ;kk++)
		{
			sprintf(buff+kk*2, "%02x", bufflog[j++]) ;
		}
		if (flag == 1)
		{
			flag = 0;
			for (kk = 0 ; kk < 10 ; kk ++ )
			{
				if (buff[kk] != 0x30)
					break ;
			}
			if (kk == 10)
				kk-- ;
			memcpy(buff, buff + kk, 12-kk) ;
			memcpy(buff+11-kk, buff+10-kk,2) ;
			buff[10-kk] = '.' ;
			buff[10-kk+3]='\0' ;
		}
		
		print(buff) ;
		print("\r");
		rightlen += logformat[i++] ;
	}
	if (i != len2 || rightlen != len1)
		return -12 ;

	print("\f");

	// �ȴ���ӡ���
	while(1)
	{
		int tpstatus;
		tpstatus=getprinterstatus();
		if(tpstatus!=TPNOTREADY)//����ӡ�����ڴ�ӡ
		{
			if(tpstatus==TPPRINTOVER)	break;
			else	return FAIL;
		}
	}
	//�ײ��ӡʱ,�û������ӳ������ӷ���������
	
	return SUCC ;
}
*/
int ParseCardLog(char * bufflog, int len1, char * logformat, int len2)
{
	char log[256] ;
	int i = 0, j= 0 , kk, col = 0;
	int rightlen = 0;
	int flag = 0 ;
//	char * logs ;
 //	memset(log, 0x00, sizeof(logs)) ;
//	logs = log ;
//Disp_deg_buf(bufflog, len1) ;
	clrscr() ;
	while(i < len2)
	{	
		switch(logformat[i++])
		{
		case 0x9A:
			if (logformat[i] != 3)
				return -1 ;
			printf("����:") ;
			break ;
		case 0x9C:
			if (logformat[i] != 1)
				return -2 ;
			printf( "����:") ;
			break ;
		case 0x9F:
			switch(logformat[i++])
			{
			case 0x21:
				if (logformat[i] != 3)
					return -3 ;
				printf( "ʱ��:") ;
				break;
			case 0x02:
				if (logformat[i] != 6)
					return -4 ;
				printf( "���:") ;
				flag = 1 ;
				break;
			case 0x03:
				if (logformat[i] != 6)
					return -5 ;
				printf("�������:") ;
				flag = 1 ;
				break;
			case 0x1A:
				if (logformat[i] != 2)
					return -6 ;
				printf("���Ҵ���:") ;
				break;
			case 0x4E:
				if (logformat[i] > 20)
					return -7 ;
				clrscr() ;
				printf( "�̻�:") ;
				break ;
			case 0x36:
				if (logformat[i] != 2)
					return -8 ;
				printf("������:") ;
				break;
			default:
				return -9 ;
			}
			break ;
		case 0x5F:
			if (logformat[i++] == 0x2A)
			{
				if (logformat[i] != 2)
						return -10 ;
				printf( "���Ҵ���:") ;
			}
			else
				return -11 ;
			break ;
		default:
			return FAIL ;
		}
//		logs += strlen(logs) ;
/*		for (kk = 0 ; kk < logformat[i] ;kk++)
		{
			printf("%02x", bufflog[j++]) ;
		}
		printf("\n") ;
*/
		memset(log, 0, sizeof(log));
		for (kk = 0 ; kk < logformat[i] ;kk++)
		{
			sprintf(log+kk*2, "%02x", bufflog[j++]) ;
		}
		if (flag == 1)
		{
			flag = 0;
			for (kk = 0 ; kk < 10 ; kk ++ )
			{
				if (log[kk] != 0x30)
					break ;
			}
			if (kk == 10)
				kk-- ;
			memcpy(log, log + kk, 12-kk) ;
			memcpy(log+11-kk, log+10-kk,2) ;
			log[10-kk] = '.' ;
			log[10-kk+3]='\0' ;
		}
		
		printf(log) ;
		printf("\n") ;
		col++ ;
		if (col == 2)
		{
			getkeycode(0) ;
			clrscr() ;
			col=0 ;
		}
		rightlen += logformat[i++] ;
	}
	if (i != len2 || rightlen != len1)
		return -12 ;
/*	clrscr() ;
	logs = log ;
	printf("%-64.64s", logs) ;
	getkeycode(0) ;
	clrscr() ;
	logs += 64 ;
	printf("%-64.64s", logs) ;
	getkeycode(0) ;*/
	if (col != 0)
		getkeycode(0) ;
	return SUCC ;
}

int ReadCardLog(void)
{
	char * pData ;
	int i = 0 ;
	int len1 = 0, len2 =0;
	char buff[MAX_CARDLOG_LENGTH] ;
	char cformat[MAX_CARDLOGFORMAT_LEN] ;
	char SW1, SW2;
	int ret ;
	//��������־��ڣ����ֽڣ�SFI�ͼ�¼����
	if ((pData = GetAppData("\x9F\x4D",NULL)) == NULL )
		return FAIL ;
/*
	while(1)
	{
		clrscr() ;
		printf("��ʾ������־?\n") ;
		printf("Enter--��\nESC  --��\n") ;
		i = getkeycode(0) ;
		if (i == ESC)
			return FAIL;
		if (i == ENTER)
			break ;
	}
*/
	if ( (len1 = IC_GetData(GETDATAMODE_LOGFORMAT, cformat)) < 0)
		return FAIL ;

	for (i = 1 ; i <= pData[1] ; i++)
	{
		len2 = IC_ReadRecord(pData[0], i, buff) ;
		if (len2 < 0)
		{
			IC_GetLastSW(&SW1, &SW2) ;
			if (SW1 == 0x6A && SW2 == 0x83)
				break ;
			return FAIL ;
		}
		clrscr() ;
		printf("\n  ��%d����¼:\n", i) ;
		getkeycode(1) ;
		if ((ret = ParseCardLog(buff, len2, cformat+3, len1-3))< 0)
			{
				clrscr() ; printf("��¼����ʧ��%d", ret) ; getkeycode(0) ;
		}
	}
	clrscr() ;
	printf("\n  ��¼��ʾ����") ;
	printf("\n  ��ο�") ;
	getkeycode(0) ;
	return SUCC ;
}
