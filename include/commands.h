#ifndef COMMANDS_H
#define COMMANDS_H

#include "utils.h"

void showInterfaces();
void showRoutes();
void showVlans();
void checkLink(const char *iface);
void checkGateway(const char *ip);
void collectDiagnostics();

#endif
