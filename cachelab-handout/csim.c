//p20150699 Jeong Tae Yeong

#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct LINE{
    int validbit;
    int tag;
    int callhist;
} line;

typedef struct SET{
    line* sline;
    int callcount;
} set;



int misscnt, hitcnt, deletecnt;
int s,e,b;//set bit, lines per set, block bit
char* trace;
int vflag;
set* cache;

void init(int argc, char* argv[])
{
    int i=0,setsize;
    for(i=0;i<argc-1;i++){
        if(!strcmp(argv[i],"-s")) s = (int)(argv[i+1][0]-'0');
        else if(!strcmp(argv[i],"-E")) e = (int)(argv[i+1][0]-'0');
        else if(!strcmp(argv[i],"-b")) b = (int)(argv[i+1][0]-'0');
        else if(!strcmp(argv[i],"-t")) trace = argv[i+1];
        else if(!strcmp(argv[i],"-v")) vflag=1;
    }
    setsize = 1<<s;
    cache = (set*)calloc(setsize,sizeof(set));
    for(i=0;i<setsize;i++)
    {
        cache[i].sline = (line*)calloc(e,sizeof(line));
    }
}

void callAddr(long long int addr, char type, int size)//chk 1:L, 2:S, 3:M
{
    int i, isValid = 0, isEmptyExist = 0;
    int LRUindex = 0;
    int setindex = 0;
    int tagvalue = 0;
    setindex = (addr>>b) - ((addr>>(b+s))<<s);
    tagvalue = addr>>(b+s);
    cache[setindex].callcount++;
    for(i=0; i<e; i++)
    {
        if(cache[setindex].sline[i].validbit == 0) continue;
        if(cache[setindex].sline[i].tag == tagvalue)
        {
            isValid = 1; cache[setindex].sline[i].callhist = cache[setindex].callcount;
            break;
        }
    }

    if(isValid == 1)
    {
        if('M' == type)
        {
            hitcnt += 2;
            if(vflag==1)
            {
                printf("%c %llx,%d hit hit\n",type,addr,size);
            }
        }
        else
        {
            hitcnt += 1;
            if(vflag==1)
            {
                printf("%c %llx,%d hit\n",type,addr,size);
            }
        }
    }
    else//miss 
    {
        for(i=0;i<e;i++)
        {
            if(cache[setindex].sline[i].validbit==0)
            {
                isEmptyExist = 1;
                cache[setindex].sline[i].validbit = 1;
                cache[setindex].sline[i].callhist = cache[setindex].callcount;
                cache[setindex].sline[i].tag      = tagvalue;
                if(type == 'M')
                {
                    misscnt++; hitcnt++;
                    if(vflag==1)printf("%c %llx,%d miss hit\n",type,addr,size);
                }
                else
                {
                    misscnt++;
                    if(vflag==1)printf("%c %llx,%d miss\n",type,addr,size);
                }
                break;
            }
        }
        if(isEmptyExist == 0)//change least recently used line
        {
            for(i=1;i<e;i++)
            {
                if(cache[setindex].sline[LRUindex].callhist > cache[setindex].sline[i].callhist) LRUindex = i;
            } 
            cache[setindex].sline[LRUindex].callhist = cache[setindex].callcount;
            cache[setindex].sline[LRUindex].tag      = tagvalue;
            if(type == 'M')
            {
                deletecnt++; hitcnt++; misscnt++;
                if(vflag==1)printf("%c %llx,%d miss eviction hit\n",type,addr,size);
            }
            else
            {
                deletecnt++; misscnt++;
                if(vflag==1)printf("%c %llx,%d miss eviction\n",type,addr,size);
            }
        }
    
    }
}

int main(int argc, char* argv[])
{
    long long int addr;
    char type;
    int querysize;
    FILE *fp;
    init(argc,argv);
    fp = fopen(trace,"r");if(fp==NULL) puts("file open error!");
    fscanf(fp,"%c %llx,%d\n",&type,&addr,&querysize);
    while(!feof(fp))
    {
        fscanf(fp,"%c %llx,%d\n",&type,&addr,&querysize);
        if(type == 'I') continue;
        callAddr(addr,type,querysize);
    }
    printSummary(hitcnt, misscnt, deletecnt);
    free(cache);
    fclose(fp);
    return 0;
}
