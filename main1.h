#include <stdio.h>

#define NRW        11     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       10     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   10     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      500    // size of code array

#define MAXSYM     30     // maximum number of symbols  

#define STACKSIZE  1000   // maximum storage

enum symtype {
	/* 0 */		SYM_NULL,	
	/* 1 */		SYM_BLOCK,	
	/* 2 */		SYM_A,		
	/* 3 */		SYM_A1,	
	/* 4 */		SYM_B,	
	/* 5 */		SYM_B1,	
	/* 6 */		SYM_C,	
	/* 7 */		SYM_IDENTIFIER,	
	/* 8 */		SYM_NUMBER,	
	/* 9 */		SYM_PLUS,	
	/* 10 */	SYM_MINUS,	
	/* 11 */	SYM_TIMES,	
	/* 12 */	SYM_SLASH,	
	/* 13 */	SYM_ODD,	
	/* 14 */	SYM_EQU,	
	/* 15 */	SYM_NEQ,	
	/* 16 */	SYM_LES,	
	/* 17 */	SYM_LEQ,	
	/* 18 */	SYM_GTR,	
	/* 19 */	SYM_GEQ,	
	/* 20 */	SYM_LPAREN,	
	/* 21 */	SYM_RPAREN,	
	/* 22 */	SYM_COMMA,	
	/* 23 */	SYM_SEMICOLON,	
	/* 24 */	SYM_PERIOD,	
	/* 25 */	SYM_BECOMES,    
	/* 26 */	SYM_BEGIN,	
	/* 27 */	SYM_END,	
	/* 28 */	SYM_IF,	
	/* 29 */	SYM_THEN,	
	/* 30 */	SYM_WHILE,	
	/* 31 */	SYM_DO,	
	/* 32 */	SYM_CALL,	
	/* 33 */	SYM_CONST,	
	/* 34 */	SYM_VAR,	
	/* 35 */	SYM_PROCEDURE
};
enum idtype {
	ID_CONSTANT, ID_VARIABLE, ID_PROCEDURE
};
enum opcode {
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC
};
enum oprcode {
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,	OPR_GEQ
};
// // 新增非终结符集
// enum left_symtype {
// 	SYM_BLOCK, SYM_A, SYM_B
// };
typedef struct {
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;
//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "",
/* 27 */    "",
/* 28 */    "",
/* 29 */    "",
/* 30 */    "",
/* 31 */    "",
/* 32 */    "There are too many levels."
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated. 正在处理的行数
int  level = 0;
int  tx = 0;	 // table符号表指针

char line[80];   //输入缓冲

instruction code[CXMAX]; // 生成的汇编代码

char* word[NRW + 1] = { //关键字列表,第一个占位符用于getsym函数中的关键字比较
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while"
};

int wsym[NRW + 1] = {
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE
};

int ssym[NSYM + 1] = {
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON
};

char csym[NSYM + 1] = {
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';'
};

// 产生式右部
int block_const_right[3] = {
	SYM_A, SYM_B, SYM_C
};
int block_var_right[4] = {
	SYM_VAR, SYM_IDENTIFIER, SYM_B, SYM_SEMICOLON
};
int block_procedure_right[5] = {
	SYM_PROCEDURE, SYM_IDENTIFIER, SYM_SEMICOLON, SYM_BLOCK, SYM_SEMICOLON
};
int A_comma_right[5] = {
	SYM_COMMA, SYM_IDENTIFIER, SYM_EQU, SYM_NUMBER, SYM_A
};
int A_e_right[1] = {
	SYM_NULL
};
int B_comma_right[3] = {
	SYM_COMMA, SYM_IDENTIFIER, SYM_B
};
int B_e_right[1] = {
	SYM_NULL
};
struct Production {
	int right[10];
	int length;
}ll1_table[10][40], elem;
int block_status;
// ll1文法表
// struct Production ll1_table[3][5];

#define MAXINS   8

char* mnemonic[MAXINS] = {
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC"
};

typedef struct {
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
} comtab;

comtab table[TXMAX];//符号表table[500]


typedef struct {
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address; // 代码所在行数？
} mask;

FILE* infile;


// 栈
typedef struct {
	int data[50];
	int p;
} Stack;
Stack sym_stack;
