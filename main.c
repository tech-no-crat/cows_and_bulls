/*
 * main.c
 *
 * Networking and multithreading code and the program's entry point.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <pthread.h> //for threading, link with lpthread
#include "game.h"
#define dprint(expr) printf(#expr " = %g\n", expr)

// Maximum size of the queue of clients waiting to be accept()-ed
#define QSIZE 10

#define DEFAULT_PORT 8888

// Creates and returns a socket descriptor listening to TCP connections
// on the given port
int create_server(int port) {
    int socket_d;
    struct sockaddr_in server;
    
    // Create the socket
    socket_d = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_d == -1) {
        puts("Could not create socket.");
        exit(1);
    }
    puts("Socket created.");

    // Bind to a port
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if(bind(socket_d, (struct sockaddr *) &server, sizeof(server)) < 0) {
        printf("Bind to port %d failed.\n", port);
        exit(1);
    }
    printf("Bind to port %d done.\n", port);

    // Start listening for clients
    listen(socket_d, QSIZE);
    return socket_d;
}


int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    if(argc > 1) port = atoi(argv[1]);
    int server_socket = create_server(port);

    int client_socket;
    struct sockaddr_in client;
    int size = sizeof(struct sockaddr_in);
    pthread_t thread_id;

    puts("Waiting for incoming connections...");
    // While we can accept the next client without trouble
    while((client_socket
        = accept(server_socket,
            (struct sockaddr *) &client,
            (socklen_t*) &size
        )
    )) {
        printf("Connection from accepted from %s\n",
            inet_ntoa(((struct sockaddr_in*)&client)->sin_addr)
        );

        // Create a new thread for the client and continue
        if(pthread_create(&thread_id, NULL, handle_client, (void *) &client_socket) < 0) {
            puts("Could not create thread");
            exit(1);
        }
        printf("Client assigned to thread\n");
    }

    return 0;
}
