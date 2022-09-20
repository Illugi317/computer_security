#include <iostream>
#include <string>
#include <fstream>
#include <jsoncpp/json/json.h>

int main(int argc, char *argv[])
{
    // Check if we get the correct number of arguments
    if (argc != 2) {
        std::cout << "Usage: <filepath/name>" << std::endl;
        return 1;
    }
    // We got the correct arguments and now try to open the config file.
    std::cout << "Trying to open config file: " << argv[1] << std::endl;
    try{
    Json::Value config;                                         // The config object file in memory
    std::ifstream config_file(argv[1], std::ifstream::binary);  // read the file
    config_file >> config;                                      // parse the file
    std::cout << config["server"]["ip"] << std::endl;
    std::cout << config["server"]["port"] << std::endl;
    } catch (std::exception &e) {
        std::cout << "Could not open config file" << std::endl;
        exit(1);
    }
}
