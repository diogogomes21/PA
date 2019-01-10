/**
 * @file: server.c
 * @date: 2019-01-10 12h14:42
 * @author: Patricio R. Domingues
 */
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "debug.h"
#include "common.h"
#include "server_opt.h"
#include <assert.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

#define C_PAR   (0)
#define C_IMPAR (1)

#define MAX(x,y)    (x)>(y)?(x):(y)
#define C_MAX_PORT  (1<<16) // ==> common.h
int my_select(int tcp_listen_sock);
int process_udp(int udp_sock);
int process_string(char *buf_with_num, int num_type);
int broadcast_msg(int *descriptors_V, int max_num_descriptors,
                        int clnt_idx, char *buffer);
int get_max_descriptor(int *descriptors_V, int num_elms);
int set_read_descriptors(int *descriptors_V,int num_elms,fd_set *read_set_ptr);
int process_client(int clnt_idx, int *descriptors_V, int max_num_descriptors);
void close_descriptor(int *descriptors_V, int descriptor_idx,
                        int max_num_descriptors);


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

    int my_port = check_port(args_info.port_arg);

    //-----------------------
    // TCP IPv4
    //-----------------------
    int tcp_server_socket;
    if ((tcp_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        ERROR(51, "Can't create tcp_server_socket (IPv4)");

    // TCP IPv4: bind a IPv4/porto
    struct sockaddr_in tcp_server_endpoint;
    memset(&tcp_server_endpoint, 0, sizeof(struct sockaddr_in));
    tcp_server_endpoint.sin_family = AF_INET;
    tcp_server_endpoint.sin_addr.s_addr = htonl(INADDR_ANY);
    tcp_server_endpoint.sin_port = htons(my_port);
    if (bind(tcp_server_socket, (struct sockaddr *) &tcp_server_endpoint,
                sizeof(struct sockaddr_in)) == -1){
        ERROR(52, "Can't bind @tcp_server_endpoint");
    }

    int tcp_max_simultaneous_clients = 16;
    if (listen(tcp_server_socket, tcp_max_simultaneous_clients)  == -1)
        ERROR(53, "Can't listen for %d clients",
                tcp_max_simultaneous_clients);

    //-----------------------
    // UDP IPv4
    //-----------------------
    my_select(tcp_server_socket);

    // Free resources
    if (close(tcp_server_socket) == -1)
        ERROR(33, "Can't close tcp_server_socket (IPv4)");

    cmdline_parser_free(&args_info);
    return 0;
}


int insert_descriptor(int new_descriptor,int *descriptors_V,int num_descriptors){
    int eureka = -1;
    for(int i=0; i<num_descriptors; i++){
        if( descriptors_V[i] == -1 ){
            descriptors_V[i] = new_descriptor;
            eureka = i;
            break;
        }
    }//for
    if( eureka == -1 ){
        fprintf(stderr,"Cannot insert descriptor %d: vector is full\n",
                new_descriptor);
    }
    return eureka;
}

int my_select(int tcp_listen_sock){
    assert( tcp_listen_sock > 0);

    fd_set read_set;
    int max_descriptor; // holds the max value of all descriptors

#define C_MAX_NUM_DESCRIPTORS   (64)
    int max_num_descriptors = C_MAX_NUM_DESCRIPTORS;
    size_t mem_size = sizeof(int)*max_num_descriptors;
    int *descriptors_V = malloc(mem_size);
    if(descriptors_V == NULL ){
           fprintf(stderr,"Cannot malloc %zu bytes\n",mem_size);
           exit(EXIT_FAILURE);
    }
    // descriptors_V vector is initialized to -1 (-1 => empty cell)
    memset(descriptors_V,-1,mem_size);

    // Listening socket get position 0
    descriptors_V[0] = tcp_listen_sock;

    struct timeval timeout;
    long timeout_sec = 10;  // adjust to match preferences
    long timeout_usec = 0;  // must be < 1E6
    assert(timeout_usec<1E6);

    struct sockaddr_in tcp_clnt_endpoint;
    socklen_t tcp_clnt_endpoint_len = sizeof(struct sockaddr_in);
    int new_client;

    while(1){
        max_descriptor = get_max_descriptor(descriptors_V,
                            max_num_descriptors);
        // 1: config select (here, only for read set)
        FD_ZERO(&read_set);
        int num_set_descriptors = set_read_descriptors(
            descriptors_V, max_num_descriptors, &read_set);
        printf("[INFO] %d descriptors set\n", num_set_descriptors);

        timeout.tv_sec = timeout_sec;
        timeout.tv_usec = timeout_usec;

        // 2: call select
        int ret_select = select(max_descriptor+1, &read_set,
                NULL, NULL,&timeout);
        if( ret_select == -1 ){
            ERROR(EXIT_FAILURE,"select failed");
        }
        if( ret_select == 0 ){
            printf("Timeout detected. Continuing...\n");
            continue;
        }
        if( ret_select > 0 ){
            // activity was detect in at least one descriptor
            int num_active = ret_select;

            if( FD_ISSET(descriptors_V[0],&read_set) ){
                // Listening socket has something: accept it...
                new_client = accept(tcp_listen_sock,
                   (struct sockaddr *) &tcp_clnt_endpoint,
                        &tcp_clnt_endpoint_len);
                if( new_client == -1 ){
                    ERROR(EXIT_FAILURE,
                            "Can't accept client");
                }
                else{
                    char tcp_clnt_string_ip[1024];
                    // New client
                    printf("Client %d: %s@%d\n",new_client,
                     inet_ntop(AF_INET,
                     &tcp_clnt_endpoint.sin_addr,
                     tcp_clnt_string_ip,
                     sizeof(tcp_clnt_string_ip)),
                     htons(tcp_clnt_endpoint.sin_port));

                    insert_descriptor(new_client,
                     descriptors_V,max_num_descriptors);
                }
            }else{
                for(int i=1; i<max_num_descriptors;i++){
                    if(FD_ISSET(descriptors_V[i],&read_set)){
                        process_client(i, descriptors_V,
                                    max_num_descriptors);
                        continue;
                    }
                    if( num_active == 0 ){
                        // all active descriptors done
                        // bail out
                        continue;
                    }
                }//for i
            }//else
        }//if

    }//while
    return 0;
}


int process_client(int clnt_idx, int *descriptors_V, int max_num_descriptors){
    char buffer[1024];

    int clnt_sock = descriptors_V[clnt_idx];
    // Read data from client
    ssize_t tcp_read_bytes = recv(clnt_sock, buffer, sizeof(buffer)-1,0);
    if( tcp_read_bytes == -1 ){
        fprintf(stderr,"Cannot read from client. Close it:%s\n",
                strerror(errno));
        close_descriptor(descriptors_V, clnt_sock, max_num_descriptors);
    }else if (tcp_read_bytes == 0 ){
        fprintf(stderr,"Remote peer has closed connection\n");
        close_descriptor(descriptors_V, clnt_sock, max_num_descriptors);
    }
    buffer[tcp_read_bytes] = '\0';
    printf("[clnt:%d/#%d]: received '%s'\n", clnt_idx, clnt_sock, buffer);

    // broadcast to all other clients
    int num_sent = broadcast_msg(descriptors_V, max_num_descriptors,
                                clnt_idx, buffer);
    // sent info message to client
    snprintf(buffer,sizeof(buffer),"Msg sent to %d clients\n", num_sent);

    ssize_t ret_send;
    ret_send = send(descriptors_V[clnt_idx], buffer, strlen(buffer)+1,0);
    if( ret_send == -1 ){
        fprintf(stderr,"Cannot send to %d\n", descriptors_V[clnt_idx]);
        close_descriptor(descriptors_V, clnt_idx, max_num_descriptors);
    }

    return 0;
}


int broadcast_msg(int *descriptors_V, int max_num_descriptors,
                        int clnt_idx, char *buffer){
    ssize_t ret_send;
    int num_sent = 0;
    for(int i=1; i<max_num_descriptors; i++){

        if( descriptors_V[i] != -1 && i != clnt_idx ){

            ret_send = send(descriptors_V[i], buffer,
                            strlen(buffer)+1,0);
            if( ret_send == -1 ){
                fprintf(stderr,"Cannot send to %d\n",
                            descriptors_V[i]);
                close_descriptor(descriptors_V, i,
                            max_num_descriptors);
            }
            num_sent++;
        }
    }//for

    return num_sent;
}

int get_max_descriptor(int *descriptors_V, int num_elms){
    int max = -1;   // We assume that the minimum value is -1
    for(int i=0; i<num_elms; i++){
        if (descriptors_V[i] > max ){
            max = descriptors_V[i];
        }
    }
    return max;
}

int set_read_descriptors(int *descriptors_V,int num_elms,fd_set *read_set_ptr){
    int count_activated = 0;
    for(int i=0; i<num_elms; i++){
        if( descriptors_V[i] >= -1 ){
            FD_SET(descriptors_V[i],read_set_ptr);
            count_activated++;
        }
    }
    return count_activated;
}

void close_descriptor(int *descriptors_V, int descriptor_idx,
                        int max_num_descriptors){
    assert(descriptor_idx >= max_num_descriptors );

    close(descriptors_V[descriptor_idx]);
    descriptors_V[descriptor_idx] = -1;
}
