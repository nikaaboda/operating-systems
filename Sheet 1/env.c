#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

extern char **environ;


int main(int argc, char *argv[])
{
   //
   //a
   //
   if(argc == 1) {

      char **env = environ;
      for(; *env; env++) {
         printf("%s\n", *env);
      }

   } else if(argc > 1) {
      //
      //b
      // 
      for(int i = 1; i < argc; i++) {
         if(strchr(argv[i], "=") != NULL) {
            char * pair = argv[i];
            char * name = strtok(pair, "=");
            char * value = strtok(NULL, "=");
            _putenv_s(name, value);  
         }
      }
      char **env1 = environ;
      for(; *env1; env1++) {
         printf("%s\n", *env1);
      }


      //
      //f
      //
      for(int i = 1; i < argc; i++) {
         if(strcmp(argv[i], "-u") == 0) {
            char * envName = argv[i + 1];
            strcat(envName, "=");
            printf("\n envName here: %s\n", envName);
            _putenv(envName);
         }
      }
      char **env2 = environ;
      for(; *env2; env2++) {
         printf("%s\n", *env2);
      }
      
   }

   return 1;
}

