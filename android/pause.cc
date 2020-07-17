#include <emmintrin.h>
#include <thread>

volatile int a = 0;

void t1()
{
	while( a == 2 )
	{
		_mm_pause();
		_mm_pause();
		_mm_pause();
		_mm_pause();
		_mm_pause();
	}
}

int main()
{
	return 0;
}
