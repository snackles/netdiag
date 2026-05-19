#include "../include/commands.h"

int main(int argc, char *argv[]){
    #ifdef DEBUG
    setLogLevel(LOG_DEBUG);
    #else
    setLogLevel(LOG_INFO);
    #endif
    
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    char *param = NULL;
	Command cmd = parseCommand(argc, argv, &param);

    switch (cmd) {
        case CMD_SHOW_INTERFACES:
            showInterfaces();
            break;
            
        case CMD_SHOW_ROUTES:
            showRoutes();
            break;
            
        case CMD_SHOW_VLANS:
            showVlans();
            break;
            
        case CMD_CHECK_LINK:
            checkLink(param);
            break;
            
        case CMD_CHECK_GATEWAY:
            checkGateway(param);
            break;
            
        case CMD_COLLECT_DIAGNOSTICS:
            collectDiagnostics();
            break;
            
            
        case CMD_UNKNOWN:			
        default:
            fprintf(stderr, "Error: Unknown command '%s'\n\n", argv[1]);
            printUsage(argv[0]);
            break;
    }

    return 0;
}
