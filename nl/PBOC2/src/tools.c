//************************************************************************************

/**
*	@file tools.c
*	@brief ���ߺ���ģ��
*
*	��������ͨ�ú���
*	@version 1.0
*	@autor liuzq
*	@date 2005/03/15
*	@li ltrim
	@li	rtrim
	@li alltrim
    @li ReadLine
	@li WriteLine
	@li ReadItemFromINI
	@li Beep
	@li Beep_Msg
	@li CalcLRC
	@li ERR_MSG
	@li MSG
	@li SUCC_MSG
    @li AsciiToBcd
    @li BcdToAscii
    @li IntToC4
    @li IntToC2
    @li C4ToInt
    @li C2ToInt
    @li ByteToBcd
    @li BcdToByte
    @li IntToBcd
    @li BcdToInt
    @li CalcLRC
    @li SetTime
    @li GetTIme
	@li	DispXY
	@li	Disp_deg_buf
*/

//************************************************************************************

#include <string.h>
#include <stdlib.h>
#include "posapi.h"
#include "define.h"
#include "tools.h"

#define LINELENGTH 80           //�����ļ���ÿ�еĳ���

/*�ַ�������ȥ�պ��� */

/*==*srcԴ�ַ�����*destĿ���ַ�����type��ȡ����0-��ȥ��1-��ȥ��2-����ȥ��*/
int _StrTrim (char *dest, const char *src, int type)
{
    int             i;
    int             iOffsetLeft;
    int             iOffsetRight;
    int             len;

    iOffsetLeft = 0;
    iOffsetRight = 0;
    len = strlen (src);

    //����Դ�ַ�������߷ǿ���ʼλ��
    for (i = 0; i < len; i++)
    {
        if (src[i] != ' ')
        {
            iOffsetLeft = i;
            break;
        }
    }

    //Դ�ַ���Ϊȫ��
    if (i == len)
    {
        dest[0] = '\0';
        return SUCC;
    }

    //����Դ�ַ������ұ߷ǿ���ʼλ��
    for (i = len - 1; i >= 0; i--)
    {
        if (src[i] != ' ')
        {
            iOffsetRight = i;
            break;
        }
    }


    switch (type)
    {
        case 0:                //��ȥ��
            memcpy (dest, src + iOffsetLeft, len - iOffsetLeft);
            dest[len - iOffsetLeft] = '\0';
            break;
        case 1:                //��ȥ��
            memcpy (dest, src, iOffsetRight + 1);
            dest[iOffsetRight + 1] = '\0';
            break;
        case 2:                //����ȥ��
            memcpy (dest, src + iOffsetLeft, iOffsetRight - iOffsetLeft + 1);
            dest[iOffsetRight - iOffsetLeft + 1] = '\0';
            break;
        default:
            return FAIL;
    }
    return SUCC;
}



/**
*	@fn 	int ltrim(char *dest, const char *src)
*	@brief	�ַ���ȥ��ո�
*	@param	src		[in] Դ�ַ���
			dest	[out] Ŀ���ַ���
*	@return	@li SUCC	�ɹ�
			@li FAIL	ʧ��	
*	@sa		define.h
*/
//************************************************************************************
int ltrim (char *dest, const char *src)
{
    return _StrTrim (dest, src, 0);
}


/**
*	@fn 	int rtrim(char *dest, const char *src)
*	@brief	�ַ���ȥ�ҿո�
*	@param	src		[in] Դ�ַ���
			dest	[out] Ŀ���ַ���
*	@return	@li SUCC	�ɹ�
			@li FAIL	ʧ��	
*	@sa		define.h
*/
//************************************************************************************
int rtrim (char *dest, const char *src)
{
    return _StrTrim (dest, src, 1);
}


/**
*	@fn 	int alltrim(char *dest, const char *src)
*	@brief	�ַ���ȥ���ҿո�
*	@param	src		[in] Դ�ַ���
			dest	[out] Ŀ���ַ���
*	@return	@li SUCC	�ɹ�
			@li FAIL	ʧ��	
*	@sa		define.h
*/
//************************************************************************************
int alltrim (char *dest, const char *src)
{
    return _StrTrim (dest, src, 2);
}




/**
*	@fn 	int ReadLine (char *pcLineBuf, int iLineBufLen, int fd)
*	@brief	���ı��ļ��ж�һ���ַ���

*	@param	pcLineBuf		[out] ��Ŷ�ȡ���ַ���������ȡiLineBufLen-1���ַ�
			iLineBufLen		[in] pcLineBuf�ĳ���
			fd				[in] �ļ����
*	@return	@li SUCC	�ɹ�
			@li FAIL	ʧ��	
*	@sa		define.h
*/
//************************************************************************************
int ReadLine (char *pcLineBuf, int iLineBufLen, int fd)
{
    int             iRet, i;
    char            rgcTemp[2];

    // ��һ���ַ���
    i = 0;
    while (1)
    {
        if (i > iLineBufLen - 1)
        {
            break;
        }
        iRet = fread (rgcTemp, 1, fd);
        if (iRet == FAIL)
        {
            return FAIL;
        }
        else if (iRet != 1)
        {
            break;
        }
        else if ((rgcTemp[0] == '\r') || (rgcTemp[0] == '\n'))
        {
            break;
        }
        else
        {
            pcLineBuf[i++] = rgcTemp[0];
        }
    }
    pcLineBuf[i] = 0;
    return SUCC;
}



/**
*	@fn 	int WriteLine(const char *pcLineBuf, int fd)
*	@brief	���ı��ļ���д��һ���ַ���
*	@param	pcLineBuf		[in] д����ַ�������0x00��β,���80���ַ�)
			fd				[in] �ļ����
*	@return	@li SUCC	�ɹ�
			@li FAIL	ʧ��	
*	@sa		define.h
*/
//************************************************************************************
int WriteLine (const char *pcLineBuf, int iLineBufLen, int fd)
{
    int             iRet;
    char            rgcTemp[LINELENGTH + 2];
    int             iLen;

    if (iLineBufLen > LINELENGTH)
    {
        iLen = LINELENGTH;
    }
    else
    {
        iLen = iLineBufLen;
    }

    memset (rgcTemp, ' ', sizeof (rgcTemp));
    memcpy (rgcTemp, pcLineBuf, iLen);
    rgcTemp[LINELENGTH] = '\n';

    iRet = fwrite (rgcTemp, LINELENGTH + 1, fd);
    if (iRet != LINELENGTH + 1)
    {
        return FAIL;
    }
    else
    {
        return SUCC;
    }
}


/**
*	@fn 	ReadItemFromINI (const char *inifile, const char *item, char *value)
*	@brief	�������ļ��ж�ȡ����
*	@param	inifile		[in] �����ļ���
*			item 		[in] ��Ŀ�� 
*           value		[out]��Ŀ��ֵ
*	@return	@li 0   �ɹ�
			@li <0  ʧ��	
*	@sa		tools.h
*/
//************************************************************************************
int ReadItemFromINI (const char *inifile, const char *item, char *value)
{
    char            buf[LINELENGTH + 2];
    char            tmpbuf[LINELENGTH + 2];
    char            itemname[LINELENGTH + 2];
    int             fd;
    int             ret;

    /* ���� item=  */
    memcpy (itemname, item, strlen (item));
    itemname[strlen (item)] = '=';

    /* ��INI�ļ� */
    if ((fd = fopen (inifile, "r")) == FAIL)
    {
        return ERR_TOOLS_INIFILE;
    }

    // ����ָ������Ŀ
    while (1)
    {
        memset (tmpbuf, 0, sizeof (tmpbuf));
        memset (buf, 0, sizeof (buf));
        ret = ReadLine (tmpbuf, LINELENGTH, fd);
        alltrim (buf, tmpbuf);
        if (ret == FAIL)
        {
            fclose (fd);
            return ERR_TOOLS_ITEMNAME;
        }

        /* �ҵ�ƥ���item */
        if (memcmp (buf, itemname, strlen (itemname)) == 0)
        {
            strcpy (value, buf + strlen (itemname));
            return SUCC;
        }
    }
}


/**
*	@fn 	char CalcLRC(char *buf, int len)
*	@brief 	����LRC
*	@param	buf     [in] ��Ҫ����LRC���ַ���
*	@return	@li	    ����ó���LRC
*	@sa	    tools.h	
*/
char CalcLRC (const char *buf, int len)
{
    int             i;
    char            ch;

    ch = 0x00;
    for (i = 0; i < len; i++)
    {
        ch ^= buf[i];
    }
    return (ch);
}





/*----��������-----------*/

/*----�������ֵ---------*/
void Beep (int num)
{
    int             j = 0;

    while (1)
    {
        beep ();
        if (j++ == num)
            break;
    }
}

//1.2��������������Ϣ��ʾ�����ذ���ֵ ��ʹ��API����getkeycode().
int Beep_Msg (char *str, int beepnum, int waitetime)
{
    clrscr ();
    Beep (beepnum);
    printf ("%s", str);
    return getkeycode (waitetime);
}

//1.3�ɹ���Ϣ������ʾ������һ�£��ȴ�ʱ�����롣
void SUCC_MSG (char *str)
{
    clrscr ();
    Beep (1);
    printf ("%s", str);
    getkeycode (3);
}

//1.4������Ϣ������ʾ���������£��ȴ������ذ���
int ERR_MSG (char *str)
{
    clrscr ();
    Beep (3);
    printf ("\n%s", str);
    return (getkeycode (60));
}

//1.5������ʾ��Ϣ�޵ȴ�
void MSG (char *str)
{
    clrscr ();
    printf ("%s", str);
 //   getkeycode(0) ;
}



/**
*	@fn 	int AsciiToBcd(unsigned char *bcd_buf, const unsigned char *ascii_buf, int len, char type)
*	@brief 	ASCII�ַ���ת��ΪBCD�ַ���
*	@param  bcd_buf		[out] ת�������BCD�ַ���
	@param	ascii_buf	[in] ��Ҫת����ACSII�ַ���
	@param	len 	    [in] �������ݳ���(ASCII�ַ����ĳ���)
	@param	type		[in] ���뷽ʽ  1�������  0���Ҷ���
*	@return	@li	 0	�ɹ�
			@li  <0 ʧ��(�ַ����а����Ƿ��ַ�)
*	@sa	    tools.h	
*/
int AsciiToBcd (unsigned char *bcd_buf, const unsigned char *ascii_buf, int len, char type)
{
    int             cnt;
    char            ch, ch1;

    if (len & 0x01 && type)     /*�б��Ƿ�Ϊ�����Լ����Ǳ߶��� */
        ch1 = 0;
    else
        ch1 = 0x55;

    for (cnt = 0; cnt < len; ascii_buf++, cnt++)
    {
        if (*ascii_buf >= 'a')
            ch = *ascii_buf - 'a' + 10;
        else if (*ascii_buf >= 'A')
            ch = *ascii_buf - 'A' + 10;
        else if (*ascii_buf >= '0')
            ch = *ascii_buf - '0';
        else if (*ascii_buf == 0)
            ch = 0x0f;
        else
        {

            ch = (*ascii_buf);
            ch &= 0x0f;         //��������λ
        }
        if (ch1 == 0x55)
            ch1 = ch;
        else
        {
            *bcd_buf++ = ch1 << 4 | ch;
            ch1 = 0x55;
        }
    }                           //for
    if (ch1 != 0x55)
        *bcd_buf = ch1 << 4;

    return 0;
}


/**
*	@fn 	int BcdToAscii(unsigned char *ascii_buf, const unsigned char *bcd_buf, int len, int type)
*	@brief 	BCD�ַ���ת��ΪASCII�ַ���
*	@param	ascii_buf	[out] ת�������ACSII�ַ���
	@param  bcd_buf		[in] ��Ҫת����BCD�ַ���
	@param	len 	    [in] ������ݳ���(ASCII�ַ����ĳ���)
	@param	type		[in] ���뷽ʽ  1�������  0���Ҷ���
*	@return	@li	 0	�ɹ�
*	@sa	    tools.h	
*/
int BcdToAscii (unsigned char *ascii_buf, const unsigned char *bcd_buf, int len, char type)
{
    int             cnt;
    unsigned char  *pBcdBuf;

    pBcdBuf = (unsigned char *) bcd_buf;

    if ((len & 0x01) && type)   /*�б��Ƿ�Ϊ�����Լ����Ǳ߶��� */
    {                           /*0��1�� */
        cnt = 1;
        len++;
    }
    else
        cnt = 0;
    for (; cnt < len; cnt++, ascii_buf++)
    {
        *ascii_buf = ((cnt & 0x01) ? (*pBcdBuf++ & 0x0f) : (*pBcdBuf >> 4));
        *ascii_buf += ((*ascii_buf > 9) ? ('A' - 10) : '0');
    }
    *ascii_buf = 0;
    return 0;
}


/**
*	@fn 	int IntToC4(unsigned char *pbuf, unsigned int num)
*	@brief 	����ת��Ϊ4�ֽ��ַ�������λ��ǰ��
*	@param	pbuf     	[out] ת��������ַ���
	@param  num 		[in] ��Ҫת����������
*	@return	@li	 0	�ɹ�
*	@sa	    tools.h	
*/
int IntToC4 (unsigned char *pbuf, unsigned int num)
{
    *pbuf = (unsigned char) (num >> 24);
    *(pbuf + 1) = (unsigned char) ((num >> 16) % 256);
    *(pbuf + 2) = (unsigned char) ((num >> 8) % 256);
    *(pbuf + 3) = (unsigned char) (num % 256);
    return 0;
}



/**
*	@fn 	int IntToC2(unsigned char *pbuf, unsigned int num)
*	@brief 	����ת��Ϊ2�ֽ��ַ�������λ��ǰ��
*	@param	pbuf     	[out] ת��������ַ���
	@param  num 		[in] ��Ҫת����������
*	@return	@li	 0	�ɹ�
*	@sa	    tools.h	
*/
int IntToC2 (unsigned char *pbuf, unsigned int num)
{
    *(pbuf) = (unsigned char) (num >> 8);
    *(pbuf + 1) = (unsigned char) (num % 256);
    return 0;
}


/**
*	@fn 	int C4ToInt(unsigned int *num, unsigned char *pbuf)
*	@brief 	4�ֽ��ַ���ת��Ϊ���ͣ���λ��ǰ��
*	@param	num     	[out] ת�������������
	@param  num 		[in] ��Ҫת�����ַ���
*	@return	@li	 0	�ɹ�
*	@sa	    tools.h	
*/
int C4ToInt (unsigned int *num, unsigned char *pbuf)
{
    *num = ((*pbuf) << 24) + ((*(pbuf + 1)) << 16) + ((*(pbuf + 2)) << 8) + (*(pbuf + 3));
    return 0;
}


/**
*	@fn 	int C2ToInt(unsigned int *num, unsigned char *pbuf)
*	@brief 	2�ֽ��ַ���ת��Ϊ���ͣ���λ��ǰ��
*	@param	num     	[out] ת�������������
	@param  num 		[in] ��Ҫת�����ַ���
*	@return	@li	 0	�ɹ�
*	@sa	    tools.h	
*/
int C2ToInt (unsigned int *num, unsigned char *pbuf)
{
    *num = ((*pbuf) << 8) + (*(pbuf + 1));
    return 0;
}


/**
*	@fn 	char ByteToBcd(int num)
*	@brief 	����(0-99)ת��Ϊһ�ֽ�BCD
*	@param	num     [in] ��Ҫת����������(0-99)
*	@return	@li	 ת�������BCD
*	@sa	    tools.h	
*/
char ByteToBcd (int num)
{
    return ((num / 10) << 4) | (num % 10);
}


/**
*	@fn 	int BcdToByte(char bcd)
*	@brief 	һ�ֽ�BCDת��Ϊ����(0-99)
*	@param	bcd     [in] ��Ҫת����bcd
*	@return	@li	 ת�����������
*	@sa	    tools.h	
*/
int BcdToByte (int num)
{
    return (num >> 4) * 10 + (num & 0x0f);
}


/**
*	@fn 	int IntToBcd(char *bcd, int num)
*	@brief 	����(0-9999)ת��Ϊ���ֽ�BCD
*	@param  num		[in] ��Ҫת��������(0-9999)
*	@param	bcd     [out] ת�������BCD(���ֽ�)
*	@return	@li	 0 �ɹ�
*	@sa	    tools.h	
*/
int IntToBcd (char *bcd, int num)
{
    bcd[0] = ByteToBcd (num / 100);
    bcd[1] = ByteToBcd (num % 100);

    return 0;
}


/**
*	@fn 	int BcdToInt(char *bcd)
*	@brief 	���ֽ�BCDת��Ϊ����(0-9999)
*	@param	bcd     [in] ��Ҫת����BCD(���ֽ�)
*	@return	@li	 ת�����������
*	@sa	    tools.h	
*/
int BcdToInt (const char *bcd)
{
    return BcdToByte (bcd[0]) * 100 + BcdToByte (bcd[1]);
}


/**
*	@fn 	GetTime
*	@brief 	��POS��ȡ��ǰ���ں�ʱ��
*	@param	(out)	char *date	���ص�����(yyyymmdd 8�ֽ�)
*	@param	(out)	char *time	���ص�ʱ��(hhmmss 6�ֽ�) 
*	@return	void
*	@sa	    tools.h	
*/
void GetTime (char *pchDate, char *pchTime)
{
    struct postime  strtime;

    getpostime (&strtime);

    BcdToAscii ((unsigned char *) (pchDate + 0), (unsigned char *) (&strtime.yearh), 2, 0);
    BcdToAscii ((unsigned char *) (pchDate + 2), (unsigned char *) (&strtime.yearl), 2, 0);
    BcdToAscii ((unsigned char *) (pchDate + 4), (unsigned char *) (&strtime.month), 2, 0);
    BcdToAscii ((unsigned char *) (pchDate + 6), (unsigned char *) (&strtime.day), 2, 0);
    BcdToAscii ((unsigned char *) (pchTime), (unsigned char *) (&strtime.hour), 2, 0);
    BcdToAscii ((unsigned char *) (pchTime + 2), (unsigned char *) (&strtime.minute), 2, 0);
    BcdToAscii ((unsigned char *) (pchTime + 4), (unsigned char *) (&strtime.second), 2, 0);
}



/**
*	@fn 	SetTime
*	@brief 	�޸�POS�����ں�ʱ��
*	@param	(in)	char *date	����(yyyymmdd 8�ֽ�)
*	@param	(in)	char *time	ʱ��(hhmmss 6�ֽ�) 
*	@return	void
*	@sa	    tools.h	
*/
void SetTime (const char *pchDate, const char *pchTime)
{
    struct postime  strtime;
    char            date[4];
    char            time[3];

    AsciiToBcd ((unsigned char *) date, (unsigned char *) pchDate, 8, 0);
    AsciiToBcd ((unsigned char *) time, (unsigned char *) pchTime, 6, 0);
    strtime.yearh = date[0];
    strtime.yearl = date[1];
    strtime.month = date[2];
    strtime.day = date[3];
    strtime.hour = time[0];
    strtime.minute = time[1];
    strtime.second = time[2];
    setpostime (strtime);
}

/**
*	@fn 	DispXY
*	@brief 	��ָ��(x,y)λ����ʾstring������
*	@param	(in)	int x
*	@param	(in)	int y
*	@param	(in)	char *string
*	@return	void
*	@sa	    tools.h	
*/
void DispXY(int x, int y, char *string)
{
	gotoxy(x,y);
	printf("%s",string);
}


//�Ե�����Ϣ��ʽ��ʾ������
//���ƻ�ԭ����
/**
*	@fn 	Disp_deg_buf
*	@brief 	�Ե�����Ϣ��ʽ��ʾ�����������ƻ�ԭ����
*	@param	(in)	char *buf ����ʾ���ַ���
*	@param	(in)	int buflen	�ַ�������
*	@return	void
*	@sa	    tools.h	
*/
void Disp_deg_buf(char *buf,int buflen)
{
	int i;

	pushscreen();
	clrscr();
	for(i=1;i<=buflen;i++)
	{
		printf("%02x",*(buf+i-1));
		if(i%8==0)	printf("\n");
		if(i%32==0)
		{
			getkeycode(0);
			clrscr();
		}
	}
	getkeycode(0);
	popscreen();
}

int JudgeValue(char *buf,int min,int max)
{

	int tmp;
	
	tmp = atoi(buf);

	if((tmp>=min)&&(tmp<=max))
		return TRUE;
	else
		return FALSE;
}

void ChangePosDate()
{
	int tmp, tmp1;
	char sbuf[30],bufyear[6],bufmon[4],bufdate[4];
	char pchDate[8+1];
	char pchTime[6+1];
	while(1)
	{
		memset(pchDate, 0, sizeof(pchDate));
		memset(pchTime, 0, sizeof(pchTime));
		GetTime(pchDate, pchTime);
		
		clrscr();
		DispString(0,0,"��ǰ����:");
		printf("\n%4.4s��%2.2s��%2.2s��",pchDate, pchDate+4, pchDate+6);
		DispString(0,32,"��������:");

		// �������
		while(1)
		{
			gotoxy(0,6);
			printf("    ��  ��  ��");
			
			gotoxy(0,7);
			memset(bufyear, 0, sizeof(bufyear));
			if((tmp=getnumstr(bufyear,4,NORMAL,0))<0) //ESC
			{
				return;
			}
			
			if(tmp==0)
			{
				memcpy(bufyear, pchDate, 4);
				break;
			}
			else if(tmp==4)
			{
				if(JudgeValue(bufyear,1990,2189)==TRUE)
				{
					break;
				}
			}
		}

		
		// �����·�
		while(1)
		{   
			sprintf(sbuf,"%s��  ��  ��",bufyear);
			gotoxy(0,6);
			printf("%s\n",sbuf);
			
			gotoxy(6,7);
			memset(bufmon, 0, sizeof(bufmon));
			if( (tmp=getnumstr(bufmon,2,NORMAL,0))<0)  //ESC��
				return;

			if(tmp==0)	//Enter��
			{
				memcpy(bufmon, pchDate+4, 2);
				break;
			}
			else if(tmp==2)
			{
				if(JudgeValue(bufmon,1,12)==TRUE)
				{
					break;
				}
			}
		}
		
		
		// ��������
		while(1)
		{   
			sprintf(sbuf,"%s��%s��  ��",bufyear,bufmon);
			gotoxy(0,6);
			printf("%s\n",sbuf);
			
			gotoxy(10,7);
			memset(bufdate, 0, sizeof(bufdate));
			if( (tmp=getnumstr(bufdate,2,NORMAL,0))<0)  //ESC��
				return;
			
			if(tmp==0)	//Enter��
			{
				memcpy(bufdate, pchDate+6, 2);
				break;
			}
			else if(tmp == 2)
			{
			
				switch(atoi(bufmon))
				{
					case 1:
					case 3:
					case 5:
					case 7:
					case 8:
					case 10:
					case 12:
						tmp1=JudgeValue(bufdate,1,31);
						break;
					case 4:
					case 6:
					case 9:
					case 11:
						tmp1=JudgeValue(bufdate,1,30);
						break;
					case 2:
						if(atoi(bufyear)%4==0)
							tmp1=JudgeValue(bufdate,1,29);
						else
							tmp1=JudgeValue(bufdate,1,28);
						break;
				}
				if(tmp1 == TRUE)
				{
					break;
				}
			}
		}

		// �޸��ꡢ�¡��գ�ʱ���֡���Ϊ��ǰֵ
		memcpy(pchDate, bufyear, 4);
		memcpy(pchDate+4, bufmon, 2);
		memcpy(pchDate+6, bufdate, 2);
		SetTime(pchDate, pchTime);
	}	
}

void ChangePosTime()
{
	int tmp;
	char sbuf[30];
	char bufhour[4],bufmin[4],bufsec[4];
	char pchDate[8+1];
	char pchTime[6+1];
	
	while(1)
	{

		memset(pchDate, 0, sizeof(pchDate));
		memset(pchTime, 0, sizeof(pchTime));
		GetTime(pchDate, pchTime);

		//.........................................................
		clrscr();
		DispString(0,0,"��ǰʱ��:");
		sprintf(sbuf,"%2.2s:%2.2s:%2.2s",pchTime,pchTime+2,pchTime+4);
		printf("\n%s",sbuf);
		DispString(0,32,"����ʱ��:");

		// ����Сʱ
		while(1)
		{   
			gotoxy(0,6);     
			printf("  :  :  ");
			
			gotoxy(0,7);
			memset(bufhour, 0, sizeof(bufhour));
			if( (tmp=getnumstr(bufhour,2,NORMAL,0))<0)  //ESC��
				return;

			if(tmp==0)	//Enter��
			{
				memcpy(bufhour, pchTime, 2);
				break;
			}
			else if(tmp == 2)
			{
				if(JudgeValue(bufhour,0, 23)==TRUE)
				{
					break;
				}
			}
		} 

		// ���÷���
		while(1)
		{
			sprintf(sbuf,"%s:  :  ",bufhour);
			gotoxy(0,6);
			printf("%s\n",sbuf);
			
			gotoxy(3,7);
			memset(bufmin, 0, sizeof(bufmin));
			if( (tmp=getnumstr(bufmin,2,NORMAL,0))<0)  //ESC��
				return;

			if(tmp==0)	//Enter��
			{
				memcpy(bufmin, pchTime+2, 2);
				break;
			}
			else if(tmp==2)
			{
				if(JudgeValue(bufmin,0,59)==TRUE)
				{
					break;
				}
			}
		}         	        	
		
		// ��������
		while(1)
		{    
			sprintf(sbuf,"%s:%s:  ",bufhour,bufmin);
			gotoxy(0,6);
			printf("%s\n",sbuf);
			
			gotoxy(6,7);
			memset(bufsec, 0, sizeof(bufsec));
			if( (tmp=getnumstr(bufsec, 2, NORMAL,0))<0)  //ESC��
				return;
			if(tmp==0)	//Enter��
			{
				memcpy(bufsec, pchTime+4, 2);
				break;
			}
			else if(tmp==2)
			{
				if(JudgeValue(bufsec,0,59)==TRUE)
				{
					break;
				}
			}
		}      	        	
		

		// �޸��ꡢ�¡��գ�ʱ���֡���Ϊ��ǰֵ
		memcpy(pchTime, bufhour, 2);
		memcpy(pchTime+2, bufmin, 2);
		memcpy(pchTime+4, bufsec, 2);
		SetTime(pchDate, pchTime);
	}
}

void ChangeDateTime(void)
{
	int kc;

	clrscr();
	do
	{
		gotoxy(1,1); printf("1. �޸�����");
		gotoxy(1,3); printf("2. �޸�ʱ��");
		gotoxy(2,6); printf("ESC...�˳�");

		kc=getkeycode(0);
		if(kc=='1'||kc=='2')
		{
			if(kc=='1')
				ChangePosDate();
			else
				ChangePosTime();
			clrscr();
		}
	}while(kc!=ESC);
}

