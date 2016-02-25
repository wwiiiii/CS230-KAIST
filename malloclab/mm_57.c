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


/*	
 *	manage freed memory by using double linked list
 *	Explicit free list 
 *	[size of data section + prevptr + nextptr + data section] 
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

void * root = NULL;

inline void setSize(void * ptr, size_t size)
{
    if(ptr == NULL) return;
    (*(size_t *)((char *)ptr - SIZE_T_SIZE)) = size;
}


inline size_t getSize(void * ptr)
{
    if(ptr == NULL) return 0;
    return *(size_t *)((char *)ptr - SIZE_T_SIZE);
}

void * getPrevPtr(void * ptr)//ptr is addr allocated by malloc
{
    if(ptr == NULL)return NULL;
    return *(void**)ptr;
}

void * getNextPtr(void * ptr)
{
    if(ptr == NULL) return NULL;
    return *(void**)((void*)((char*)ptr + sizeof(void*)));
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
        printf("%x",(unsigned int) now);
        return;
    }
    *(void**)((void *)((char *)now + sizeof(void *))) = next;
}

void deletePtr(void * now)
{
    if(now == NULL) return;
    if(now == root)
    {
        root = getNextPtr(now);
    }
    setNextPtr(getPrevPtr(now),getNextPtr(now));
    setPrevPtr(getNextPtr(now),getPrevPtr(now));
}


void coalesce(void *now)//now is addr alloacted by malloc, and must be freed
{
    if(now == NULL) return;
    int nowsize = getSize(now);
    int extrasize;
    void * prev = getPrevPtr(now);
    void * next = getNextPtr(now);
    while(1)//coalesce with next freed memory
    {
        next = getNextPtr(now);
        if(next == NULL) break;
        if(!(((char*)now + nowsize + SIZE_T_SIZE) == (char*)next)) break;
		nowsize = extrasize = 0;
		if(now!=NULL) nowsize = getSize(now);
        if(next!=NULL) extrasize = getSize(next);
        setPrevPtr(getNextPtr(next),now);
        setNextPtr(now,getNextPtr(next));
        if(now!=NULL)setSize(now, nowsize + extrasize + SIZE_T_SIZE);
    }
    while(1)
    {
        prev = getPrevPtr(now);
        if(prev == NULL) break;
        if(!(((char *)prev + getSize(prev) + SIZE_T_SIZE) == (char *)now)) break;
		nowsize = extrasize = 0;
		if (now != NULL)nowsize = getSize(now);
		if (prev != NULL)extrasize = getSize(prev);
        setNextPtr(prev,getNextPtr(now));
        setPrevPtr(getNextPtr(now),prev);
        if(now!=NULL)setSize(prev, nowsize + extrasize + SIZE_T_SIZE);
        now = prev;
    }
    return;
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
	if (p == NULL) return;
	size_t psize = getSize(p);
    size_t extrasize = psize - size - SIZE_T_SIZE;
    void * rem = (void*)((char*)p + size + SIZE_T_SIZE);
    if(p == root)
    {
        root = rem;
    }
    setPrevPtr(rem, getPrevPtr(p));
    setNextPtr(rem, getNextPtr(p));

    setNextPtr(getPrevPtr(p), rem);
    setPrevPtr(getNextPtr(p), rem);
    setSize(p,size);
    setSize(rem, extrasize);
    return;
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    root = NULL;
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
    size_t newsize = ALIGN(size + SIZE_T_SIZE);
    p = findBlank(newsize - SIZE_T_SIZE);
    if(p == NULL)
    {
        p = mem_sbrk(newsize);
        if (p == (void *)-1){
            puts(" NULL");
            return NULL;
        }
        else {
            *(size_t *)p = newsize - SIZE_T_SIZE;
    	    return (void *)((char *)p + SIZE_T_SIZE);
        }
    }
    else
    {
        size_t psize = getSize(p);
        if(psize - (newsize-SIZE_T_SIZE) >= ALIGN(sizeof(void*)*2 + SIZE_T_SIZE))
        {
            split(p,newsize-SIZE_T_SIZE);
            return p;
        }
        else 
        {
            deletePtr(p);
            return p;
        }
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    void *last = NULL;
    void *temp = NULL;
    if(ptr > mem_heap_hi()) return;
	if(root == ptr)
    {
        puts("free error");        return;
    }
    
    if(root == NULL)
	{
		root = ptr;
		setNextPtr(ptr, NULL);
		setPrevPtr(ptr, NULL);
	}	
	else
	{	
        if(ptr < root)
        {
            temp = root;
            setNextPtr(ptr, temp);
            setPrevPtr(ptr, NULL);
            setPrevPtr(temp, ptr);
            root = ptr;
        }
        else
        {
            last = root;
            while(( getNextPtr(last) < ptr) && (getNextPtr(last) != NULL))
    		{
			    last = getNextPtr(last);
	     	}
             temp = getNextPtr(last);
             if(temp == ptr){
                puts("free err");         return;
             }
	    	setNextPtr(last, ptr);
            setNextPtr(ptr, temp);
		    setPrevPtr(ptr, last);
		    setPrevPtr(temp, ptr);
        }
	}
    coalesce(ptr);
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size = ALIGN(size);
    void *oldptr = ptr;
    int frontblank = 0, backblank = 0;
    void *front = NULL, *back = NULL, *newptr;
    void *rem, *trace;
    const size_t oldsize = getSize(ptr);
    if(((int)oldsize -(int)size) >= (int)(ALIGN(SIZE_T_SIZE + 2*sizeof(void))))
    {
        rem = (void *)((char *)ptr + size + SIZE_T_SIZE);
        setSize(ptr, size);
        setSize(rem, oldsize - size - SIZE_T_SIZE);
        mm_free(rem);    
        return ptr;
    }
    else if(oldsize >= size) return ptr;
    else
    {
        rem = (void *)((char *)ptr + oldsize +SIZE_T_SIZE);
        trace = root; 
        while(trace!=NULL)
        {
            if(trace == rem)
            {
                backblank = getSize(trace) + SIZE_T_SIZE; 
                back = trace;
                break;
            }
            trace = getNextPtr(trace);
        }
        trace = root;
        while(trace!=NULL)
        {
            if((void*)((char*)trace + getSize(trace) + SIZE_T_SIZE) == ptr)
            {
                frontblank = getSize(trace) + SIZE_T_SIZE;
                front = trace;
                break;
            }
            trace = getNextPtr(trace);
        }
        if(oldsize + frontblank + backblank >= size)
        {
            //int i=0;
            if(frontblank>0)ptr = (void*)((char *)oldptr - (frontblank - SIZE_T_SIZE));
            deletePtr(front); deletePtr(back);
            setSize(ptr, oldsize + frontblank + backblank);
            memcpy(ptr, oldptr, oldsize);
            //for(i=0;i<oldsize;i++) *(char*)(ptr+i) = *(char*)(oldptr+i);
            return ptr;
        }
        else
        {
            newptr = mm_malloc(size);
            if(newptr == NULL) return NULL;
            memcpy(newptr, oldptr, oldsize);
            mm_free(oldptr);
            return newptr;
        }
    } 
}
