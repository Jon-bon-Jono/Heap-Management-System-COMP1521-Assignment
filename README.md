# Heap-Management-System
My own implementation of malloc() and free().
Enables the user to manage a large region of memory as a heap, consisting of chunks of space that are either allocated or free

# int initHeap(int size)
Allocates a region of memory (heapMem) with N bytes. Sets heapMem to be all zeros.
Allocates an array of N/MIN_CHUNK pointers to free chunks of memory (freeList 
Returns 0 if successful else returns -1

# freeHeap()
Frees memory associated with heapMem and freeList

# dumpHeap()
Prints information about each chunk of allocated memory in this format:
  heapOffset(chunk), status(free/allocated): [F/A], chunk_size

# heapOffset(void *p)
Returns distance between p and heapMem

# void *myMalloc(int N)
Allocated a chunk of atleast N bytes
Scan freeList to find best-fit free chunk

# myFree(void *p)
Turns allocated chunk pointed to by p into a free chunk
Will merge adjacent free chunks

