#ifndef __REQUEST_H__
#define __REQUEST_H__
#include "request_stats.h"

//void requestHandle(int fd);

void requestHandle(int fd, ReqStats request_stats);

#endif //__REQUEST_H__
