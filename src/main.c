#include "../include/commands.h"
#include <string.h>

int main(int argc, char *argv[]){
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <comand> {args...]\n", argv[0]);
		return 1;
	}

	const char *cmd = argv[1];

	if (strcmp(cmd, "show") == 0 && argc >= 3) {
		const char *sub = argv[2];
		if (strcmp(sub, "interfaces") == 0) {
			showInterfaces();
			return 0;
		} else if (strcmp(sub, "routes") == 0) {
			showRoutes();
			return 0;
		} else if (strcmp(sub, "vlans") == 0) {
			showVlans();
			return 0;
		}
	} else if (strcmp(cmd, "check") == 0 && argc > 3) {
		const char *sub   = argv[2];
		const char *value = argv[3];
		if (strcmp(sub, "link") == 0) {
			checkLink(value);
			return 0;
		} else if (strcmp(sub, "gateway") == 0){
			checkGateway(value);
			return 0;
		}
	} else if (strcmp(cmd, "collect") == 0) {
		collectDiagnostics();
		return 0;
	}

	fprintf(stderr, "Unknown command\n");
	return 1;
}
