#pragma once

//----����������---------------------------------------------------------------------

#define OP_TYPE_nullptr             -1          // ������
#define OP_TYPE_INT					0			//�������泣��ֵ
#define OP_TYPE_PTR					1			//����ָ��
#define OP_TYPE_STRING_REF			2			//�ַ�������
#define OP_TYPE_ABS_STACK_INDEX		3			//���Զ�ջ����
#define OP_TYPE_REL_STACK_INDEX		4			//��Զ�ջ����
#define OP_TYPE_INSTR_INDEX			5			//ָ������
#define OP_TYPE_FUNC_INDEX			6			//��������
#define OP_TYPE_HOST_API_CALL_INDEX	7			//��Ӧ�ó���API��������
#define OP_TYPE_REG					8			//�Ĵ���

//----ָ�������---------------------------------------------------------------------

#define INSTR_MOV                   0			//��ֵָ��

#define INSTR_ADD                   1			//����ָ��
#define INSTR_SUB                   2
#define INSTR_MUL                   3
#define INSTR_DIV                   4

#define INSTR_JMP                   5			//��תָ��
#define INSTR_JE                    6
#define INSTR_JNE                   7
#define INSTR_JG                    8
#define INSTR_JL                    9

#define INSTR_PUSH                  10			//��ջָ��
#define INSTR_POP                   11

#define INSTR_CALL                  12			//����ָ��
#define INSTR_RET                   13
#define INSTR_IRET					14
#define INSTR_CALLCORE              15

#define INSTR_PAUSE                 16			//����ָ��
#define INSTR_QUIT                  17

//----cpu����----------------------------------------------------------------------

#define INSTR_LEN	32							//ָ���

#define CPU_RATE_M	5							//cpuƵ��

//----�ṹ��-----------------------------------------------------------------------

//������
typedef struct _op {			
	int iType;									//����������
	union {
		int iIntLiteral;						//����������
		float fFloatLiteral;					//������������
		int iStringTableIndex;					//�ַ���������
		int iStackIndex;						//��ջ����
		int iInstrIndex;						//ָ������
		int iFuncIndex;							//��������
		int iHostAPICallIndex;					//��Ӧ�ó���API��������
		int iReg;								//�Ĵ�����
	};
	int iOffsetIndex;							//ƫ�����������������������Ƕ�ջ����ʱʹ�ã��洢��ջƫ��
}Op;

//ָ��
typedef struct _instr {
	unsigned short mOpCode;						//ָ�������
	unsigned char mOpNum;						//����������
	Op mOpList[4];								//ָ�������
}Instr;

