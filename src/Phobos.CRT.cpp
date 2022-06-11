#include <Phobos.CRT.h>

#include <cstring>
#include <cstdlib>

void PhobosCRT::strCopy(char* Dest, const char* Source, size_t Count) {
	strncpy_s(Dest, Count, Source, Count - 1);
	Dest[Count - 1] = 0;
}

void PhobosCRT::wstrCopy(wchar_t* Dest, const wchar_t* Source, size_t Count) {
	wcsncpy_s(Dest, Count, Source, Count - 1);
	Dest[Count - 1] = 0;
}

char* PhobosCRT::stristr(const char* str, const char* str_search)
{
	char* sors, * subs, * res = nullptr;
	if ((sors = _strdup(str)) != nullptr)
	{
		if ((subs = _strdup(str_search)) != nullptr)
		{
			res = strstr(_strlwr(sors), _strlwr(subs));
			if (res != nullptr)
				res = (char*)(str + (res - sors));
			free(subs);
		}
		free(sors);
	}
	return res;
}