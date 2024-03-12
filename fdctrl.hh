#ifndef FDCTRL_H
#define FDCTRL_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

extern void AddFd(const int & epollfd, const int & fd, const bool & one_shot);

extern void RemoveFd(const int & epollfd, const int & fd);

extern void ModifyFd(const int & epollfd, const int & fd, const int & ev);

extern void SetNonBlocking(const int & fd);

#endif // FDCTRL_H
