#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H

#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>

class RedisDatabase {
public:
    static RedisDatabase& getInstance();

    //Persistenance - dump and load from a file
    bool dump(const std::string& filename);
    bool load(const std::string& filename);

private:
    //private constructor to prevent instantiation
    //private destructor to prevent deletion
    RedisDatabase() = default;
    ~RedisDatabase() = default;
    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator=(const RedisDatabase&) = delete;

    std::mutex db_mutex; // mutex for thread-safe database operations
    std::unordered_map<std::string, std::string> kv_store; // simple key-value store
    std::unordered_map<std::string, std::vector<std::string>> list_store; // simple list store
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store; // simple hash store

};

#endif 