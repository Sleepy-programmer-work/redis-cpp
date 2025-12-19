#include "../include/RedisDatabase.h"
#include <fstream>
#include <mutex>
#include <iostream>
#include <exception>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>

RedisDatabase& RedisDatabase::getInstance() {
    static RedisDatabase instance;
    return instance;
}

    //command operations
    bool RedisDatabase::flushAll(){
        std::lock_guard<std::mutex> lock(db_mutex); 
        kv_store.clear();
        list_store.clear();
        hash_store.clear();
        expiry_map.clear();
        return true;
    }

    //key-value operations
    bool RedisDatabase::set(const std::string& key, const std::string& value){
        std::lock_guard<std::mutex> lock(db_mutex); 
        kv_store[key] = value;
        return true;
    }

    bool RedisDatabase::get(const std::string& key, std::string& value){//not const std::string& value
        std::lock_guard<std::mutex> lock(db_mutex); 
        auto it = kv_store.find(key);
        if(it != kv_store.end()){
            value = it->second;
            return true;
        }
        return false;
    }

    std::vector<std::string> RedisDatabase::keys(){
        std::lock_guard<std::mutex> lock(db_mutex); 
        std::vector<std::string> keys;
        for(const auto& kv:kv_store){
            keys.push_back(kv.first);
        }
        for(const auto& list:list_store){
            keys.push_back(list.first);
        }
        for(const auto& hash:hash_store){
            keys.push_back(hash.first);
        }
        return keys;
    }

    std::string RedisDatabase::type(const std::string& key){
        std::lock_guard<std::mutex> lock(db_mutex); 
        if(kv_store.find(key) != kv_store.end()){
            return "string";
        }
        if(list_store.find(key) != list_store.end()){
            return "list";
        }
        if(hash_store.find(key) != hash_store.end()){
            return "hash";
        }
        return "none";
    }

    bool RedisDatabase::del(const std::string& key){
        std::lock_guard<std::mutex> lock(db_mutex); 
        bool deleted = false;
        deleted |= (kv_store.erase(key) > 0);//return number of elements removed
        deleted |= (list_store.erase(key) > 0);
        deleted |= (hash_store.erase(key) > 0);
        expiry_map.erase(key);
        return deleted;
    }

    bool RedisDatabase::expire(const std::string& key, int seconds){
        std::lock_guard<std::mutex> lock(db_mutex); 
        bool exists = (kv_store.find(key) != kv_store.end()) ||
                      (list_store.find(key) != list_store.end()) ||
                      (hash_store.find(key) != hash_store.end());
        if(exists){
            expiry_map[key] = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
            return true;
        }
        return false;
    }

    bool RedisDatabase::rename(const std::string& oldKey, const std::string& newKey){
        std::lock_guard<std::mutex> lock(db_mutex); 
        if(kv_store.find(oldKey) != kv_store.end()){
            kv_store[newKey] = kv_store[oldKey];
            kv_store.erase(oldKey);
            return true;
        }
        if(list_store.find(oldKey) != list_store.end()){
            list_store[newKey] = list_store[oldKey];
            list_store.erase(oldKey);
            return true;
        }
        if(hash_store.find(oldKey) != hash_store.end()){
            hash_store[newKey] = hash_store[oldKey];
            hash_store.erase(oldKey);
            return true;
        }
        return false;
    }

    //list operations
    std::vector<std::string> RedisDatabase::lget(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end()) {
        return it->second; 
    }
    return {}; 
}

ssize_t RedisDatabase::llen(const std::string& key) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end()) 
        return it->second.size();
    return 0;
}

void RedisDatabase::lpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    list_store[key].insert(list_store[key].begin(), value);
}

void RedisDatabase::rpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    list_store[key].push_back(value);
}

bool RedisDatabase::lpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.front();
        it->second.erase(it->second.begin());
        return true;
    }
    return false;
}

bool RedisDatabase::rpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.back();
        it->second.pop_back();
        return true;
    }
    return false;
}

int RedisDatabase::lrem(const std::string& key, int count, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    int removed = 0;
    auto it = list_store.find(key);
    if (it == list_store.end()) 
        return 0;

    auto& lst = it->second;

    if (count == 0) {
        // Remove all occurances
        auto new_end = std::remove(lst.begin(), lst.end(), value);
        removed = std::distance(new_end, lst.end());
        lst.erase(new_end, lst.end());
    } else if (count > 0) {
        // Remove from head to tail
        for (auto iter = lst.begin(); iter != lst.end() && removed < count; ) {
            if (*iter == value) {
                iter = lst.erase(iter);
                ++removed;
            } else {
                ++iter;
            }
        }
    } else {
        // Remove from tail to head (count is negative)
        for (auto riter = lst.rbegin(); riter != lst.rend() && removed < (-count); ) {
            if (*riter == value) {
                auto fwdIter = riter.base();
                --fwdIter;
                fwdIter = lst.erase(fwdIter);
                ++removed;
                riter = std::reverse_iterator<std::vector<std::string>::iterator>(fwdIter);
            } else {
                ++riter;
            }
        }
    }
    return removed;
}

bool RedisDatabase::lindex(const std::string& key, int index, std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it == list_store.end()) 
        return false;

    const auto& lst = it->second;
    if (index < 0)
        index = lst.size() + index;
    if (index < 0 || index >= static_cast<int>(lst.size()))
        return false;
    
    value = lst[index];
    return true;
}

bool RedisDatabase::lset(const std::string& key, int index, const std::string& value) {
    std::lock_guard<std::mutex> lock(db_mutex);
    auto it = list_store.find(key);
    if (it == list_store.end()) 
        return false;

    auto& lst = it->second;
    if (index < 0)
        index = lst.size() + index;
    if (index < 0 || index >= static_cast<int>(lst.size()))
        return false;
    
    lst[index] = value;
    return true;
}

//SIMPLE DUMP AND LOAD IMPLEMENTATION USING A BINARY FILE 

bool RedisDatabase::dump(const std::string& filename){
    std::lock_guard<std::mutex> lock(db_mutex);// lock the database during dump
    // For simplicity, we just create an empty file to simulate dumping
    std::ofstream ofs(filename, std::ios::binary);
    if(!ofs){
        return false;
    }
    // In a real implementation, serialize the database contents 
    for(const auto& kv:kv_store){
        ofs << "KV " << kv.first << " " << kv.second << "\n";
    }
    for(const auto& list:list_store){
        ofs << "LIST " << list.first << "\n";
        for(const auto& item:list.second){
            ofs << "  ITEM " << item << "\n";
        }
    }
    for(const auto& hash:hash_store){
        ofs << "HASH " << hash.first << "\n";
        for(const auto& field:hash.second){
            ofs << "  FIELD " << field.first << " " << field.second << "\n";
        }
    }
    return true;
}


bool RedisDatabase::load(const std::string& filename){
    std::lock_guard<std::mutex> lock(db_mutex);// lock the database during load
    // For simplicity, we just check if the file exists to simulate loading
    std::ifstream ifs(filename, std::ios::binary);
    if(!ifs){
        return false;
    }

    // In a real implementation, deserialize the database contents
    kv_store.clear();
    list_store.clear();
    hash_store.clear();

    std::string line;
    while(std::getline(ifs, line)){
        std::istringstream iss(line);
        std::string type;
        iss >> type;
        if(type == "KV"){
            std::string key, value;
            iss >> key >> value;
            kv_store[key] = value;
        } else if(type == "LIST"){
            std::string list_name;
            iss >> list_name;
            std::string item;
            std::vector<std::string> list;
            while(iss >> item){
                list.push_back(item);
            }
            list_store[list_name] = list;
        } else if(type == "HASH"){
            std::string hash_name;
            iss >> hash_name;
            std::unordered_map<std::string, std::string> fields;
            std::string pairs;
            while(iss >> pairs){
                auto pos = pairs.find(' ');
                if(pos != std::string::npos){
                    std::string field = pairs.substr(0, pos);
                    std::string value = pairs.substr(pos + 1);
                    fields[field] = value;
                }
            }
            hash_store[hash_name] = fields;
        }
    }
    return true;
}