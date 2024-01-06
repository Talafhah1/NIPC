//TODO: add documentation
//TODO: rename args if needed

// src/NIPC.h

/**
 * @file  src/NIPC.h
 * @brief  Notifier Inter-Process Communication (NIPC) header file
 * @date  26/12/2023
 * @version  1.0.0
 */

#pragma once

#ifndef NIPC_H
#define NIPC_H

#include <sys/types.h>	// key_t

struct nipc_message //TODO: nipc_message
{
	long type;
	char* data;
};

typedef void (*nipc_handler_t)(struct nipc_message* const restrict);
#define NIPC_BROADCAST 0L
#define NIPC_UNICAST(pid) pid
#define NIPC_MULTICAST(type) -type

const int nipc_create(const key_t _key);
const int nipc_get(const key_t _key);
const int nipc_subscribe(const int id, const long type, nipc_handler_t handler);
const int nipc_send(const int id, const struct nipc_message msg, const long type);
const int nipc_close(const int id);
const int nipc_remove(const key_t _key);
//void nipc_cow(); //TODO: do something funny

//FIXME: DELETE THIS 
void printShit();

//TODO: maybe add some #define constants



#endif  // NIPC_H
// End of src/NIPC.h
