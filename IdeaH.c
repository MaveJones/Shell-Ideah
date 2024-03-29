//Francisco Roza de Moraes 6427146
//Vitor Faglioni Rossi     6427167
#include "IdeaH.h"
#include "parser.h"

/**
 @brif Trata a String, removendo o espaço no final.
 @param string Comando de entrada. 
 */
void stringRepair(char* string) {
  char* p = string ;
  size_t n ;
  while (isspace(*p))				//Chega no primeiro espaço em branco//
    ++p ;
  if ((n = strlen(p)) > 0)			//Vai do final da string até o ultimo espaço em branco//
    while (isspace(p[n-1]))
      --n ;
  ((char*)memmove(string, p, n))[n] = '\0' ;	// Faz o tratamento para remover os espaços em branco e coloca \0 no final
}

/**
 @brief Main da SHELL
 */
int main(int argc, char **argv) {
  char* dir;
  struct passwd* user;
  char* host;
  char* input;

  dir = (char*)malloc(255*sizeof(char));
  user = (struct passwd*)malloc(sizeof(struct passwd));
  host = (char*)malloc(255*sizeof(char));
  input = (char*)malloc(255*sizeof(char));

  while(TRUE){
    user = getpwuid(getuid());
    gethostname(host,255);
    getcwd(dir,255);

    if(strncmp(user->pw_dir,dir,strlen(user->pw_dir)) == 0){
      //Troca o Home por ~
      dir += strlen(user->pw_dir);
      sprintf(dir,"~%s",strdup(dir));
    }
    printf("%s@%s:%s||%s",user->pw_name,host,dir,PROMPT);
    //--------------------------------------
    fgets(input,1024,stdin);
    stringRepair(input);
    parse(input);
    //--------------------------------------
  }
  return 0;
}


