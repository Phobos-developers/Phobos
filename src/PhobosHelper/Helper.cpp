#include "Helper.h"

DynamicVectorClass<char> IntToVector(int num)
{
	DynamicVectorClass<char>res;
	if (num == 0)
	{
		res.AddItem(0);
		return res;
	}
	while (num)
	{
		res.AddItem(num % 10);
		num /= 10;
	}
	return res;
}