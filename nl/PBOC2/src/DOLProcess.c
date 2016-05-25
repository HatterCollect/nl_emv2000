/**
 * @file DOLProcess.c
 * @brief ���� DOL

 * @version Version 1.0
 * @author Ҷ��ͳ
 * @date 2005-8-25
 */
#include <string.h>
#include "posapi.h"
#include "posdef.h"
#include "AppData.h"
#include "tvrtsi.h"
#include "DolProcess.h"
#include "rsa.h"


#define N 	0x01
#define CN 	0x02
#define O 	0x04
/**
 * @fn DOLProcess
 * @brief ��DOL(��ǩ�ͳ����б�)���д���

 * @param (in) char * DOLStyle Ҫ�����DOL����
 * @param (out) char * DOL_Value ����DOL��Ӧ������Ԫֵ
 * @return SUCC  �ɹ�
 		  FAIL    ʧ��
 */
int DOLProcess(char * DOLStyle, char * DOL_Value)
{
	int len , nFormat, dollen ;
	char * ptr , *pData ;
	int i = 0 , offset = 0 ;
	char tmp[256] ;
	int flag9f37 = FALSE ;
	int iTVRPos = -1 ;
	int iTVRLenReq = 0 ;

	ptr = GetAppData(DOLStyle,  &len) ;
	if (!ptr)
	{
		if (*DOLStyle == 0x9F && *(DOLStyle + 1) == 0x38) //PDOL
		{
			DOL_Value[0] = 0x02 ;
			DOL_Value[1] = 0x83 ;
			DOL_Value[2] = 0x00 ;
			return SUCC ;
		}
		if (*DOLStyle == 0x8C) //CDOL1
			return FAIL ;			
	}

	memset(tmp, 0x00, 256) ;
	while (i < len)
	{
		//��ȫ�ֻ�����������
		pData = GetAppData (ptr + i, &dollen ) ; 
		if (*(ptr + i) & 0x20)
			pData =NULL ;

		//�������ݵĸ�ʽ�Ƿ�Ϊn��cn��others,����ǩ����
		switch (*(ptr + i))
		{
		case 0x9F:
			i += 1 ; // 2 �ֽڵı�ǩ
			switch (*(ptr + i))
			{
			case 0x02:
			case 0x03:
			case 0x15:
			case 0x11:
			case 0x1A:
			case 0x42:
			case 0x44:
			case 0x51:
			case 0x54:
			case 0x5C:
			case 0x73:
			case 0x75:
			case 0x57:
			case 0x76:
			case 0x39:
			case 0x35:
			case 0x3C:
			case 0x3D:
			case 0x41:
			case 0x21:
				nFormat = N ;
				break ;
			case 0x62:
			case 0x20:
				nFormat = CN ;
				break ;
			case 0x37:
				nFormat = O ;
				flag9f37 = TRUE ;
				break ;
			default:
				nFormat = O ;
				break ;
			}
			break ;
		case 0x5F:
			i += 1 ;
			switch (*(ptr + i))
			{
			case 0x25:
			case 0x24:
			case 0x34:
			case 0x28:
			case 0x2A:
			case 0x36:
				nFormat = N ;
				break ;
			default:
				nFormat = O ;
				break ;
			}
			break ;
		case 0x9A:
		case 0x9C:
			nFormat = N;
			break;
		case 0x5A:
			nFormat = CN ;
			break ;
		case 0x98:
			if (!HasAppData("\x97"))
			{
				if (g_termparam.DefaultTDOLen != 0)
				{
					SetTVR(USE_DEFAULT_TDOL, 1) ;
					if (offset >= 0)
						memcpy(tmp + iTVRPos, GetAppData("\x95", NULL), iTVRLenReq) ;
				}
			}
			nFormat = O ;
			break ;
		case 0x95:
			iTVRPos = offset ;
			iTVRLenReq = *(ptr + i + 1) > 5 ? 5 : *(ptr + i + 1) ;
		default:
			nFormat = O ;
			break ;
		}

		//���ݲ�����,OK, ��0x00
		if (pData == NULL ) 
		{ 
			memset(tmp + offset, 0x00, *(ptr + i +1)) ;
		}
		else 
		{
			//�������ݸ�ʽ����Ӧ�����
			if (dollen < *(ptr + i + 1)) //�������Ԫ��ʵ�ʳ���С��Ҫ��ĳ���
			{
				if (nFormat & N)	//�Ҷ���,��0
					memcpy(tmp + offset + *(ptr + i + 1) - dollen, pData, dollen) ;
				else
					memcpy(tmp + offset, pData, dollen) ;
				if (nFormat & CN)  //�����,�Ҳ�F
					memset(tmp + offset + dollen, 0xFF, *(ptr + i + 1) - dollen) ;
			}
			else //����Ԫ��ʵ�ʳ��ȴ��ڻ����Ҫ��ĳ���,��Ҫ��ȡ
			{
				if (nFormat & N) //������˿�ʼ����
					memcpy(tmp + offset, pData + dollen - *(ptr + i + 1), *(ptr + i + 1) ) ;
				else
					memcpy(tmp + offset, pData, *(ptr + i + 1)) ;
			}
		}
		offset += *(ptr + i + 1) ;
		i += 2 ;
	}

	//��װDOL��ֵ
	if (*DOLStyle == 0x9F && *(DOLStyle + 1) == 0x38) //PDOL
	{
			DOL_Value[0] = offset + 2 ;
			DOL_Value[1] = 0x83 ;
			DOL_Value[2] = offset  ;
			memcpy(DOL_Value + 3, tmp, offset) ;
			return SUCC ;
	}
	if (*DOLStyle == 0x9F  && *(DOLStyle + 1) == 0x49) //DDOL
	{
		if (!flag9f37) //�����������
			return FAIL ;
	}
	DOL_Value[0] = offset ;
	memcpy(DOL_Value + 1, tmp, offset) ;
	return SUCC ;
}
