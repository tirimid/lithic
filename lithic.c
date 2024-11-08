#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <unistd.h>

#define MAX_MODULE_PATHS 32

enum SizeMod
{
	SM_NULL = 0,
	SM_8,
	SM_16,
	SM_32,
	SM_64,
	SM_SIZE
};

enum TokenType
{
	TT_IDENT = 0,
	
	// literals.
	TT_LIT_STR,
	TT_LIT_INT,
	TT_LIT_FLOAT,
	TT_LIT_BOOL,
	
	// keywords.
	TT_KW_FIRST__,
	TT_KW_AS = TT_KW_FIRST__,
	TT_KW_BASE,
	TT_KW_BLOCK,
	TT_KW_BOOL,
	TT_KW_BREAK,
	TT_KW_CASE,
	TT_KW_CONTINUE,
	TT_KW_ELIF,
	TT_KW_ELSE,
	TT_KW_END,
	TT_KW_ENUM,
	TT_KW_EXTERNPROC,
	TT_KW_EXTERNVAR,
	TT_KW_FALSE,
	TT_KW_FLOAT32,
	TT_KW_FLOAT64,
	TT_KW_IF,
	TT_KW_IMPORT,
	TT_KW_INT8,
	TT_KW_INT16,
	TT_KW_INT32,
	TT_KW_INT64,
	TT_KW_ISIZE,
	TT_KW_LENOF,
	TT_KW_MUT,
	TT_KW_NEXTVARG,
	TT_KW_NULL,
	TT_KW_OPAQUETYPE,
	TT_KW_PROC,
	TT_KW_RESETVARGS,
	TT_KW_RETURN,
	TT_KW_SELF,
	TT_KW_SIZEOF,
	TT_KW_STRUCT,
	TT_KW_SWITCH,
	TT_KW_TRUE,
	TT_KW_TYPEALIAS,
	TT_KW_UINT8,
	TT_KW_UINT16,
	TT_KW_UINT32,
	TT_KW_UINT64,
	TT_KW_UNION,
	TT_KW_USIZE,
	TT_KW_VAR,
	TT_KW_VARGCOUNT,
	TT_KW_WHILE,
	TT_KW_LAST__ = TT_KW_WHILE,
	
	// special characters.
	TT_NEWLINE,
	TT_PBEGIN,
	TT_PEND,
	TT_BKBEGIN,
	TT_BKEND,
	TT_DOUBLE_PLUS,
	TT_DOUBLE_MINUS,
	TT_AT,
	TT_CARET,
	TT_TRIPLE_PERIOD,
	TT_PERIOD,
	TT_PERIOD_CARET_PERIOD,
	TT_MINUS,
	TT_BANG,
	TT_TILDE,
	TT_AMPERSAND,
	TT_ASTERISK,
	TT_SLASH,
	TT_PERCENT,
	TT_PLUS,
	TT_DOUBLE_GREATER,
	TT_DOUBLE_LESS,
	TT_PIPE,
	TT_GREATER,
	TT_GREQUAL,
	TT_LESS,
	TT_LEQUAL,
	TT_DOUBLE_EQUAL,
	TT_BANG_EQUAL,
	TT_DOUBLE_AMPERSAND,
	TT_DOUBLE_TILDE,
	TT_DOUBLE_PIPE,
	TT_QUESTION,
	TT_COLON,
	TT_COLON_EQUAL,
	TT_PLUS_EQUAL,
	TT_MINUS_EQUAL,
	TT_ASTERISK_EQUAL,
	TT_SLASH_EQUAL,
	TT_PERCENT_EQUAL,
	TT_DOUBLE_GREATER_EQUAL,
	TT_DOUBLE_LESS_EQUAL,
	TT_AMPERSAND_EQUAL,
	TT_TILDE_EQUAL,
	TT_PIPE_EQUAL,
	TT_AT_QUOTE,
	TT_COMMA
};

enum NodeType
{
	NT_PROGRAM = 0,
	
	// expression nodes.
	NT_EXPR,
	NT_EXPR_ATOM,
	NT_EXPR_LIST,
	NT_EXPR_LENOF,
	NT_EXPR_NEXTVARG,
	NT_EXPR_LAMBDA,
	NT_EXPR_SIZEOF,
	NT_EXPR_STRUCT,
	NT_EXPR_VARGCOUNT,
	NT_EXPR_NULL,
	NT_EXPR_POST_INC,
	NT_EXPR_POST_DEC,
	NT_EXPR_CALL,
	NT_EXPR_NTH,
	NT_EXPR_DEREF,
	NT_EXPR_ACCESS,
	NT_EXPR_CAST,
	NT_EXPR_DEREF_ACCESS,
	NT_EXPR_PRE_INC,
	NT_EXPR_PRE_DEC,
	NT_EXPR_UNARY_MINUS,
	NT_EXPR_LOG_NOT,
	NT_EXPR_BIT_NOT,
	NT_EXPR_ADDR_OF,
	NT_EXPR_MUL,
	NT_EXPR_DIV,
	NT_EXPR_MOD,
	NT_EXPR_ADD,
	NT_EXPR_SUB,
	NT_EXPR_SHR,
	NT_EXPR_SHL,
	NT_EXPR_BIT_AND,
	NT_EXPR_BIT_XOR,
	NT_EXPR_BIT_OR,
	NT_EXPR_GREATER,
	NT_EXPR_GREQUAL,
	NT_EXPR_LESS,
	NT_EXPR_LEQUAL,
	NT_EXPR_EQUAL,
	NT_EXPR_NEQUAL,
	NT_EXPR_LOG_AND,
	NT_EXPR_LOG_XOR,
	NT_EXPR_LOG_OR,
	NT_EXPR_TERNARY,
	NT_EXPR_ASSIGN,
	NT_EXPR_ADD_ASSIGN,
	NT_EXPR_SUB_ASSIGN,
	NT_EXPR_MUL_ASSIGN,
	NT_EXPR_DIV_ASSIGN,
	NT_EXPR_MOD_ASSIGN,
	NT_EXPR_SHR_ASSIGN,
	NT_EXPR_SHL_ASSIGN,
	NT_EXPR_BIT_AND_ASSIGN,
	NT_EXPR_BIT_XOR_ASSIGN,
	NT_EXPR_BIT_OR_ASSIGN,
	
	// type nodes.
	NT_TYPE,
	NT_TYPE_ATOM,
	NT_TYPE_PTR,
	NT_TYPE_PROC,
	NT_TYPE_ARRAY,
	NT_TYPE_BUFFER,
	
	// language structure nodes.
	NT_IMPORT,
	NT_PROC,
	NT_VAR,
	NT_COND_TREE,
	NT_WHILE,
	NT_BREAK,
	NT_CONTINUE,
	NT_BLOCK,
	NT_SWITCH,
	NT_CASE,
	NT_RETURN,
	NT_RESET_VARGS,
	NT_STRUCT,
	NT_ENUM,
	NT_UNION,
	NT_TYPE_ALIAS,
	NT_OPAQUE_TYPE
};

enum NodeFlag
{
	NF_PUBLIC = 0x1,
	NF_EXTERN = 0x2,
	NF_MUT = 0x4
};

enum ConfFlag
{
	CF_DUMP_TOKS = 0x1,
	CF_DUMP_AST = 0x2,
	CF_NO_FLOAT = 0x4
};

struct Conf
{
	char const *InFile;
	FILE *InFp;
	
	char const *OutFile;
	FILE *OutFp;
	
	char const *ModulePaths[MAX_MODULE_PATHS];
	size_t ModulePathCnt;
	
	unsigned long Flags;
};

struct FileData
{
	char *Name;
	char *Data;
	size_t Len;
};

struct Token
{
	union
	{
		struct
		{
			char *Text;
			size_t Len;
		} Str;
		uint64_t Int;
		double Float;
		bool Bool;
	} Data;
	
	size_t Pos, Len;
	unsigned char SizeMod;
	unsigned char Type;
};

struct LexData
{
	struct Token *Toks;
	size_t TokCnt;
};

struct StrNumLimit
{
	char const *Limit;
	size_t Len;
	int Base;
	unsigned char SizeMod;
};

struct Node
{
	struct Token const **Toks;
	size_t TokCnt;
	
	struct Node *Children;
	size_t ChildCnt;
	
	unsigned long Flags;
	unsigned char Type;
};

struct ParseState
{
	struct FileData const *File;
	struct LexData const *Lex;
	size_t i;
};

struct BindPower
{
	int Left, Right;
};

static int Conf_Read(int Argc, char const *Argv[]);
static void Conf_Quit(void);
static int ConvEscSequence(char const *Src, size_t *i, char **Str, size_t *Len);
static void DynStr_AppendChar(char **Str, size_t *Len, char Ch);
static void DynStr_Init(char **Str, size_t *Len);
static struct Token const *ExpectToken(struct ParseState *Ps, enum TokenType Type);
static void FileData_Destroy(struct FileData *Data);
static int FileData_Read(struct FileData *Out, FILE *Fp, char const *File);
static bool IsIdentInit(char ch);
static int Lex(struct LexData *Out, struct FileData const *Data);
static int LexChar(struct FileData const *Data, struct Token *Out, size_t *i);
static void LexData_AddSpecialChar(struct LexData *Out, size_t Pos, size_t Len, enum TokenType Type);
static void LexData_AddToken(struct LexData *Out, struct Token const *Tok);
static void LexData_Destroy(struct LexData *Data);
static int LexString(struct FileData const *Data, struct Token *Out, size_t *i);
static int LexNum(struct FileData const *Data, struct Token *Out, size_t *i);
static int LexWord(struct FileData const *Data, struct Token *Out, size_t *i);
static size_t LineNumber(char const *Str, size_t Pos);
static void LogErr(char const *Fmt, ...);
static void LogProgErr(struct FileData const *Data, size_t i, char const *Fmt, ...);
static void LogTokErr(struct FileData const *Data, struct Token const *Tok, char const *Fmt, ...);
static struct Token const *NextToken(struct ParseState *Ps);
static void Node_AddChild(struct Node *Node, struct Node const *Child);
static void Node_AddToken(struct Node *Node, struct Token const *Tok);
static void Node_Destroy(struct Node *Node);
static void Node_Print(FILE *Fp, struct Node const *Node, unsigned Depth);
static int Parse(struct Node *Out, struct FileData const *File, struct LexData const *Lex);
static int ParseEnum(struct Node *Out, struct ParseState *Ps);
static int ParseImport(struct Node *Out, struct ParseState *Ps);
static int ParseOpaqueType(struct Node *Out, struct ParseState *Ps);
static int ParseProc(struct Node *Out, struct ParseState *Ps);
static int ParseProgram(struct Node *Out, struct ParseState *Ps);
static int ParseStruct(struct Node *Out, struct ParseState *Ps);
static int ParseTypeAlias(struct Node *Out, struct ParseState *Ps);
static int ParseUnion(struct Node *Out, struct ParseState *Ps);
static int ParseVar(struct Node *Out, struct ParseState *Ps);
static struct Token const *PeekToken(struct ParseState const *Ps);
static unsigned SizeModBits(enum SizeMod Mod);
static int StrNumCmp(char const *a, size_t LenA, char const *b, size_t LenB);
static char *Substr(char const *Str, size_t Lb, size_t Ub);
static void Token_Destroy(struct Token *Tok);
static void Token_Print(FILE *Fp, struct Token const *Tok, size_t Ind);
static void Usage(char const *Name);

static struct StrNumLimit StrNumLimits[] =
{
	// binary.
	{"11111111", 8, 2, SM_8},
	{"1111111111111111", 16, 2, SM_16},
	{"11111111111111111111111111111111", 32, 2, SM_32},
	{"1111111111111111111111111111111111111111111111111111111111111111", 64, 2, SM_64},
	{"1111111111111111111111111111111111111111111111111111111111111111", 64, 2, SM_SIZE},
	
	// decimal.
	{"255", 3, 10, SM_8},
	{"65535", 5, 10, SM_16},
	{"4294967295", 10, 10, SM_32},
	{"18446744073709551615", 20, 10, SM_64},
	{"18446744073709551615", 20, 10, SM_SIZE},
	
	// hexadecimal.
	{"ff", 2, 16, SM_8},
	{"ffff", 4, 16, SM_16},
	{"ffffffff", 8, 16, SM_32},
	{"ffffffffffffffff", 16, 16, SM_64},
	{"ffffffffffffffff", 16, 16, SM_SIZE}
};

static char const *Keywords[] =
{
	"As",
	"Base",
	"Block",
	"Bool",
	"Break",
	"Case",
	"Continue",
	"Elif",
	"Else",
	"End",
	"Enum",
	"ExternProc",
	"ExternVar",
	"False",
	"Float32",
	"Float64",
	"If",
	"Import",
	"Int8",
	"Int16",
	"Int32",
	"Int64",
	"Isize",
	"LenOf",
	"Mut",
	"NextVarg",
	"Null",
	"OpaqueType",
	"Proc",
	"ResetVargs",
	"Return",
	"Self",
	"SizeOf",
	"Struct",
	"Switch",
	"True",
	"TypeAlias",
	"Uint8",
	"Uint16",
	"Uint32",
	"Uint64",
	"Union",
	"Usize",
	"Var",
	"VargCount",
	"While"
};

static char const *TokenTypeNames[] =
{
	"TT_IDENT",
	
	// literals.
	"TT_LIT_STR",
	"TT_LIT_INT",
	"TT_LIT_FLOAT",
	"TT_LIT_BOOL",
	
	// keywords.
	"TT_KW_AS",
	"TT_KW_BASE",
	"TT_KW_BLOCK",
	"TT_KW_BOOL",
	"TT_KW_BREAK",
	"TT_KW_CASE",
	"TT_KW_CONTINUE",
	"TT_KW_ELIF",
	"TT_KW_ELSE",
	"TT_KW_END",
	"TT_KW_ENUM",
	"TT_KW_EXTERNPROC",
	"TT_KW_EXTERNVAR",
	"TT_KW_FALSE",
	"TT_KW_FLOAT32",
	"TT_KW_FLOAT64",
	"TT_KW_IF",
	"TT_KW_IMPORT",
	"TT_KW_INT8",
	"TT_KW_INT16",
	"TT_KW_INT32",
	"TT_KW_INT64",
	"TT_KW_ISIZE",
	"TT_KW_LENOF",
	"TT_KW_MUT",
	"TT_KW_NEXTVARG",
	"TT_KW_NULL",
	"TT_KW_OPAQUETYPE",
	"TT_KW_PROC",
	"TT_KW_RESETVARGS",
	"TT_KW_RETURN",
	"TT_KW_SELF",
	"TT_KW_SIZEOF",
	"TT_KW_STRUCT",
	"TT_KW_SWITCH",
	"TT_KW_TRUE",
	"TT_KW_TYPEALIAS",
	"TT_KW_UINT8",
	"TT_KW_UINT16",
	"TT_KW_UINT32",
	"TT_KW_UINT64",
	"TT_KW_UNION",
	"TT_KW_USIZE",
	"TT_KW_VAR",
	"TT_KW_VARGCOUNT",
	"TT_KW_WHILE",
	
	// special characters.
	"TT_NEWLINE",
	"TT_PBEGIN",
	"TT_PEND",
	"TT_BKBEGIN",
	"TT_BKEND",
	"TT_DOUBLE_PLUS",
	"TT_DOUBLE_MINUS",
	"TT_AT",
	"TT_CARET",
	"TT_TRIPLE_PERIOD",
	"TT_PERIOD",
	"TT_PERIOD_CARET_PERIOD",
	"TT_MINUS",
	"TT_BANG",
	"TT_TILDE",
	"TT_AMPERSAND",
	"TT_ASTERISK",
	"TT_SLASH",
	"TT_PERCENT",
	"TT_PLUS",
	"TT_DOUBLE_GREATER",
	"TT_DOUBLE_LESS",
	"TT_PIPE",
	"TT_GREATER",
	"TT_GREQUAL",
	"TT_LESS",
	"TT_LEQUAL",
	"TT_DOUBLE_EQUAL",
	"TT_BANG_EQUAL",
	"TT_DOUBLE_AMPERSAND",
	"TT_DOUBLE_TILDE",
	"TT_DOUBLE_PIPE",
	"TT_QUESTION",
	"TT_COLON",
	"TT_COLON_EQUAL",
	"TT_PLUS_EQUAL",
	"TT_MINUS_EQUAL",
	"TT_ASTERISK_EQUAL",
	"TT_SLASH_EQUAL",
	"TT_PERCENT_EQUAL",
	"TT_DOUBLE_GREATER_ASSIGN",
	"TT_DOUBLE_LESS_ASSIGN",
	"TT_AMPERSAND_ASSIGN",
	"TT_TILDE_ASSIGN",
	"TT_PIPE_ASSIGN",
	"TT_AT_QUOTE",
	"TT_COMMA"
};

static char const *NodeTypeNames[] =
{
	"NT_PROGRAM",
	
	// expression nodes.
	"NT_EXPR",
	"NT_EXPR_ATOM",
	"NT_EXPR_LIST",
	"NT_EXPR_LENOF",
	"NT_EXPR_NEXTVARG",
	"NT_EXPR_LAMBDA",
	"NT_EXPR_SIZEOF",
	"NT_EXPR_STRUCT",
	"NT_EXPR_VARGCOUNT",
	"NT_EXPR_NULL",
	"NT_EXPR_POST_INC",
	"NT_EXPR_POST_DEC",
	"NT_EXPR_CALL",
	"NT_EXPR_NTH",
	"NT_EXPR_DEREF",
	"NT_EXPR_ACCESS",
	"NT_EXPR_CAST",
	"NT_EXPR_DEREF_ACCESS",
	"NT_EXPR_PRE_INC",
	"NT_EXPR_PRE_DEC",
	"NT_EXPR_UNARY_MINUS",
	"NT_EXPR_LOG_NOT",
	"NT_EXPR_BIT_NOT",
	"NT_EXPR_ADDR_OF",
	"NT_EXPR_MUL",
	"NT_EXPR_DIV",
	"NT_EXPR_MOD",
	"NT_EXPR_ADD",
	"NT_EXPR_SUB",
	"NT_EXPR_SHR",
	"NT_EXPR_SHL",
	"NT_EXPR_BIT_AND",
	"NT_EXPR_BIT_XOR",
	"NT_EXPR_BIT_OR",
	"NT_EXPR_GREATER",
	"NT_EXPR_GREQUAL",
	"NT_EXPR_LESS",
	"NT_EXPR_LEQUAL",
	"NT_EXPR_EQUAL",
	"NT_EXPR_NEQUAL",
	"NT_EXPR_LOG_AND",
	"NT_EXPR_LOG_XOR",
	"NT_EXPR_LOG_OR",
	"NT_EXPR_TERNARY",
	"NT_EXPR_ASSIGN",
	"NT_EXPR_ADD_ASSIGN",
	"NT_EXPR_SUB_ASSIGN",
	"NT_EXPR_MUL_ASSIGN",
	"NT_EXPR_DIV_ASSIGN",
	"NT_EXPR_MOD_ASSIGN",
	"NT_EXPR_SHR_ASSIGN",
	"NT_EXPR_SHL_ASSIGN",
	"NT_EXPR_BIT_AND_ASSIGN",
	"NT_EXPR_BIT_XOR_ASSIGN",
	"NT_EXPR_BIT_OR_ASSIGN",
	
	// type nodes.
	"NT_TYPE",
	"NT_TYPE_ATOM",
	"NT_TYPE_PTR",
	"NT_TYPE_PROC",
	"NT_TYPE_ARRAY",
	"NT_TYPE_BUFFER",
	
	// language structure nodes.
	"NT_IMPORT",
	"NT_PROC",
	"NT_VAR",
	"NT_COND_TREE",
	"NT_WHILE",
	"NT_BREAK",
	"NT_CONTINUE",
	"NT_BLOCK",
	"NT_SWITCH",
	"NT_CASE",
	"NT_RETURN",
	"NT_RESET_VARGS",
	"NT_STRUCT",
	"NT_ENUM",
	"NT_UNION",
	"NT_TYPE_ALIAS",
	"NT_OPAQUE_TYPE"
};

static struct BindPower ExprBindPower[] =
{
	{0}, // dummy entry.
	
	// first types have their own parsing schemes.
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	{0},
	
	// precedence group 14.
	{27, 28}, // ++
	{27, 28}, // --
	{27, 28}, // ()
	{27, 28}, // @
	{27, 28}, // ^
	{27, 28}, // .
	{27, 28}, // As
	{27, 28}, // .^.
	
	// precedence group 13.
	{26, 25}, // ++
	{26, 25}, // --
	{26, 25}, // -
	{26, 25}, // !
	{26, 25}, // ~
	{26, 25}, // ^
	
	// precedence group 12.
	{23, 24}, // *
	{23, 24}, // /
	{23, 24}, // %
	
	// precedence group 11.
	{21, 22}, // +
	{21, 22}, // -
	
	// precedence group 10.
	{19, 20}, // <<
	{19, 20}, // >>
	
	// precedence group 9.
	{17, 18}, // &
	
	// precedence group 8.
	{15, 16}, // ~
	
	// precedence group 7.
	{13, 14}, // |
	
	// precedence group 6.
	{11, 12}, // >
	{11, 12}, // >=
	{11, 12}, // <
	{11, 12}, // <=
	{11, 12}, // ==
	{11, 12}, // !=
	
	// precedence group 5.
	{9, 10}, // &&
	
	// precedence group 4.
	{7, 8}, // ~~
	
	// precedence group 3.
	{5, 6}, // ||
	
	// precedence group 2.
	{4, 3}, // ?
	
	// precedence group 1.
	{2, 1}, // :=
	{2, 1}, // +=
	{2, 1}, // -=
	{2, 1}, // *=
	{2, 1}, // /=
	{2, 1}, // %=
	{2, 1}, // >>=
	{2, 1}, // <<=
	{2, 1}, // &=
	{2, 1}, // ~=
	{2, 1} // |=
};

static struct Conf Conf;

int
main(int Argc, char const *Argv[])
{
	if (Conf_Read(Argc, Argv))
		return 1;
	
	struct FileData FileData = {0};
	if (FileData_Read(&FileData, Conf.InFp, Conf.InFile))
		return 1;
	
	int Rc = 0;
	
	struct LexData LexData = {0};
	if (Lex(&LexData, &FileData))
	{
		Rc = 1;
		goto ExitFileData;
	}
	
	if (Conf.Flags & CF_DUMP_TOKS)
	{
		for (size_t i = 0; i < LexData.TokCnt; ++i)
			Token_Print(Conf.OutFp, &LexData.Toks[i], i);
		goto ExitLexData;
	}
	
	struct Node Ast = {0};
	if (Parse(&Ast, &FileData, &LexData))
	{
		Rc = 1;
		goto ExitLexData;
	}
	
	if (Conf.Flags & CF_DUMP_AST)
	{
		Node_Print(Conf.OutFp, &Ast, 0);
		goto ExitAst;
	}
	
	// TODO: implement rest.
	
	// cleanup.
	{
	ExitAst:
		Node_Destroy(&Ast);
	ExitLexData:
		LexData_Destroy(&LexData);
	ExitFileData:
		FileData_Destroy(&FileData);
	}
	
	return Rc;
}

static int
Conf_Read(int Argc, char const *Argv[])
{
	atexit(Conf_Quit);
	
	// get option arguments.
	int c;
	while ((c = getopt(Argc, (char *const *)Argv, "Ac:hI:Lo:")) != -1)
	{
		switch (c)
		{
		case 'A':
			Conf.Flags |= CF_DUMP_AST;
			break;
		case 'c':
			if (!strcmp(optarg, "no-float"))
				Conf.Flags |= CF_NO_FLOAT;
			else
			{
				LogErr("unrecognized option - '%s'!", optarg);
				return 1;
			}
			break;
		case 'h':
			Usage(Argv[0]);
			exit(0);
		case 'I':
			if (Conf.ModulePathCnt >= MAX_MODULE_PATHS)
			{
				LogErr("cannot add more than %d module search paths!", MAX_MODULE_PATHS);
				return 1;
			}
			
			Conf.ModulePaths[Conf.ModulePathCnt] = optarg;
			++Conf.ModulePathCnt;
			
			// TODO: verify that directory can be opened.
			
			break;
		case 'L':
			Conf.Flags |= CF_DUMP_TOKS;
			break;
		case 'o':
			if (Conf.OutFp)
			{
				LogErr("cannot specify multiple output files!");
				return 1;
			}
			
			Conf.OutFile = optarg;
			Conf.OutFp = fopen(optarg, "wb");
			if (!Conf.OutFp)
			{
				LogErr("failed to open output file for writing - '%s'!", optarg);
				return 1;
			}
			
			break;
		default:
			Usage(Argv[0]);
			return 1;
		}
	}
	
	// get non-option arguments.
	{
		if (optind != Argc - 1)
		{
			LogErr("expected a single non-option argument!");
			return 1;
		}
		
		Conf.InFile = Argv[Argc - 1];
		Conf.InFp = fopen(Argv[Argc - 1], "rb");
		if (!Conf.InFp)
		{
			LogErr("failed to open input file for reading - '%s'!", Argv[Argc - 1]);
			return 1;
		}
	}
	
	// set unset default configuration.
	{
		if (!Conf.OutFp)
		{
			Conf.OutFile = "stdout";
			Conf.OutFp = stdout;
		}
	}
	
	return 0;
}

static void
Conf_Quit(void)
{
	// close opened files.
	{
		if (Conf.OutFp)
			fclose(Conf.OutFp);
		
		if (Conf.InFp)
			fclose(Conf.InFp);
	}
}

static int
ConvEscSequence(char const *Src, size_t *i, char **Str, size_t *Len)
{
	if (!strncmp(&Src[*i], "\\n", 2))
	{
		DynStr_AppendChar(Str, Len, '\n');
		*i += 2;
	}
	else if (!strncmp(&Src[*i], "\\r", 2))
	{
		DynStr_AppendChar(Str, Len, '\r');
		*i += 2;
	}
	else if (!strncmp(&Src[*i], "\\t", 2))
	{
		DynStr_AppendChar(Str, Len, '\t');
		*i += 2;
	}
	else if (!strncmp(&Src[*i], "\\\\", 2))
	{
		DynStr_AppendChar(Str, Len, '\\');
		*i += 2;
	}
	else if (!strncmp(&Src[*i], "\\'", 2))
	{
		DynStr_AppendChar(Str, Len, '\'');
		*i += 2;
	}
	else if (!strncmp(&Src[*i], "\\\"", 2))
	{
		DynStr_AppendChar(Str, Len, '\"');
		*i += 2;
	}
	else if (!strncmp(&Src[*i], "\\b", 2))
	{
		// TODO: implement binary sequences.
	}
	else if (!strncmp(&Src[*i], "\\x", 2))
	{
		// TODO: implement hex sequences.
	}
	else
		return 1;
	
	return 0;
}

static void
DynStr_AppendChar(char **Str, size_t *Len, char Ch)
{
	++*Len;
	*Str = realloc(*Str, *Len + 1);
	(*Str)[*Len - 1] = Ch;
	(*Str)[*Len] = 0;
}

static void
DynStr_Init(char **Str, size_t *Len)
{
	*Str = malloc(1);
	(*Str)[0] = 0;
	*Len = 0;
}

static struct Token const *
ExpectToken(struct ParseState *Ps, enum TokenType Type)
{
	struct Token const *Tok = NextToken(Ps);
	if (!Tok)
	{
		LogTokErr(Ps->File, Tok, "expected %s at end of file, found nothing!", TokenTypeNames[Type]);
		return NULL;
	}
	
	if (Tok->Type != Type)
	{
		LogTokErr(Ps->File, Tok, "expected %s, found %s!", TokenTypeNames[Type], TokenTypeNames[Tok->Type]);
		return NULL;
	}
	
	return Tok;
}

static void
FileData_Destroy(struct FileData *Data)
{
	// free allocated memory.
	{
		free(Data->Name);
		free(Data->Data);
	}
}

static int
FileData_Read(struct FileData *Out, FILE *Fp, char const *File)
{
	// read input file.
	{
		fseek(Fp, 0, SEEK_END);
		
		long Len = ftell(Fp);
		if (Len == -1)
		{
			LogErr("failed to get size of input file - '%s'!", File);
			return 1;
		}
		fseek(Fp, 0, SEEK_SET);
		
		Out->Name = strdup(File);
		Out->Len = Len;
		Out->Data = malloc(Len + 1);
		if (fread(Out->Data, sizeof(char), Len, Fp) != Len)
		{
			LogErr("failed to read input file - '%s'!", File);
			return 1;
		}
		Out->Data[Len] = 0;
	}
	
	return 0;
}

static bool
IsIdentInit(char ch)
{
	return isalpha(ch) || ch == '_';
}

static int
Lex(struct LexData *Out, struct FileData const *Data)
{
	bool InComment = false;
	for (size_t i = 0; i < Data->Len; ++i)
	{
		// handle comments and whitespace.
		{
			if (Data->Data[i] == '\n')
			{
				InComment = false;
				LexData_AddSpecialChar(Out, i, 1, TT_NEWLINE);
				continue;
			}
			
			if (Data->Data[i] == ';')
			{
				InComment = true;
				continue;
			}
			
			if (InComment || isspace(Data->Data[i]))
				continue;
		}
		
		// handle non-special characters.
		{
			if (IsIdentInit(Data->Data[i])
			    || (Data->Data[i] == '@' && IsIdentInit(Data->Data[i + 1])))
			{
				struct Token Tok;
				if (LexWord(Data, &Tok, &i))
					return 1;
				LexData_AddToken(Out, &Tok);
				--i;
				continue;
			}
			else if (isdigit(Data->Data[i]))
			{
				struct Token Tok;
				if (LexNum(Data, &Tok, &i))
					return 1;
				LexData_AddToken(Out, &Tok);
				--i;
				continue;
			}
			else if (Data->Data[i] == '\'')
			{
				struct Token Tok;
				if (LexChar(Data, &Tok, &i))
					return 1;
				LexData_AddToken(Out, &Tok);
				--i;
				continue;
			}
			else if (Data->Data[i] == '"')
			{
				struct Token Tok;
				if (LexString(Data, &Tok, &i))
					return 1;
				LexData_AddToken(Out, &Tok);
				--i;
				continue;
			}
		}
		
		// handle special characters.
		{
			switch (Data->Data[i])
			{
			case '(':
				LexData_AddSpecialChar(Out, i, 1, TT_PBEGIN);
				break;
			case ')':
				LexData_AddSpecialChar(Out, i, 1, TT_PEND);
				break;
			case '[':
				LexData_AddSpecialChar(Out, i, 1, TT_BKBEGIN);
				break;
			case ']':
				LexData_AddSpecialChar(Out, i, 1, TT_BKEND);
				break;
			case '+':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "++", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_DOUBLE_PLUS);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "+=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_PLUS_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_PLUS);
				break;
			case '-':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "--", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_DOUBLE_MINUS);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "-=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_MINUS_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_MINUS);
				break;
			case '@':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "@'", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_AT_QUOTE);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_AT);
				break;
			case '^':
				LexData_AddSpecialChar(Out, i, 1, TT_CARET);
				break;
			case '.':
				if (i + 2 < Data->Len && !strncmp(&Data->Data[i], "...", 3))
				{
					LexData_AddSpecialChar(Out, i, 3, TT_TRIPLE_PERIOD);
					i += 2;
				}
				else if (i + 2 < Data->Len && !strncmp(&Data->Data[i], ".^.", 3))
				{
					LexData_AddSpecialChar(Out, i, 3, TT_PERIOD_CARET_PERIOD);
					i += 2;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_PERIOD);
				break;
			case '!':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "!=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_BANG_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_BANG);
				break;
			case '~':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "~~", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_DOUBLE_TILDE);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "~=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_TILDE_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_TILDE);
				break;
			case '&':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "&&", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_DOUBLE_AMPERSAND);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "&=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_AMPERSAND_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_AMPERSAND);
				break;
			case '*':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "*=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_ASTERISK_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_ASTERISK);
				break;
			case '/':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "/=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_SLASH_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_SLASH);
				break;
			case '%':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "%=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_PERCENT_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_PERCENT);
				break;
			case '>':
				if (i + 2 < Data->Len && !strncmp(&Data->Data[i], ">>=", 3))
				{
					LexData_AddSpecialChar(Out, i, 3, TT_DOUBLE_GREATER_EQUAL);
					i += 2;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], ">>", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_DOUBLE_GREATER);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], ">=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_GREQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_GREATER);
				break;
			case '<':
				if (i + 2 < Data->Len && !strncmp(&Data->Data[i], "<<=", 3))
				{
					LexData_AddSpecialChar(Out, i, 3, TT_DOUBLE_LESS_EQUAL);
					i += 2;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "<<", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_DOUBLE_LESS);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "<=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_LEQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_LESS);
				break;
			case '|':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "||", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_DOUBLE_PIPE);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "|=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_PIPE_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_PIPE);
				break;
			case '?':
				LexData_AddSpecialChar(Out, i, 1, TT_QUESTION);
				break;
			case ':':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], ":=", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_COLON_EQUAL);
					++i;
				}
				else
					LexData_AddSpecialChar(Out, i, 1, TT_COLON);
				break;
			case ',':
				LexData_AddSpecialChar(Out, i, 1, TT_COMMA);
				break;
			default:
				LogProgErr(Data, i, "unhandled character - '%c'!", Data->Data[i]);
				return 1;
			}
		}
	}
	
	return 0;
}

static int
LexChar(struct FileData const *Data, struct Token *Out, size_t *i)
{
	size_t Lb = *i;
	
	char *ChData;
	size_t ChDataLen;
	DynStr_Init(&ChData, &ChDataLen);
	
	// get character data.
	{
		++*i;
		if (Data->Data[*i] == '\'')
		{
			LogProgErr(Data, Lb, "cannot have empty character literals!");
			free(ChData);
			return 1;
		}
		else if (Data->Data[*i] == '\\')
		{
			if (ConvEscSequence(Data->Data, i, &ChData, &ChDataLen))
			{
				LogProgErr(Data, Lb, "invalid escape in character - '%c'!", Data->Data[*i + 1]);
				free(ChData);
				return 1;
			}
		}
		else
			DynStr_AppendChar(&ChData, &ChDataLen, Data->Data[*i]);
		
		if (Data->Data[*i] != '\'')
		{
			LogProgErr(Data, Lb, "cannot have unterminated character literals!");
			free(ChData);
			return 1;
		}
		++*i;
	}
	
	// get size modifier if present.
	enum SizeMod SizeMod = SM_8;
	if (Data->Data[*i] == '\'')
	{
		++*i;
		size_t ModLb = *i;
		
		while (isalnum(Data->Data[*i]) || Data->Data[*i] == '_')
			++*i;
		
		if (*i - ModLb == 2 && !strncmp(&Data->Data[ModLb], "32", 2))
			SizeMod = SM_32;
		else if (*i - ModLb == 2 && !strncmp(&Data->Data[ModLb], "16", 2))
			SizeMod = SM_16;
		else if (*i - ModLb == 1 && !strncmp(&Data->Data[ModLb], "8", 1))
			SizeMod = SM_8;
		else
		{
			LogProgErr(Data, Lb, "character literal has invalid size modifier!");
			free(ChData);
			return 1;
		}
	}
	
	*Out = (struct Token)
	{
		.Data.Int = ChData[0],
		.Pos = Lb,
		.Len = *i - Lb,
		.SizeMod = SizeMod,
		.Type = TT_LIT_INT
	};
	free(ChData);
	
	return 0;
}

static void
LexData_AddSpecialChar(struct LexData *Out,
                       size_t Pos,
                       size_t Len,
                       enum TokenType Type)
{
	struct Token Tok =
	{
		.Pos = Pos,
		.Len = Len,
		.Type = Type
	};
	LexData_AddToken(Out, &Tok);
}

static void
LexData_AddToken(struct LexData *Out, struct Token const *Tok)
{
	++Out->TokCnt;
	Out->Toks = reallocarray(Out->Toks, Out->TokCnt, sizeof(struct Token));
	Out->Toks[Out->TokCnt - 1] = *Tok;
}

static void
LexData_Destroy(struct LexData *Data)
{
	for (size_t i = 0; i < Data->TokCnt; ++i)
		Token_Destroy(&Data->Toks[i]);
	free(Data->Toks);
}

static int
LexString(struct FileData const *Data, struct Token *Out, size_t *i)
{
	size_t Lb = *i;
	
	char *StrData;
	size_t StrDataLen;
	DynStr_Init(&StrData, &StrDataLen);
	
	// get string data and length.
	for (++*i; *i < Data->Len; ++*i)
	{
		if (Data->Data[*i] == '"')
		{
			++*i;
			break;
		}
		else if (Data->Data[*i] == '\\')
		{
			if (ConvEscSequence(Data->Data, i, &StrData, &StrDataLen))
			{
				LogProgErr(Data, Lb, "invalid escape in string - '%c'!", Data->Data[*i + 1]);
				free(StrData);
				return 1;
			}
			--*i;
		}
		else
			DynStr_AppendChar(&StrData, &StrDataLen, Data->Data[*i]);
	}
	
	// get size modifier if present.
	enum SizeMod SizeMod = SM_8;
	if (Data->Data[*i] == '\'')
	{
		++*i;
		size_t ModLb = *i;
		
		while (isalnum(Data->Data[*i]) || Data->Data[*i] == '_')
			++*i;
		
		if (*i - ModLb == 2 && !strncmp(&Data->Data[ModLb], "32", 2))
			SizeMod = SM_32;
		else if (*i - ModLb == 2 && !strncmp(&Data->Data[ModLb], "16", 2))
			SizeMod = SM_16;
		else if (*i - ModLb == 1 && !strncmp(&Data->Data[ModLb], "8", 1))
			SizeMod = SM_8;
		else
		{
			LogProgErr(Data, Lb, "string literal has invalid size modifier!");
			free(StrData);
			return 1;
		}
	}
	
	*Out = (struct Token)
	{
		.Data.Str.Text = StrData,
		.Data.Str.Len = StrDataLen,
		.Pos = Lb,
		.Len = *i - Lb,
		.SizeMod = SizeMod,
		.Type = TT_LIT_STR
	};
	
	return 0;
}

static int
LexNum(struct FileData const *Data, struct Token *Out, size_t *i)
{
	size_t Lb = *i;
	
	// get integer literal base.
	int NumBase = 10;
	{
		if (*i + 1 < Data->Len && !strncmp(&Data->Data[*i], "0b", 2))
		{
			NumBase = 2;
			*i += 2;
		}
		else if (*i + 1 < Data->Len && !strncmp(&Data->Data[*i], "0x", 2))
		{
			NumBase = 16;
			*i += 2;
		}
	}
	size_t NumLb = *i;
	
	// get upper bound of number.
	unsigned DpCnt = 0;
	unsigned DigitsBeforeDp = 0, DigitsAfterDp = 0;
	{
		for (;;)
		{
			if (*i >= Data->Len
			    || (NumBase == 2 && !strchr("01", Data->Data[*i]) && Data->Data[*i] != '.')
			    || (NumBase == 10 && !isdigit(Data->Data[*i]) && Data->Data[*i] != '.')
			    || (NumBase == 16 && !isxdigit(Data->Data[*i]) && Data->Data[*i] != '.'))
			{
				break;
			}
			
			DpCnt += Data->Data[*i] == '.';
			DigitsBeforeDp += !DpCnt;
			DigitsAfterDp += !!DpCnt;
			
			++*i;
		}
		
		if (isalnum(Data->Data[*i]))
		{
			LogProgErr(Data, Lb, "number literal contains invalid digit - '%c'!", Data->Data[*i]);
			return 1;
		}
	}
	size_t NumUb = *i;
	
	// perform non-numerical validation.
	{
		if (DpCnt > 0 && NumBase != 10)
		{
			LogProgErr(Data, Lb, "floating-point literals must be decimal!");
			return 1;
		}
		
		if (DpCnt > 0 && DigitsBeforeDp == 0)
		{
			LogProgErr(Data, Lb, "expected a digit prior to the decimal point!");
			return 1;
		}
		
		if (DpCnt > 0 && DigitsAfterDp == 0)
		{
			LogProgErr(Data, Lb, "expected a digit after the decimal point!");
			return 1;
		}
		
		if (DpCnt > 1)
		{
			LogProgErr(Data, Lb, "floating-point literals cannot have more than one decimal point!");
			return 1;
		}
		
		if (NumBase != 10 && NumUb == Lb + 2)
		{
			LogProgErr(Data, Lb, "expected value after base prefix!");
			return 1;
		}
	}
	
	// get size modifier if present.
	enum SizeMod SizeMod = SM_32;
	if (Data->Data[*i] == '\'')
	{
		++*i;
		size_t ModLb = *i;
		
		while (isalnum(Data->Data[*i]) || Data->Data[*i] == '_')
			++*i;
		
		if (*i - ModLb == 2 && !strncmp(&Data->Data[ModLb], "64", 2))
			SizeMod = SM_64;
		else if (*i - ModLb == 2 && !strncmp(&Data->Data[ModLb], "32", 2))
			SizeMod = SM_32;
		else if (*i - ModLb == 2 && !strncmp(&Data->Data[ModLb], "16", 2))
			SizeMod = SM_16;
		else if (*i - ModLb == 1 && !strncmp(&Data->Data[ModLb], "8", 1))
			SizeMod = SM_8;
		else if (*i - ModLb == 1 && !strncmp(&Data->Data[ModLb], "s", 1))
			SizeMod = SM_SIZE;
		else
		{
			LogProgErr(Data, Lb, "number literal has invalid size modifier!");
			return 1;
		}
		
		if (DpCnt > 0 && SizeMod != SM_32 && SizeMod != SM_64)
		{
			LogProgErr(Data, Lb, "invalid size modifier on float literal, only 32 and 64 supported!");
			return 1;
		}
	}
	size_t Ub = *i;
	
	// perform numerical validation on integers.
	if (DpCnt == 0)
	{
		bool Invalid = false;
		for (size_t j = 0; j < sizeof(StrNumLimits) / sizeof(StrNumLimits[0]); ++j)
		{
			struct StrNumLimit const *Lim = &StrNumLimits[j];
			if (Lim->Base != NumBase || Lim->SizeMod != SizeMod)
				continue;
			
			if (StrNumCmp(&Data->Data[NumLb], NumUb - NumLb, Lim->Limit, Lim->Len) > 0)
			{
				Invalid = true;
				break;
			}
		}
		
		if (Invalid)
		{
			LogProgErr(Data, Lb, "integer literal cannot fit in %d bits!", SizeModBits(SizeMod));
			return 1;
		}
	}
	
	// yield successful token and write data based on literal type.
	{
		*Out = (struct Token)
		{
			.Pos = Lb,
			.Len = Ub - Lb,
			.SizeMod = SizeMod,
			.Type = DpCnt ? TT_LIT_FLOAT : TT_LIT_INT
		};
		
		char *Str = Substr(Data->Data, NumLb, NumUb);
		if (DpCnt > 0)
			Out->Data.Float = atof(Str);
		else
			Out->Data.Int = strtoul(Str, NULL, NumBase);
		free(Str);
	}
	
	return 0;
}

static int
LexWord(struct FileData const *Data, struct Token *Out, size_t *i)
{
	bool IsRaw = Data->Data[*i] == '@';
	*i += IsRaw;
	size_t Lb = *i;
	
	// get upper bound of word.
	{
		if (IsRaw && !isalnum(Data->Data[*i]) && Data->Data[*i] != '_')
		{
			LogProgErr(Data, *i, "expected an identifier after @!");
			return 1;
		}
		
		while (isalnum(Data->Data[*i]) || Data->Data[*i] == '_')
			++*i;
	}
	
	// determine word contents and token type.
	char *Word = Substr(Data->Data, Lb, *i);
	enum TokenType Type = TT_IDENT;
	for (size_t Kw = TT_KW_FIRST__; Kw <= TT_KW_LAST__; ++Kw)
	{
		if (!strcmp(Word, Keywords[Kw - TT_KW_FIRST__]))
		{
			Type = Kw;
			break;
		}
	}
	
	// output token, accounting for keyword literals.
	{
		if (Type == TT_KW_TRUE || Type == TT_KW_FALSE)
		{
			free(Word);
			*Out = (struct Token)
			{
				.Data.Bool = Type == TT_KW_TRUE,
				.Pos = Lb,
				.Len = *i - Lb,
				.Type = TT_LIT_BOOL
			};
		}
		else if (Type >= TT_KW_FIRST__ && Type <= TT_KW_LAST__)
		{
			free(Word);
			*Out = (struct Token)
			{
				.Pos = Lb,
				.Len = *i - Lb,
				.Type = Type,
			};
		}
		else
		{
			*Out = (struct Token)
			{
				.Data.Str.Text = Word,
				.Data.Str.Len = *i - Lb,
				.Pos = Lb,
				.Len = *i - Lb,
				.Type = Type
			};
		}
	}
	
	return 0;
}

static size_t
LineNumber(char const *Str, size_t Pos)
{
	size_t Line = 1;
	for (size_t i = 0; i < Pos; ++i)
		Line += Str[i] == '\n';
	return Line;
}

static void
LogErr(char const *Fmt, ...)
{
	va_list Args;
	va_start(Args, Fmt);
	
	fprintf(stderr, "\x1b[31merr\x1b[0m: ");
	vfprintf(stderr, Fmt, Args);
	fprintf(stderr, "\n");
	
	va_end(Args);
}

static void
LogProgErr(struct FileData const *Data, size_t i, char const *Fmt, ...)
{
	// write out error message.
	{
		va_list Args;
		va_start(Args, Fmt);
		
		fprintf(stderr, "%s \x1b[31merr\x1b[0m: ", Data->Name);
		vfprintf(stderr, Fmt, Args);
		fprintf(stderr, "\n");
		
		va_end(Args);
	}
	
	// write out line and error position indicator
	{
		size_t Begin = i;
		while (Begin > 0 && Data->Data[Begin - 1] != '\n')
			--Begin;
		
		size_t End = i;
		while (End < Data->Len && Data->Data[End] != '\n')
			++End;
		
		fprintf(stderr,
		        "\x1b[2m[line %zu (byte %zu)]\n"
		        "\\->\x1b[0m ",
		        LineNumber(Data->Data, i),
		        i);
		
		for (size_t j = Begin; j < End; ++j)
			fprintf(stderr, "%c", Data->Data[j] == '\t' ? ' ' : Data->Data[j]);
		
		fprintf(stderr, "\n    ");
		
		for (size_t j = Begin; j < i; ++j)
			fprintf(stderr, " ");
		
		fprintf(stderr, "\x1b[31m^\x1b[0m\n");
	}
}

static void
LogTokErr(struct FileData const *Data,
          struct Token const *Tok,
          char const *Fmt,
          ...)
{
	// write out error message.
	{
		va_list Args;
		va_start(Args, Fmt);
		
		fprintf(stderr, "%s \x1b[31merr\x1b[0m: ", Data->Name);
		vfprintf(stderr, Fmt, Args);
		fprintf(stderr, "\n");
		
		va_end(Args);
	}
	
	// write out line and error position indicator
	{
		size_t Begin = Tok->Pos;
		while (Begin > 0 && Data->Data[Begin - 1] != '\n')
			--Begin;
		
		size_t End = Tok->Pos;
		while (End < Data->Len && Data->Data[End] != '\n')
			++End;
		
		fprintf(stderr,
		        "\x1b[2m[line %zu (byte %zu)]\n"
		        "\\->\x1b[0m ",
		        LineNumber(Data->Data, Tok->Pos),
		        Tok->Pos);
		
		for (size_t i = Begin; i < End; ++i)
			fprintf(stderr, "%c", Data->Data[i] == '\t' ? ' ' : Data->Data[i]);
		
		fprintf(stderr, "\n    ");
		
		for (size_t i = Begin; i < Tok->Pos; ++i)
			fprintf(stderr, " ");
		
		fprintf(stderr, "\x1b[31m^\x1b[0m\n");
	}
}

static struct Token const *
NextToken(struct ParseState *Ps)
{
	++Ps->i;
	struct Token const *Tok = Ps->i >= Ps->Lex->TokCnt ? NULL : &Ps->Lex->Toks[Ps->i];
	return Tok;
}

static void
Node_AddChild(struct Node *Node, struct Node const *Child)
{
	++Node->ChildCnt;
	Node->Children = reallocarray(Node->Children, Node->ChildCnt, sizeof(struct Node));
	Node->Children[Node->ChildCnt - 1] = *Child;
}

static void
Node_AddToken(struct Node *Node, struct Token const *Tok)
{
	++Node->TokCnt;
	Node->Toks = reallocarray(Node->Toks, Node->TokCnt, sizeof(struct Token *));
	Node->Toks[Node->TokCnt - 1] = Tok;
}

static void
Node_Destroy(struct Node *Node)
{
	for (size_t i = 0; i < Node->ChildCnt; ++i)
		Node_Destroy(&Node->Children[i]);
	
	// free allocated memory if needed.
	{
		if (Node->Toks)
			free(Node->Toks);
		
		if (Node->Children)
			free(Node->Children);
	}
}

static void
Node_Print(FILE *Fp, struct Node const *Node, unsigned Depth)
{
	// print out node.
	{
		for (unsigned i = 0; i < Depth; ++i)
			fprintf(Fp, "  ");
		
		fprintf(Fp,
		        "%s (%c%c%c)\n",
		        NodeTypeNames[Node->Type],
		        Node->Flags & NF_PUBLIC ? 'P' : '-',
		        Node->Flags & NF_EXTERN ? 'E' : '-',
		        Node->Flags & NF_MUT ? 'M' : '-');
		
		for (size_t i = 0; i < Node->TokCnt; ++i)
		{
			for (unsigned j = 0; j < Depth; ++j)
				fprintf(Fp, "  ");
			Token_Print(Fp, Node->Toks[i], i);
		}
	}
	
	// print out children.
	{
		for (size_t i = 0; i < Node->ChildCnt; ++i)
			Node_Print(Fp, &Node->Children[i], Depth + 1);
	}
}

static int
Parse(struct Node *Out, struct FileData const *File, struct LexData const *Lex)
{
	if (Lex->TokCnt == 0)
	{
		LogErr("cannot parse empty program!");
		return 1;
	}
	
	struct ParseState Ps =
	{
		.File = File,
		.Lex = Lex,
		.i = -1,
	};
	
	if (ParseProgram(Out, &Ps))
		return 1;
	
	return 0;
}

static int
ParseEnum(struct Node *Out, struct ParseState *Ps)
{
	// TODO: implement.
	return 1;
}

static int
ParseImport(struct Node *Out, struct ParseState *Ps)
{
	struct Token const *Import = ExpectToken(Ps, TT_KW_IMPORT);
	if (!Import)
		return 1;
	
	struct Token const *Vis = PeekToken(Ps);
	if (Vis && Vis->Type == TT_ASTERISK)
	{
		Out->Flags |= NF_PUBLIC;
		++Ps->i;
	}
	
	for (;;)
	{
		struct Token const *Target = ExpectToken(Ps, TT_IDENT);
		if (!Target)
		{
			Node_Destroy(Out);
			return 1;
		}
		
		struct Token const *Tok = NextToken(Ps);
		if (!Tok)
		{
			LogTokErr(Ps->File, Target, "expected TT_PERIOD or TT_NEWLINE after target!");
			Node_Destroy(Out);
			return 1;
		}
		
		switch (Tok->Type)
		{
		case TT_PERIOD:
			Node_AddToken(Out, Target);
			break;
		case TT_NEWLINE:
			Node_AddToken(Out, Target);
			goto Done;
		default:
			LogTokErr(Ps->File, Tok, "expected TT_PERIOD or TT_NEWLINE!");
			Node_Destroy(Out);
			return 1;
		}
	}
Done:
	
	Out->Type = NT_IMPORT;
	
	return 0;
}

static int
ParseOpaqueType(struct Node *Out, struct ParseState *Ps)
{
	struct Token const *OpaqueType = ExpectToken(Ps, TT_KW_OPAQUETYPE);
	if (!OpaqueType)
		return 1;
	
	struct Token const *Vis = PeekToken(Ps);
	if (Vis && Vis->Type == TT_ASTERISK)
	{
		Out->Flags |= NF_PUBLIC;
		++Ps->i;
	}
	
	struct Token const *Ident = ExpectToken(Ps, TT_IDENT);
	if (!Ident)
		return 1;
	
	Node_AddToken(Out, Ident);
	Out->Type = NT_OPAQUE_TYPE;
	
	return 0;
}

static int
ParseProc(struct Node *Out, struct ParseState *Ps)
{
	// TODO: implement.
	return 1;
}

static int
ParseProgram(struct Node *Out, struct ParseState *Ps)
{
	for (;;)
	{
		struct Token const *Tok = PeekToken(Ps);
		if (!Tok)
			break;
		
		switch (Tok->Type)
		{
		case TT_KW_IMPORT:
		{
			struct Node Child = {0};
			if (ParseImport(&Child, Ps))
			{
				Node_Destroy(Out);
				return 1;
			}
			Node_AddChild(Out, &Child);
			break;
		}
		case TT_KW_PROC:
		case TT_KW_EXTERNPROC:
		{
			struct Node Child = {0};
			if (ParseProc(&Child, Ps))
			{
				Node_Destroy(Out);
				return 1;
			}
			Node_AddChild(Out, &Child);
			break;
		}
		case TT_KW_VAR:
		case TT_KW_EXTERNVAR:
		{
			struct Node Child = {0};
			if (ParseVar(&Child, Ps))
			{
				Node_Destroy(Out);
				return 1;
			}
			Node_AddChild(Out, &Child);
			break;
		}
		case TT_KW_STRUCT:
		{
			struct Node Child = {0};
			if (ParseStruct(&Child, Ps))
			{
				Node_Destroy(Out);
				return 1;
			}
			Node_AddChild(Out, &Child);
			break;
		}
		case TT_KW_ENUM:
		{
			struct Node Child = {0};
			if (ParseEnum(&Child, Ps))
			{
				Node_Destroy(Out);
				return 1;
			}
			Node_AddChild(Out, &Child);
			break;
		}
		case TT_KW_UNION:
		{
			struct Node Child = {0};
			if (ParseUnion(&Child, Ps))
			{
				Node_Destroy(Out);
				return 1;
			}
			Node_AddChild(Out, &Child);
			break;
		}
		case TT_KW_TYPEALIAS:
		{
			struct Node Child = {0};
			if (ParseTypeAlias(&Child, Ps))
			{
				Node_Destroy(Out);
				return 1;
			}
			Node_AddChild(Out, &Child);
			break;
		}
		case TT_KW_OPAQUETYPE:
		{
			struct Node Child = {0};
			if (ParseOpaqueType(&Child, Ps))
			{
				Node_Destroy(Out);
				return 1;
			}
			Node_AddChild(Out, &Child);
			break;
		}
		case TT_NEWLINE:
			++Ps->i;
			break;
		default:
			LogTokErr(Ps->File, Tok, "expected global scope element!");
			Node_Destroy(Out);
			return 1;
		}
	}
	
	Out->Type = NT_PROGRAM;
	
	return 0;
}

static int
ParseStruct(struct Node *Out, struct ParseState *Ps)
{
	// TODO: implement.
	return 1;
}

static int
ParseTypeAlias(struct Node *Out, struct ParseState *Ps)
{
	// TODO: implement.
	return 1;
}

static int
ParseUnion(struct Node *Out, struct ParseState *Ps)
{
	// TODO: implement.
	return 1;
}

static int
ParseVar(struct Node *Out, struct ParseState *Ps)
{
	// TODO: implement.
	return 1;
}

static struct Token const *
PeekToken(struct ParseState const *Ps)
{
	return Ps->i + 1 >= Ps->Lex->TokCnt ? NULL : &Ps->Lex->Toks[Ps->i + 1];
}

static unsigned
SizeModBits(enum SizeMod Mod)
{
	static unsigned Bits[] =
	{
		0,
		8, // SM_8.
		16, // SM_16.
		32, // SM_32.
		64, // SM_64.
		64 // SM_SIZE.
	};
	
	return Bits[Mod];
}

static int
StrNumCmp(char const *a, size_t LenA, char const *b, size_t LenB)
{
	static int DigitValue[256] =
	{
		['0'] = 0,
		['1'] = 1,
		['2'] = 2,
		['3'] = 3,
		['4'] = 4,
		['5'] = 5,
		['6'] = 6,
		['7'] = 7,
		['8'] = 8,
		['9'] = 9,
		['a'] = 10,
		['A'] = 10,
		['b'] = 11,
		['B'] = 11,
		['c'] = 12,
		['C'] = 12,
		['d'] = 13,
		['D'] = 13,
		['e'] = 14,
		['E'] = 14,
		['f'] = 15,
		['F'] = 15
	};
	
	if (LenA > LenB)
		return 1;
	else if (LenA < LenB)
		return -1;
	
	for (size_t i = 0; i < LenA; ++i)
	{
		if (DigitValue[(size_t)a[i]] > DigitValue[(size_t)b[i]])
			return 1;
		else if (DigitValue[(size_t)a[i]] < DigitValue[(size_t)b[i]])
			return -1;
	}
	
	return 0;
}

static char *
Substr(char const *Str, size_t Lb, size_t Ub)
{
	char *Sub = malloc(Ub - Lb + 1);
	memcpy(Sub, &Str[Lb], Ub - Lb);
	Sub[Ub - Lb] = 0;
	return Sub;
}

static void
Token_Destroy(struct Token *Tok)
{
	switch (Tok->Type)
	{
	case TT_IDENT:
	case TT_LIT_STR:
		free(Tok->Data.Str.Text);
		break;
	default:
		break;
	}
}

static void
Token_Print(FILE *Fp, struct Token const *Tok, size_t Ind)
{
	fprintf(Fp, "[%zu] [%zu+%zu]:%s", Ind, Tok->Pos, Tok->Len, TokenTypeNames[Tok->Type]);
	switch (Tok->Type)
	{
	case TT_IDENT:
		fprintf(Fp, " %s", Tok->Data.Str.Text);
		break;
	case TT_LIT_STR:
		fprintf(Fp, " S%d:%s", SizeModBits(Tok->SizeMod), Tok->Data.Str.Text);
		break;
	case TT_LIT_INT:
		fprintf(Fp, " S%d:%lu", SizeModBits(Tok->SizeMod), Tok->Data.Int);
		break;
	case TT_LIT_FLOAT:
		fprintf(Fp, " S%d:%f", SizeModBits(Tok->SizeMod), Tok->Data.Float);
		break;
	case TT_LIT_BOOL:
		fprintf(Fp, " %s", Tok->Data.Bool ? "True" : "False");
		break;
	default:
		break;
	}
	fprintf(Fp, "\n");
}

static void
Usage(char const *Name)
{
	printf("lithic - programming language transpiler\n"
	       "\n"
	       "for more information, consult the manual at the\n"
	       "following link: https://tirimid.net/tirimid/lithic.html\n"
	       "\n"
	       "usage:\n"
	       "\t%s [options] file\n"
	       "\n"
	       "options:\n"
	       "\t-A       dump the parsed out AST\n"
	       "\t-c flag  specify a language / transpiler flag\n"
	       "\t-h       display this help text\n"
	       "\t-I dir   add a module search directory\n"
	       "\t-L       dump the lexed tokens\n"
	       "\t-o file  write output to the specified file\n",
	       Name);
}
