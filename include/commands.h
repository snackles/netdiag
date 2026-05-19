#ifndef COMMANDS_H
#define COMMANDS_H

#include "utils.h"

typedef enum {
    CMD_SHOW_INTERFACES,
    CMD_SHOW_ROUTES,
    CMD_SHOW_VLANS,
    CMD_CHECK_LINK,
    CMD_CHECK_GATEWAY,
    CMD_COLLECT_DIAGNOSTICS,
    CMD_UNKNOWN,
} Command;

// Main functions
void showInterfaces();
void showRoutes();
void showVlans();
void checkLink(const char  *iface);
void checkGateway(const char  *ip);
void collectDiagnostics();

// Auxiliary functions
int executeDiagnosticCommand (const char *cmd,
							  const char *output_file);
Command parseCommand(int argc,
					 char *argv[],
					 char **param);
void printUsage(const char *program_name);

#endif
