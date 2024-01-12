//TODO: for returns with error codes make sure to mention all possible err nos in the docs, use the @throws or @exception doxygen directives
//TODO: comment verboseness
//TODO: local variable docs
//TODO: mutex everything that is shared

// src/NIPC.cpp

/**
 * @file  src/NIPC.cpp
 * @brief  Notifier Inter-Process Communication (NIPC) source file
 * @date  26/12/2023
 * @version  1.0.0
 */

#include "NIPC.h"		// nipc_message, nipc_handler_t, nipc_create, nipc_get, nipc_subscribe, nipc_send, nipc_close, nipc_remove
#include <unordered_map>	// std::unordered_map
#include <vector>		// std::vector
#include <algorithm>		// std::transform
#include <cerrno>		// errno, Error number definitions
#include <sys/msg.h>		// msgget, msgctl, msgsnd, msgrcv
#include <sys/shm.h>		// shmget, shmat, shmdt, shmctl
#include <sys/types.h>		// key_t, pid_t
#include <sys/ipc.h>		// IPC_CREAT, IPC_RMID, IPC_EXCL
#include <csignal>		// signal, kill, SIGUSR1, SIG_DFL
#include <unistd.h>		// getpid
#include <cstdlib>		// NULL, malloc

/**
 * @name  msgq_buf
 * @brief  A buffer to store a message in a message queue.
 */
struct msgq_buf
{
	/**
	 * @name  receiver
	 * @brief  The PID of the process that will receive the message.
	 * @remark  This corresponds to the `type` parameter of `msgsnd()`.  Here, the message queue type is abstracted an inbox for each process.
	 */
	long receiver;

	/**
	 * @name  channel
	 * @brief  The channel on which to send the message.
	 */
	long channel;

	/**
	 * @name  sender
	 * @brief  The PID of the process that sent the message.
	 */
	pid_t sender;

	/**
	 * @name  message
	 * @brief  The message to send to the process.
	 */
	nipc_message message;

	/**
	 * @name msgq_buf()
	 * @brief  Constructs a new empty message queue buffer.
	 */
	msgq_buf() : receiver(0), channel(0), sender(0), message({}) {}

	/**
	 * @name  msgq_buf()
	 * @brief  Constructs a new message queue buffer with the provided data.
	 * @param  _receiver  {const long}  The PID of the process that will receive the message.
	 * @param  _channel  {const long}  The channel on which to send the message.
	 * @param  _sender  {const pid_t}  The PID of the process that sent the message.
	 * @param  _message  {const nipc_message}  The message to send to the process.
	 */
	msgq_buf(const long _receiver, const long _channel, const pid_t _sender, const nipc_message _message) : receiver(_receiver), channel(_channel), sender(_sender), message(_message) {}
};

/**
 * @name  _subscription_list
 * @brief  A list of all subscriptions to NIPC instances.
 * @remark  The key is the ID of the NIPC instance and the value is a pair of the shared memory segment pointer and the notification handler.
 */
std::unordered_map<int, std::unordered_map<pid_t, long>*> _subscription_list;

/**
 * @name  _handler
 * @brief  The function handler to invoke upon any notification from the NIPC instance.
 */
nipc_handler_t _handler = nullptr;

/**
 * @brief  The signal handler for `SIGUSR1`.
 * @param  signal  {const int}  The signal number.
 * @remark  The signal handler will allocate a buffer containing the message and call the notification handler; it is the responsibility of the notification handler to free the buffer.
 */
void _nipc_handler(const int signal)
{
	// Allocate a buffer to store the message.
	nipc_message* message = static_cast<nipc_message*>(malloc(sizeof(nipc_message)));
	//TODO: check if ptr is valid

	// Receive the message from the message queue.
	msgq_buf buf;

	// Iterate over all NICPs in the subscription list.
	// Once a message is read, break out of the loop.
	for (const std::pair<const int, std::unordered_map<int, long>*>& pair : _subscription_list) if (msgrcv(pair.first, &buf, sizeof(msgq_buf), getpid(), IPC_NOWAIT) != -1) break;

	// Copy the message from the message queue buffer to the allocated buffer.
	*message = buf.message;

	// Ensure a notification handler is set and invoke it.
	if (_handler) _handler(message);

	// If the message wasn't delivered discard the message.
	else free(message);

	// Return.
	return;
}

/**
 * @name  nipc_create()
 * @brief  Creates a NIPC instance that has a key `_key`.  If a NIPC instance with the same key exists, the function fails.
 * @remark  The NIPC instance relies on a message queue to store and manage messages.
 * @param  _key  {const key_t}  The key of the NIPC instance to create.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_create(const key_t _key)
{// TODO: dont need to capture the value of shmid and msqid (maybe)
	// Create a shared memory segment with the provided to store the NIPC instance ensuring that the segment does not already exist.
	const int shmid = shmget(_key, sizeof(std::unordered_map<pid_t, long>), IPC_CREAT | IPC_EXCL | 0666); // TODO: replace 0666 with actual constants
	// If the shared memory segment could not be created, return an error.
	if (shmid == -1 || errno == EEXIST) return -1; //TODO: ensure that this is the correct error code

	// Create a message queue with the provided key to store and manage messages ensuring that the queue does not already exist.
	const int msgq_id = msgget(_key, IPC_CREAT | IPC_EXCL | 0666); // TODO: replace 0666 with actual constants
	// If the message queue could not be created, return an error.
	if (msgq_id == -1) { errno = EEXIST; return -1; } //TODO: ensure that this is the correct error code

	// Attach the shared memory segment to the address space of the creator process.
	std::unordered_map<pid_t, long>* shm = static_cast<std::unordered_map<pid_t, long>*>(shmat(shmid, NULL, 0));
	// If the shared memory segment could not be attached, return an error.
	if (shm == reinterpret_cast<std::unordered_map<pid_t, long>*>(-1)) { errno = ENOMEM; return -1; } //TODO: ensure that this is the correct error code

	// Instantiate a new NIPC instance in the shared memory segment.
	new (shm) std::unordered_map<pid_t, long>();

	// Return success.
	return 0;
}

//TODO: what happens if you fork a child
/**
 * @name  nipc_get()
 * @brief  Opens a NIPC instance whose key is `_key`.
 * @param  _key  {const key_t}  The key of the NIPC instance to open.
 * @return  {const int}  If the instance exists, a unique positive non-zero integer to identify the instance is returned.  Otherwise, `-1` is returned.
 */
const int nipc_get(const key_t _key)
{
	// Get the ID of the message queue with the provided key.  The message queue ID is used to identify the NIPC instance.
	const int msgq_id = msgget(_key, 0666); // TODO: replace 0666 with actual constants
	// If the message queue could not be found, return an error.
	if (msgq_id == -1) { errno = ENOENT; return -1; } //TODO: ensure that this is the correct error code

	// Get the ID of the shared memory segment with the provided key.
	const int shmid = shmget(_key, sizeof(std::unordered_map<pid_t, long>), 0666); // TODO: replace 0666 with actual constants
	// If the shared memory segment could not be attached, return an error.
	if (shmid == -1) { errno = ENOENT; return -1; } //TODO: ensure that this is the correct error code
	// Attach the shared memory segment to the address space of the  process.
	std::unordered_map<pid_t, long>* nipc = static_cast<std::unordered_map<pid_t, long>*>(shmat(shmid, NULL, 0));
	// If the shared memory segment could not be attached, return an error.
	if (nipc == reinterpret_cast<std::unordered_map<pid_t, long>*>(-1)) { errno = ENOMEM; return -1; } //TODO: ensure that this is the correct error code

	// Store the pointer to the shared memory segment in the subscription list such that it could be referenced via the NIPC ID.
	_subscription_list[msgq_id] = nipc;
	// if (_subscription_list.find(msgq_id) != _subscription_list.end()) _subscription_list[msgq_id]->operator[](key) = new_nipc;

	// Return the ID of the NIPC instance.
	return msgq_id;
}

/**
 * @name  nipc_subscribe()
 * @brief  Subscribes the calling process to the opened NIPC instance identified by `id` under the specified type `type`.
 * @param  id  {const int}  The ID of the NIPC instance to subscribe to.
 * @param  type  {const long}  The multicast channel to subscribe to.
 * @param  handler  {nipc_handler_t}  The function handler to invoke upon any notification from the NIPC instance.
 * @remarks  `SIGUSR1` is used to notify the process of a new message.
 * @remarks  When a new message is received, the signal handler will allocate a buffer containing the message and call the notification handler; it is the responsibility of the notification handler to free the buffer.
 * @remarks  For compatibility, the buffer is allocated using `malloc()`; it must be released using `free()`.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_subscribe(const int id, const long type, nipc_handler_t handler)
{
	// Ensure that the NIPC instance exists and the process called `nipc_get()` for this NIPC instance.
	if (_subscription_list.find(id) == _subscription_list.end()) { errno = ENOENT; return -1; } //TODO: ensure that this is the correct error code

	// Processes must subscribe to a valid channel.
	else if (type >= 0) { errno = EINVAL; return -1; } //TODO: ensure that this is the correct error code
	
	// Admit the process to the NIPC instance.
	(*_subscription_list[id])[getpid()] = type;
	//TODO: what if this kvp already exists, same issue as the parent child fork issue

	// Set the signal and notification handlers
	_handler = handler;
	signal(SIGUSR1, _nipc_handler);
	
	// Return success.
	return 0;
}

/**
 * @name  nipc_send()
 * @brief  Sends the message `msg` to the NIPC instance identified by `id`.
 * @param  id  {const int}  The ID of the NIPC instance to the send the message to.
 * @param  msg  {const nipc_message}  The message to send to the NIPC instance.
 * @param  type  {const long}  The channel on which to send the message.  If `type` is `0`, the message is broadcast to all subscribers of this NIPC instance.  If `type` is greater than 0, the message will be sent to the process whose process ID matches `type` (also known as a unicast).  If `type` is less than 0, the message will be sent to all processes subscribed to the channel whose channel ID matches `type` (also known as a multicast).
 * @remark  Once the message is sent, all subscribers of the NIPC instance will be notified of the message.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_send(const int id, const nipc_message msg, const long type)
{
	// Ensure that the NIPC instance exists and the process called `nipc_get()` for this NIPC instance.
	if (_subscription_list.find(id) == _subscription_list.end()) { errno = ENOENT; return -1; } //TODO: ensure that this is the correct error code

	// Instantiate a buffer to hold the PIDs of all processes to receive this message.
	std::vector<pid_t> mailing_list = {};

	// Broadcast the message to all subscribers of the NIPC instance.
	// Iterate over the subscription list and add the PIDs of all subscribers to the mailing list.
	if (!type) std::transform(_subscription_list[id]->begin(), _subscription_list[id]->end(), std::back_inserter(mailing_list), [](const auto& pair) { return pair.first; });

	// Multicast the message to all subscribers of a multicast channel.
	// Iterate over the subscription list and add the PIDs of all subscribers whose channel ID matches `type` to the mailing list.
	else if (type < 0) { for (const std::pair<const pid_t, long>& pair : *_subscription_list[id]) if (pair.second == -type) mailing_list.push_back(pair.first); }

	// Unicast the message to a specific subscriber process.
	// Ensure that the process is a subscriber of the NIPC instance.
	else if (type > 0 && _subscription_list[id]->find(type) != _subscription_list[id]->end()) mailing_list.push_back(-type);

	// If the mailing list is empty, return an error.
	if (mailing_list.empty()) { errno = ENOENT; return -1; } //TODO: ensure that this is the correct error code

	// Iterate over the mailing list.
	for (const pid_t& pid : mailing_list)
	{
		// Create a message queue buffer to store the message and the PID of the process to receive the message.
		msgq_buf buf(pid, type, getpid(), msg);

		// Send the message to the process's inbox.
		if (msgsnd(id, &buf, sizeof(msgq_buf), 0) == -1) { errno = ENOMEM; return -1; } //TODO: ensure that this is the correct error code

		// Notify the process of a new message.
		if (kill(pid, SIGUSR1) == -1) { errno = ESRCH; return -1; } //TODO: ensure that this is the correct error code
	}

	// Return success.
	return 0;
}

/**
 * @name  nipc_close()
 * @brief  Unsubscribes the calling process from the NIPC instance identified by `id`. A closed NIPC instance cannot be used unless opened again.
 * @param  id  {const int}  The ID of the NIPC instance to unsubscribe from.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_close(const int id)
{
	// Ensure that the NIPC instance exists and the process called `nipc_get()` for this NIPC instance.
	if (_subscription_list.find(id) == _subscription_list.end()) { errno = ENOENT; return -1; } //TODO: ensure that this is the correct error code

	// Remove the process from the NIPC instance and detach the shared memory segment.
	_subscription_list[id]->erase(getpid());
	if (shmdt(_subscription_list[id]) == -1) { errno = ENOMEM; return -1; } //TODO: ensure that this is the correct error code

	// Remove the NIPC instance from the subscription list.
	_subscription_list.erase(id);

	// Restore the default signal handler for `SIGUSR1`.
	signal(SIGUSR1, SIG_DFL);

	// Return success.
	return 0;
}

/**
 * @name  nipc_remove()
 * @brief  Removes an NIPC instance identified by `_key` from the system.
 * @param  id  {const int}  The ID of the NIPC instance to remove.
 * @return  {const int}  `0` on success, `-1` on failure.
 */
const int nipc_remove(const key_t _key)
{
	// Get the ID of the message queue with the provided key.
	const int msgq_id = msgget(_key, 0666); // TODO: replace 0666 with actual constants
	// If the message queue could not be found, return an error.
	if (msgq_id == -1) { errno = ENOENT; return -1; } //TODO: ensure that this is the correct error code

	// Get the ID of the shared memory segment with the provided key.
	const int shmid = shmget(_key, sizeof(std::unordered_map<pid_t, long>), 0666); // TODO: replace 0666 with actual constants
	// If the shared memory segment could not be attached, return an error.
	if (shmid == -1) { errno = ENOENT; return -1; } //TODO: ensure that this is the correct error code

	// Remove the message queue.
	if (msgctl(msgq_id, IPC_RMID, NULL) == -1) { errno = ENOMEM; return -1; } //TODO: ensure that this is the correct error code
	// Remove the shared memory segment.
	if (shmctl(shmid, IPC_RMID, NULL) == -1) { errno = ENOMEM; return -1; } //TODO: ensure that this is the correct error code

	// Return success.
	return 0;
}

// End of src/NIPC.cpp
