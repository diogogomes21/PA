#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

int main(int argc, char* argv[]){
    int result;

    (void) argc;
    (void) argv;

    result = system("ps -l");
    if (result < 0) {
        ERROR(1, "Chamada 'a funcao system() falhou.");
    } else {
        printf("Chamada 'a funcao system() retornou: %d.\n", result);
    }
    return 0;
}
