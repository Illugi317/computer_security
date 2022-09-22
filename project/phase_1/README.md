Computer security project phase-1

Client-server arcitecture
 

Server passively online

When client starts it will read a config file that is in a JSON format like such:

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

Then connects to the server with the id and then if the server confirms the ID then the password will become accepted and the server will start with the ACTIONS with the set delay in seconds. Identical users can use the same ID and change the same values with the ACTIONS at the same time. {Threading and Locks for this}

Once the last user disconnects the server drops all memory and writes everything into a logfile regarding that client respectofly.

TODO:
# Client
Redo all of client
read config from json CHECK
connect to server CHECK
send steps CHECK
establish password and such
# Server
Logging
ENUM -> Struct
