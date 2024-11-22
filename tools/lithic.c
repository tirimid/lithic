#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/time.h>
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
	TT_KW_DEFER,
	TT_KW_ELIF,
	TT_KW_ELSE,
	TT_KW_END,
	TT_KW_ENUM,
	TT_KW_EXTERNPROC,
	TT_KW_EXTERNVAR,
	TT_KW_FALSE,
	TT_KW_FLOAT32,
	TT_KW_FLOAT64,
	TT_KW_FOR,
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
	TT_KW_LAST__ = TT_KW_VARGCOUNT,
	
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
	TT_PERIOD_CARET,
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

enum AstNodeType
{
	ANT_PROGRAM = 0,
	
	// expression nodes.
	ANT_EXPR,
	ANT_EXPR_ATOM,
	ANT_EXPR_LIST,
	ANT_EXPR_LENOF,
	ANT_EXPR_NEXTVARG,
	ANT_EXPR_LAMBDA,
	ANT_EXPR_SIZEOF,
	ANT_EXPR_STRUCT,
	ANT_EXPR_UNION,
	ANT_EXPR_MEMB,
	ANT_EXPR_VARGCOUNT,
	ANT_EXPR_NULL,
	ANT_EXPR_POST_INC,
	ANT_EXPR_POST_DEC,
	ANT_EXPR_CALL,
	ANT_EXPR_NTH,
	ANT_EXPR_ADDR_OF,
	ANT_EXPR_ACCESS,
	ANT_EXPR_CAST,
	ANT_EXPR_DEREF,
	ANT_EXPR_PRE_INC,
	ANT_EXPR_PRE_DEC,
	ANT_EXPR_UNARY_MINUS,
	ANT_EXPR_LOG_NOT,
	ANT_EXPR_BIT_NOT,
	ANT_EXPR_MUL,
	ANT_EXPR_DIV,
	ANT_EXPR_MOD,
	ANT_EXPR_ADD,
	ANT_EXPR_SUB,
	ANT_EXPR_SHR,
	ANT_EXPR_SHL,
	ANT_EXPR_BIT_AND,
	ANT_EXPR_BIT_XOR,
	ANT_EXPR_BIT_OR,
	ANT_EXPR_GREATER,
	ANT_EXPR_GREQUAL,
	ANT_EXPR_LESS,
	ANT_EXPR_LEQUAL,
	ANT_EXPR_EQUAL,
	ANT_EXPR_NEQUAL,
	ANT_EXPR_LOG_AND,
	ANT_EXPR_LOG_XOR,
	ANT_EXPR_LOG_OR,
	ANT_EXPR_TERNARY,
	ANT_EXPR_ASSIGN,
	ANT_EXPR_ADD_ASSIGN,
	ANT_EXPR_SUB_ASSIGN,
	ANT_EXPR_MUL_ASSIGN,
	ANT_EXPR_DIV_ASSIGN,
	ANT_EXPR_MOD_ASSIGN,
	ANT_EXPR_SHR_ASSIGN,
	ANT_EXPR_SHL_ASSIGN,
	ANT_EXPR_BIT_AND_ASSIGN,
	ANT_EXPR_BIT_XOR_ASSIGN,
	ANT_EXPR_BIT_OR_ASSIGN,
	
	// type nodes.
	ANT_TYPE,
	ANT_TYPE_ATOM,
	ANT_TYPE_PTR,
	ANT_TYPE_PROC,
	ANT_TYPE_ARRAY,
	ANT_TYPE_BUFFER,
	
	// language structure nodes.
	ANT_IMPORT,
	ANT_PROC,
	ANT_ARG_LIST,
	ANT_ARG,
	ANT_STATEMENT_LIST,
	ANT_VAR,
	ANT_COND_TREE,
	ANT_FOR,
	ANT_BREAK,
	ANT_CONTINUE,
	ANT_BLOCK,
	ANT_SWITCH,
	ANT_CASE,
	ANT_RETURN,
	ANT_RESET_VARGS,
	ANT_DEFER,
	ANT_STRUCT,
	ANT_ENUM,
	ANT_UNION,
	ANT_MEMBER,
	ANT_ENUM_MEMBER,
	ANT_TYPE_ALIAS
};

enum AstNodeFlag
{
	ANF_PUBLIC = 0x1,
	ANF_EXTERN = 0x2,
	ANF_MUT = 0x4,
	ANF_BASE = 0x8,
	ANF_VARIADIC = 0x10
};

enum ConfFlag
{
	CF_DUMP_TOKS = 0x1,
	CF_DUMP_AST = 0x2,
	CF_TIME = 0x4,
	CF_NO_FLOAT = 0x8
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

struct AstNode
{
	struct Token const **Toks;
	size_t TokCnt;
	
	struct AstNode *Children;
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

struct ModuleData
{
	struct FileData File;
	struct LexData Lex;
	struct AstNode Ast;
	char *FullPath;
};

struct ModuleDataGroup
{
	struct ModuleData *Modules;
	size_t ModuleCnt;
};

struct TimeData
{
	// begin and end timestamps for each key stage of transpilation.
	uint64_t ConfReadBegin, ConfReadEnd;
	uint64_t FileReadBegin, FileReadEnd;
	uint64_t LexBegin, LexEnd;
	uint64_t ParseBegin, ParseEnd;
	uint64_t ExtractImportsBegin, ExtractImportsEnd;
};

static void AstNode_AddChild(struct AstNode *Node, struct AstNode const *Child);
static void AstNode_AddToken(struct AstNode *Node, struct Token const *Tok);
static void AstNode_Destroy(struct AstNode *Node);
static void AstNode_Print(FILE *Fp, struct AstNode const *Node, unsigned Depth);
static int Conf_Read(int Argc, char const *Argv[]);
static void Conf_Quit(void);
static int ConvEscSequence(char const *Src, size_t SrcLen, size_t *i, char **Str, size_t *Len);
static void DynStr_AppendChar(char **Str, size_t *Len, char Ch);
static void DynStr_AppendStr(char **Str, size_t *Len, char const *Append);
static void DynStr_Init(char **Str, size_t *Len);
static struct Token const *ExpectToken(struct ParseState *Ps, enum TokenType Type);
static int ExtractImports(struct FileData const *File, struct ModuleDataGroup *Append, struct AstNode const *Ast, unsigned Depth);
static void FileData_Destroy(struct FileData *Data);
static int FileData_Read(struct FileData *Out, FILE *Fp, char const *File);
static char *FullPathname(char const *Path);
static uint64_t GetUnixTimeMs(void);
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
static void LogAstNodeErr(struct FileData const *Data, struct AstNode const *Node, char const *Fmt, ...);
static void LogErr(char const *Fmt, ...);
static void LogProgErr(struct FileData const *Data, size_t i, char const *Fmt, ...);
static void LogProgPosition(struct FileData const *Data, size_t Pos, size_t Len, char const *HlStyle);
static void LogTokErr(struct FileData const *Data, struct Token const *Tok, char const *Fmt, ...);
static void ModuleData_Destroy(struct ModuleData *Data);
static void ModuleDataGroup_Append(struct ModuleDataGroup *Group, struct ModuleData const *Data);
static void ModuleDataGroup_Destroy(struct ModuleDataGroup *Group);
static struct Token const *NextToken(struct ParseState *Ps);
static FILE *OpenFile(char const *File, char const *Mode);
static int Parse(struct AstNode *Out, struct FileData const *File, struct LexData const *Lex);
static int ParseArgList(struct AstNode *Out, struct ParseState *Ps);
static int ParseBlock(struct AstNode *Out, struct ParseState *Ps);
static int ParseBreak(struct AstNode *Out, struct ParseState *Ps);
static int ParseCondTree(struct AstNode *Out, struct ParseState *Ps);
static int ParseContinue(struct AstNode *Out, struct ParseState *Ps);
static int ParseDefer(struct AstNode *Out, struct ParseState *Ps);
static int ParseEnum(struct AstNode *Out, struct ParseState *Ps);
static int ParseExpr(struct AstNode *Out, struct ParseState *Ps, unsigned char const Term[], size_t TermCnt, int MinBp);
static int ParseExprLed(struct AstNode *Out, struct ParseState *Ps, struct AstNode const *Lhs, unsigned char const Term[], size_t TermCnt);
static int ParseExprList(struct AstNode *Out, struct ParseState *Ps);
static int ParseExprNud(struct AstNode *Out, struct ParseState *Ps, unsigned char const Term[], size_t TermCnt);
static int ParseFor(struct AstNode *Out, struct ParseState *Ps);
static int ParseImport(struct AstNode *Out, struct ParseState *Ps);
static int ParseProc(struct AstNode *Out, struct ParseState *Ps);
static int ParseProgram(struct AstNode *Out, struct ParseState *Ps);
static int ParseResetVargs(struct AstNode *Out, struct ParseState *Ps);
static int ParseReturn(struct AstNode *Out, struct ParseState *Ps);
static int ParseStatement(struct AstNode *Out, struct ParseState *Ps);
static int ParseStatementList(struct AstNode *Out, struct ParseState *Ps, unsigned char const Term[], size_t TermCnt);
static int ParseStruct(struct AstNode *Out, struct ParseState *Ps);
static int ParseSwitch(struct AstNode *Out, struct ParseState *Ps);
static int ParseType(struct AstNode *Out, struct ParseState *Ps, unsigned char const Term[], size_t TermCnt);
static int ParseTypeAlias(struct AstNode *Out, struct ParseState *Ps);
static int ParseTypeLiteral(struct AstNode *Out, struct ParseState *Ps);
static int ParseUnion(struct AstNode *Out, struct ParseState *Ps);
static int ParseVar(struct AstNode *Out, struct ParseState *Ps, unsigned char const Term[], size_t TermCnt);
static int ParseWrappedExpr(struct AstNode *Out, struct ParseState *Ps, unsigned char const Term[], size_t TermCnt);
static int ParseWrappedType(struct AstNode *Out, struct ParseState *Ps, unsigned char const Term[], size_t TermCnt);
static struct Token const *PeekPrevToken(struct ParseState const *Ps);
static struct Token const *PeekToken(struct ParseState const *Ps);
static void PrintTimeData(void);
static char *ResolveImport(struct AstNode const *Import);
static struct Token const *RequireToken(struct ParseState *Ps);
static unsigned SizeModBits(enum SizeMod Mod);
static void SkipParseNewlines(struct ParseState *Ps);
static int StrNumCmp(char const *a, size_t LenA, char const *b, size_t LenB);
static char *Substr(char const *Str, size_t Lb, size_t Ub);
static void Token_Destroy(struct Token *Tok);
static void Token_Print(FILE *Fp, struct Token const *Tok, size_t Ind);
static enum AstNodeType TokenTypeToLed(enum TokenType Type);
static enum AstNodeType TokenTypeToNud(enum TokenType Type);
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

static int HexDigitValue[256] =
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

static char const *Keywords[] =
{
	"As",
	"Base",
	"Block",
	"Bool",
	"Break",
	"Case",
	"Continue",
	"Defer",
	"Elif",
	"Else",
	"End",
	"Enum",
	"ExternProc",
	"ExternVar",
	"False",
	"Float32",
	"Float64",
	"For",
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
	"VargCount"
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
	"TT_KW_DEFER",
	"TT_KW_ELIF",
	"TT_KW_ELSE",
	"TT_KW_END",
	"TT_KW_ENUM",
	"TT_KW_EXTERNPROC",
	"TT_KW_EXTERNVAR",
	"TT_KW_FALSE",
	"TT_KW_FLOAT32",
	"TT_KW_FLOAT64",
	"TT_KW_FOR",
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
	"TT_PERIOD_CARET",
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
	"ANT_PROGRAM",
	
	// expression nodes.
	"ANT_EXPR",
	"ANT_EXPR_ATOM",
	"ANT_EXPR_LIST",
	"ANT_EXPR_LENOF",
	"ANT_EXPR_NEXTVARG",
	"ANT_EXPR_LAMBDA",
	"ANT_EXPR_SIZEOF",
	"ANT_EXPR_STRUCT",
	"ANT_EXPR_UNION",
	"ANT_EXPR_MEMB",
	"ANT_EXPR_VARGCOUNT",
	"ANT_EXPR_NULL",
	"ANT_EXPR_POST_INC",
	"ANT_EXPR_POST_DEC",
	"ANT_EXPR_CALL",
	"ANT_EXPR_NTH",
	"ANT_EXPR_ADDR_OF",
	"ANT_EXPR_ACCESS",
	"ANT_EXPR_CAST",
	"ANT_EXPR_DEREF",
	"ANT_EXPR_PRE_INC",
	"ANT_EXPR_PRE_DEC",
	"ANT_EXPR_UNARY_MINUS",
	"ANT_EXPR_LOG_NOT",
	"ANT_EXPR_BIT_NOT",
	"ANT_EXPR_MUL",
	"ANT_EXPR_DIV",
	"ANT_EXPR_MOD",
	"ANT_EXPR_ADD",
	"ANT_EXPR_SUB",
	"ANT_EXPR_SHR",
	"ANT_EXPR_SHL",
	"ANT_EXPR_BIT_AND",
	"ANT_EXPR_BIT_XOR",
	"ANT_EXPR_BIT_OR",
	"ANT_EXPR_GREATER",
	"ANT_EXPR_GREQUAL",
	"ANT_EXPR_LESS",
	"ANT_EXPR_LEQUAL",
	"ANT_EXPR_EQUAL",
	"ANT_EXPR_NEQUAL",
	"ANT_EXPR_LOG_AND",
	"ANT_EXPR_LOG_XOR",
	"ANT_EXPR_LOG_OR",
	"ANT_EXPR_TERNARY",
	"ANT_EXPR_ASSIGN",
	"ANT_EXPR_ADD_ASSIGN",
	"ANT_EXPR_SUB_ASSIGN",
	"ANT_EXPR_MUL_ASSIGN",
	"ANT_EXPR_DIV_ASSIGN",
	"ANT_EXPR_MOD_ASSIGN",
	"ANT_EXPR_SHR_ASSIGN",
	"ANT_EXPR_SHL_ASSIGN",
	"ANT_EXPR_BIT_AND_ASSIGN",
	"ANT_EXPR_BIT_XOR_ASSIGN",
	"ANT_EXPR_BIT_OR_ASSIGN",
	
	// type nodes.
	"ANT_TYPE",
	"ANT_TYPE_ATOM",
	"ANT_TYPE_PTR",
	"ANT_TYPE_PROC",
	"ANT_TYPE_ARRAY",
	"ANT_TYPE_BUFFER",
	
	// language structure nodes.
	"ANT_IMPORT",
	"ANT_PROC",
	"ANT_ARG_LIST",
	"ANT_ARG",
	"ANT_STATEMENT_LIST",
	"ANT_VAR",
	"ANT_COND_TREE",
	"ANT_FOR",
	"ANT_BREAK",
	"ANT_CONTINUE",
	"ANT_BLOCK",
	"ANT_SWITCH",
	"ANT_CASE",
	"ANT_RETURN",
	"ANT_RESET_VARGS",
	"ANT_DEFER",
	"ANT_STRUCT",
	"ANT_ENUM",
	"ANT_UNION",
	"ANT_MEMBER",
	"ANT_ENUM_MEMBER",
	"ANT_TYPE_ALIAS"
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
	{27, 28}, // .^
	
	// precedence group 13.
	{26, 25}, // ++
	{26, 25}, // --
	{26, 25}, // -
	{26, 25}, // !
	{26, 25}, // ~
	
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
static struct TimeData TimeData;

int
main(int Argc, char const *Argv[])
{
	atexit(PrintTimeData);
	
	// read configuration.
	{
		TimeData.ConfReadBegin = GetUnixTimeMs();
		if (Conf_Read(Argc, Argv))
			return 1;
		TimeData.ConfReadEnd = GetUnixTimeMs();
	}
	
	// read input file.
	struct FileData FileData = {0};
	{
		TimeData.FileReadBegin = GetUnixTimeMs();
		if (FileData_Read(&FileData, Conf.InFp, Conf.InFile))
			return 1;
		TimeData.FileReadEnd = GetUnixTimeMs();
	}
	
	// lex input file.
	struct LexData LexData = {0};
	{
		TimeData.LexBegin = GetUnixTimeMs();
		if (Lex(&LexData, &FileData))
		{
			FileData_Destroy(&FileData);
			return 1;
		}
		TimeData.LexEnd = GetUnixTimeMs();
		
		if (Conf.Flags & CF_DUMP_TOKS)
		{
			for (size_t i = 0; i < LexData.TokCnt; ++i)
				Token_Print(Conf.OutFp, &LexData.Toks[i], i);
			
			LexData_Destroy(&LexData);
			FileData_Destroy(&FileData);
			return 0;
		}
	}
	
	// parse input file.
	struct AstNode Ast = {0};
	{
		TimeData.ParseBegin = GetUnixTimeMs();
		if (Parse(&Ast, &FileData, &LexData))
		{
			LexData_Destroy(&LexData);
			FileData_Destroy(&FileData);
			return 1;
		}
		TimeData.ParseEnd = GetUnixTimeMs();
		
		if (Conf.Flags & CF_DUMP_AST)
		{
			AstNode_Print(Conf.OutFp, &Ast, 0);
			
			AstNode_Destroy(&Ast);
			LexData_Destroy(&LexData);
			FileData_Destroy(&FileData);
			return 0;
		}
	}
	
	struct ModuleData ModuleData =
	{
		.File = FileData,
		.Lex = LexData,
		.Ast = Ast,
		.FullPath = FullPathname(Conf.InFile)
	};
	
	struct ModuleDataGroup ModuleDataGroup = {0};
	ModuleDataGroup_Append(&ModuleDataGroup, &ModuleData);
	
	// extract and read imports.
	{
		TimeData.ExtractImportsBegin = GetUnixTimeMs();
		if (ExtractImports(&FileData, &ModuleDataGroup, &Ast, 0))
		{
			ModuleDataGroup_Destroy(&ModuleDataGroup);
			return 1;
		}
		TimeData.ExtractImportsBegin = GetUnixTimeMs();
	}
	
	// TODO: implement rest of transpilation process.
	
	ModuleDataGroup_Destroy(&ModuleDataGroup);
	return 0;
}

static void
AstNode_AddChild(struct AstNode *Node, struct AstNode const *Child)
{
	++Node->ChildCnt;
	Node->Children = reallocarray(
		Node->Children,
		Node->ChildCnt,
		sizeof(struct AstNode)
	);
	Node->Children[Node->ChildCnt - 1] = *Child;
}

static void
AstNode_AddToken(struct AstNode *Node, struct Token const *Tok)
{
	++Node->TokCnt;
	Node->Toks = reallocarray(
		Node->Toks,
		Node->TokCnt,
		sizeof(struct Token *)
	);
	Node->Toks[Node->TokCnt - 1] = Tok;
}

static void
AstNode_Destroy(struct AstNode *Node)
{
	for (size_t i = 0; i < Node->ChildCnt; ++i)
		AstNode_Destroy(&Node->Children[i]);
	
	// free allocated memory if needed.
	{
		if (Node->Toks)
			free(Node->Toks);
		
		if (Node->Children)
			free(Node->Children);
	}
}

static void
AstNode_Print(FILE *Fp, struct AstNode const *Node, unsigned Depth)
{
	// print out node.
	{
		for (unsigned i = 0; i < Depth; ++i)
			fprintf(Fp, "      ");
		
		fprintf(
			Fp,
			"%s (%c%c%c%c%c)\n",
			NodeTypeNames[Node->Type],
			Node->Flags & ANF_PUBLIC ? 'P' : '-',
			Node->Flags & ANF_EXTERN ? 'E' : '-',
			Node->Flags & ANF_MUT ? 'M' : '-',
			Node->Flags & ANF_BASE ? 'B' : '-',
			Node->Flags & ANF_VARIADIC ? 'V' : '-'
		);
		
		for (size_t i = 0; i < Node->TokCnt; ++i)
		{
			for (unsigned j = 0; j < Depth; ++j)
				fprintf(Fp, "      ");
			Token_Print(Fp, Node->Toks[i], i);
		}
	}
	
	// print out children.
	{
		for (size_t i = 0; i < Node->ChildCnt; ++i)
			AstNode_Print(Fp, &Node->Children[i], Depth + 1);
	}
}

static int
Conf_Read(int Argc, char const *Argv[])
{
	atexit(Conf_Quit);
	
	struct option Opts[] =
	{
		{"ast", no_argument, NULL, 'a'},
		{"conf", required_argument, NULL, 'c'},
		{"help", no_argument, NULL, 'h'},
		{"lex", no_argument, NULL, 'l'},
		{"modpath", required_argument, NULL, 'm'},
		{"out", required_argument, NULL, 'o'},
		{"time", no_argument, NULL, 't'},
		{0}
	};
	
	// get option arguments.
	int c, LongInd;
	while ((c = getopt_long(Argc, (char *const *)Argv, "c:hm:o:", Opts, &LongInd)) != -1)
	{
		switch (c)
		{
		case 'a':
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
		case 'l':
			Conf.Flags |= CF_DUMP_TOKS;
			break;
		case 'm':
		{
			if (Conf.ModulePathCnt >= MAX_MODULE_PATHS)
			{
				LogErr("cannot add more than %d module search paths!", MAX_MODULE_PATHS);
				return 1;
			}
			
			Conf.ModulePaths[Conf.ModulePathCnt] = optarg;
			++Conf.ModulePathCnt;
			
			DIR *Dp = opendir(optarg);
			if (!Dp)
			{
				LogErr("module search path either does not exist or is not a directory - '%s'!", optarg);
				return 1;
			}
			closedir(Dp);
			
			break;
		}
		case 'o':
			if (Conf.OutFp)
			{
				LogErr("cannot specify multiple output files!");
				return 1;
			}
			
			Conf.OutFile = optarg;
			Conf.OutFp = OpenFile(optarg, "wb");
			if (!Conf.OutFp)
			{
				LogErr("failed to open output file for writing - '%s'!", optarg);
				return 1;
			}
			
			break;
		case 't':
			Conf.Flags |= CF_TIME;
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
		Conf.InFp = OpenFile(Argv[Argc - 1], "rb");
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
ConvEscSequence(
	char const *Src,
	size_t SrcLen,
	size_t *i,
	char **Str,
	size_t *Len
)
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
		if (*i + 10 >= SrcLen)
			return 1;
		
		uint8_t Val = 0;
		for (size_t j = *i + 2; j < *i + 10; ++j)
		{
			if (!strchr("01", Src[j]))
				return 1;
			
			Val <<= 1;
			Val += Src[j] - '0';
		}
		
		DynStr_AppendChar(Str, Len, Val);
		*i += 10;
	}
	else if (!strncmp(&Src[*i], "\\x", 2))
	{
		if (*i + 4 >= SrcLen)
			return 1;
		
		uint8_t Val = 0;
		for (size_t j = *i + 2; j < *i + 4; ++j)
		{
			if (!isxdigit(Src[j]))
				return 1;
			
			Val <<= 4;
			Val += HexDigitValue[(size_t)Src[j]];
		}
		
		DynStr_AppendChar(Str, Len, Val);
		*i += 4;
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
DynStr_AppendStr(char **Str, size_t *Len, char const *Append)
{
	size_t AppendLen = strlen(Append);
	*Str = realloc(*Str, *Len + AppendLen + 1);
	strcpy(*Str + *Len, Append);
	*Len += AppendLen;
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
		LogProgErr(Ps->File, Ps->File->Len, "expected %s at end of file, found nothing!", TokenTypeNames[Type]);
		return NULL;
	}
	
	if (Tok->Type != Type)
	{
		LogTokErr(Ps->File, Tok, "expected %s, found %s!", TokenTypeNames[Type], TokenTypeNames[Tok->Type]);
		return NULL;
	}
	
	return Tok;
}

static int
ExtractImports(
	struct FileData const *File,
	struct ModuleDataGroup *Append,
	struct AstNode const *Ast,
	unsigned Depth
)
{
	// it is the caller's responsibility to clean up `Append`.
	
	for (size_t i = 0; i < Ast->ChildCnt; ++i)
	{
		struct AstNode const *Child = &Ast->Children[i];
		if (Child->Type != ANT_IMPORT)
			continue;
		
		char *Path = ResolveImport(Child);
		if (!Path)
		{
			LogAstNodeErr(File, Child, "import path was unresolved!");
			return 1;
		}
		
		FILE *Fp = OpenFile(Path, "rb");
		if (!Fp)
		{
			LogAstNodeErr(File, Child, "failed to open module file for reading - '%s'!", Path);
			free(Path);
			return 1;
		}
		
		char *FullPath = FullPathname(Path);
		
		// ignore modules with duplicate paths.
		for (size_t Mod = 0; Mod < Append->ModuleCnt; ++Mod)
		{
			if (!strcmp(FullPath, Append->Modules[Mod].FullPath))
			{
				free(FullPath);
				fclose(Fp);
				free(Path);
				continue;
			}
		}
		
		struct FileData FileData = {0};
		if (FileData_Read(&FileData, Fp, Path))
		{
			fclose(Fp);
			free(Path);
			free(FullPath);
			return 1;
		}
		
		fclose(Fp);
		free(Path);
		
		struct LexData LexData = {0};
		if (Lex(&LexData, &FileData))
		{
			FileData_Destroy(&FileData);
			free(FullPath);
			return 1;
		}
		
		struct AstNode Ast = {0};
		if (Parse(&Ast, &FileData, &LexData))
		{
			LexData_Destroy(&LexData);
			FileData_Destroy(&FileData);
			free(FullPath);
			return 1;
		}
		
		struct ModuleData ModuleData =
		{
			.File = FileData,
			.Lex = LexData,
			.Ast = Ast,
			.FullPath = FullPath
		};
		
		ModuleDataGroup_Append(Append, &ModuleData);
	}
	
	return 0;
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

static char *
FullPathname(char const *Path)
{
	char PathBuf[PATH_MAX + 1] = {0};
	realpath(Path, PathBuf);
	return strdup(PathBuf);
}

static uint64_t
GetUnixTimeMs(void)
{
	struct timeval Tv;
	gettimeofday(&Tv, NULL);
	return (uint64_t)Tv.tv_sec * 1000 + (uint64_t)Tv.tv_usec / 1000;
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
	bool NoNewline = false;
	
	for (size_t i = 0; i < Data->Len; ++i)
	{
		// handle comments and whitespace.
		{
			if (Data->Data[i] == '\n')
			{
				InComment = false;
				if (!NoNewline)
					LexData_AddSpecialChar(Out, i, 1, TT_NEWLINE);
				NoNewline = false;
				continue;
			}
			
			if (Data->Data[i] == ';')
			{
				InComment = true;
				continue;
			}
			
			if (InComment || isspace(Data->Data[i]))
				continue;
			else if (Data->Data[i] == '\\')
			{
				NoNewline = true;
				continue;
			}
			else
			{
				if (NoNewline)
				{
					LogProgErr(Data, i, "newline cancels cannot be followed by code!");
					LexData_Destroy(Out);
					return 1;
				}
				NoNewline = false;
			}
		}
		
		// handle non-special characters.
		{
			if (IsIdentInit(Data->Data[i])
				|| (Data->Data[i] == '@' && IsIdentInit(Data->Data[i + 1])))
			{
				struct Token Tok;
				if (LexWord(Data, &Tok, &i))
				{
					LexData_Destroy(Out);
					return 1;
				}
				LexData_AddToken(Out, &Tok);
				--i;
				continue;
			}
			else if (isdigit(Data->Data[i]))
			{
				struct Token Tok;
				if (LexNum(Data, &Tok, &i))
				{
					LexData_Destroy(Out);
					return 1;
				}
				LexData_AddToken(Out, &Tok);
				--i;
				continue;
			}
			else if (Data->Data[i] == '\'')
			{
				struct Token Tok;
				if (LexChar(Data, &Tok, &i))
				{
					LexData_Destroy(Out);
					return 1;
				}
				LexData_AddToken(Out, &Tok);
				--i;
				continue;
			}
			else if (Data->Data[i] == '"')
			{
				struct Token Tok;
				if (LexString(Data, &Tok, &i))
				{
					LexData_Destroy(Out);
					return 1;
				}
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
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], ".^", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_PERIOD_CARET);
					++i;
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
			case '=':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "==", 2))
				{
					LexData_AddSpecialChar(Out, i, 2, TT_DOUBLE_EQUAL);
					++i;
				}
				else
				{
					LogProgErr(Data, i, "expected double '==', found single '='!");
					LexData_Destroy(Out);
					return 1;
				}
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
				LexData_Destroy(Out);
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
			if (ConvEscSequence(Data->Data, Data->Len, i, &ChData, &ChDataLen))
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
LexData_AddSpecialChar(
	struct LexData *Out,
	size_t Pos,
	size_t Len,
	enum TokenType Type
)
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
			if (ConvEscSequence(Data->Data, Data->Len, i, &StrData, &StrDataLen))
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
	if (!IsRaw)
	{
		for (size_t Kw = TT_KW_FIRST__; Kw <= TT_KW_LAST__; ++Kw)
		{
			if (!strcmp(Word, Keywords[Kw - TT_KW_FIRST__]))
			{
				Type = Kw;
				break;
			}
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
LogAstNodeErr(
	struct FileData const *Data,
	struct AstNode const *Node,
	char const *Fmt,
	...
)
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
	
	struct Token const *FirstTok = Node->Toks[0];
	LogProgPosition(Data, FirstTok->Pos, FirstTok->Len, "31");
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
	
	LogProgPosition(Data, i, 1, "31");
}

static void
LogProgPosition(
	struct FileData const *Data,
	size_t Pos,
	size_t Len,
	char const *HlStyle
)
{
	size_t Begin = Pos;
	while (Begin > 0 && Data->Data[Begin - 1] != '\n')
		--Begin;
	
	// write out line contents.
	{
		size_t End = Pos;
		while (End < Data->Len && Data->Data[End] != '\n')
			++End;
		
		fprintf(
			stderr,
			"\x1b[2m[line %zu (byte %zu)]\n"
			"\\->\x1b[0m ",
			LineNumber(Data->Data, Pos),
			Pos
		);
		
		for (size_t i = Begin; i < End; ++i)
		{
			fprintf(
				stderr,
				"%c",
				Data->Data[i] == '\t' ? ' ' : Data->Data[i]
			);
		}
		
		fprintf(stderr, "\n    ");
	}
	
	// write out highlight indicator.
	{
		for (size_t i = Begin; i < Pos; ++i)
			fprintf(stderr, " ");
		
		fprintf(stderr, "\x1b[%sm^", HlStyle);
		for (size_t i = 1; i < Len; ++i)
			fprintf(stderr, "~");
		fprintf(stderr, "\x1b[0m\n");
	}
}

static void
LogTokErr(
	struct FileData const *Data,
	struct Token const *Tok,
	char const *Fmt,
	...
)
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
	
	LogProgPosition(Data, Tok->Pos, Tok->Len, "31");
}

static void
ModuleData_Destroy(struct ModuleData *Data)
{
	// free resources.
	{
		AstNode_Destroy(&Data->Ast);
		LexData_Destroy(&Data->Lex);
		FileData_Destroy(&Data->File);
		free(Data->FullPath);
	}
}

static void
ModuleDataGroup_Append(
	struct ModuleDataGroup *Group,
	struct ModuleData const *Data
)
{
	++Group->ModuleCnt;
	Group->Modules = reallocarray(
		Group->Modules,
		Group->ModuleCnt,
		sizeof(struct ModuleData)
	);
	Group->Modules[Group->ModuleCnt - 1] = *Data;
}

static void
ModuleDataGroup_Destroy(struct ModuleDataGroup *Group)
{
	for (size_t i = 0; i < Group->ModuleCnt; ++i)
		ModuleData_Destroy(&Group->Modules[i]);
	free(Group->Modules);
}

static struct Token const *
NextToken(struct ParseState *Ps)
{
	++Ps->i;
	struct Token const *Tok = Ps->i >= Ps->Lex->TokCnt ? NULL : &Ps->Lex->Toks[Ps->i];
	return Tok;
}

static FILE *
OpenFile(char const *File, char const *Mode)
{
	struct stat Stat;
	if (stat(File, &Stat) || !S_ISREG(Stat.st_mode))
		return NULL;
	return fopen(File, Mode);
}

static int
Parse(
	struct AstNode *Out,
	struct FileData const *File,
	struct LexData const *Lex
)
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
ParseArgList(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = ExpectToken(Ps, TT_PBEGIN);
	if (!FirstTok)
		return 1;
	
	struct AstNode ArgList =
	{
		.Type = ANT_ARG_LIST
	};
	
	for (;;)
	{
		struct Token const *Next = RequireToken(Ps);
		if (!Next)
		{
			AstNode_Destroy(&ArgList);
			return 1;
		}
		
		switch (Next->Type)
		{
		case TT_TRIPLE_PERIOD:
			if (!ExpectToken(Ps, TT_PEND))
			{
				AstNode_Destroy(&ArgList);
				return 1;
			}
			ArgList.Flags |= ANF_VARIADIC;
			goto Done;
		case TT_KW_BASE:
			if (!ExpectToken(Ps, TT_TRIPLE_PERIOD))
			{
				AstNode_Destroy(&ArgList);
				return 1;
			}
			if (!ExpectToken(Ps, TT_PEND))
			{
				AstNode_Destroy(&ArgList);
				return 1;
			}
			ArgList.Flags |= ANF_BASE;
			ArgList.Flags |= ANF_VARIADIC;
			goto Done;
		case TT_KW_SELF:
		{
			--Ps->i;
			
			struct AstNode ArgType = {0};
			unsigned char Term[] = {TT_COMMA, TT_PEND};
			if (ParseWrappedType(&ArgType, Ps, Term, 2))
			{
				AstNode_Destroy(&ArgList);
				return 1;
			}
			
			struct AstNode Arg =
			{
				.Type = ANT_ARG
			};
			AstNode_AddChild(&Arg, &ArgType);
			AstNode_AddToken(&Arg, Next);
			
			AstNode_AddChild(&ArgList, &Arg);
			
			if (Ps->Lex->Toks[Ps->i].Type == TT_PEND)
				goto Done;
			
			break;
		}
		case TT_PEND:
			if (PeekPrevToken(Ps)->Type == TT_COMMA)
			{
				LogTokErr(Ps->File, Next, "expected another argument, found TT_PEND!");
				AstNode_Destroy(&ArgList);
				return 1;
			}
			goto Done;
		case TT_IDENT:
		{
			struct AstNode ArgType = {0};
			unsigned char Term[] = {TT_COMMA, TT_PEND};
			if (ParseWrappedType(&ArgType, Ps, Term, 2))
			{
				AstNode_Destroy(&ArgList);
				return 1;
			}
			
			struct AstNode Arg =
			{
				.Type = ANT_ARG
			};
			AstNode_AddChild(&Arg, &ArgType);
			AstNode_AddToken(&Arg, Next);
			
			AstNode_AddChild(&ArgList, &Arg);
			
			if (Ps->Lex->Toks[Ps->i].Type == TT_PEND)
				goto Done;
			
			break;
		}
		default:
			LogTokErr(Ps->File, Next, "expected TT_TRIPLE_PERIOD, TT_KW_BASE, or TT_IDENT!");
			AstNode_Destroy(&ArgList);
			return 1;
		}
	}
Done:
	AstNode_AddToken(&ArgList, FirstTok);
	*Out = ArgList;
	
	return 0;
}

static int
ParseBlock(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = ExpectToken(Ps, TT_KW_BLOCK);
	if (!FirstTok)
		return 1;
	
	struct AstNode Block =
	{
		.Type = ANT_BLOCK
	};
	
	// block name if present.
	struct Token const *Name = NULL;
	{
		struct Token const *Next = PeekToken(Ps);
		if (Next && Next->Type == TT_AT_QUOTE)
		{
			++Ps->i;
			Name = ExpectToken(Ps, TT_IDENT);
			if (!Name)
				return 1;
		}
	}
	
	// block contents.
	struct AstNode StmtList = {0};
	{
		unsigned char Term[] = {TT_KW_END};
		if (ParseStatementList(&StmtList, Ps, Term, 1))
			return 1;
	}
	
	AstNode_AddChild(&Block, &StmtList);
	AstNode_AddToken(&Block, FirstTok);
	if (Name)
		AstNode_AddToken(&Block, Name);
	
	*Out = Block;
	
	return 0;
}

static int
ParseBreak(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = ExpectToken(Ps, TT_KW_BREAK);
	if (!FirstTok)
		return 1;
	
	struct Token const *Name = NULL;
	
	struct Token const *Next = PeekToken(Ps);
	if (Next && Next->Type == TT_AT_QUOTE)
	{
		++Ps->i;
		Name = ExpectToken(Ps, TT_IDENT);
		if (!Name)
			return 1;
	}
	
	if (!ExpectToken(Ps, TT_NEWLINE))
		return 1;
	
	Out->Type = ANT_BREAK;
	AstNode_AddToken(Out, FirstTok);
	if (Name)
		AstNode_AddToken(Out, Name);
	
	return 0;
}

static int
ParseCondTree(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = RequireToken(Ps);
	if (!FirstTok)
		return 1;
	
	switch (FirstTok->Type)
	{
	case TT_KW_IF:
	case TT_KW_ELIF:
		break;
	default:
		LogTokErr(Ps->File, FirstTok, "expected either TT_KW_IF or TT_KW_ELIF for condition tree!");
		return 1;
	}
	
	struct AstNode CondTree =
	{
		.Type = ANT_COND_TREE
	};
	
	// necessary condition tree info.
	{
		struct AstNode Cond = {0};
		unsigned char Term[] = {TT_NEWLINE};
		if (ParseWrappedExpr(&Cond, Ps, Term, 1))
			return 1;
		
		AstNode_AddChild(&CondTree, &Cond);
	}
	
	// condition tree body and children.
	{
		struct AstNode TruePath = {0};
		unsigned char Term[] = {TT_KW_END, TT_KW_ELIF, TT_KW_ELSE};
		if (ParseStatementList(&TruePath, Ps, Term, 3))
		{
			AstNode_Destroy(&CondTree);
			return 1;
		}
		
		AstNode_AddChild(&CondTree, &TruePath);
		
		switch (Ps->Lex->Toks[Ps->i].Type)
		{
		case TT_KW_END:
			break;
		case TT_KW_ELIF:
		{
			--Ps->i;
			
			struct AstNode FalsePath = {0};
			if (ParseCondTree(&FalsePath, Ps))
			{
				AstNode_Destroy(&CondTree);
				return 1;
			}
			
			AstNode_AddChild(&CondTree, &FalsePath);
			
			break;
		}
		case TT_KW_ELSE:
		{
			struct AstNode FalsePath = {0};
			unsigned char Term[] = {TT_KW_END};
			if (ParseStatementList(&FalsePath, Ps, Term, 1))
			{
				AstNode_Destroy(&CondTree);
				return 1;
			}
			
			AstNode_AddChild(&CondTree, &FalsePath);
			
			break;
		}
		default:
			break;
		}
	}
	
	*Out = CondTree;
	
	return 0;
}

static int
ParseContinue(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = ExpectToken(Ps, TT_KW_CONTINUE);
	if (!FirstTok)
		return 1;
	
	struct Token const *Name = NULL;
	
	struct Token const *Next = PeekToken(Ps);
	if (Next && Next->Type == TT_AT_QUOTE)
	{
		++Ps->i;
		Name = ExpectToken(Ps, TT_IDENT);
		if (!Name)
			return 1;
	}
	
	if (!ExpectToken(Ps, TT_NEWLINE))
		return 1;
	
	Out->Type = ANT_CONTINUE;
	AstNode_AddToken(Out, FirstTok);
	if (Name)
		AstNode_AddToken(Out, Name);
	
	return 0;
}

static int
ParseDefer(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = ExpectToken(Ps, TT_KW_DEFER);
	if (!FirstTok)
		return 1;
	
	struct AstNode Stmt = {0};
	if (ParseStatement(&Stmt, Ps))
		return 1;
	
	Out->Type = ANT_DEFER;
	AstNode_AddChild(Out, &Stmt);
	AstNode_AddToken(Out, FirstTok);
	
	return 0;
}

static int
ParseEnum(struct AstNode *Out, struct ParseState *Ps)
{
	if (!ExpectToken(Ps, TT_KW_ENUM))
		return 1;
	
	struct AstNode Enum =
	{
		.Type = ANT_ENUM
	};
	
	// base enum information.
	{
		struct Token const *Vis = PeekToken(Ps);
		if (Vis && Vis->Type == TT_ASTERISK)
		{
			Enum.Flags |= ANF_PUBLIC;
			++Ps->i;
		}
		
		struct Token const *Name = ExpectToken(Ps, TT_IDENT);
		if (!Name)
			return 1;
		
		struct AstNode Type = {0};
		unsigned char Term[] = {TT_NEWLINE};
		if (ParseWrappedType(&Type, Ps, Term, 1))
			return 1;
		
		AstNode_AddChild(&Enum, &Type);
		AstNode_AddToken(&Enum, Name);
	}
	
	// get enum member information.
	for (;;)
	{
		SkipParseNewlines(Ps);
		struct Token const *MembName = ExpectToken(Ps, TT_IDENT);
		if (!MembName)
		{
			AstNode_Destroy(&Enum);
			return 1;
		}
		
		struct Token const *Next = RequireToken(Ps);
		if (!Next)
		{
			AstNode_Destroy(&Enum);
			return 1;
		}
		
		struct AstNode Memb =
		{
			.Type = ANT_ENUM_MEMBER
		};
		
		switch (Next->Type)
		{
		case TT_COLON_EQUAL:
		{
			struct AstNode Value = {0};
			unsigned char Term[] = {TT_NEWLINE};
			if (ParseWrappedExpr(&Value, Ps, Term, 1))
			{
				AstNode_Destroy(&Enum);
				return 1;
			}
			
			AstNode_AddChild(&Memb, &Value);
			AstNode_AddToken(&Memb, MembName);
			
			AstNode_AddChild(&Enum, &Memb);
			
			break;
		}
		case TT_NEWLINE:
			AstNode_AddToken(&Memb, MembName);
			AstNode_AddChild(&Enum, &Memb);
			break;
		default:
			AstNode_Destroy(&Enum);
			LogTokErr(Ps->File, Next, "expected either TT_COLON_EQUAL or TT_NEWLINE!");
			break;
		}
		
		Next = RequireToken(Ps);
		if (!Next)
		{
			AstNode_Destroy(&Enum);
			return 1;
		}
		
		if (Next->Type == TT_KW_END)
			break;
		
		--Ps->i;
	}
	
	*Out = Enum;
	
	return 0;
}

static int
ParseExpr(
	struct AstNode *Out,
	struct ParseState *Ps,
	unsigned char const Term[],
	size_t TermCnt,
	int MinBp
)
{
	struct Token const *Tok = RequireToken(Ps);
	if (!Tok)
		return 1;
	
	// get base LHS operand of expression.
	struct AstNode Lhs = {0};
	switch (Tok->Type)
	{
	case TT_IDENT:
	case TT_LIT_STR:
	case TT_LIT_INT:
	case TT_LIT_FLOAT:
	case TT_LIT_BOOL:
	case TT_KW_SELF:
	case TT_KW_VARGCOUNT:
		Lhs.Type = ANT_EXPR_ATOM;
		AstNode_AddToken(&Lhs, Tok);
		break;
	case TT_KW_BASE:
	{
		struct Token const *Modified = RequireToken(Ps);
		if (!Modified)
			return 1;
		
		switch (Modified->Type)
		{
		case TT_LIT_STR:
			Lhs.Type = ANT_EXPR_ATOM;
			Lhs.Flags |= ANF_BASE;
			AstNode_AddToken(&Lhs, Tok);
			break;
		case TT_BKBEGIN:
			--Ps->i;
			if (ParseExprList(&Lhs, Ps))
				return 1;
			Lhs.Flags |= ANF_BASE;
			break;
		default:
			LogTokErr(Ps->File, Modified, "%s is not base-modifiable!", TokenTypeNames[Modified->Type]);
			return 1;
		}
		
		break;
	}
	case TT_BKBEGIN:
		--Ps->i;
		if (ParseExprList(&Lhs, Ps))
			return 1;
		break;
	case TT_PBEGIN:
	{
		unsigned char Term[] = {TT_PEND};
		if (ParseExpr(&Lhs, Ps, Term, 1, 0))
			return 1;
		++Ps->i;
		
		break;
	}
	default:
	{
		enum AstNodeType NudType = TokenTypeToNud(Tok->Type);
		if (NudType == -1)
		{
			LogTokErr(Ps->File, Tok, "expected null denotation!");
			return 1;
		}
		
		if (ParseExprNud(&Lhs, Ps, Term, TermCnt))
			return 1;
		
		break;
	}
	}
	
	// process operations.
	for (;;)
	{
		Tok = RequireToken(Ps);
		if (!Tok)
		{
			AstNode_Destroy(&Lhs);
			return 1;
		}
		--Ps->i;
		
		for (size_t i = 0; i < TermCnt; ++i)
		{
			if (Term[i] == Tok->Type)
				goto Done;
		}
		
		enum AstNodeType LedType = TokenTypeToLed(Tok->Type);
		if (LedType != -1)
		{
			struct BindPower Bp = ExprBindPower[LedType - ANT_EXPR];
			if (Bp.Left < MinBp)
				break;
			
			struct AstNode NewLhs = {0};
			if (ParseExprLed(&NewLhs, Ps, &Lhs, Term, TermCnt))
			{
				AstNode_Destroy(&Lhs);
				return 1;
			}
			
			Lhs = NewLhs;
		}
		else
		{
			LogTokErr(Ps->File, Tok, "expected left denotation!");
			AstNode_Destroy(&Lhs);
			return 1;
		}
	}
Done:
	*Out = Lhs;
	
	return 0;
}

static int
ParseExprLed(
	struct AstNode *Out,
	struct ParseState *Ps,
	struct AstNode const *Lhs,
	unsigned char const Term[],
	size_t TermCnt
)
{
	struct Token const *Tok = NextToken(Ps);
	enum AstNodeType Type = TokenTypeToLed(Tok->Type);
	
	struct AstNode NewLhs =
	{
		.Type = Type
	};
	
	switch (Type)
	{
	case ANT_EXPR_CALL:
	{
		AstNode_AddChild(&NewLhs, Lhs);
		
		struct Token const *Next = RequireToken(Ps);
		if (!Next)
		{
			AstNode_Destroy(&NewLhs);
			return 1;
		}
		
		if (Next->Type != TT_PEND)
		{
			--Ps->i;
			
			for (;;)
			{
				struct AstNode Arg = {0};
				unsigned char Term[] = {TT_COMMA, TT_PEND};
				if (ParseExpr(&Arg, Ps, Term, 2, 0))
				{
					AstNode_Destroy(&NewLhs);
					return 1;
				}
				++Ps->i;
				
				AstNode_AddChild(&NewLhs, &Arg);
				
				if (Ps->Lex->Toks[Ps->i].Type == TT_PEND)
					break;
			}
		}
		
		AstNode_AddToken(&NewLhs, Tok);
		
		break;
	}
	case ANT_EXPR_CAST:
	{
		if (!ExpectToken(Ps, TT_BKBEGIN))
			return 1;
		
		struct AstNode Rhs = {0};
		unsigned char Term[] = {TT_BKEND};
		if (ParseWrappedType(&Rhs, Ps, Term, 1))
			return 1;
		
		AstNode_AddChild(&NewLhs, Lhs);
		AstNode_AddChild(&NewLhs, &Rhs);
		AstNode_AddToken(&NewLhs, Tok);
		
		break;
	}
	case ANT_EXPR_TERNARY:
	{
		struct AstNode Mhs = {0};
		unsigned char MhsTerm[] = {TT_COLON};
		if (ParseExpr(&Mhs, Ps, MhsTerm, 1, 0))
			return 1;
		++Ps->i;
		
		struct BindPower Bp = ExprBindPower[Type - ANT_EXPR];
		struct AstNode Rhs = {0};
		if (ParseExpr(&Rhs, Ps, Term, TermCnt, Bp.Right))
		{
			AstNode_Destroy(&Mhs);
			return 1;
		}
		
		AstNode_AddChild(&NewLhs, Lhs);
		AstNode_AddChild(&NewLhs, &Mhs);
		AstNode_AddChild(&NewLhs, &Rhs);
		AstNode_AddToken(&NewLhs, Tok);
		
		break;
	}
	case ANT_EXPR_NTH:
	case ANT_EXPR_ACCESS:
	case ANT_EXPR_MUL:
	case ANT_EXPR_DIV:
	case ANT_EXPR_MOD:
	case ANT_EXPR_ADD:
	case ANT_EXPR_SUB:
	case ANT_EXPR_SHR:
	case ANT_EXPR_SHL:
	case ANT_EXPR_BIT_AND:
	case ANT_EXPR_BIT_XOR:
	case ANT_EXPR_BIT_OR:
	case ANT_EXPR_GREATER:
	case ANT_EXPR_GREQUAL:
	case ANT_EXPR_LESS:
	case ANT_EXPR_LEQUAL:
	case ANT_EXPR_EQUAL:
	case ANT_EXPR_NEQUAL:
	case ANT_EXPR_LOG_AND:
	case ANT_EXPR_LOG_XOR:
	case ANT_EXPR_LOG_OR:
	case ANT_EXPR_ASSIGN:
	case ANT_EXPR_ADD_ASSIGN:
	case ANT_EXPR_SUB_ASSIGN:
	case ANT_EXPR_MUL_ASSIGN:
	case ANT_EXPR_DIV_ASSIGN:
	case ANT_EXPR_MOD_ASSIGN:
	case ANT_EXPR_SHR_ASSIGN:
	case ANT_EXPR_SHL_ASSIGN:
	case ANT_EXPR_BIT_AND_ASSIGN:
	case ANT_EXPR_BIT_XOR_ASSIGN:
	case ANT_EXPR_BIT_OR_ASSIGN:
	{
		struct BindPower Bp = ExprBindPower[Type - ANT_EXPR];
		struct AstNode Rhs = {0};
		if (ParseExpr(&Rhs, Ps, Term, TermCnt, Bp.Right))
			return 1;
		
		AstNode_AddChild(&NewLhs, Lhs);
		AstNode_AddChild(&NewLhs, &Rhs);
		AstNode_AddToken(&NewLhs, Tok);
		
		break;
	}
	case ANT_EXPR_POST_INC:
	case ANT_EXPR_POST_DEC:
	case ANT_EXPR_ADDR_OF:
	case ANT_EXPR_DEREF:
		AstNode_AddChild(&NewLhs, Lhs);
		AstNode_AddToken(&NewLhs, Tok);
		break;
	default:
		break;
	}
	
	*Out = NewLhs;
	
	return 0;
}

static int
ParseExprList(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = ExpectToken(Ps, TT_BKBEGIN);
	if (!FirstTok)
		return 1;
	
	struct AstNode List =
	{
		.Type = ANT_EXPR_LIST
	};
	
	for (;;)
	{
		struct AstNode Item = {0};
		unsigned char Term[] = {TT_COMMA, TT_BKEND};
		if (ParseExpr(&Item, Ps, Term, 2, 0))
		{
			AstNode_Destroy(&List);
			return 1;
		}
		++Ps->i;
		
		AstNode_AddChild(&List, &Item);
		
		if (Ps->Lex->Toks[Ps->i].Type == TT_BKEND)
			break;
	}
	
	AstNode_AddToken(&List, FirstTok);
	*Out = List;
	
	return 0;
}

static int
ParseExprNud(
	struct AstNode *Out,
	struct ParseState *Ps,
	unsigned char const Term[],
	size_t TermCnt
)
{
	struct Token const *Tok = &Ps->Lex->Toks[Ps->i];
	enum AstNodeType Type = TokenTypeToNud(Tok->Type);
	
	struct AstNode Lhs =
	{
		.Type = Type
	};
	
	switch (Type)
	{
	case ANT_EXPR_LAMBDA:
	{
		struct AstNode ArgList = {0};
		if (ParseArgList(&ArgList, Ps))
			return 1;
		AstNode_AddChild(&Lhs, &ArgList);
		
		struct AstNode ReturnType = {0};
		unsigned char ReturnTypeTerm[] = {TT_NEWLINE};
		if (ParseWrappedType(&ReturnType, Ps, ReturnTypeTerm, 1))
		{
			AstNode_Destroy(&Lhs);
			return 1;
		}
		AstNode_AddChild(&Lhs, &ReturnType);
		
		struct AstNode StmtList = {0};
		unsigned char LambdaTerm[] = {TT_KW_END};
		if (ParseStatementList(&StmtList, Ps, LambdaTerm, 1))
		{
			AstNode_Destroy(&Lhs);
			return 1;
		}
		AstNode_AddChild(&Lhs, &StmtList);
		
		AstNode_AddToken(&Lhs, Tok);
		
		break;
	}
	case ANT_EXPR_STRUCT:
	case ANT_EXPR_UNION:
		if (ParseTypeLiteral(&Lhs, Ps))
			return 1;
		break;
	case ANT_EXPR_SIZEOF:
	{
		struct Token const *Next = RequireToken(Ps);
		if (!Next)
			return 1;
		
		switch (Next->Type)
		{
		case TT_PBEGIN:
		{
			struct AstNode Rhs = {0};
			unsigned char Term[] = {TT_PEND};
			if (ParseExpr(&Rhs, Ps, Term, 1, 0))
				return 1;
			++Ps->i;
			
			AstNode_AddChild(&Lhs, &Rhs);
			
			break;
		}
		case TT_BKBEGIN:
		{
			struct AstNode Rhs = {0};
			unsigned char Term[] = {TT_BKEND};
			if (ParseWrappedType(&Rhs, Ps, Term, 1))
				return 1;
			
			AstNode_AddChild(&Lhs, &Rhs);
			
			break;
		}
		default:
			LogTokErr(Ps->File, Next, "expected TT_PBEGIN or TT_BKBEGIN!");
			return 1;
		}
		
		AstNode_AddToken(&Lhs, Tok);
		
		break;
	}
	case ANT_EXPR_LENOF:
	{
		if (!ExpectToken(Ps, TT_PBEGIN))
			return 1;
		
		struct AstNode Rhs = {0};
		unsigned char Term[] = {TT_PEND};
		if (ParseExpr(&Rhs, Ps, Term, 1, 0))
			return 1;
		++Ps->i;
		
		AstNode_AddChild(&Lhs, &Rhs);
		AstNode_AddToken(&Lhs, Tok);
		
		break;
	}
	case ANT_EXPR_NEXTVARG:
	{
		if (!ExpectToken(Ps, TT_BKBEGIN))
			return 1;
		
		struct AstNode Rhs = {0};
		unsigned char Term[] = {TT_BKEND};
		if (ParseWrappedType(&Rhs, Ps, Term, 1))
			return 1;
		
		AstNode_AddChild(&Lhs, &Rhs);
		AstNode_AddToken(&Lhs, Tok);
		
		break;
	}
	case ANT_EXPR_NULL:
	{
		struct Token const *Peek = PeekToken(Ps);
		if (Peek && Peek->Type == TT_BKBEGIN)
		{
			++Ps->i;
			
			struct AstNode Rhs = {0};
			unsigned char Term[] = {TT_BKEND};
			if (ParseWrappedType(&Rhs, Ps, Term, 1))
				return 1;
			
			AstNode_AddChild(&Lhs, &Rhs);
			AstNode_AddToken(&Lhs, Tok);
		}
		else
		{
			Lhs.Type = ANT_EXPR_ATOM;
			AstNode_AddToken(&Lhs, Tok);
		}
		
		break;
	}
	case ANT_EXPR_PRE_INC:
	case ANT_EXPR_PRE_DEC:
	case ANT_EXPR_UNARY_MINUS:
	case ANT_EXPR_LOG_NOT:
	case ANT_EXPR_BIT_NOT:
	case ANT_EXPR_DEREF:
	{
		struct BindPower Bp = ExprBindPower[Type - ANT_EXPR];
		struct AstNode Rhs = {0};
		if (ParseExpr(&Rhs, Ps, Term, TermCnt, Bp.Right))
			return 1;
		
		AstNode_AddChild(&Lhs, &Rhs);
		AstNode_AddToken(&Lhs, Tok);
		
		break;
	}
	default:
		break;
	}
	
	*Out = Lhs;
	
	return 0;
}

static int
ParseFor(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = ExpectToken(Ps, TT_KW_FOR);
	if (!FirstTok)
		return 1;
	
	struct AstNode For =
	{
		.Type = ANT_FOR
	};
	AstNode_AddToken(&For, FirstTok);
	
	// get label data.
	{
		struct Token const *Next = PeekToken(Ps);
		if (Next && Next->Type == TT_AT_QUOTE)
		{
			++Ps->i;
			struct Token const *Name = ExpectToken(Ps, TT_IDENT);
			if (!Name)
			{
				AstNode_Destroy(&For);
				return 1;
			}
			AstNode_AddToken(&For, Name);
		}
	}
	
	// get condition / iteration parameters.
	{
		struct Token const *Next = PeekToken(Ps);
		if (Next && Next->Type == TT_KW_VAR)
		{
			struct AstNode Init = {0};
			unsigned char InitTerm[] = {TT_COMMA};
			if (ParseVar(&Init, Ps, InitTerm, 1))
			{
				AstNode_Destroy(&For);
				return 1;
			}
			AstNode_AddChild(&For, &Init);
			
			struct AstNode Cond = {0};
			unsigned char CondTerm[] = {TT_COMMA};
			if (ParseWrappedExpr(&Cond, Ps, CondTerm, 1))
			{
				AstNode_Destroy(&For);
				return 1;
			}
			AstNode_AddChild(&For, &Cond);
			
			struct AstNode Inc = {0};
			unsigned char IncTerm[] = {TT_NEWLINE};
			if (ParseWrappedExpr(&Inc, Ps, IncTerm, 1))
			{
				AstNode_Destroy(&For);
				return 1;
			}
			AstNode_AddChild(&For, &Inc);
		}
		else
		{
			struct AstNode FirstValue = {0};
			unsigned char Term[] = {TT_NEWLINE, TT_COMMA};
			if (ParseWrappedExpr(&FirstValue, Ps, Term, 2))
			{
				AstNode_Destroy(&For);
				return 1;
			}
			AstNode_AddChild(&For, &FirstValue);
			
			if (Ps->Lex->Toks[Ps->i].Type == TT_COMMA)
			{
				struct AstNode Cond = {0};
				unsigned char CondTerm[] = {TT_COMMA};
				if (ParseWrappedExpr(&Cond, Ps, CondTerm, 1))
				{
					AstNode_Destroy(&For);
					return 1;
				}
				AstNode_AddChild(&For, &Cond);
				
				struct AstNode Inc = {0};
				unsigned char IncTerm[] = {TT_NEWLINE};
				if (ParseWrappedExpr(&Inc, Ps, IncTerm, 1))
				{
					AstNode_Destroy(&For);
					return 1;
				}
				AstNode_AddChild(&For, &Inc);
			}
		}
	}
	
	// get contained code.
	{
		struct AstNode StmtList = {0};
		unsigned char Term[] = {TT_KW_END};
		if (ParseStatementList(&StmtList, Ps, Term, 1))
		{
			AstNode_Destroy(&For);
			return 1;
		}
		
		AstNode_AddChild(&For, &StmtList);
	}
	
	*Out = For;
	
	return 0;
}

static int
ParseImport(struct AstNode *Out, struct ParseState *Ps)
{
	if (!ExpectToken(Ps, TT_KW_IMPORT))
		return 1;
	
	struct Token const *Vis = PeekToken(Ps);
	if (Vis && Vis->Type == TT_ASTERISK)
	{
		Out->Flags |= ANF_PUBLIC;
		++Ps->i;
	}
	
	for (;;)
	{
		struct Token const *Target = ExpectToken(Ps, TT_IDENT);
		if (!Target)
		{
			AstNode_Destroy(Out);
			return 1;
		}
		
		struct Token const *Tok = NextToken(Ps);
		if (!Tok)
		{
			LogTokErr(Ps->File, Target, "expected TT_PERIOD or TT_NEWLINE after target!");
			AstNode_Destroy(Out);
			return 1;
		}
		
		switch (Tok->Type)
		{
		case TT_PERIOD:
			AstNode_AddToken(Out, Target);
			break;
		case TT_NEWLINE:
			AstNode_AddToken(Out, Target);
			goto Done;
		default:
			LogTokErr(Ps->File, Tok, "expected TT_PERIOD or TT_NEWLINE!");
			AstNode_Destroy(Out);
			return 1;
		}
	}
Done:
	
	Out->Type = ANT_IMPORT;
	
	return 0;
}

static int
ParseProc(struct AstNode *Out, struct ParseState *Ps)
{
	if (!ExpectToken(Ps, TT_KW_PROC))
		return 1;
	
	struct AstNode Proc =
	{
		.Type = ANT_PROC
	};
	
	// base procedure information.
	{
		struct Token const *Vis = PeekToken(Ps);
		if (Vis && Vis->Type == TT_ASTERISK)
		{
			Proc.Flags |= ANF_PUBLIC;
			++Ps->i;
		}
		
		struct Token const *NameLhs = ExpectToken(Ps, TT_IDENT);
		if (!NameLhs)
			return 1;
		
		struct Token const *NameRhs = NULL;
		
		struct Token const *Next = PeekToken(Ps);
		if (Next && Next->Type == TT_PERIOD)
		{
			++Ps->i;
			
			NameRhs = ExpectToken(Ps, TT_IDENT);
			if (!NameRhs)
				return 1;
		}
		
		AstNode_AddToken(&Proc, NameLhs);
		if (NameRhs)
			AstNode_AddToken(&Proc, NameRhs);
	}
	
	// argument and return type information.
	{
		struct AstNode Args = {0};
		if (ParseArgList(&Args, Ps))
		{
			AstNode_Destroy(&Proc);
			return 1;
		}
		
		AstNode_AddChild(&Proc, &Args);
		
		struct AstNode ReturnType = {0};
		unsigned char Term[] = {TT_NEWLINE};
		if (ParseWrappedType(&ReturnType, Ps, Term, 1))
		{
			AstNode_Destroy(&Proc);
			return 1;
		}
		
		AstNode_AddChild(&Proc, &ReturnType);
	}
	
	// procedure contents.
	{
		struct AstNode StmtList = {0};
		unsigned char Term[] = {TT_KW_END};
		if (ParseStatementList(&StmtList, Ps, Term, 1))
		{
			AstNode_Destroy(&Proc);
			return 1;
		}
		
		AstNode_AddChild(&Proc, &StmtList);
	}
	
	*Out = Proc;
	
	return 0;
}

static int
ParseProgram(struct AstNode *Out, struct ParseState *Ps)
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
			struct AstNode Child = {0};
			if (ParseImport(&Child, Ps))
			{
				AstNode_Destroy(Out);
				return 1;
			}
			AstNode_AddChild(Out, &Child);
			break;
		}
		case TT_KW_PROC:
		case TT_KW_EXTERNPROC:
		{
			struct AstNode Child = {0};
			if (ParseProc(&Child, Ps))
			{
				AstNode_Destroy(Out);
				return 1;
			}
			AstNode_AddChild(Out, &Child);
			break;
		}
		case TT_KW_VAR:
		case TT_KW_EXTERNVAR:
		{
			struct AstNode Child = {0};
			unsigned char Term[] = {TT_NEWLINE};
			if (ParseVar(&Child, Ps, Term, 1))
			{
				AstNode_Destroy(Out);
				return 1;
			}
			AstNode_AddChild(Out, &Child);
			break;
		}
		case TT_KW_STRUCT:
		{
			struct AstNode Child = {0};
			if (ParseStruct(&Child, Ps))
			{
				AstNode_Destroy(Out);
				return 1;
			}
			AstNode_AddChild(Out, &Child);
			break;
		}
		case TT_KW_ENUM:
		{
			struct AstNode Child = {0};
			if (ParseEnum(&Child, Ps))
			{
				AstNode_Destroy(Out);
				return 1;
			}
			AstNode_AddChild(Out, &Child);
			break;
		}
		case TT_KW_UNION:
		{
			struct AstNode Child = {0};
			if (ParseUnion(&Child, Ps))
			{
				AstNode_Destroy(Out);
				return 1;
			}
			AstNode_AddChild(Out, &Child);
			break;
		}
		case TT_KW_TYPEALIAS:
		{
			struct AstNode Child = {0};
			if (ParseTypeAlias(&Child, Ps))
			{
				AstNode_Destroy(Out);
				return 1;
			}
			AstNode_AddChild(Out, &Child);
			break;
		}
		case TT_NEWLINE:
			++Ps->i;
			break;
		default:
			LogTokErr(Ps->File, Tok, "expected global scope element!");
			AstNode_Destroy(Out);
			return 1;
		}
	}
	
	Out->Type = ANT_PROGRAM;
	
	return 0;
}

static int
ParseResetVargs(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *Tok = ExpectToken(Ps, TT_KW_RESETVARGS);
	if (!Tok)
		return 1;
	
	*Out = (struct AstNode)
	{
		.Type = ANT_RESET_VARGS
	};
	AstNode_AddToken(Out, Tok);
	
	return 0;
}

static int
ParseReturn(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = ExpectToken(Ps, TT_KW_RETURN);
	if (!FirstTok)
		return 1;
	
	struct Token const *Next = RequireToken(Ps);
	if (!Next)
		return 1;
	
	switch (Next->Type)
	{
	case TT_NEWLINE:
		break;
	default:
	{
		--Ps->i;
		
		struct AstNode Value = {0};
		unsigned char Term[] = {TT_NEWLINE};
		if (ParseWrappedExpr(&Value, Ps, Term, 1))
			return 1;
		
		AstNode_AddChild(Out, &Value);
		
		break;
	}
	}
	
	Out->Type = ANT_RETURN;
	AstNode_AddToken(Out, FirstTok);
	
	return 0;
}

static int
ParseStatement(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *Which = RequireToken(Ps);
	if (!Which)
		return 1;
	--Ps->i;
	
	switch (Which->Type)
	{
	case TT_KW_BLOCK:
		return ParseBlock(Out, Ps);
	case TT_KW_BREAK:
		return ParseBreak(Out, Ps);
	case TT_KW_CONTINUE:
		return ParseContinue(Out, Ps);
	case TT_KW_DEFER:
		return ParseDefer(Out, Ps);
	case TT_KW_FOR:
		return ParseFor(Out, Ps);
	case TT_KW_IF:
		return ParseCondTree(Out, Ps);
	case TT_KW_RESETVARGS:
		return ParseResetVargs(Out, Ps);
	case TT_KW_RETURN:
		return ParseReturn(Out, Ps);
	case TT_KW_SWITCH:
		return ParseSwitch(Out, Ps);
	case TT_KW_VAR:
	{
		unsigned char Term[] = {TT_NEWLINE};
		return ParseVar(Out, Ps, Term, 1);
	}
	default:
	{
		unsigned char Term[] = {TT_NEWLINE};
		return ParseWrappedExpr(Out, Ps, Term, 1);
	}
	}
}

static int
ParseStatementList(
	struct AstNode *Out,
	struct ParseState *Ps,
	unsigned char const Term[],
	size_t TermCnt
)
{
	struct AstNode StmtList =
	{
		.Type = ANT_STATEMENT_LIST
	};
	
	for (;;)
	{
		SkipParseNewlines(Ps);
		
		struct Token const *Next = PeekToken(Ps);
		for (size_t i = 0; i < TermCnt; ++i)
		{
			if (Term[i] == Next->Type)
			{
				++Ps->i;
				goto Done;
			}
		}
		
		struct AstNode Stmt = {0};
		if (ParseStatement(&Stmt, Ps))
		{
			AstNode_Destroy(&StmtList);
			return 1;
		}
		
		AstNode_AddChild(&StmtList, &Stmt);
	}
Done:
	*Out = StmtList;
	
	return 0;
}

static int
ParseStruct(struct AstNode *Out, struct ParseState *Ps)
{
	if (!ExpectToken(Ps, TT_KW_STRUCT))
		return 1;
	
	struct AstNode Struct =
	{
		.Type = ANT_STRUCT
	};
	
	// base struct information.
	{
		struct Token const *Vis = PeekToken(Ps);
		if (Vis && Vis->Type == TT_ASTERISK)
		{
			Struct.Flags |= ANF_PUBLIC;
			++Ps->i;
		}
		
		struct Token const *Name = ExpectToken(Ps, TT_IDENT);
		if (!Name)
			return 1;
		
		if (!ExpectToken(Ps, TT_NEWLINE))
			return 1;
		
		AstNode_AddToken(&Struct, Name);
	}
	
	// get struct member information.
	for (;;)
	{
		SkipParseNewlines(Ps);
		struct Token const *MembName = ExpectToken(Ps, TT_IDENT);
		if (!MembName)
		{
			AstNode_Destroy(&Struct);
			return 1;
		}
		
		struct AstNode MembType = {0};
		unsigned char Term[] = {TT_NEWLINE};
		if (ParseWrappedType(&MembType, Ps, Term, 1))
		{
			AstNode_Destroy(&Struct);
			return 1;
		}
		
		struct AstNode Memb =
		{
			.Type = ANT_MEMBER
		};
		AstNode_AddChild(&Memb, &MembType);
		AstNode_AddToken(&Memb, MembName);
		
		AstNode_AddChild(&Struct, &Memb);
		
		struct Token const *Next = RequireToken(Ps);
		if (!Next)
		{
			AstNode_Destroy(&Struct);
			return 1;
		}
		
		if (Next->Type == TT_KW_END)
			break;
		
		--Ps->i;
	}
	
	*Out = Struct;
	
	return 0;
}

static int
ParseSwitch(struct AstNode *Out, struct ParseState *Ps)
{
	struct Token const *FirstTok = ExpectToken(Ps, TT_KW_SWITCH);
	if (!FirstTok)
		return 1;
	
	struct AstNode Switch =
	{
		.Type = ANT_SWITCH
	};
	
	// get what is being switched over.
	{
		struct AstNode Over = {0};
		unsigned char Term[] = {TT_NEWLINE};
		if (ParseWrappedExpr(&Over, Ps, Term, 1))
			return 1;
		
		AstNode_AddChild(&Switch, &Over);
	}
	
	// get case statements.
	{
		struct Token const *Which = RequireToken(Ps);
		if (!Which)
		{
			AstNode_Destroy(&Switch);
			return 1;
		}
		
		if (Which->Type != TT_KW_CASE && Which->Type != TT_KW_BASE)
		{
			LogTokErr(Ps->File, Which, "expected either TT_KW_CASE or TT_KW_BASE!");
			AstNode_Destroy(&Switch);
			return 1;
		}
		
		while (Which->Type == TT_KW_CASE)
		{
			struct AstNode Case =
			{
				.Type = ANT_CASE
			};
			
			struct AstNode Matches = {0};
			unsigned char MatchTerm[] = {TT_NEWLINE};
			if (ParseWrappedExpr(&Matches, Ps, MatchTerm, 1))
			{
				AstNode_Destroy(&Switch);
				return 1;
			}
			
			AstNode_AddChild(&Case, &Matches);
			
			struct AstNode StmtList = {0};
			unsigned char StmtListTerm[] = {TT_KW_CASE, TT_KW_BASE};
			if (ParseStatementList(&StmtList, Ps, StmtListTerm, 2))
			{
				AstNode_Destroy(&Case);
				AstNode_Destroy(&Switch);
				return 1;
			}
			
			AstNode_AddChild(&Case, &StmtList);
			AstNode_AddToken(&Case, Which);
			
			AstNode_AddChild(&Switch, &Case);
			
			Which = &Ps->Lex->Toks[Ps->i];
		}
	}
	
	// get base statement.
	{
		struct AstNode StmtList = {0};
		unsigned char Term[] = {TT_KW_END};
		if (ParseStatementList(&StmtList, Ps, Term, 1))
		{
			AstNode_Destroy(&Switch);
			return 1;
		}
		
		AstNode_AddChild(&Switch, &StmtList);
	}
	
	AstNode_AddToken(&Switch, FirstTok);
	
	*Out = Switch;
	
	return 0;
}

static int
ParseType(
	struct AstNode *Out,
	struct ParseState *Ps,
	unsigned char const Term[],
	size_t TermCnt
)
{
	// get base type.
	struct AstNode Lhs = {0};
	{
		struct Token const *BaseType = RequireToken(Ps);
		if (!BaseType)
			return 1;
		
		switch (BaseType->Type)
		{
		case TT_IDENT:
		case TT_KW_UINT8:
		case TT_KW_UINT16:
		case TT_KW_UINT32:
		case TT_KW_UINT64:
		case TT_KW_USIZE:
		case TT_KW_INT8:
		case TT_KW_INT16:
		case TT_KW_INT32:
		case TT_KW_INT64:
		case TT_KW_ISIZE:
		case TT_KW_BOOL:
		case TT_KW_FLOAT32:
		case TT_KW_FLOAT64:
		case TT_KW_SELF:
		case TT_KW_NULL:
			Lhs.Type = ANT_TYPE_ATOM;
			AstNode_AddToken(&Lhs, BaseType);
			break;
		default:
			LogTokErr(Ps->File, BaseType, "expected type atom!");
			return 1;
		}
	}
	
	// process type modifiers.
	for (;;)
	{
		struct Token const *Mod = RequireToken(Ps);
		if (!Mod)
		{
			AstNode_Destroy(&Lhs);
			return 1;
		}
		
		for (size_t i = 0; i < TermCnt; ++i)
		{
			if (Term[i] == Mod->Type)
				goto Done;
		}
		
		switch (Mod->Type)
		{
		case TT_CARET:
		{
			struct AstNode NewLhs =
			{
				.Type = ANT_TYPE_PTR
			};
			
			AstNode_AddChild(&NewLhs, &Lhs);
			AstNode_AddToken(&NewLhs, Mod);
			Lhs = NewLhs;
			
			break;
		}
		case TT_BKBEGIN:
		{
			if (!ExpectToken(Ps, TT_BKEND))
			{
				AstNode_Destroy(&Lhs);
				return 1;
			}
			
			struct AstNode NewLhs =
			{
				.Type = ANT_TYPE_ARRAY
			};
			
			AstNode_AddChild(&NewLhs, &Lhs);
			AstNode_AddToken(&NewLhs, Mod);
			Lhs = NewLhs;
			
			break;
		}
		case TT_KW_BASE:
		{
			if (!ExpectToken(Ps, TT_BKBEGIN))
			{
				AstNode_Destroy(&Lhs);
				return 1;
			}
			
			struct AstNode Size = {0};
			unsigned char Term[] = {TT_BKEND};
			if (ParseWrappedExpr(&Size, Ps, Term, 1))
			{
				AstNode_Destroy(&Lhs);
				return 1;
			}
			
			struct AstNode NewLhs =
			{
				.Type = ANT_TYPE_BUFFER
			};
			
			AstNode_AddChild(&NewLhs, &Lhs);
			AstNode_AddChild(&NewLhs, &Size);
			AstNode_AddToken(&NewLhs, Mod);
			Lhs = NewLhs;
			
			break;
		}
		case TT_PBEGIN:
		{
			struct AstNode NewLhs =
			{
				.Type = ANT_TYPE_PROC
			};
			
			AstNode_AddChild(&NewLhs, &Lhs);
			AstNode_AddToken(&NewLhs, Mod);
			Lhs = NewLhs;
			
			for (;;)
			{
				struct Token const *Tok = RequireToken(Ps);
				if (!Tok)
				{
					AstNode_Destroy(&Lhs);
					return 1;
				}
				
				switch (Tok->Type)
				{
				case TT_TRIPLE_PERIOD:
					if (!ExpectToken(Ps, TT_PEND))
					{
						AstNode_Destroy(&Lhs);
						return 1;
					}
					Lhs.Flags |= ANF_VARIADIC;
					break;
				case TT_KW_BASE:
					if (!ExpectToken(Ps, TT_TRIPLE_PERIOD))
					{
						AstNode_Destroy(&Lhs);
						return 1;
					}
					if (!ExpectToken(Ps, TT_PEND))
					{
						AstNode_Destroy(&Lhs);
						return 1;
					}
					Lhs.Flags |= ANF_VARIADIC;
					Lhs.Flags |= ANF_BASE;
					break;
				case TT_PEND:
					break;
				default:
				{
					--Ps->i;
					struct AstNode Child = {0};
					unsigned char Term[] = {TT_COMMA, TT_PEND};
					if (ParseType(&Child, Ps, Term, 2))
					{
						AstNode_Destroy(&Lhs);
						return 1;
					}
					AstNode_AddChild(&Lhs, &Child);
					break;
				}
				}
				
				if (Ps->Lex->Toks[Ps->i].Type == TT_PEND)
					break;
			}
			
			break;
		}
		case TT_KW_MUT:
			if (Lhs.Flags & ANF_MUT)
			{
				LogTokErr(Ps->File, Mod, "mutability modifier cannot be applied on a type multiple times!");
				AstNode_Destroy(&Lhs);
				return 1;
			}
			Lhs.Flags |= ANF_MUT;
			break;
		default:
			LogTokErr(Ps->File, Mod, "expected type modifier or terminator!");
			AstNode_Destroy(&Lhs);
			return 1;
		}
	}
Done:
	*Out = Lhs;
	
	return 0;
}

static int
ParseTypeAlias(struct AstNode *Out, struct ParseState *Ps)
{
	if (!ExpectToken(Ps, TT_KW_TYPEALIAS))
		return 1;
	
	struct Token const *Vis = PeekToken(Ps);
	if (Vis && Vis->Type == TT_ASTERISK)
	{
		Out->Flags |= ANF_PUBLIC;
		++Ps->i;
	}
	
	struct Token const *Name = ExpectToken(Ps, TT_IDENT);
	if (!Name)
		return 1;
	
	if (!ExpectToken(Ps, TT_COLON_EQUAL))
		return 1;
	
	struct AstNode Child = {0};
	unsigned char Term[] = {TT_NEWLINE};
	if (ParseWrappedType(&Child, Ps, Term, 1))
		return 1;
	
	Out->Type = ANT_TYPE_ALIAS;
	AstNode_AddToken(Out, Name);
	AstNode_AddChild(Out, &Child);
	
	return 0;
}

static int
ParseTypeLiteral(struct AstNode *Out, struct ParseState *Ps)
{
	// assumes that current token is an expression Struct / Union.
	struct Token const *FirstTok = &Ps->Lex->Toks[Ps->i];
	if (FirstTok->Type != TT_KW_STRUCT && FirstTok->Type != TT_KW_UNION)
	{
		LogTokErr(Ps->File, FirstTok, "expected TT_KW_STRUCT or TT_KW_UNION in type literal!");
		return 1;
	}
	
	struct Token const *TypeName = ExpectToken(Ps, TT_IDENT);
	if (!TypeName)
		return 1;
	
	if (!ExpectToken(Ps, TT_NEWLINE))
		return 1;
	
	struct AstNode TypeLiteral =
	{
		.Type = FirstTok->Type == TT_KW_STRUCT ? ANT_EXPR_STRUCT : ANT_EXPR_UNION
	};
	
	// get member data.
	for (;;)
	{
		struct AstNode Memb =
		{
			.Type = ANT_EXPR_MEMB
		};
		
		struct Token const *MembName = ExpectToken(Ps, TT_IDENT);
		if (!MembName)
		{
			AstNode_Destroy(&TypeLiteral);
			return 1;
		}
		
		AstNode_AddToken(&Memb, MembName);
		
		struct Token const *Next = PeekToken(Ps);
		while (Next && Next->Type == TT_PERIOD)
		{
			++Ps->i;
			
			MembName = ExpectToken(Ps, TT_IDENT);
			if (!MembName)
			{
				AstNode_Destroy(&Memb);
				AstNode_Destroy(&TypeLiteral);
				return 1;
			}
			
			AstNode_AddToken(&Memb, MembName);
			
			Next = PeekToken(Ps);
		}
		
		if (!ExpectToken(Ps, TT_COLON_EQUAL))
		{
			AstNode_Destroy(&Memb);
			AstNode_Destroy(&TypeLiteral);
			return 1;
		}
		
		struct AstNode Value = {0};
		unsigned char Term[] = {TT_NEWLINE};
		if (ParseExpr(&Value, Ps, Term, 1, 0))
		{
			AstNode_Destroy(&Memb);
			AstNode_Destroy(&TypeLiteral);
			return 1;
		}
		++Ps->i;
		
		AstNode_AddChild(&Memb, &Value);
		AstNode_AddChild(&TypeLiteral, &Memb);
		
		Next = PeekToken(Ps);
		if (Next && Next->Type == TT_KW_END)
		{
			++Ps->i;
			break;
		}
	}
	
	AstNode_AddToken(&TypeLiteral, FirstTok);
	*Out = TypeLiteral;
	
	return 0;
}

static int
ParseUnion(struct AstNode *Out, struct ParseState *Ps)
{
	if (!ExpectToken(Ps, TT_KW_UNION))
		return 1;
	
	struct AstNode Union =
	{
		.Type = ANT_UNION
	};
	
	// base union information.
	{
		struct Token const *Vis = PeekToken(Ps);
		if (Vis && Vis->Type == TT_ASTERISK)
		{
			Union.Flags |= ANF_PUBLIC;
			++Ps->i;
		}
		
		struct Token const *Name = ExpectToken(Ps, TT_IDENT);
		if (!Name)
			return 1;
		
		if (!ExpectToken(Ps, TT_NEWLINE))
			return 1;
		
		AstNode_AddToken(&Union, Name);
	}
	
	// get union member information.
	for (;;)
	{
		SkipParseNewlines(Ps);
		struct Token const *MembName = ExpectToken(Ps, TT_IDENT);
		if (!MembName)
		{
			AstNode_Destroy(&Union);
			return 1;
		}
		
		struct AstNode MembType = {0};
		unsigned char Term[] = {TT_NEWLINE};
		if (ParseWrappedType(&MembType, Ps, Term, 1))
		{
			AstNode_Destroy(&Union);
			return 1;
		}
		
		struct AstNode Memb =
		{
			.Type = ANT_MEMBER
		};
		AstNode_AddChild(&Memb, &MembType);
		AstNode_AddToken(&Memb, MembName);
		
		AstNode_AddChild(&Union, &Memb);
		
		struct Token const *Next = RequireToken(Ps);
		if (!Next)
		{
			AstNode_Destroy(&Union);
			return 1;
		}
		
		if (Next->Type == TT_KW_END)
			break;
		
		--Ps->i;
	}
	
	*Out = Union;
	
	return 0;
}

static int
ParseVar(
	struct AstNode *Out,
	struct ParseState *Ps,
	unsigned char const Term[],
	size_t TermCnt
)
{
	struct Token const *VarDecl = RequireToken(Ps);
	if (!VarDecl)
		return 1;
	
	struct AstNode Var =
	{
		.Type = ANT_VAR
	};
	
	// base var information.
	{
		switch (VarDecl->Type)
		{
		case TT_KW_EXTERNVAR:
			Var.Flags |= ANF_EXTERN;
			break;
		case TT_KW_VAR:
			break;
		default:
			LogTokErr(Ps->File, VarDecl, "expected either TT_KW_EXTERNVAR or TT_KW_VAR!");
			return 1;
		}
		
		struct Token const *Vis = PeekToken(Ps);
		if (Vis && Vis->Type == TT_ASTERISK)
		{
			Var.Flags |= ANF_PUBLIC;
			++Ps->i;
		}
		
		struct Token const *Name = ExpectToken(Ps, TT_IDENT);
		if (!Name)
			return 1;
		
		struct AstNode Type = {0};
		unsigned char *TypeTerm = malloc(TermCnt + 1);
		memcpy(TypeTerm, Term, TermCnt);
		TypeTerm[TermCnt] = TT_COLON_EQUAL;
		
		int Rc = ParseWrappedType(&Type, Ps, TypeTerm, TermCnt + 1);
		free(TypeTerm);
		if (Rc)
			return 1;
		
		AstNode_AddChild(&Var, &Type);
		AstNode_AddToken(&Var, Name);
	}
	
	// get initial value if present.
	{
		if (Ps->Lex->Toks[Ps->i].Type == TT_COLON_EQUAL)
		{
			struct AstNode Value = {0};
			if (ParseWrappedExpr(&Value, Ps, Term, TermCnt))
			{
				AstNode_Destroy(&Var);
				return 1;
			}
			
			AstNode_AddChild(&Var, &Value);
		}
	}
	
	*Out = Var;
	
	return 0;
}

static int
ParseWrappedExpr(
	struct AstNode *Out,
	struct ParseState *Ps,
	unsigned char const Term[],
	size_t TermCnt
)
{
	struct Token const *FirstTok = PeekToken(Ps);
	
	struct AstNode Child = {0};
	if (ParseExpr(&Child, Ps, Term, TermCnt, 0))
		return 1;
	++Ps->i;
	
	Out->Type = ANT_EXPR;
	AstNode_AddChild(Out, &Child);
	AstNode_AddToken(Out, FirstTok);
	
	return 0;
}

static int
ParseWrappedType(
	struct AstNode *Out,
	struct ParseState *Ps,
	unsigned char const Term[],
	size_t TermCnt
)
{
	struct Token const *FirstTok = PeekToken(Ps);
	
	struct AstNode Child = {0};
	if (ParseType(&Child, Ps, Term, TermCnt))
		return 1;
	
	Out->Type = ANT_TYPE;
	AstNode_AddChild(Out, &Child);
	AstNode_AddToken(Out, FirstTok);
	
	return 0;
}

static struct Token const *
PeekPrevToken(struct ParseState const *Ps)
{
	return Ps->i > 0 && Ps-> i - 1 < Ps->Lex->TokCnt ? &Ps->Lex->Toks[Ps->i - 1] : NULL;
}

static struct Token const *
PeekToken(struct ParseState const *Ps)
{
	return Ps->i + 1 >= Ps->Lex->TokCnt ? NULL : &Ps->Lex->Toks[Ps->i + 1];
}

static void
PrintTimeData(void)
{
	if (TimeData.ConfReadEnd)
	{
		fprintf(
			stderr,
			"conf read: %lums\n",
			TimeData.ConfReadEnd - TimeData.ConfReadBegin
		);
	}
	
	if (TimeData.FileReadEnd)
	{
		fprintf(
			stderr,
			"file read: %lums\n",
			TimeData.FileReadEnd - TimeData.FileReadBegin
		);
	}
	
	if (TimeData.LexEnd)
	{
		fprintf(
			stderr,
			"lex: %lums\n",
			TimeData.LexEnd - TimeData.LexBegin
		);
	}
	
	if (TimeData.ParseEnd)
	{
		fprintf(
			stderr,
			"parse: %lums\n",
			TimeData.ParseEnd - TimeData.ParseBegin
		);
	}
	
	if (TimeData.ExtractImportsEnd)
	{
		fprintf(
			stderr,
			"extract imports: %lums\n",
			TimeData.ExtractImportsEnd - TimeData.ExtractImportsBegin
		);
	}
}

static char *
ResolveImport(struct AstNode const *Import)
{
	for (size_t i = 0; i < Conf.ModulePathCnt; ++i)
	{
		char *Path;
		size_t PathLen;
		DynStr_Init(&Path, &PathLen);
		
		DynStr_AppendStr(&Path, &PathLen, Conf.ModulePaths[i]);
		if (Path[PathLen - 1] != '/')
			DynStr_AppendChar(&Path, &PathLen, '/');
		for (size_t j = 0; j < Import->TokCnt; ++j)
		{
			DynStr_AppendStr(
				&Path,
				&PathLen,
				Import->Toks[j]->Data.Str.Text
			);
			if (j + 1 < Import->TokCnt)
				DynStr_AppendChar(&Path, &PathLen, '/');
		}
		DynStr_AppendStr(&Path, &PathLen, ".lc");
		
		FILE *Fp = OpenFile(Path, "rb");
		if (Fp)
		{
			fclose(Fp);
			return Path;
		}
		
		free(Path);
	}
	
	return NULL;
}

static struct Token const *
RequireToken(struct ParseState *Ps)
{
	struct Token const *Tok = NextToken(Ps);
	if (!Tok)
	{
		LogProgErr(Ps->File, Ps->File->Len, 0, "required token at end of file, found nothing!");
		return NULL;
	}
	
	return Tok;
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

static void
SkipParseNewlines(struct ParseState *Ps)
{
	for (;;)
	{
		struct Token const *Peek = PeekToken(Ps);
		if (!Peek || Peek->Type != TT_NEWLINE)
			break;
		++Ps->i;
	}
}

static int
StrNumCmp(char const *a, size_t LenA, char const *b, size_t LenB)
{
	if (LenA > LenB)
		return 1;
	else if (LenA < LenB)
		return -1;
	
	for (size_t i = 0; i < LenA; ++i)
	{
		if (HexDigitValue[(size_t)a[i]] > HexDigitValue[(size_t)b[i]])
			return 1;
		else if (HexDigitValue[(size_t)a[i]] < HexDigitValue[(size_t)b[i]])
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

static enum AstNodeType
TokenTypeToLed(enum TokenType Type)
{
	switch (Type)
	{
	case TT_DOUBLE_PLUS:
		return ANT_EXPR_POST_INC;
	case TT_DOUBLE_MINUS:
		return ANT_EXPR_POST_DEC;
	case TT_PBEGIN:
		return ANT_EXPR_CALL;
	case TT_AT:
		return ANT_EXPR_NTH;
	case TT_CARET:
		return ANT_EXPR_ADDR_OF;
	case TT_PERIOD:
		return ANT_EXPR_ACCESS;
	case TT_KW_AS:
		return ANT_EXPR_CAST;
	case TT_PERIOD_CARET:
		return ANT_EXPR_DEREF;
	case TT_ASTERISK:
		return ANT_EXPR_MUL;
	case TT_SLASH:
		return ANT_EXPR_DIV;
	case TT_PERCENT:
		return ANT_EXPR_MOD;
	case TT_PLUS:
		return ANT_EXPR_ADD;
	case TT_MINUS:
		return ANT_EXPR_SUB;
	case TT_DOUBLE_GREATER:
		return ANT_EXPR_SHR;
	case TT_DOUBLE_LESS:
		return ANT_EXPR_SHL;
	case TT_AMPERSAND:
		return ANT_EXPR_BIT_AND;
	case TT_TILDE:
		return ANT_EXPR_BIT_XOR;
	case TT_PIPE:
		return ANT_EXPR_BIT_OR;
	case TT_GREATER:
		return ANT_EXPR_GREATER;
	case TT_GREQUAL:
		return ANT_EXPR_GREQUAL;
	case TT_LESS:
		return ANT_EXPR_LESS;
	case TT_LEQUAL:
		return ANT_EXPR_LEQUAL;
	case TT_DOUBLE_EQUAL:
		return ANT_EXPR_EQUAL;
	case TT_BANG_EQUAL:
		return ANT_EXPR_NEQUAL;
	case TT_DOUBLE_AMPERSAND:
		return ANT_EXPR_LOG_AND;
	case TT_DOUBLE_TILDE:
		return ANT_EXPR_LOG_XOR;
	case TT_DOUBLE_PIPE:
		return ANT_EXPR_LOG_OR;
	case TT_QUESTION:
		return ANT_EXPR_TERNARY;
	case TT_COLON_EQUAL:
		return ANT_EXPR_ASSIGN;
	case TT_PLUS_EQUAL:
		return ANT_EXPR_ADD_ASSIGN;
	case TT_MINUS_EQUAL:
		return ANT_EXPR_SUB_ASSIGN;
	case TT_ASTERISK_EQUAL:
		return ANT_EXPR_MUL_ASSIGN;
	case TT_SLASH_EQUAL:
		return ANT_EXPR_DIV_ASSIGN;
	case TT_PERCENT_EQUAL:
		return ANT_EXPR_MOD_ASSIGN;
	case TT_DOUBLE_GREATER_EQUAL:
		return ANT_EXPR_SHR_ASSIGN;
	case TT_DOUBLE_LESS_EQUAL:
		return ANT_EXPR_SHL_ASSIGN;
	case TT_AMPERSAND_EQUAL:
		return ANT_EXPR_BIT_AND_ASSIGN;
	case TT_TILDE_EQUAL:
		return ANT_EXPR_BIT_XOR_ASSIGN;
	case TT_PIPE_EQUAL:
		return ANT_EXPR_BIT_OR_ASSIGN;
	default:
		return -1;
	}
}

static enum AstNodeType
TokenTypeToNud(enum TokenType Type)
{
	switch (Type)
	{
	case TT_KW_PROC:
		return ANT_EXPR_LAMBDA;
	case TT_KW_STRUCT:
		return ANT_EXPR_STRUCT;
	case TT_KW_UNION:
		return ANT_EXPR_UNION;
	case TT_KW_SIZEOF:
		return ANT_EXPR_SIZEOF;
	case TT_KW_LENOF:
		return ANT_EXPR_LENOF;
	case TT_KW_NEXTVARG:
		return ANT_EXPR_NEXTVARG;
	case TT_KW_NULL:
		return ANT_EXPR_NULL;
	case TT_DOUBLE_PLUS:
		return ANT_EXPR_PRE_INC;
	case TT_DOUBLE_MINUS:
		return ANT_EXPR_PRE_DEC;
	case TT_MINUS:
		return ANT_EXPR_UNARY_MINUS;
	case TT_BANG:
		return ANT_EXPR_LOG_NOT;
	case TT_TILDE:
		return ANT_EXPR_BIT_NOT;
	default:
		return -1;
	}
}

static void
Usage(char const *Name)
{
	printf(
		"lithic - programming language transpiler\n"
		"\n"
		"for more information, consult the manual at the\n"
		"following link: https://tirimid.net/tirimid/lithic.html\n"
		"\n"
		"usage:\n"
		"\t%s [options] file\n"
		"\n"
		"options:\n"
		"\t--ast                  dump the parsed out AST\n"
		"\t--conf flag, -c flag   specify a language / transpiler flag\n"
		"\t--help, -h             display this help text\n"
		"\t--lex                  dump the lexed tokens\n"
		"\t--modpath dir, -m dir  add a module search directory\n"
		"\t--out file, -o file    write output to the specified file\n"
		"\t--time                 display time taken per transpile stage\n",
		Name
	);
}
