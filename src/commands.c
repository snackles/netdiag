#include "../include/commands.h"
#include <stdio.h>
#include <stdlib.h>

void showInterfaces(){
	DIR *dir = opendir("/sys/class/net");
	if (!dir) {
		perror("opendir /sys/class/net");
		return;
	}

	printf("%-12s %-12s %-18s %-6s %-30s\n", "IFACE", "STATE", "MAC", "MTU", "IP ADDRESSES");
	 printf("--------------------------------------------------------------------\n");

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_LNK && entry->d_type != DT_DIR) continue;
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

		char state[16], mac[32], mtu [16];
		char ip_addresses[256] = {0};

		readSysFile(entry->d_name, "operstate", state, sizeof(state)); 
		readSysFile(entry->d_name, "address",   mac,   sizeof(mac));
		readSysFile(entry->d_name, "mtu",       mtu,   sizeof(mtu));

		char cmd[512];
        snprintf(cmd,
				 sizeof(cmd),
				 "ip -4 addr show %s | grep -oP '(?<=inet\\s)\\d+\\.\\d+\\.\\d+\\.\\d+/?\\d*' | tr '\\n' ' '",
				 entry->d_name);
        
        FILE *fp = popen(cmd, "r");
        if (fp) {
            fgets(ip_addresses, sizeof(ip_addresses), fp);
            pclose(fp);
            size_t len = strlen(ip_addresses);
            if (len > 0 && ip_addresses[len-1] == ' ') {
                ip_addresses[len-1] = '\0';
            }
		}
		if (strlen(ip_addresses) == 0) {
			snprintf(cmd,
					 sizeof(cmd),
					 "ip -6 addr show %s | grep -oP '(?<=inet6\\s)[a-fA-F0-9:]+/?\\d*' | tr '\\n' ' '",
					 entry->d_name);
			fp = popen(cmd, "r");
			if (fp) {
				fgets(ip_addresses, sizeof(ip_addresses), fp);
				pclose(fp);
				size_t len = strlen(ip_addresses);
				if (len > 0 && ip_addresses[len-1] == ' ') {
					ip_addresses[len-1] = '\0';
				}
			}
		}
		if (strlen(ip_addresses) == 0) {
			strcpy(ip_addresses, "-");
		}
			
		printf("%-12s %-12s %-18s %-6s %-30s\n",
			   entry->d_name,
			   state,
			   mac,
			   mtu,
			   ip_addresses);
	}
  
	closedir(dir);
}

void showRoutes() {
    printf("%-15s %-12s %s\n", "DESTINATION", "GATEWAY", "INTERFACE");
    printf("------------------------------------------------\n");
    
    FILE *fp = popen("ip -4 route show | awk '{if($1==\"default\") print \"default\",$3,$5; else print $1,$3,$5}' | column -t", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
        }
        pclose(fp);
    } else {
        system("ip route");
    }
}

void showVlans() {
    printf("%-12s %-12s %-12s\n", "IFACE", "VID", "PARENT");
    printf("--------------------------------\n");
    
    DIR *dir = opendir("/sys/class/net");
    if (!dir) {
        perror("opendir /sys/class/net");
        return;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_LNK && entry->d_type != DT_DIR) continue;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        
        char vlan_path[512];
        snprintf(vlan_path,
				 sizeof(vlan_path),
				 "/sys/class/net/%s/vlan",
				 entry->d_name);
        
        DIR *vlan_dir = opendir(vlan_path);
        if (vlan_dir) {
            closedir(vlan_dir);
            
            char vid   [16] = "-";
            char parent[64] = "-";
            
            readSysFile(entry->d_name, "vlan/id",    vid,    sizeof(vid));
            readSysFile(entry->d_name, "vlan/upper", parent, sizeof(parent));
            
            printf("%-12s %-12s %-12s\n", entry->d_name, vid, parent);
        }
    }
    
    closedir(dir);
}

void checkLink(const char *iface) {
	char state[32];
	if (readSysFile(iface, "operstate", state, sizeof(state)) == 0) {
		printf("Interface state %s: %s\n",
			   iface,
			   state);
	} else {
		fprintf(stderr, "Error: couldn't read the state for the interface %s: %s\n",
				iface,
				state);
	}
}

void checkGateway(const char *ip) {
	char command[256];
	snprintf(command,
			 sizeof(command),
			 "ping -c 4 %s",
			 ip);
	system(command);
}

void collectDiagnostics() {
	printf("Collecting diagnostic information...\n");

	system("mkdir -p netdiag_temp");

	system("ip addr > netdiag_temp/ip_addr.txt");
	system("ip route > netdiag_temp/ip_route.txt");
	system("dmesg > netdiag_temp/dmesg.txt");
	system("ls -la /sys/class/net > netdiag_temp/sys_class_net.txt");

	printf("Creating an achive netdiag_archive.tar.gz...\n");
	system("tar -czf netdiag_archive.tar.gz netdiag_temp");

	system("rm -rf netdiag_temp");
	printf("Done!\n");
}
