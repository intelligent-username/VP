#ifndef SOCKET_IPC_H
#define SOCKET_IPC_H

#include <libavcodec/avcodec.h>

int ipc_init_sockets(int socks[2]);
void ipc_increase_buffer(int fd, int size);
int ipc_send_packet(int fd, AVPacket *pkt);
int ipc_recv_packet(int fd, AVPacket *pkt);
int ipc_recv_packet_timeout(int fd, AVPacket *pkt, int timeout_ms);

#endif
