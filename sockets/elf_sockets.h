#ifndef ELF_SOCKETS_H
#define ELF_SOCKETS_H

void ElfSocket_init(int procId, int nProc);
void ElfSocket_send(void *buf, int bufLen, int dest);
void ElfSocket_recv(void *buf, int bufLen, int src);
void ElfSocket_finalize();

#endif
