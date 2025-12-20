#include "../include/RedisCommandHandler.h"
#include "../include/RedisDatabase.h"
#include <sstream>
#include <string>

//Common Commands

std::string RedisCommandHandler::handlePing(const std::vector<std::string>& tokens, RedisDatabase& db) {
    std::ostringstream response;
    if (tokens.size() == 1) {
        response << "+PONG\r\n";
    } else {
        response << "$" << tokens[1].size() << "\r\n" << tokens[1] << "\r\n";
    }
    return response.str();
}

std::string RedisCommandHandler::handleEcho(const std::vector<std::string>& tokens, RedisDatabase& db) {
    std::ostringstream response;
    if (tokens.size() < 2) {
        response << "-ERR wrong number of arguments for 'echo' command\r\n";
    } else {
        response << "$" << tokens[1].size() << "\r\n" << tokens[1] << "\r\n";
    }
    return response.str();
}

std::string RedisCommandHandler::handleFlushAll(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (db.flushAll()) {
        return "+OK\r\n";
    }
    return "-ERR could not flush database\r\n";
}

//Key/Value Operations 

std::string RedisCommandHandler::handleSet(const std::vector<std::string>& tokens, RedisDatabase& db) {
    std::ostringstream response;
    if (tokens.size() < 3) {
        response << "-ERR wrong number of arguments for 'set' command\r\n";
    } else {
        db.set(tokens[1], tokens[2]);
        response << "+OK\r\n";
    }
    return response.str();
}

std::string RedisCommandHandler::handleGet(const std::vector<std::string>& tokens, RedisDatabase& db) {
    std::ostringstream response;
    if (tokens.size() < 2) {
        response << "-ERR wrong number of arguments for 'get' command\r\n";
    } else {
        std::string value;
        if (db.get(tokens[1], value)) {
            response << "$" << value.size() << "\r\n" << value << "\r\n";
        } else {
            response << "$-1\r\n";
        }
    }
    return response.str();
}

std::string RedisCommandHandler::handleKeys(const std::vector<std::string>& tokens, RedisDatabase& db) {
    std::ostringstream response;
    auto keys = db.keys();
    response << "*" << keys.size() << "\r\n";
    for (const auto& key : keys) {
        response << "$" << key.size() << "\r\n" << key << "\r\n";
    }
    return response.str();
}

std::string RedisCommandHandler::handleType(const std::vector<std::string>& tokens, RedisDatabase& db) {
    std::ostringstream response;
    if (tokens.size() < 2) {
        response << "-ERR wrong number of arguments for 'type' command\r\n";
    } else {
        std::string type = db.type(tokens[1]);
        response << "+" << type << "\r\n";
    }
    return response.str();
}

std::string RedisCommandHandler::handleDel(const std::vector<std::string>& tokens, RedisDatabase& db) {
    std::ostringstream response;
    if (tokens.size() < 2) {
        response << "-ERR wrong number of arguments for 'del' command\r\n";
    } else {
        bool deleted = db.del(tokens[1]);
        response << ":" << (deleted ? 1 : 0) << "\r\n";
    }
    return response.str();
}

std::string RedisCommandHandler::handleExpire(const std::vector<std::string>& tokens, RedisDatabase& db) {
    std::ostringstream response;
    if (tokens.size() < 3) {
        response << "-ERR wrong number of arguments for 'expire' command\r\n";
    } else {
        bool result = db.expire(tokens[1], std::stoi(tokens[2]));
        response << ":" << (result ? 1 : 0) << "\r\n";
    }
    return response.str();
}

std::string RedisCommandHandler::handleRename(const std::vector<std::string>& tokens, RedisDatabase& db) {
    std::ostringstream response;
    if (tokens.size() < 3) {
        response << "-ERR wrong number of arguments for 'rename' command\r\n";
    } else {
        bool result = db.rename(tokens[1], tokens[2]);
        if (result) {
            response << "+OK\r\n";
        } else {
            response << "-ERR no such key\r\n";
        }
    }
    return response.str();
}

//List Operations

std::string RedisCommandHandler::handleLpush(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3){ 
        return "-Error: LPUSH requires key and value\r\n";
    }
    for (size_t i = 2; i < tokens.size(); ++i) {
        db.lpush(tokens[1], tokens[i]);
    }
    ssize_t len = db.llen(tokens[1]);
    return ":" + std::to_string(len) + "\r\n";
}

std::string RedisCommandHandler::handleLpop(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2) {
        return "-ERR wrong number of arguments for 'lpop' command\r\n";
    }
    std::string value;
    if (db.lpop(tokens[1], value)) {
        return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    } else {
        return "$-1\r\n";
    }
}

std::string RedisCommandHandler::handleRpush(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3){
        return "-Error: RPUSH requires key and value\r\n";
    }
    for (size_t i = 2; i < tokens.size(); ++i) {
        db.rpush(tokens[1], tokens[i]);
    }    
    ssize_t len = db.llen(tokens[1]);
    return ":" + std::to_string(len) + "\r\n";
}

std::string RedisCommandHandler::handleRpop(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2){ 
        return "-Error: RPOP requires key\r\n";
    }
    std::string val;
    if (db.rpop(tokens[1], val))
        return "$" + std::to_string(val.size()) + "\r\n" + val + "\r\n";
    return "$-1\r\n";
}

std::string RedisCommandHandler::handleLlen(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if(tokens.size() < 2){
        return "-ERR wrong number of arguments for 'llen' command\r\n";
    }
    ssize_t length = db.llen(tokens[1]);
    return ":" + std::to_string(length) + "\r\n";
}

std::string RedisCommandHandler::handleLindex(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3){ 
        return "-Error: LINDEX requires key and index\r\n";
    }
    try {
        int index = std::stoi(tokens[2]);
        std::string value;
        if (db.lindex(tokens[1], index, value)) 
            return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
        else 
            return "$-1\r\n";
    } catch (const std::exception&) {
        return "-Error: Invalid index\r\n";
    }
}

std::string RedisCommandHandler::handleLget(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2){
        return "-Error: LGET requires a key\r\n";
    }
    auto elems = db.lget(tokens[1]);
    std::ostringstream oss;
    oss << "*" << elems.size() << "\r\n";
    for (const auto& e : elems) {
        oss << "$" << e.size() << "\r\n"
            << e << "\r\n";
    }
    return oss.str();
}
   
std::string RedisCommandHandler::handleLset(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4){ 
        return "-Error: LSET requires key, index and value\r\n";
    }
    try {
        int index = std::stoi(tokens[2]);
        if (db.lset(tokens[1], index, tokens[3]))
            return "+OK\r\n";
        else 
            return "-Error: Index out of range\r\n";
    } catch (const std::exception&) {
        return "-Error: Invalid index\r\n";
    }
}

std::string RedisCommandHandler::handleLrem(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4){ 
        return "-Error: LREM requires key, count and value\r\n";
    }
    try {
        int count = std::stoi(tokens[2]);
        int removed = db.lrem(tokens[1], count, tokens[3]);
        return ":" +std::to_string(removed) + "\r\n";
    } catch (const std::exception&) {
        return "-Error: Invalid count\r\n";
    }
}

//Hash Operations

std::string RedisCommandHandler::handleHset(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4) 
        return "-Error: HSET requires key, field and value\r\n";
    db.hset(tokens[1], tokens[2], tokens[3]);
    return ":1\r\n";
}

std::string RedisCommandHandler::handleHget(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3) 
        return "-Error: HGET requires key and field\r\n";
    std::string value;
    if (db.hget(tokens[1], tokens[2], value))
        return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    return "$-1\r\n";
}

std::string RedisCommandHandler::handleHexists(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3) 
        return "-Error: HEXISTS requires key and field\r\n";
    bool exists = db.hexists(tokens[1], tokens[2]);
    return ":" + std::to_string(exists ? 1 : 0) + "\r\n";
}

std::string RedisCommandHandler::handleHdel(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3) 
        return "-Error: HDEL requires key and field\r\n";
    bool res = db.hdel(tokens[1], tokens[2]);
    return ":" + std::to_string(res ? 1 : 0) + "\r\n";
}

std::string RedisCommandHandler::handleHgetall(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2) 
        return "-Error: HGETALL requires key\r\n";
    auto hash = db.hgetall(tokens[1]);
    std::ostringstream oss;
    oss << "*" << hash.size() * 2 << "\r\n";
    for (const auto& pair: hash) {
        oss << "$" << pair.first.size() << "\r\n" << pair.first << "\r\n";
        oss << "$" << pair.second.size() << "\r\n" << pair.second << "\r\n";
    }
    return oss.str();
}

std::string RedisCommandHandler::handleHkeys(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2) 
        return "-Error: HKEYS requires key\r\n";
    auto keys = db.hkeys(tokens[1]);
    std::ostringstream oss;
    oss << "*" << keys.size() << "\r\n";
    for (const auto& key: keys) {
        oss << "$" << key.size() << "\r\n" << key << "\r\n";
    }
    return oss.str();
}

std::string RedisCommandHandler::handleHvals(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2) 
        return "-Error: HVALS requires key\r\n";
    auto values = db.hvals(tokens[1]);
    std::ostringstream oss;
    oss << "*" << values.size() << "\r\n";
    for (const auto& val: values) {
        oss << "$" << val.size() << "\r\n" << val << "\r\n";
    }
    return oss.str();
}

std::string RedisCommandHandler::handleHlen(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2) 
        return "-Error: HLEN requires key\r\n";
    ssize_t len = db.hlen(tokens[1]);
    return ":" + std::to_string(len) + "\r\n";
}

std::string RedisCommandHandler::handleHmset(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4 || (tokens.size() % 2) == 1) 
        return "-Error: HMSET requires key followed by field value pairs\r\n";
    std::vector<std::pair<std::string, std::string>> fieldValues;
    for (size_t i = 2; i < tokens.size(); i += 2) {
        fieldValues.emplace_back(tokens[i], tokens[i+1]);
    }
    db.hmset(tokens[1], fieldValues);
    return "+OK\r\n";
}

