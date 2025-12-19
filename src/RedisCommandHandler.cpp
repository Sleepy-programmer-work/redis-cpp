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
    auto tokens = parseRespoCommand(command);
    if (tokens.empty()) {
        return "-ERR invalid command format\r\n";
    }

    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    RedisDatabase& db = RedisDatabase::getInstance();

    // Dispatch to handlers
    if (cmd == "PING")
        return handlePing(tokens, db);
    else if (cmd == "ECHO")
        return handleEcho(tokens, db);
    else if (cmd == "FLUSHALL")
        return handleFlushAll(tokens, db);
    else if (cmd == "SET")
        return handleSet(tokens, db);
    else if (cmd == "GET")
        return handleGet(tokens, db);
    else if (cmd == "KEYS")
        return handleKeys(tokens, db);
    else if (cmd == "TYPE")
        return handleType(tokens, db);
    else if (cmd == "DEL" || cmd == "UNLINK")
        return handleDel(tokens, db);
    else if (cmd == "EXPIRE")
        return handleExpire(tokens, db);
    else if (cmd == "RENAME")
        return handleRename(tokens, db);
    else if (cmd == "LPUSH")
        return handleLpush(tokens, db);
    else if (cmd == "LPOP")
        return handleLpop(tokens, db);
    else if (cmd == "RPUSH")
        return handleRpush(tokens, db);
    else if (cmd == "RPOP")
        return handleRpop(tokens, db);
    else if (cmd == "LLEN")
        return handleLlen(tokens, db);
    else if (cmd == "LGET")
        return handleLget(tokens, db);
    else if (cmd == "LINDEX")
        return handleLindex(tokens, db);
    else if (cmd == "LSET")
        return handleLset(tokens, db);
    else if (cmd == "LREM")
        return handleLrem(tokens, db);
    else if (cmd == "HSET")
        return handleHset(tokens, db);
    else if (cmd == "HGET")
        return handleHget(tokens, db);
    else if (cmd == "HGETALL")
        return handleHgetall(tokens, db);
    else if (cmd == "HEXISTS")
        return handleHexists(tokens, db);
    else if (cmd == "HDEL")
        return handleHdel(tokens, db);
    else if (cmd == "HKEYS")
        return handleHkeys(tokens, db);
    else if (cmd == "HVALS")
        return handleHvals(tokens, db);
    else if (cmd == "HLEN")
        return handleHlen(tokens, db);
    else if (cmd == "HMSET")
        return handleHmset(tokens, db);
    else
        return "-ERR unknown command '" + cmd + "'\r\n";
}


