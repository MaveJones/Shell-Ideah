#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

//Constantes
#define STDIN 0
#define STDOUT 1
#define DEFAULT 2
#define TOPO 49

//Estrutura que representa os comandos
typedef struct command_t {
  char *name;
  int argc;
  char **argv;
} CommandType;

//Estrutura que representa a fila
typedef struct queue {
    int pos_q;
    char *status;
    char *command;
    int pid;
} Queue;

//Cabeçalhos utilizados
void desempilhar(Queue q[],int posC);
char* getCommandPath(char* command);
char* getCommandName(char* command);
void runPwdCommand();
void runKillCommand(int pos_q_job);
void runBgCommand(int pos_q_job);
void runFgCommand(int pos_q_job);
int runCommand(CommandType* command);
void runJobsCommand();
int runConcurrentCommand(CommandType* command);
int runPipedCommands(CommandType* command, CommandType* command2);
int runRedirectedCommand(CommandType* command, char* file, int fileid);


