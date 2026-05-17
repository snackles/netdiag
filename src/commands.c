#include "../include/commands.h"
#include <dirent.h>
#include <stdio.h>

int show_interfaces(void){
	DIR *dir = opendir("/sys/class/net");
	if (!dir) {
		perror("opendir /sys/class/net");
		return 1;
	}

	printf("%-12s %-12s %-36s %s\n", "IFACE", "STATE", "MAC", "MTU");
	printf("---------------------------------------------------------------\n");

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_LNK && entry->d_type != DT_DIR) continue;
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

		char path[256];
		snprintf(path, sizeof(path), "/sys/class/net/%s", entry->d_name);

		char state[16] = "UNKNOWN";
		FILE *f = fopen(strcat(path, "/operstate"), "r");
		if(f) {
			fscanf(f, "%15sn", state);
			fclose(f);
		}

		char mac[32] = "00:00:00:00:00:00";
		snprintf(path, sizeof(path), "/sys/class/net/%s/address", entry->d_name);
		f = fopen(path, "r");
		if (f){
			fscanf(f, "%31s", mac);
			fclose(f);
		}

		int mtu = 0;
		snprintf(path, sizeof(path), "/sys/class/net/%s/mtu", entry->d_name);
		f = fopen(path, "r");
		if (f) {
			fscanf(f, "%d", &mtu);
			fclose(f);
		}

		printf("%-12s %-12s %-36s %d\n", entry->d_name, state, mac, mtu);
	}

	closedir(dir);
	return 0;
}
