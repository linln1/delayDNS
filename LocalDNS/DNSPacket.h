#pragma once

#ifndef DNSPACKET_H
#define DNSPACKET_H


typedef unsigned short Byte2;

#define DOMAIN_MAX 64+1

#include <string.h>
#include <string>
SYSTEMTIME sysTime;

typedef struct {
	Byte2 ID;
	Byte2 Flags;
	Byte2 QuestNum;
	Byte2 AnswerNum;
	Byte2 AuthorNum;
	Byte2 Addition;
}DNSPacket;

char domain[DOMAIN_MAX];

void getDomainFromRequest(char* recv_buf, int recvnum) 
{
	char dom1[DOMAIN_MAX];
	int i = 0, k = 0;
	int j;

	memset(domain, 0, DOMAIN_MAX);
	memcpy(dom1, &(recv_buf[sizeof(DNSPacket)]), recvnum-16);

	int len = strlen(dom1);

	while (i < len)
	{
		if (dom1[i] > 0 && dom1[i] <= 63)
			for (j = dom1[i], i++; j > 0; j--, i++, k++)
				domain[k] = dom1[i];

		if (dom1[i] != 0)
		{
			domain[k] = '.';
			k++;
		}
	}
	domain[k] = '\0';
}

void getDomainFromResponse(char* recv_buf, char* dest) {
	int len = strlen(recv_buf);
	int i = 0, j = 0, k = 0;
	while (i < len)
	{
		if (recv_buf[i] > 0 && recv_buf[i] <= 63) //如果是个计数
		{
			for (j = recv_buf[i], i++; j > 0; j--, i++, k++) //j是计数是几，k是目标位置下标，i是报文里的下标
				dest[k] = recv_buf[i];
		}

		if (recv_buf[i] != 0)    //如果没结束就在dest里加个'.'
		{
			dest[k] = '.';
			k++;
		}
	}
	dest[k] = '\0';

}

#endif //DNSPACKET_H
