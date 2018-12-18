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
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#include "debug.h"
#include "common.h"
#include "server_opt.h"

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

int handle_client(int sock);

int main(int argc, char *argv[]){
    /* Estrutura gerada pelo utilitario gengetopt */
    struct gengetopt_args_info args;

    /* Processa os parametros da linha de comando */
    if (cmdline_parser (argc, argv, &args) != 0){
        exit(ERR_ARGS);
    }

  //Check port
  int my_port = check_port(args.port_arg);

  //Check seed
  int my_seed;
  if (args.seed_given) {
    my_seed = args.seed_arg;
    if (my_seed<=0 || my_seed>C_MAX_PORT) {
      fprintf(stderr, "[ERROR] Invalid seed '%d' [1,%d]\n", my_seed,C_MAX_PORT);
      exit(EXIT_FAILURE);
    }
  }else{
    my_seed = 1;
  }

  //Initialize Random Generator
  srand(my_seed);

  // 1: create socket
  int udp_server_socket;
  if ((udp_server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
    ERROR(EXIT_FAILURE, "Can't create udp_server_socket (IPv4)");
  }

  // 2: bind socket
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;  	//AF_INET = PF_INET
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  	// Todas as interfaces de rede
  serv_addr.sin_port = htons(my_port);  	// Server port
  printf("Server UDP/%d\n", my_port);
  printf("Seed: %d\n", my_seed);
  int ret_bind = bind(udp_server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (ret_bind == -1){
    fprintf(stderr, "ERROR: cannot bind at port: %d: %s\n", my_port, strerror(errno));
    exit(EXIT_FAILURE);
  }

  // 3: loop:recvfrom/sendto
  while (1) {
    int ret = handle_client(udp_server_socket);
    if (ret == -1) {
      fprintf(stderr,"Problem with a client: continue\n");
    }
  }

  // 4: close socket
  if (close(udp_server_socket) == -1){
    ERROR(EXIT_FAILURE, "Can't close udp_server_socket (IPv4)");
  }

    cmdline_parser_free(&args);
    return 0;
}

int handle_client(int sock){
  int numberOfFlips;
  uint32_t numberToSend = 0;
  struct sockaddr_in clnt_addr;
  socklen_t clnt_addr_len;

  //recvfrom - number of flips to do
  clnt_addr_len = sizeof(clnt_addr);
  printf("Waiting for coin flip requests...\n");
  ssize_t ret_recvfrom = recvfrom(sock, &numberOfFlips, sizeof(numberOfFlips), 0, (struct sockaddr *)&clnt_addr, &clnt_addr_len);
  if (ret_recvfrom == -1) {
    fprintf(stderr, "ERROR: recvfrom (buff = %zu bytes): %s\n", sizeof(clnt_addr), strerror(errno));
    return -1;
  }

  //Verify numberOfFlips
  numberOfFlips = ntohs(numberOfFlips);
  if (numberOfFlips<1 || numberOfFlips>16) {
    fprintf(stderr, "Unsuported flip number (%d times)\n", numberOfFlips);
    numberToSend = UINT32_MAX;
    //Send Error
    numberToSend = htonl(numberToSend);
    ssize_t ret_sendto = sendto(sock, &numberToSend, sizeof(numberToSend), 0, (struct sockaddr *)&clnt_addr, clnt_addr_len);
    if (ret_sendto == -1) {
      fprintf(stderr, "ERROR: sendto (buff = %zd bytes): %s\n", sizeof(numberToSend), strerror(errno));
      return -1;
    }
    return 0;
  }
  printf("Flipping coin %d times\n", numberOfFlips);

  //Handle - number of flips
  //0 -> Tail
  //1 -> Head
  int randomNumber; //0 or 1
  int heads=0, tails=0; //Counters
  int mask;
  for (int i = 0; i < numberOfFlips; i++) {
    randomNumber = rand()%2; //Set 0 or 1
    if (randomNumber) { //1
      heads++;
    }else{ //0
      tails++;
    }
  	mask = 1 << i;
    if (randomNumber) { //if randomNumber=1 set bit to 1
      numberToSend |= mask;
    }else{ //if randomNumber=0 set bit to 0
      numberToSend &= ~mask;
    }
  }
  //TEST SHOW INT IN BITS
  char ascResult[64];
  *binaryToAbits(numberToSend,ascResult)='\0';
  printf("NumberToSend %s\n",ascResult);
  //END
  printf("Heads -> %d\n", heads);
  printf("Tails -> %d\n", tails);

  //sendto - numberToSend
  numberToSend = htonl(numberToSend);
  ssize_t ret_sendto = sendto(sock, &numberToSend, sizeof(numberToSend), 0, (struct sockaddr *)&clnt_addr, clnt_addr_len);
  if (ret_sendto == -1) {
    fprintf(stderr, "ERROR: sendto (buff = %zu bytes): %s\n", sizeof(numberToSend), strerror(errno));
    return -1;
  }

  return 0;
}
