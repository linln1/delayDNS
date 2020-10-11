#pragma once
#ifndef DISPLAY_H
#define DISPLAY_H

#include "IpDomain.h"
#include "IDTrans.h"
#include "DNSPacket.h"
#include "cache.h"

#include <stdlib.h>


void DisplayInfo(unsigned short RID,int find,int level)
{	
	switch(level) {
		case 1:
			if (find == NOT_FOUND) {
				printf("======================\n");
				printf("[中继]:\n");
				printf("[域名]:%s\n", domain);
				printf("======================\n");
			}
			else {
				if (strcmp(IPD[find].ip , "0.0.0.0")==0)
				{
					printf("======================\n");
					printf("[屏蔽]\n");
					printf("[域名]:*.*.*.*%s\n", domain);
					printf("======================\n");
				}
				else {
					printf("======================\n");
					printf("[服务器]\n");
					printf("[域名]:%s\n", domain);
					printf("[IP]:%s\n", IPD[find].ip);
					printf("======================\n");
				}
			}
			break;

		case 2: {
				printf("==========================================\n");
				time_t t = time(NULL);
				char temp[64];
				strftime(temp, sizeof(temp), "%Y/%m/%d %X %A", localtime(&t));
				printf("时间坐标:%s\n", temp);
				printf("ID:%u\n", RID);
				printf("==========================================\n");
				if (find == -1) {
					printf("======================\n");
					printf("[中继]\n");
					printf("[域名]:%s\n", domain);
					printf("======================\n");
				}
				else {
					if (IPD[find].ip == "0.0.0.0")
					{
						printf("======================\n");
						printf("[屏蔽]\n");
						printf("[域名]:*.*.*.*%s\n", domain);
						printf("======================\n");
					}
					else {
						printf("======================\n");
						printf("[服务器]\n");
						printf("[域名]:%s\n", domain);
						printf("[IP]:%s\n", IPD[find].ip);
						//打印客户端IP地址
						char ip_temp[40];
						strcpy(ip_temp, inet_ntoa(IDT[RID].client.sin_addr));
						printf("[CLIENT IP]: %s", ip_temp);
						printf("======================\n");
					}
				}
			}
			break;

		default:
			break;
	}
}


void DisplayAnswer(unsigned short RID, int level, char* recv_buf) {

	int index = (int)RID;
	char url[LEN];
	int length = -1;

	length = IDT[index].offset;
	int nquery = ntohs(*(unsigned short*)(recv_buf + 4));
	int nresponse = ntohs(*(unsigned short*)(recv_buf + 6));

	char* p = recv_buf + 12;
	char Ip[16];
	int ip1, ip2, ip3, ip4;
	int i;
	for (i = 0; i < nquery; i++) {
		getDomainFromResponse(p, url);
		while (*p > 0)
			p += (*p) + 1;
		p += 5;
	}
	for (i = 0; i < nresponse; i++) {
		if ((unsigned char)*p == 0xc0)
			p += 2;
		else {
			while (*p > 0)
				p += (*p) + 1;
			++p;
		}
		unsigned short resp_type = ntohs(*(unsigned short*)p);//回复类型
		p += 2;
		unsigned short resp_class = ntohs(*(unsigned short*)p); //回复类
		p += 2;
		unsigned short high = ntohs(*(unsigned short*)p);   //生存时间高位
		p += 2;
		unsigned short low = ntohs(*(unsigned short*)p);    //生存时间低位
		p += 2;
		unsigned long  ttl = (((int)high) << 16) | low;    //高低位组合成生存时间
		int datalen = ntohs(*(unsigned short*)p);   //后面数据长度
		p += 2;

		if (resp_type == 1) //是A类型，回复的是dom的ip
		{
			memset(Ip, 0, sizeof(Ip));
			//读取4个ip部分
			ip1 = (unsigned char)*p++;
			ip2 = (unsigned char)*p++;
			ip3 = (unsigned char)*p++;
			ip4 = (unsigned char)*p++;

			sprintf(Ip,"%d.%d.%d.%d", ip1, ip2, ip3, ip4);
			//if (level) {
			printf("[url]:%s  [ip]:%s\n", url, Ip);
			//}
			if (level == 2)
			{
				printf("type:%d  class:%d  ttl:%d\n", resp_type, resp_class, ttl);
			}
			//加入缓存
			char ip[16];
			memcpy(ip, Ip, sizeof(Ip));
			add_to_cache(url, ip, ttl);
			output_cache();
			break;
		}
		else p += datalen;  //直接跳过
	}
}
#endif
