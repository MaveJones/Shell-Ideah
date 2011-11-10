#include "builtin.h"
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>

Queue queue[50];
int posicao = 0;


//Função de manipulação da fila, do início ao fim os processos
//são passados para baixo, shift para esquerda.

void desempilhar(Queue q[], int posC) {
  int i;
  for (i=posC+1; i<posicao; i++) {
    q[i-1].pos_q = q[i].pos_q-1;
    q[i-1].status = q[i].status;
    q[i-1].command = q[i].command;
    q[i-1].pid = q[i].pid;
  }
}

//Função que encontra o caminho de um comando, este comando é o
// parametro de entrada da função.
//Retorna NULL se não encontrar o comando e retorna o caminho do
// comando se encontrar.

char* getCommandPath(char* command) {

  FILE *fpin;
  char p[1024];
  char *temppath;
  char *temp;
  char *path;

  temp = (char *)calloc(1024, sizeof(char));            //Calloc utilizado para evitar que seja criado com sujeira
  path = (char *) getenv("PATH");                       //Retorna o caminho do ambiente PATH

  strcpy(p,path);
  path = strtok (p,":");                                //Tokeniza até o primeiro PATH
  while ( path != NULL ) {
    strcpy ( temp, path );
    strcat ( temp, "/" );                               //Adiciona o / no final do caminho
    strcat ( temp, command );                           //Adiciona o comando no final
    if ( ( fpin = fopen ( temp, "r" ) ) == NULL ) {     //Testa para verificar se o caminho é aceito
      temp[0] = '\0';                                       //Joga fora o conteúdo caso o teste tenha sido NULL
      path = strtok ( ( char * ) 0, ":" );                  //Recebe o proximo PATH
    }
    else {
      break;                                            //O caminhdo é válido
    }
  }
  if ( fpin == NULL ) {
    return NULL;
  }
  else {
    return temp;
  }
}

 //Faz uma cópia do comando desejado,parametro de entrada,
 // e retorna essa cópia.

char* getCommandName(char* command) {

  char *temp;

  temp = (char *)calloc(1024, sizeof(char));            //Calloc utilizado para evitar que seja criado com sujeira
  strcpy(temp, command);

  return temp;
}

//Função que encontra o caminho do Workspace
//Retorna o caminho atual de trabalho

void runPwdCommand(){
  printf("%s\n", get_current_dir_name());
}

//Função que mata o job que está sendo executado
//A entrada "pos_q_job" é a posição do job na queue
// de processo.

void runKillCommand(int pos_q_job) {
	if(pos_q_job <= 50 && pos_q_job > 0){
	  kill(queue[pos_q_job-1].pid,9);                       //Mata o job, kill(pid,signal) o signal utilizado é o 9
	  printf("PID: %d\n",queue[pos_q_job-1].pid);           // pois é o "hard kill".
	  printf("O processo { %d } foi finalizado\n",pos_q_job);
	  desempilhar(queue,pos_q_job-1);
	  posicao--;
	  printf("Numero de jobs na fila: %d\n",posicao);
        }
}

//Manda um processo existente para background

void runBgCommand(int pos_q_job) {
 	if(pos_q_job <= 50 && pos_q_job > 0){
	  signal(SIGSTOP,queue[pos_q_job-1].pid);
	  printf("O processo { %d } foi para background.\n",queue[pos_q_job-1].pid);
	  queue[pos_q_job-1].status = "BG";
 	}
}


//Manda um processo existente para foreground
void runFgCommand(int pos_q_job) {
	if(pos_q_job <= 50 && pos_q_job > 0){		  	
	  signal(SIGCONT,queue[pos_q_job-1].pid);
  	  printf("O processo { %d } foi para foreground.\n",queue[pos_q_job-1].pid);
  	  queue[pos_q_job-1].status = "FG";
	}
}


//Roda o comando(command) normalmente,
// sem a utilização de nenhum parametro.
//Retorna 0 em caso de sucesso e 1 em
// caso de falha.

int runCommand(CommandType* command) {
  int pid;
  int status;
  char* filename;
  int tam;

  tam = strlen(command->name);
  char aux1[tam+1];
  strcpy(aux1,command->name);
				
  if ((aux1[0] == '.') && (aux1[1] == '/')) {			   
     memmove(command->name,command->name+2,tam-1);
     filename = command->pwd;
     strcat (filename, "/" );	
     strcat (filename, command->name);	
     printf("%s\n",filename);
     return 1;		   
  }else{
	filename = getCommandPath(command->name);
  }
  
  if(filename == NULL) {
    printf("Comando nao encontrado!\n");
    return 1;
  }
  pid = fork();                                         // Cria o novo processo filho.
                                                        // Atribui os valores para cada
  if (posicao < TOPO) {                                 //campo da fila.
    queue[posicao].command = getCommandName(command->name);
    queue[posicao].pos_q = posicao + 1;
    queue[posicao].status = "ATIVO";
    queue[posicao].pid = pid;
    printf("PID: %d",queue[posicao].pid);
    posicao++;
  }
  if(pid < 0) {
    printf("\nErro na criacao do processo.\n");
    return 1;
  }
  else if(pid == 0) {
    execv(filename, command->argv);                     //Executa o comando filename, com os
    return 1;                                           // os argumentos contidos em ARGV
  }
  else {
    //printf("\nProcesso sendo executado.\n");
    while(wait(&status) != pid);                        //Espera a execução terminar.
    //printf("\nProcesso parou de ser executado.\n");
    queue[posicao-1].status = "SUSPENSO";	
  }
  return 0;
}

//Roda o comando(command) em background,
// sem a utilização de nenhum parametro.
//Retorna 0 em caso de sucesso e 1 em
// caso de falha.

int runConcurrentCommand(CommandType* command) {
  int pid;
  char* filename;
  filename = getCommandPath(command->name);

  if(filename == NULL) {
    printf("Comando nao encontrado!\n");
    return 1;
  }

  pid = fork();                                         // Cria o novo processo filho.
                                                        // Atribui os valores para cada
  if (posicao < TOPO) {                                 //campo da fila.
    queue[posicao].command = getCommandName(command->name);
    queue[posicao].pos_q = posicao + 1;
    queue[posicao].status = "ATIVO";
    queue[posicao].pid = pid;
    printf("PID: %d",queue[posicao].pid);
    posicao++;
  }

  if (pid < 0) {
    printf("\nErro na criacao do processo.\n");
    return 1;
  }
  else if(pid == 0) {
    execv(filename, command->argv);
    return 1;
  }
  else {                                                //Parte que trata a execução em BG.
                                                        //Não fica esperando o término do processo.
    //printf("\nProcesso sendo executado em background.\n");
  }
  return 0;
}


//Mostra todos os processos sendo executados por essa SHELL
void runJobsCommand(){
    int i;
    for(i=0; i<posicao; i++)
        printf("[%d] %s %s %d\n", queue[i].pos_q, queue[i].status, queue[i].command, queue[i].pid);
}

//Executa mais de um comando por vez
//Command = Primeiro executável
//Command2 = Segundo executável
//Retorna 0 para falha e 1 para sucesso
int runPipedCommands(CommandType* command, CommandType* command2) {
  int status;

                                                        //Trata o primeiro command
  char* filename = getCommandPath(command->name);
  if(filename == NULL) {
    printf("%s Comando nao encontrado!\n", command->name);
    return 1;
  }
                                                        //Trata o segundo command
  char* filename2 = getCommandPath(command2->name);
  if(filename2 == NULL) {
    printf("%s Comando nao encontrado!\n", command2->name);
    return 1;
  }

  int fd[2];
  int pid, pid2;

  if(pipe(fd) == -1) {
    printf("Erro no PIPE.\n");
    return 1;
  }
  if((pid = fork()) < 0) {
    printf("Erro na criacao do processo.\n");
    return 1;
  }

  if (pid == 0) {
    close(fd[1]);
    dup2(fd[0], 0);
    close(fd[0]);
    execv(filename2, command2->argv);
    printf("Erro de argumentos\n");
    return 1;
  }
  else {
    if ((pid2 = fork()) < 0) {
      printf("Erro na criacao do processo.\n");
      return 1;
    }
    if(pid2 == 0) {
      close(fd[0]);
      dup2(fd[1], 1);
      close(fd[1]);
      execv(filename, command->argv);
      printf("Erro de argumentos\n");
      return 1;
    }
    else {
      close(fd[0]);                                     //Fecha o processo
      close(fd[1]);                                     //Fecha o processo
      while (wait(&status) != pid2);
    }
  }
  return 0;
}

//Redirecionamento de entrada e saída
//command = comando a ser executado
//file = arquivo para ser redirecionado
//fileid = descritor do arquivo
//Retorna 0 com sucesso e 1 com falha

int runRedirectedCommand(CommandType* command, char* file, int fileid) {

  int status;
  char* filename = getCommandPath(command->name);

                                                        //Trata o command
  if(filename == NULL) {
    printf("%s Comando nao existente\n", command->name);
    return 1;
  }

  int pid;
  if((pid = fork()) < 0) {
    printf("Erro na criacao\n");
    return 1;
  }
  if (pid == 0) {
    int fid;
    if(fileid == STDIN) {
      fid = open(file, O_RDONLY, 0600);                 //0600 utilizado para criar um arquivo privado
    }
    else if (fileid == STDOUT) {
      fid = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    }
    dup2(fid, fileid);
    close(fid);

    execv(filename,command->argv);
    printf("Erro de argumento\n");
    return 1;
  }
  else {
    while (wait(&status) != pid);                       //Aguarda a execução
  }

  return 0;
}
