#include "../include/RedisDatabase.h"
#include <fstream>
#include <mutex>
#include <iostream>
#include <exception>
#include <sstream>


RedisDatabase& RedisDatabase::getInstance() {
    static RedisDatabase instance;
    return instance;
}

//SIMPLE DUMP AND LOAD IMPLEMENTATION
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
            std::vector<std::string> items;
            while(std::getline(ifs, line) && line.find("  ITEM ") == 0){
                std::istringstream item_iss(line);
                std::string item_label, item_value;
                item_iss >> item_label >> item_value;
                items.push_back(item_value);
            }
            list_store[list_name] = items;
            if(!line.empty()){
                // put back the last read line for next iteration
                ifs.seekg(-static_cast<int>(line.length()) - 1, std::ios::cur);
            }
        } else if(type == "HASH"){
            std::string hash_name;
            iss >> hash_name;
            std::unordered_map<std::string, std::string> fields;
            while(std::getline(ifs, line) && line.find("  FIELD ") == 0){
                std::istringstream field_iss(line);
                std::string field_label, field_key, field_value;
                field_iss >> field_label >> field_key >> field_value;
                fields[field_key] = field_value;
            }
            hash_store[hash_name] = fields;
            if(!line.empty()){
                // put back the last read line for next iteration
                ifs.seekg(-static_cast<int>(line.length()) - 1, std::ios::cur);
            }
        }


    return true;
}