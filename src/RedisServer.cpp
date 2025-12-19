#include "../include/RedisServer.h"
#include "../include/RedisCommandHandler.h"
#include "../include/RedisDatabase.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <thread>
#include <vector>
#include <cstring>
#include <signal.h>

static RedisServer* globalServer = nullptr; // global pointer to the RedisServer instance

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    if (globalServer) {
        std::cout << "\n"<<signum<<"Shutting down Redis server gracefully...\n";
        globalServer->shutdown();
    }
    exit(signum);
}

void RedisServer::setupSignalHandlers() {
    signal(SIGINT, signalHandler); 
} 


RedisServer::RedisServer(int port) : port(port) , server_socket(-1) , running(true){
    globalServer = this;// set the global server pointer

}

void RedisServer::shutdown(){
    running = false;
    if(server_socket != -1){
        close(server_socket);
    }
    std::cout << "Server shutdown initiated." << std::endl;
}

void RedisServer::run(){
    server_socket = socket(AF_INET,SOCK_STREAM ,0);// create a TCP socket 
    //AF_INET for IPv4,SOCK_STREAM for TCP,0 for default protocol 
    if(server_socket < 0){
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));// set socket options  
    //SOL_SOCKET to manipulate the socket at the socket level, SO_REUSEADDR to allow reuse of local addresses
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // IPv4
    serverAddr.sin_port = htons(port); // set port,htons to convert to network byte order
    serverAddr.sin_addr.s_addr = INADDR_ANY; // bind to any available interface

    if(bind(server_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        std::cerr << "Failed to bind socket." << std::endl;
        close(server_socket);
        return;
    }

    if(listen(server_socket,10)<0){
        std::cerr << "Failed to listen on socket." << std::endl;
        close(server_socket);
        return;
    }

    std::cout << "Server is running on port " << port << std::endl;

    std::vector<std::thread> threads;
    RedisCommandHandler cmdHandler;

    while(running){
        int client_socket = accept (server_socket, nullptr, nullptr);// accept incoming connection
        if(client_socket < 0){
            if(running){
                std::cerr << "Failed to accept connection." << std::endl;
            }
            break;
        }

        threads.emplace_back([client_socket, &cmdHandler]() {
            char buffer[1024];
            while(true){
                memset(buffer, 0, sizeof(buffer));// clear the buffer & memset used to fill a block of memory with a particular value
                int bytes = recv(client_socket, buffer, sizeof(buffer)-1, 0);// receive data from client
                if(bytes <= 0){
                    break; // connection closed or error
                }
                std::string request(buffer, bytes);// construct string from received data
                std::string response = cmdHandler.processCommand(request);// process the command
                send(client_socket, response.c_str(), response.size(), 0);// send response back to client
            }
            close(client_socket);// close client socket
        });
    }

    // Join all threads before exiting because they are handling client connections
    //if we don't join them, they may continue running after server shutdown
    for(auto& t : threads){
        if(t.joinable()){
            t.join();
        }
    }

    //before shutting down the server, we load the database from db
    if(!RedisDatabase::getInstance().dump("dump.my_rdb")){
        std::cerr << "Failed to dump database to dump.my_rdb during shutdown." << std::endl;
    } else {
        std::cout << "Database dumped to dump.my_rdb successfully during shutdown." << std::endl;
    }

}