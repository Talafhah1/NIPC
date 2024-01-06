//TODO: do

// src/NIPC.c

/**
 * @file  src/NIPC.c
 * @brief  Notifier Inter-Process Communication (NIPC) source file
 * @date  26/12/2023
 * @version  1.0.0
 */

#include "NIPC.h"	// struct nipc_message, nipc_create, nipc_get, nipc_subscribe, nipc_send, nipc_close, nipc_remove //TODO: ensure that names are consistent if you change them
#include <errno.h>	// errno, Error number definitions
#include <string.h>	// memset
#include <sys/msg.h>	// msgget, msgctl, msgsnd, msgrcv
#include <sys/shm.h>	// shmget, shmat, shmdt, shmctl
#include <sys/types.h>	// key_t, pid_t
#include <sys/ipc.h>	// IPC_CREAT, IPC_RMID, IPC_EXCL
#include <signal.h>	// SIGUSR1
#include <unistd.h>	// getpid
#include <stdlib.h>	// malloc

//TODO: document
nipc_handler_t _handler;

struct _nipc_node
{
	struct _nipc_node* next;
	struct _nipc_node* prev;
	pid_t pid;
	long type;
};

/**
 * @brief  
 * 
 * @param  tail 
 * @return  int 
 */
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



void _nipc_handler(const int signal)
{
	if (_handler)
	{
		
	}
}

/**
 * @brief  Creates a NIPC instance that has a key `_key`. If a NIPC instance with the same key exists, the function fails.
 * @remark  The NIPC instance relies on a message queue to store and manage messages.
 * @param  _key {const key_t} The key of the NIPC instance to create.
 * @return  {const int} `0` on success, `-1` on failures.
 */
const int nipc_create(const key_t _key)
{
	int shmid = shmget(_key, 2 * sizeof(struct _nipc_node**), IPC_CREAT | IPC_EXCL | 0666);
	if (shmid == -1 && errno == EEXIST) return -1;
	const int id = msgget(_key, IPC_CREAT | IPC_EXCL | 0666);
	if (id == -1)
	{
		errno = EEXIST; //TODO: ensure that this is the correct error code
		return -1;
	}
	void* shm = shmat(shmid, NULL, 0);
	if (shm == (void*) -1)
	{
		errno = ENOMEM; //TODO: ensure that this is the correct error code
		return -1;
	}
	memset(shm, 0, 2 * sizeof(struct _nipc_node**));
	return 0;
}

/**
 * @brief  Opens a NIPC instance whose key is `_key`.
 * @param  _key {const key_t} The key of the NIPC instance to open.
 * @return  {const int} If the instance exists, a unique positive non-zero integer to identify the instance is returned. Otherwise, `-1` is returned.
 */
const int nipc_get(const key_t _key)
{
	const int id = msgget(_key, 0666);
	if (id == -1)
	{
		errno = ENOENT; //TODO: ensure that this is the correct error code
		return -1;
	}
	return id;
}

/**
 * @brief  Subscribes the calling process to the opened NIPC instance identified by `id` under the specified type `type`.
 * @remarks  The function handler is invoked upon any notification from the NIPC instance.
 * @remarks  `SIGUSR1` is used to notify the process of a new message.
 * @param  id {const int} The ID of the NIPC instance to subscribe to.
 * @param  type {const long} The multicast channel to subscribe to.
 * @param  handler {nipc_handler_t} The function handler to invoke upon any notification from the NIPC instance.
 * @return  {const int} `0` on success, `-1` on failures.
 */
const int nipc_subscribe(const int id, const long type, nipc_handler_t handler)
{
	if (msgctl(id, IPC_STAT, NULL) == -1)
	{
		errno = ENOENT; //TODO: ensure that this is the correct error code
		return -1;
	}
	else if (type <= 0)
	{
		errno = EINVAL; //TODO: ensure that this is the correct error code
		return -1;
	}
	else if (handler == NULL)
	{
		errno = EINVAL; //TODO: ensure that this is the correct error code
		return -1;
	}

	//FIXME: This shit ain't working
	// we need to know the key to shmat
	// int shmid = shmget(_key, 2 * sizeof(struct _nipc_node**), 0666);
	// if (shmid == -1) { errno = ENOENT; return -1; }

	// void* shm = shmat(shmid, NULL, 0);
	// if (shm == (void*) -1)
	// {
	// 	errno = ENOMEM; //TODO: ensure that this is the correct error code
	// 	return -1;
	// }


	_handler = handler;
	//ok these are unique per process


	// signal(SIGUSR1, handler); //



	return 0;
}

//FIXME: delete this
#include <stdio.h>
void printShit()
{
	printf("[%d] handler: %p\n", getpid(), _handler);
}


const int nipc_send(const int id, const struct nipc_message msg, const long type)
{


	return 0;
}


const int nipc_close(const int id)
{


	return 0;
}


const int nipc_remove(const key_t _key)
{


	return 0;
}

// End of src/NIPC.c
