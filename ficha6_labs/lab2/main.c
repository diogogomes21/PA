/**
* @file main.c
* @brief Description
* @date 2018-1-1
* @author name of author
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <arpa/inet.h>

#include "debug.h"
#include "memory.h"

int main(int argc, char *argv[]){
	/* Disable warnings */
	(void)argc; (void)argv;
	short ShortTest = 0x1122;
	if (sizeof(short)!=2) {
		fprintf(stderr, "sizeof(short)=%u(wanted 2)\n", ShortTest);
		exit(1);
	}

    /* Verificar tamanho do tipo short */

		if (htons(ShortTest)==ShortTest) {
			printf("BIG ENDIAN (no conversion was done)\n");
		}else{
			printf("LITTLE ENDIAN (conversion was done): %x\n", htons(ShortTest));
		}

    /* Determinar endianess da máquina local */

	return 0;
}
