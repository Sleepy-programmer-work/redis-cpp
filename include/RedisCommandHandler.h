#ifndef REDIS_COMMAND_HANDLER_H
#define REDIS_COMMAND_HANDLER_H

#include <string>

class RedisCommandHandler {
public:
    RedisCommandHandler();
    // Process a Redis command and return the response RESP FORMAT
    std::string processCommand(const std::string& command);
};

#endif