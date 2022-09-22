#include <iostream>
#include <string>
#include <fstream>
#include <jsoncpp/json/json.h>
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
#include <algorithm>
#include <set>
#include <vector>
#include <signal.h>
#include <sstream>
#include <thread>
#include <array>
#include <cstring>
#include <chrono>

// I have no idea what's being used and what's not. Shit compiles and im happy about that.

char AKNOWlEDGEMENT_MESSAGE[] = "ACK";

Json::Value get_config(char* config_file)
{
    // Read config file and throw error if it fails
    Json::Value config;
    try
    {
        std::ifstream config_stream(config_file);
        config_stream >> config;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return config;
}

void send_message(std::string message, int socket)
{
    // Send message to server
    // std::cout << "Sending message: " << message << " to " << ip << ":" << port << std::endl;
    int send_err = send(socket, message.c_str(), message.length(), 0);
    if (send_err < 0)
    {
        std::cerr << "Error sending message" << std::endl;
    }
}

char* read_response(int socket)
{
    // Read response from server
    char* buffer = new char[1024];
    memset(buffer, 0, 1024);
    std::cout << "Reading response from server" << std::endl;
    int read_err = read(socket, buffer, 1024);
    if (read_err < 0)
    {
        std::cerr << "Error reading response" << std::endl;
    }
    return buffer;
}

bool validate_message(char* message)
{
    // Validate message
    if (strcmp(message,AKNOWlEDGEMENT_MESSAGE) == 0)
    {
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    // Check if we get the correct number of arguments
    if (argc != 2) {
        std::cout << "Usage: <filepath/name>" << std::endl;
        return 1;
    }
    // We got the correct arguments and now try to open the config file.
    std::cout << "Trying to open config file: " << argv[1] << std::endl;
    const Json::Value config = get_config(argv[1]);
    std::cout << "Config file opened successfully" << std::endl;
    // Get the server address and port from the config file
    std::string server_address = config["server"]["ip"].asString();
    int server_port = std::stoi(config["server"]["port"].asCString());

    std::cout << "Server address: " << server_address << std::endl;
    std::cout << "Server port: " << server_port  << std::endl;

    // Get Delay in the config file
    int delay = std::stoi(config["actions"]["delay"].asCString());

    // Steps is an array of strings in the config file and we need to iterate over it
    const Json::Value steps = config["actions"]["steps"];
    std::vector<std::string> steps_vector;
    steps_vector.reserve(steps.size());
    std::transform(steps.begin(), steps.end(), std::back_inserter(steps_vector), [](const Json::Value& v) { return v.asString(); }); // what the fuck?
    //Networking time
    int sock;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "\n Socket creation error \n" << std::endl;
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, server_address.c_str(), &serv_addr.sin_addr)<=0)
    {
        std::cout << "\nInvalid address/ Address not supported \n" << std::endl;
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cout << "\nConnection Failed \n" << std::endl;
        return -1;
    }
    // Connection worked and we are connected to the server now.
    std::cout << "Connected to server" << std::endl;


    //Send UUID to server
    send_message(config["id"].asCString(), sock);
    std::cout << "UUID sent" << std::endl;
    
    // Read response from server
    char* response = read_response(sock);
    std::cout << "Response from server: " << response << std::endl;
    if (validate_message(response))
    {
        std::cout << "Message validated" << std::endl;
        //send password
        send_message(config["password"].asCString(), sock);
        std::cout << "Password sent" << std::endl;
        // Read response from server
        char* response = read_response(sock);
        std::cout << "Response from server: " << response << std::endl;
        if (validate_message(response))
        {
            std::cout << "Message validated" << std::endl;
            //send actions
            std::cout << "Sending actions to server" << std::endl;
            for(size_t i=0; i<steps_vector.size(); i++)
            {
                std::cout << "Sending action: " << steps_vector[i] << std::endl;
                send_message(steps_vector[i], sock);
                std::this_thread::sleep_for(std::chrono::milliseconds(delay*1000));
            }
            close(sock);
        }
    }
    else
    {
        std::cout << "shits broken" << std::endl;
    }
    return 0;
    //Get aknowledgement from server
    //Send password to the server
    //Get aknowledgement from server
    //Send actions to server with DELAY
    //After that we are done and can close the connection
}