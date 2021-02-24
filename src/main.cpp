#include "main_traits.h"
#include "smartdata.h"
#include <stdio.h>

using Antigravity = Responsive_SmartData<Dummy_Transducer>;

int main(void)
{
	printf("Hello world!\n");

	Antigravity a(0, 1000000, SmartData::ADVERTISED);

	return 0;
}