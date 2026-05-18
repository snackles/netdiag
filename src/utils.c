#include "../include/utils.h"

int readSysFile(const char   *iface,
				const char   *filename,
					  char   *buf,
					  size_t  buf_size) {
	char path[128];
	snprintf(path, sizeof(path), "/sys/class/net/%s/%s", iface, filename);

	FILE *f = fopen(path, "r");
	if (f == NULL) {
		return -1;
	}

	if (fgets(buf, buf_size, f) == NULL) {
		fclose(f);
		return -1;
	}

	buf[strcspn(buf, "\n")] = 0;
	fclose(f);
	return 0;
}
