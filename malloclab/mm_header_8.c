/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Segmentation fault1",
    /* First member's full name */
    "Jeong Tae Yeong",
    /* First member's email address */
    "wwiiiii@kaist.ac.kr",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define FREE (1)
#define ALOC (0)
#define UI (unsigned int)
/*	
 *	manage freed memory by using double linked list
 *	Explicit free list 
 *	[header + prevptr + nextptr + data section + footer] 
 *	return value of malloc() is address of data section
 *	use Address-ordered policy for efficient merge
 */

/*
 *	list of necessary function
 *	1. split : when malloc function allocates memory to freed section,
 *		   it should be splited into assigned section and rest freed section
 *	2. coalsece : when free function clear allocated memory before,
 *		   it should be coalesced with neighbor freed memory
 */ 

/*
 * ---------------------------------------------------------
 * |header|prev|next|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|footer|
 * ---------------------------------------------------------
 *      (ptr)       (           data section        )
 *
 */
size_t getSize(void *);
void printlist(void);
void * root = NULL;
void * tail = NULL;

void setFlag(void * ptr, int flag)
{
    size_t * target = (size_t*)((char *)ptr - SIZE_T_SIZE);//header
    if(((*target)&1) == 0) *target += flag;
    else *target -= (!flag);
    target = (size_t*)((char *)ptr + getSize(ptr));//footer
    if(((*target)&1) == 0) *target += flag;
    else *target -= (!flag);
    if ((*(size_t*)((char *)ptr - SIZE_T_SIZE)) != (*(size_t*)((char *)ptr + getSize(ptr)))){puts("flag error"); exit(1);}
}

int getFlag(void *ptr)
{
    if(ptr == NULL) return ALOC;
    return (*(size_t *)((char*)ptr - SIZE_T_SIZE))&1;
}

void setSize(void * ptr, size_t size)//possiblity of interrupting flag
{
    if(ptr == NULL) return;
    int flag = getFlag(ptr);
    (*(size_t *)((char *)ptr - SIZE_T_SIZE)) = size;
    (*(size_t *)((char *)ptr + size)) = size;
    setFlag(ptr, flag);
}


size_t getSize(void * ptr)
{
    if(ptr == NULL) return 0;
    size_t res = *(size_t *)((char *)ptr - SIZE_T_SIZE);
    res -= res%2;
    return res;
}

void * getPrevPtr(void * ptr)//ptr is addr allocated by malloc
{
    if(ptr == NULL)return NULL;
    return *(void**)ptr;
}

void * getNextPtr(void * ptr)
{
    if(ptr == NULL) return NULL;
    return *(void**)((char*)ptr + sizeof(void*));
}

void setPrevPtr(void *now, void * prev)//now is addr allocated by malloc
{
    if(now == NULL) return;
    if(now == prev){
        return;
    }
    *(void**)now = prev;
}

void setNextPtr(void *now, void * next)//now is addr allocated by malloc
{
    if(now == NULL) return;
    if(now == next){
        return;
    }
    *(void**)((char *)now + sizeof(void *)) = next;
}

void insertPtr(void * now)
{
    if(now == NULL) return;
    if(root == NULL){
        tail = root = now;
        setPrevPtr(now, NULL);
        setNextPtr(now, NULL);
        setFlag(now, FREE);
        return;
    }
    setNextPtr(tail, now);
    setPrevPtr(now, tail);
    setNextPtr(now , NULL);
    setFlag(now, FREE);
    tail = now;
}

void deletePtr(void * now)//delete from free list
{
    if(now == NULL) return;
    if(now == root)
    {
        root = getNextPtr(now);
    }
    if(now == tail)
    {
        tail = getPrevPtr(now);
    }
    setNextPtr(getPrevPtr(now),getNextPtr(now));
    setPrevPtr(getNextPtr(now),getPrevPtr(now));
    setFlag(now, ALOC);
}

//should be changed
void coalesce(void *now)//now is addr alloacted by malloc, and must be freed
{
    if(now == NULL) return;
    size_t oldsize = 0, prevsize, nextsize, newsize;
    void *prev, *next;//front is pointer of front footer, back is next header
    printlist();
    prev = (void *)((char *)now - 2 * SIZE_T_SIZE);
    while(prev > mem_heap_lo() &&  (((*(size_t*)prev) &1) == FREE))
    {
        oldsize = getSize(now);
        prevsize = *(size_t*)prev; prevsize -= prevsize%2;
     //   printf("old size %d, prev size %d, now %x, prev %x\n",oldsize,prevsize,UI(now),UI(((void*)((char*)prev - prevsize))));
        deletePtr((void*)((char*)prev - prevsize));
        deletePtr(now);
        newsize = oldsize + prevsize + 2 * SIZE_T_SIZE;
        now = (void *)((char *)now - 2 * SIZE_T_SIZE - prevsize);
        setFlag(now, FREE); setSize(now, newsize); insertPtr(now);
        prev = (void *)((char *)now - 2 * SIZE_T_SIZE);
    }
    next = (void *)((char *)now + getSize(now) + SIZE_T_SIZE);
    while(next<mem_heap_hi() && ( ((*(size_t*)next)&1) == FREE))
    {
        oldsize = getSize(now);
        nextsize = *(size_t*)next; nextsize -= nextsize%2;
       // printf("old size %d, next size %d, now %x, next %x\n",oldsize,nextsize,UI(now),UI(((char*)next + SIZE_T_SIZE)));
        deletePtr((void*)((char*)next + SIZE_T_SIZE));
        newsize = oldsize + nextsize + 2 * SIZE_T_SIZE;
        setFlag(now, FREE); setSize(now, newsize); insertPtr(now);
        next = (void *)((char *)now + getSize(now) + SIZE_T_SIZE);
    }
    printlist();
}

void * findBlank(size_t size)
{
    void * now = root;
    void * best = NULL;
    if(now == NULL) return NULL;
    while(now!=NULL)
    {
        if(getSize(now) >= size)
        {
            if(best == NULL)
            {
                best = now;
                return best;
            } 
            else
            {
                if(getSize(now) < getSize(best)) best = now;
            }
        }
        now = getNextPtr(now);
    }
    return best;
}

/*
 *  ----------------------------------------
 *  |size| p | ~~~~~~~~~~|size| rem|~~~~~~~|
 *  ----------------------------------------
 *     
 *       |~~~~~~~~~~~~~~psize~~~~~~~~~~~~~~|
 *       |~~~~~~~~size~~~|    |~~extrasize~|
 *
 *  
 */


void split(void * p, size_t size)
{
}

void printlist(void)
{
    return;
    if(root == NULL){puts("list is empty"); return;}
    void * trace = root;
    printf("root %x tail %x\n",UI(root),UI(tail));
    while(trace != NULL)
    {
        if(getFlag(trace)!=FREE) {puts("flag err!!"); exit(1);}
        printf("%x(%d)-> ",UI(trace),getSize(trace));
        trace = getNextPtr(trace);
    }
    puts("");return;

}



/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    root = NULL;
    tail = NULL;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void * p = NULL;
    if(size < 2*sizeof(void*)) size = 2*sizeof(void*);
    size_t newsize = ALIGN(size + 2*SIZE_T_SIZE);
    //printf("malloc %d ", newsize - 2*SIZE_T_SIZE);
    p = findBlank(newsize - 2*SIZE_T_SIZE);
    if(p == NULL)
    {
        p = mem_sbrk(newsize);
        if (p == (void *)-1){
            puts(" NULL");
            return NULL;
        }
        else {
            *(size_t *)p = newsize - 2*SIZE_T_SIZE;
       //     printf("%x allocated\n",UI((void *)((char *)p + SIZE_T_SIZE)));
       //     printf("with size %d\n",getSize((void*)((char *)p + SIZE_T_SIZE)));
            setSize((void *)((char *)p + SIZE_T_SIZE), newsize - 2 * SIZE_T_SIZE);
            setFlag((void *)((char *)p + SIZE_T_SIZE), ALOC);
            return (void *)((char *)p + SIZE_T_SIZE);
        }
    }
    else
    {
        size_t psize = getSize(p);
        if(psize - (newsize-2*SIZE_T_SIZE) >= ALIGN(sizeof(void*)*2 + 2*SIZE_T_SIZE))
        {
            //split(p,newsize-2*SIZE_T_SIZE);
            deletePtr(p);
       //     printf("%x allocated-\n",UI(p));
            return p;
        }
        else 
        {
            deletePtr(p);
       //     printf("%x allocated--\n",UI(p));
            return p;
        }
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if(ptr == NULL) return;
   // printf("%x freed\n",UI(ptr));
    setFlag(ptr, FREE);
    insertPtr(ptr);
    coalesce(ptr);
   // puts("free end\n");
    return;
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    int oldsize = getSize(ptr);
    size = ALIGN(size);
    if(size <= oldsize) return ptr;
    void * newptr = mm_malloc(size);
    memcpy(newptr, ptr, oldsize);
    mm_free(ptr);
    return newptr;
}
