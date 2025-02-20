#include <Phobos.CRT.h>

#include <cstring>

void PhobosCRT::strCopy(char* Dest, const char* Source, size_t Count)
{
	strncpy_s(Dest, Count, Source, Count - 1);
	Dest[Count - 1] = 0;
}

void PhobosCRT::wstrCopy(wchar_t* Dest, const wchar_t* Source, size_t Count)
{
	wcsncpy_s(Dest, Count, Source, Count - 1);
	Dest[Count - 1] = 0;
}
