/**
 * @file: client.c
 * @date: 2016-11-17
 * @author: autor
 */
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include "memory.h"
#include "debug.h"
#include "common.h"
#include "client_opt.h"

int main(int argc, char *argv[]){
	/* Estrutura gerada pelo utilitario gengetopt */
	struct gengetopt_args_info args_info;

	/* Processa os parametros da linha de comando */
	if (cmdline_parser (argc, argv, &args_info) != 0){
		exit(ERR_ARGS);
	}

	//int remote_port = check_port(args_info.port_arg);

	int udp_client_socket;	
	udp_client_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if( udp_client_socket == -1 ){
		ERROR(EXIT_FAILURE, "Can't create udp_client_socket (IPv4)");
	}


	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	int ret = setsockopt(udp_client_socket,SOL_SOCKET,
				SO_RCVTIMEO,&timeout,sizeof(timeout));
	if( ret == -1 ){
		ERROR(EXIT_FAILURE,"Cannot setsockopt SO_RCVTIMEO");
	}

	// UDP IPv4: informação do servidor UDP
	socklen_t udp_server_endpoint_length = sizeof(struct sockaddr_in);	
	struct sockaddr_in udp_server_endpoint;
	memset(&udp_server_endpoint, 0, sizeof(struct sockaddr_in));
	udp_server_endpoint.sin_family = AF_INET;
	int ret_inet_pton = inet_pton(AF_INET, args_info.ip_arg, 
			&udp_server_endpoint.sin_addr.s_addr);
	if(ret_inet_pton == -1 ){
		ERROR(EXIT_FAILURE,"Invalid family");
	}
	if(ret_inet_pton == 0 ){
		fprintf(stderr,"Invalid IPv4 address: '%s'\n",
				args_info.ip_arg);
		exit(EXIT_FAILURE);
	}
	udp_server_endpoint.sin_port = htons(args_info.port_arg);


	uint16_t request;
	request = 0x00;
	request = htons(request);
	ssize_t udp_read_bytes, udp_sent_bytes;
	udp_sent_bytes = sendto(udp_client_socket, &request, sizeof(request), 0,
	  (struct sockaddr *) &udp_server_endpoint,udp_server_endpoint_length);
	if( udp_sent_bytes == -1 ){
		ERROR(EXIT_FAILURE, "Can't sendto server");
	}
	printf("ok.  (%d bytes enviados)\n", (int)udp_sent_bytes);

	printf("à espera de dados do servidor... "); 
	uint16_t response;

	int flags = 0
	//int flags = MSG_DONWAIT;
	//while(1){
	//
	// ret = recvfrom(..., MSG_DONWAIT,...)
	// if( ret == -1 ){
	// 	if( errno == EAGAIN ){
	//		sleep(1);
	//		continue;
	// 	}else{
	// 	ERROR(EXIT_FAILURE,"Cannot recvfrom");
	// 	...
	//}
	udp_read_bytes=recvfrom(udp_client_socket,&response,
			sizeof(response), flags,
	  (struct sockaddr *) &udp_server_endpoint,&udp_server_endpoint_length);
	if( udp_read_bytes == -1 ){
		ERROR(EXIT_FAILURE, "Can't recvfrom server");
	}
	printf("ok.  (%d bytes recebidos)\n", (int)udp_read_bytes);	

	// UDP IPv4: fecha socket (client)
	if (close(udp_client_socket) == -1){
		ERROR(EXIT_FAILURE, "Can't close udp_client_socket (IPv4)");
	}

	cmdline_parser_free(&args_info);
	return 0;
}
