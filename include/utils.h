#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/rtnetlink.h>
#include <linux/if_addr.h>

int readSysFile(const char   *iface,
				const char   *filename,
					  char   *buf,
					  size_t  buf_size);

#endif
