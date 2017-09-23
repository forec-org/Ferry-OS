#include "compiler.h"
#include "fasmconfig.h"
#include "config.h"
#include "fs.h"

FASM *FASM::gInstance = nullptr;

FASM::FASM() {
	g_ppstrSourceCode = nullptr;
	g_iSourceCodeSize = 0;
	g_pSourceFile = nullptr;

	g_pInstrStream = nullptr;
}

void strupr(char * src) {
    for (unsigned int i = 0; i < strlen(src); i++) {
        if ('a' <= src[i] && src[i] <= 'z')
            src[i] -= ('a' - 'A');
    }
}

FASM::~FASM() {
}

unsigned int FASM::Exec(const std::string& srcFileName, const std::string &destFileName) {
	unsigned int err;
	
	PrintLogo();
    std::string srcPath = FS::getInstance()->getPath(srcFileName);
    std::string dstPath = FS::getInstance()->getPath(destFileName);

    SetSourceFileName(srcPath, dstPath);

	Init();

	err = LoadSourceFile();

	if (err != ERR_NO_ERR) {
		return err;
	}

	err = AssmblSourceFile();

	if (err != ERR_NO_ERR) {
		return err;
	}

	err = BuildFSE();

	if (err != ERR_NO_ERR) {
		return err;
	}

	PrintAssmblStats();

	ShutDown();
	return ERR_NO_ERR;
}

void FASM::SetSourceFileName(const std::string &srcFileName, const std::string &destFileName) {
	strcpy(g_pstrSourceFilename, srcFileName.c_str());

	strupr(g_pstrSourceFilename);

    if (!strstr(g_pstrSourceFilename, SOURCE_FILE_EXT)) {
		strcat(g_pstrSourceFilename, SOURCE_FILE_EXT);
	}

	if (!destFileName.empty()) {
		// Yes, so repeat the validation process

		strcpy(g_pstrExecFilename, destFileName.c_str());
		strupr(g_pstrExecFilename);

		// Check for the presence of the .XSE extension and add it if it's not there

		if (!strstr(g_pstrExecFilename, Config::getInstance()->OS.EXEC_FILE_EXT.c_str())) {
			// The extension was not found, so add it to string

			strcat(g_pstrExecFilename, Config::getInstance()->OS.EXEC_FILE_EXT.c_str());
		}
	}
	else {
		// No, so base it on the source filename

		// First locate the start of the extension, and use pointer subtraction to find the index

		int ExtOffset = strrchr(g_pstrSourceFilename, '.') - g_pstrSourceFilename;
		strncpy(g_pstrExecFilename, g_pstrSourceFilename, ExtOffset);

		// Append null terminator

		g_pstrExecFilename[ExtOffset] = '\0';

		// Append executable extension

		strcat(g_pstrExecFilename, Config::getInstance()->OS.EXEC_FILE_EXT.c_str());
	}
}

void FASM::InitLinkedList(LinkedList * pList) {
	// Set both the head and tail pointers to null
	pList->pHead = NULL;
	pList->pTail = NULL;

	// Set the node count to zero, since the list is currently empty
	pList->iNodeCount = 0;
}

void FASM::FreeLinkedList(LinkedList * pList) {
	// If the list is empty, exit

	if (!pList)
		return;

	// If the list is not empty, free each node

	if (pList->iNodeCount) {
		// Create a pointer to hold each current node and the next node

		LinkedListNode * pCurrNode,
			*pNextNode;

		// Set the current node to the head of the list

		pCurrNode = pList->pHead;

		// Traverse the list

		while (true) {
			// Save the pointer to the next node before freeing the current one

			pNextNode = pCurrNode->pNext;

			// Clear the current node's data

			if (pCurrNode->pData)
				free(pCurrNode->pData);

			// Clear the node itself

			if (pCurrNode)
				free(pCurrNode);

			// Move to the next node if it exists; otherwise, exit the loop

			if (pNextNode)
				pCurrNode = pNextNode;
			else
				break;
		}
	}
}

int FASM::AddNode(LinkedList * pList, void * pData) {
	// Create a new node

	LinkedListNode * pNewNode = (LinkedListNode *)malloc(sizeof(LinkedListNode));

	// Set the node's data to the specified pointer

	pNewNode->pData = pData;

	// Set the next pointer to NULL, since nothing will lie beyond it

	pNewNode->pNext = NULL;

	// If the list is currently empty, set both the head and tail pointers to the new node

	if (!pList->iNodeCount) {
		// Point the head and tail of the list at the node

		pList->pHead = pNewNode;
		pList->pTail = pNewNode;
	}

	// Otherwise append it to the list and update the tail pointer

	else {
		// Alter the tail's next pointer to point to the new node

		pList->pTail->pNext = pNewNode;

		// Update the list's tail pointer

		pList->pTail = pNewNode;
	}

	// Increment the node count

	++pList->iNodeCount;

	// Return the new size of the linked list - 1, which is the node's index

	return pList->iNodeCount - 1;
}

void FASM::StripComments(char * pstrSourceLine) {
	int iCurrCharIndex;
	int iInString;

	// Scan through the source line and terminate the string at the first semicolon

	iInString = 0;
	for (iCurrCharIndex = 0; iCurrCharIndex < strlen(pstrSourceLine) - 1; ++iCurrCharIndex) {
		// Look out for strings; they can contain semicolons

		if (pstrSourceLine[iCurrCharIndex] == '"') {
            if (iInString)
                iInString = 0;
            else
                iInString = 1;
        }

		// If a non-string semicolon is found, terminate the string at it's position

		if (pstrSourceLine[iCurrCharIndex] == ';') {
			if (!iInString) {
				pstrSourceLine[iCurrCharIndex] = '\n';
				pstrSourceLine[iCurrCharIndex + 1] = '\0';
				break;
			}
		}
	}
}

void FASM::PrintLogo() {
	printf("FASM [Version 0.1] Written by FlyingX\n");
}

void FASM::PrintUsage() {
	printf("Usage:\tFASM Source.FASM [Executable.FSE]\n");
	printf("\n");
	printf("\t- File extensions are not required.\n");
	printf("\t- Executable name is optional; source name is used by default.\n");
}

void FASM::Init() {
	InitInstrTable();
	FASMDebug("init instr table\n", 3);
	// Initialize tables

	InitLinkedList(&g_SymbolTable);
	FASMDebug("init symbol table\n", 3);
	InitLinkedList(&g_LabelTable);
	FASMDebug("init label table\n", 3);
	InitLinkedList(&g_FuncTable);
	FASMDebug("init func table\n", 3);
	InitLinkedList(&g_StringTable);
	FASMDebug("init string table\n", 3);
	InitLinkedList(&g_HostAPICallTable);
	FASMDebug("init host api table\n", 3);
}

void FASM::ShutDown() {
	// ---- Free source code array

	// Free each source line individually

	for (int iCurrLineIndex = 0; iCurrLineIndex < g_iSourceCodeSize; ++iCurrLineIndex)
		free(g_ppstrSourceCode[iCurrLineIndex]);

	// Now free the base pointer

	free(g_ppstrSourceCode);

	// ---- Free the assembled instruction stream

	if (g_pInstrStream) {
		// Free each instruction's operand list

		for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++iCurrInstrIndex)
			if (g_pInstrStream[iCurrInstrIndex].pOpList)
				free(g_pInstrStream[iCurrInstrIndex].pOpList);

		// Now free the stream itself

		free(g_pInstrStream);
	}

	// ---- Free the tables

	FreeLinkedList(&g_SymbolTable);
	FreeLinkedList(&g_LabelTable);
	FreeLinkedList(&g_FuncTable);
	FreeLinkedList(&g_StringTable);
	FreeLinkedList(&g_HostAPICallTable);
}

unsigned int FASM::LoadSourceFile() {

	if (!(g_pSourceFile = fopen(g_pstrSourceFilename, "rb"))) {
		ExitOnError("Could not open source file");
		return ERR_FILE_NOT_FOUND;
	}
	
	while (!feof(g_pSourceFile))
		if (fgetc(g_pSourceFile) == '\n')
			++g_iSourceCodeSize;
	++g_iSourceCodeSize;

	fclose(g_pSourceFile);

	// Reopen the source file in ASCII mode

	if (!(g_pSourceFile = fopen(g_pstrSourceFilename, "r"))) {
		ExitOnError("Could not open source file");
		return ERR_FILE_NOT_FOUND;
	}
		

	// Allocate an array of strings to hold each source line

	if (!(g_ppstrSourceCode = (char **)malloc(g_iSourceCodeSize * sizeof(char *)))) {
		ExitOnError("Could not allocate space for source code");
		return ERR_MALLOC_FAILED;
	}
	
	// Read the source code in from the file

	for (int iCurrLineIndex = 0; iCurrLineIndex < g_iSourceCodeSize; ++iCurrLineIndex) {
		// Allocate space for the line

		if (!(g_ppstrSourceCode[iCurrLineIndex] = (char *)malloc(MAX_SOURCE_LINE_SIZE + 1))) {
			ExitOnError("Could not allocate space for source line");
			return ERR_MALLOC_FAILED;
		}


		// Read in the current line

		fgets(g_ppstrSourceCode[iCurrLineIndex], MAX_SOURCE_LINE_SIZE, g_pSourceFile);

		// Strip comments and trim whitespace

		StripComments(g_ppstrSourceCode[iCurrLineIndex]);
		TrimWhitespace(g_ppstrSourceCode[iCurrLineIndex]);

		// Make sure to add a new newline if it was removed by the stripping of the
		// comments and whitespace. We do this by checking the character right before
		// the null terminator to see if it's \n. If not, we move the terminator over
		// by one and add it. We use strlen () to find the position of the newline
		// easily.

		int iNewLineIndex = strlen(g_ppstrSourceCode[iCurrLineIndex]) - 1;
		if (g_ppstrSourceCode[iCurrLineIndex][iNewLineIndex] != '\n') {
			g_ppstrSourceCode[iCurrLineIndex][iNewLineIndex + 1] = '\n';
			g_ppstrSourceCode[iCurrLineIndex][iNewLineIndex + 2] = '\0';
		}
	}

	// Close the source file

	fclose(g_pSourceFile);
	FASMDebug("-----------------------------------------------------------------------\n", 3);
	for (int i = 0; i < g_iSourceCodeSize; ++i) {
		FASMDebug(g_ppstrSourceCode[i], 3);
	}
	FASMDebug("-----------------------------------------------------------------------\n", 3);
	FASMDebug(g_iSourceCodeSize, 3);
	FASMDebug("loading finish\n", 3);
	return ERR_NO_ERR;
}

unsigned int FASM::AssmblSourceFile() {
	// ---- Initialize the script header

	g_ScriptHeader.iStackSize = 0;
	g_ScriptHeader.iIsMainFuncPresent = false;

	// ---- Set some initial variables

	g_iInstrStreamSize = 0;
	g_iIsSetStackSizeFound = false;
	g_ScriptHeader.iGlobalDataSize = 0;

	// Set the current function's flags and variables

	int iIsFuncActive = false;							//是否在函数体内							
	FuncNode * pCurrFunc;								//保存当前函数信息节点
	int iCurrFuncIndex;									//当前函数索引
	char pstrCurrFuncName[MAX_IDENT_SIZE];				//当前函数名
	int iCurrFuncLocalDataSize = 0;						//当前函数局部变量

														// Create an instruction definition structure to hold instruction information when
														// dealing with instructions.

	InstrLookup CurrInstr;

	// ---- Perform first pass over the source

	// Reset the lexer

	FASMDebug("start first loop\n", 3);

	ResetLexer();

	//第一轮扫描,保存行标签，函数定义，变量等信息
	while (true) {
		//保证没有超过属性字流的尾部
		if (GetNextToken() == END_OF_TOKEN_STREAM)
			break;

		//处理不同的属性字
		switch (g_Lexer.CurrToken) {
			// ---- Start by checking for directives

			// SetStackSize

		case TOKEN_TYPE_SETSTACKSIZE:

			//	SetStackSize 只能出现在全局范围，所以应确保不在函数内部
			if (iIsFuncActive) {
				ExitOnCodeError(ERROR_MSSG_LOCAL_SETSTACKSIZE);
				return ERR_LOCAL_SETSTACKSIZE;
			}


			//只能出现一次
			if (g_iIsSetStackSizeFound) {
				ExitOnCodeError(ERROR_MSSG_MULTIPLE_SETSTACKSIZES);
				return ERR_MULTIPLE_SETSTACKSIZE;
			}
			
			//读取下一个单词，应该包含堆栈大小
			if (GetNextToken() != TOKEN_TYPE_INT) {
				ExitOnCodeError(ERROR_MSSG_INVALID_STACK_SIZE);
				return ERR_INVALID_STACK_SIZE;
			}
				
			//把这个单词从字符串的表示转换成一个整形值，并存储在脚本头中
			g_ScriptHeader.iStackSize = atoi(GetCurrLexeme());

			//标记SetStackSize已经出现过
			g_iIsSetStackSizeFound = true;

			break;

		case TOKEN_TYPE_STRING_DEF:
		{
			//字符串只能定义为全局变量
			if (iIsFuncActive) {
				ExitOnCodeError(ERROR_MSSG_LOCAL_DEF_STRING);
				return ERR_LOCAL_DEF_STRING;
			}
			//下一个词法单元应该为字符串名
			if (GetNextToken() != TOKEN_TYPE_IDENT) {
				ExitOnCodeError(ERROR_MSSG_IDENT_EXPECTED);
				return ERR_IDENT_EXPECTED;
			}
			
			//获取变量名
			char pstrIdent[MAX_IDENT_SIZE];
			strcpy(pstrIdent, GetCurrLexeme());

			if (GetNextToken() != TOKEN_TYPE_QUOTE) {
				ExitOnCodeError(ERROR_MSSG_STRING_NOT_DEF);
				return ERR_STR_NOT_DEF;
			}

			GetNextToken();

			if (g_Lexer.CurrToken == TOKEN_TYPE_QUOTE) {
				ExitOnCodeError(ERROR_MSSG_STRING_NOT_DEF);
				return ERR_STR_NOT_DEF;
			}

			int iStringIndex;

			if (g_Lexer.CurrToken == TOKEN_TYPE_STRING) {
				char *pstrString = GetCurrLexeme();
				iStringIndex = AddString(&g_StringTable, pstrString);
				//获取字符串表中的绝对偏移量（单位是字节）

				if (GetNextToken() != TOKEN_TYPE_QUOTE) {
					ExitOnCharExpectedError('\\');
					return ERR_CHAR_EXPECTED;
				}
			}

			bool bIsArr = false;
			int iSize = 4;
			int iStackIndex = g_ScriptHeader.iGlobalDataSize;
			g_ScriptHeader.iGlobalDataSize += iSize;
			//stackIndex为正表示全局变量，为负表示局部变量
			if (AddSymbol(pstrIdent, OP_TYPE_STRING_REF, iStringIndex, bIsArr, iSize, iStackIndex, iCurrFuncIndex) == -1) {
				ExitOnCodeError(ERROR_MSSG_IDENT_REDEFINITION);
				return ERR_IDENT_REDEFINITION;
			}
			break;
		}


			// Int/Int []
		case TOKEN_TYPE_INT_DEF:
		{
			//	获取变量名，如果不是标识符，则提示错误
			if (GetNextToken() != TOKEN_TYPE_IDENT) {
				ExitOnCodeError(ERROR_MSSG_IDENT_EXPECTED);
				return ERR_IDENT_EXPECTED;
			}
			
			char pstrIdent[MAX_IDENT_SIZE];
			strcpy(pstrIdent, GetCurrLexeme());

			//通过检查是否是数组判定它的大小，否则默认为1
			int iSize = sizeof(int);

			bool bIsArr = false;

			//向前看检查是否有左中括号
			if (GetLookAheadChar() == '[') {
				//确认左中括号
				if (GetNextToken() != TOKEN_TYPE_OPEN_BRACKET) {
					ExitOnCharExpectedError('[');
					return ERR_CHAR_EXPECTED;
				}

				//因为是分析数组，所以下一个单词应该是描述数组大小的整数
				if (GetNextToken() != TOKEN_TYPE_INT) {
					ExitOnCodeError(ERROR_MSSG_INVALID_ARRAY_SIZE);
					return ERR_INVALID_ARRAY_SIZE;
				}

				iSize = atoi(GetCurrLexeme()) * sizeof(int);

				if (iSize <= 0) {
					ExitOnCodeError(ERROR_MSSG_INVALID_ARRAY_SIZE);
					return ERR_INVALID_ARRAY_SIZE;
				}

				//确保右中括号存在
				if (GetNextToken() != TOKEN_TYPE_CLOSE_BRACKET) {
					ExitOnCharExpectedError(']');
					return ERR_CHAR_EXPECTED;
				}
				bIsArr = true;
			}

			int iStackIndex;

			//判断变量在堆栈中的索引
			//如果是局部变量，那么它的堆栈索引是
			//用零减去局部变量数据大小+2的值
			if (iIsFuncActive) {
				iCurrFuncLocalDataSize += iSize;
				iStackIndex = -(iCurrFuncLocalDataSize);
			}
			//否则就是全局变量，相当于目前全局数据的个数
			else {
				iStackIndex = g_ScriptHeader.iGlobalDataSize;
				g_ScriptHeader.iGlobalDataSize += iSize;
			}

			//stackIndex为正表示全局变量，为负表示局部变量
			if (AddSymbol(pstrIdent, OP_TYPE_INT, 0, bIsArr, iSize, iStackIndex, iCurrFuncIndex) == -1) {
				ExitOnCodeError(ERROR_MSSG_IDENT_REDEFINITION);
				return ERR_IDENT_REDEFINITION;
			}
			//if (iIsFuncActive)
			//	
			//else
			break;
		}

		// Func
		case TOKEN_TYPE_FUNC:
		{
			//首先确认不在函数内部， 因为嵌套函数定义不合法
			if (iIsFuncActive) {
				ExitOnCodeError(ERROR_MSSG_NESTED_FUNC);
				return ERR_NESTED_FUNC;
			}
			
			//读取下一个单词，函数名称
			if (GetNextToken() != TOKEN_TYPE_IDENT) {
				ExitOnCodeError(ERROR_MSSG_IDENT_EXPECTED);
				return ERR_IDENT_EXPECTED;
			}
				

			char * pstrFuncName = GetCurrLexeme();

			//计算函数的入口点，即直接跟在当前指令后面的指令
			//也就相当于当前指令流大小
			int iEntryPoint = g_iInstrStreamSize;

			//试图把它添加到函数表，如果已经被声明过打印重定义错误
			int iFuncIndex = AddFunc(pstrFuncName, iEntryPoint);
			if (iFuncIndex == -1) {
				ExitOnCodeError(ERROR_MSSG_FUNC_REDEFINITION);
				return ERR_FUNC_REDEFINE;
			}

			//是否函数main()
			if (strcmp(pstrFuncName, MAIN_FUNC_NAME) == 0) {
				g_ScriptHeader.iIsMainFuncPresent = true;
				g_ScriptHeader.iMainFuncIndex = iFuncIndex;
			}

			//把函数标记设为true，并且重置函数的跟踪变量
			iIsFuncActive = true;
			strcpy(pstrCurrFuncName, pstrFuncName);
			iCurrFuncIndex = iFuncIndex;
			iCurrFuncLocalDataSize = 0;

			//读取换行符直到遇到左大括号
			while (GetNextToken() == TOKEN_TYPE_NEWLINE);

			//确认单词是个左大括号
			if (g_Lexer.CurrToken != TOKEN_TYPE_OPEN_BRACE) {
				ExitOnCharExpectedError('{');
				return ERR_CHAR_EXPECTED;
			}

			//所有的函数自动追加ret，所以增大指令流所需的大小
			++g_iInstrStreamSize;
			break;
		}

		// Closing bracket
		case TOKEN_TYPE_CLOSE_BRACE:
			//应该是函数结尾，保证在函数内部
			if (!iIsFuncActive) {
				ExitOnCharExpectedError('}');
				return ERR_CHAR_EXPECTED;
			}
			
			//设置收到的信息
			SetFuncInfo(pstrCurrFuncName, iCurrFuncLocalDataSize);

			//关闭分析函数状态
			iIsFuncActive = false;

			break;

			// ---- Instructions
		case TOKEN_TYPE_INSTR:
		{
			// Make sure we aren't in the global scope, since instructions
			// can only appear in functions

			if (!iIsFuncActive) {
				ExitOnCodeError(ERROR_MSSG_GLOBAL_INSTR);
				return ERR_GLOBAL_INSTR;
			}

			// Increment the instruction stream size

			++g_iInstrStreamSize;

			break;
		}

		// ---- Identifiers (line labels)
		case TOKEN_TYPE_IDENT:
		{
			//判断是否是行标签
			if (GetLookAheadChar() != ':') {
				ExitOnCodeError(ERROR_MSSG_INVALID_INSTR);
				return ERR_INVALID_LABEL;
			}
			
			//确保处于函数内部
			if (!iIsFuncActive) {
				ExitOnCodeError(ERROR_MSSG_GLOBAL_LINE_LABEL);
				return ERR_GLOBAL_LABEL;
			}
			
			char * pstrIdent = GetCurrLexeme();

			//目标指令通常是当前指令计数的值，等于当前大小减1
			int iTargetIndex = g_iInstrStreamSize - 1;

			//保存行标签的函数索引
			int iFuncIndex = iCurrFuncIndex;

			//尝试把行标签加入标签表，如果已经存在就打印错误信息
			if (AddLabel(pstrIdent, iTargetIndex, iFuncIndex) == -1) {
				ExitOnCodeError(ERROR_MSSG_LINE_LABEL_REDEFINITION);
				return ERR_LABEL_REDEFINE;
			}
			
			break;
		}

		default:
			//其他标识符
			if (g_Lexer.CurrToken != TOKEN_TYPE_NEWLINE) {
				ExitOnCodeError(ERROR_MSSG_INVALID_INPUT);
				return ERR_INVALID_INPUT;
			}
		}

		// Skip to the next line, since the initial tokens are all we're really worrid
		// about in this phase

		if (!SkipToNextLine())
			break;
	} // while (ture)

	FASMDebug("finish first loop\n", 3);

	  //根据第一轮扫描得出来的指令数量为指令流分配内存
	g_pInstrStream = (Instr *)malloc(g_iInstrStreamSize * sizeof(Instr));

	// 初始化每条指令操作数列表为NULL
	for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++iCurrInstrIndex)
		g_pInstrStream[iCurrInstrIndex].pOpList = NULL;

	//设置当前的指令索引为0
	g_iCurrInstrIndex = 0;

	// ---- Perform the second pass over the source

	FASMDebug("start second loop\n", 3);

	//重置词法分析器
	ResetLexer();

	//第二轮扫描
	while (true) {
		// Get the next token and make sure we aren't at the end of the stream

		if (GetNextToken() == END_OF_TOKEN_STREAM)
			break;

		// Check the initial token

		switch (g_Lexer.CurrToken) {
			// Func

		case TOKEN_TYPE_FUNC:
		{
			// We've encountered a Func directive, but since we validated the syntax
			// of all functions in the previous phase, we don't need to perform any
			// error handling here and can assume the syntax is perfect.

			//读取函数名称
			GetNextToken();

			//获取表示当前函数信息的结构体指针
			pCurrFunc = GetFuncByName(GetCurrLexeme());

			//标识当前正在分析函数
			iIsFuncActive = true;

			//保存函数索引
			iCurrFuncIndex = pCurrFunc->iIndex;

			//跳过多个换行
			while (GetNextToken() == TOKEN_TYPE_NEWLINE);
			break;
		}

		// Closing brace
		case TOKEN_TYPE_CLOSE_BRACE:
		{
			//退出函数分析状态
			iIsFuncActive = false;

			//如果当前分析的函数是主函数
			if (strcmp(pCurrFunc->pstrName, MAIN_FUNC_NAME) == 0) {
				//函数结尾添加一个结束指令
				g_pInstrStream[g_iCurrInstrIndex].iOpcode = INSTR_MNEMONIC_QUIT;

				//设置参数数量
				g_pInstrStream[g_iCurrInstrIndex].iOpCount = 0;
				g_pInstrStream[g_iCurrInstrIndex].pOpList = NULL;
			}

			//普通函数
			else {
				//函数结尾添加一个返回指令
				g_pInstrStream[g_iCurrInstrIndex].iOpcode = INSTR_MNEMONIC_RET;
				g_pInstrStream[g_iCurrInstrIndex].iOpCount = 0;
				g_pInstrStream[g_iCurrInstrIndex].pOpList = NULL;
			}
			++g_iCurrInstrIndex;
			break;
		}

		// Instructions

		case TOKEN_TYPE_INSTR:
		{
			//获取指令信息
			GetInstrByMnemonic(GetCurrLexeme(), &CurrInstr);

			//将当前指令码写入流中
			g_pInstrStream[g_iCurrInstrIndex].iOpcode = CurrInstr.iOpcode;

			//将当前指令的操作数数量写入
			g_pInstrStream[g_iCurrInstrIndex].iOpCount = CurrInstr.iOpCount;

			//分配指令列表
			Op * pOpList = (Op *)malloc(CurrInstr.iOpCount * sizeof(Op));

			//检查每个操作数
			for (int iCurrOpIndex = 0; iCurrOpIndex < CurrInstr.iOpCount; ++iCurrOpIndex) {
				// 读取合法操作数类型
				OpTypes CurrOpTypes = CurrInstr.OpList[iCurrOpIndex];

				// Read in the next token, which is the initial token of the operand

				Token InitOpToken = GetNextToken();
				switch (InitOpToken) {

					//整数常量
				case TOKEN_TYPE_INT:
					//判断当前参数的类型与指令支持的类型是否相符
					if (CurrOpTypes & OP_FLAG_TYPE_INT) {
						pOpList[iCurrOpIndex].iType = OP_TYPE_INT;
						pOpList[iCurrOpIndex].iIntLiteral = atoi(GetCurrLexeme());
					}
					else {
						ExitOnCodeError(ERROR_MSSG_INVALID_OP);
						return ERR_INVALID_OP;
					}
						
					break;

					// 寄存器
				case TOKEN_TYPE_R0: case TOKEN_TYPE_R1: case TOKEN_TYPE_R2: case TOKEN_TYPE_R3:
					if (CurrOpTypes & OP_FLAG_TYPE_REG) {
						pOpList[iCurrOpIndex].iType = OP_TYPE_REG;
						pOpList[iCurrOpIndex].iReg = InitOpToken - TOKEN_TYPE_R0;
					}
					else {
						ExitOnCodeError(ERROR_MSSG_INVALID_OP);
						return ERR_INVALID_OP;
					}
					
					break;

					//标识符
					//标识符可能的种类
					//		变量/数组
					//		行标签
					//		函数名
					//		主程序调用
				case TOKEN_TYPE_IDENT:
				{
					//分析是否是变量或者数组或者字符串或者指针
					if (CurrOpTypes & OP_FLAG_TYPE_MEM_REF) {
						char pstrIdent[MAX_IDENT_SIZE];
						strcpy(pstrIdent, GetCurrLexeme());

						// 从标识符表中查找判断是否被定义
						if (!GetSymbolByIdent(pstrIdent, iCurrFuncIndex)) {
							ExitOnCodeError(ERROR_MSSG_UNDEFINED_IDENT);
							return ERR_UNDEF_IDENT;
						}
							
						//	获取标识符的索引，如果是单个变量就是绝对索引，如果是数组就作为
						//	基地址
						int iBaseIndex = GetStackIndexByIdent(pstrIdent, iCurrFuncIndex);

						//如果不是数组
						if (GetLookAheadChar() != '[') {
							if (GetIsArrByIdent(pstrIdent, iCurrFuncIndex) == true)
								ExitOnCodeError(ERROR_MSSG_INVALID_ARRAY_NOT_INDEXED);

							//如果是字符串索引
							SymbolNode *pSymbol = GetSymbolByIdent(pstrIdent, iCurrFuncIndex);
							if (pSymbol->iType == OP_TYPE_STRING_REF) {
								pOpList[iCurrOpIndex].iType = OP_TYPE_STRING_REF;
								pOpList[iCurrOpIndex].iStringTableIndex = pSymbol->iInitValue;
							}
							else {
								//设置操作类型为绝对堆栈索引
								pOpList[iCurrOpIndex].iType = OP_TYPE_ABS_STACK_INDEX;
								pOpList[iCurrOpIndex].iIntLiteral = iBaseIndex;
							}
						}
						//如果是数组，则使用时需要有下标
						else {
							if (GetIsArrByIdent(pstrIdent, iCurrFuncIndex) == false) {
								ExitOnCodeError(ERROR_MSSG_INVALID_ARRAY);
								return ERR_ARR_NOT_INDEXED;
							}
							
							// 确认是否有中括号
							if (GetNextToken() != TOKEN_TYPE_OPEN_BRACKET) {
								ExitOnCharExpectedError('[');
								return ERR_CHAR_EXPECTED;
							}

							// 下一个单元必须是数值常量或者变量
							Token IndexToken = GetNextToken();

							if (IndexToken == TOKEN_TYPE_INT) {
								//如果是常量则使用绝对索引
								int iOffsetIndex = atoi(GetCurrLexeme());

								pOpList[iCurrOpIndex].iType = OP_TYPE_ABS_STACK_INDEX;
								pOpList[iCurrOpIndex].iStackIndex = iBaseIndex + iOffsetIndex;
							}
							else if (IndexToken == TOKEN_TYPE_IDENT) {
								// 如果是变量则使用相对索引

								char * pstrIndexIdent = GetCurrLexeme();

								// Make sure the index is a valid array index, in
								// that the identifier represents a single variable
								// as opposed to another array

								if (!GetSymbolByIdent(pstrIndexIdent, iCurrFuncIndex)) {
									ExitOnCodeError(ERROR_MSSG_UNDEFINED_IDENT);
									return ERR_UNDEF_IDENT;
								}
								
								if (GetSizeByIdent(pstrIndexIdent, iCurrFuncIndex) > 1) {
									ExitOnCodeError(ERROR_MSSG_INVALID_ARRAY_INDEX);
									return ERR_INVALID_ARRAY_SIZE;
								}
								
								// 如果是变量则使用相对索引

								int iOffsetIndex = GetStackIndexByIdent(pstrIndexIdent, iCurrFuncIndex);

								pOpList[iCurrOpIndex].iType = OP_TYPE_REL_STACK_INDEX;
								pOpList[iCurrOpIndex].iStackIndex = iBaseIndex;		//数组基地址
								pOpList[iCurrOpIndex].iOffsetIndex = iOffsetIndex;	//变量的索引
							}
							else {
								// Whatever it is, it's invalid

								ExitOnCodeError(ERROR_MSSG_INVALID_ARRAY_INDEX);
								return ERR_INVALID_ARRAY_SIZE;
							}

							// Lastly, make sure the closing brace is present as well

							if (GetNextToken() != TOKEN_TYPE_CLOSE_BRACKET) {
								ExitOnCharExpectedError('[');
								return ERR_CHAR_EXPECTED;
							}
							
						}
					} // if (CurrOpTypes & OP_FLAG_TYPE_MEM_REF)

					  // 分析行标签
					if (CurrOpTypes & OP_FLAG_TYPE_LINE_LABEL) {
						// Get the current lexeme, which is the line label

						char * pstrLabelIdent = GetCurrLexeme();

						//查找行标签
						LabelNode * pLabel = GetLabelByIdent(pstrLabelIdent, iCurrFuncIndex);

						//确认行标签存在
						if (!pLabel) {
							ExitOnCodeError(ERROR_MSSG_UNDEFINED_LINE_LABEL);
							return ERR_INVALID_LABEL;
						}


						// 设置类型和值
						pOpList[iCurrOpIndex].iType = OP_TYPE_INSTR_INDEX;
						pOpList[iCurrOpIndex].iInstrIndex = pLabel->iTargetIndex;
					}

					// 分析函数名
					if (CurrOpTypes & OP_FLAG_TYPE_FUNC_NAME) {
						char * pstrFuncName = GetCurrLexeme();

						// 用函数名查找函数表
						FuncNode * pFunc = GetFuncByName(pstrFuncName);

						// 确保函数存在
						if (!pFunc) {
							ExitOnCodeError(ERROR_MSSG_UNDEFINED_FUNC);
							return ERR_UNDEF_FUNC;
						}

						// 设置操作数的值为函数表索引
						pOpList[iCurrOpIndex].iType = OP_TYPE_FUNC_INDEX;
						pOpList[iCurrOpIndex].iFuncIndex = pFunc->iIndex;
					}

					// 分析内核函数名
					if (CurrOpTypes & OP_FLAG_TYPE_CORE_API_CALL) {
						char * pstrHostAPICall = GetCurrLexeme();

						int iIndex = AddString(&g_HostAPICallTable, pstrHostAPICall);

						pOpList[iCurrOpIndex].iType = OP_TYPE_HOST_API_CALL_INDEX;
						pOpList[iCurrOpIndex].iHostAPICallIndex = iIndex;
					}

					break;
				}
				default:
					ExitOnCodeError(ERROR_MSSG_INVALID_OP);
					return ERR_INVALID_OP;
					break;
				}

				// Make sure a comma follows the operand, unless it's the last one

				if (iCurrOpIndex < CurrInstr.iOpCount - 1)
					if (GetNextToken() != TOKEN_TYPE_COMMA) {
						ExitOnCharExpectedError(',');
						return ERR_CHAR_EXPECTED;
					}

			}

			// Make sure there's no extranous stuff ahead

			if (GetNextToken() != TOKEN_TYPE_NEWLINE) {
				return ERR_INVALID_INPUT;
				ExitOnCodeError(ERROR_MSSG_INVALID_INPUT);
			}
			
			// Copy the operand list pointer into the assembled stream

			g_pInstrStream[g_iCurrInstrIndex].pOpList = pOpList;

			// Move along to the next instruction in the stream

			++g_iCurrInstrIndex;

			break;
		}
		}

		// Skip to the next line

		if (!SkipToNextLine())
			break;
	}
	FASMDebug("finish second loop\n", 3);
	return ERR_NO_ERR;
}

void FASM::PrintAssmblStats() {
	// ---- Calculate statistics

	// Symbols

	// Create some statistic variables

	int iVarCount = 0,
		iArrayCount = 0,
		iGlobalCount = 0;

	// Create a pointer to traverse the list

	LinkedListNode * pCurrNode = g_SymbolTable.pHead;

	// Traverse the list to count each symbol type

	for (int iCurrNode = 0; iCurrNode < g_SymbolTable.iNodeCount; ++iCurrNode) {
		// Create a pointer to the current symbol structure

		SymbolNode * pCurrSymbol = (SymbolNode *)pCurrNode->pData;

		// It's an array if the size is greater than 1

		if (pCurrSymbol->iSize > 1)
			++iArrayCount;

		// It's a variable otherwise

		else
			++iVarCount;

		// It's a global if it's stack index is nonnegative

		if (pCurrSymbol->iStackIndex >= 0)
			++iGlobalCount;

		// Move to the next node

		pCurrNode = pCurrNode->pNext;
	}

	// Print out final calculations

	printf("%s created successfully!\n\n", g_pstrExecFilename);
	printf("Source Lines Processed: %d\n", g_iSourceCodeSize);

	printf("            Stack Size: ");
	if (g_ScriptHeader.iStackSize)
		printf("%d", g_ScriptHeader.iStackSize);
	else
		printf("Default");

	printf("\n");
	printf("Instructions Assembled: %d\n", g_iInstrStreamSize);
	printf("             Variables: %d\n", iVarCount);
	printf("                Arrays: %d\n", iArrayCount);
	printf("               Globals: %d\n", iGlobalCount);
	printf("       String Literals: %d\n", g_StringTable.iNodeCount);
	printf("                Labels: %d\n", g_LabelTable.iNodeCount);
	printf("        Host API Calls: %d\n", g_HostAPICallTable.iNodeCount);
	printf("             Functions: %d\n", g_FuncTable.iNodeCount);

	printf("      _Main () Present: ");
	if (g_ScriptHeader.iIsMainFuncPresent)
		printf("Yes (Index %d)\n", g_ScriptHeader.iMainFuncIndex);
	else
		printf("No\n");
}

unsigned int FASM::BuildFSE() {
	// ---- Open the output file

	FILE * pExecFile;
	if (!(pExecFile = fopen(g_pstrExecFilename, "wb"))) {
		ExitOnError("Could not open executable file for output");
		return ERR_BUILD_FILE_FAILED;
	}


	// ---- 写文件头

	// 写ID字符串 (4 bytes)
	fwrite(XSE_ID_STRING, 4, 1, pExecFile);
	FASMDebug("id: ", 3);
	FASMDebug(XSE_ID_STRING, 3);
	FASMDebug("\n", 3);

	// 写版本号 (1 byte for each component, 2 total)
	short cVersionMajor = VERSION_MAJOR,
		cVersionMinor = VERSION_MINOR;
	fwrite(&cVersionMajor, 2, 1, pExecFile);
	fwrite(&cVersionMinor, 2, 1, pExecFile);
	FASMDebug("version: ", 3);
	FASMDebug(VERSION_MAJOR, 3);
	FASMDebug(". ", 3);
	FASMDebug(VERSION_MINOR, 3);
	FASMDebug("\n", 3);

	// 写堆栈大小 (4 bytes)

	fwrite(&g_ScriptHeader.iStackSize, 4, 1, pExecFile);
	FASMDebug("stack size: ", 3);
	FASMDebug(g_ScriptHeader.iStackSize, 3);
	FASMDebug("\n", 3);

	// 写全局数据段大小 (4 bytes )

	fwrite(&g_ScriptHeader.iGlobalDataSize, 4, 1, pExecFile);
	FASMDebug("global data size: ", 3);
	FASMDebug(g_ScriptHeader.iGlobalDataSize, 3);
	FASMDebug("\n", 3);

	// 写主函数索引 (4 bytes)

	fwrite(&g_ScriptHeader.iMainFuncIndex, 4, 1, pExecFile);

	FASMDebug("main function index: ", 3);
	FASMDebug(g_ScriptHeader.iMainFuncIndex, 3);
	FASMDebug("\n", 3);

	// 写指令流

	// 指令数 (4 bytes)

	fwrite(&g_iInstrStreamSize, 4, 1, pExecFile);

	FASMDebug("instr num: ", 3);
	FASMDebug(g_iInstrStreamSize, 3);
	FASMDebug("\n", 3);

	// 写每一条指令
	for (int iCurrInstrIndex = 0; iCurrInstrIndex < g_iInstrStreamSize; ++iCurrInstrIndex) {
		// 写两字节的操作码 (2 bytes)
		short len = 0;
		short sOpcode = g_pInstrStream[iCurrInstrIndex].iOpcode;
		fwrite(&sOpcode, 2, 1, pExecFile);

		FASMDebug(mnemonics[sOpcode], 3);
		FASMDebug(" ", 3);

		// 写一字节的操作数数量 (1 byte)
		
		len += 3;

		char iOpCount = g_pInstrStream[iCurrInstrIndex].iOpCount;
		fwrite(&iOpCount, 1, 1, pExecFile);

		FASMDebug("opcount:", 3);
		FASMDebug(iOpCount, 3);
		FASMDebug(" ", 3);

		// 写每一个操作数

		for (int iCurrOpIndex = 0; iCurrOpIndex < iOpCount; ++iCurrOpIndex) {
			// Make a copy of the operand pointer for convinience

			Op CurrOp = g_pInstrStream[iCurrInstrIndex].pOpList[iCurrOpIndex];

			// 写一个字节的操作数类型

			char cOpType = CurrOp.iType;
			fwrite(&cOpType, 1, 1, pExecFile);

			len += 1;

			FASMDebug("optype:", 3);
			FASMDebug(cOpType, 3);
			FASMDebug(" ", 3);

			// 根据类型写操作数的值
			switch (CurrOp.iType) {
				// 整数常量
			case OP_TYPE_INT:
				fwrite(&CurrOp.iIntLiteral, sizeof(int), 1, pExecFile);

				FASMDebug(CurrOp.iIntLiteral, 3);
				FASMDebug(" ", 3);

				len += 4;

				break;

				// 字符串索引
			case OP_TYPE_STRING_REF:
				fwrite(&CurrOp.iStringTableIndex, sizeof(int), 1, pExecFile);

				FASMDebug(CurrOp.iStringTableIndex, 3);
				FASMDebug(" ", 3);

				len += 4;

				break;

				// 指令索引
			case OP_TYPE_INSTR_INDEX:
				fwrite(&CurrOp.iInstrIndex, sizeof(int), 1, pExecFile);

				FASMDebug(CurrOp.iInstrIndex, 3);
				FASMDebug(" ", 3);

				len += 4;

				break;

				// 绝对堆栈索引
			case OP_TYPE_ABS_STACK_INDEX:
				fwrite(&CurrOp.iStackIndex, sizeof(int), 1, pExecFile);

				FASMDebug(CurrOp.iStackIndex, 3);
				FASMDebug(" ", 3);

				len += 4;

				break;

				// 相对堆栈索引
			case OP_TYPE_REL_STACK_INDEX:
				fwrite(&CurrOp.iStackIndex, sizeof(int), 1, pExecFile);
				fwrite(&CurrOp.iOffsetIndex, sizeof(int), 1, pExecFile);

				FASMDebug(CurrOp.iStackIndex, 3);
				FASMDebug(" ", 3);
				FASMDebug(CurrOp.iOffsetIndex, 3);
				FASMDebug(" ", 3);

				len += 8;

				break;

				// 函数表索引
			case OP_TYPE_FUNC_INDEX:
				fwrite(&CurrOp.iFuncIndex, sizeof(int), 1, pExecFile);

				FASMDebug(CurrOp.iFuncIndex, 3);
				FASMDebug(" ", 3);

				len += 4;

				break;

				// 内核api表索引

			case OP_TYPE_HOST_API_CALL_INDEX:
				fwrite(&CurrOp.iHostAPICallIndex, sizeof(int), 1, pExecFile);

				FASMDebug(CurrOp.iHostAPICallIndex, 3);
				FASMDebug(" ", 3);

				len += 4;

				break;

				// 寄存器号
			case OP_TYPE_REG:
				fwrite(&CurrOp.iReg, sizeof(int), 1, pExecFile);

				FASMDebug(CurrOp.iReg, 3);
				FASMDebug(" ", 3);

				len += 4;

				break;
			}
		}
		if (len < INSTR_LEN) {
			char fill = 0;
			for (int i = 0; i < INSTR_LEN - len; ++i) {
				fwrite((void*)&fill, sizeof(char), 1, pExecFile);
			}
		}

		FASMDebug("\n", 3);
	}

	// Create a node pointer for traversing the lists

	int iCurrNode;
	LinkedListNode * pNode;

	// ---- 写字符串表

	// 写字符串数量 (4 bytes)

	fwrite(&g_StringTable.iNodeCount, 4, 1, pExecFile);

	FASMDebug("string num: ", 3);
	FASMDebug(g_StringTable.iNodeCount, 3);
	FASMDebug("\n", 3);

	// Set the pointer to the head of the list

	pNode = g_StringTable.pHead;

	// Loop through each node in the list and write out its string

	for (iCurrNode = 0; iCurrNode < g_StringTable.iNodeCount; ++iCurrNode) {
		// Copy the string and calculate its length

		char * pstrCurrString = (char *)pNode->pData;
		int iCurrStringLength = strlen(pstrCurrString);

		// 写字符串的长度(4 bytes), 跟着字符串内容 (N bytes)
		fwrite(&iCurrStringLength, 4, 1, pExecFile);
		fwrite(pstrCurrString, strlen(pstrCurrString), 1, pExecFile);

		FASMDebug(iCurrStringLength, 3);
		FASMDebug(" ", 3);
		FASMDebug(pstrCurrString, 3);
		FASMDebug("\n", 3);

		// Move to the next node
		pNode = pNode->pNext;
	}

	// 写函数表

	// 写函数数量 (4 bytes)

	fwrite(&g_FuncTable.iNodeCount, 4, 1, pExecFile);
	FASMDebug("func num: ", 3);
	FASMDebug(g_FuncTable.iNodeCount, 3);
	FASMDebug("\n", 3);

	// Set the pointer to the head of the list

	pNode = g_FuncTable.pHead;

	// Loop through each node in the list and write out its function info

	for (iCurrNode = 0; iCurrNode < g_FuncTable.iNodeCount; ++iCurrNode) {
		// Create a local copy of the function

		FuncNode * pFunc = (FuncNode *)pNode->pData;

		// 写入口点地址 (4 bytes)

		fwrite(&pFunc->iEntryPoint, sizeof(int), 1, pExecFile);

		// 写局部数据大小 (4 bytes)

		fwrite(&pFunc->iLocalDataSize, sizeof(int), 1, pExecFile);


		FASMDebug(pFunc->iEntryPoint, 3);
		FASMDebug(" ", 3);
		FASMDebug(pFunc->iLocalDataSize, 3);
		FASMDebug("\n", 3);


		// Move to the next node

		pNode = pNode->pNext;
	}

	// ---- 内核API

	// 写调用数量 (4 bytes)

	fwrite(&g_HostAPICallTable.iNodeCount, 4, 1, pExecFile);
	FASMDebug("api num: ", 3);
	FASMDebug(g_HostAPICallTable.iNodeCount, 3);
	FASMDebug("\n", 3);
	// Set the pointer to the head of the list

	pNode = g_HostAPICallTable.pHead;

	// Loop through each node in the list and write out its string

	for (iCurrNode = 0; iCurrNode < g_HostAPICallTable.iNodeCount; ++iCurrNode) {
		// Copy the string pointer and calculate its length

		char * pstrCurrHostAPICall = (char *)pNode->pData;
		int cCurrHostAPICallLength = strlen(pstrCurrHostAPICall);

		// Write the length (1 byte), followed by the string data (N bytes)

		fwrite(&cCurrHostAPICallLength, sizeof(int), 1, pExecFile);
		fwrite(pstrCurrHostAPICall, strlen(pstrCurrHostAPICall), 1, pExecFile);

		FASMDebug(cCurrHostAPICallLength, 3);
		FASMDebug(" ", 3);
		FASMDebug(pstrCurrHostAPICall, 3);
		FASMDebug("\n", 3);

		// Move to the next node

		pNode = pNode->pNext;
	}

	// ---- Close the output file

	fclose(pExecFile);
	return ERR_NO_ERR;
}

void FASM::Exit() {
	// Give allocated resources a chance to be freed

	ShutDown();

	// Exit the program

	//exit(0);
}

void FASM::ExitOnError(const std::string & pstrErrorMssg) {
	// Print the message

	printf("Fatal Error: %s.\n", pstrErrorMssg.c_str());

	// Exit the program

	Exit();
}

void FASM::ExitOnCodeError(const std::string & pstrErrorMssg) {
	// Print the message

	printf("Error: %s.\n\n", pstrErrorMssg.c_str());
	printf("Line %d\n", g_Lexer.iCurrSourceLine);

	// Reduce all of the source line's spaces to tabs so it takes less space and so the
	// karet lines up with the current token properly

	char pstrSourceLine[MAX_SOURCE_LINE_SIZE];
	strcpy(pstrSourceLine, g_ppstrSourceCode[g_Lexer.iCurrSourceLine]);

	// Loop through each character and replace tabs with spaces

	for (unsigned int iCurrCharIndex = 0; iCurrCharIndex < strlen(pstrSourceLine); ++iCurrCharIndex)
		if (pstrSourceLine[iCurrCharIndex] == '\t')
			pstrSourceLine[iCurrCharIndex] = ' ';

	// Print the offending source line

	printf("%s", pstrSourceLine);

	// Print a karet at the start of the (presumably) offending lexeme

	for (unsigned int iCurrSpace = 0; iCurrSpace < g_Lexer.iIndex0; ++iCurrSpace)
		printf(" ");
	printf("^\n");

	// Print message indicating that the script could not be assembled

	printf("Could not assemble %s.\n", g_pstrExecFilename);

	// Exit the program

	Exit();
}

void FASM::ExitOnCharExpectedError(char cChar) {
	// Create an error message based on the character

	char * pstrErrorMssg = (char *)malloc(strlen("' ' expected"));
	sprintf(pstrErrorMssg, "'%c' expected", cChar);

	// Exit on the code error

	ExitOnCodeError(pstrErrorMssg);
}

void FASM::ResetLexer() {
	// Set the current line to the start of the file

	g_Lexer.iCurrSourceLine = 0;

	// Set both indices to point to the start of the string

	g_Lexer.iIndex0 = 0;
	g_Lexer.iIndex1 = 0;

	// Set the token type to invalid, since a token hasn't been read yet

	g_Lexer.CurrToken = TOKEN_TYPE_INVALID;

	// Set the lexing state to no strings

	g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;
}

FASM::Token FASM::GetNextToken() {
	g_Lexer.iIndex0 = g_Lexer.iIndex1;

	if (g_Lexer.iIndex0 >= strlen(g_ppstrSourceCode[g_Lexer.iCurrSourceLine])) {
		if (!SkipToNextLine())
			return END_OF_TOKEN_STREAM;
	}

	if (g_Lexer.iCurrLexState == LEX_STATE_END_STRING)
		g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;

	if (g_Lexer.iCurrLexState != LEX_STATE_IN_STRING) {
		while (true) {
			if (!IsCharWhitespace(g_ppstrSourceCode[g_Lexer.iCurrSourceLine][g_Lexer.iIndex0]))
				break;
			++g_Lexer.iIndex0;
		}
	}

	g_Lexer.iIndex1 = g_Lexer.iIndex0;

	// 扫描文本直到找到一个分隔符
	while (true) {

		if (g_Lexer.iCurrLexState == LEX_STATE_IN_STRING) {
			//字符串需在一行内定义，如果行末尾没有双引号则为错误
			if (g_Lexer.iIndex1 >= strlen(g_ppstrSourceCode[g_Lexer.iCurrSourceLine])) {
				g_Lexer.CurrToken = TOKEN_TYPE_INVALID;
				return g_Lexer.CurrToken;
			}

			if (g_ppstrSourceCode[g_Lexer.iCurrSourceLine][g_Lexer.iIndex1] == '\\') {
				g_Lexer.iIndex1 += 2;
				continue;
			}

			if (g_ppstrSourceCode[g_Lexer.iCurrSourceLine][g_Lexer.iIndex1] == '"')
				break;

			++g_Lexer.iIndex1;
		}

		else {

			if (g_Lexer.iIndex1 >= strlen(g_ppstrSourceCode[g_Lexer.iCurrSourceLine]))
				break;

			if (IsCharDelimiter(g_ppstrSourceCode[g_Lexer.iCurrSourceLine][g_Lexer.iIndex1]))
				break;

			++g_Lexer.iIndex1;
		} // if (g_Lexer.iCurrLexState == LEX_STATE_IN_STRING)
	} // while (true)

	if (g_Lexer.iIndex1 - g_Lexer.iIndex0 == 0)
		++g_Lexer.iIndex1;

	unsigned int iCurrDestIndex = 0;

	// 复制到CurrLexeme中
	for (unsigned int iCurrSourceIndex = g_Lexer.iIndex0; iCurrSourceIndex < g_Lexer.iIndex1; ++iCurrSourceIndex) {
		if (g_Lexer.iCurrLexState == LEX_STATE_IN_STRING)
			if (g_ppstrSourceCode[g_Lexer.iCurrSourceLine][iCurrSourceIndex] == '\\')
				++iCurrSourceIndex;

		g_Lexer.pstrCurrLexeme[iCurrDestIndex] = g_ppstrSourceCode[g_Lexer.iCurrSourceLine][iCurrSourceIndex];

		++iCurrDestIndex;
	} // for
	g_Lexer.pstrCurrLexeme[iCurrDestIndex] = '\0';

	// 全部替换为大写
	if (g_Lexer.iCurrLexState != LEX_STATE_IN_STRING)
		strupr(g_Lexer.pstrCurrLexeme);

	// ---- Token Identification

	// Let's find out what sort of token our new lexeme is

	// We'll set the type to invalid now just in case the lexer doesn't match any
	// token types

	g_Lexer.CurrToken = TOKEN_TYPE_INVALID;

	if (strlen(g_Lexer.pstrCurrLexeme) > 1 || g_Lexer.pstrCurrLexeme[0] != '"') {
		if (g_Lexer.iCurrLexState == LEX_STATE_IN_STRING) {
			g_Lexer.CurrToken = TOKEN_TYPE_STRING;
			return TOKEN_TYPE_STRING;
		}
	}

	// 判断单字符的词法单元
	if (strlen(g_Lexer.pstrCurrLexeme) == 1) {
		switch (g_Lexer.pstrCurrLexeme[0]) {
			// Double-Quote

		case '"':
			// If a quote is read, advance the lexing state so that strings are lexed
			// properly

			switch (g_Lexer.iCurrLexState) {
				// If we're not lexing strings, tell the lexer we're now
				// in a string

			case LEX_STATE_NO_STRING:
				g_Lexer.iCurrLexState = LEX_STATE_IN_STRING;
				break;

				// If we're in a string, tell the lexer we just ended a string

			case LEX_STATE_IN_STRING:
				g_Lexer.iCurrLexState = LEX_STATE_END_STRING;
				break;
			}

			g_Lexer.CurrToken = TOKEN_TYPE_QUOTE;
			break;

			// Comma

		case ',':
			g_Lexer.CurrToken = TOKEN_TYPE_COMMA;
			break;

			// Colon

		case ':':
			g_Lexer.CurrToken = TOKEN_TYPE_COLON;
			break;

			// Opening Bracket

		case '[':
			g_Lexer.CurrToken = TOKEN_TYPE_OPEN_BRACKET;
			break;

			// Closing Bracket

		case ']':
			g_Lexer.CurrToken = TOKEN_TYPE_CLOSE_BRACKET;
			break;

			// Opening Brace

		case '{':
			g_Lexer.CurrToken = TOKEN_TYPE_OPEN_BRACE;
			break;

			// Closing Brace

		case '}':
			g_Lexer.CurrToken = TOKEN_TYPE_CLOSE_BRACE;
			break;

			// Newline

		case '\n':
			g_Lexer.CurrToken = TOKEN_TYPE_NEWLINE;
			break;
		}
	} // if (strlen(g_Lexer.pstrCurrLexeme) == 1)

	FASMDebug("currlex: ", 3);
	FASMDebug(g_Lexer.pstrCurrLexeme, 3);
	FASMDebug("\n", 3);

	  // 判断多字符的词法单元
	  // Is it an integer?
	if (IsStringInteger(g_Lexer.pstrCurrLexeme))
		g_Lexer.CurrToken = TOKEN_TYPE_INT;

	// Is it an identifier (which may also be a line label or instruction)?
	if (IsStringIdent(g_Lexer.pstrCurrLexeme))
		g_Lexer.CurrToken = TOKEN_TYPE_IDENT;

	// Check for register
	if (strcmp(g_Lexer.pstrCurrLexeme, "R0") == 0)
		g_Lexer.CurrToken = TOKEN_TYPE_R0;
	if (strcmp(g_Lexer.pstrCurrLexeme, "R1") == 0)
		g_Lexer.CurrToken = TOKEN_TYPE_R1;
	if (strcmp(g_Lexer.pstrCurrLexeme, "R2") == 0)
		g_Lexer.CurrToken = TOKEN_TYPE_R2;
	if (strcmp(g_Lexer.pstrCurrLexeme, "R3") == 0)
		g_Lexer.CurrToken = TOKEN_TYPE_R3;

	// Is it SetStackSize?
	if (strcmp(g_Lexer.pstrCurrLexeme, "SETSTACKSIZE") == 0)
		g_Lexer.CurrToken = TOKEN_TYPE_SETSTACKSIZE;

	// Is it Int/Int []?
	if (strcmp(g_Lexer.pstrCurrLexeme, "INT") == 0)
		g_Lexer.CurrToken = TOKEN_TYPE_INT_DEF;

	// 声明字符串STR
	if (strcmp(g_Lexer.pstrCurrLexeme, "STR") == 0) {
		g_Lexer.CurrToken = TOKEN_TYPE_STRING_DEF;
	}

	//	声明指针PTR
	if (strcmp(g_Lexer.pstrCurrLexeme, "PTR") == 0) {
		g_Lexer.CurrToken = TOKEN_TYPE_PTR;
	}

	// 函数声明
	if (strcmp(g_Lexer.pstrCurrLexeme, "FUNC") == 0)
		g_Lexer.CurrToken = TOKEN_TYPE_FUNC;

	// 指令
	InstrLookup Instr;
	if (GetInstrByMnemonic(g_Lexer.pstrCurrLexeme, &Instr))
		g_Lexer.CurrToken = TOKEN_TYPE_INSTR;

	FASMDebug("curr token: ", 3);
	FASMDebug(g_Lexer.CurrToken, 3);
	FASMDebug("\n", 3);

	return g_Lexer.CurrToken;
}

char * FASM::GetCurrLexeme() {
	// Simply return the pointer rather than making a copy

	return g_Lexer.pstrCurrLexeme;
}

char FASM::GetLookAheadChar() {
	// We don't actually want to move the lexer's indices, so we'll make a copy of them

	int iCurrSourceLine = g_Lexer.iCurrSourceLine;
	unsigned int iIndex = g_Lexer.iIndex1;

	// If the next lexeme is not a string, scan past any potential leading whitespace

	if (g_Lexer.iCurrLexState != LEX_STATE_IN_STRING) {
		// Scan through the whitespace and check for the end of the line

		while (true) {
			// If we've passed the end of the line, skip to the next line and reset the
			// index to zero

			if (iIndex >= strlen(g_ppstrSourceCode[iCurrSourceLine])) {
				// Increment the source code index

				iCurrSourceLine += 1;

				// If we've passed the end of the source file, just return a null character

				if (iCurrSourceLine >= g_iSourceCodeSize)
					return 0;

				// Otherwise, reset the index to the first character on the new line

				iIndex = 0;
			}

			// If the current character is not whitespace, return it, since it's the first
			// character of the next lexeme and is thus the look-ahead

			if (!IsCharWhitespace(g_ppstrSourceCode[iCurrSourceLine][iIndex]))
				break;

			// It is whitespace, however, so move to the next character and continue scanning

			++iIndex;
		}
	}

	// Return whatever character the loop left iIndex at

	return g_ppstrSourceCode[iCurrSourceLine][iIndex];
}

int FASM::SkipToNextLine() {
	// Increment the current line

	++g_Lexer.iCurrSourceLine;

	// Return false if we've gone past the end of the source code

	if (g_Lexer.iCurrSourceLine >= g_iSourceCodeSize)
		return false;

	// Set both indices to point to the start of the string

	g_Lexer.iIndex0 = 0;
	g_Lexer.iIndex1 = 0;

	// Turn off string lexeme mode, since strings can't span multiple lines

	g_Lexer.iCurrLexState = LEX_STATE_NO_STRING;

	// Return true to indicate success

	return true;
}

int FASM::GetInstrByMnemonic(char * pstrMnemonic, InstrLookup * pInstr) {
	// Loop through each instruction in the lookup table

	for (int iCurrInstrIndex = 0; iCurrInstrIndex < MAX_INSTR_LOOKUP_COUNT; ++iCurrInstrIndex)

		// Compare the instruction's mnemonic to the specified one

		if (strcmp(g_InstrTable[iCurrInstrIndex].pstrMnemonic, pstrMnemonic) == 0) {
			// Set the instruction definition to the user-specified pointer

			*pInstr = g_InstrTable[iCurrInstrIndex];

			// Return true to signify success

			return true;
		}

	// A match was not found, so return false

	return false;

}

void FASM::InitInstrTable() {
	// Create a temporary index to use with each instruction

	int iInstrIndex;

	// The following code makes repeated calls to AddInstrLookup () to add a hardcoded
	// instruction set to the assembler's vocabulary. Each AddInstrLookup () call is
	// followed by zero or more calls to SetOpType (), whcih set the supported types of
	// a specific operand. The instructions are grouped by family.

	// ---- Main

	// Mov          Destination, Source

	iInstrIndex = AddInstrLookup("Mov", INSTR_MNEMONIC_MOV, 2);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF | OP_FLAG_TYPE_REG);
	SetOpType(iInstrIndex, 1, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);

	// ---- Arithmetic

	// Add         Destination, Source

	iInstrIndex = AddInstrLookup("Add", INSTR_MNEMONIC_ADD, 2);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF);
	SetOpType(iInstrIndex, 1, OP_FLAG_TYPE_INT | OP_FLAG_TYPE_MEM_REF);

	// Sub          Destination, Source

	iInstrIndex = AddInstrLookup("Sub", INSTR_MNEMONIC_SUB, 2);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF);
	SetOpType(iInstrIndex, 1, OP_FLAG_TYPE_INT | OP_FLAG_TYPE_MEM_REF);

	// Mul          Destination, Source

	iInstrIndex = AddInstrLookup("Mul", INSTR_MNEMONIC_MUL, 2);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF);
	SetOpType(iInstrIndex, 1, OP_FLAG_TYPE_INT | OP_FLAG_TYPE_MEM_REF);

	// Div          Destination, Source

	iInstrIndex = AddInstrLookup("Div", INSTR_MNEMONIC_DIV, 2);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF);
	SetOpType(iInstrIndex, 1, OP_FLAG_TYPE_INT | OP_FLAG_TYPE_MEM_REF);

	// Jmp          Label

	iInstrIndex = AddInstrLookup("Jmp", INSTR_MNEMONIC_JMP, 1);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_LINE_LABEL);

	// JE           Op0, Op1, Label

	iInstrIndex = AddInstrLookup("JE", INSTR_MNEMONIC_JE, 3);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);
	SetOpType(iInstrIndex, 1, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);
	SetOpType(iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL);

	// JNE          Op0, Op1, Label

	iInstrIndex = AddInstrLookup("JNE", INSTR_MNEMONIC_JNE, 3);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);
	SetOpType(iInstrIndex, 1, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);
	SetOpType(iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL);

	// JG           Op0, Op1, Label

	iInstrIndex = AddInstrLookup("JG", INSTR_MNEMONIC_JG, 3);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);
	SetOpType(iInstrIndex, 1, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);
	SetOpType(iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL);

	// JL           Op0, Op1, Label

	iInstrIndex = AddInstrLookup("JL", INSTR_MNEMONIC_JL, 3);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);
	SetOpType(iInstrIndex, 1, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);
	SetOpType(iInstrIndex, 2, OP_FLAG_TYPE_LINE_LABEL);

	// ---- The Stack Interface

	// Push          Source

	iInstrIndex = AddInstrLookup("Push", INSTR_MNEMONIC_PUSH, 1);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_STRING_REF |
		OP_FLAG_TYPE_REG);

	// Pop           Destination

	iInstrIndex = AddInstrLookup("Pop", INSTR_MNEMONIC_POP, 1);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_MEM_REF | OP_FLAG_TYPE_REG);

	// ---- The Function Interface

	// Call          FunctionName

	iInstrIndex = AddInstrLookup("Call", INSTR_MNEMONIC_CALL, 1);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_FUNC_NAME);

	// Ret

	iInstrIndex = AddInstrLookup("Ret", INSTR_MNEMONIC_RET, 0);

	// CallHost      FunctionName

	iInstrIndex = AddInstrLookup("CallCore", INSTR_MNEMONIC_CALLHOST, 1);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_CORE_API_CALL);

	// ---- Miscellaneous

	// Pause        Duration

	iInstrIndex = AddInstrLookup("Pause", INSTR_MNEMONIC_PAUSE, 1);
	SetOpType(iInstrIndex, 0, OP_FLAG_TYPE_INT |
		OP_FLAG_TYPE_MEM_REF |
		OP_FLAG_TYPE_REG);

	// Exit         Code

	iInstrIndex = AddInstrLookup("Quit", INSTR_MNEMONIC_QUIT, 0);
}

int FASM::AddInstrLookup(const std::string & pstrMnemonic, int iOpcode, int iOpCount) {
	// Just use a simple static int to keep track of the next instruction index in the
	// table.

	static int iInstrIndex = 0;

	// Make sure we haven't run out of instruction indices

	if (iInstrIndex >= MAX_INSTR_LOOKUP_COUNT)
		return -1;

	// Set the mnemonic, opcode and operand count fields

	strcpy(g_InstrTable[iInstrIndex].pstrMnemonic, pstrMnemonic.c_str());
	strupr(g_InstrTable[iInstrIndex].pstrMnemonic);
	g_InstrTable[iInstrIndex].iOpcode = iOpcode;
	g_InstrTable[iInstrIndex].iOpCount = iOpCount;

	// Allocate space for the operand list

	g_InstrTable[iInstrIndex].OpList = (OpTypes *)malloc(iOpCount * sizeof(OpTypes));

	// Copy the instruction index into another variable so it can be returned to the caller

	int iReturnInstrIndex = iInstrIndex;

	// Increment the index for the next instruction

	++iInstrIndex;

	// Return the used index to the caller

	return iReturnInstrIndex;
}

void FASM::SetOpType(int iInstrIndex, int iOpIndex, OpTypes iOpType) {
	g_InstrTable[iInstrIndex].OpList[iOpIndex] = iOpType;
}

int FASM::AddString(LinkedList * pList, char * pstrString) {
	// ---- First check to see if the string is already in the list

	// Create a node to traverse the list

	LinkedListNode * pNode = pList->pHead;
	int iSize = 0;

	// Loop through each node in the list

	for (int iCurrNode = 0; iCurrNode < pList->iNodeCount; ++iCurrNode) {
		// If the current node's string equals the specified string, return its index

		if (strcmp((char *)pNode->pData, pstrString) == 0)
			return iSize;

		// Otherwise move along to the next node

		iSize += strlen((char*)pNode->pData) + 4;

		printf("string size is: %d\n", iSize);

		pNode = pNode->pNext;
	}

	// ---- Add the new string, since it wasn't added

	// Create space on the heap for the specified string

	char * pstrStringNode = (char *)malloc(strlen(pstrString) + 1);
	strcpy(pstrStringNode, pstrString);

	// Add the string to the list and return its index

	AddNode(pList, pstrStringNode);

	return iSize;
}

int FASM::AddFunc(char * pstrName, int iEntryPoint) {
	// If a function already exists with the specified name, exit and return an invalid
	// index

	if (GetFuncByName(pstrName))
		return -1;

	// Create a new function node

	FuncNode * pNewFunc = (FuncNode *)malloc(sizeof(FuncNode));

	// Initialize the new function

	strcpy(pNewFunc->pstrName, pstrName);
	pNewFunc->iEntryPoint = iEntryPoint;

	// Add the function to the list and get its index

	int iIndex = AddNode(&g_FuncTable, pNewFunc);

	// Set the function node's index

	pNewFunc->iIndex = iIndex;

	// Return the new function's index

	return iIndex;
}

FASM::FuncNode * FASM::GetFuncByName(char * pstrName) {
	// If the table is empty, return a NULL pointer

	if (!g_FuncTable.iNodeCount)
		return NULL;

	// Create a pointer to traverse the list

	LinkedListNode * pCurrNode = g_FuncTable.pHead;

	// Traverse the list until the matching structure is found

	for (int iCurrNode = 0; iCurrNode < g_FuncTable.iNodeCount; ++iCurrNode) {
		// Create a pointer to the current function structure

		FuncNode * pCurrFunc = (FuncNode *)pCurrNode->pData;

		// If the names match, return the current pointer

		if (strcmp(pCurrFunc->pstrName, pstrName) == 0)
			return pCurrFunc;

		// Otherwise move to the next node

		pCurrNode = pCurrNode->pNext;
	}

	// The structure was not found, so return a NULL pointer

	return NULL;
}

void FASM::SetFuncInfo(char * pstrName, int iLocalDataSize) {
	// Based on the function's name, find its node in the list

	FuncNode * pFunc = GetFuncByName(pstrName);

	// Set the remaining fields

	pFunc->iLocalDataSize = iLocalDataSize;
}

int FASM::AddLabel(char * pstrIdent, int iTargetIndex, int iFuncIndex) {
	// If a label already exists, return -1

	if (GetLabelByIdent(pstrIdent, iFuncIndex))
		return -1;

	// Create a new label node

	LabelNode * pNewLabel = (LabelNode *)malloc(sizeof(LabelNode));

	// Initialize the new label

	strcpy(pNewLabel->pstrIdent, pstrIdent);
	pNewLabel->iTargetIndex = iTargetIndex;
	pNewLabel->iFuncIndex = iFuncIndex;

	// Add the label to the list and get its index

	int iIndex = AddNode(&g_LabelTable, pNewLabel);

	// Set the index of the label node

	pNewLabel->iIndex = iIndex;

	// Return the new label's index

	return iIndex;
}

FASM::LabelNode * FASM::GetLabelByIdent(char * pstrIdent, int iFuncIndex) {
	// If the table is empty, return a NULL pointer

	if (!g_LabelTable.iNodeCount)
		return NULL;

	// Create a pointer to traverse the list

	LinkedListNode * pCurrNode = g_LabelTable.pHead;

	// Traverse the list until the matching structure is found

	for (int iCurrNode = 0; iCurrNode < g_LabelTable.iNodeCount; ++iCurrNode) {
		// Create a pointer to the current label structure

		LabelNode * pCurrLabel = (LabelNode *)pCurrNode->pData;

		// If the names and scopes match, return the current pointer

		if (strcmp(pCurrLabel->pstrIdent, pstrIdent) == 0 && pCurrLabel->iFuncIndex == iFuncIndex)
			return pCurrLabel;

		// Otherwise move to the next node

		pCurrNode = pCurrNode->pNext;
	}

	// The structure was not found, so return a NULL pointer

	return NULL;
}

int FASM::AddSymbol(char * pstrIdent, int iType, int iInitValue, bool bIsArr, int iSize, int iStackIndex, int iFuncIndex) {
	// If a label already exists

	if (GetSymbolByIdent(pstrIdent, iFuncIndex))
		return -1;

	// Create a new symbol node

	SymbolNode * pNewSymbol = (SymbolNode *)malloc(sizeof(SymbolNode));

	// Initialize the new label

	strcpy(pNewSymbol->pstrIdent, pstrIdent);
	pNewSymbol->iType = iType;
	pNewSymbol->iInitValue = iInitValue;
	pNewSymbol->bIsArr = bIsArr;
	pNewSymbol->iSize = iSize;
	pNewSymbol->iStackIndex = iStackIndex;
	pNewSymbol->iFuncIndex = iFuncIndex;

	// Add the symbol to the list and get its index

	int iIndex = AddNode(&g_SymbolTable, pNewSymbol);

	// Set the symbol node's index

	pNewSymbol->iIndex = iIndex;

	// Return the new symbol's index

	return iIndex;
}

FASM::SymbolNode * FASM::GetSymbolByIdent(char * pstrIdent, int iFuncIndex) {
	// If the table is empty, return a NULL pointer

	if (!g_SymbolTable.iNodeCount)
		return NULL;

	// Create a pointer to traverse the list

	LinkedListNode * pCurrNode = g_SymbolTable.pHead;

	// Traverse the list until the matching structure is found

	for (int iCurrNode = 0; iCurrNode < g_SymbolTable.iNodeCount; ++iCurrNode) {
		// Create a pointer to the current symbol structure

		SymbolNode * pCurrSymbol = (SymbolNode *)pCurrNode->pData;

		// See if the names match

		if (strcmp(pCurrSymbol->pstrIdent, pstrIdent) == 0)

			// If the functions match, or if the existing symbol is global, they match.
			// Return the symbol.

			if (pCurrSymbol->iFuncIndex == iFuncIndex || pCurrSymbol->iStackIndex >= 0)
				return pCurrSymbol;

		// Otherwise move to the next node

		pCurrNode = pCurrNode->pNext;
	}

	// The structure was not found, so return a NULL pointer

	return NULL;
}

int FASM::GetStackIndexByIdent(char * pstrIdent, int iFuncIndex) {
	// Get the symbol's information

	SymbolNode * pSymbol = GetSymbolByIdent(pstrIdent, iFuncIndex);

	// Return its stack index

	return pSymbol->iStackIndex;
}

int FASM::GetSizeByIdent(char * pstrIdent, int iFuncIndex) {
	// Get the symbol's information

	SymbolNode * pSymbol = GetSymbolByIdent(pstrIdent, iFuncIndex);

	// Return its size

	return pSymbol->iSize;
}

bool FASM::GetIsArrByIdent(char * pstrIdent, int iFuncIndex) {
	SymbolNode * pSymbol = GetSymbolByIdent(pstrIdent, iFuncIndex);
	return pSymbol->bIsArr;
}

FASM * FASM::getInstance() {
	if (gInstance == nullptr) {
		gInstance = new FASM();
	}
	return gInstance;
}

void FASM::release() {
	delete gInstance;
	gInstance = nullptr;
}

void FASM::FASMDebug(const char * tip, short level) {
#if DEBUG_ENABLE
	if (level <= DEBUG_LEVEL) {
		printf(tip);
	}
#endif // DEBUG_ENABLE
}

void FASM::FASMDebug(int tip, short level) {
#if DEBUG_ENABLE
	if (level <= DEBUG_LEVEL) {
		printf("%d", tip);
	}
#endif // DEBUG_ENABLE
}

void FASM::FASMLog(const char * tip) {
#if LOG_ENABLE
	printf(tip);
#endif // LOG_ENABLE
}

bool FASM::IsCharWhitespace(char cChar) {
	if (cChar == ' ' || cChar == '\t') {
		return true;
	}
	return false;
}

bool FASM::IsCharNumeric(char cChar) {
	if (cChar >= '0' && cChar <= '9') {
		return true;
	}
	return false;

}

bool FASM::IsCharIdent(char cChar) {
	if ((cChar >= '0' && cChar <= '9') ||
		(cChar >= 'A' && cChar <= 'Z') ||
		(cChar >= 'a' && cChar <= 'z') ||
		cChar == '_') {
		return true;
	}
	return false;
}

bool FASM::IsCharDelimiter(char cChar) {
	if (cChar == ':' || cChar == ',' ||
		cChar == '[' || cChar == ']' ||
		cChar == '{' || cChar == '}' ||
		cChar == '"' || IsCharWhitespace(cChar) || cChar == '\n') {
		return true;
	}
	else {
		return false;
	}
}

void FASM::TrimWhitespace(char * pstrString) {
	unsigned int iStringLength = strlen(pstrString);
	unsigned int iPadLength;
	unsigned int i;

	//如果在清除了注释后该行只剩一个换行符，则跳过该行
	if (iStringLength <= 1) {
		return;
	}

	//测定左边空白符的个数
	for (i = 0; i < iStringLength; ++i) {
		if (!IsCharWhitespace(pstrString[i])) {
			break;
		}
	}

	//字符串向左移动覆盖空白符
	iPadLength = i;
	if (iPadLength) {
		for (i = iPadLength; i < iStringLength; ++i) {
			pstrString[i - iPadLength] = pstrString[i];
		}
		//在右边空出来的地方填补空格
		for (i = iStringLength - iPadLength; i < iStringLength; ++i) {
			pstrString[i] = ' ';
		}
	}

	//在右边空白符开始的地方终止字符串
	for (i = iStringLength - 1; i > 0; --i) {
		if (!IsCharWhitespace(pstrString[i])) {
			pstrString[i + 1] = '\0';
			break;
		}
	}
}

bool FASM::IsStringWhitespace(char * pstrString) {
	if (!pstrString) {
		return false;
	}

	if (strlen(pstrString) == 0) {
		return true;
	}

	for (int i = 0; i < strlen(pstrString); ++i) {
		if (!IsCharWhitespace(pstrString[i])) {
			return false;
		}
	}
	return true;
}

bool FASM::IsStringIdent(char * pstrString) {
	if (!pstrString || strlen(pstrString) == 0) {
		return false;
	}

	if (pstrString[0] >= '0' && pstrString[0] <= '9') {
		return false;
	}

	for (int i = 0; i < strlen(pstrString); ++i) {
		if (!IsCharIdent(pstrString[i])) {
			return false;
		}
	}
	return true;
}

bool FASM::IsStringInteger(char * pstrString) {
	if (!pstrString || strlen(pstrString) == 0) {
		return false;
	}
	//判断第一个是不是负号或者数字
	if (pstrString[0] != '-' && !IsCharNumeric(pstrString[0])) {
		return false;
	}

	for (int i = 1; i < strlen(pstrString); ++i) {
		if (!IsCharNumeric(pstrString[i])) {
			return false;
		}
	}
	return true;
}
