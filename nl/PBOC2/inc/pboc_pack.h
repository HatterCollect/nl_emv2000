#ifndef _PBOC_PACK_H_
#define _PBOC_PACK_H_

//������ʱ����Ҫ��TAG
#define TAG_TRANS_TYPE 		1	//Transaction Type
#define TAG_TRANS_COUNTER 		2	//Application Transaction Counter
#define TAG_ARQC				3	//ARQC
#define TAG_CRYPT_INFO_DATA	4	//Cryption Information Data
#define TAG_AMOUNT_AUTH		5	//Amount Authorized
#define TAG_AMOUNT_OTHER		6	//Amount Other
#define TAG_ENCIPHERED_PIN		7	//Enciphered PIN
#define TAG_AUTH_RESP_CODE	8	//Authorization Response Code
#define TAG_ISS_AUTH_DATA		9	//Issuer Authorized Data
#define TAG_ISS_SCRIPT			10	//Issuer Script
#define TAG_ISS_RESULT			11	//Issuer Script Results
#define TAG_TERM_VERIFI_RESULT	12	//Terminal Verification Result
#define TAG_TRANS_STATUS_INFO	13	//Transaction Status Information
#define TAG_APP_PAN			14	//Application PAN
#define TAG_TRANS_DATE			15	//Transaction Date
#define TAG_TRANS_TIME			16	//Transaction Time
#define TAG_CAPK_UPDATE		17	//CAPK Update Data
#define TAG_TRANS_RESULT		18	//transaction result
#define TAG_POS_ENTRY_MODE	19	//pos entry mode
#define TAG_ISS_DATA			20   //
#define TAG_TRANS_FORCEACCEPT	21

#define MAX_LABEL_LEN 4

#define TOTAL_TAG_NUMBER	21 ///<�����Ҫ��TAG����

///<����TLV��������
#define TRANS_AUTH 0x01 //authorization
#define TRANS_FINANCIAL 0x02 //Financial
#define TRANS_BATCH 0x03 //Batch Data Capture 
#define TRANS_BATCH_LAST 0x04 //Batch Last
#define TRANS_REVERSAL 0x05//Reversal  
#define TRANS_UPDATE_CAK 0x06//Update CA Key
#define TRANS_ENTRY_MODE 0x07//Pos Entry Mode

#define TOTAL_TRANS_NUMBER 7///<�������TLV�������͵�����


#define PACKTLV_BASE -1200
#define PACKTLV_ERR_LENGTH (PACKTLV_BASE-1)
#define PACKTLV_ERR_BITMAP (PACKTLV_BASE-2)
#define PACKTLV_ERR_TAGMAP (PACKTLV_BASE-3)

typedef struct
{
	int iLabelLen;
	char cLabel[MAX_LABEL_LEN + 1];//int iLabel;
	int iValueLen;
	char *pxValue;
}PBOC_TLV;

typedef struct
{
	char TransType;
	char bitmap[TOTAL_TAG_NUMBER + 1];
}PBOC_BITMAP;

typedef struct
{
 	char TransType;//Transaction Type 
 	char ATC[2];//IC���е�Ӧ�ý��׵���ż�����
 	char ARQC[8];//ARQC
 	char CID;//Crypt Infor Data 
 	char gAmount[6];//Amount Authorized
 	char gAmount_O[6];//Amount Other 
 	char E_PIN[12];//Encrypted PIN by using des
 	char Auth_Res_Code[2];//��Ȩ��Ӧ����
 	char IADLen;
 	char IAD[16];//Tag 91 Issuer Authen Data
	char ISLen;//Issuer Script length
	char ISFlag;//0x72 or 0x71
 	char IS[256];//Issuer Script
	char ISRLen;//Issuer Script Results length
 	char ISR[256];//Issuer Script Results
 	char TVR[5];//Terminal Verification Results
 	char TSI[2];//Transaction Status Information
 	char Pan[11];
	char PanLen;//App PAN
 //	char gCardno[11];//App CardNo
 	char POS_date[5];//����(�ն�)����
 	char POS_time[5];//����(�ն�)ʱ��
	char UpdateKeyLen;//CAPK update length
 	char UpdateKey[255];//CAPK update buffer
 	char TransResult;//transaction result   
//	char NoUseLen;//No use data length
// 	char NoUse[16];//���ڴ����������
 	char POS_Enter_Mode;//Tag 9f39 ���۵����뷽ʽ
 	char ISS_APP_DATA[32] ;
	char IssAppDataLen ;
	char iAcceptForced ;
}PBOC_TRANS_DATA;

extern int CommWithHost(char cTransType, PBOC_TRANS_DATA * pxInData,  char * pxOutData);

#endif
