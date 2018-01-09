#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "set.h"

symset uniteset(symset s1, symset s2) {//��elem��С��С����ϲ�s1��s2
	symset s;
	snode* p;
	
	s = p = (snode*) malloc(sizeof(snode)); // sָ��ͷ��㣬��������
	if (s1)
		s1 = s1->next;
	if (s2)
		s2 = s2->next;
	while (s1 && s2) {
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		if (s1->elem < s2->elem) {
			p->elem = s1->elem;
			s1 = s1->next;
		}else{
			p->elem = s2->elem;
			s2 = s2->next;
		}
	}

	while (s1)	{ // s1��ʣ�����ݣ�ֱ������ں���
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		p->elem = s1->elem;
		s1 = s1->next;
	}

	while (s2)	{ // ͬ��
		p->next = (snode*) malloc(sizeof(snode));
		p = p->next;
		p->elem = s2->elem;
		s2 = s2->next;
	}

	p->next = NULL; // β�����ָ���ÿ�

	return s;
} // uniteset

void setinsert(symset s, int elem) {//����elem��s
	snode* p = s;
	snode* q;

	while (p->next && p->next->elem < elem)	{
		p = p->next;
	}
	
	q = (snode*) malloc(sizeof(snode));
	q->elem = elem;
	q->next = p->next;
	p->next = q;
} // setinsert

//���ݲ����½�һ��symset��ע������ͷû��Ԫ��
symset createset(int elem, .../* SYM_NULL */){
	va_list list;
	symset s;

	s = (snode*) malloc(sizeof(snode));
	s->next = NULL;

	va_start(list, elem);
	while (elem) {
		setinsert(s, elem);
		elem = va_arg(list, int);
	}
	va_end(list);
	return s;
} // createset

void destroyset(symset s){//ɾ��s
	snode* p;

	while (s) {
		p = s;
		s = s->next;
		free(p);
	}
} // destroyset

int inset(int elem, symset s){//�ж�elem�Ƿ���s��
	s = s->next;
	while (s && s->elem < elem)
		s = s->next;

	if (s && s->elem == elem)
		return 1;
	else
		return 0;
} // inset

void rm_from_set(int elem, symset s) { // ɾ��һ��Ԫ��
	snode *p;

	// s = s -> next;
	while (s->next && s->next->elem < elem) {
		s = s->next;
	}
	while (s->next && s->next->elem == elem) {
		p = s->next;
		s->next = s->next->next;
		free(p);
	}
}

void printset(symset s) {
	s = s->next;
	while (s) {
		printf("%d, ", s->elem);
		s = s->next;
	}
	printf("\n");
}
// EOF set.c
