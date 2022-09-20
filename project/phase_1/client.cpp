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
}
