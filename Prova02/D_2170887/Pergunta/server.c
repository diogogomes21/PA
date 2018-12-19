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

void processaCliente(int sock);

int main(int argc, char *argv[]){
  int ser_fd, cli_fd;
  socklen_t cli_len;
  struct sockaddr_in ser_addr, cli_addr;

  /* Processa os parâmetros da linha de comando */
  struct gengetopt_args_info args_info;
  if (cmdline_parser(argc, argv, &args_info) != 0)
  ERROR(EXIT_FAILURE, "cmdline_parser");

  int port = check_port(args_info.port_arg);

  /* cria um socket */
  if ((ser_fd = socket(AF_INET, SOCK_STREAM, 0)) == 1)
  ERROR(EXIT_FAILURE, "socket");

  /* preenche estrutura: ip/porto do servidor */
  memset(&ser_addr, 0, sizeof(ser_addr));
  ser_addr.sin_family = AF_INET;
  ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  ser_addr.sin_port = htons(port);

  /* disponibiliza o porto para escuta */
  if (bind(ser_fd, (struct sockaddr *) &ser_addr, sizeof(ser_addr)) == -1){
    ERROR(EXIT_FAILURE, "bind");
  }
  if (listen(ser_fd, 5) == 1){
    ERROR(EXIT_FAILURE, "Listen");
  }
  printf("Servidor %s no porto %d\n", argv[0], port);

  /* ciclo infinito para atender todos os clientes */
  while (1) {
    cli_len = sizeof(struct sockaddr);
    /* accept - bloqueante */
    cli_fd = accept(ser_fd, (struct sockaddr *) &cli_addr, &cli_len);
    if (cli_fd < 0){
      if (errno == EINTR ){
        printf("accept got EINTR:continuing\n");
        continue;
      }else{
        ERROR(EXIT_FAILURE, "Accept");
      }
    }

    /* mostra informação sobre o cliente e processa pedido */
    char ip[20];
    size_t num_clients = 0;
    DEBUG("cliente [%s@%d]",
    inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip)),
    ntohs(cli_addr.sin_port));

    pid_t my_pid = fork();
    if (my_pid == -1) {
      ERROR(EXIT_FAILURE,"Cannot fork");
    }
    if (my_pid == 0) {
      close(ser_fd);
      processaCliente(cli_fd);
      close(cli_fd);
      exit(0);
    }else{
      num_clients++;
      printf("[SERVER-main] client #%ld\n", num_clients);
      close(cli_fd);
    }
  }//while (1)

    return 0;
}

void processaCliente(int sock)
{
  uint8_t tipoPedido;
  char string[1024];
  srand(time(NULL));
  int tcp_sent_bytes;
  int key;
  char keep;

  // recebe tipoPedido do cliente - chamada bloqueante
  int ret_recv = recv(sock, &tipoPedido, sizeof(uint8_t), 0);
  if( ret_recv == -1 ){
    if( errno == EINTR ){
      printf("recv: got EINTR - continuing\n");
      //continue;
    }else{
      ERROR(EXIT_FAILURE, "recv");
    }
  }
  if (tipoPedido==UINT8_MAX){ //Recebeu CIFRAR
    key = (rand()%127)+1; //1 - 127
    // recebe mensagem
    ret_recv = recv(sock, string, sizeof(string)-1, 0);
    if( ret_recv == -1 ){
      if( errno == EINTR ){
        printf("recv: got EINTR - continuing\n");
        //continue;
      }else{
        ERROR(EXIT_FAILURE, "recv");
      }
    }
    string[ret_recv]='\0';

    //cifrar - NÃO ACABADO
    for (size_t i = 0; i < strlen(string); i++) {
      //keep=string[i] /*XOR*/ key;//XOR
      if (keep>=32) {
        string[i]=keep;
      }
    }

    //Enviar chave
    if (send(sock, &key, sizeof(int), 0) == -1){
          ERROR(EXIT_FAILURE, "send");
    }

    strcpy(string, "Cifragem nao feita"); //Sem tempo problems maquina virtual
    //Enviar mensagem cifrada
    if ((tcp_sent_bytes = send(sock, string, strlen(string), 0)) == -1){
    		ERROR(58, "Can't send to client");
    }
    close(sock);
  }else{ //Recebeu DECIFRAR
    //Receção chave
    ret_recv = recv(sock, &key, sizeof(int), 0);
    if( ret_recv == -1 ){
      if( errno == EINTR ){
        printf("recv: got EINTR - continuing\n");
        //continue;
      }else{
        ERROR(EXIT_FAILURE, "recv");
      }
    }
    // recebe mensagem
    ret_recv = recv(sock, string, sizeof(string)-1, 0);
    if( ret_recv == -1 ){
      if( errno == EINTR ){
        printf("recv: got EINTR - continuing\n");
        //continue;
      }else{
        ERROR(EXIT_FAILURE, "recv");
      }
    }
    string[ret_recv]='\0';
    //Reverter cifragem - NÃO ACABADO
    for (size_t i = 0; i < strlen(string); i++) {
      //keep=string[i] /*XOR*/ key;//XOR
      if (keep>=32) {
        string[i]=keep;
      }
    }

    //Enviar mensagem decifrada
    strcpy(string, "Decifragem nao feita"); //Sem tempo problems maquina virtual
    if ((tcp_sent_bytes = send(sock, string, strlen(string), 0)) == -1){
    		ERROR(58, "Can't send to client");
    }
    close(sock);
  }
    return;
}
