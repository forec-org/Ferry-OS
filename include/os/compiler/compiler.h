#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

class FASM {
	
	const static int MAX_INSTR_LOOKUP_COUNT = 256;
												//查找表将保存的指令的最大个数
	const static int MAX_FILENAME_SIZE = 2048;	//文件名最大长度
	const static int MAX_IDENT_SIZE = 256;		//最大标识符长度
	const static int MAX_LEXEME_SIZE = 256;		//符号最大长度
	const static int MAX_INSTR_MNEMONIC_SIZE = 16;
												//指令助记符的最大长度
	
	//	链表
	typedef struct _LinkedListNode {			//链表节点
		void* pData;							//指向节点数据的指针
		_LinkedListNode* pNext;					//指向下一个节点的指针
	}LinkedListNode;

	typedef struct _linkedList {				//链表
		LinkedListNode* pHead;					//指向头节点的指针
		LinkedListNode* pTail;					//指向尾结点的指针
		int iNodeCount;							//节点计数
	}LinkedList;

	//	词法分析器
	typedef int Token;

	typedef struct _Lexer {
		int iCurrSourceLine;					//记录当前代码行
		unsigned int iIndex0, iIndex1;			//用于分析的两个字符索引
		Token CurrToken;						//当前的属性字
		char pstrCurrLexeme[MAX_LEXEME_SIZE];	//当前属性字的字符串
		int iCurrLexState;						//当前词法分析器的状态
	}Lexer;

	//	脚本可执行文件头
	typedef struct _ScriptHeader {
		int iStackSize;							//堆栈大小
		int iGlobalDataSize;					//脚本全局变量大小
		int iIsMainFuncPresent;					//是否有主函数
		int iMainFuncIndex;						//主函数索引
	}ScriptHeader;

	//	查找指令，用于记录指令的合法格式
	typedef int OpTypes;                            // Operand type bitfield alias type
	typedef struct _InstrLookup                     // An instruction lookup
	{
		char pstrMnemonic[MAX_INSTR_MNEMONIC_SIZE];  // Mnemonic string
		int iOpcode;                                // Opcode
		int iOpCount;                               // Number of operands
		OpTypes * OpList;                           // Pointer to operand list
	}InstrLookup;

	//	操作数
	typedef struct _op {
		int iType;					//操作数类型
		union {
			int iIntLiteral;		//整形字面量
			float fFloatLiteral;	//浮点型字面量
			int iStringTableIndex;	//字符串表索引
			int iStackIndex;		//堆栈索引
			int iInstrIndex;		//指令索引
			int iFuncIndex;			//函数索引
			int iHostAPICallIndex;	//主应用程序API调用索引
			int iReg;				//寄存器码
		};
		int iOffsetIndex;			//偏移量索引，仅在数据类型是堆栈索引时使用，存储堆栈偏移
	}Op;

	//	指令
	typedef struct _Instr                           // An instruction
	{
		int iOpcode;                                // Opcode
		int iOpCount;                               // Number of operands
		Op * pOpList;                               // Pointer to operand list
	}Instr;

	//	函数表
	typedef struct _FuncNode {
		int iIndex;						//索引
		char pstrName[MAX_IDENT_SIZE];	//名称
		int iEntryPoint;				//入口点
		int iLocalDataSize;				//局部数据大小
	}FuncNode;

	//	行标签表
	typedef struct _LabelNode {
		int iIndex;						//索引
		char pstrIdent[MAX_IDENT_SIZE];	//标识符
		int iTargetIndex;				//目标指令的索引
		int iFuncIndex;					//标签所属函数
	}LabelNode;

	//变量符号表
	typedef struct _SymbolNode {			//符号表节点
		int iIndex;							//索引
		char pstrIdent[MAX_IDENT_SIZE];		//标识符
		int iType;							//变量类型，Int、Str、Ptr
		int iInitValue;						//变量预设的值，目前只有string引用能预设
		bool bIsArr;						//标志是否是数组
		int iSize;							//大小（字节数）
		int iStackIndex;					//符号指向的堆栈索引
		int iFuncIndex;						//符号所在的函数
	}SymbolNode;

	char mnemonics[18][12] =
	{
		"Mov",
		"Add", "Sub", "Mul", "Div",
		"Jmp", "JE", "JNE", "JG", "JL",
		"Push", "Pop",
		"Call", "Ret", "Iret", "CallHost",
		"Pause", "Quit"
	};

	static FASM *gInstance;

private:
	//----全局变量---------------------------------------------------------
	Lexer g_Lexer;										//词法分析器

	//----保存源代码-------------------------------------------------------
	char **g_ppstrSourceCode;							//保存整个源文件的数组,每行为一个指令

	int g_iSourceCodeSize;								//源文件行数

	FILE* g_pSourceFile;								//源文件指针

	char g_pstrSourceFilename[MAX_FILENAME_SIZE],		//源代码文件名
		g_pstrExecFilename[MAX_FILENAME_SIZE];			//目标文件名

	//----脚本头-----------------------------------------------------------
	ScriptHeader g_ScriptHeader;						//脚本头数据

	int g_iIsSetStackSizeFound;							//记录源文件中是否定义了堆栈大小

	//----指令规则查找表---------------------------------------------------
	InstrLookup g_InstrTable[MAX_INSTR_LOOKUP_COUNT];

	//----指令流-----------------------------------------------------------
	Instr* g_pInstrStream;								//指令流
	int g_iInstrStreamSize;								//指令数
	int g_iCurrInstrIndex;								//当前指令索引

	//----函数表-----------------------------------------------------------
	LinkedList g_FuncTable;

	//----行标签表---------------------------------------------------------
	LinkedList g_LabelTable;

	//----符号表-----------------------------------------------------------
	LinkedList g_SymbolTable;

	//----字符串表---------------------------------------------------------
	LinkedList g_StringTable;

	//----主程序api表------------------------------------------------------
	LinkedList g_HostAPICallTable;

public:
	FASM();
	~FASM();

	unsigned int Exec(const std::string &srcFileName, const std::string &destFileName = "");

	void SetSourceFileName(const std::string &srcFileName, const std::string &destFileName);

	//----函数原型--------------------------------------------------------------
	//----链表-------------------------------------------------------------
	void InitLinkedList(LinkedList* pList);				//初始化链表
	void FreeLinkedList(LinkedList* pList);				//释放链表节点
	int AddNode(LinkedList* pList, void* pData);		//添加节点
	void StripComments(char* pstrSourceLine);			//去除代码后的注释

	//----打印提示---------------------------------------------------------
	void PrintLogo();
	void PrintUsage();

	//----主程序-----------------------------------------------------------
	void Init();										//初始化指令查找表等各种数据结构
	void ShutDown();									//清空资源

	unsigned int LoadSourceFile();
	unsigned int AssmblSourceFile();					//对源文件进行汇编
	void PrintAssmblStats();
	unsigned int BuildFSE();

	//----退出加错误提示---------------------------------------------------
	void Exit();
	void ExitOnError(const std::string & pstrErrorMssg);
	void ExitOnCodeError(const std::string &pstrErrorMssg);
	void ExitOnCharExpectedError(char cChar);

	//----词法分析---------------------------------------------------------
	void ResetLexer();									//重置词法分析器
	Token GetNextToken();								//获取下一个属性字类型
	char* GetCurrLexeme();								//获取当前属性字名字
	char GetLookAheadChar();							//读取后一个字节，用于向前看分析
	int SkipToNextLine();								//跳到下一行

	//----指令查找表-------------------------------------------------------
	void InitInstrTable();														//初始化指令查找表
	int GetInstrByMnemonic(char* pstrMnemonic, InstrLookup* pInstr);			//根据助记符查指令
	int AddInstrLookup(const std::string & pstrMnemonic, int iOpcode, int iOpCount);			//添加指令结构体
	void SetOpType(int iInstrIndex, int iOpIndex, OpTypes iOpType);				//设置指令结构体信息

	//----表操作-----------------------------------------------------------
	int AddString(LinkedList* pList, char* pstrString);							//添加字符串
	int AddFunc(char* pstrName, int iEntryPoint);								//添加函数
	FuncNode* GetFuncByName(char* pstrName);									//通过函数名获取函数信息
	void SetFuncInfo(char* pstrName, int iLocalDataSize);						//设置函数信息

	int AddLabel(char* pstrIdent, int iTargetIndex, int iFuncIndex);			//添加行标签
	LabelNode* GetLabelByIdent(char* pstrIdent, int iFuncIndex);				//获取行标签信息

	int AddSymbol(char* pstrIdent, int iType, int iInitValue, bool bIsArr, int iSize, int iStackIndex, int iFuncIndex);	//添加变量
	SymbolNode* GetSymbolByIdent(char* pstrIdent, int iFuncIndex);				//通过标识符获取变量信息
	int GetStackIndexByIdent(char* pstrIdent, int iFuncIndex);					//通过标识符获取对咱索引
	int GetSizeByIdent(char* pstrIdent, int iFuncIndex);						//通过标识符获取变量大小
	bool GetIsArrByIdent(char* pstrIdent, int iFuncIndex);						//通过标识符获取变量是否是数组

	static FASM *getInstance();
	static void release();

	//----调试用-----------------------------------------------------------
#define DEBUG_ENABLE 0
#define DEBUG_LEVEL 0
	void FASMDebug(const char *tip, short level);
	void FASMDebug(int tip, short level);

#define LOG_ENABLE 0
	void FASMLog(const char *tip);

public:
	//----字符串函数----------------------------------------------
	bool IsCharWhitespace(char cChar);					//判断字符是不是空白符
	bool IsCharNumeric(char cChar);						//判断字符是不是数字
	bool IsCharIdent(char cChar);						//判断字符是不是合法标识符的组成
	bool IsCharDelimiter(char cChar);					//判断字符是不是分隔符
	void TrimWhitespace(char* pstrString);				//清除指令两头的空白符
	bool IsStringWhitespace(char* pstrString);			//判断字符串是不是空白符
	bool IsStringIdent(char* pstrString);				//判断字符串是不是标识符
	bool IsStringInteger(char* pstrString);				//判断字符串是不是数
};

