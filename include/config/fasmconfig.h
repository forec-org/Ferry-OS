#pragma once

namespace fasm {
	//----文件名定义---------------------------------------------
//#define MAX_FILENAME_SIZE			2048		//文件名最大长度

#define SOURCE_FILE_EXT				".FASM"		//汇编源文件后缀
#define EXEC_FILE_EXT				".FSE"		//可执行文件后缀

	//----源文件属性-------------------------------------------------
#define MAX_SOURCE_CODE_SIZE		65536		//源文件最大行数
#define	MAX_SOURCE_LINE_SIZE		4096		//每行最大长度

	//----可执行文件头-----------------------------------------------
#define XSE_ID_STRING				"FSE0"
#define VERSION_MAJOR				0			//主版本号
#define VERSION_MINOR				1			//副版本号

#define INSTR_LEN					32			//指令长度

	//----词法分析器-------------------------------------------------
//#define MAX_LEXEME_SIZE 256						//符号最大长度

#define LEX_STATE_NO_STRING			0			//词法分析器状态，标志是否在分析字符串字面量
#define LEX_STATE_IN_STRING			1
#define LEX_STATE_END_STRING		2

	//----属性字类型常量---------------------------------------------
#define TOKEN_TYPE_INT				0			//整型
#define TOKEN_TYPE_INT_DEF			1			//声明整型变量
#define TOKEN_TYPE_STRING			2			//字符串，不包括字符串
#define TOKEN_TYPE_STRING_DEF		3			//声明字符串变量
#define TOKEN_TYPE_PTR				4			//声明指针
#define TOKEN_TYPE_QUOTE			5			//双引号
#define TOKEN_TYPE_IDENT			6			//标识符
#define TOKEN_TYPE_COLON			7			//冒号
#define TOKEN_TYPE_OPEN_BRACKET		8			//左中括号
#define TOKEN_TYPE_CLOSE_BRACKET	9			//右中括号
#define TOKEN_TYPE_COMMA			10			//逗号
#define TOKEN_TYPE_OPEN_BRACE		11			//左大括号
#define TOKEN_TYPE_CLOSE_BRACE		12			//右大括号
#define TOKEN_TYPE_NEWLINE			13			//换行
#define TOKEN_TYPE_INSTR			14			//指令助记符
#define TOKEN_TYPE_SETSTACKSIZE		15			//设置堆栈大小指示符
	//#define TOKEN_TYPE_VAR				16			//var指示符
#define TOKEN_TYPE_FUNC				17			//Func指示符
	//#define TOKEN_TYPE_PARAM			18			//param指示符
	//#define TOKEN_TYPE_REG_RETVAL		19			//_RetVal寄存器
#define TOKEN_TYPE_INVALID			20			//错误编码
#define END_OF_TOKEN_STREAM			21			//到达属性字流的尾部
#define TOKEN_TYPE_R0				22			//0号寄存器
#define TOKEN_TYPE_R1				23			//0号寄存器
#define TOKEN_TYPE_R2				24			//0号寄存器
#define TOKEN_TYPE_R3				25			//0号寄存器

#define MAX_IDENT_SIZE 256						//最大标识符长度	

	//----指令查找表------------------------------------------------
//#define MAX_INSTR_LOOKUP_COUNT		256			//查找表将保存的指令的最大个数

//#define MAX_INSTR_MNEMONIC_SIZE		16			//指令助记符的最大长度

	//----指令码----------------------------------------------------
	//赋值
#define INSTR_MNEMONIC_MOV			0

	//算数
#define INSTR_MNEMONIC_ADD			1
#define INSTR_MNEMONIC_SUB			2
#define INSTR_MNEMONIC_MUL			3	
#define INSTR_MNEMONIC_DIV			4
	//跳转
#define INSTR_MNEMONIC_JMP			5
#define INSTR_MNEMONIC_JE			6
#define INSTR_MNEMONIC_JNE			7
#define INSTR_MNEMONIC_JG			8
#define INSTR_MNEMONIC_JL			9
	//堆栈
#define INSTR_MNEMONIC_PUSH			10
#define INSTR_MNEMONIC_POP			11
	//函数调用
#define INSTR_MNEMONIC_CALL			12
#define INSTR_MNEMONIC_RET			13
#define INSTR_MNEMONIC_IRET			14
#define INSTR_MNEMONIC_CALLHOST		15
	//退出
#define INSTR_MNEMONIC_PAUSE		16
#define INSTR_MNEMONIC_QUIT			17

	//----指令操作数类型--------------------------------------------
#define OP_TYPE_INT					0		//整形字面常量值
#define OP_TYPE_PTR					1		//整形指针
#define OP_TYPE_STRING_REF			2		//字符串索引
#define OP_TYPE_ABS_STACK_INDEX		3		//绝对堆栈索引
#define OP_TYPE_REL_STACK_INDEX		4		//相对堆栈索引
#define OP_TYPE_INSTR_INDEX			5		//指令索引
#define OP_TYPE_FUNC_INDEX			6		//函数索引
#define OP_TYPE_HOST_API_CALL_INDEX	7		//主应用程序API调用索引
#define OP_TYPE_REG					8		//寄存器

	//----指令支持的操作数类型掩码---------------------------------
#define OP_FLAG_TYPE_INT			1		//整形字面值
#define OP_FLAG_TYPE_PTR			2		//整形指针
#define OP_FLAG_TYPE_STRING_REF		4		//字符串索引
#define OP_FLAG_TYPE_MEM_REF		8		//内存引用
#define OP_FLAG_TYPE_LINE_LABEL		16		//行标签
#define OP_FLAG_TYPE_FUNC_NAME		32		//函数名
#define OP_FLAG_TYPE_CORE_API_CALL	64		//主应用程序API
#define OP_FLAG_TYPE_REG			128		//寄存器

	//----主函数名------------------------------------------------
#define MAIN_FUNC_NAME				"MAIN"

	//----错误提示------------------------------------------------
#define ERROR_MSSG_INVALID_INPUT	\
			"Invalid input"

#define ERROR_MSSG_LOCAL_SETSTACKSIZE	\
			"SetStackSize can only appear in the global scope"

#define ERROR_MSSG_LOCAL_DEF_STRING \
			"String can only define in the global scope"

#define ERROR_MSSG_STRING_NOT_DEF \
			"String should be define"

#define ERROR_MSSG_INVALID_STACK_SIZE	\
			"Invalid stack size"

#define ERROR_MSSG_MULTIPLE_SETSTACKSIZES	\
			"Multiple instances of SetStackSize illegal"

#define ERROR_MSSG_IDENT_EXPECTED	\
			"Identifier expected"

#define ERROR_MSSG_INVALID_ARRAY_SIZE	\
			"Invalid array size"

#define ERROR_MSSG_IDENT_REDEFINITION	\
			"Identifier redefinition"

#define ERROR_MSSG_UNDEFINED_IDENT	\
			"Undefined identifier"

#define ERROR_MSSG_NESTED_FUNC	\
			"Nested functions illegal"

#define ERROR_MSSG_FUNC_REDEFINITION	\
			"Function redefinition"

#define ERROR_MSSG_UNDEFINED_FUNC	\
			"Undefined function"

#define ERROR_MSSG_GLOBAL_PARAM	\
			"Parameters can only appear inside functions"

#define ERROR_MSSG_MAIN_PARAM	\
			"_Main () function cannot accept parameters"

#define ERROR_MSSG_GLOBAL_LINE_LABEL	\
			"Line labels can only appear inside functions"

#define ERROR_MSSG_LINE_LABEL_REDEFINITION	\
			"Line label redefinition"

#define ERROR_MSSG_UNDEFINED_LINE_LABEL	\
			"Undefined line label"

#define ERROR_MSSG_GLOBAL_INSTR	\
			"Instructions can only appear inside functions"

#define ERROR_MSSG_INVALID_INSTR	\
			"Invalid instruction"

#define ERROR_MSSG_INVALID_OP	\
			"Invalid operand"

#define ERROR_MSSG_INVALID_STRING	\
			"Invalid string"

#define ERROR_MSSG_INVALID_ARRAY_NOT_INDEXED	\
			"Arrays must be indexed"

#define ERROR_MSSG_INVALID_ARRAY	\
			"Invalid array"

#define ERROR_MSSG_INVALID_ARRAY_INDEX	\
			"Invalid array index"

	//----错误码--------------------------------------------------
#define ERR_NO_ERR						0x0000u			//无错误
#define ERR_FILE_NOT_FOUND				0x0001u			//无效文件
#define ERR_MALLOC_FAILED				0x0002u			//动态申请无效
#define ERR_BUILD_FILE_FAILED			0x0003u			//创建可执行文件失败
#define ERR_LOCAL_SETSTACKSIZE			0x0004u			//SetStackSize只能出现在全局范围
#define ERR_MULTIPLE_SETSTACKSIZE		0x0005u			//SetStackSize多次出现
#define ERR_INVALID_STACK_SIZE			0x0006u			//堆栈大小值无效
#define ERR_LOCAL_DEF_STRING			0x0007u			//字符串只能定义为全局常量
#define ERR_IDENT_EXPECTED				0x0008u			//变量声明后没有变量名
#define ERR_STR_NOT_DEF					0x0009u			//字符串未定义
#define	ERR_INVALID_STR					0x000au			//无效字符串
#define ERR_IDENT_REDEFINITION			0x000bu			//变量名重定义
#define ERR_INVALID_ARRAY_SIZE			0x000cu			//数组下标无效
#define ERR_NESTED_FUNC					0x000du			//函数嵌套定义
#define ERR_FUNC_REDEFINE				0x000eu			//函数重复定义
#define ERR_GLOBAL_INSTR				0x000fu			//全局指令
#define ERR_INVALID_LABEL				0x0010u			//无效行标签
#define ERR_GLOBAL_LABEL				0x0011u			//全局行标签
#define ERR_LABEL_REDEFINE				0x0012u			//行标签重定义
#define ERR_INVALID_OP					0x0013u			//无效操作数
#define ERR_UNDEF_IDENT					0x0014u			//操作数未定义
#define ERR_ARR_NOT_INDEXED				0x0015u			//数组未加下标
#define ERR_UNDEF_FUNC					0x0016u			//调用未定义函数
#define ERR_CHAR_EXPECTED				0x0017u			//语法错误
#define ERR_INVALID_INPUT				0x0018u			//无效输入
}

