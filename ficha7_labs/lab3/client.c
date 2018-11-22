#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#include "../debug/debug.h"
#include "../cmdline/client_cmdline.h"

#define MAX_PORT ((1<<16)-1)

int main(int argc, char *argv[])
{
	struct gengetopt_args_info args;

	// cmdline_parser: deve ser a primeira linha de código no main
	if( cmdline_parser(argc, argv, &args) ){
		fprintf(stderr, "Cannot cmdline_parser\n");
		exit(1);
	}

	int clnt_sock = socket(AF_INET, SOCK_STREAM,0);
	if(clnt_sock == -1){
		ERROR(2,"Cannot create socker");
	}

	int port = args.porto_arg;
	if (port<=0 || port>MAX_PORT) {
		fprintf(stderr, "Port %d out of range [1,%d]\n", port, MAX_PORT);
		exit(3);
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	char *ip_ptr = args.ip_arg;
	int ret_inet = inet_pton(AF_INET, ip_ptr, &serv_addr.sin_addr.s_addr);
	if (ret_inet<=0) {
		if (ret_inet==0) {
			fprintf(stderr, "Not in presentation format\n");
			exit(32);
		}else{
			ERROR(31,"cannot inet_pton of IP '%s'", ip_ptr);
		}
	}

	int ret_connect = connect(clnt_sock, (const struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (ret_connect == -1) {
		ERROR(33,"Cannot connect to server");
	}

	//Create random number
	srandom(getpid() * time(NULL));
	int number = (random()%9999) + 1;
	char S[32];
	snprintf(	S,sizeof(S),"%d",number);

	int ret_send = send(clnt_sock, S, strlen(S), 0);
	if (ret_send == -1) {
		fprintf(stderr, "Cannot send to server: %s\n", strerror(errno));
		exit(4);
	}

	uint16_t clnt_number;
	int ret_recv = recv(clnt_sock, &clnt_number, sizeof(clnt_number), 0);
	if (ret_recv == -1) {
		fprintf(stderr, "Cannot recv: %s\n", strerror(errno));
		exit(5);
	}else if(ret_recv == 0){
		fprintf(stderr, "Server has closed connection :O\n");
		close(clnt_sock);
		exit(6);
	}
	clnt_number = ntohs(clnt_number);
	printf("clnt_number=%u\n", clnt_number);

	// libertar recurso (cmdline_parser)
	cmdline_parser_free(&args);

	exit(0);
}
