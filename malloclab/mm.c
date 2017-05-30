/*
 *  mm.c written by team [BugOfDebugger]
 *  Managing free memory with double-linked explicit list
 *  
 *  each node of list has following form:
 *  [header + data section + footer]
 *  
 *  header and footer are same, they have information about size of data
 *  and whether data is allocated.
 *
 *  each freed memory is coalesced with neighborhood when it's freed.
 *  
 *  mm_malloc can allocate memory at already freed memory,
 *  and it uses First-fit search algorithm to find one.
 *  it returns pointer of data section.
 *
 *  least size of data section is (2 * sizeof(void *)), since
 *  it stores information about pointer of previous and next node
 *  and block is freed.
 *
 *  Free list is managed by two global pointer:root and tail
 *  use simple FILO method, so time complexity of insertion and deletion is constant.
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
    "BugOfDebugger",
    /* First member's full name */
    "Jeong TaeYeong",
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
#define SIZE_HEADER (ALIGN(sizeof(size_t)))

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
size_t maxFreed = 0;

inline void setFlag(void * ptr, int flag)
{
    size_t * target = (size_t*)((char *)ptr - SIZE_HEADER);//header
    *target += flag-(*target&1);
    target = (size_t*)((char *)ptr + getSize(ptr));//footer
    *target += flag-(*target&1);
}

inline int getFlag(void *ptr)
{
    if(ptr == NULL) return ALOC;
    return (*(size_t *)((char*)ptr - SIZE_HEADER))&1;
}

void setSize(void * ptr, size_t size)//possiblity of interrupting flag
{
    if(ptr == NULL) return;
    int flag = getFlag(ptr);
    (*(size_t *)((char *)ptr - SIZE_HEADER)) = size;
    (*(size_t *)((char *)ptr + size)) = size;
    setFlag(ptr, flag);
}


size_t getSize(void * ptr)
{
    if(ptr == NULL) return 0;
    size_t res = *(size_t *)((char *)ptr - SIZE_HEADER);
    res -= res&1;
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

void insertPtr(void * now)//insert now in free list
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

void coalesce(void *now)//now is addr alloacted by malloc, and must be freed
{
    if(now == NULL) return;
    size_t oldsize = 0, prevsize, nextsize, newsize;
    void *prev, *next;//front is pointer of front footer, back is next header
    
    prev = (void *)((char *)now - 2 * SIZE_HEADER);
    while(prev > mem_heap_lo() &&  (((*(size_t*)prev) &1) == FREE))
    {
        oldsize = getSize(now);
        prevsize = *(size_t*)prev; prevsize -= prevsize%2;
        
        deletePtr((void*)((char*)prev - prevsize));
        deletePtr(now);
        
        newsize = oldsize + prevsize + 2 * SIZE_HEADER;
        now = (void *)((char *)now - 2 * SIZE_HEADER - prevsize);
        setFlag(now, FREE); setSize(now, newsize); insertPtr(now);
        prev = (void *)((char *)now - 2 * SIZE_HEADER);
    }
    
    next = (void *)((char *)now + getSize(now) + SIZE_HEADER);
    while(next<mem_heap_hi() && ( ((*(size_t*)next)&1) == FREE))
    {
        oldsize = getSize(now);
        nextsize = *(size_t*)next; nextsize -= nextsize%2;
        
        deletePtr((void*)((char*)next + SIZE_HEADER));
        
        newsize = oldsize + nextsize + 2 * SIZE_HEADER;
        setFlag(now, FREE); setSize(now, newsize); insertPtr(now);
        next = (void *)((char *)now + getSize(now) + SIZE_HEADER);
    }
    if(maxFreed < getSize(now)) maxFreed = getSize(now);
}

void * findBlank(size_t size)
{
    void * now = root;
    void * best = NULL;
    if(now == NULL) return NULL;
    if(size > maxFreed) return NULL;
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
 *
 *
 *  --------------------------------------------------------------
 *  |header| p | ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|footer|
 *  --------------------------------------------------------------
 *                                |
 *                                |
 *                                V
 *  --------------------------------------------------------------
 *  |header| p | ~~~~~~~~~~|footer|header|rem|~~~~~~~~~~~~|footer|
 *  --------------------------------------------------------------
 *     
 *         |~~~~~~~~~~~~~~~~~oldsize~~~~~~~~~~~~~~~~~~~~~~|
 *         |~~~~~~size~~~~~|              |~~~~latsize~~~~|
 *
 *  
 */


void split(void * p, size_t size)//split one free node into two free node
{
    void * former = p;
    void * latter = NULL;
    size_t oldsize = getSize(p);
    size_t latsize = oldsize - 2 * SIZE_HEADER - size;
    latter = (void*)((char *)former + size + 2*SIZE_HEADER);
    setSize(former, size); setSize(latter,latsize);
    insertPtr(latter);
}

void printlist(void)//print free list, for debugging
{
    return;
    if(root == NULL){puts("list is empty"); return;}
    void * trace = root;
    printf("root %p tail %p\n",root,tail);
    while(trace != NULL)
    {
        if(getFlag(trace)!=FREE) {puts("flag err!!"); exit(1);}
        printf("%p(%d)-> ",trace,getSize(trace));
        trace = getNextPtr(trace);
    }
    puts("");return;

}

int mm_check(void)
{
    //check whether every node in free list marked as free
    void * trace = root;
    while(trace != NULL)
    {
        if(getFlag(trace) != FREE)
        {
            printf("%p is not marked as free.\n",trace);
            return 0;
        }
        trace = getNextPtr(trace);
    }
    //whether every node in free list is coalesced with neighbor
    //whether node in free list is pointing heap section
    trace = root;
    while(trace != NULL)
    {
        if( trace > mem_heap_hi() || trace < mem_heap_lo())
        {
            printf("%p is out of heap\n", trace);
            return 0;
        }
        if((void*)((char*)trace - 2*SIZE_HEADER) > mem_heap_lo())
        {
            if( ((*(size_t*)((char*)trace - 2*SIZE_HEADER))&1)==FREE)
            {
                printf("%p is not coalesced with neighbor\n",trace);
                return 0;
            }
        }
        if((void*)((char*)trace + getSize(trace) + SIZE_HEADER) < mem_heap_hi())
        {
            if( ((*(size_t*)((char*)trace +getSize(trace) + SIZE_HEADER))&1)==FREE)
            {
                printf("%p is not coalesced with neighbor\n",trace);
                return 0;
            }
        }
    }
    return 1;
}



/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    root = NULL;
    tail = NULL;
    maxFreed = 0;
    if(SIZE_HEADER == 4) mem_sbrk(4);
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    void * p = NULL;
    int i;
    size_t lastsize;
    int flag = 0;
    if(size < 2*sizeof(void*)) size = 2*sizeof(void*);
    for(i=5;i<10;i++)//for medium size, power of 2 is better
    {
        if(((1<<i) + (1<<(i+1))) < size && size <= (1<<(i+2))) {size = 1<<(i+2); break;}
    }
    size_t newsize = ALIGN(size + 2*SIZE_HEADER);
    p = findBlank(newsize - 2*SIZE_HEADER);
    if(p == NULL)//there's no freed memory with enough size
    {

        /*if(mem_heapsize() != 0)//if last block of heap is freed memory, use that
        {
            lastsize = *(size_t *)((char *)mem_heap_hi() + 1 - SIZE_HEADER);
            flag = lastsize & 1; lastsize -= flag;
            if(flag == FREE)
            {
                p = (void *)((char *)mem_heap_hi() +1 - SIZE_HEADER - lastsize);
                deletePtr(p);
                mem_sbrk(newsize - 2*SIZE_HEADER - getSize(p));
                setSize(p, newsize - 2 * SIZE_HEADER); setFlag(p, ALOC);
                return p;
            }
        }//pointer of last block*/
        p = mem_sbrk(newsize);//extend heap
        if (p == (void *)-1){
            puts(" NULL");
            return NULL;
        }
        else {
            *(size_t *)p = newsize - 2*SIZE_HEADER;
            setSize((void *)((char *)p + SIZE_HEADER), newsize - 2 * SIZE_HEADER);
            setFlag((void *)((char *)p + SIZE_HEADER), ALOC);
            return (void *)((char *)p + SIZE_HEADER);
        }
    }
    else
    {
        size_t psize = getSize(p);
        if(psize - (newsize-2*SIZE_HEADER) >= ALIGN(sizeof(void*)*2 + 2*SIZE_HEADER))//split when freed memory has extra size
        {
            split(p,newsize-2*SIZE_HEADER);
            deletePtr(p);
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
    if(ptr == NULL) return;
    setFlag(ptr, FREE);
    insertPtr(ptr);
    coalesce(ptr);
    return;
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void * newptr = NULL, *former=NULL, *latter=NULL;
    size_t oldsize = getSize(ptr);
    size_t newsize;
    size_t formersize = 0 , lattersize = 0;
    size = ALIGN(size);
    if(size <= oldsize) return ptr;
    else//if there are former or latter freed memory with enough size, extend current block to them
    {
        newptr = ptr;
        if(mem_heap_lo() < (void *)((char *)ptr - 2 * SIZE_HEADER))
            {formersize = *(size_t *)((char *)ptr - 2 * SIZE_HEADER);}
        if(mem_heap_hi() > (void *)((char *)ptr + oldsize + SIZE_HEADER))
            {lattersize = *(size_t *)((char *)ptr + oldsize + SIZE_HEADER);}
        if((formersize&1) == ALOC) formersize = 0;
        else formersize -= 1;
        if((lattersize&1) == ALOC) lattersize = 0;
        else lattersize -= 1;
        
        if(formersize > 0) former = (void*)((char*)ptr - 2*SIZE_HEADER - formersize);
        if(lattersize > 0) latter = (void *)((char*)ptr + getSize(newptr) + 2*SIZE_HEADER);
        
        newsize = oldsize + formersize + lattersize;
        if(formersize > 0 ) newsize += 2 * SIZE_HEADER;
        if(lattersize > 0 ) newsize += 2 * SIZE_HEADER;
        if(newsize >= size)
        {
            if(formersize > 0)
            {
                deletePtr(former);
                newptr = former;
                setSize(newptr, oldsize + 2 * SIZE_HEADER + formersize); 
            }
            if(lattersize > 0)
            {
                deletePtr(latter);
                setSize(newptr, getSize(newptr) + 2*SIZE_HEADER + lattersize);
            }
            //memcpy(newptr, ptr, oldsize);
            for(int i=0;i<oldsize;i++){
                *(char *)(newptr + i) = *(char *)(ptr + i);
            }
            setFlag(newptr, ALOC);
            return newptr;
        }
        else//when no freed memory with enough size near ptr
        {
            newptr = mm_malloc(size);
            //memcpy(newptr, ptr, oldsize);
            for(int i=0;i<oldsize;i++){
                *(char *)(newptr + i) = *(char *)(ptr + i);
            }
            mm_free(ptr);
            setFlag(newptr, ALOC);
            return newptr;
        }
    }
}
