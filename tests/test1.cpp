//TODO: do

// tests/test1.c

/**
 * @file  tests/test1.c
 * @brief  NIPC test case number 1
 * @date  26/12/2023
 * @version  1.0.0
 */

#include <stdio.h>		// printf
#include <stdlib.h>		// exit
#include "../src/NIPC.h"	// nipc_create, nipc_get, nipc_subscribe, nipc_send, nipc_close, nipc_remove


int main(const int argc, const char* const argv[], const char* const envp[])
{
	exit(EXIT_SUCCESS);

	
}

// End of tests/test1.c

//// TODO: design the tests; test 1 will be a simple case, test 2 will have a lot of procs and mx and bx, test 3 will have more than one nipc

//// TODO: you'll have an expected output for each test, and you'll compare it with the actual output, if they dont match, you'll exit(EXIT_FAILURE)

//// TODO: write a build script that compiles the tests into the tests dir
//// TODO: write a run script that forks the tests and checks the exit code, if passed print :white_check_mark: otherwise print :x:
//// TODO: combine both into a single script