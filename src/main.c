#include "../include/commands.h"

int main(int argc, char *argv[]){
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <comand> {args...]\n", argv[0]);
		return 1;
	}

	const char *cmd = argv[1];

	if (strcmp(cmd, "show") == 0 && argc >=3) {
		const char *sub = argv[2];
		if (strcmp(sub, "interfaces") == 0) {
			return show_interfaces();
		}
	}

	fprintf(stderr, "Unknown command\n");
	return 1;
}
