#include "main_traits.h"
#include "smartdata.h"
#include <stdio.h>
#include <stdlib.h>

using Antigravity = Responsive_SmartData<Dummy_Transducer>;

// This is not good, but it will work...
unsigned int globalCoord = 0;
char* globalIPAddress = 0;

int main(int argc, char* argv[])
{
	printf("SmarData!\n");

	if (argc != 3)
	{
		printf("Usage:\n");
		printf("\n");
		printf("smardata <location> <remote-ip-address>\n");
		printf("\n");
		printf("	location: integer representing coordinates (just to make two program instances running in different places)\n");
		printf("	remote-ip-address: ip address of remote node\n");
		printf("\n");
		return -1;
	}

	globalCoord = atol(argv[1]);
	globalIPAddress = argv[2];

	TSTP::init();

	Antigravity a(0, 1000000, SmartData::ADVERTISED);

	return 0;
}