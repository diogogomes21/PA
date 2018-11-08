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

#include "debug.h"
#include "memory.h"
#include "args.h"

int bin_to_decimal(const char *bin_s_ptr);
int is_bit_n_set(int input, size_t bit_n);

struct gengetopt_args_info args;

int main(int argc, char *argv[]){
	/* Disable warnings */
	(void)argc; (void)argv;

	if(cmdline_parser(argc, argv, &args)){
		ERROR(1, "Erro: execução de cmdline_parser\n");
	}

	if(args.bin_given!=0){ /*Se -b usado executa*/
		/*Verificacao de string*/
		size_t num_bits_int = sizeof(int)*8;
		size_t bin_str_len = strlen(args.bin_arg);
		if(bin_str_len > (num_bits_int-1)){ /*Verificar tamanho de string para conversao de binario para decimal(int)*/
			fprintf(stderr, "ERROR: string '%s' too large for int (%zu chars)\n",
				args.bin_arg, bin_str_len);
			return -1;
		}
		for (size_t i = 0; i < strlen(args.bin_arg); i++) { /*Verificar se e binario*/
			if(args.bin_arg[i]!='0'&&args.bin_arg[i]!='1'){
				fprintf(stderr, "ERROR: number is not binary '%s'\n",args.bin_arg);
				return -1;
			}
		}
		/*FIM Verificacao*/
		printf("Valor inserido (Binary): %s\n", args.bin_arg);
		printf("Valor em decimal: %d\n", bin_to_decimal(args.bin_arg));
	}

	cmdline_parser_free(&args);

	return 0;
}

int bin_to_decimal(const char *bin_s_ptr){
	int i=0/*, j*/;
  /*j = sizeof(int)*8;*/
	for (int j = sizeof(int)*8; ((*bin_s_ptr=='0') || (*bin_s_ptr=='1')); j--) {
		i <<= 1;
		if ( *bin_s_ptr=='1' ) i++;
		bin_s_ptr++;
	}
  /*while ( (j--) && ((*bin_s_ptr=='0') || (*bin_s_ptr=='1')) ) {
      i <<= 1;
      if ( *bin_s_ptr=='1' ) i++;
      bin_s_ptr++;
  }*/
  return i;
}

int is_bit_n_set(int input, size_t bit_n){

}
