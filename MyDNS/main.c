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
#include <string.h>

#include "DNSPacket.h"
#include "IDTrans.h"
#include "IpDomain.h"
#include "Display.h"
#include "cache.h"
//#include <thread>


#define BUF_SIZE 1024
#define REMOTE_DNS "10.3.9.5"
//#define REMOTE_DNS "59.64.129.76" //北京市海淀区 教育网
#define LOCAL_DNS "127.0.0.1"
#define FILENAME_LEN 128
#define ADDR_LEN 256	
#define DNS_PORT 53
//#define BACK_LOG 5 //同时最多监听5个socket请求
#define TIME_OUT 10000 // 10s 超时重传



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
	if (!fp) {
		printf("fail to open the file!\n");
	}
	else {
		printf("The file is open!\n");
		char ip[16], url[256];
		while (fscanf(fp,"%s %s", ip, url)>0) {
			
			memcpy(IPD[IPDCount].ip, ip, sizeof(ip));
			memcpy(IPD[IPDCount].dom, url, sizeof(url));
			
			printf("Read from 'dnsrelay.txt' url : %s, ip : %s\n", url, ip);
			IPDCount++;
		}
		fclose(fp);
		printf("loadIPDomainTable successful\n");
	}
	printf("===============================\n");
}



//双socket
/*
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
/*		if (argv[1][2] == 'd') debuglevel++; /* Debug level add to 2 */
/*	}

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

	init_cache();
	InitIDTrans();
	loadIPD(file, IPD);


	//获取socket版本库
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		//printf("Error @ WSAStartUp()\n");
	}
	else {
		//printf("successful load winsock2 dll lib\n");
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
		//printf("Successful setup remoteSocket socket\n");
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
		//printf("icotlsocket successful setup remoteSocket's I/O Mode!\n");
	}
	//建立 localDNS 套接字
	localSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (localSocket == INVALID_SOCKET) {
		//printf("Error @ setup localSocket socket\n");
		WSACleanup();
		exit(1);
	}
	else {
		//printf("Successful setup localSocket!\n");
	}
	iResult = ioctlsocket(localSocket, FIONBIO, (u_long FAR*)&iMode);
	if (iResult != NO_ERROR) {
		printf("icotlsokcet failed with error\n");
	}
	else {
		//printf("icotlsocket successful setup localSocket's I/O Mode!\n");
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(DNS_PORT);
	localAddr.sin_addr.s_addr = INADDR_ANY;

	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(DNS_PORT);
	remoteAddr.sin_addr.S_un.S_addr = inet_addr(REMOTE_DNS_ADDR);
	

	//设置socket 避免端口被占用
	int reuse = 1;
	setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

	//localDNS服务器需要同时监听多个client发出的请求，所以需要listen(localSocket, BACK_LOG)
	int ibind;
	
	ibind = bind(localSocket, (struct sockaddr*)&localAddr, sizeof(localAddr));
	if (ibind != NO_ERROR) {
		printf("Error @ bind localSocket \n");
		WSACleanup();
		exit(1);
	}
	else {
		//printf("bind port 53 successful!\n");
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
//	int timeout = TIME_OUT;
//	//设置发送超时时间
//	int ret_send = setsockopt(localSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(int));
//	//设置接收超时时间
//	int ret_recv = setsockopt(localSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

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
/*
	while (1) {

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
/*		requestVal = recvfrom(localSocket, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&clientAddr, &len);
		//requestVal = recvfrom(localSocket, recv_buf, BUF_SIZE, 0, (struct SOCKADDR*)&clientAddr, sizeof(clientAddr));
		if (requestVal <= 0) {
			//printf("send request Error from client to localDNS!\n");
			//closesocket(localSocket);
		}
		else {
			//printf("localDNS receive request from client, length: %d\n", requestVal);

			time_t t = time(NULL);
			char temp[64];
			strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
			printf("\n%s\n", temp);

			recv_buf[requestVal] = '\0';
			
			{
				send_port = ntohs(clientAddr.sin_port);
				strcpy(send_addr, inet_ntoa(clientAddr.sin_addr));
				recv_port = DNS_PORT;
				strcpy(recv_addr, LOCAL_DNS);
				// 
				printf("requset from :\n");
				printf("send_addr:%s\n",send_addr);
			}
			//printf("before getDomainFromRequset!\n");
			getDomainFromRequest(recv_buf, requestVal);//url放入全局变量domain里面
			//printf("before getIpByDomain\n");
			int find = getIpByDomain(domain);
			int find_in_cache = getIncache(domain);
 			printf("[requset domain] :%s\n",domain);

			//如果是IPv6请求，就自动忽略掉
			unsigned short qtype = 0;
			//qtype = (unsigned short*)malloc(sizeof(unsigned short));

			char dom1[LEN];
			memcpy(dom1, recv_buf + 12, requestVal - 16);
			int len = strlen(dom1);

			memcpy(&qtype, recv_buf + requestVal - 4, sizeof(unsigned short));
			//printf("%u\n", qtype);
			printf("[qtype]:%u\n", ntohs(qtype));
			//if (ntohs(qtype) == 28) {
			//	continue;
			//}

			if (find == NOT_FOUND && find_in_cache == NOT_FOUND) {
				printf("can't find in the cache and local dns\n");

				//将QID信息进行转换
				Byte2* QID = (Byte2*)malloc(sizeof(Byte2));
				memcpy(QID, recv_buf, sizeof(Byte2));


				//转换成RID
				Byte2 RID = htons(TransID(ntohs(*QID), clientAddr, FALSE));
				memcpy(recv_buf, &RID, sizeof(Byte2));

				IDT[IDTCount - 1].url = domain;
				IDT[IDTCount - 1].offset = requestVal;

				//迭代查询，向remoteSocket发送请求				
				request2Val = sendto(remoteSocket, recv_buf, requestVal, 0, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr));
				if (request2Val == SOCKET_ERROR) {
					continue;
				}
				else if (request2Val) {
					//printf("transfer request to remoteSocket!  length:%d\n",request2Val);
				}
				free(QID);

				//接收remoteAddr 的应答
				int count = 3;
				while (count) {
					responseVal = recvfrom(remoteSocket, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&remoteAddr, &len);
					if (responseVal > -1)//	recv_buf[responseVal] = '\0';
					{
						//printf("accept response from remoteAddr!\n");
						send_port = ntohs(remoteAddr.sin_port);
						strcpy(send_addr, inet_ntoa(remoteAddr.sin_addr));
						recv_port = DNS_PORT;
						strcpy(recv_addr, LOCAL_DNS);
						//print time RID function domain ip
						//printf("before displayInfo\n");
						DisplayInfo(ntohs(RID), find, debuglevel);
						DisplayAnswer(ntohs(RID), debuglevel, recv_buf);
						
						if (debuglevel == 2) {
							unsigned char message;
							printf("send_addr:%s\n", send_addr);
							printf("send_port:%u\n", send_port);
							printf("recv_addr:%s\n", recv_addr);
							printf("recv_port:%u\n", recv_port);
							printf("DNSPacket length = %d\n", requestVal);
							printf("packet showed as hex:\n");
							int i;
							for (i = 0; i < responseVal; i++)
							{
								message = (unsigned char)recv_buf[i];
								printf("%02x ", message);
							}
							printf("\n");
							printf("===============================\n");
						}


						RID = (Byte2)malloc(sizeof(Byte2));
						memcpy(&RID, recv_buf, sizeof(Byte2));
						int index = ntohs(RID);
						Byte2 oID = htons(IDT[index].oID);
						memcpy(recv_buf, &oID, sizeof(Byte2));
						IDT[index].done = 1;

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
					count--;
				}
				//如果要再加附加功能
				//从响应里面取出对应的ip地址，写入到cache里面去
			}
			else if (find == NOT_FOUND && find_in_cache != NOT_FOUND) {
				//服务器从缓存里面找到了,需要构造信息返回结果 sendto(localSocket, (sockaddr*)&clientAddr, sizeof(clientAddr));
				printf("find in the cache!\n");

				int index = find_in_cache;
				char*url = cache[index].url;
				char*ip = cache[index].ip;
				unsigned long ttl = cache[index].ttl;
				add_to_cache(url, ip, ttl);//更新一下,LRU算法

				Byte2* QID = (Byte2*)malloc(sizeof(Byte2));
				memcpy(QID, recv_buf, sizeof(Byte2));

				Byte2 RID = TransID(ntohs(*QID), clientAddr, 1);

				if (debuglevel == 2) {
					printf("======================\n");
					printf("[ID]:%u\n", RID);
				}


				printf("======================\n");
				printf("[缓存]\n");
				printf("[域名]:%s\n",url);
				printf("[IP]:%s\n",ip);
				printf("======================\n");
				//printf("[域名]:%s\n", url);
				//printf("[IP]:%s\n", ip);
				
				//打印现在缓存中的信息
				printf("======================\n");
				time_t t = time(NULL);
				char temp[64];
				strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
				printf("%s\n", temp);

				output_cache();
				printf("======================\n");

				//构造响应报文
				memcpy(send_buf, recv_buf, requestVal);
				Byte2 a = htons(0x8180);
				memcpy(&send_buf[2], &a, sizeof(Byte2));

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

				Byte2 timeLive = htons(0x7b);//ttl时间
				memcpy(answer + curLen, &timeLive, sizeof(unsigned long));
				curLen += sizeof(unsigned long);

				Byte2 IPLen = htons(0x0004);
				memcpy(answer + curLen, &IPLen, sizeof(Byte2));
				curLen += sizeof(Byte2);

				Byte2 Ip = (Byte2)inet_addr(ip);
				memcpy(answer + curLen, &Ip, sizeof(unsigned long));
				curLen += sizeof(unsigned long);
				curLen += requestVal;

				memcpy(send_buf + requestVal, answer, sizeof(answer));


				responseVal = sendto(localSocket, send_buf, curLen, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
				if (responseVal == SOCKET_ERROR) {
					printf("send error from localDNS to client!\n");
					continue;
				}
				else if (responseVal == 0) {
					break;
				}

				free(QID);

			}
			else {//找到了相应的域名
				printf("find in localDNS!\n");
				printf("\n======================\n");
				time_t t = time(NULL);
				char temp[64];
				strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
				printf("[时间坐标]:%s\n", temp);
				printf("======================\n");

				Byte2* QID = (Byte2*)malloc(sizeof(Byte2));
				memcpy(QID, recv_buf, sizeof(Byte2));

				Byte2 RID = TransID(ntohs(*QID), clientAddr, 1);

				DisplayInfo(RID, find, debuglevel);

				memcpy(send_buf, recv_buf, requestVal);
				//AA=1
				Byte2 a, b;
				if (strcmp(IPD[find].ip, "0.0.0.0") == 0) {
					b = htons(0x0000);
					a = htons(0x8583);
					memcpy(&send_buf[2], &a, sizeof(Byte2));
				}
				else {
					
					a = htons(0x8580);
					memcpy(&send_buf[2], &a, sizeof(Byte2));
					b = htons(0x0001);
				}
					
				memcpy(&send_buf[6], &b, sizeof(Byte2));
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

				Byte2 Ip = (Byte2)inet_addr(IPD[find].ip);
				memcpy(answer + curLen, &Ip, sizeof(unsigned long));
				curLen += sizeof(unsigned long);
				curLen += requestVal;

				memcpy(send_buf + requestVal, answer, sizeof(answer));

				
				responseVal = sendto(localSocket, send_buf, curLen, 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
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
*/

//void start() {
//	thread 
//
//}

int main(int argc, char** argv) {

	WSADATA wsaData;
	char send_buf[BUF_SIZE], recv_buf[BUF_SIZE];
	SOCKET Socket;
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

	init_cache();
	InitIDTrans();
	loadIPD(file, IPD);


	//获取socket版本库
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR) {
		//printf("Error @ WSAStartUp()\n");
	}
	else {
		//printf("successful load winsock2 dll lib\n");
	}
	//建立 remoteDNS 服务器连接
	//AF_UNIX:AF_LOCAL本地通信

	//AF_INET : IPv4网络通信

	//AF_INET6 : IPv6网络通信

	//AF_PACKET : 链路层通信
	//内核自己据定使用哪个协议

	//建立 localDNS 套接字
	Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (Socket == INVALID_SOCKET) {
		//printf("Error @ setup localSocket socket\n");
		WSACleanup();
		exit(1);
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(DNS_PORT);
	localAddr.sin_addr.s_addr = INADDR_ANY;

	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(DNS_PORT);
	remoteAddr.sin_addr.S_un.S_addr = inet_addr(REMOTE_DNS_ADDR);

	int ibind;

	ibind = bind(Socket, (struct sockaddr*)&localAddr, sizeof(localAddr));
	if (ibind != NO_ERROR) {
		printf("Error @ bind localSocket \n");
		WSACleanup();
		exit(1);
	}
	else {
		//printf("bind port 53 successful!\n");
	}

	while (1) {

		memset(recv_buf, 0, BUF_SIZE);
		//printf("waiting for connection!\n");
		int len = sizeof(clientAddr);

		requestVal = recvfrom(Socket, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&clientAddr, &len);
		if (requestVal <= 0) {
		}
		else {

			time_t t = time(NULL);
			char temp[64];
			strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
			printf("\n%s\n", temp);

			recv_buf[requestVal] = '\0';
			
			char flag = 0;
			memcpy(&flag, recv_buf + 2, 1);

			//如果是IPv6请求，就自动忽略掉
			unsigned short qtype = 0;
			//qtype = (unsigned short*)malloc(sizeof(unsigned short));

			char dom1[LEN];
			memcpy(dom1, recv_buf + 12, requestVal - 16);
			int len = strlen(dom1);

			memcpy(&qtype, recv_buf + requestVal - 4, sizeof(unsigned short));
			//printf("%u\n", qtype);
			//printf("[qtype]:%u\n", ntohs(qtype));
			//if (ntohs(qtype) == 28) {
			//	continue;
			//}

			//收到请求报文
			if (flag == 1 ) {
				recv_port = localAddr.sin_port;
				strcpy(recv_addr, inet_ntoa(localAddr.sin_addr));

				send_port = ntohs(clientAddr.sin_port);
				strcpy(send_addr, inet_ntoa(clientAddr.sin_addr));

				// 
				printf("requset from :\n");
				printf("send_addr:%s\n", send_addr);

				getDomainFromRequest(recv_buf, requestVal);//url放入全局变量domain里面

				int find = getIpByDomain(domain);
				int find_in_cache = getIncache(domain);
				printf("[requset domain] :%s\n", domain);

				if ((find == NOT_FOUND && find_in_cache == NOT_FOUND) || ntohs(qtype) == 28) {
					printf("can't find in the cache and local dns\n");

					//将QID信息进行转换
					Byte2* QID = (Byte2*)malloc(sizeof(Byte2));
					memcpy(QID, recv_buf, sizeof(Byte2));


					//转换成RID
					Byte2 RID = htons(TransID(ntohs(*QID), clientAddr, FALSE));
					memcpy(recv_buf, &RID, sizeof(Byte2));

					IDT[IDTCount - 1].url = domain;
					IDT[IDTCount - 1].offset = requestVal;

					//迭代查询，向remoteSocket发送请求				
					request2Val = sendto(Socket, recv_buf, requestVal, 0, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr));
					if (request2Val == SOCKET_ERROR) {
						continue;
					}
					else if (request2Val) {
						//printf("transfer request to remoteSocket!  length:%d\n",request2Val);
					}
					free(QID);

				}
				else if (find == NOT_FOUND && find_in_cache != NOT_FOUND) {
					//服务器从缓存里面找到了,需要构造信息返回结果 sendto(localSocket, (sockaddr*)&clientAddr, sizeof(clientAddr));
					printf("find in the cache!\n");
					printf("Question Type:%u\n", ntohs(qtype));
					int index = find_in_cache;
					char* url = cache[index].url;
					char* ip = cache[index].ip;
					unsigned long ttl = cache[index].ttl;
					add_to_cache(url, ip, ttl);//更新一下,LRU算法

					Byte2* QID = (Byte2*)malloc(sizeof(Byte2));
					memcpy(QID, recv_buf, sizeof(Byte2));

					Byte2 RID = TransID(ntohs(*QID), clientAddr, 1);

					if (debuglevel == 2) {
						printf("======================\n");
						printf("[ID]:%u\n", RID);
					}


					printf("======================\n");
					printf("[缓存]\n");
					printf("[域名]:%s\n", url);
					printf("[IP]:%s\n", ip);
					printf("======================\n");
					//printf("[域名]:%s\n", url);
					//printf("[IP]:%s\n", ip);

					//打印现在缓存中的信息
					printf("======================\n");
					time_t t = time(NULL);
					char temp[64];
					strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
					printf("%s\n", temp);

					output_cache();
					printf("======================\n");

					//构造响应报文
					memcpy(send_buf, recv_buf, requestVal);
					Byte2 a = htons(0x8580);
					memcpy(&send_buf[2], &a, sizeof(Byte2));

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

					unsigned long timeLive = htons(0x7b);//ttl时间,123
					memcpy(answer + curLen, &timeLive, sizeof(unsigned long));
					curLen += sizeof(unsigned long);

					Byte2 IPLen = htons(0x0004);
					memcpy(answer + curLen, &IPLen, sizeof(Byte2));
					curLen += sizeof(Byte2);

					unsigned long Ip = (unsigned long)inet_addr(ip);
					memcpy(answer + curLen, &Ip, sizeof(unsigned long));
					curLen += sizeof(unsigned long);
					curLen += requestVal;

					memcpy(send_buf + requestVal, answer, sizeof(answer));


					responseVal = sendto(Socket, send_buf, curLen, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
					if (responseVal == SOCKET_ERROR) {
						printf("send error from localDNS to client!\n");
						continue;
					}
					else if (responseVal == 0) {
						break;
					}

					free(QID);

				}
				else {//找到了相应的域名
					printf("find in localDNS!\n");
					printf("\n======================\n");
					time_t t = time(NULL);
					char temp[64];
					strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
					printf("[时间坐标]:%s\n", temp);
					printf("======================\n");

					Byte2* QID = (Byte2*)malloc(sizeof(Byte2));
					memcpy(QID, recv_buf, sizeof(Byte2));

					Byte2 RID = TransID(ntohs(*QID), clientAddr, 1);

					DisplayInfo(RID, find, debuglevel);

					memcpy(send_buf, recv_buf, requestVal);
					//AA=1
					Byte2 a, b;
					if (strcmp(IPD[find].ip, "0.0.0.0") == 0) {
						b = htons(0x0000);
						a = htons(0x8583);
						memcpy(&send_buf[2], &a, sizeof(Byte2));
					}
					else {

						a = htons(0x8580);
						memcpy(&send_buf[2], &a, sizeof(Byte2));
						b = htons(0x0001);
					}

					memcpy(&send_buf[6], &b, sizeof(Byte2));
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

					unsigned long timeLive = htons(0x7b);
					memcpy(answer + curLen, &timeLive, sizeof(unsigned long));
					curLen += sizeof(unsigned long);

					Byte2 IPLen = htons(0x0004);
					memcpy(answer + curLen, &IPLen, sizeof(Byte2));
					curLen += sizeof(Byte2);

					unsigned long Ip = (unsigned long)inet_addr(IPD[find].ip);
					memcpy(answer + curLen, &Ip, sizeof(unsigned long));
					curLen += sizeof(unsigned long);
					curLen += requestVal;

					memcpy(send_buf + requestVal, answer, sizeof(answer));


					responseVal = sendto(Socket, send_buf, curLen, 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
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

			else{
			//接收remoteAddr 的应答
				Byte2 RID = 0;
				memcpy(&RID, recv_buf, sizeof(Byte2));

				int index = ntohs(RID);
				Byte2 oID = htons(IDT[index].oID);
				
				memcpy(recv_buf, &oID, sizeof(Byte2));
				IDT[index].done = 1;

				clientAddr = IDT[index].client;

				memcpy(send_buf, recv_buf, sizeof(recv_buf));


				DisplayInfo(ntohs(RID), -1, debuglevel);
				DisplayAnswer(ntohs(RID), debuglevel, recv_buf);
				
				if (debuglevel == 2) {
					unsigned char message;
					printf("send_addr:%s\n", send_addr);
					printf("send_port:%u\n", send_port);
					printf("recv_addr:%s\n", recv_addr);
					printf("recv_port:%u\n", recv_port);
					printf("DNSPacket length = %d\n", requestVal);
					printf("packet showed as hex:\n");
					int i;
					for (i = 0; i < requestVal; i++)
					{
						message = (unsigned char)recv_buf[i];
						printf("%02x ", message);
					}
					printf("\n");
					printf("===============================\n");
				}

				responseVal = sendto(Socket, send_buf, sizeof(send_buf), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));

			}
			
		}

	}

	closesocket(Socket);
	WSACleanup();

	return 0;
}



/*long int count = 0;
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
				//printf("recieve from remoteDNS ip: %s response\n", send_addr);
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
				//IDTCount--;

				//从ID转换表中获取发出DNS请求者的信息
				clientAddr = IDT[m].client;

				//打印id信息 
				DisplayAnswer(m ,debuglevel, recv_buf);
				if (debuglevel == 2) {
					unsigned char message;
					printf("send_addr:%u\n", send_addr);
					printf("send_port:%u\n", send_port);
					printf("recv_addr:%u\n", recv_addr);
					printf("recv_port:%u\n", recv_port);
					printf("DNSPacket length = %d\n", requestVal);
					printf("packet showed as hex:\n");
					for (int i = 0; i < responseVal; i++)
					{
						message = (unsigned char)recv_buf[i];
						printf("%02x ", message);
					}
				}

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
		}*/
