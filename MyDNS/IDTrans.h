#ifndef IDTRANS_H
#define IDTRANS_H

#define LEN 256
#define TRANS_MAX 1024
typedef unsigned short Byte2;

#include <winsock2.h>
#include<MSWSock.h>
#include<Windows.h>
#include <string.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")
//#include <string>

//#define TTL 10// 过期时间是10s

//在中继DNS的ID转换表
typedef struct {
	Byte2 oID;
	int done;
	SOCKADDR_IN client;
	//int joinTime;
	char *url;
	int offset;

}IDTrans;

int IDTCount;
IDTrans IDT[TRANS_MAX];


void InitIDTrans() {
	int i;
	for (i = 0; i < TRANS_MAX; i++) {
		IDT[i].oID = 0;
		IDT[i].done = 0;
		//IDT[i].joinTime = 0;
		IDT[i].offset = 0;
		memset(&(IDT[i].client), 0, sizeof(SOCKADDR_IN));
		memset(&(IDT[i].url), 0, LEN * sizeof(char));
	}

}


Byte2 TransID(Byte2 ID,struct sockaddr_in clientAddr, int flag) {

	if (IDTCount == TRANS_MAX) {
		IDTCount == 0;
	}

	IDT[IDTCount].oID = ID;
	IDT[IDTCount].client = clientAddr;
	IDT[IDTCount].done = flag;
	//IDT[IDTCount].joinTime = time(NULL);
	printf("[ID %u->%d]\n", ID, IDTCount);
	IDTCount++;

	return (Byte2)(IDTCount - 1);
}

#endif //IDTRANS_H
