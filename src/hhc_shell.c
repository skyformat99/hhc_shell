#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <db.h>
#include <readline/readline.h>
#include <readline/history.h>

#define HHC_SHELL_MAX_COMMAND_SIZE 256
#define HHC_SHELL_TOKEN_BUFSIZE 64
#define HHC_SHELL_TOKEN_DELIMITER " \t\r\n\a"
#define clear() printf("\033[H\033[J")

/*
 * Error codes
 */
typedef enum{
        E_HHC_SHELL_SUCCESS = 0,
        E_HHC_SHELL_FAILURE,
        E_HHC_SHELL_ALLOCATION_ERROR,
        E_HHC_SHELL_INVALID_INPUT_ERROR,
        E_HHC_SHELL_DB_CREATE_ERROR,
        E_HHC_SHELL_DB_OPEN_ERROR
}hhc_shell_error_e;

/*
 * bool enum
 */
typedef enum { false, true } bool;

/*
 * Global vars
 */
int hhc_shell_argc;
bool db_exists =  false;
DB *hhc_shell_db;
u_int32_t hhc_shell_db_flags;

/*
 * Function signature
 */
void hhc_shell();

#ifdef HHC_SHELL_DEBUG

hhc_shell_error_e hhc_shell_test()
{
    return E_HHC_SHELL_SUCCESS;
}

#endif

hhc_shell_error_e hhc_shell_help()
{
    /*
     * Display help message
     */
    printf("Pindrop configuration cli\n");
    printf("Available Commands: \n");
#ifdef HHC_SHELL_DEBUG
    printf("\t 0. test: Run User defined tests [Debug mode only]\n");
#endif
    printf("\t 1. help: Displays this help message\n");
    return E_HHC_SHELL_SUCCESS;
}

static
char* hhc_shell_make_command(const char* command_fmt_string, ... )
{
    va_list args;
    char* command_buffer = malloc(sizeof(char) * HHC_SHELL_MAX_COMMAND_SIZE);
    va_start(args, command_fmt_string);
    vsnprintf(command_buffer,
            HHC_SHELL_MAX_COMMAND_SIZE - 1,
            command_fmt_string, args);
    va_end(args);
    return command_buffer;
}

static
hhc_shell_error_e hhc_shell_create_db()
{
    int ret;
    ret = db_create(&hhc_shell_db, NULL, 0);
    if(ret != 0){
        printf("Failed to create DB\n");
        return E_HHC_SHELL_DB_CREATE_ERROR;
    }
    hhc_shell_db_flags = DB_CREATE;
    ret = hhc_shell_db->open(hhc_shell_db,
                            NULL,
                            "hhc_shell_db.db",
                            NULL,
                            DB_BTREE,
                            hhc_shell_db_flags,
                            0);
    if(ret != 0){
        printf("Failed to open DB\n");
        return E_HHC_SHELL_DB_OPEN_ERROR;
    }

    db_exists = true;

    return E_HHC_SHELL_SUCCESS;
}

static
hhc_shell_error_e hhc_shell_db_put()
{
    return E_HHC_SHELL_SUCCESS;
}

static
hhc_shell_error_e hhc_shell_db_get()
{
    return E_HHC_SHELL_SUCCESS;
}

hhc_shell_error_e hhc_shell_execute(char** hhc_shell_args)
{
    /*
     * execute command with hhc_shell_args
     */
    int status;
    char* command = hhc_shell_args[0];
    if((strncmp(command, "help", 4) == 0) || (strncmp(command, "?", 1) == 0)){
        status = hhc_shell_help();
    }
    else if(strncmp(command, "clear", 5) == 0){
        clear();
        status = E_HHC_SHELL_SUCCESS;
    }
#ifdef HHC_SHELL_DEBUG
    else if(strncmp(command, "test", 4) == 0){
        status = hhc_shell_test();
        if(status == E_HHC_SHELL_FAILURE){
            printf("Error\n");
        }
        status = E_HHC_SHELL_SUCCESS;
    }
#endif
    else{
        //printf("hhc_shell: %s: command not found...\n", command);
        //printf("hhc_shell: Enter \"help\" for more information\n");
        status = E_HHC_SHELL_SUCCESS;
    }
	return status;
}

char** hhc_shell_parse_hhc_shell_args(char* line)
{
    /*
     *  Parse line into tokens
     */
    int bufsize = HHC_SHELL_TOKEN_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    hhc_shell_argc = 0;

    if (!tokens) {
        fprintf(stderr, "config cli: allocation error\n");
        exit(E_HHC_SHELL_SUCCESS);
    }

    token = strtok(line, HHC_SHELL_TOKEN_DELIMITER);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        hhc_shell_argc++;

        if (position >= bufsize) {
            bufsize += HHC_SHELL_TOKEN_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "config cli: allocation error\n");
                exit(E_HHC_SHELL_SUCCESS);
            }
        }

        token = strtok(NULL, HHC_SHELL_TOKEN_DELIMITER);
    }
    tokens[position] = NULL;
    return tokens;
}

static
void sig_handler(int signo)
{
    switch (signo) {
        case SIGINT:
            printf("\n");
            hhc_shell();
            break;
        default:
            break;
    }
    return;
}

void hhc_shell(void)
{
    /*
     * Command Line interface loop
     */
    char *line;
    char **hhc_shell_args;
    int status;
    char shell_prompt[16];

    // Signal Handler
    struct sigaction hhc_shell_signal_action;
    hhc_shell_signal_action.sa_handler = sig_handler;
    sigaction(SIGINT, &hhc_shell_signal_action, NULL);

    snprintf(shell_prompt, sizeof(shell_prompt), "protect $ ");

    do {
        line = readline(shell_prompt);

        if(!line){
            continue;
        }

        if((strcmp(line, "") == 0)){
            continue;
        }

        add_history(line);

        if ((strncmp(line, "exit", 4) == 0) || (strncmp(line,"quit", 4) == 0)){
            exit(E_HHC_SHELL_SUCCESS);
        }
        hhc_shell_args = hhc_shell_parse_hhc_shell_args(line);
        status = hhc_shell_execute(hhc_shell_args);
        free(line);
        free(hhc_shell_args);
    }while(status == E_HHC_SHELL_SUCCESS);

    if(hhc_shell_db != NULL){
        hhc_shell_db->close(hhc_shell_db, 0);
    }
}

int
main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
    hhc_shell_help();
    hhc_shell();
    return E_HHC_SHELL_SUCCESS;
}
