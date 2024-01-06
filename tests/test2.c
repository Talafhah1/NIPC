//TODO: do

// tests/test2.c

/**
 * @file  tests/test2.c
 * @brief  NIPC test case number 2
 * @date  26/12/2023
 * @version  1.0.0
 */

#include <stdio.h>		// printf
#include <stdlib.h>		// exit
#include "../src/NIPC.h"	// nipc_create, nipc_get, nipc_subscribe, nipc_send, nipc_close, nipc_remove

#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(const int argc, const char* const argv[], const char* const envp[])
{
	key_t key = ftok(".",65); 
	int shmid = shmget(key,sizeof(int*),0666|IPC_CREAT); 
	int **shm_ptr = (int**) shmat(shmid,(void*)0,0); 
	*shm_ptr = (int*) malloc(sizeof(int));
	**shm_ptr = 9000;
	printf("Shared memory pointer: %p\n", shm_ptr);
	printf("Address of the int: %p\n", *shm_ptr);

	int pid = fork();
	if(pid == 0)
	{
		printf("Read from shared memory: %d\n", **shm_ptr);
		printf("Shared memory pointer: %p\n", shm_ptr);
		printf("Address of the int: %p\n", *shm_ptr);

		exit(EXIT_SUCCESS);
	}
	else 
	{
		wait(NULL);
		shmdt(shm_ptr); 
		shmctl(shmid,IPC_RMID,NULL); 
	}

	exit(EXIT_SUCCESS);
}

// End of tests/test2.c
