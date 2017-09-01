#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <glib-2.0/glib.h>
#include <libnm/NetworkManager.h>

#define PD_SHELL_MAX_COMMAND_SIZE 256
#define PD_SHELL_TOKEN_BUFSIZE 64
#define PD_SHELL_TOKEN_DELIMITER " \t\r\n\a"

/*
 * Error codes
 */
typedef enum{
        E_PD_SHELL_SUCCESS = 0,
        E_PD_SHELL_FAILURE,
        E_PD_SHELL_ALLOCATION_ERROR,
        E_PD_SHELL_NM_CONNECTION_ERROR
}pd_shell_error_e;

/*
 * Global vars
 */
int argc;
NMClient *nmclient;
GError *error;

typedef struct{
    char* pd_netmask;
    char* pd_ip;
    char* pd_gateway;
    char* pd_dns;
    char* pd_ntp;
}NetInfo;

pd_shell_error_e pd_shell_help ()
{
    /*
     * Display help message
     */
    printf("Pindrop configuration wizard\n");
    printf("Available Commands: \n");
#ifdef PD_SHELL_DEBUG
    printf("\t 0. test: Run User defined tests [Debug mode only]\n");
#endif
    printf("\t 1. network_config: Opens a network configuration wizard\n");
    printf("\t 2. help: Displays this help message\n");
    return E_PD_SHELL_SUCCESS;
}


static
char* pd_shell_make_command(char* command_fmt_string, ... ){
    va_list args;
    char* command_buffer = malloc(sizeof(char) * PD_SHELL_MAX_COMMAND_SIZE);
    va_start(args, command_fmt_string);
    vsnprintf(command_buffer, PD_SHELL_MAX_COMMAND_SIZE - 1, command_fmt_string, args);
    va_end(args);
    return command_buffer;
}

#ifdef PD_SHELL_DEBUG

pd_shell_error_e pd_shell_test_network_config ()
{
    char* command = pd_shell_make_command("nmcli %s %s", "con", "show");
    system(command);
    printf("\n");
    system("nmcli dev status");
    return E_PD_SHELL_SUCCESS;
}


pd_shell_error_e pd_shell_test ()
{
    pd_shell_test_network_config();
    return E_PD_SHELL_SUCCESS;
}

#endif

static
void pd_shell_read_line(char **buffer)
{
    /*
     *  Read line from CLI
     */
    size_t bufsize = 0;
    int buffer_len;
    buffer_len = getline(buffer, &bufsize, stdin);
    if(buffer_len == -1){
        printf("Failed to read line\n");
        exit(E_PD_SHELL_SUCCESS);
    }
    (*buffer)[buffer_len - 1] = '\0';
}

static
NetInfo* pd_shell_get_nm_vars()
{
    /*
     *  Read network info from command line and set the NetInfo struct values
     */

    NetInfo *net_info;
    net_info = malloc(sizeof(NetInfo));
    printf("Enter Netmask: ");
    pd_shell_read_line(&(net_info->pd_netmask));

    printf("Enter IP: ");
    pd_shell_read_line(&(net_info->pd_ip));

    printf("Enter Gateway: ");
    pd_shell_read_line(&(net_info->pd_gateway));

    printf("Enter DNS: ");
    pd_shell_read_line(&(net_info->pd_dns));

    printf("Enter NTP: ");
    pd_shell_read_line(&(net_info->pd_ntp));
    return net_info;

}

pd_shell_error_e pd_shell_network_config_wizard(int __attribute__((unused)) argc,
                                                char __attribute__((unused)) **args)
{
    /*
     * Open network configuration wizard
     */
    NetInfo* pd_network_vars;
    error = NULL;
    nmclient = nm_client_new (NULL, &error);
    if (!nmclient) {
        g_error_free(error);
        return E_PD_SHELL_NM_CONNECTION_ERROR;
    }
    printf("Connected to network Manager\n");
    pd_network_vars = pd_shell_get_nm_vars();
#ifdef PD_SHELL_DEBUG
    printf("Netmask: %s\n", pd_network_vars->pd_netmask);
    printf("IP: %s\n", pd_network_vars->pd_ip);
    printf("Gateway: %s\n", pd_network_vars->pd_gateway);
    printf("DNS: %s\n", pd_network_vars->pd_dns);
    printf("NTP: %s\n", pd_network_vars->pd_ntp);
#endif
    // Perform Network Operations
    free(pd_network_vars);
    return E_PD_SHELL_SUCCESS;
}

pd_shell_error_e pd_shell_execute(char** args)
{
    /*
     * execute command with args
     */
    int status;
    char* command = args[0];
    if(strcmp(command, "help") == 0){
        status = pd_shell_help();
    }else if(strcmp(command, "network_config") == 0){
        status = pd_shell_network_config_wizard(argc, args);
        if(status == E_PD_SHELL_NM_CONNECTION_ERROR){
            printf("Error: Could not connect to NetworkManager: %s.\n", error->message);
        }
        status = E_PD_SHELL_SUCCESS;
    }
#ifdef PD_SHELL_DEBUG
    else if(strcmp(command, "test") == 0){
        status = pd_shell_test();
        if(status == E_PD_SHELL_FAILURE){
            printf("Error: %s.\n", error->message);
        }
        status = E_PD_SHELL_SUCCESS;
    }
#endif
    else{
        printf("pd_shell: %s: command not found...\n", command);
        printf("pd_shell: Enter \"help\" for more information\n");
        status = E_PD_SHELL_SUCCESS;
    }
    return status;
}

char** pd_shell_parse_args(char* line)
{
    /*
     *  Parse line into tokens
     */
    int bufsize = PD_SHELL_TOKEN_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    argc = 0;

    if (!tokens) {
        fprintf(stderr, "pindrop shell: allocation error\n");
        exit(E_PD_SHELL_SUCCESS);
    }

    token = strtok(line, PD_SHELL_TOKEN_DELIMITER);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        argc++;

        if (position >= bufsize) {
            bufsize += PD_SHELL_TOKEN_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "pindrop shell: allocation error\n");
                exit(E_PD_SHELL_SUCCESS);
            }
        }

        token = strtok(NULL, PD_SHELL_TOKEN_DELIMITER);
    }
    tokens[position] = NULL;
    return tokens;
}


void pd_shell(void)
{
    /*
     * Command Line interface loop
     */
    char *line;
    char **args;
    int status;
    do {
        printf("PD Shell> ");
        pd_shell_read_line(&line);
        if ((strcmp(line,"exit") == 0) || (strcmp(line,"quit") == 0)){
            exit(E_PD_SHELL_SUCCESS);
        }
        args = pd_shell_parse_args(line);
        status = pd_shell_execute(args);
        free(line);
        free(args);
    }while(status == E_PD_SHELL_SUCCESS);
}

int
main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
    pd_shell();
    return E_PD_SHELL_SUCCESS;
}