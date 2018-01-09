// pl0 compiler source code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "set.h"
#include "pl0.h"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n) {
	int i;

	printf("      ");
	// for (i = 1; i <= cc - 1; i++)
	for (i = 1; i < cc + 6; i++)
		printf(" ");
	printf("^\n");
	printf("[Error] %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
//从源代码读入一行到缓冲line,每次从line中读取一个字符
//ll指line中的总字符数,cc指当前已读的字符数
void getch(void){
	if (cc == ll) { //缓冲line消耗完,需再读入一行（该行处理结束）
		int ii;
		// printf("\n\t");
		// for (ii = 0; ii < ll; ii++) {
		// 	printf("%c(%d) ", line[ii], line[ii]);
		// }
		// printf("\n");

		if (feof(infile)){
			printf("\n[Error] PROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("[code]%5d  ", cx);
		while (!feof(infile) && (ch = getc(infile))!='\n')	{//读入一行
			line[++ll] = ch; // line 是输入缓冲  第一个字符空余？
			// printf("%c(%d, %d)", ch, cc, ll);
			printf("%c", ch);
		} // while
		printf("\n");
		line[++ll] = ' ';
    }
	// printf("cc = %d\n", cc);
	ch = line[++cc]; // 获取行读取缓冲中的下一个字符
} // getch

//////////////////////////////////////////////////////////////////////
// 从输入读入一个词法符号，词法分析器
void getsym(void) {
	int ii, anno;
	char temp;

	int i, k, state;
	char a[MAXIDLEN + 1];//a[11] 单词（临时存放缓冲）

	// getsym()第一次运行时，ch的值是被声明但未定义状态下的值Null，ascii码为0
	while (ch == ' ')//忽略空白，获取当前行读取缓冲中第一个非空格的字符
		getch();

	state = 1;
	k = num = anno = 0;
	// sym = SYM_NULL;
	while (state != 0) {
		switch(state) {
		case 1: // 初始状态？刚读取了第一个字符
			if (isalpha(ch)) { // 以字母开头，可能是关键字或标识符
				a[k++] = ch;
				getch();
				state = 2;
			} else if (isdigit(ch)) { // 以数字开头，应该是数
				sym = SYM_NUMBER; // 确定是数
				num = num * 10 + ch - '0';
				if (++k > MAXNUMLEN) {
					error(25); // （打印还是记录？）出错信息
					state = 0;
				} else {
					getch();
					state = 3; // 这个跳转是原地跳转
				}
			} else if (ch == ':') {
				getch();
				state = 4;
			} else if (ch == '>') {
				getch();
				state = 5;
			} else if (ch == '<') {
				getch();
				state = 6;
			} else {
				state = 7;
			}
			break;
		case 2:
			if (isalpha(ch) || isdigit(ch)) { // 组成部分 字母或数字
				a[k++] = ch;
				getch();
				state = 2; // 原地跳转
			} else { // 字段捕获结束，现在确定类型
				a[k] = 0; // 添加字符串结束符
				strcpy(id, a);
				word[0] = id;
				i = NRW;
				while (strcmp(id, word[i--])) ;
				if (++i) { // 关键字
					sym = wsym[i];
				} else { // 标识符
					sym = SYM_IDENTIFIER;
				}
				state = 0;
			}
			break;
		case 3:
			if (isdigit(ch)) {
				num = num * 10 + ch - '0';
				if (++k > MAXNUMLEN) {
					error(25); // （打印还是记录？）出错信息
					getch();
					// state = 0;
					// printf("状态为0？？");
				} else {
					getch();
					state = 3; // 原地跳转
				}
			} else {
				state = 0;
			}
			break;
		case 4: // ':'
			if (ch == '=') {
				sym = SYM_BECOMES;
				getch();
			} else {
				sym = SYM_NULL; // illegal?
			}
			state = 0;
			break;
		case 5: // '>=' or '>'
			if (ch == '=') {
				sym = SYM_GEQ;
			} else {
				sym = SYM_GTR;
			}
			getch();
			state = 0;
			break;
		case 6: // '<=' or '<' or '<>'
			if (ch == '=') {
				sym = SYM_LEQ;
			} else if (ch == '>') {
				sym = SYM_NEQ;
			} else {
				sym = SYM_LES;
			}
			getch();
			state = 0;
			break;
		case 7: // other tokens
			if (anno) { // 注释块内
				if (ch == '*') {
					getch();
					if (ch == ')') { // 连续的 '*' ')'
						anno = 0; // 注释结束
						getch();
						getsym(); // 获取注释块后面的第一个token
						state = 0;
					} else {
						state = 7; // 继续原地跳转
					}
				} else {
					getch();
					state = 7;
				}
			} else { // 注释块外
				i = NSYM;
				csym[0] = ch;
				while (csym[i--] != ch) ;
				if (++i) {
					sym = ssym[i];
					getch();
				} else {
					printf("Fatal Error: Unknown character.\n");
					exit(1);
				}
				if (sym == SYM_LPAREN) { // '('
					// getch();
					if (ch == '*') { // 连续的 '(' '*'
						sym = SYM_NULL;
						anno = 1;
						getch();
						state = 7; // 原地跳转
					} else {
						getch();
						state = 0;
					}
				} else {
					state = 0;
				}
			}
			break;
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z){//生成汇编指令
	if (cx > CXMAX)	{// cx > 500
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n){//参考第一部分2.6节
	symset s;

	if (! inset(sym, s1)) {
		error(n);
		// printf("sym = %d\n", sym);
		// printset(s1);
		s = uniteset(s1, s2);
		while(! inset(sym, s)) // 跳过所有非关键字的token
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind){//标识符填入符号表
	mask* mk;

	tx++; // table符号表指针？
	strcpy(table[tx].name, id); // 常量名/变量名/过程名
	table[tx].kind = kind; // 当前符号的类型（常量/变量/过程）
	switch (kind)	{
	case ID_CONSTANT: // 常量
		if (num > MAXADDRESS) {//常数的大小不能超过32767
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num; // 置数常量对应数值
		break;
	case ID_VARIABLE: // 变量
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE: // 过程定义
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id){//在符号表table中查找id（变量名）
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0); // 符号表指针前移，判断，如不相等则继续
	return i; // 返回0，说明没有找到
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration(){//常量填入符号表
	if (sym == SYM_IDENTIFIER)	{//全局变量id中存有已识别的标识符
		getsym(); // 上一次执行的时候获取了常量符号，保存在id。这次必须获取字符'='
		if (sym == SYM_EQU || sym == SYM_BECOMES){ // EQU = || BECOMES :=
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym(); // 获取 '=' or ':=' 后面的数字（数值），保存在num
			if (sym == SYM_NUMBER) {//全局变量num中存有已识别的数字
				enter(ID_CONSTANT); // 将当前字段（已识别为常量部分）加入符号表
				getsym(); // 继续获取下一个字符，可能是','或';'
			}else{
				error(2); // There must be a number to follow '='.
			}
		}else{
			error(3); // There must be an '=' to follow the identifier.
		}
    } else {
        error(4); // There must be an identifier to follow 'const'.
	}
} // constdeclaration

//////////////////////////////////////////////////////////////////////
void vardeclaration(void){ //变量填入符号表
	if (sym == SYM_IDENTIFIER) {
		enter(ID_VARIABLE);
		getsym(); // 捕获一个 ','（因为现在是声明阶段，没有赋值），可能捕获到分号
	}else{
		error(4); // There must be an identifier to follow 'var'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to){
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys){
	void expression();
	int i;
	symset set;
	
	test(facbegsys, fsys, 24); // The symbol cannot be as the beginning of an expression.

	while (inset(sym, facbegsys)){
		if (sym == SYM_IDENTIFIER)	{
			if ((i = position(id)) == 0){
				error(11); // Undeclared identifier.
			}else{
				switch (table[i].kind){
					mask* mk;
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask*) &table[i];
					gen(LOD, level - mk->level, mk->address);
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier cannot be in an expression.
					break;
				} // switch
			}
			getsym();
		}else if (sym == SYM_NUMBER)	{
			if (num > MAXADDRESS){
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}else if (sym == SYM_LPAREN)	{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN){
				getsym();
			}else{
				error(22); // Missing ')'.
			}
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // while
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys){
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH){
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
void expression(symset fsys){ //递归下降法中对<表达式>的处理
	int addop;
	symset set;

	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));
	if (sym == SYM_PLUS || sym == SYM_MINUS){
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_MINUS)	{
			gen(OPR, 0, OPR_NEG);
		}
	}else{
		term(set);
	}

	while (sym == SYM_PLUS || sym == SYM_MINUS){
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)	{
			gen(OPR, 0, OPR_ADD);
		}else{
			gen(OPR, 0, OPR_MIN);
		}
	} // while

	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys){ //递归下降法中对<条件>的处理
	int relop;
	symset set;

	if (sym == SYM_ODD)	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}else{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (! inset(sym, relset)){
			error(20);
		}else{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop){//根据比较关系生成指令
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition

//////////////////////////////////////////////////////////////////////
void statement(symset fsys){ //递归下降法中对<语句>的处理
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER){ // variable assignment 捕获到变量名
		mask* mk;
		if (! (i = position(id))){ // 没有找到变量名
			error(11); // Undeclared identifier.
		}else if (table[i].kind != ID_VARIABLE){
			error(12); // Illegal assignment. 该符号不是变量
			i = 0;
		}
		getsym();
		if (sym == SYM_BECOMES)	{
			getsym();
		}else{
			error(13); // ':=' expected.
		}
		expression(fsys);
		mk = (mask*) &table[i];
		if (i){
			gen(STO, level - mk->level, mk->address);
		}
	}
	else if (sym == SYM_CALL){ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER){
			error(14); // There must be an identifier to follow the 'call'.
		}else{
			if (! (i = position(id))){
				error(11); // Undeclared identifier.
			}else if (table[i].kind == ID_PROCEDURE) {
				mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address);
			}else{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} else if (sym == SYM_IF)	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN){
			getsym();
		}else{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;	
	}
	else if (sym == SYM_BEGIN)	{ // block
		getsym(); // 捕获begin后面的第一个字段
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);//该行代码和后续的while循环处理语句序列
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys)){
			if (sym == SYM_SEMICOLON){
				getsym();
			}else{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)	{ // begin - end 块结束
			getsym();
		}else{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE){ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)	{
			getsym();
		}else{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys){//递归下降法中对<程序体>的处理 【实验三】只需要修改此函数，意思是只需要改写“程序体”部分
	int cx0; // initial code index

    // 后续变量定义主要用于代码生成
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;
	dx = 3;
	mk = (mask*) &table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL){
		error(32); // There are too many levels. level是递归之外的全局变量
	}

	// 语法分析开始
	do{
		if (sym == SYM_CONST){ // constant declarations
			getsym(); // 获取const字段后面出现的第一个常量符号id1
			do{//循环处理id1=num1,id2=num2,……
				constdeclaration(); // 处理const字段之后的第一组常量定义id1=num1
				while (sym == SYM_COMMA){ // 上一组常量定义代码段（上一行中的函数内部逻辑）最后一次运行getsym()函数时捕获了一个','
					getsym(); // 捕获一个常量名 id
					constdeclaration();
				}// 循环结束条件是捕获到';'，也意味着本组声明结束
				if (sym == SYM_SEMICOLON){ // 获取到分号';'
					getsym(); // 继续获取';'后的一个字段
				} else {
					error(5); // Missing ',' or ';'.
				}
			}while (sym == SYM_IDENTIFIER); // 问题：分号后面捕获到一个标识符，仍然是常量定义，这是什么意思
		} // if

		if (sym == SYM_VAR)	{ // variable declarations
			getsym(); // 上一次执行已经捕获了一个 "var"，这次执行应该捕获变量名，存放在id中
			do{
				vardeclaration();
				while (sym == SYM_COMMA){
					getsym(); // 捕获下一个变量名。可能捕获到分号';'（但这属于出错情况），说明变量声明结束
					vardeclaration();
				} // id1, id2, id3;
				if (sym == SYM_SEMICOLON){ // 上次捕获到';'，变量声明结束
					getsym(); // 继续捕获下一个字段
				}else{
					error(5); // Missing ',' or ';'.
				}
			}while (sym == SYM_IDENTIFIER);
//			block = dx;
		} // if

		// if (sym == SYM_VAR) {
		// 	printf("yes, get another var.\n");
		// } else {
		// 	printf("no, it's not var. It is %d.\n", sym);
		// }

		while (sym == SYM_PROCEDURE){ // procedure declarations
			getsym(); // 捕获"procedure"之后的第一个字段，即过程名
			if (sym == SYM_IDENTIFIER){
				enter(ID_PROCEDURE);
				getsym(); // 捕获过程名后面的分号';'
			}else{
				error(4); // There must be an identifier to follow 'procedure'.
			}

			if (sym == SYM_SEMICOLON) { // 上次捕获的是分号';'
				getsym(); //继续捕获下一个字段。第一次捕获的是第4行的"var"
			}else{
				error(5); // Missing ',' or ';'.
			}
			level++;//函数可以嵌套,level是函数的嵌套级别
			savedTx = tx;
			block_dx = dx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);//嵌套（递归）调用函数定义中的<程序体>处理函数block
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;
            dx = block_dx;
			// 当前的procedure定义中的block部分结束，后面紧跟着的应该是分号
			if (sym == SYM_SEMICOLON) {
				getsym();
			
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6); // 检查捕获的token是不是 procedure 或 语句的预测符集
				destroyset(set1);
				destroyset(set);
			}else{
				error(5); // Missing ',' or ';'.
			}
		} // while
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7); // 这个有什么作用？在语法错误出现时，跳过错误部分，继续识别后续语法正确部分
		destroyset(set1);
		destroyset(set);
	}while (inset(sym, declbegsys)); // 刚刚捕获到的是'const' 'var' 'procedure' 'Null'中的一个

	// 后续部分主要用于代码生成
	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);

	statement(set); // 语句

	// if (sym == SYM_SEMICOLON) {
	// 	printf("yes, get ';'.");
	// 	getsym();
	// 	printf("next is %d\n", sym);
	// 	exit(0);
	// }

	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx); // 控制台显示生成代码？
	
	// printf("暂停\n");
	// exit(0);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff){
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret(){
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do	{
		i = code[pc++];
		switch (i.f){
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0) {
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main (){
	FILE* hbin;
	char s[80]; // 读取缓冲区
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL){
		printf("[Error] File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL); 
	
	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_NULL);

    /*
	err 词法（/语法？）错误数
    cx 正在处理的代码所在行数
    cc 当前读取字符数
    ll 好像是输入缓冲（读取一行）指针
    */
	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN; // kk=10

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

system("pause");

// 后续代码用于解释执行中间代码
/*
	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0) {
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
*/
} // main    END OF PL0.c
