#pragma once
#ifndef IDTRANS_H
#define IDTRANS_H

#define LEN 64 + 1
#define TRANS_MAX 1024
typedef unsigned short Byte2;

#include <winsock2.h>
#include <string>
#include <string.h>
#include <time.h>

#define TTL 10// 过期时间是10s

//在中继DNS的ID转换表
typedef struct {
	Byte2 oID;
	bool done;
	SOCKADDR_IN client;
	//int joinTime;
	std::string url;
	int offset;

}IDTrans;

int IDTCount;
IDTrans IDT[TRANS_MAX];


void InitIDTrans() {
	int i;
	for (i = 0; i < TRANS_MAX; i++) {
		IDT[i].oID = 0;
		IDT[i].done = false;
		//IDT[i].joinTime = 0;
		IDT[i].offset = 0;
		memset(&(IDT[i].client), 0, sizeof(SOCKADDR_IN));
		memset(&(IDT[i].url), 0, LEN * sizeof(char));
	}

}


Byte2 TransID(Byte2 ID, sockaddr_in clientAddr, bool flag) {

	if (IDTCount == TRANS_MAX) {
		IDTCount == 0;
	}

	IDT[IDTCount].oID = ID;
	IDT[IDTCount].client = clientAddr;
	IDT[IDTCount].done = flag;
	//IDT[IDTCount].joinTime = time(NULL);

	IDTCount++;

	return (Byte2)(IDTCount - 1);
}

#endif //IDTRANS_H