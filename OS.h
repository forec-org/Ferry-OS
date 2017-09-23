#pragma once

//-------------------------------------------------------------------------
//
//	�������Ͷ���
//
//-------------------------------------------------------------------------
typedef bool				BOOLEAN;
typedef unsigned char		BYTE;
typedef char				CHAR;
typedef unsigned char		UINT8;
typedef char				INT8;
typedef unsigned short		UINT16;
typedef short				INT16;
typedef unsigned int		UINT32;
typedef int					INT32;
typedef unsigned long long	UINT64;
typedef long long			INT64;
typedef float				F32;
typedef double				F64;

typedef unsigned int		STK_SIZE;

#define NULL				nullptr
#define FALSE				0
#define TRUE				1


//-------------------------------------------------------------------------
//
//	��������
//
//-------------------------------------------------------------------------

//����״̬��
#define PROC_STATE_READY				0x00						//������������Ա�����ִ��
#define PROC_STATE_RUNNING				0x01						//����������
#define PROC_STATE_WAITING				0x02						//����ȴ��¼����ź���
#define PROC_STATE_SUSPEND				0x04						//�������
#define PROC_STATE_TERMINATED			0x08						//������ֹ���˳��ڴ�
#define PROC_STATE_SWAPPED_READY		0x10						//���񱻻���ʱ����
#define PROC_STATE_SWAPPED_WAITING		0x20						//���񱻻���ʱ�ȴ��¼����ź���
#define PROC_STATE_SWAPPED_SUSPEND		0x40						//���񱻻���ʱ����

#define PROC_STATE_WAITING_ANY			PROC_STATE_WAITING | PROC_STATE_SWAPPED_WAITING					
#define PROC_STATE_SUSPEND_ANY			PROC_STATE_SUSPEND | PROC_STATE_SWAPPED_SUSPEND					


//������
#define ERR_NO_ERR						0x00						//�޴���
#define ERR_NULL_PTR					0x01						//��ָ��
#define ERR_SIZE_ZERO					0x02						//�ַ�������Ϊ0
#define ERR_STACK_OVERFLOW				0x03						
#define ERR_TOO_LARGE_SIZE				0x04						

#define ERR_NO_FREE_PCB					0x05						//û�п���pcb

#define ERR_DEL_INVALID					0x06						//ɾ�������ڵĳ���
#define ERR_DEL_IDLE					0x07						//ɾ����ת����

#define ERR_NO_FREE_SEM					0x08						//û�п����ź���
#define ERR_SEM_INVALID					0x09						//�ź���������
#define ERR_SEM_DEL_PENDING				0x0a						//ɾ�����ź������н��̵ȴ�
#define ERR_SEM_POST_NO_PEND			0x0b						//�ź����ĵȴ�����Ϊ��

#define ERR_FILE_IO						0x0c						//δ�ҵ���ִ���ļ�
#define ERR_INVALID_FSE					0x0d						//��ִ���ļ���׺������FSE
#define ERR_INVALID_VERSION				0x0e						//��Ч�Ŀ�ִ���ļ��汾

//pid
#define PID_IDLE						0x0000u						//��ת����pid
#define PID_CURR						0xffffu						//��ǰ����pid

//�ź���
#define SEM_NAME_SIZE					64							//

//-------------------------------------------------------------------------
//
//	���Ժ�������
//
//-------------------------------------------------------------------------
#define DEBUG_LEVEL 0
void	OSDebugStr(const char *tip, int level);
void	OSDebugStrn(const char *tip, int level);
void	OSDebugInt(int num, int level);
void	OSDebugIntn(int num, int level);
void	OSDebugFloat(F32 num, int level);
void	OSDebugFloatn(F32 num, int level);

#define LOG 1
void	OSLogStr(const char *tip);

//-------------------------------------------------------------------------
//
//	�ṹ��
//
//-------------------------------------------------------------------------
typedef struct _execFileHeader {
	char			idString[4];		//�ļ�id�ַ���
	unsigned short	versionMajor;		//��汾��
	unsigned short	versionMinor;		//С�汾��
	unsigned int	stackSize;			//��ջ��С
	unsigned int	globalDataSize;		//ȫ�����ݴ�С
	unsigned int	mainFuncIndex;		//��������С
}ExecFileHeader;