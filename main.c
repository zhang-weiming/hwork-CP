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
//��Դ�������һ�е�����line,ÿ�δ�line�ж�ȡһ���ַ�
//llָline�е����ַ���,ccָ��ǰ�Ѷ����ַ���
void getch(void){
	if (cc == ll) { //����line������,���ٶ���һ�У����д��������
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
		while (!feof(infile) && (ch = getc(infile))!='\n')	{//����һ��
			line[++ll] = ch; // line �����뻺��  ��һ���ַ����ࣿ
			// printf("%c(%d, %d)", ch, cc, ll);
			printf("%c", ch);
		} // while
		printf("\n");
		line[++ll] = ' ';
    }
	// printf("cc = %d\n", cc);
	ch = line[++cc]; // ��ȡ�ж�ȡ�����е���һ���ַ�
} // getch

//////////////////////////////////////////////////////////////////////
// ���������һ���ʷ����ţ��ʷ�������
void getsym(void) {
	int ii, anno;
	char temp;

	int i, k, state;
	char a[MAXIDLEN + 1];//a[11] ���ʣ���ʱ��Ż��壩

	// getsym()��һ������ʱ��ch��ֵ�Ǳ�������δ����״̬�µ�ֵNull��ascii��Ϊ0
	while (ch == ' ')//���Կհף���ȡ��ǰ�ж�ȡ�����е�һ���ǿո���ַ�
		getch();

	state = 1;
	k = num = anno = 0;
	// sym = SYM_NULL;
	while (state != 0) {
		switch(state) {
		case 1: // ��ʼ״̬���ն�ȡ�˵�һ���ַ�
			if (isalpha(ch)) { // ����ĸ��ͷ�������ǹؼ��ֻ��ʶ��
				a[k++] = ch;
				getch();
				state = 2;
			} else if (isdigit(ch)) { // �����ֿ�ͷ��Ӧ������
				sym = SYM_NUMBER; // ȷ������
				num = num * 10 + ch - '0';
				if (++k > MAXNUMLEN) {
					error(25); // ����ӡ���Ǽ�¼����������Ϣ
					state = 0;
				} else {
					getch();
					state = 3; // �����ת��ԭ����ת
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
			if (isalpha(ch) || isdigit(ch)) { // ��ɲ��� ��ĸ������
				a[k++] = ch;
				getch();
				state = 2; // ԭ����ת
			} else { // �ֶβ������������ȷ������
				a[k] = 0; // ����ַ���������
				strcpy(id, a);
				word[0] = id;
				i = NRW;
				while (strcmp(id, word[i--])) ;
				if (++i) { // �ؼ���
					sym = wsym[i];
				} else { // ��ʶ��
					sym = SYM_IDENTIFIER;
				}
				state = 0;
			}
			break;
		case 3:
			if (isdigit(ch)) {
				num = num * 10 + ch - '0';
				if (++k > MAXNUMLEN) {
					error(25); // ����ӡ���Ǽ�¼����������Ϣ
					getch();
					// state = 0;
					// printf("״̬Ϊ0����");
				} else {
					getch();
					state = 3; // ԭ����ת
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
			if (anno) { // ע�Ϳ���
				if (ch == '*') {
					getch();
					if (ch == ')') { // ������ '*' ')'
						anno = 0; // ע�ͽ���
						getch();
						getsym(); // ��ȡע�Ϳ����ĵ�һ��token
						state = 0;
					} else {
						state = 7; // ����ԭ����ת
					}
				} else {
					getch();
					state = 7;
				}
			} else { // ע�Ϳ���
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
					if (ch == '*') { // ������ '(' '*'
						sym = SYM_NULL;
						anno = 1;
						getch();
						state = 7; // ԭ����ת
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
void gen(int x, int y, int z){//���ɻ��ָ��
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
void test(symset s1, symset s2, int n){//�ο���һ����2.6��
	symset s;

	if (! inset(sym, s1)) {
		error(n);
		// printf("sym = %d\n", sym);
		// printset(s1);
		s = uniteset(s1, s2);
		while(! inset(sym, s)) // �������зǹؼ��ֵ�token
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind){//��ʶ��������ű�
	mask* mk;

	tx++; // table���ű�ָ�룿
	strcpy(table[tx].name, id); // ������/������/������
	table[tx].kind = kind; // ��ǰ���ŵ����ͣ�����/����/���̣�
	switch (kind)	{
	case ID_CONSTANT: // ����
		if (num > MAXADDRESS) {//�����Ĵ�С���ܳ���32767
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num; // ����������Ӧ��ֵ
		break;
	case ID_VARIABLE: // ����
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE: // ���̶���
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id){//�ڷ��ű�table�в���id����������
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0); // ���ű�ָ��ǰ�ƣ��жϣ��粻��������
	return i; // ����0��˵��û���ҵ�
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration(){//����������ű�
	if (sym == SYM_IDENTIFIER)	{//ȫ�ֱ���id�д�����ʶ��ı�ʶ��
		getsym(); // ��һ��ִ�е�ʱ���ȡ�˳������ţ�������id����α����ȡ�ַ�'='
		if (sym == SYM_EQU || sym == SYM_BECOMES){ // EQU = || BECOMES :=
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym(); // ��ȡ '=' or ':=' ��������֣���ֵ����������num
			if (sym == SYM_NUMBER) {//ȫ�ֱ���num�д�����ʶ�������
				enter(ID_CONSTANT); // ����ǰ�ֶΣ���ʶ��Ϊ�������֣�������ű�
				getsym(); // ������ȡ��һ���ַ���������','��';'
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
void vardeclaration(void){ //����������ű�
	if (sym == SYM_IDENTIFIER) {
		enter(ID_VARIABLE);
		getsym(); // ����һ�� ','����Ϊ�����������׶Σ�û�и�ֵ�������ܲ��񵽷ֺ�
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
void expression(symset fsys){ //�ݹ��½����ж�<���ʽ>�Ĵ���
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
void condition(symset fsys){ //�ݹ��½����ж�<����>�Ĵ���
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
			switch (relop){//���ݱȽϹ�ϵ����ָ��
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
void statement(symset fsys){ //�ݹ��½����ж�<���>�Ĵ���
	int i, cx1, cx2;
	symset set1, set;

	if (sym == SYM_IDENTIFIER){ // variable assignment ���񵽱�����
		mask* mk;
		if (! (i = position(id))){ // û���ҵ�������
			error(11); // Undeclared identifier.
		}else if (table[i].kind != ID_VARIABLE){
			error(12); // Illegal assignment. �÷��Ų��Ǳ���
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
		getsym(); // ����begin����ĵ�һ���ֶ�
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);//���д���ͺ�����whileѭ�������������
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
		if (sym == SYM_END)	{ // begin - end �����
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
void block(symset fsys){//�ݹ��½����ж�<������>�Ĵ��� ��ʵ������ֻ��Ҫ�޸Ĵ˺�������˼��ֻ��Ҫ��д�������塱����
	int cx0; // initial code index

    // ��������������Ҫ���ڴ�������
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;
	dx = 3;
	mk = (mask*) &table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL){
		error(32); // There are too many levels. level�ǵݹ�֮���ȫ�ֱ���
	}

	// �﷨������ʼ
	do{
		if (sym == SYM_CONST){ // constant declarations
			getsym(); // ��ȡconst�ֶκ�����ֵĵ�һ����������id1
			do{//ѭ������id1=num1,id2=num2,����
				constdeclaration(); // ����const�ֶ�֮��ĵ�һ�鳣������id1=num1
				while (sym == SYM_COMMA){ // ��һ�鳣���������Σ���һ���еĺ����ڲ��߼������һ������getsym()����ʱ������һ��','
					getsym(); // ����һ�������� id
					constdeclaration();
				}// ѭ�����������ǲ���';'��Ҳ��ζ�ű�����������
				if (sym == SYM_SEMICOLON){ // ��ȡ���ֺ�';'
					getsym(); // ������ȡ';'���һ���ֶ�
				} else {
					error(5); // Missing ',' or ';'.
				}
			}while (sym == SYM_IDENTIFIER); // ���⣺�ֺź��沶��һ����ʶ������Ȼ�ǳ������壬����ʲô��˼
		} // if

		if (sym == SYM_VAR)	{ // variable declarations
			getsym(); // ��һ��ִ���Ѿ�������һ�� "var"�����ִ��Ӧ�ò���������������id��
			do{
				vardeclaration();
				while (sym == SYM_COMMA){
					getsym(); // ������һ�������������ܲ��񵽷ֺ�';'���������ڳ����������˵��������������
					vardeclaration();
				} // id1, id2, id3;
				if (sym == SYM_SEMICOLON){ // �ϴβ���';'��������������
					getsym(); // ����������һ���ֶ�
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
			getsym(); // ����"procedure"֮��ĵ�һ���ֶΣ���������
			if (sym == SYM_IDENTIFIER){
				enter(ID_PROCEDURE);
				getsym(); // �������������ķֺ�';'
			}else{
				error(4); // There must be an identifier to follow 'procedure'.
			}

			if (sym == SYM_SEMICOLON) { // �ϴβ�����Ƿֺ�';'
				getsym(); //����������һ���ֶΡ���һ�β�����ǵ�4�е�"var"
			}else{
				error(5); // Missing ',' or ';'.
			}
			level++;//��������Ƕ��,level�Ǻ�����Ƕ�׼���
			savedTx = tx;
			block_dx = dx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);//Ƕ�ף��ݹ飩���ú��������е�<������>������block
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;
            dx = block_dx;
			// ��ǰ��procedure�����е�block���ֽ�������������ŵ�Ӧ���Ƿֺ�
			if (sym == SYM_SEMICOLON) {
				getsym();
			
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6); // ��鲶���token�ǲ��� procedure �� ����Ԥ�����
				destroyset(set1);
				destroyset(set);
			}else{
				error(5); // Missing ',' or ';'.
			}
		} // while
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7); // �����ʲô���ã����﷨�������ʱ���������󲿷֣�����ʶ������﷨��ȷ����
		destroyset(set1);
		destroyset(set);
	}while (inset(sym, declbegsys)); // �ող��񵽵���'const' 'var' 'procedure' 'Null'�е�һ��

	// ����������Ҫ���ڴ�������
	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);

	statement(set); // ���

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
	listcode(cx0, cx); // ����̨��ʾ���ɴ��룿
	
	// printf("��ͣ\n");
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
	char s[80]; // ��ȡ������
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
	err �ʷ���/�﷨����������
    cx ���ڴ���Ĵ�����������
    cc ��ǰ��ȡ�ַ���
    ll ���������뻺�壨��ȡһ�У�ָ��
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

// �����������ڽ���ִ���м����
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
