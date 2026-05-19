#include "../include/commands.h"

Command parseCommand(int argc,
					 char *argv[],
					 char **param) {
	
    if (argc < 2) { return CMD_UNKNOWN; }
    
    const char *cmd   = argv[1];
    *param = NULL;

    if (strcmp(cmd, "show") == 0 && argc >= 3) {
        const char *sub = argv[2];
        
        if (strcmp(sub, "interfaces")    == 0) {
            return CMD_SHOW_INTERFACES;
        } else if (strcmp(sub, "routes") == 0) {
            return CMD_SHOW_ROUTES;
        } else if (strcmp(sub, "vlans")  == 0) {
            return CMD_SHOW_VLANS;
        }
    }
    
    if (strcmp(cmd, "check") == 0 && argc > 3) {
        const char *sub   = argv[2];
        *param = argv[3];
        
        if (strcmp(sub, "link")           == 0) {
            return CMD_CHECK_LINK;
        } else if (strcmp(sub, "gateway") == 0) {
            return CMD_CHECK_GATEWAY;
        }
    }
    
    if (strcmp(cmd, "collect") == 0) {
        return CMD_COLLECT_DIAGNOSTICS;
    }
    
    return CMD_UNKNOWN;
}

void printUsage(const char *program_name) {
    
    printf("Usage: %s <command> [options] [arguments]\n\n", program_name);
    
    printf("Commands:\n");
    printf("  show interfaces           Display network interfaces information\n");
    printf("  show routes               Display routing table\n");
    printf("  show vlans                Display VLAN interfaces\n");
    printf("  check link <interface>    Check interface link status\n");
    printf("  check gateway <ip>        Check gateway connectivity\n");
    printf("  collect                   Collect diagnostic information\n");
}

int executeDiagnosticCommand(const char *cmd,
							 const char *output_file) {
	
    if (!cmd || !output_file) { return -1; }
    
    char full_cmd[MAX_CMD_LENGTH];
	// Redirect both stdout and stderr to diagnostic file
    snprintf(full_cmd,
			 sizeof(full_cmd),
			 "%s > %s/%s 2>&1", 
             cmd,
			 DIAG_DIR,
			 output_file);
    
    logMessage(LOG_INFO, "Executing: %s", cmd);
    int result = system(full_cmd);
    
    if (result != 0) {
        logMessage(LOG_WARNING, "Command returned non-zero: %s", cmd);
    }
    
    return result;
}

void showInterfaces() {
	DIR *dir = opendir("/sys/class/net");
	if (!dir) {
		logMessage(LOG_ERROR, "Cannot open /sys/class/net: %s", strerror(errno));
		return;
	}

	const char *headers[] = {"IFACE", "STATE", "MAC", "MTU", "IP ADDRESSES"};
    const int widths[]    = {12, 12, 18, 6, 30};

	printTableHeader(headers, 5, widths);
    printTableSeparator(widths, 5);

    int interface_count = 0;

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
        if (!isValidInterface(entry)) {
            continue;
        }

        char state[MAX_STATE_LENGTH]   = {0};
        char mac[MAX_MAC_LENGTH]       = {0};
        char mtu[MAX_MTU_LENGTH]       = {0};
        char ip_addresses[MAX_IP_ADDR] = {0};
		
		// Read interface information from sysfs
        if (readSysFile(entry->d_name, "operstate", state, sizeof(state)) != 0) {
            strcpy(state, "unknown");
        }
        if (readSysFile(entry->d_name, "address",   mac,   sizeof(mac))   != 0) {
            strcpy(mac, "unknown");
        }
        if (readSysFile(entry->d_name, "mtu",       mtu,   sizeof(mtu))   != 0) {
            strcpy(mtu, "unknown");
        }

		// Collect assigned IPv4/IPv6 addresses
        getInterfaceIPs(entry->d_name, ip_addresses, sizeof(ip_addresses));

        const char *values[] = {entry->d_name, state, mac, mtu, ip_addresses};
        printTableRow(values, widths, 5);
        interface_count++;
	}
	
	closedir(dir);
    
    if (interface_count == 0) {
        logMessage(LOG_WARNING, "No network interfaces found");
    } else {
        logMessage(LOG_INFO, "Total interfaces: %d", interface_count);
    }
}

void showRoutes() {
	logMessage(LOG_INFO, "Displaying routing table");
    
    const char *headers[] = {"DESTINATION", "GATEWAY", "INTERFACE"};
    const int widths[]    = {15, 15, 15};
    
    printTableHeader(headers, 3, widths);
    printTableSeparator(widths, 3);

	// Format routing table using awk for aligned output
    const char *cmd = "ip -4 route show | awk '{if($1==\"default\") "
                      "printf \"%-16s %-16s %-15s\\n\", \"default\", $3, $5; "
                      "else printf \"%-16s %-16s %-15s\\n\", $1, $3, $5}'";
	
	FILE *fp = popen(cmd, "r");
    if (fp) {
        char line[MAX_LINE_LENGTH];
        int route_count = 0;
        
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
            route_count++;
        }
        pclose(fp);
        
        if (route_count == 0) {
            printf("No routes found\n");
        }
    } else {
        logMessage(LOG_WARNING, "Failed to format routes, using fallback");
		// Fallback to raw routing table output
        system("ip route");
    }
}

void showVlans() {
	logMessage(LOG_INFO, "Displaying VLAN interfaces");
    
    const char *headers[] = {"IFACE", "VID", "PARENT"};
    const int widths[]    = {12, 12, 12};
    
    printTableHeader(headers, 3, widths);
    printTableSeparator(widths, 3);
    
    DIR *dir = opendir("/sys/class/net");
    if (!dir) {
        logMessage(LOG_ERROR, "Cannot open /sys/class/net: %s", strerror(errno));
        return;
    }
    
    int vlan_count = 0;
    struct dirent *entry;    
    while ((entry = readdir(dir)) != NULL) {
        if (!isValidInterface(entry)) continue;
        if (!isVlanInterface(entry->d_name)) continue;
        
        char vid[16]    = "-";
        char parent[64] = "-";
        
        if (readSysFile(entry->d_name, "vlan/id", vid, sizeof(vid))     != 0) {
            strcpy(vid, "error");
        }
        if (readSysFile(entry->d_name, "upper", parent, sizeof(parent)) != 0) {
            strcpy(parent, "error");
        }
        
        const char *values[] = {entry->d_name, vid, parent};
        printTableRow(values, widths, 3);
        vlan_count++;
    }
    
    closedir(dir);
    
    if (vlan_count == 0) {
        printf("No VLAN interfaces found\n");
	} else {
        logMessage(LOG_INFO, "Total VLAN interfaces: %d", vlan_count);
    }   
}

void checkLink(const char *iface) {
	    if (!iface || strlen(iface) == 0) {
        logMessage(LOG_ERROR, "Interface name is required");
        return;
    }
    
    char state[MAX_STATE_LENGTH];
    
    if (readSysFile(iface, "operstate", state, sizeof(state)) != 0) {
        logMessage(LOG_ERROR, "Cannot read state for interface: %s", iface);
        return;
    }
    
    if (strcmp(state, "up") == 0) {
        printf("Interface %s: ", iface);
        printf("UP \n");
    } else if (strcmp(state, "down") == 0) {
        printf("Interface %s: ", iface);
        printf("DOWN\n");
    } else {
        printf("Interface %s: %s\n", iface, state);
    }
    
    char mac[MAX_MAC_LENGTH] = {0};
    if (readSysFile(iface, "address", mac, sizeof(mac)) == 0) {
        printf("  MAC Address: %s\n", mac);
    }
    
    char mtu[MAX_MTU_LENGTH] = {0};
    if (readSysFile(iface, "mtu", mtu, sizeof(mtu)) == 0) {
        printf("  MTU: %s\n", mtu);
    }
    
    char carrier[8] = {0};
    if (readSysFile(iface, "carrier", carrier, sizeof(carrier)) == 0) {
        printf("  Carrier: %s\n", (strcmp(carrier, "1") == 0) ? "yes" : "no");
    }
}

void checkGateway(const char *ip) {
   if (!ip || strlen(ip) == 0) {
        logMessage(LOG_ERROR, "IP address is required");
        return;
    }
    
    if (!validateIPv4(ip)) {
        logMessage(LOG_ERROR, "Invalid IPv4 address: %s", ip);
        return;
    }
    
    logMessage(LOG_INFO, "Pinging gateway: %s", ip);
    
    char command[MAX_CMD_LENGTH];
    snprintf(command,
			 sizeof(command), 
             "ping -c %d -W %d %s", 
             DEFAULT_PING_COUNT,
			 PING_TIMEOUT,
			 ip);
    
    printf("Sending %d ICMP packets to %s...\n", DEFAULT_PING_COUNT, ip);
    printf("----------------------------------------\n");
    
    int result    = system(command);
    int exit_code = WEXITSTATUS(result);
    
    printf("----------------------------------------\n");
    
    if (exit_code == 0) {
        logMessage(LOG_INFO, "Gateway %s is reachable", ip);
    } else {
        logMessage(LOG_WARNING, "Gateway %s is not responding", ip);
    }
}

void collectDiagnostics() {
	logMessage(LOG_INFO, "Starting diagnostic collection...");
	
	// Create temporary directory for diagnostic files
    char mkdir_cmd[MAX_CMD_LENGTH];
    snprintf(mkdir_cmd,
			 sizeof(mkdir_cmd),
			 "mkdir -p %s",
			 DIAG_DIR);
    
    if (system(mkdir_cmd) != 0) {
        logMessage(LOG_ERROR, "Failed to create diagnostic directory");
        return;
    }

	// Generate collection timestamp
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str,
			 sizeof(time_str),
			 "%Y-%m-%d_%H:%M:%S",
			 localtime(&now));
    
    char info_file[MAX_PATH_LENGTH];
    snprintf(info_file,
			 sizeof(info_file),
			 "%s/collection_info.txt",
			 DIAG_DIR);
    
    FILE *info = fopen(info_file, "w");
    if (info) {
		fprintf(info, "Diagnostic collection time: %s\n", time_str);

		// Store hostname temporarily using system() output redirection
		system("hostname > /tmp/netdiag_hostname.txt 2>/dev/null");
    
		FILE *hostname_file = fopen("/tmp/netdiag_hostname.txt", "r");
		if (hostname_file) {
			char hostname[256] = {0};
			if (fgets(hostname, sizeof(hostname), hostname_file) != NULL) {
				hostname[strcspn(hostname, "\n")] = 0;
				fprintf(info, "Hostname: %s\n", hostname);
			}
			fclose(hostname_file);
			remove("/tmp/netdiag_hostname.txt");
		} else {
			fprintf(info, "Hostname: unknown\n");
		}
    
		fclose(info);
    }

	printf("Collecting network diagnostics...\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    struct {
        const char *command;
        const char *filename;
        const char *description;
    } diagnostics[] = {
        {"ip addr show",               "ip_addr.txt",         "IP addresses"      },
        {"ip route show",              "ip_route.txt",        "Routing table"     },
        {"ip link show",               "ip_link.txt",         "Link information"  },
        {"ip neigh show",              "arp_table.txt",       "ARP table"         },
        {"ss -tuln",                   "listening_ports.txt", "Listening ports"   },
        {"dmesg | tail -50",           "dmesg.txt",           "Kernel messages"   },
        {"ls -la /sys/class/net",      "sys_class_net.txt",   "Sysfs network info"},
        {"cat /proc/net/dev",          "net_stats.txt",       "Network statistics"},
        {"iptables -L -n 2>/dev/null", "iptables.txt",        "Firewall rules"    },
    };
    
    int num_diags = sizeof(diagnostics) / sizeof(diagnostics[0]);
    
    for (int i = 0; i < num_diags; i++) {
        printf("[%d/%d] Collecting %s...\n", i + 1, num_diags, 
               diagnostics[i].description);
        executeDiagnosticCommand(diagnostics[i].command, diagnostics[i].filename);
    }
    
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
	
    printf("Creating diagnostic archive: %s\n", ARCHIVE_NAME);
    
    char tar_cmd[MAX_CMD_LENGTH];
    snprintf(tar_cmd,
			 sizeof(tar_cmd), 
             "tar -czf %s %s 2>/dev/null",
			 ARCHIVE_NAME,
			 DIAG_DIR);
    
    if (system(tar_cmd) == 0) {
        printf("Archive created successfully!\n");
        printf("File: %s\n", ARCHIVE_NAME);
        
        system("ls -lh netdiag_archive.tar.gz | awk '{print \"Size: \" $5}'");
        
        snprintf(tar_cmd,
				 sizeof(tar_cmd),
				 "rm -rf %s",
				 DIAG_DIR);
        system(tar_cmd);
        
        logMessage(LOG_INFO, "Diagnostic collection completed");
    } else {
        logMessage(LOG_ERROR, "Failed to create diagnostic archive");
    }
}
