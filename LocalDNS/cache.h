#pragma once
#ifndef CACHE_H
#define CACHE_H
#define _CRT_SECURE_NO_WARNINGS
#include <time.h>
#include <string>
#include <iostream>

#define CACHE_SIZE 30
#define TTL 10000
#pragma warning(disable: 4996)
using namespace std;

//增加cache功能
typedef struct {
	string url;
	string ip;
	time_t intime;
	unsigned long ttl;
	bool inuse;
}cache_entity;

int cache_count = 0;
cache_entity cache[CACHE_SIZE];

void init_cache() {
	for (int i = 0; i < CACHE_SIZE; i++) {
		cache[i].inuse = false;
		cache[i].ttl = TTL;
	}
	return;
}

bool add_to_cache(string url, string ip, unsigned long ttl) {
	for (int i = 0; i < cache_count; i++) {
		if (cache[i].url == url && cache[i].inuse == true) {
			cache[i].intime = time(NULL);
			cache[i].ttl = ttl;
			if (cache[i].ip == ip) {
				//printf("this url:%s already exits", url);
				return true;
			}
			else {
				cache[i].ip = ip;
				//printf("update domain  %s's ip address to %s", url, ip.c_str());
				return true;
			}
		}
	}


	//超时机制
	for (int i = 0; i < CACHE_SIZE; i++) {
		if (cache[i].intime + cache[i].ttl < time(NULL)) {
			cache[i].inuse = false;
		}
	}


	//LRU 机制
	if (cache_count < CACHE_SIZE) {
		for (int i = 0; i < cache_count; i++) {
			if (cache[i].inuse = false) {
				cache[i].url = url;
				cache[i].ip = ip;
				cache[i].intime = time(NULL);
				cache[i].inuse = true;
				cache[i].ttl = ttl;
				return true;
			}
		}
		cache[cache_count].url = url;
		cache[cache_count].ip = ip;
		cache[cache_count].intime = time(NULL);
		cache[cache_count].ttl = ttl;
		cache[cache_count++].inuse = true;
		//printf("add url:%s ip:%s successfully", url, ip.c_str());
		return true;
	}
	else {
		time_t maxtime = 0;
		int change = -1;
		for (int i = 0; i < cache_count; i++) {
			if (maxtime < cache[i].intime) {
				maxtime = cache[i].intime;
				change = i;
			}
		}
		if (change != -1) {
			cache[change].url = url;
			cache[change].ip = ip;
			cache[change].intime = time(NULL);
			cache[change].ttl = ttl;
		}
	}

	return false;
}



int getIncache(string dom) {
	for (int i = 0; i < cache_count; i++) {
		if (cache[i].url == dom && cache[i].inuse) {
			return i;
		}
	}
	return -1;
}

void output_cache() {
	for (int i = 0; i < cache_count; i++)
		if (cache[i].inuse) {
			printf("======================\n");
			cout << i << "th" << endl;
			cout << " [timestamp] :";
			char temp[64];
			strftime(temp, sizeof(temp), "add to cache at time: %Y/%m/%d %X %A", localtime(&cache[i].intime));
			printf("%s\n", temp);
			cout << "[url name] : " << cache[i].url << " | [ip address] :" << cache[i].ip << endl;
			
			//printf("[url name] : %s | [ip address] : %s | [timestamp] :", cache[i].url, cache[i].ip.c_str());
			
		}

}

#endif //CACHE_H