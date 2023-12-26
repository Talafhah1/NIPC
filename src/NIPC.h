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

int nipc_create(key_t _key);
int nipc_get(key_t _key);
int nipc_subscribe(int id, long type, void (*handler)(struct nipc_message*)); //TODO: typedef handler
int nipc_send(int id, struct nipc_message msg, long type); 
int nipc_close(int id);
int nipc_remove(key_t _key);
//void nipc_cow(); //TODO: do something funny

//TODO: maybe add some #define constants
//TODO: CV qualify these functions, make sure to update the impl file



#endif  // NIPC_H
// End of src/NIPC.h
