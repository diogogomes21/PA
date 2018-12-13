/**
 * @file: server.c
 * @date: 2016-11-17
 * @author: autor
 */
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "debug.h"
#include "common.h"
#include "server_opt.h"
int show_status(uint16_t status);

#define C_MAX_PORT	(1<<16) // ==> common.h
int check_port(int port){
	if( port <= 0 || port >= C_MAX_PORT ){
		fprintf(stderr,"ERROR: invalid port '%d'. Must be within"
				"[1,%d]\n", port, C_MAX_PORT-1);              
		exit(EXIT_FAILURE);
	}
	return port;
}

int main(int argc, char *argv[]){
	/* Estrutura gerada pelo utilitario gengetopt */
	struct gengetopt_args_info args_info;

	/* Processa os parametros da linha de comando */
	if (cmdline_parser (argc, argv, &args_info) != 0){
		exit(ERR_ARGS);
	}


	int status = args_info.status_arg;
	if( status < 0 || status >= (1<<16) ){
		fprintf(stderr,"[ERROR] Invalid status '0x%x' (out of range)\n",
				status);
		exit(EXIT_FAILURE);
	}
	uint16_t domus_status = (uint16_t) status;
	 //DEBUG
	show_status(domus_status);

	int remote_port = check_port(args_info.port_arg);

	int udp_server_socket;
	udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if( udp_server_socket == -1 ){
		ERROR(EXIT_FAILURE, "Can't create udp_server_socket (IPv4)");
	}

	struct sockaddr_in udp_server_endpoint;
	memset(&udp_server_endpoint, 0, sizeof(struct sockaddr_in));
	udp_server_endpoint.sin_family = AF_INET;
	udp_server_endpoint.sin_addr.s_addr = htonl(INADDR_ANY);  	
	udp_server_endpoint.sin_port = htons(remote_port);
	int ret_bind = bind(udp_server_socket, 
	  (struct sockaddr *) &udp_server_endpoint, sizeof(struct sockaddr_in));
	if( ret_bind == -1 ){
		ERROR(EXIT_FAILURE,"Can't bind @udp_server_endpoint info");
	}

	// liberta recurso: socket UDP IPv4
	if (close(udp_server_socket) == -1){
		ERROR(EXIT_FAILURE, "Can't close udp_server_socket (IPv4)");
	}

	cmdline_parser_free(&args_info);
	return 0;
}

int show_status(uint16_t status){
	char status_S[16][128];
	strcpy(status_S[0],"1 Portão da garagem (aberto / fechado)");
	strcpy(status_S[1],"2 Iluminação do hall de entrada");
	strcpy(status_S[2],"3 Iluminação sala");
	strcpy(status_S[3],"4 Iluminação jardim");
	strcpy(status_S[4],"5 Persiana 1");
	strcpy(status_S[5],"6 Persiana 2");
	strcpy(status_S[6],"7 Persiana 3");
	strcpy(status_S[7],"8 Piso radiante (ligado / desligado)");
	strcpy(status_S[8],"");
	strcpy(status_S[9],"");
	strcpy(status_S[10],"");
	strcpy(status_S[11],"");
	strcpy(status_S[12],"");
	strcpy(status_S[13],"");
	strcpy(status_S[14],"");
	strcpy(status_S[15],"");


	int num_bits = sizeof(status) * 8;
	for(int i=0; i<num_bits; i++){

		int mask = 1 << i;
		int bit = status & mask;
		if(strlen(status_S[i])>0) {
			printf("%s:%s\n",status_S[i],bit?"ON":"OFF");
		}
	}//for

	return 1;
}
