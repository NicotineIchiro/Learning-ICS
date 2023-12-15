#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
size_t i2str(int, char *);
int printf(const char *fmt, ...)
{
		if (fmt == NULL) {
		panic("NULL fmt!");
	}
	// should the format use regex?
	va_list ap;
	int temp = 0;
	char *temps;
	size_t i = 0;
	int cnt = 0;
	int errf = 0;

	va_start(ap, fmt);
		//size_t flen = strlen(fmt);

	while (fmt[i] != '\0')//i in s0, flen in s3.
	{
		if (fmt[i] != '%')
		{
			//out[cnt] = fmt[i];
			putch(fmt[i]);
			++i;
			++cnt;//cnt in s1
		}
		else
		{
			switch (fmt[i + 1])
			{
			case 'c':
				char tc;
				tc = (char)va_arg(ap, int);
				putch(tc);
				i += 2;
				cnt++;
			case 'd':
				temp = va_arg(ap, int);//the va_arg for %d is int!
				char i2s[64] = {0};
				size_t test_len = i2str(temp, i2s);
				size_t islen = strlen(i2s);
				if (test_len != islen)
					panic("i2s bug!");
				//strncpy(out + cnt, i2s, islen);
				for (size_t i = 0; i < islen; ++i) {
					putch(i2s[i]);
				}
				i += 2;
				cnt += islen;
				break;
			case 's':
				temps = va_arg(ap, char *);
				size_t slen = strlen(temps);
				//strncpy(out + cnt, temps, slen);
				for (size_t i = 0; i < slen; ++i) {
					putch(temps[i]);
				}
				i += 2;
				cnt += slen;
				break;
			default:
				errf = 1;
				break;
			}
		}
		if (errf == 1)
		{
			panic("Invalid format.");
			return -1;
		}
	}
	
	//out[cnt] = '\0';
	va_end(ap);

	return cnt;//char *temp;

	panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap)
{
	panic("Not implemented");
}
size_t i2str(int n, char *str)
{
	size_t i;
	char tc;
	i = 0;
	
	while (n != 0) { 
		str[i] = n % 10 + '0';
		n /= 10;
		++i;
	}

	for (size_t j = 0; j < i / 2; ++j) {
																			//使之与数字顺序一致
		tc = str[j];
		str[j] = str[i - j - 1];
		str[i - j - 1] = tc;
	}
	str[i] = '\0';
	return i;
}
int sprintf(char *out, const char *fmt, ...)
{
	if (fmt == NULL) {
		panic("NULL fmt!");
	}
	// should the format use regex?
	va_list ap;
	int temp = 0;
	char *temps;
	size_t i;
	i = 0;
	int cnt;
	cnt = 0;
	int errf;
	errf = 0;

	va_start(ap, fmt);
		//size_t flen = strlen(fmt);

	while (fmt[i] != '\0')//i in s0, flen in s3.
	{
		if (fmt[i] != '%')
		{
			out[cnt] = fmt[i];
			++i;
			++cnt;//cnt in s1
		}
		else
		{
			switch (fmt[i + 1])
			{
			case 'd':
				temp = va_arg(ap, int);//the va_arg for %d is int!
				char i2s[64] = {0};
				size_t test_len = i2str(temp, i2s);
				size_t islen = strlen(i2s);
				if (test_len != islen)
					panic("i2s bug!");
				strncpy(out + cnt, i2s, islen);
				i += 2;
				cnt += islen;
				break;
			case 's':
				temps = va_arg(ap, char *);
				size_t slen = strlen(temps);
				strncpy(out + cnt, temps, slen);
				i += 2;
				cnt += slen;
				break;
			default:
				errf = 1;
				break;
			}
		}
		if (errf == 1)
		{
			panic("Invalid format.");
			return -1;
		}
	}
	
	out[cnt] = '\0';
	va_end(ap);

	return cnt;

	//panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...)
{
	panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap)
{
	panic("Not implemented");
}

#endif
