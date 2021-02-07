#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<getopt.h>
#include<string.h>
#include<malloc.h>
#include "cachelab.h"

int hit_count = 0; 
int miss_count = 0;
int eviction_count = 0;
FILE* p_file = NULL;
typedef unsigned long long address;
const int INF = 0x3f3f3f3f;
int s;
unsigned long long S;
int E;
int b;
char cmd;
address addr;
size_t size;
char t[1000];

typedef enum boolean {false,true } boolean;
typedef struct  cache_line{
    enum boolean valid_bit;
    address tag;
    int lru_counter;
}cache_line;
cache_line** cache = NULL;

void cache_init(void);
void load_store_op(address addr);
void modify_op(address addr);
void get_tag_and_s_index(address addr,address* tag,address* s_index);
int lru(cache_line* p_cl);

int main(int argc, char* argv[])
{
    int opt;
    while(-1 != (opt = getopt(argc, argv, "s:E:b:t:"))){
        switch(opt){
            case 's': s = atoi(optarg); break;
            case 'E': E = atoi(optarg); break;
            case 'b': b = atoi(optarg); break;
            case 't': strcpy(t,optarg); break;
            default:{
                printf("./csim: Missing required command line argument.\n");
                break;
            }
        }
    }
    S = (1 << s);
    cache_init();
    p_file = fopen(t,"r");
    while(fscanf(p_file," %c %llx,%ld",&cmd,&addr,&size) != EOF){
        if(cmd == 'I'){
            continue;
        };
        printf("%c %llx,%ld ",cmd,addr,size);
        switch(cmd){
            case 'L': 
            case 'S': load_store_op(addr); break;
            case 'M': modify_op(addr); break;
            default:{
                printf("Instruction Error.\nProgram exit.\n");
                exit(-1);
            }
        }
        printf("\n");
    }
    printSummary(hit_count,miss_count, eviction_count);
    for(int i = 0; i < S; ++i){
        free(cache[i]);
    }
    free(cache);
    return 0;
}

void cache_init(void)
{
    cache = (cache_line**)malloc(S*sizeof(cache_line*));
    if(!cache){
        printf("Malloc failed.\nProgram exit.\n");
        exit(-1);
    }
    for(int i = 0; i < S; ++i){
        cache[i] = (cache_line*)malloc(E*sizeof(cache_line));
        if(!cache[i]){
            printf("Malloc failed.\nProgram exit in line 89.\n");
            exit(-1);
        }
        for(int j = 0; j < E; ++j){
            cache[i][j].valid_bit = false;
            cache[i][j].lru_counter = 0;
            cache[i][j].tag = 0;
        }
    }
    return ;
}

void load_store_op(address addr)
{
    address tag;
    address s_index;
    boolean get_hits = false;
    boolean need_lru = true;
    int empty_line = -1;
    get_tag_and_s_index(addr,&tag,&s_index);
    for(int i = 0; i < E; ++i){
        if(cache[s_index][i].tag == tag && cache[s_index][i].valid_bit == true){
            get_hits = true;
            cache[s_index][i].lru_counter = INF;
            for(int j = 0;j < E; ++j){
                if(j != i && cache[s_index][j].valid_bit == true){
                    --cache[s_index][j].lru_counter;
                }
            }
            break;
        }
        if(need_lru == true && cache[s_index][i].valid_bit == false){
            need_lru = false;
            empty_line = i;
        }
    }
    if(get_hits == true){
        ++hit_count;
        printf("hit ");
    }else if(need_lru == false){
        printf("miss ");
        ++miss_count;
        cache[s_index][empty_line].tag = tag;
        cache[s_index][empty_line].valid_bit = true; 
        cache[s_index][empty_line].lru_counter = INF;
        for(int j = 0;j < E; ++j){
            if(j != empty_line && cache[s_index][j].valid_bit == true){
                --cache[s_index][j].lru_counter;
            }
        }
    }else{
        printf("miss eviction");
        ++miss_count;
        ++eviction_count;
        int index = lru(cache[s_index]);
        cache[s_index][index].tag = tag;
        cache[s_index][index].lru_counter = INF;
        for(int j = 0;j < E; ++j){
            if(j != index && cache[s_index][j].valid_bit == true){
                --cache[s_index][j].lru_counter;
            }
        }
    }
    return ;
}

void get_tag_and_s_index(address addr,address* tag,address* s_index)
{
    address t = 64 - (b + s);
    *s_index = ((addr << t) >> (b + t));
    *tag = (addr >> (b + s));
    return ;
}

int lru(cache_line* p_cl)
{
    int index = 0;
    int min = INF;
    for(int i = 0; i < E; ++i){
        if(p_cl[i].lru_counter < min){
            index = i;
            min = p_cl[i].lru_counter;
        }
    }
    return index;
}

void modify_op(address addr)
{
    load_store_op(addr);
    load_store_op(addr);
    return ;
}