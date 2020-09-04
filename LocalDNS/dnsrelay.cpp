//DNS在进行区域传输的时候使用TCP协议，其它时候则使用UDP协议
//实现缓存文件
//多客户端并发 允许多个客户端（可能会位于不同的多个计算机）的并发查询，即：允许第一个查询尚未得到答案前就启动处理另外一个客户端查询请求（DNS协议头中ID字段的作用）
//超时处理 由于UDP的不可靠性，考虑求助外部DNS服务器（中继）却不能得到应答或者收到迟到应答的情形

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//相当于这里我们的客户端和本地DNS服务器建立了连接，因为使用了listen 和 accept函数， 然后back_log=5就是说最大等待队列是5

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <string.h>
#include <winsock2.h>
#include <iostream>
#include <fstream>

#include "DNSPacket.h"
#include "IDTrans.h"
#include "IpDomain.h"
#include "Display.h"


#define BUF_SIZE 1024
#define REMOTE_DNS "10.3.9.5"
//#define REMOTE_DNS "59.64.129.76" //北京市海淀区 教育网
#define LOCAL_DNS "127.0.0.1"
#define FILENAME_LEN 128
#define ADDR_LEN 256	
#define DNS_PORT 53
#define BACK_LOG 5 //同时最多监听5个socket请求
#define TIME_OUT 10000 //10s
#define CACHE_SIZE 30

#pragma comment(lib, "ws2_32.lib")

using namespace std;

//域名长度最大65

//

int Day, Hour, Minute, Second, Millionseconds;


void printinit(int level) {
	printf("=====================================\n");
	printf("DNS_Server Designed by linln1@2020\n");
	printf("\n");
	printf("running...\n");
	printf("debug level = %d\n", level);
	printf("=====================================\n");

}

void loadIPD(char* filepath, IpDomain* IPD) {
	FILE* fp;
	char data[256];
	IPDCount = 0;
	fp = fopen(filepath, "r");
	if (fp == nullptr) {
		printf("fail to open the file!\n");
	}
	else {
		printf("The file is open!\n");
		char ip[16], url[65];
		while (fscanf(fp,"%s %s", ip, url)>0) {

			IPD[IPDCount].ip = ip;
			IPD[IPDCount].dom = url;
			printf("Read from 'dnsrelay.txt' url : %s, ip : %s\n", url, ip);
			IPDCount++;
		}
		fclose(fp);
		printf("loadIPDomainTable successful");
	}

}


//增加cache功能
typedef struct {
	string url;
	string ip;
}cache_entity;

cache_entity cache[CACHE_SIZE];

void add_to_cache(string url, string ip) {
	
}

int main(int argc, char** argv) {

	WSADATA wsaData;
	char send_buf[BUF_SIZE], recv_buf[BUF_SIZE];
	SOCKET localSocket, remoteSocket;
	SOCKADDR_IN clientAddr, localAddr, remoteAddr;
	int requestVal, responseVal;
	int request2Val, response2Val; // 转发请求和转发响应
	Byte2 send_port, recv_port;
	char send_addr[LEN], recv_addr[LEN];
	char file[FILENAME_LEN];
	char REMOTE_DNS_ADDR[16];
	int jTime;
	int debuglevel = 0;
	if (argc > 1 && argv[1][0] == '-')
	{
		if (argv[1][1] == 'd') debuglevel++; /* Debug level add to 1 */
		if (argv[1][2] == 'd') debuglevel++; /* Debug level add to 2 */
	}

	printinit(debuglevel);

	strcpy(file, "dnsrelay.txt");
	strcpy(REMOTE_DNS_ADDR, REMOTE_DNS);

	switch (argc)
	{
	case 2:
		strcpy(REMOTE_DNS_ADDR, REMOTE_DNS);
		strcpy(file, "dnsrelay.txt");
		break;
	case 3:
		strcpy(REMOTE_DNS_ADDR, argv[2]);
		strcpy(file, "dnsrelay.txt");
		break;
	case 4:
		strcpy(REMOTE_DNS_ADDR, argv[2]);
		strcpy(file, argv[3]);
		break;
	default:
		break;
	}

	InitIDTrans();
	loadIPD(file, IPD);
	GetLocalTime(&sysTime);
	Day = sysTime.wDay;
	Hour = sysTime.wHour;
	Minute = sysTime.wMinute;
	Second = sysTime.wSecond;
	Millionseconds = sysTime.wMilliseconds;


	//获取socket版本库
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		//printf("Error @ WSAStartUp()\n");
	}
	else {
		printf("successful load winsock2 dll lib\n");
	}
	//建立 remoteDNS 服务器连接
	//AF_UNIX:AF_LOCAL本地通信

	//AF_INET : IPv4网络通信

	//AF_INET6 : IPv6网络通信

	//AF_PACKET : 链路层通信
	//内核自己据定使用哪个协议
	remoteSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (remoteSocket == INVALID_SOCKET) {
		printf("Error @ setup remoteSocket socket.\n");
		WSACleanup();
	}
	else {
		printf("Successful setup remoteSocket socket\n");
	}
	//set the socket I/O mode: In this case FIONBIO
	//if iMode = 0, blocking is enabled; if iMode != 0, non-blocking mode is enabled.
	int iResult;
	u_long iMode = 1;
	iResult = ioctlsocket(remoteSocket, FIONBIO, (u_long FAR*)&iMode);
	if (iResult != NO_ERROR) {
		printf("icotlsocket failed with error\n");
	}
	else {
		printf("icotlsocket successful setup remoteSocket's I/O Mode!\n");
	}
	//建立 localDNS 套接字
	localSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (localSocket == INVALID_SOCKET) {
		printf("Error @ setup localSocket socket\n");
		WSACleanup();
		exit(1);
	}
	else {
		printf("Successful setup localSocket!\n");
	}
	iResult = ioctlsocket(localSocket, FIONBIO, (u_long FAR*)&iMode);
	if (iResult != NO_ERROR) {
		printf("icotlsokcet failed with error\n");
	}
	else {
		printf("icotlsocket successful setup localSocket's I/O Mode!\n");
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(DNS_PORT);
	localAddr.sin_addr.S_un.S_addr = inet_addr(LOCAL_DNS);

	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(DNS_PORT);
	remoteAddr.sin_addr.S_un.S_addr = inet_addr(REMOTE_DNS_ADDR);
	

	//设置socket 避免端口被占用
	int reuse = 1;
	setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

	//localDNS服务器需要同时监听多个client发出的请求，所以需要listen(localSocket, BACK_LOG)
	int ibind;
	
	ibind = bind(localSocket, (sockaddr*)&localAddr, sizeof(localAddr));
	if (ibind != NO_ERROR) {
		printf("Error @ bind localSocket \n");
		WSACleanup();
		exit(1);
	}
	else {
		printf("bind port 53 successful!\n");
	}
		
/*
	int ilisten;
	//BACK_LOG 并发连接数
	ilisten = listen(localSocket, BACK_LOG);
	if (ilisten == NO_ERROR) {
		printf("successful listen client's request!\n");
	}
*/
//windows:
	int timeout = TIME_OUT;
	//设置发送超时时间
	int ret_send = setsockopt(localSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));
	//设置接收超时时间
	int ret_recv = setsockopt(localSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	/*
		

	//接受连接请求，返回一个新的对应于此次链接的套接字
	SOCKET communication;
	communication = accept(localSocket, (SOCKADDR *)&clientAddr, sizeof(clientAddr));

	//使用返回的套接字和客户端进行通讯
	send(communication, send_buf, sizeof(send_buf), 0);


	//基于UDP协议的DNS协议不需要连接
		responseVal = connect(localSocket, (struct SOCKADDR*)&localAddr, sizeof(localAddr));
		if (responseVal == SOCKET_ERROR{
			printf("Failure to connect Server\n");
			closesocket(localSocket);
			WSACleanup();
			exit(1);
	}
	*/
	//sendto和recvfrom一般用于UDP协议中,但是如果在TCP中connect函数调用后也可以用.

	//从 客户端 向 localAddr 发送请求
	while (true) {

		int sizeofclient = sizeof(clientAddr);
		memset(recv_buf, 0, BUF_SIZE);

		//判断是否收到外部DNS的反馈
		long int count = 0;
		while (count < 3)
		{
			//接收来自外部DNS服务器的响应报文
			requestVal = recvfrom(remoteSocket, recv_buf, sizeof(recv_buf), 0, (SOCKADDR*)&remoteAddr, &sizeofclient);
			
			{
				send_port = ntohs(remoteAddr.sin_port);
				strcpy(send_addr, inet_ntoa(remoteAddr.sin_addr));
				recv_port = DNS_PORT;
				strcpy(recv_addr, LOCAL_DNS);
			}

			if (requestVal > -1)
			{
				printf("收到外部dns ip: %ud回答\n", send_port);
//				time_t t = time(NULL);
	//			char temp[64];
		//		strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
			//	printf("接收时间:%s\n", temp);
				
				//ID转换
				unsigned short* pID = (unsigned short*)malloc(sizeof(unsigned short));
				memcpy(pID, recv_buf, sizeof(unsigned short));
				int m = ntohs(*pID);
				free(pID);//释放动态分配的内存

				unsigned short oID = htons(IDT[m].oID);
				memcpy(recv_buf, &oID, sizeof(unsigned short));
				IDT[m].done = TRUE;
				IDTCount--;

				//从ID转换表中获取发出DNS请求者的信息
				clientAddr = IDT[m].client;

				//打印id信息 
				DisplayAnswer(m ,debuglevel, recv_buf);

				//把recvbuf转发至请求者处
				responseVal = sendto(localSocket, recv_buf, requestVal, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
				if (responseVal == SOCKET_ERROR) {
					cout << "sendto Failed: " << WSAGetLastError() << endl;
					continue;
				}
				else if (responseVal == 0)
					break;	 
			}

			count++;
		}



		memset(recv_buf, 0, BUF_SIZE);
		//printf("waiting for connection!\n");
		int len = sizeof(clientAddr);
		/*
		SOCKET clientSocket = accept(localSocket, (sockaddr*)&clientAddr, &len);
		if (clientSocket == INVALID_SOCKET) {
				//printf("localDNS Socket accept error!\n");
			continue;
		}
		*/
		requestVal = recvfrom(localSocket, recv_buf, sizeof(recv_buf), 0, (sockaddr *)&clientAddr, &len);
		//requestVal = recvfrom(localSocket, recv_buf, BUF_SIZE, 0, (struct SOCKADDR*)&clientAddr, sizeof(clientAddr));
		if (requestVal == SOCKET_ERROR) {
			//printf("send request Error from client to localDNS!\n");
			//closesocket(localSocket);
		}
		else {
			printf("localDNS receive request from client, length: %d\n", requestVal);

			time_t t = time(NULL);
			char temp[64];
			strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
			printf("%s\n", temp);

			recv_buf[requestVal] = '\0';

			{
				send_port = ntohs(clientAddr.sin_port);
				strcpy(send_addr, inet_ntoa(clientAddr.sin_addr));
				recv_port = DNS_PORT;
				strcpy(recv_addr, LOCAL_DNS);
			}
			printf("before getDomainFromRequset!\n");
			getDomainFromRequest(recv_buf, requestVal);//url放入全局变量domain里面
			printf("before getIpByDomain\n");
			int find = getIpByDomain(domain);
			printf("after getIpByDomain\n");
			if (find == NOT_FOUND) {
				printf("can't find\n");
				GetLocalTime(&sysTime);
				Day = sysTime.wDay;
				Hour = sysTime.wHour;
				Minute = sysTime.wMinute;
				Second = sysTime.wSecond;
				jTime = (((sysTime.wDay - Day) * 24 + sysTime.wHour - Hour) * 60 + sysTime.wMinute - Minute) * 60 + sysTime.wSecond - Second;

				//将QID信息进行转换
				Byte2* QID = (Byte2*)malloc(sizeof(Byte2));
				memcpy(QID, recv_buf, sizeof(Byte2));


				//Maybe wrong
				Byte2 RID = htons(TransID(ntohs(*QID), clientAddr, FALSE));
				memcpy(recv_buf, &RID, sizeof(Byte2));

				IDT[IDTCount - 1].url = domain;
				IDT[IDTCount - 1].offset = requestVal;

				//迭代查询，向remoteSocket发送请求				
				request2Val = sendto(remoteSocket, recv_buf, sizeof(recv_buf), 0, (sockaddr*)&remoteAddr, sizeof(remoteAddr));
				if (request2Val == SOCKET_ERROR) {
					continue;
				}
				else if (request2Val) {
					printf("transfer request to remoteSocket!  length:%d\n",request2Val);
				}
				free(QID);

				//接收remoteAddr 的应答
				responseVal = recvfrom(remoteSocket, recv_buf, sizeof(recv_buf), 0, (sockaddr*)&remoteAddr, &len);
				printf("accept response from remoteAddr!\n");
				printf("length:%d\n", responseVal);
				if(responseVal > -1)//	recv_buf[responseVal] = '\0';
				{
					send_port = ntohs(remoteAddr.sin_port);
					strcpy(send_addr, inet_ntoa(remoteAddr.sin_addr));
					recv_port = DNS_PORT;
					strcpy(recv_addr, LOCAL_DNS);
					//print time RID function domain ip
					printf("before displayInfo\n");
					DisplayInfo(ntohs(RID), find, debuglevel);
					DisplayAnswer(ntohs(RID), debuglevel, recv_buf);
					if (debuglevel == 2) {
						unsigned char message;
						printf("send_addr:%ud\n", send_addr);
						printf("send_port:%ud\n", send_port);
						printf("recv_addr:%ud\n", recv_addr);
						printf("recv_port:%ud\n", recv_port);
						printf("DNSPacket length = %d\n", requestVal);
						printf("Connect showed as hex:\n");
						for (int i = 0; i < responseVal; i++)
						{
							message = (unsigned char)recv_buf[i];
							printf("%02x ", message);
						}
					}


					RID = (Byte2)malloc(sizeof(Byte2));
					memcpy(&RID, recv_buf, sizeof(Byte2));
					int index = ntohs(RID);
					Byte2 oID = htons(IDT[index].oID);
					memcpy(recv_buf, &oID, sizeof(Byte2));
					IDT[index].done = true;

					clientAddr = IDT[index].client;

					response2Val = sendto(localSocket, recv_buf, responseVal, 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
					if (response2Val == SOCKET_ERROR) {
						continue;
					}
					else if (response2Val == 0) {
						break;
					}
					//free(&RID);
				}
				
				

				//如果要再加附加功能
				//从响应里面取出对应的ip地址，写入到IpDomain_cache.txt 文件里面去
			}
			else {//找到了相应的域名
				printf("already find!\n");
				

				Byte2* QID = (Byte2*)malloc(sizeof(Byte2));
				memcpy(QID, recv_buf, sizeof(Byte2));

				Byte2 RID = TransID(ntohs(*QID), clientAddr, true);

				DisplayInfo(RID, find, debuglevel);

				memcpy(send_buf, recv_buf, requestVal);
				Byte2 a = htons(0x8180);
				memcpy(&send_buf[2], &a, sizeof(Byte2));

				if (strcmp(IPD[find].ip.c_str(), "0.0.0.0") == 0) {
					a = htons(0x0000);
				}
				else
					a = htons(0x0001);
				memcpy(&send_buf[6], &a, sizeof(Byte2));
				int curLen = 0;

				char answer[16];
				Byte2 Name = htons(0xc00c);
				memcpy(answer, &Name, sizeof(Byte2));
				curLen += sizeof(Byte2);

				Byte2 TypeA = htons(0x0001);
				memcpy(answer + curLen, &TypeA, sizeof(Byte2));
				curLen += sizeof(Byte2);

				Byte2 ClassA = htons(0x0001);
				memcpy(answer + curLen, &ClassA, sizeof(Byte2));
				curLen += sizeof(Byte2);

				Byte2 timeLive = htons(0x7b);
				memcpy(answer + curLen, &timeLive, sizeof(unsigned long));
				curLen += sizeof(unsigned long);

				Byte2 IPLen = htons(0x0004);
				memcpy(answer + curLen, &IPLen, sizeof(Byte2));
				curLen += sizeof(Byte2);

				Byte2 Ip = (Byte2)inet_addr(IPD[find].ip.c_str());
				memcpy(answer + curLen, &Ip, sizeof(unsigned long));
				curLen += sizeof(unsigned long);
				curLen += requestVal;

				memcpy(send_buf + requestVal, answer, sizeof(answer));

				
				responseVal = sendto(localSocket, send_buf, curLen, 0, (sockaddr *)&clientAddr, sizeof(clientAddr));
				if (responseVal == SOCKET_ERROR) {
					printf("send error from localDNS to client!\n");
					continue;
				}
				else if (responseVal == 0) {
					break;
				}
				
				free(QID);
			}
		}
	
	}

	closesocket(localSocket);
	closesocket(remoteSocket);
	WSACleanup();
	
	return 0;
}	


