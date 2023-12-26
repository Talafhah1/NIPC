//TODO: do

// src/NIPC.c

/**
 * @file  src/NIPC.c
 * @brief  Notifier Inter-Process Communication (NIPC) source file
 * @date  26/12/2023
 * @version  1.0.0
 */

#include "NIPC.h"	// struct nipc_message, nipc_create, nipc_get, nipc_subscribe, nipc_send, nipc_close, nipc_remove //TODO: ensure that names are consistent if you change them
// #include 

int nipc_create(key_t _key) { return 0; }
int nipc_get(key_t _key) { return 0; }
int nipc_subscribe(int id, long type, void (*handler)(struct nipc_message*)) { return 0; }
int nipc_send(int id, struct nipc_message msg, long type) { return 0; }
int nipc_close(int id) { return 0; }
int nipc_remove(key_t _key) { return 0; }

// End of src/NIPC.c
