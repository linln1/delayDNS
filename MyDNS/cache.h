
#ifndef CACHE_H
#define CACHE_H
#define _CRT_SECURE_NO_WARNINGS
#include <time.h>
//#include <string>
//#include <iostream>

#define CACHE_SIZE 30
#define TTL 10000
#pragma warning(disable: 4996)

//增加cache功能
typedef struct {
	char url[256];
	char ip[16];
	time_t intime;
	unsigned long ttl;
	int inuse;
}cache_entity;

int cache_count = 0;
cache_entity cache[CACHE_SIZE];

void init_cache() {
	int i;
	for (i = 0; i < CACHE_SIZE; i++) {
		cache[i].inuse = 0;
		cache[i].ttl = TTL;
	}
	return;
}

int add_to_cache(char *url,char *ip, unsigned long ttl) {
	int i;
	for (i = 0; i < cache_count; i++) {
		if (strcmp(cache[i].url,url)== 0 && cache[i].inuse == 1) {
			cache[i].intime = time(NULL);
			cache[i].ttl = ttl;
			if (strcmp(cache[i].ip,ip)== 0) {
				//printf("this url:%s already exits", url);
				return 1;
			}
			else {
				memcpy(cache[i].ip,ip, sizeof(*ip));
				//printf("update domain  %s's ip address to %s", url, ip.c_str());
				return 1;
			}
		}
	}


	//超时机制
	for (i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].intime + cache[i].ttl < time(NULL)) {
			cache[i].inuse = 0;
		}
	}


	//LRU 机制
	if (cache_count <= CACHE_SIZE) {
		for (i = 0; i < cache_count; i++) {
			if (cache[i].inuse = 0) {
				memcpy(cache[i].url, url, sizeof(*url));
				memcpy(cache[i].ip, ip, sizeof(*ip));
				cache[i].intime = time(NULL);
				cache[i].inuse = 1;
				cache[i].ttl = ttl;
				return 1;
			}
		}
		memcpy(cache[cache_count].url, url, 256);
		memcpy(cache[cache_count].ip, ip, 16);
		cache[cache_count].intime = time(NULL);
		cache[cache_count].ttl = ttl;
		cache[cache_count++].inuse = 1;
		//printf("add url:%s ip:%s successfully", url, ip.c_str());
		return 1;
	}
	else {
		time_t mintime = 0xfffffff;
		int change = -1;
		for (i = 0; i < cache_count; i++) {
			if (mintime > cache[i].intime) {
				mintime = cache[i].intime;
				change = i;
			}
		}
		if (change != -1) {
			memcpy(cache[change].url, url, sizeof(*url));
			memcpy(cache[change].ip, ip, sizeof(*ip));
			cache[change].intime = time(NULL);
			cache[change].ttl = ttl;
		}
	}

	return 0;
}



int getIncache(char *dom) {
	int i;
	for (i = 0; i < cache_count; i++) {
		if (strcmp(cache[i].url,dom )==0&& cache[i].inuse) {
			return i;
		}
	}
	return -1;
}

void output_cache() {
	int i;
	for (i = 0; i < cache_count; i++)
		if (cache[i].inuse) {
			printf("\n======================\n");
			printf("%dth",i);
			printf(" [timestamp] :");
			char temp[64];
			strftime(temp, sizeof(temp), "add to cache at time: %Y/%m/%d %X %A", localtime(&cache[i].intime));
			printf("%s\n", temp);
    		printf("[url name] : %s | [ip address] :%s\n",cache[i].url,cache[i].ip);
			
			//printf("[url name] : %s | [ip address] : %s | [timestamp] :", cache[i].url, cache[i].ip.c_str());
			
		}

}

#endif //CACHE_H
