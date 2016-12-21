#include "global.h"
#include <sys/epoll.h>

prog_state prog_ctl;
int sigfd = 0;
int efd = 0;
struct epoll_event events;

