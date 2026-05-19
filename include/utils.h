#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/rtnetlink.h>
#include <linux/if_addr.h>
#include "constants.h"

// Auxiliary functions
int readSysFile(const char *iface,
				const char *filename,
				char *buf,
				size_t buf_size);
int isValidInterface(const struct dirent *entry);
int isVlanInterface(const char *iface);
int getInterfaceIPs(const char *iface,
					char *buf,
					size_t buf_size);
int validateIPv4(const char *ip);
int executeCommand(const char *cmd,
				   char *output,
				   size_t output_size);
void trimNewline(char *str);
void trimTrailingSpace(char *str);

// Format functions
void printTableHeader(const char **headers,
					  int num_columns,
					  const int *widths);
void printTableSeparator(const int *width,
						 int num_columns);
void printTableRow(const char **values,
				   const int *widths,
				   int num_columns);

// Logger functions
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

void logMessage (LogLevel level,
				 const char *format,
				 ...);
void setLogLevel(LogLevel level);
const char* getLogLevelString(LogLevel level);

#endif
