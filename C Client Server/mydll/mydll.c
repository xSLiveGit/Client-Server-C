#include "mydll.h"
#include "Windows.h"
MYDLL_API int Sum(int a, int b)
{
	return a + b;
}

unsigned int minim(unsigned int a,unsigned int b)
{
	if(a < b)
	{
		return a;
	}
	return b;
}
