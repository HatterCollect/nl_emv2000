
#ifndef _App_Data_
#define _App_Data_


#define ADERR_BASE	(-4000)
#define ADERR_NOTFOUND 	(ADERR_BASE -1)
#define ADERR_LENOVER 		(ADERR_BASE -2)
#define ADERR_TAGOVER 		(ADERR_BASE -3)
#define ADERR_INVALIDTAG 	(ADERR_BASE -4)
#define ADERR_DATADUP		(ADERR_BASE -5) //�����ظ�,�����ظ�������,�����Ƿ����,������Ĵ���

void InitAppData (void) ;
int DelAppData (char  * tag) ;
int SetAppData (char  * tag, char  * content, int length) ;
char  * GetAppData (char  * tag, int * length) ;
int HasAppData(char * tag) ;
void GetTags(int * tmp, int * num) ;
#endif
