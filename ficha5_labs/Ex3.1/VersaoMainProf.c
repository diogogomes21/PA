/**
* @file main.c
* @brief Description
* @date 2018-10-25 12h57:27 
* @author Patricio R. Domingues
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
int bin_to_str(const char* bin_str_ptr);


int bin_to_str(const char* bin_str_ptr){
	if( bin_str_ptr == NULL ){
		fprintf(stderr,"ERROR: NULL string\n");
		return -1;
	}
	size_t num_bits_int = sizeof(int) * 8;
	size_t bin_str_len = strlen(bin_str_ptr);
	if( bin_str_len > (num_bits_int-1) ){
		fprintf(stderr,"ERROR: string '%s' too large for int"
			       " (%zu chars)\n", bin_str_ptr,bin_str_len); 
		return -1;
	}

	if( bin_str_len == 0 ){
		fprintf(stderr,"ERROR: Empty string\n");
		return -1;
	}
	const char *ptr;
	int total = 0;
	int current_weight = 1;
	ptr = bin_str_ptr+bin_str_len-1; //ptr=&bin_str_ptr[bin_str_len-1];
	for(size_t i=0; i<bin_str_len;i++){
		if( (*ptr != '0') && (*ptr != '1') ){
			fprintf(stderr,"ERROR: invalid char '%c' (pos:%zu)\n",
					*ptr,i);
			return -1;
		}
		if( *ptr == '1' ){
			total += current_weight;
		}
		current_weight = (current_weight << 1);
		// current_weigth *= 2;
		ptr--;
	}
	return total;
}



#define C_ERROR_MISSING_STR	(3)
#define C_ERROR_CANT_CONVERT	(4)	
int main(int argc, char *argv[]){
	struct gengetopt_args_info args;
	// gengetopt parser: deve ser a primeira linha de código no main
	if(cmdline_parser(argc, argv, &args))
		ERROR(1, "Erro: execução de cmdline_parser\n");
	char *bin_to_convert_ptr;
	if( args.bin_given ){
		bin_to_convert_ptr = args.bin_arg;
	}else{
		fprintf(stderr,"ERROR: missing string to convert\n");
		exit(C_ERROR_MISSING_STR);
	}
	// bin_to_convert_ptr 
	int ret_bin_to_str = bin_to_str(bin_to_convert_ptr);
	if (ret_bin_to_str == -1 ){
		exit(C_ERROR_CANT_CONVERT);
	}
	printf("'%s' ==> '%d'\n", bin_to_convert_ptr, ret_bin_to_str);

	// gengetopt: libertar recurso (assim que possível)
	cmdline_parser_free(&args);

	return 0;
}