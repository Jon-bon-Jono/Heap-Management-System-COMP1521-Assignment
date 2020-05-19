# Heap-Management-System
My own implementation of malloc() and free().
Enables the user to manage a large region of memory as a heap, consisting of chunks of space that are either allocated or free

## int initHeap(int size)
Allocates a region of memory (heapMem) with N bytes. Sets heapMem to be all zeros.
Allocates an array of N/MIN_CHUNK pointers to free chunks of memory (freeList 
Returns 0 if successful else returns -1

## freeHeap()
Frees memory associated with heapMem and freeList

## dumpHeap()
Prints information about each chunk of allocated memory in this format:
  heapOffset(chunk), status(free/allocated): [F/A], chunk_size

## heapOffset(void *p)
Returns distance between p and heapMem

## void *myMalloc(int N)
Allocated a chunk of atleast N bytes
Scan freeList to find best-fit free chunk

## myFree(void *p)
Turns allocated chunk pointed to by p into a free chunk
Will merge adjacent free chunks

## Test 1
gcc myHeap.c test1.c -o test1
./test1 [heap size]
Inits a heap and then dumps it

## Test 2
gcc myHeap.c test2.c -o test2
./test2
Implements a sorted linked list using myHeap, inserts 20 random numbers and then dumps the whole heap

## Test 3
gcc myHeap.c test3.c -o test3
./test3 [heap size]
Allows for freeing and mallocing memory chunks under a label (letter)
Reads commands:
  [letter] = malloc [size]
  free [letter]
 
 ## Test 4
 gcc myHeap.c test4.c -o test4
 ./


