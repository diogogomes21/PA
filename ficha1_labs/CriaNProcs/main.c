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

#include "debug.h"
#include "memory.h"

int main(int argc, char *argv[]){

	/* Disable warnings
	(void)argc; (void)argv;*/

	gengetopt_args_info ai;
  if (cmdline_parser (argc, argv, &ai) != 0) {
      exit(1);
  }
	/* Main code */
	DEBUG("Main is empty -- add code here");

	for (int i = 0; i < n ; i++) {
			pid=fork();
			if (pid==0) {
				printf("%dProcesso #n(PID=%d)\n", i, getpid());
			}else if (pid>0) {
				WAIT(NULL);
			}else{
				ERROR(1,"Cannot create Process");
			}
	}

	return 0;
}
