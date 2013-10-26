#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>


typedef unsigned long long Address;

typedef struct CacheLine{
    short valid;
    Address tag;
} CacheLine;

typedef struct CacheSet{
    int length;
    CacheLine *line;
} CacheSet;

CacheSet *initCache(int s, int e){
    int nset = 2 << (s-1);
    CacheSet *ptr = (CacheSet *)malloc(nset*sizeof(CacheSet));
    for(int i = 0; i < nset; i++){
	(ptr+i)->length = e;
	(ptr+i)->line = (CacheLine *)realloc(e*sizeof(CacheLine));
	((ptr+i)->line)->valid = 0;
	((ptr+i)->line)->tag = 0;
    }
    return ptr;
}

// util functions to process the addresses

int addr2setidx(Address addr, int s, int b){
    addr >>= b;
    int tagbits = sizeof(addr) - s - b;
    addr = (addr << tagbits) >> tagbits;
    return (int)addr;
}

Address addr2tag(Address addr, int s, int b){
    return addr >> (b+s);
}



int main()
{
    // init cache
    // CacheSet *cache = initCache(4, 1);
    
    /* for(int i = 0; i < (2<<3); i++){ */
    /* 	if(cache[i].line->valid != 0){ */
    /* 	    printf("crap!"); */
    /* 	} */
    /* } */
    // printf("%lx ", (long unsigned int)addr2tag(0xefef1234, 15, 8));
    
    // read valgrind file
    
    printSummary(0, 0, 0);
    return 0;
}
