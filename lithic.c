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
	TT_KW_BLOCK,
	TT_KW_BOOL,
	TT_KW_BREAK,
	TT_KW_CASE,
	TT_KW_CONTINUE,
	TT_KW_ELIF,
	TT_KW_ELSE,
	TT_KW_END,
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
	TT_KW_PROC,
	TT_KW_RESETVARGS,
	TT_KW_RETURN,
	TT_KW_SIZEOF,
	TT_KW_STRUCT,
	TT_KW_SWITCH,
	TT_KW_TRUE,
	TT_KW_UINT8,
	TT_KW_UINT16,
	TT_KW_UINT32,
	TT_KW_UINT64,
	TT_KW_USIZE,
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
	TT_PERIOD,
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
	TT_AT_QUOTE
};

struct Conf
{
	char const *InFile;
	FILE *InFp;
	
	char const *OutFile;
	FILE *OutFp;
	
	bool DumpToks;
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
		char *Str;
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

static void AddSpecialCharToken(struct LexData *Out, size_t Pos, size_t Len, enum TokenType Type);
static void AddToken(struct LexData *out, struct Token const *Tok);
static int Conf_Read(int Argc, char const *Argv[]);
static void Conf_Quit(void);
static int FileData_Read(struct FileData *Out, FILE *Fp, char const *File);
static bool IsIdentInit(char ch);
static int Lex(struct LexData *Out, struct FileData const *Data);
static int LexChar(struct FileData const *Data, struct Token *Out, size_t *i);
static int LexString(struct FileData const *Data, struct Token *Out, size_t *i);
static int LexNum(struct FileData const *Data, struct Token *Out, size_t *i);
static int LexWord(struct FileData const *Data, struct Token *Out, size_t *i);
static size_t LineNumber(char const *Str, size_t Pos);
static void LogErr(char const *Fmt, ...);
static void LogProgErr(struct FileData const *Data, size_t i, char const *Fmt, ...);
static unsigned SizeModBits(enum SizeMod Mod);
static int StrNumCmp(char const *a, size_t LenA, char const *b, size_t LenB);
static char *Substr(char const *Str, size_t Lb, size_t Ub);
static void Token_Print(FILE *Fp, struct Token const *Tok, size_t Ind);
static void Usage(char const *Name);
static int ValidateEscChar(char const *Src, size_t Pos);

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

static struct Conf Conf;

int
main(int Argc, char const *Argv[])
{
	if (Conf_Read(Argc, Argv))
		return 1;
	
	struct FileData FileData = {0};
	if (FileData_Read(&FileData, Conf.InFp, Conf.InFile))
		return 1;
	
	struct LexData LexData = {0};
	if (Lex(&LexData, &FileData))
		return 1;
	
	if (Conf.DumpToks)
	{
		for (size_t i = 0; i < LexData.TokCnt; ++i)
			Token_Print(Conf.OutFp, &LexData.Toks[i], i);
		return 0;
	}
	
	// TODO: implement rest.
	
	return 0;
}

static void
AddSpecialCharToken(struct LexData *Out,
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
	AddToken(Out, &Tok);
}

static void
AddToken(struct LexData *Out, struct Token const *Tok)
{
	++Out->TokCnt;
	Out->Toks = reallocarray(Out->Toks, Out->TokCnt, sizeof(struct Token));
	Out->Toks[Out->TokCnt - 1] = *Tok;
}

static int
Conf_Read(int Argc, char const *Argv[])
{
	atexit(Conf_Quit);
	
	// get option arguments.
	int c;
	while ((c = getopt(Argc, (char *const *)Argv, "hLo:")) != -1)
	{
		switch (c)
		{
		case 'h':
			Usage(Argv[0]);
			exit(0);
		case 'L':
			Conf.DumpToks = true;
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
				AddSpecialCharToken(Out, i, 1, TT_NEWLINE);
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
				AddToken(Out, &Tok);
				--i;
				continue;
			}
			else if (isdigit(Data->Data[i]))
			{
				struct Token Tok;
				if (LexNum(Data, &Tok, &i))
					return 1;
				AddToken(Out, &Tok);
				--i;
				continue;
			}
			else if (Data->Data[i] == '\'')
			{
				struct Token Tok;
				if (LexChar(Data, &Tok, &i))
					return 1;
				AddToken(Out, &Tok);
				--i;
				continue;
			}
			else if (Data->Data[i] == '"')
			{
				struct Token Tok;
				if (LexString(Data, &Tok, &i))
					return 1;
				AddToken(Out, &Tok);
				--i;
				continue;
			}
		}
		
		// handle special characters.
		{
			switch (Data->Data[i])
			{
			case '(':
				AddSpecialCharToken(Out, i, 1, TT_PBEGIN);
				break;
			case ')':
				AddSpecialCharToken(Out, i, 1, TT_PEND);
				break;
			case '[':
				AddSpecialCharToken(Out, i, 1, TT_BKBEGIN);
				break;
			case ']':
				AddSpecialCharToken(Out, i, 1, TT_BKEND);
				break;
			case '+':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "++", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_DOUBLE_PLUS);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_PLUS);
				break;
			case '-':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "--", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_DOUBLE_MINUS);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_MINUS);
				break;
			case '@':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "@'", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_AT_QUOTE);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_AT);
				break;
			case '^':
				AddSpecialCharToken(Out, i, 1, TT_CARET);
				break;
			case '.':
				AddSpecialCharToken(Out, i, 1, TT_PERIOD);
				break;
			case '!':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "!=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_BANG_EQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_BANG);
				break;
			case '~':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "~~", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_DOUBLE_TILDE);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "~=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_TILDE_EQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_TILDE);
				break;
			case '&':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "&&", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_DOUBLE_AMPERSAND);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "&=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_AMPERSAND_EQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_AMPERSAND);
				break;
			case '*':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "*=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_ASTERISK_EQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_ASTERISK);
				break;
			case '/':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "/=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_SLASH_EQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_SLASH);
				break;
			case '%':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "%=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_PERCENT_EQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_PERCENT);
				break;
			case '>':
				if (i + 2 < Data->Len && !strncmp(&Data->Data[i], ">>=", 3))
				{
					AddSpecialCharToken(Out, i, 3, TT_DOUBLE_GREATER_EQUAL);
					i += 2;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], ">>", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_DOUBLE_GREATER);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], ">=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_GREQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_GREATER);
				break;
			case '<':
				if (i + 2 < Data->Len && !strncmp(&Data->Data[i], "<<=", 3))
				{
					AddSpecialCharToken(Out, i, 3, TT_DOUBLE_LESS_EQUAL);
					i += 2;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "<<", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_DOUBLE_LESS);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "<=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_LEQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_LESS);
				break;
			case '|':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "||", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_DOUBLE_PIPE);
					++i;
				}
				else if (i + 1 < Data->Len && !strncmp(&Data->Data[i], "|=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_PIPE_EQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_PIPE);
				break;
			case '?':
				AddSpecialCharToken(Out, i, 1, TT_QUESTION);
				break;
			case ':':
				if (i + 1 < Data->Len && !strncmp(&Data->Data[i], ":=", 2))
				{
					AddSpecialCharToken(Out, i, 2, TT_COLON_EQUAL);
					++i;
				}
				else
					AddSpecialCharToken(Out, i, 1, TT_COLON);
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
	// TODO: implement.
	return 1;
}

static int
LexString(struct FileData const *Data, struct Token *Out, size_t *i)
{
	// TODO: implement.
	return 1;
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
	static char const *Keywords[] =
	{
		"As",
		"Block",
		"Bool",
		"Break",
		"Case",
		"Continue",
		"Elif",
		"Else",
		"End",
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
		"Proc",
		"ResetVargs",
		"Return",
		"SizeOf",
		"Struct",
		"Switch",
		"True",
		"Uint8",
		"Uint16",
		"Uint32",
		"Uint64",
		"Usize",
		"VargCount",
		"While"
	};
	
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
	
	*Out = (struct Token)
	{
		.Data.Str = Word,
		.Pos = Lb,
		.Len = *i - Lb,
		.Type = Type
	};
	
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
			fprintf(stderr, "%c", Data->Data[j]);
		
		fprintf(stderr, "\n    ");
		
		for (size_t j = Begin; j < i; ++j)
			fprintf(stderr, " ");
		
		fprintf(stderr, "\x1b[31m^\x1b[0m\n");
	}
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
Token_Print(FILE *Fp, struct Token const *Tok, size_t Ind)
{
	static char const *Names[] =
	{
		"TT_IDENT",
		
		// literals.
		"TT_LIT_STR",
		"TT_LIT_INT",
		"TT_LIT_FLOAT",
		"TT_LIT_BOOL",
		
		// keywords.
		"TT_KW_AS",
		"TT_KW_BLOCK",
		"TT_KW_BOOL",
		"TT_KW_BREAK",
		"TT_KW_CASE",
		"TT_KW_CONTINUE",
		"TT_KW_ELIF",
		"TT_KW_ELSE",
		"TT_KW_END",
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
		"TT_KW_PROC",
		"TT_KW_RESETVARGS",
		"TT_KW_RETURN",
		"TT_KW_SIZEOF",
		"TT_KW_STRUCT",
		"TT_KW_SWITCH",
		"TT_KW_TRUE",
		"TT_KW_UINT8",
		"TT_KW_UINT16",
		"TT_KW_UINT32",
		"TT_KW_UINT64",
		"TT_KW_USIZE",
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
		"TT_PERIOD",
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
		"TT_AT_QUOTE"
	};
	
	fprintf(Fp, "[%zu] [%zu+%zu]:%s", Ind, Tok->Pos, Tok->Len, Names[Tok->Type]);
	switch (Tok->Type)
	{
	case TT_IDENT:
		fprintf(Fp, " %s", Tok->Data.Str);
		break;
	case TT_LIT_STR:
		fprintf(Fp, " S%d:%s", SizeModBits(Tok->SizeMod), Tok->Data.Str);
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
	       "\t-h       display this help text\n"
	       "\t-o file  write output to the specified file\n"
	       "\t-L       dump the lexed tokens\n",
	       Name);
}

static int
ValidateEscChar(char const *Src, size_t Pos)
{
	// TODO: implement.
	return 1;
}
