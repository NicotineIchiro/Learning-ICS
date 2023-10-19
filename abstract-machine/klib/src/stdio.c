#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...)
{
	panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap)
{
	panic("Not implemented");
}
size_t i2str(int n, char *str)
{
	size_t i;
	i = 0;
	
	while (n != 0) {
		str[i] = n % 10 + '0';
		n /= 10;
		++i;
	}

	for (size_t j = 0; j < i / 2; ++j) {
		char temp;
		temp = str[j];
		str[j] = str[i - j - 1];
		str[i - j - 1] = temp;
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
	int temp = 0;
	char *temps;
	va_list al;

	va_start(al, fmt);
	size_t i = 0;
	int cnt = 0;
	int errf = 0;
	size_t flen = strlen(fmt);

	while (i < flen)
	{
		if (fmt[i] != '%')
		{
			out[cnt] = fmt[i];
			++i;
			++cnt;
		}
		else
		{
			switch (fmt[i + 1])
			{
			case 'd':
				temp = va_arg(al, int);//the va_arg for %d is int!
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
				temps = va_arg(al, char *);
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

	va_end(al);
	return cnt;

	panic("Not implemented");
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
