#ifndef ELF_SOCKETS_H
#define ELF_SOCKETS_H

/* Initializes internal variables used by the library.
 * 'procId' is the identifier number of the process.
 * 'nProc' is the number of processes forked.
 * There shouldn't be more than one process with the same 'procId'.
 */
void ElfSocket_init(int procId, int nProc);

/* Sends a message to other process.
 * 'buf' is the buffer where the message is located.
 * 'bufLen' is the message length, in bytes.
 * 'dest' is the process ID of the process that should receive the message.
 */
void ElfSocket_send(void *buf, int bufLen, int dest);

/* Receives a message from other process.
 * 'buf' is the buffer where the message should be stored.
 * 'bufLen' is the length of the message being expected
 * 'src' is the process ID of the process from which to receive the message.
 */
void ElfSocket_recv(void *buf, int bufLen, int src);

/* Frees all memory allocated internally in the library
 */
void ElfSocket_finalize();

#endif
