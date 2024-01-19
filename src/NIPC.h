// src/NIPC.h

/**
 * @file  src/NIPC.h
 * @brief  Notifier Inter-Process Communication (NIPC) header file
 * @date  26/12/2023
 * @version  1.0.0
 */

// Omar Barakat Talafhah	2020980115
// Bara’ Hani Mukattash		2019980043
// Sarah Mazin Al-Tieneh	2020980050

#pragma once

#ifndef NIPC_H
#define NIPC_H

#include <sys/types.h>	// key_t, pid_t
#include <cstring>	// strncpy

/**
 * @name  nipc_message
 * @brief  A message to be sent through the NIPC.
 */
struct nipc_message
{
	/**
	 * @name  {long}  channel
	 * @brief  The channel on which to send the message.
	 */
	long channel;

	/**
	 * @name  {pid_t}  sender
	 * @brief  The PID of the process that sent the message.
	 */
	pid_t sender;

	/**
	 * @name  {char[256]}  data
	 * @brief  The data to send.
	 */
	char data[256];

	/**
	 * @name  nipc_message()
	 * @brief  Constructs a new empty message.
	 */
	nipc_message() : channel(0L), sender(0) { data[0] = '\0'; }

	/**
	 * @name  nipc_message(const long _channel, const pid_t _sender, const char* const _data)
	 * @brief  Constructs a new message with the given channel, sender and data.
	 * @param  _channel  The channel on which to send the message.
	 * @param  _sender  The PID of the process that sent the message.
	 * @param  _data  The data to send.
	 */
	nipc_message(const long _channel, const pid_t _sender, const char* const _data) : channel(_channel), sender(_sender) { strncpy(data, _data, 256); }
	
};

// The notification handler's function type.
typedef void (*nipc_handler_t)(nipc_message* const);

#define NIPC_BROADCAST 0L
#define NIPC_UNICAST(pid) static_cast<long>(pid)
#define NIPC_MULTICAST(type) static_cast<long>(-type)

/**
 * @name  nipc_create()
 * @brief  Creates a NIPC instance that has a key `_key`.  If a NIPC instance with the same key exists, the function fails.
 * @remark  The NIPC instance relies on a message queue to store and manage messages.
 * @param  _key  {const key_t}  The key of the NIPC instance to create.
 * @throws  EEXIST  If a NIPC instance with the same key already exists.
 * @throws  ENOMEM  If the NIPC instance could not be created due to a lack of memory.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_create(const key_t _key);

/**
 * @name  nipc_get()
 * @brief  Opens a NIPC instance whose key is `_key`.
 * @param  _key  {const key_t}  The key of the NIPC instance to open.
 * @throws  ENOENT  If a NIPC instance with the provided key does not exist.
 * @throws  ENOMEM  If the NIPC instance could not be opened due to a lack of memory.
 * @return  {const int}  If the instance exists, a unique positive non-zero integer to identify the instance is returned.  Otherwise, `-1` is returned.
 */
const int nipc_get(const key_t _key);

/**
 * @name  nipc_subscribe()
 * @brief  Subscribes the calling process to the opened NIPC instance identified by `id` under the specified type `type`.
 * @param  id  {const int}  The ID of the NIPC instance to subscribe to.
 * @param  type  {const long}  The multicast channel to subscribe to.
 * @param  handler  {nipc_handler_t}  The function handler to invoke upon any notification from the NIPC instance.
 * @remarks  `SIGUSR1` is used to notify the process of a new message.
 * @remarks  When a new message is received, the signal handler will allocate a buffer containing the message and call the notification handler; it is the responsibility of the notification handler to free the buffer.
 * @remarks  For compatibility, the buffer is allocated using `malloc()`; it must be released using `free()`.
 * @throws  ENOENT  If the NIPC instance does not exist or it has not been opened.
 * @throws  EINVAL  If the process attempts to subscribe to a channel greater than 0.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_subscribe(const int id, const long type, nipc_handler_t handler);

/**
 * @name  nipc_send()
 * @brief  Sends the message `msg` to the NIPC instance identified by `id`.
 * @param  id  {const int}  The ID of the NIPC instance to the send the message to.
 * @param  msg  {const nipc_message}  The message to send to the NIPC instance.
 * @param  type  {const long}  The channel on which to send the message.  If `type` is `0`, the message is broadcast to all subscribers of this NIPC instance.  If `type` is greater than 0, the message will be sent to the process whose process ID matches `type` (also known as a unicast).  If `type` is less than 0, the message will be sent to all processes subscribed to the channel whose channel ID matches `type` (also known as a multicast).
 * @remark  Once the message is sent, all subscribers of the NIPC instance will be notified of the message.
 * @throws  ENOENT  If the NIPC instance does not exist or it has not been opened.
 * @throws  ENODATA  If the target channel has no subscribers.
 * @throws  ENOMEM  If the message could not be sent due to a lack of memory.
 * @throws  ESRCH  If a target process could not be notified of the message.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_send(const int id, const nipc_message msg, const long type);

/**
 * @name  nipc_close()
 * @brief  Unsubscribes the calling process from the NIPC instance identified by `id`. A closed NIPC instance cannot be used unless opened again.
 * @param  id  {const int}  The ID of the NIPC instance to unsubscribe from.
 * @throws  ENOENT  If the NIPC instance does not exist or it has not been opened.
 * @throws  ENOMEM  If the NIPC instance could not be closed due to a memory error.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_close(const int id);

/**
 * @name  nipc_remove()
 * @brief  Removes an NIPC instance identified by `_key` from the system.
 * @param  id  {const int}  The ID of the NIPC instance to remove.
 * @throws  ENOENT  If the NIPC instance does not exist or it has not been opened.
 * @throws  ENOMEM  If the NIPC instance could not be removed due to a memory error.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_remove(const key_t _key);

/**
 * @name  nipc_thats_what_im_sayin()
 * @brief  thats what im sayin
 * @param  id  {const int}  The ID of the NIPC instance to send the message to.
 * @remarks  augh!
 */
void nipc_thats_what_im_sayin(const int id);

#endif  // NIPC_H
// End of src/NIPC.h
  