#include "../include/utils.h"

static LogLevel current_log_level = LOG_INFO;

// Read interface attributes from /sys/class/net
int readSysFile(const char *iface,
				const char *filename,
				char *buf,
				size_t buf_size) {
	
	char path[MAX_PATH_LENGTH];
	snprintf(path,
			 sizeof(path),
			 "/sys/class/net/%s/%s",
			 iface,
			 filename);

	FILE *f = fopen(path, "r");
	if (f == NULL) {
		logMessage(LOG_DEBUG, "Cannot open %s", path);
		return -1;
	}

	if (fgets(buf, buf_size, f) == NULL) {
		logMessage(LOG_DEBUG, "Cannot read from %s", path);
		fclose(f);
		return -1;
	}

    trimNewline(buf);
	fclose(f);
	return 0;
}

void printTableHeader(const char **headers,
					  int num_columns,
					  const int *widths) {
	
    for (int i = 0; i < num_columns; i++) {
        printf("%-*s", widths[i], headers[i]);
        if (i < num_columns - 1) {
            printf("  ");
        }
    }
    printf("\n");
}

void printTableSeparator(const int *widths, int num_columns) {
    int total_width = 0;
    for (int i = 0; i < num_columns; i++) {
        total_width += widths[i];
    }
    total_width += (num_columns - 1) * 2;
    
    for (int i = 0; i < total_width; i++) {
        printf("-");
    }
    printf("\n");
}

void printTableRow(const char **values,
				   const int *widths,
				   int num_columns) {
	
    for (int i = 0; i < num_columns; i++) {
		
        char truncated[256];
        strncpy(truncated, values[i] ? values[i] : "-", sizeof(truncated) - 1);
        truncated[sizeof(truncated) - 1] = '\0';
        
        if (strlen(truncated) > (size_t)widths[i]) {
            truncated[widths[i]] = '\0';
        }
        
        printf("%-*s", widths[i], truncated);
        if (i < num_columns - 1) {
            printf("  ");
        }
    }
    printf("\n");
}

void setLogLevel(LogLevel level) {
    current_log_level = level;
}

const char* getLogLevelString(LogLevel level) {
	switch (level) {
	case LOG_DEBUG:   return "DEBUG";
	case LOG_INFO:    return "INFO";
	case LOG_WARNING: return "WARNING";
	case LOG_ERROR:   return "ERROR";
	default:          return "UNKNOWN";
	}
}

void logMessage(LogLevel level,
				const char *format,
				...) {

    if (level < current_log_level) {
        return;
    }
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[32];
	
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    FILE *output = (level == LOG_ERROR) ? stderr : stdout;   
    fprintf(output, "[%s] [%s] ", time_str, getLogLevelString(level));
    
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);
    
    fprintf(output, "\n");
    
    if (level == LOG_ERROR) {
        fflush(stderr);
    }
}

int isValidInterface(const struct dirent *entry) {	
    if (entry->d_type != DT_LNK && entry->d_type != DT_DIR) {
        return 0;
    }
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        return 0;
    }
    
    char path[MAX_PATH_LENGTH];
    snprintf(path,
			 sizeof(path),
			 "/sys/class/net/%s/operstate",
			 entry->d_name);
    
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        return 0;
    }
    fclose(f);
    
    return 1;
}

int isVlanInterface(const char *iface) {	
    char vlan_path[MAX_VLAN_PATH];
    snprintf(vlan_path,
			 sizeof(vlan_path),
			 "/sys/class/net/%s/vlan",
			 iface);
    
    DIR *vlan_dir = opendir(vlan_path);
    if (vlan_dir) {
        closedir(vlan_dir);
        return 1;
    }
    return 0;
}

int getInterfaceIPs(const char *iface,
					char *buf,
					size_t buf_size) {
	
    if (!iface || !buf || buf_size == 0) {
        return -1;
    }
    
    buf[0] = '\0';
    
    const char *ip_commands[] = {
        "ip -4 addr show %s 2>/dev/null | grep -oP '(?<=inet\\s)\\d+\\.\\d+\\.\\d+\\.\\d+/?\\d*' | tr '\\n' ' '",
        "ip -6 addr show %s 2>/dev/null | grep -oP '(?<=inet6\\s)[a-fA-F0-9:]+/?\\d*' | tr '\\n' ' '"
    };
    
    for (size_t i = 0; i < sizeof(ip_commands) / sizeof(ip_commands[0]); i++) {
        char cmd[MAX_CMD_LENGTH];
        snprintf(cmd,
				 sizeof(cmd),
				 ip_commands[i],
				 iface);
        
        FILE *fp = popen(cmd, "r");
        if (!fp) {
            logMessage(LOG_DEBUG, "Failed to execute command for %s", iface);
            continue;
        }
        
        char temp_buf[MAX_IP_ADDR] = {0};
        if (fgets(temp_buf, sizeof(temp_buf), fp) != NULL) {
            trimTrailingSpace(temp_buf);
            
            if (strlen(temp_buf) > 0) {
                if (strlen(buf) > 0) {
                    strncat(buf, " ", buf_size - strlen(buf) - 1);
                }
                strncat(buf, temp_buf, buf_size - strlen(buf) - 1);
            }
        }
        pclose(fp);
    }
    
    if (strlen(buf) == 0) {
        strncpy(buf, "-", buf_size - 1);
        buf[buf_size - 1] = '\0';
    }
    
    return 0;
}

int validateIPv4(const char *ip) {
    if (!ip || strlen(ip) == 0) {
        return 0;
    }
    
    struct in_addr addr;
    return inet_pton(AF_INET, ip, &addr) == 1;
}

int executeCommand(const char *cmd,
				   char *output,
				   size_t output_size) {
	
    if (!cmd) { return -1; }
    
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        logMessage(LOG_ERROR, "Failed to execute command: %s", cmd);
        return -1;
    }
    
    if (output && output_size > 0) {
        output[0] = '\0';
        char line[MAX_LINE_LENGTH];
        
        while (fgets(line, sizeof(line), fp) != NULL) {
            if (strlen(output) + strlen(line) < output_size - 1) {
                strcat(output, line);
            } else {
                break;
            }
        }
    }
    
    int status = pclose(fp);
    return WEXITSTATUS(status);
}

void trimNewline(char *str) {
    if (!str) return;
    
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

void trimTrailingSpace(char *str) {
    if (!str) return;
    
    size_t len = strlen(str);
    while (len > 0 && str[len - 1] == ' ') {
        str[len - 1] = '\0';
        len--;
    }
}
