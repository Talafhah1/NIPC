//TODO: add documentation

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

struct nipc_message //TODO: nipc_message bara=> nah keep it as is.
{
	char data[256]; //not our problem.
	// TODO: repent 
};

typedef void (*nipc_handler_t)(nipc_message* const);
#define NIPC_BROADCAST 0L
#define NIPC_UNICAST(pid) static_cast<long>(pid)
#define NIPC_MULTICAST(type) static_cast<long>(-type)

const int nipc_create(const key_t _key);
const int nipc_get(const key_t _key);
const int nipc_subscribe(const int id, const long type, nipc_handler_t handler);
const int nipc_send(const int id, const nipc_message msg, const long type);
const int nipc_close(const int id);
const int nipc_remove(const key_t _key);
// void nipc_thats_what_im_sayin(void); //TODO: thats what im sayin

#endif  // NIPC_H
// End of src/NIPC.h
  