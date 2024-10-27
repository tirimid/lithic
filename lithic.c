#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <getopt.h>
#include <unistd.h>

enum TokenType
{
	TT_IDENT = 0,
	
	// literals.
	TT_LIT_STR,
	TT_LIT_INT,
	TT_LIT_BOOL,
	
	// keywords.
	TT_KW_BOOL,
	TT_KW_END,
	TT_KW_FALSE,
	TT_KW_IF,
	TT_KW_IMPORT,
	TT_KW_INT8,
	TT_KW_INT16,
	TT_KW_INT32,
	TT_KW_INT64,
	TT_KW_MUT,
	TT_KW_NULL,
	TT_KW_PROC,
	TT_KW_RETURN,
	TT_KW_TRUE,
	TT_KW_UINT8,
	TT_KW_UINT16,
	TT_KW_UINT32,
	TT_KW_UINT64,
	TT_KW_WHILE,
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
	char *Data;
	size_t Len;
};

struct Token
{
	union
	{
		char *Str;
		uint64_t Int;
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

static int Conf_Read(int Argc, char const *Argv[]);
static void Conf_Quit(void);
static int FileData_Read(void);
static int Lex(void);
static void LogErr(char const *Fmt, ...);
static void Usage(char const *Name);

static struct Conf Conf;
static struct FileData FileData;
static struct LexData LexData;

int
main(int Argc, char const *Argv[])
{
	if (Conf_Read(Argc, Argv))
		return 1;
	
	if (FileData_Read())
		return 1;
	
	if (Lex())
		return 1;
	
	return 0;
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
FileData_Read(void)
{
	// read input file.
	{
		fseek(Conf.InFp, 0, SEEK_END);
		
		long Len = ftell(Conf.InFp);
		if (Len == -1)
		{
			LogErr("failed to get size of input file - '%s'!", Conf.InFile);
			return 1;
		}
		fseek(Conf.InFp, 0, SEEK_SET);
		
		FileData.Len = Len;
		FileData.Data = malloc(Len + 1);
		if (fread(FileData.Data, sizeof(char), Len, Conf.InFp) != Len)
		{
			LogErr("failed to read input file - '%s'!", Conf.InFile);
			return 1;
		}
		FileData.Data[Len] = 0;
	}
	
	return 0;
}

static int
Lex(void)
{
	// TODO: implement.
	return 1;
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
Usage(char const *Name)
{
	printf("LithiC - Programming Language Transpiler\n"
	       "\n"
	       "For more information, consult the manual at the\n"
	       "following link: https://tirimid.net/tirimid/lithic.html\n"
	       "\n"
	       "usage:\n"
	       "\t%s [options] file\n"
	       "options:\n"
	       "\t-h       display this help text\n"
	       "\t-o file  write output to the specified file\n"
	       "\t-L       dump the lexed tokens\n",
	       Name);
}
