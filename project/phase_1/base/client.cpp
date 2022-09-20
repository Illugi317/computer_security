#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <algorithm>
#include <set>
#include <vector>
#include <signal.h>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;

int listenSock;   // Socket for connections to server

void signal_handler(int signal_n) {
    // Function that handles signals and allows the client to shut down gracefully
    if (signal_n == SIGINT) {
        printf("\nShutting down...\n");
        //Close the listening socket
        shutdown(listenSock, SHUT_RDWR);
        close(listenSock);
        exit(signal_n);
    }
}

void sendToServer(int serverSocket, const char *message) {
    // Function that sends message to server
    if (send(serverSocket, message, strlen(message), 0) < 0) {
        perror("Didn't work to send to server!");
    }
}

void command_prompt(int &sock) {
    // A function that mimicks a command prompt - prompts the user for a command until
    // user quits
    int buffer_size = 2048;
    char buffer[buffer_size];

    while (true) {
        cout << "Enter a command: ";
        bzero(buffer, buffer_size);
        fgets(buffer, sizeof(buffer) -1, stdin);

        sendToServer(sock, buffer);
        
        bzero(buffer, sizeof(buffer)); // Clearing the buffer
        
        if (recv(sock, buffer, sizeof(buffer) - 1, 0) == 0) {
            perror("Could not connect");
            exit(0);
        }

        cout << "\n" << buffer << endl;
    }
}

int main(int argc, char *argv[]) {

    int sock;
    struct sockaddr_in serv_addr;

    // Initialize memory
    memset(&serv_addr, 0, sizeof(serv_addr));

    sock = socket(AF_INET, SOCK_STREAM , 0);
    if (sock < 0) {
        perror("Failed to open socket");
        return(-1);
    }

    // Setup socket address structure for connections
    serv_addr.sin_family        = AF_INET;
    serv_addr.sin_addr.s_addr   = INADDR_ANY;
    serv_addr.sin_port          = htons(atoi(argv[1]));

    // Need to know the IP address of the server
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("failed to set socket address");
        exit(0);
    }

    // Connect to remote address
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Could not connect");
        exit(0);
    }

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        printf("\nCannot catch SIGINT!\n");
        exit(0);
    }
    
    command_prompt(sock);
}