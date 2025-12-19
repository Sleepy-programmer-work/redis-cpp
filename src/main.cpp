#include "../include/RedisServer.h"
#include "../include/RedisDatabase.h"
#include <iostream>
#include <thread>
#include <chrono>    
#include <csignal>
#include <signal.h>


int main(int argc, char* argv[]) {
    int port = 6379;
    if(argc >= 2){
        port = std::stoi(argv[1]);
    }
    RedisServer server(port);

    //Background persistance thread - dumping the database every 300 seconds((5*60 save databse to disk))

    std::thread persistenceThread([&server]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(300));
            // the database 
            if(!RedisDatabase::getInstance().dump("dump.my_rdb")){
                std::cerr << "Failed to dump database to disk." << std::endl;
            } else {
                std::cout << "Database dumped to dump.my_rdb successfully." << std::endl;
            }
        }
    });
    persistenceThread.detach();
    server.run();
    return 0;
}