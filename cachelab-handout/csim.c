#define _GNU_SOURCE
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


typedef unsigned long long Address;

typedef struct CacheLine{
    short valid;
    Address tag;
    unsigned long long lu;
} CacheLine;

typedef struct CacheSet{
    int length;
    CacheLine *line;
} CacheSet;

CacheSet *cache;
int verbose = 1;
unsigned long long glu = 0;
int s, e, b;

CacheSet *initCache(int s, int e){
    int nset = 2 << (s-1);
    CacheSet *ptr = (CacheSet *)malloc((nset*sizeof(CacheSet)) + (nset*e*sizeof(CacheLine)));
    CacheLine *line_ptr = (CacheLine *)(ptr + nset);
    for(int i = 0; i < nset; i++){
	(ptr+i)->length = e;
	(ptr+i)->line = line_ptr;
	line_ptr += e;
	((ptr+i)->line)->valid = 0;
	((ptr+i)->line)->tag = 0;
    }
    return ptr;
}

// util functions to process the addresses

int addr2setidx(Address addr, int s, int b){
    int tagbits = 8*sizeof(addr) - s - b;
    addr = (addr << tagbits) >> tagbits;
    addr >>= b;
    return ((int)addr);
}

Address addr2tag(Address addr, int s, int b){
    return addr >> (b+s);
}

void handleLoad(int *miss, int *hit, int *envic, char *line){
    Address addr;
    unsigned int a;
    int length;
    sscanf(line, "%x,%d", &a, &length);
    addr = (Address) a;
    int setidx = addr2setidx(addr, s, b);
    printf("setidx is %d\n", setidx);
    Address tag = addr2tag(addr, s, b);
    CacheLine *cline = (cache + setidx) -> line;
    int lineidx = -1;
    int lru = -1;
    unsigned long long lu = 0xffffffff;
    for(int i = 0; i < (cache + setidx) -> length; i++){
	if(cline[i].valid == 0 && lineidx < 0){
	    lineidx = i;
	}else if(cline[i].valid > 0){
	    if(cline[i].lu < lu){
		lu = cline[i].lu;
		lru = i;
	    }
	    if(cline[i].tag == tag){
		if(verbose){
		    printf("%s\n", "Load hit!");
		}
		*hit += 1;
		cline[i].lu = ++glu;
		return;
	    }
	}
    }
    if(verbose){
	printf("%s\n", "Load miss!");
    }
    *miss += 1;
    if(lineidx < 0) {
	lineidx = lru;
	if(verbose)
	    printf("%s\n", "Enviction!");
	*envic += 1;
	/* printf("lineidx: %d\n", lineidx); */
	/* printf("envic: %d\n", *envic); */
    }
    cline[lineidx].valid = 1;
    cline[lineidx].tag = tag;
    cline[lineidx].lu = ++glu;
}
/* void handleStore(int *miss, int *hit, int *envic, char *line){} */
/* void handleModify(int *miss, int *hit, int *envic, char *line){} */

void processLine(int *miss, int *hit, int *envic, char *line){
    if(line[0] != ' '){
	return;
    }else{
	if(line[1] == '\0' || line[2] == '\0'){
	    return;
	}
	switch(line[1]){
	case 'L':
	    handleLoad(miss, hit, envic, line + 3);
	    break;
	case 'S':
	    handleLoad(miss, hit, envic, line + 3);
	    break;
	case 'M':
	    handleLoad(miss, hit, envic, line + 3);
	    handleLoad(miss, hit, envic, line + 3);
	    break;
	default:
	    return;
	}
    }
}


int main(int argc, char **argv)
{
    verbose = 0;
    // opt processing
    char *filename;
    int c;
    short result = 0;
    while((c = getopt(argc, argv, "vs:E:b:t:")) != -1){
	switch(c){
	case 'v':
	    verbose = 1;
	    break;
	case 's':
	    s = atoi(optarg);
	    result |= 0x01;
	    break;
	case 'E':
	    e = atoi(optarg);
	    result |= 0x02;
	    break;
	case 'b':
	    b = atoi(optarg);
	    result |= 0x04;
	    break;
	case 't':
	    filename = optarg;
	    result |= 0x08;
	    break;
	case '?':
	    printf("%s\n", "error in the args!");
	    return 1;
	}
    } 
    if(result != 0x0f){
	printf("%s \n", "Missing args!");
	return 1;
    }

    cache = initCache(s, e);
    // read valgrind file
    FILE *fp = fopen(filename, "r");
    char *l; size_t len = 0;
    if(fp == NULL){
	printf("%s\n", "file does not exist!");
	return 1;
    }
    int miss = 0, hit = 0, envic = 0;
    while(getline(&l, &len, fp) != -1){
	if(verbose)
	    printf("%s", l);
	processLine(&miss, &hit, &envic, l);
    }

    free(l);
    free(cache);
    printSummary(hit, miss, envic);
    return 0;
}
