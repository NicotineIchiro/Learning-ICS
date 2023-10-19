#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define CHECK_DOUBLE_NULL(a, b) (a == NULL || b == NULL)

static void attach_zero(char * s, size_t len) {
	s[len] = '\0';
}
size_t strlen(const char *s) {
	size_t end = 0;
	while (s[end] != '\0' && end < SIZE_MAX - 1) end++;
	if (end == SIZE_MAX - 1 && s[end] != '\0') {
		panic("over the max len");
		//return end;
	}
	return end;

  panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
	if (CHECK_DOUBLE_NULL(dst, src)) {
		panic("strcpy(): NULL arg!");
	}

	size_t lensrc = strlen(src);
	
	memmove(dst, src, lensrc);
	attach_zero(dst, lensrc);

	return dst;

  panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
	if (CHECK_DOUBLE_NULL(dst, src)) {
		panic("strncpy(): NULL arg!");
	}
	memmove(dst, src, n);
	attach_zero(dst, n);	

	return dst;
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
	size_t lendst = strlen(dst), lensrc = strlen(src);

	memmove(dst + lendst, src, lensrc);
	attach_zero(dst, lendst + lensrc);

	if (strlen(dst) != lensrc + lendst) {
		panic("strcat(): buffer len error.");
	}	

	return dst;

  panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
	if (CHECK_DOUBLE_NULL(s1, s2)) {
		panic("strcmp(): NULL arg!");
	}
	size_t len1 = strlen(s1), len2 = strlen(s2);
	size_t lenmin = len1 < len2 ? len1 : len2;

	int dif_val = memcmp(s1, s2, lenmin);
	return dif_val;

  panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
	if (CHECK_DOUBLE_NULL(s1, s2)) {
		panic("strncmp(): NULL arg!");
	}
	return memcmp(s1, s2, n);

  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
	if (s == NULL) {
		panic("memset(): NULL arg!");
	}
	for (size_t i = 0; i < n; ++i) {
		*((unsigned char *)s + i) = c;
	}	

	return s;

  panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
	//heap or stack?
	if (CHECK_DOUBLE_NULL(src, dst)) {
		panic("memmove(): NULL args!");
	}
	

	unsigned char temp[n], * _src = (unsigned char *) src, * _dst = (unsigned char *) dst;
	for (size_t i = 0; i < n; ++i)
		temp[i] = _src[i];

	for (size_t i = 0; i < n; ++i)
		_dst[i] = temp[i];

	return dst;
	
  panic("Not implemented");
}

static int overlap(const void * pa, const void *pb, size_t n) {
	return (pa < pb && pa + n > pb) || (pb < pa && pb + n > pa);
}
void *memcpy(void *out, const void *in, size_t n) {
	if (overlap(in, out, n)) {
		panic("Memory region overlap, please use the memmove().");
		//return NULL;
	}	
	unsigned char * _out = (unsigned char *)out, * _in = (unsigned char *)in;
	for (size_t i = 0; i < n; ++i) {
		_out[i] = _in[i];
	}

	return out;

  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
	size_t i = 0;

	while (i < n && *((unsigned char *)s1 + i) == *((unsigned char *)s2 + i)) ++i;

	return *((unsigned char *)s1 + i) - *((unsigned char *)s2 + i);

  panic("Not implemented");
}

#endif
