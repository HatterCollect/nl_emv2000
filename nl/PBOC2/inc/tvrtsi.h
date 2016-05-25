/**
* @file tvrtsi.h
* @brief �ն���֤���TVR������״̬TSI��λ�ĺ���

* ���ļ�����EMV2000��Ŀ
* @version Version 1.0
* @author Ҷ��ͳ
* @date 2005-08-02
*/

#ifndef _TVR_TSI_
#define _TVR_TSI_

/* TVR - Terminal Verification Results */

/* ��һ�ֽڣ�����ߣ�*/
#define OFFLINE_AUTH_UNDO  0x0080  /* δ�����ѻ�������֤      */
#define OFFLINE_SDA_FAIL   0x0040  /* �ѻ���̬������֤ʧ��    */
#define ICC_DATA_LOST      0x0020  /* IC������ȱʧ            */
#define CARD_IN_EXCEP_FILE 0x0010  /* ��Ƭ�������ն��쳣�ļ���*/
#define OFFLINE_DDA_FAIL   0x0008  /* �ѻ���̬������֤ʧ��    */
#define CDA_FAIL           0x0004  /* ���϶�̬������֤ʧ��    */
#define GEN_AC_FAIL        0x0004  /* Ӧ����������ʧ��        */

/* �ڶ��ֽ� */
#define VERSION_NOT_MATCHED 0x0180  /* IC�����ն�Ӧ�ð汾��һ�� */
#define APP_EXPIRE          0x0140  /* Ӧ���ѹ���               */
#define APP_NOT_EFFECT      0x0120  /* Ӧ����δ��Ч             */
#define SERV_REFUSE         0x0110  /* ��Ƭ��Ʒ��������������� */
#define NEW_CARD            0x0108  /* �¿�                     */

/* �����ֽ� */
#define CV_NOT_SUCCESS    0x0280  /* �ֿ�����֤δ�ɹ� */
#define UNKNOW_CVM        0x0240  /* δ֪��CVM        */
#define PIN_RETRY_EXCEED  0x0220  /* PIN���Դ�������  */
#define REQ_PIN_PAD_FAIL  0x0210  /* Ҫ������PIN������PIN PAD��PIN PAD���� */
#define REQ_PIN_NOT_INPUT 0x0208  /* Ҫ������PIN����PIN PAD����δ����PIN   */
#define INPUT_ONLINE_PIN  0x0204  /* ��������PIN      */

/* �����ֽ� */
#define TRADE_EXCEED_FLOOR_LIMIT       0x0380  /* ���׳�������޶�       */
#define EXCEED_CON_OFFLINE_FLOOR_LIMIT 0x0340  /* ���������ѻ���������   */
#define EXCEED_CON_OFFLINE_UPPER_LIMIT 0x0320  /* ���������ѻ���������   */
#define SELECT_ONLILNE_RANDOM          0x0310  /* ���ױ����ѡ���������� */
#define MERCHANT_REQ_ONLINE            0x0308  /* �̻�Ҫ����������       */

/* �����ֽ� */
#define USE_DEFAULT_TDOL    0x0480   /* ʹ��ȱʡTDOL   */
#define ISSUER_AUTH_FAIL    0x0440   /* ��������֤ʧ�� */

#define SCRIPT_FAIL_BEFORE_LAST_GEN_AC 0x0420 /* ���һ������Ӧ����������֮ǰ
                                               �ű�����ʧ�� */
#define SCRIPT_FAIL_AFTER_LAST_GEN_AC  0x0410 /* ���һ������Ӧ����������֮��
                                               �ű�����ʧ�� */


/* TSI - Transaction Status Information */

/* ��һ�ֽڣ�����ߣ�*/
#define OFFLINE_DA_COMPLETION      0x0080  /* �ѻ�������֤����� */
#define CV_COMPLETION              0x0040  /* �ֿ�����֤�����   */
#define CARD_RISK_MANA_COMPLETION  0x0020  /* ��Ƭ���չ�������� */
#define ISSUER_AUTH_COMPLETION     0x0010  /* ��������֤�����   */
#define TERM_RISK_MANA_COMPLETION  0x0008  /* �ն˷��չ�������� */
#define SCRIPT_PROC_COMPLETION     0x0004  /* �ű����������     */


/* ��ʼ��TVR�ֽ� */
void InitTVR(void) ;

/* ����TVR�ֽ� */
void SetTVR( int iSetMask, int OnOff ) ;

/* ����TVR�ֽ� */
void GetTVR( char * tvrtmp ) ;

/* ��ʼ��TSI�ֽ� */
void InitTSI(void) ;

/* ����TSI�ֽ� */
void SetTSI( int iSetMask, int OnOff ) ;

/* ����TSI�ֽ� */
void GetTSI( char * tsitmp ) ;

#endif