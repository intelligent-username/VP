#include "socket_ipc.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

typedef struct {
    int size;
    int64_t pts;
    int64_t dts;
    int stream_index;
    int flags;
} IPC_Header;

static int write_all(int fd, const void *buf, size_t count) {
    size_t written = 0;
    while (written < count) {
        ssize_t w = write(fd, (const char*)buf + written, count - written);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        written += w;
    }
    return 0;
}

static int read_all(int fd, void *buf, size_t count) {
    size_t r = 0;
    while (r < count) {
        ssize_t n = read(fd, (char*)buf + r, count - r);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return -1; // EOF
        r += n;
    }
    return 0;
}

int ipc_init_sockets(int socks[2]) {
    return socketpair(AF_UNIX, SOCK_STREAM, 0, socks);
}

void ipc_increase_buffer(int fd, int size) {
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size));
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
}

int ipc_send_packet(int fd, AVPacket *pkt) {
    IPC_Header hdr;
    hdr.size = pkt->size;
    hdr.pts = pkt->pts;
    hdr.dts = pkt->dts;
    hdr.stream_index = pkt->stream_index;
    hdr.flags = pkt->flags;

    if (write_all(fd, &hdr, sizeof(hdr)) < 0) return -1;
    if (hdr.size > 0) {
        if (write_all(fd, pkt->data, hdr.size) < 0) return -1;
    }
    return 0;
}

int ipc_recv_packet(int fd, AVPacket *pkt) {
    IPC_Header hdr;
    if (read_all(fd, &hdr, sizeof(hdr)) < 0) return -1;

    av_new_packet(pkt, hdr.size);
    pkt->pts = hdr.pts;
    pkt->dts = hdr.dts;
    pkt->stream_index = hdr.stream_index;
    pkt->flags = hdr.flags;

    if (hdr.size > 0) {
        if (read_all(fd, pkt->data, hdr.size) < 0) {
            av_packet_unref(pkt);
            return -1;
        }
    }
    return 0;
}

// Returns 1 if received, 0 if timeout, -1 if error
int ipc_recv_packet_timeout(int fd, AVPacket *pkt, int timeout_ms) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    int ret = poll(&pfd, 1, timeout_ms);
    if (ret > 0) {
        if (ipc_recv_packet(fd, pkt) == 0) return 1;
        return -1;
    }
    return ret; // 0 or < 0
}
