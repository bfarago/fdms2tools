// testdd.cpp : Defines the entry point for the console application.
//

#include "windows.h"
#include <stdio.h>
#include <tchar.h>

int _tmain(int argc, _TCHAR* argv[])
{
    for (int i=10; i<16; i++){
        printf("  %i\n",i);
        flushall();
        Sleep(1000);
    }
	return 0;
}

