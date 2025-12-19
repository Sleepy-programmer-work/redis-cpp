#ifndef REDIS_COMMAND_HANDLER_H
#define REDIS_COMMAND_HANDLER_H

#include <string>
#include <vector>

class RedisDatabase;

class RedisCommandHandler {
public:
    RedisCommandHandler();
    // Process a Redis command and return the response RESP FORMAT
    std::string processCommand(const std::string& command);

private:
    // Common Commands
    std::string handlePing(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleEcho(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleFlushAll(const std::vector<std::string>& tokens, RedisDatabase& db);

    // Key/Value Operations
    std::string handleSet(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleGet(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleKeys(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleType(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleDel(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleExpire(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleRename(const std::vector<std::string>& tokens, RedisDatabase& db);

    // List Operations
    std::string handleLpush(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleLpop(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleRpush(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleRpop(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleLlen(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleLindex(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleLget(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleLset(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleLrem(const std::vector<std::string>& tokens, RedisDatabase& db);

    // Hash Operations
    std::string handleHset(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleHget(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleHgetall(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleHexists(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleHdel(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleHkeys(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleHvals(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleHlen(const std::vector<std::string>& tokens, RedisDatabase& db);
    std::string handleHmset(const std::vector<std::string>& tokens, RedisDatabase& db);
};

#endif