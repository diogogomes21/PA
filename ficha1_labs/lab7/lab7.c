#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "debug.h"

int main(int argc, char* argv[]){
    pid_t pid;
    int var1;

    (void) argc;
    (void) argv;

    var1 = 10;

    switch (pid = fork()) {
        case -1:
            /* erro */
            ERROR(1, "Erro na execução do fork()");
            break;

        case 0:
            /* filho */
            /*Como o execlp() tem um no variável de parâmetros o
             * último parâmetro tem de ser sempre NULL*/
            execlp("ls", "ls","-lF", "-a", NULL);
            ERROR(1, "erro no execlp");
            break;

        default:
            /* pai */
            wait(NULL);
            printf("Fim da execução do comando ls –lF -a.\n");
            break;
    }
    return 0;
}
