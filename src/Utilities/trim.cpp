#include <cstring>
#include "Trim.h"

char* Trim::LeftTrim(char* szX)
{
	if (' ' == szX[0]) while (' ' == (++szX)[0]);
	return szX;
}

char* Trim::RightTrim(char* szX)
{
	int i = strlen(szX);
	while (' ' == szX[--i]) szX[i] = 0;
	return szX;
}

char* Trim::FullTrim(char* szX)
{
	szX = LeftTrim(szX);
	szX = RightTrim(szX);
	return szX;
}