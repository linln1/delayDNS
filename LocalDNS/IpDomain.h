#pragma once

#ifndef IPDOMAIN_H
#define IPDOMAIN_H

#include <string>
#define TABLE_MAX 1024
#define NOT_FOUND -1
using namespace std;

typedef struct {
	string ip;
	string dom;
}IpDomain;

int IPDCount;
IpDomain IPD[TABLE_MAX];

int getIpByDomain(string dom) {
	int i;
	for (i = 0; i < IPDCount; i++) {
		if (dom == IPD[i].dom) {
			return i;
		}
	}
	return NOT_FOUND;
}
#endif //IPDOMAIN_H
