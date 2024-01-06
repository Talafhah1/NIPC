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
// #include "../src/NIPC.h"	// nipc_create, nipc_get, nipc_subscribe, nipc_send, nipc_close, nipc_remove

#include <unistd.h>
#include <errno.h>

struct _nipc_node
{
	struct _nipc_node* next;
	struct _nipc_node* prev;
	pid_t pid;
	long type;
};

const int _add_node(struct _nipc_node** head, struct _nipc_node** tail, const pid_t pid, const long type)
{
	struct _nipc_node* node = malloc(sizeof(struct _nipc_node));
	if (!node) { errno = ENOMEM; return -1; }
	if (!*head) *head = node;
	node->next = NULL;
	node->pid = pid;
	node->type = type;
	if (*tail)
	{
		node->prev = *tail;
		(*tail)->next = node;
	}
	else node->prev = NULL;
	*tail = node; 
	return 0;
}

const int _remove_node(struct _nipc_node** head, struct _nipc_node** tail, const pid_t pid)
{
	struct _nipc_node* ptr = *head;
	while (ptr)
	{
		if (ptr->pid == pid)
		{
			if (ptr->prev) ptr->prev->next = ptr->next;
			if (ptr->next) ptr->next->prev = ptr->prev;
			if (*tail == ptr) *tail = ptr->prev;
			free(ptr);
			return 0;
		}
		ptr = ptr->next;
	}
	errno = ENOENT;
	return -1;
}


int main(const int argc, const char* const argv[], const char* const envp[])
{
	struct _nipc_node* head = NULL;
	struct _nipc_node* tail = NULL;
	_add_node(&head, &tail, 1, 11); //
	_add_node(&head, &tail, 2, 22);
	_add_node(&head, &tail, 3, 33); //
	_remove_node(&head, &tail, 2);
	
	struct _nipc_node* ptr = head;
	while (ptr)
	{
		printf("PID: %d\n", ptr->pid);
		printf("TYPE: %ld\n", ptr->type);
		ptr = ptr->next;
	}

	// pid_t pid = fork();
	// if (pid==0)
	// {

	// 	nipc_subscribe(1234,123456789, (nipc_handler_t)3);
	// 	printShit(); 
	// }
	// else
	// {
		
	// 	nipc_subscribe(1223423434,122342343456789, (nipc_handler_t)2342343);
	// 	printShit();
	// }
	

	// exit(EXIT_SUCCESS);

	
}

// End of tests/test1.c
