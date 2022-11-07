Computer security project phase-1 <br>
Client-server arcitecture <br>
Server passively online <br>

When client starts it will read a config file that is in a JSON format like such:
```
{
    "id" : "ID",
    "password" : "PASSWORD",
    "server" :
    {
        "ip" : "SERVER_IP" ,
        "port" : "PORT"
    } ,
"actions" : {
    "delay" : "DELAY IN SECONDS" ,
    "steps" : [
        "ACTION 1" ,
        "ACTION 2" ,
        "..."
    ]
    }
}
``` 

Then connects to the server with the id and then if the server confirms the ID then the password will become accepted and the server will start with the ACTIONS with the set delay in seconds. Identical users can use the same ID and change the same values with the ACTIONS at the same time. {Threading and Locks for this}

Once the last user disconnects the server drops all memory and writes everything into a logfile regarding that client respectofly.

TODO:
# how to
## Environment
This was devoloped on WSL with OS: Ubuntu 20.04 LTS on Windows 10 x86_64 with Kernel: 5.10.16.3-microsoft-standard-WSL2

### Server
To run the server you need to comile it first. To do that you need to have the following installed:
* gcc
* libjsoncpp-dev
* libjsoncpp1

And then you need to run the command ``` make ``` to compile the server and the client.

After that you can run the server like so ``` ./server.out <port> ``` and it will listen on the port you specified.

### Client
When you have compiled the server you can run the client like so ``` ./client.out <config file> ``` and it will connect to the server and start the actions.

# how it functions
Client sends UUID to the server   
Server checks if the UUID is in the database if not it will add it and send a confirmation to the client  
Client sends password to the server  
Server checks if the password is set if not it will set it and send a confirmation to the client if its already set it will hash it and try to match the current one.  
Client sends actions to the server  
Server will execute the actions  
Client disconnects  
Server will remove the UUID from the database   

# Technical details
The client is very verbose on what it's currently doing, and the server is not. It does however log every action to a file.  
The Server is multithreaded and when a client connects a new thread is created for that client to authenticate it. After it's authenticated its added to a list of clients and the thread is closed. The main thread checks for actions sent from the client. 

