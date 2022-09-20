// Simple server for TSAM-409 Programming Assignment 1
//
// Compile: g++ -Wall -std=c++11 server.cpp 
//
// Command line: ./server 5000 
//
// Author(s):
// Jacky Mallett (jacky@ru.is)
// Einar Örn Gissurarson (einarog05@ru.is)
// Stephan Schiffel (stephans@ru.is)
// Last modified: 20.08.2021
#include <stdio.h>
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
#include <array>

#ifndef SOCK_NONBLOCK
#include <fcntl.h>
#endif

#define BACKLOG  5          // Allowed length of queue of waiting connections

// predefined replies from the server
char SUCCESS_MESSAGE[] = "Command executed successfully\n"; 
char ERROR_MESSAGE[] = "Error executing command\n"; 
char MALFORMED_MESSAGE[] = "Unknown command\n"; 

//Global variable for graceful shutdown
int listenSock;   // Socket for connections to server

// Somewhat gracefully terminate program when user presses ctrl+c
void signal_handler( int signal_n)
{
    if (signal_n == SIGINT)
    {
        printf("\nShutting down...\n");
        //Close the listening socket
        shutdown(listenSock, SHUT_RDWR);
        close(listenSock);
        exit(signal_n);
    }
}

std::set<int> clients; // set of currently open client sockets (file descriptors)

// Open socket for specified port.
//
// Returns -1 if unable to create the socket for any reason.

int open_socket(int portno)
{
   struct sockaddr_in sk_addr;   // address settings for bind()
   int sock;                     // socket opened for this port
   int set = 1;                  // for setsockopt

   // Create socket for connection. Note: OSX doesn´t support SOCK_NONBLOCK
   // so we have to use a fcntl (file control) command there instead. 

   #ifndef SOCK_NONBLOCK
      if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
      {
         perror("Failed to open socket");
         return(-1);
      }

      int flags = fcntl(sock, F_GETFL, 0);

      if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
      {
         perror("Failed to set O_NONBLOCK");
      }
   #else
      if((sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK , IPPROTO_TCP)) < 0)
      {
         perror("Failed to open socket");
         return(-1);
      }
    #endif

   // Turn on SO_REUSEADDR to allow socket to be quickly reused after 
   // program exit.

   if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(set)) < 0)
   {
      perror("Failed to set SO_REUSEADDR:");
   }

   // Initialise memory
   memset(&sk_addr, 0, sizeof(sk_addr));

   // Set type of connection

   sk_addr.sin_family      = AF_INET;
   sk_addr.sin_addr.s_addr = INADDR_ANY;
   sk_addr.sin_port        = htons(portno);

   // Bind to socket to listen for connections from clients

   if(bind(sock, (struct sockaddr *)&sk_addr, sizeof(sk_addr)) < 0)
   {
      perror("Failed to bind to socket:");
      return(-1);
   }
   else
   {
      return(sock);
   }
}

// Close a client's connection, remove it from the client list, and
// tidy up select sockets afterwards.

void closeClient(int clientSocket, fd_set *openSockets, int *maxfds)
{
    close(clientSocket);      

    // If this client's socket is maxfds then the next lowest
    // one has to be determined. Socket fd's can be reused by the Kernel,
    // so there aren't any nice ways to do this.

    if(*maxfds == clientSocket)
    {
      if(clients.empty())
      {
        *maxfds = listenSock;
      }
      else
      {
        // clients is a sorted set, so the last element is the largest
        *maxfds = std::max(*clients.rbegin(), listenSock);
      }
    }

     // And remove from the list of open sockets.

     FD_CLR(clientSocket, openSockets);
}

// send a message to a client
// Note: This is not done well. E.g., send might fail if the send buffer is
// full. In that case the remaining message should be sent once the socket
// accepts more data (i.e., once it is returned in writefds by select).

void sendToClient(int clientSocket, const char *message)
{
    if (send(clientSocket, message, strlen(message), 0) < 0)
    {
        perror("Error sending message to client\n");
    }
}

// Process any message received from client on the server

void clientCommand(int clientSocket, fd_set *openSockets, int *maxfds, 
                  char *buffer) 
{
    std::vector<std::string> tokens;     // List of tokens in command from client
    std::string token;                   // individual token being parsed
    std::string result = "";
    std::vector<char> buffer_res(512);
    // Split command from client into tokens for parsing
    std::stringstream stream(buffer);

    // By storing them as a vector - tokens[0] is first word in string
    while(stream >> token)
        tokens.push_back(token);

    // Check if the command has atleast two items and check if the frist word is SYS
    if((tokens.size() >= 2) && (tokens[0].compare("SYS") == 0))
    {
        std::string stderr_to_stdout(" 2>&1 "); //I use this to get error messages to stdout - this is a bash way to redirect stderr to stdout
        std::string cmd_str = tokens[1].append(stderr_to_stdout);
        //build the command string
        for (int x=2;x<(int)tokens.size();x++)
        {
            cmd_str.append(tokens[x]);
            cmd_str.append(" ");
        }
        cmd_str.pop_back(); //remove the leading space
        //Open up a pointer that leads to the stdout stream from the command - that stdout stream includes stderr aswell due to line 187
        FILE* pipe = popen(cmd_str.c_str(),"r");
        /*
        The program will essentialy never send the ERROR_MESSAGE since stderr is being redirected
        to stdout and the pipe pointer does not differentiate between stderr and stdout, only way
        for the program to send the ERROR_MESSAGE is when popen is unable to get a pointer to an
        open stream from the output of the command, it would then return NULL
        */
        if (pipe == NULL) {
            sendToClient(clientSocket, ERROR_MESSAGE);
        }
        else
        {
            //read into success message
            buffer_res.clear(); //Clear the buffer
            //Read from the output stream by using fgets
            while(fgets(buffer_res.data(),512,pipe) != NULL)
            {
                //add to the output string that we send to the client
                result += buffer_res.data();
                // if buffer is full
                // make it 2x larger and continue
            }
            pclose(pipe); //Close the output stream
            std::cout << sizeof(result) << std::endl;
            char size = static_cast<char>(sizeof(result));
            sendToClient(clientSocket, &size); //this is disgusting
            std::cout << "why why why am in learning computer science" << std::endl;
            sendToClient(clientSocket, strdup(result.c_str())); //Send the output string to the client
        }
    }
else
    {
        //We go here when the first word in token is not "SYS"
        std::cout << "Unknown command from client:" << buffer << std::endl;
        sendToClient(clientSocket, MALFORMED_MESSAGE);
    }
}

int main(int argc, char* argv[])
{
    bool finished;
    int clientSock;                 // Socket of connecting client
    fd_set openSockets;             // Current open sockets 
    fd_set readSockets;             // Socket list for select()        
    fd_set exceptSockets;           // Exception socket list
    int maxfds;                     // Passed to select() as max fd in set
    struct sockaddr_in client;      // address of incoming client
    socklen_t clientLen;            // address length
    char buffer[1025];              // buffer for reading from clients
    std::vector<int> clientSocketsToClear; // List of closed sockets to remove

    if(argc != 2)
    {
        printf("Usage: server <ip port>\n");
        exit(0);
    }

    // Setup socket for server to listen to

    listenSock = open_socket(atoi(argv[1])); 

    printf("Listening on port: %d\n", atoi(argv[1]));
    printf("       (range of available ports on skel.ru.is is 4000-4100)\n");

    if(listen(listenSock, BACKLOG) < 0)
    {
        printf("Listen failed on port %s\n", argv[1]);
        exit(0);
    }
    else 
    // Add the listen socket to socket set
    {
        FD_SET(listenSock, &openSockets);
        maxfds = listenSock;
    }

    finished = false;

    //Attempt to gracefully shut down program on signal interrupt (ctrl+c)
    if (signal(SIGINT, signal_handler) == SIG_ERR)
    {
        printf("\nCannot catch SIGINT!\n");
    }

    while(!finished)
    {
        // Get modifiable copy of readSockets
        readSockets = exceptSockets = openSockets;
        memset(buffer, 0, sizeof(buffer));

        int n = select(maxfds + 1, &readSockets, NULL, &exceptSockets, NULL);

        if(n < 0)
        {
            perror("select failed - closing down\n");
            finished = true;
        }
        else
        {
            // Accept any new connections to the server
            if(FD_ISSET(listenSock, &readSockets))
            {
               clientSock = accept(listenSock, (struct sockaddr *)&client,
                                   &clientLen);

               FD_SET(clientSock, &openSockets);
               maxfds = std::max(maxfds, clientSock);

               clients.insert(clientSock);
               n--;

               printf("Client connected on server\n");
            }
            // Check for commands from already connected clients
            while(n-- > 0)
            {
               for(auto fd : clients)
               {
                  if(FD_ISSET(fd, &readSockets))
                  {
                      if(recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT) == 0)
                      {
                          printf("Client closed connection: %d", fd);

                          closeClient(fd, &openSockets, &maxfds);
                          // Remember that we need to remove the client from the list.
                          // Removing will be done outside the loop.
                          clientSocketsToClear.push_back(fd);

                      }
                      else // if something was recieved from the client
                      {
                          std::cout << buffer << std::endl;
                          // Attempt to execute client command
                          clientCommand(fd, &openSockets, &maxfds, buffer);
                      }
                  }
               }
               // Remove client from the clients list. This has to be done out of
               // the loop, since we can't modify the iterator inside the loop.
               for(auto fd : clientSocketsToClear)
               {
                   clients.erase(fd);
               }
               clientSocketsToClear.clear();
            }
        }
    }
}