// Server
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
#include <fstream>
#include <ctime>
#include <thread>
#include <array>

#ifndef SOCK_NONBLOCK
#include <fcntl.h>
#endif

#define BACKLOG  5          // Allowed length of queue of waiting connections


enum class state { ID, PASSWORD, COMMAND };
// Pair Struct
struct Pair
{
    long long int counter;
    std::string uuid;
    size_t hashed_password = 0;
    int current_state;
    int socket;
    std::string DEBUGcurrent_state;
};

std::vector<Pair> counters;
std::hash<std::string> hash_fn;
// predefined replies from the server
char SUCCESS_MESSAGE[] = "Command executed successfully\n"; 
char ERROR_MESSAGE[] = "ERROR"; 
char MALFORMED_MESSAGE[] = "Unknown command\n"; 

char ACKNOWLEDGEMENT_MESSAGE[] = "ACK";
//char ERROR_MESSAGE[] = "ERROR\n";

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

auto get_pair(int socket)
{
    // Get counter for uuid
    for (auto& pair : counters) 
    {
        if (pair.socket == socket)
        {
            return &pair;
        }
    }
    throw std::runtime_error("not found");
}
bool check_id(int client_socket, char* buffer)
{
    try
    {
        //check if id is in a pair in the counter vector
        Pair* count = get_pair(client_socket);
        return true;
    }
    catch(const std::exception& e)
    {
        // add pair to counter vector
        Pair count;
        count.uuid = std::string(buffer);
        count.counter = 0;
        count.socket = client_socket;
        //count.current_state = state::ID;
        count.current_state = 1;
        count.DEBUGcurrent_state = "ID";
        counters.push_back(count);
        return false;
    }
}
bool check_password(int client_socket, char* buffer)
{
    //check if password is set
    //if password is set hash the string buffer and then check if it's the same as the password
    //if password is not set, has the current buffer and continue the flow.
    Pair* count = get_pair(client_socket);
    if ((*count).hashed_password == 0)
    {
        count->hashed_password = hash_fn(std::string(buffer));
        return true;
    }
    else
    {
        size_t hashed = hash_fn(std::string(buffer));
        if ((*count).hashed_password == hashed)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

auto get_current_time()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    return std::string(std::ctime(&in_time_t));
}

void write_logfile(int client_socket, char* buffer, std::string uuid, long long int counter)
{
    //write to logfile
    //std::ofstream logfile;
    //logfile.open("logfile.txt", std::ios_base::app)
    // FORMAT OF LOGFILE: <timestamp> <client_socket OR ID> <message>;
    // message can be whatever but problably first with INCREASE and DECREASE but then later on add connected and disconnected
    std::ofstream logfile;
    logfile.open("logfile.log", std::fstream::app);
    std::string time = get_current_time();
    //chomp newline
    time.erase(std::remove(time.begin(), time.end(), '\n'), time.cend());
    logfile << time << " - " << uuid << " - " << std::string(buffer)<< " - " << counter << std::endl;
    logfile.close();
}

void client_command(int client_socket, fd_set* open_sockets, int* maxfds, char* buffer)
{
    /*
    States:

    0: ID ack'ed waiting for password
    1: Password hashed and ack'ed
    2: Parse commands

    */
   //check state
    check_id(client_socket, buffer);
    Pair* count = get_pair(client_socket);
    std::cout << "Current state: " << (*count).DEBUGcurrent_state << std::endl;
    switch ((*count).current_state)
    {   
        case 1:
            if (check_id(client_socket, buffer))
            {
                //count.current_state = state::PASSWORD;
                //increment_enum(count);
                (*count).DEBUGcurrent_state = "PASSWORD";
                ++(*count).current_state;
                sendToClient(client_socket, ACKNOWLEDGEMENT_MESSAGE);
            }
            break;
        case 2:
            if (check_password(client_socket, buffer))
            {
                //increment_enum(count);
                //count.current_state = state::COMMAND;
                ++(*count).current_state;
                (*count).DEBUGcurrent_state = "COMMAND";
                sendToClient(client_socket, ACKNOWLEDGEMENT_MESSAGE);
            }
            else
            {
                sendToClient(client_socket, ERROR_MESSAGE);
                closeClient(client_socket, open_sockets, maxfds);
            }
            break;
        case 3:
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
            if(tokens.size() == 2)
            {
                Pair* count = get_pair(client_socket);
                if(tokens[0].compare("INCREASE") == 0)
                {
                    //Find a way to define the socket as a client socket
                    count -> counter = (*count).counter + stoi(tokens[1]);
                    write_logfile(client_socket, buffer, (*count).uuid, (*count).counter);
                    std::cout << "Increase - Counter: " << (*count).counter << std::endl;
                }
                else if(tokens[0].compare("DECREASE") == 0)
                {
                    count -> counter = (*count).counter - stoi(tokens[1]);
                    write_logfile(client_socket, buffer, (*count).uuid, (*count).counter);
                    std::cout << "Decrease - Counter: " << (*count).counter << std::endl;
                }
            }
            else
            {
                std::cout << "Unknown command from client:" << buffer << std::endl;
                sendToClient(client_socket, MALFORMED_MESSAGE);
            }
        }
            break;
        default:
            break;
    }
 /*
    //COMMAND ITSELF    
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
    if(tokens.size() == 2)
    {
        if(tokens[0].compare("INCREASE") == 0)
        {
            //Find a way to define the socket as a client socket
            Pair count = get_pair(client_socket);
            count.counter = count.counter + stoi(tokens[1]);
        
        }
        else if(tokens[0].compare("DECREASE") == 0)
        {
            Pair count = get_pair(client_socket);
            count.counter = count.counter - stoi(tokens[1]);
        }
    }
else
    {
        //Not a valid command
        std::cout << "Unknown command from client:" << buffer << std::endl;
        sendToClient(client_socket, MALFORMED_MESSAGE);
    }
    */
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
                      if(recv(fd, buffer, sizeof(buffer), 0) == 0)
                      {
                          printf("Client closed connection: %d", fd);

                          closeClient(fd, &openSockets, &maxfds);
                          // Remember that we need to remove the client from the list.
                          // Removing will be done outside the loop.
                          clientSocketsToClear.push_back(fd);

                      }
                      else // if something was recieved from the client
                      {
                            // Attempt to execute client command
                            client_command(fd, &openSockets, &maxfds, buffer);
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