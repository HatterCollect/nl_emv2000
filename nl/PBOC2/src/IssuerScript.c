#include <string.h>
#include "posapi.h"
#include "posdef.h"
#include "tvrtsi.h"
#include "ic.h"
#include "AppData.h"
#include "tools.h"

#define MAX_CMD 6	//PBOC���/���ǹ淶֧�ֵĽű�����,6��


typedef struct
{
	char * pCmd ;
	int iCmdLen ;
}ScriptCmd ;

static ScriptCmd _ScriptCmd[MAX_CMD] ;
char  _ISResult[5 * MAX_SCRIPT]  ;
char ScriptNum ;

int ParseScript(char * buff, int len, char * ISID)
{
	int  tag86len = 0, i;
	int k = 0 ;
	char * p = buff ;
	
	memset(_ScriptCmd, 0, sizeof(_ScriptCmd));
	
	//�ű���ʶ
	if (*buff  == 0x9f)
	{
		if (*(buff+1) != 0x18)
			return FAIL ;
		memcpy(ISID, buff + 3 , 4)  ;
		p += 7 ;  //ָ��86���Ŀ�ʼ
		tag86len = len - 7 ; //86���ĳ���
	}
	else
	{
		tag86len =len ; //86���ĳ���
	}

	if (len > 128)
	{
		ERR_MSG("�ű����ȳ���֧�ֵĳ���") ;
		return FAIL ;
	}
	
	//����86����
	i = 0 ;
	k = 0 ; 
	while ( i < tag86len )
	{
		if (*(p + i) != 0x86)
		{
			return FAIL ; //����ʧ��
		}
		if (*(p + i + 1) < 4)	//APDU����4���ֽ�
		{
			return FAIL ;
		}
		switch (p[i + 3])
		{
		case 0x1E:
		case 0x18:
		case 0x16:
		case 0x24:
			if (p[i + 2] == 0x84)
				break ;
			else
				return FAIL ; //�淶��֧�ֵĽű������������ִ�У�����ʧ��
		case 0xDA:
		case 0xDC:	
			if (p[i + 2] == 0x04)
				break ;
			else
				return FAIL ;
		}
		_ScriptCmd[k].pCmd = p + i + 2 ;
		_ScriptCmd[k].iCmdLen = p[i + 1] ;
		i += (p[i + 1] + 2) ;
		k++;
	}

	if (i != tag86len)
	{
		return FAIL ; //���Ƚ�������
	}
	
	return SUCC;
}

//int iTime ִ��ʱ�� TRUE--�ڶ���Generate AC֮��ִ��
//					 FALSE--�ڶ���Generate AC֮ǰִ��
int  _IssuerScript(char * pData , int len, char * ISResult)
{
	int ret ;
	int port = IC_GetPort() ;
	int retlen ;
	char resp[256] ;
	int i;

	SetTSI(SCRIPT_PROC_COMPLETION, 1) ;

	if (ParseScript(pData, len, ISResult + 1) == FAIL)
	{
		ERR_MSG("  �ű���������");
		return FAIL;
	}

	for (i = 0 ; i <= MAX_CMD; i ++) 
	{
		if(_ScriptCmd[i].iCmdLen == 0)
		{
			break;
		}	
		memset(resp, 0x00, sizeof(resp)) ;
		ret = iccrw_new(port , _ScriptCmd[i].iCmdLen, 
			_ScriptCmd[i].pCmd, &retlen, resp ) ;
		
/*		clrscr() ;
		printf("ִ�н��= %d\n", ret) ;
		printf("SW:%02x%02x", resp[retlen-2] , resp[ret-1]) ;
		getkeycode(0) ;
*/
		if( (ret == 0) && (resp[retlen -2] == 0x90 ||resp[retlen -2] == 0x62 ||
			resp[retlen -2] == 0x63))
		{
				continue ; //������һ������
		}
		ISResult[0] = (i+1) > 15 ? 15 : i+1;
		ISResult[0] |= 0x10 ;
		return FAIL;
	}
	ISResult[0]  = 0x20 ;
	return SUCC ;
}

void IssuerScript(int iTime)
{
	char * pData ;
	int len ;
	int ret ;
	char buff[2] ;
	
	if(iTime)	//�ڶ���Generate AC֮��	
		buff[0] = 0x7F ;
	else //�ڶ���Generate AC֮ǰ
		buff[0] = 0x3F ;
	buff[1] = 0x01 ;

	while((pData = GetAppData(buff, &len)) != NULL)
	{
		ret = _IssuerScript(pData, len, _ISResult + ScriptNum * 5) ;
		if (ret == FAIL)
		{
			if (iTime)
				SetTVR(SCRIPT_FAIL_AFTER_LAST_GEN_AC, 1) ;
			else
				SetTVR(SCRIPT_FAIL_BEFORE_LAST_GEN_AC, 1) ;
		}
		ScriptNum ++ ;
		buff[1] ++ ;
	}

	
}
