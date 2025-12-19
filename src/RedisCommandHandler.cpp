#include "../include/RedisCommandHandler.h"
#include "../include/RedisDatabase.h"
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <exception>
/*Resp parser :- *2\r\n$4\r\n\PING\r\n$4\r\nTEST\r\n
 * -> array
 *2-> array of 2 elements
 $4-> bulk string of length 4 next
 PING -> actual string
 \r\n -> carriage return and newline
 TEST -> actual string */

 std::vector<std::string> parseRespoCommand(const std::string& input) {
    std::vector<std::string> tokens;
    if(input.empty() ) {
        return tokens; // Invalid RESP format
    }

    // Simple case: if the input does not start with '*', treat it as a single line command
    if(input[0]!= '*'){
        std::istringstream iss(input);
        std::string token;
        while(iss >> token){
            tokens.push_back(token);
        }
        return tokens;
    }

    size_t pos = 0;
    if(input[pos]!= '*'){
        return tokens; // Invalid RESP format
    }
    pos++;

    size_t crlf= input.find("\r\n", pos); //crlf - carriage return (\r)and line feed (\n)
    if(crlf == std::string::npos){
        return tokens; // Invalid RESP format
    }

    int numElements = std::stoi(input.substr(pos, crlf - pos));
    pos = crlf + 2; // Move past \r\n

    // Parse each bulk string element 
    for(int i=0; i< numElements; ++i){
        if(pos >= input.size() || input[pos]!= '$'){
            return tokens; // Invalid RESP format
        }
        pos++;

        crlf = input.find("\r\n", pos);
        if(crlf == std::string::npos){
            return tokens; // Invalid RESP format
        }

        int strLength = std::stoi(input.substr(pos, crlf - pos));
        pos = crlf + 2; // Move past \r\n

        if(pos + strLength > input.size()){
            return tokens; // Invalid RESP format
        }

        std::string element = input.substr(pos, strLength);
        tokens.push_back(element);
        pos += strLength + 2; // Move past the string and \r\n
    }
    return tokens;
}

RedisCommandHandler::RedisCommandHandler() {}

// Process a Redis command and return the response in RESP FORMAT
std::string RedisCommandHandler::processCommand(const std::string& command) {

    auto tokens = parseRespoCommand(command);// parse the command into tokens
    if(tokens.empty()){
        return "-ERR invalid command format\r\n";   
    }
    /*
    for(auto& t : tokens){
        std::cout << t << "\n"<< std::endl;
    }
    */
    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper); // convert command to uppercase
    std::ostringstream response;// to build the response string

    RedisDatabase& db = RedisDatabase::getInstance();

    // Handle PING command
    if (cmd == "PING") {
        if (tokens.size() == 1) {
            response << "+PONG\r\n";
        } else {
            // Return the argument as bulk string
            response << "$" << tokens[1].size() << "\r\n" << tokens[1] << "\r\n";
        }
    }
    // Handle ECHO command
    else if (cmd == "ECHO") {
        if (tokens.size() < 2) {
            response << "-ERR wrong number of arguments for 'echo' command\r\n";
        } else {
            response << "$" << tokens[1].size() << "\r\n" << tokens[1] << "\r\n";
        }
    }
    // Handle SET command
    else if (cmd == "SET") {
        if (tokens.size() < 3) {
            response << "-ERR wrong number of arguments for 'set' command\r\n";
        } else {
            // For now, just acknowledge the SET command
            response << "+OK\r\n";
        }
    }
    // Handle GET command
    else if (cmd == "GET") {
        if (tokens.size() < 2) {
            response << "-ERR wrong number of arguments for 'get' command\r\n";
        } else {
            // For now, return nil
            response << "$-1\r\n";
        }
    }

    //hash functions, list functions, set functions, sorted set functions etc.
    /*
    PING,ECHO,FLUSHALL
    SET,GET,KEYS,TYPE,DEL,EXPIRE,RENAME
    HSET,HGET,HGETALL,HEXISTS,HDEL,HKEYS,HLEN,HVALS,HGETALL,HSETALL
    LGET,LSET,LPUSH,RPOP,LLEN,LRANGE,LINDEX,LSET
    */
    // Handle unknown commands
    else {
        response << "-ERR unknown command '" << cmd << "'\r\n";
    }

    return response.str();
}


