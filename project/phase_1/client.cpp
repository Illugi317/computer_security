#include <iostream>
#include <string>
#include <fstream>
#include <jsoncpp/json/json.h>

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

    std::cout << "Reading response from server" << std::endl;
    
    return 
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
    Json::Value config = get_config(argv[1]);
    std::cout << "Config file opened successfully" << std::endl;
    // Get the server address and port from the config file
    std::string server_address = config["server"]["ip"].asString();
    int server_port = std::stoi(config["server"]["port"].asCString());

    std::cout << "Server address: " << server_address << std::endl;
    std::cout << "Server port: " << server_port  << std::endl;

    // Get Actions in the config file


    //Networking time
    int sock;
    struckt sockaddr_in serv_addr;
    char buffer[1024] = {0};
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
    std::string uuid = config["uuid"].asString();
    std::cout << "Sending UUID: " << uuid << " to server" << std::endl;
    send(sock , uuid.c_str() , uuid.length() , 0 );
    std::cout << "UUID sent" << std::endl;
    // Read response from server
    char* response = read_response(sock);
    std::cout << "Response from server: " << response << std::endl;
    return 0;
}

    //Get aknowledgement from server
    //Send password to the server
    //Get aknowledgement from server
    //Send actions to server with DELAY
    //After that we are done and can close the connection
}
