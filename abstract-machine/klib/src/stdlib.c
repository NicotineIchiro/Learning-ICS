#include <am.h>
#include <klib.h>
#include <klib-macros.h>
//#include <nemu.h>
#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}
static int malloc_cnt = 0;
extern Area heap;
////extern Area heap = RANGE(&_heap_start, PMEM_END);
//extern char _heap_start;
//static char * heap_ptr = &_heap_start;
static void * hbrk;
void malloc_reset() {
	//hbrk = (char *)ROUNDUP(heap.start, 8);
	hbrk = heap.start;
}
void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
	if (malloc_cnt == 0) {
		malloc_reset();
		malloc_cnt++;
	}

	size = ROUNDUP(size, 8);
	void * old = hbrk;
	hbrk += size;
	assert(heap.start <= hbrk && hbrk < heap.end);
	return old;
 //panic("Not implemented");
#endif
	return NULL;
}

void free(void *ptr) {
}

#endif
