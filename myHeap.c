// Implementation of heap management system
// By Jonathan Williams
// May 2018
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myHeap.h"

// minimum total space for heap
#define MIN_HEAP  4096
// minimum amount of space for a free Chunk (excludes Header)
#define MIN_CHUNK 32

#define ALLOC     0x55555555 //1431655765
#define FREE      0xAAAAAAAA //-1431655766

typedef unsigned int uint;   // counters, bit-strings, ...

typedef void *Addr;          // addresses

typedef struct {             // headers for Chunks
   uint  status;             // status (ALLOC or FREE)
   uint  size;               // #bytes, including header
} Header;

static Addr  heapMem;        // space allocated for Heap
static int   heapSize;       // number of bytes in heapMem
static Addr *freeList;       // array of pointers to free chunks
static int   freeElems;      // number of elements in freeList[]
static int   nFree;          // number of free chunks

//inserts a (Header) pointer into the freeList, returns its index
static int freeListInsert(void *p); 

//given an index number, deletes corresponding element of freeList 
static void freeListDelete(int index);


// initialise heap
int initHeap(int size)
{
   //set size equal to minimum heap size if less than 4096
   //rounded size up to the nearest multiple of 4 otherwise
   if(size < 4096) size = 4096;
   else if(size % 4 != 0){
      size = size/4;
      size = 4*size + 4;
   }
   
   //malloc space for heapMem specified by size
   heapMem = malloc(size);
   if(heapMem == NULL) return -1;
   heapSize = size;
   
   //'zero out' region of memory pointed to by heapMem
   memset(heapMem, '0', size);
   
   //initialise region to a single free-space chunk
   freeList = malloc(size/32);
   if(freeList == NULL) return -1;
   
   //connect freeList[0] to heapMem 
   freeList[0] = (Header*)heapMem;
   Header* initialHeader = freeList[0];
   initialHeader->status = FREE;
   initialHeader->size = size;
   
   freeElems = size/MIN_CHUNK;
   nFree = 1;
   
   return 0;
}

// clean heap
void freeHeap()
{
   free(heapMem);
   free(freeList);
}

// allocate a chunk of memory
//finds smallested free chunk that can fit size bytes
void *myMalloc(int size)
{
   if(size < 0) return NULL;
   
   //rounded size up to the nearest multiple of 4
   if(size % 4 != 0){
      size = size/4;
      size = 4*size + 4;
   }
   
   int req_size = size + sizeof(Header);
   Header* curr_header;
   int i, min_index = -1, min_value = heapSize + 1;
   //find the smallest free chunk that is larger than req_size
   for(i = 0; i < nFree; i++){
      curr_header = freeList[i];
      if(curr_header->size < min_value && curr_header->size >= req_size){
         min_index = i;
         min_value = curr_header->size;
      }
   }
   
   //couldn't find a free chunk large enough to fit 
   if(min_index == -1){
      return NULL;
   }
   
   //If the free chunk is smaller than N+HeaderSize+MIN_CHUNK, allocate the whole chunk
   if(min_value < (req_size + MIN_CHUNK)){
      curr_header = freeList[min_index];
      //(size remains the same as the free's size)
      curr_header->status = ALLOC;
      //delete the reference to free block from freeList
      freeListDelete(min_index);
   }
   //If the free chunk is larger than N+HeaderSize+MIN_CHUNK, then split it into two chunks, with the lower chunk allocated for the request, and the upper chunk being a new free chunk.
   else{
      int original_free_size = min_value;
      curr_header = freeList[min_index];
      curr_header->size = req_size;
      curr_header->status = ALLOC;
      
      //offset element in freeList to then insert its new header into the appropriate spot in heapMem
      freeList[min_index] = freeList[min_index] + req_size;
      Header* new_free = freeList[min_index];
      new_free->size = original_free_size - req_size;
      new_free->status = FREE;
   }
   
   //return a pointer to the first usable byte of data
   return (char*)curr_header + sizeof(Header); 
}

// free a chunk of memory
void myFree(void *block)
{
   //check if our block points to an address in heapMem
   if(heapOffset(block) == -1){
      printf("Attempt to free unallocated chunk\n");
      exit(1);
   }
   Header* blockHeader = (void*)((char*)block - sizeof(Header));
   
   if(blockHeader->status != ALLOC){
      printf("Attempt to free unallocated chunk\n");
      exit(1);
   }
   
   //set our block header to FREE and insert it into the freeList
   blockHeader->status = FREE;
   int block_index = freeListInsert(blockHeader);
   
   
   //if any free blocks are found to be adjacent to our new block, merge the blocks:
   
   Header* prevFreeHeader = (block_index > 0)? freeList[block_index-1] : NULL;
   Header* nextFreeHeader = (block_index < (nFree-1) )? freeList[block_index+1] : NULL;
   
   //if prevFree exists and is adjacent to our block, merge
   //heapMem: |      F      |      F      |      A/F      |
   //freeList:| b_index - 1 |   b_index   |  b_index + 1  |<--(or not in the list)
   if(prevFreeHeader != NULL && (heapOffset(blockHeader) - heapOffset(prevFreeHeader) == prevFreeHeader->size)){
      prevFreeHeader->size += blockHeader->size;
      //delete freeList[block_index]
      freeListDelete(block_index);
      //if the upadated prevFree is now then adjacent to nextFree, merge
      //heapMem: |             F             |      F      |
      //freeList:|        b_index - 1        |   b_index   |
      if(nextFreeHeader != NULL && (heapOffset(nextFreeHeader) - heapOffset(prevFreeHeader) == prevFreeHeader->size)){
         prevFreeHeader->size += nextFreeHeader->size;
         //delete freeList[block_index]
         freeListDelete(block_index);
         //heapMem: |                    F                    |
         //freeList:|                b_index - 1              |
      }
   //else if prevFree is not adjacent to blockHeader, but nextFree is 
   //heapMem: |      A      |      F      |       F       |
   //freeList:| not in list |   b_index   |  b_index + 1  |
   }else if(nextFreeHeader != NULL && (heapOffset(nextFreeHeader) - heapOffset(blockHeader) == blockHeader->size)){
      blockHeader->size += nextFreeHeader->size;
      //delete freeList[block_index + 1]
      freeListDelete((block_index + 1));
      //heapMem: |      A      |             F               |
      //freeList:| not in list |          b_index            |
   }
}

// convert pointer to offset in heapMem
int  heapOffset(void *p)
{
   Addr heapTop = (Addr)((char *)heapMem + heapSize);
   if (p == NULL || p < heapMem || p >= heapTop)
      return -1;
   else
      return p - heapMem;
}

// dump contents of heap
void dumpHeap()
{
   Addr    curr;
   Header *chunk;
   Addr    endHeap = (Addr)((char *)heapMem + heapSize);
   int     onRow = 0;

   curr = heapMem;
   while (curr < endHeap) {
      char stat;
      chunk = (Header *)curr;
      switch (chunk->status) {
      case FREE:  stat = 'F'; break;
      case ALLOC: stat = 'A'; break;
      default:    fprintf(stderr,"Corrupted heap %08x\n",chunk->status); exit(1); break;
      }
      //if(stat != 'F')
      printf("+%05d (%c,%5d) ", heapOffset(curr), stat, chunk->size);
      onRow++;
     if (onRow%5 == 0) printf("\n");
      curr = (Addr)((char *)curr + chunk->size);
   }
   if (onRow > 0) printf("\n");
}

//inserts a (Header) pointer into the freeList, respecting the order the array
//returns the index of the inserted pointer

static int freeListInsert(void *p){
   int index;
   int i;
   for(i = 0; i <= nFree; i++){
      if(i == nFree){ //p belongs at the very end of the list
         freeList[i] = p;
         index = i;
         nFree++;
         return index;
      }
      if(heapOffset(freeList[i]) > heapOffset(p)) break; //we've found where p belongs
   }
   index = i;
   for(i = nFree; i > index; i--){ //shift everything right of index one space to the right
      freeList[i] = freeList[i-1];
   }
   freeList[index] = p;
   nFree++;
   return index;
}

//given an index number, deletes corresponding element of freeList 
static void freeListDelete(int index){
   int i;
   for(i = index; i < nFree - 1; i++){//shifts list right from index to overwrite it
      freeList[i] = freeList[i+1];
   }
   freeList[i] = NULL;
   nFree--;
}







