#include "parser.h"
#include "builtin.h"

/**
 @brief parse trata a entrada de forma a identificar o comando e os seus argumentos.
 @param input entrada do sistema.
 */
void parse(char *input){

  CommandType *command = (CommandType*) malloc(sizeof(CommandType));
  CommandType *command2 = (CommandType*) malloc(sizeof(CommandType));
  int count;
  int runmode = 9;
  int piped;
  char* redirectionFilename;
  int redirectionType;
  char* buffer = (char*)calloc(1024, sizeof(char));
  char* ptr;
  char* aux;

  piped = FALSE;
  redirectionType = DEFAULT;
  redirectionFilename = (char *)calloc(1024, sizeof(char));

//!Verifica se o comando não é vazio.
  if((strcmp(input, "") != 0) && (strcmp(input, ".") != 0) && (strcmp(input, "..") != 0)) {
    //!Verifica se o comando é para sair da shell
   if((strcmp(input, "quit")== 0) || (strcmp(input, "exit")== 0)) {
      printf("Finalizando a SHELL\n");
      exit(0);
    }
    //!verifica se o comando é o PWD
    if(strcmp(input, "pwd")== 0){
        runPwdCommand();
    } else{
        //!verifica se o comando é o JOBS
        if(strcmp(input, "jobs")== 0){
            runJobsCommand();
        } else{
        //!Inicia os arrays de argumentos
            command->argv = (char**)calloc(256, sizeof(char*));
            command2->argv = (char**)calloc(256, sizeof(char*));    
            /**< Quebra a string no comando */
            ptr = strtok(input, " ");
            command->name = ptr;
	    command->argv[0] = ptr;
            command->argc = 1;
            count = 1;
            runmode = NORMALMODE;
            ptr = strtok(NULL, " ");   
            while (ptr != NULL) {     
                if(strcmp(ptr, "|")==0) { /**< Utiliza | pipe */              
                    if(piped) {
                        printf("ERRO! Soh eh suportado um pipe.\n");
                    }
                    else {
                     //! Le o argumento
                        piped = TRUE;
                        ptr = strtok (NULL, " ");
                        if(ptr != NULL) {
                            command2->name = ptr;
                            command2->argv[0] = ptr;
                            command2->argc = 1;
                            count = 1;
                        }
                        else {
                            piped = FALSE;
                            printf("Erro no pipe!\n");
                        }
                    }
                }
                else if(strcmp(ptr, ">")==0) { /**< Utiliza o redirecionamento de saída */   
                        ptr = strtok (NULL, " ");
                        if (ptr != NULL) {
                          redirectionFilename = ptr;
                          redirectionType = STDOUT;             
                          ptr = NULL;           
                        }
                        else {
                            printf("Erro! Sem arquivo para saida.\n");
                        }   
                }
                else if (strcmp(ptr, "<")==0) {/**< Utiliza o redirecionamento de entrada */
                        ptr = strtok (NULL, " ");
                        if (ptr != NULL) {
                            redirectionFilename = ptr;
                            redirectionType = STDIN;
                            ptr = NULL;
                        }
                    else {
                        printf("Erro! Sem arquivo para entrada.\n");
                    }
                }
                else {
                    if (piped) {
                        command2->argv[count++] = ptr;
                        command2->argc++;
                    }
                    else {
                        command->argv[count++] = ptr;
                        command->argc++;
                    }
                }
                aux = ptr;
                ptr = strtok (NULL, " ");
            }
           
            if(piped) {
                runPipedCommands(command, command2);
            }
            else if(redirectionType != DEFAULT) {
                runRedirectedCommand(command, redirectionFilename, redirectionType);
            }
            else {
                if(strcmp(command->argv[command->argc-1],"&")==0) {/**< Trata se encontrar &(rodar em BG) */  
                    runmode = CONCURRENTMODE;
                    command->argv[command->argc-1] = NULL;
                }     
                if(strcmp(command->argv[0], "echo") == 0) { /**< Trata o comando ECHO */
                    int i;
                    for (i = 1; i < command->argc; i++) {
                        printf("%s ", command->argv[i]);
                    }
                    printf("\n");
                }
                else if(strcmp(command->argv[0],"kill")==0) { /**< Trata o comando KILL */    
                    int num = atoi(aux);
		    runKillCommand(num);		    
                }     
                else if(strcmp(command->argv[0],"bg")==0) { /**< Trata o comando BG */
                    int num = atoi(aux);
                    runBgCommand(num);
                }     
                else if(strcmp(command->argv[0],"fg")==0) { /**< Trata o comando FG */
                    int num = atoi(aux);
                    runFgCommand(num);
                }     
                else if(strcmp(command->argv[0],"cd") == 0) { /**< Mudar de diretório */
                        if (command->argc == 2) {
                            if(chdir(command->argv[1]) != 0) { /**< diferente de 0 muda o diretório */
                                printf("Diretorio invalido.\n");
                            }
                        }
                        else {
                            printf("Uso incorreto de CD\n");
                        }
                }
                else if (strcmp(command->argv[0],"getenv") == 0) { /**< mostra o ambiente */
                        if (command->argc == 2) {
                            char* variable = (char *) getenv(command->argv[1]);
                            if (variable == NULL) {
                                printf("Erro, ambiente invalido.\n");
                            }
                            else {
                                printf("%s\n", variable);
                            }
                        }
                        else {
                            printf("Erro, Uso incorreto de Getenv.\n");
                        }   
                }
                else if(strcmp(command->argv[0],"setenv") == 0) {
                    if (command->argc == 3) {
                        char* value = (char*)calloc(1024,sizeof(char));
                        strcpy(value, command->argv[1]);
                        strcat(value, "=");
                        strcat(value, command->argv[2]);
                        putenv(value);
                    }
                    else {
                        printf("Erro, Uso incorreto de Setenv.\n");
                    }
                }
                else {/**< Roda o comando em background */
                    if (runmode == NORMALMODE) {/**< NORMALMODE = 0 */
			runCommand(command);
                    }
                    else if (runmode == CONCURRENTMODE) {/**< CONCURRENTMODE = 1 */
                        runConcurrentCommand(command);
                    }
                }
            }
        }
    }
  }
}
