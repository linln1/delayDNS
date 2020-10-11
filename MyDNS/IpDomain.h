
#ifndef IPDOMAIN_H
#define IPDOMAIN_H
#include<stdio.h>
#include <string.h>
#define TABLE_MAX 1024
#define NOT_FOUND -1

typedef struct {
	char ip[16];
	char dom[256];
}IpDomain;

int IPDCount;
IpDomain IPD[TABLE_MAX];

int getIpByDomain(char *dom) {
	int i;
	for (i = 0; i < IPDCount; i++) {
		if (strcmp(dom,IPD[i].dom)==0) {
			return i;
		}
	}
	return NOT_FOUND;
}
#endif //IPDOMAIN_H
