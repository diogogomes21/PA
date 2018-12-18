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
#include <errno.h>
#include <stdint.h>

#include "memory.h"
#include "debug.h"
#include "common.h"
#include "client_opt.h"

#define C_MAX_PORT ((1<<16)-1)

int check_port(int port){
  if (port <= 0 || port > C_MAX_PORT) {
    fprintf(stderr, "[ERROR] Invalid port '%d' [1,%d]\n", port,C_MAX_PORT);
    exit(EXIT_FAILURE);
  }
  return port;
}

char *binaryToAbits(unsigned int answer, char *result){ //Test function
  if(answer>1) {
    result=binaryToAbits(answer>>1,result);
  }
  *result='0'+(answer & 0x01);
  return result+1;
}

int main(int argc, char *argv[]){
  /* Estrutura gerada pelo utilitario gengetopt */
  struct gengetopt_args_info args;

  /* Processa os parametros da linha de comando */
  if (cmdline_parser (argc, argv, &args) != 0){
    exit(ERR_ARGS);
  }

  //Check port
  int serv_port = check_port(args.port_arg);

  //Check nubmer of flips (0-255)
  int flips = args.flips_arg;
  if (flips<0 || flips>255) {
    fprintf(stderr, "[ERROR] Invalid number of flips '%d' [0,255]\n", flips);
    exit(EXIT_FAILURE);
  }

  //create socket
  int udp_server_socket;
  if ((udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
    ERROR(EXIT_FAILURE, "Can't create udp_server_socket (IPv4)");
  }

  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;  	//AF_INET = PF_INET
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(serv_port);  	// Server port

  char *serv_IP = args.ip_arg;
  int ret_inet_pton = inet_pton(AF_INET, serv_IP, &(serv_addr.sin_addr.s_addr));
  if (ret_inet_pton < 1) {
    fprintf(stderr, "Wrong server address '%s'\n", serv_IP);
    exit(EXIT_FAILURE);
  }

  flips = htons(flips);
  socklen_t serv_addr_len = sizeof(serv_addr);
  //sendto - number of flips to do
  ssize_t ret_sendto = sendto(udp_server_socket, &flips, sizeof(flips), 0, (struct sockaddr *)&serv_addr, serv_addr_len);
  if (ret_sendto == -1) {
    fprintf(stderr, "ERROR: sendto (buff = %zu bytes): %s\n", sizeof(flips), strerror(errno));
    return 1;
  }

  //recvfrom - Result
  serv_addr_len = sizeof(serv_addr);
  uint32_t result;
  ssize_t ret_recvfrom = recvfrom(udp_server_socket, &result, sizeof(result), 0, (struct sockaddr *)&serv_addr, &serv_addr_len);
  if (ret_recvfrom == -1) {
    fprintf(stderr, "ERROR: recvfrom (buff = %zu bytes): %s\n", sizeof(serv_addr), strerror(errno));
    return 1;
  }
  result = ntohl(result);

  if (result == UINT32_MAX) {
    printf("The requested number of flips is not supported by the server (%d)\n", flips);
    exit(EXIT_FAILURE);
  }

  int mask;
  int showEach;
  for (int i = 0; i < ntohs(flips); i++) {
    mask = 1 << i;
    showEach = result & mask;
    printf("[%d] %s\n", i, showEach?"Head":"Tail");
  }

  //TEST SHOW INT IN BITS
  char ascResult[64];
  *binaryToAbits(result,ascResult)='\0';
  printf("Result: %s\n",ascResult);
  //END

  cmdline_parser_free(&args);

  return 0;
}
