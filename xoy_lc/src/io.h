#ifndef _IO_H_
#define _IO_H_

#include <sys/types.h>

// sock tcp
int io_tcp_connect(char *ip, short port);

// read and write
ssize_t io_readn(int fd, void *vptr, size_t n);
ssize_t io_writen(int fd, const void *vptr, size_t n);
ssize_t io_readline(int fd, void *vptr, size_t maxlen);

#endif /* end of include guard: _RW_H_ */
