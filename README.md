# Redis C++ Implementation

A lightweight, multi-threaded Redis server implementation in C++ with support for string, list, and hash data structures.

---

## Overview

This project implements a Redis-compatible server in C++ that provides an in-memory data structure store with support for:
- **Key-Value pairs**: Simple string storage and retrieval
- **Lists**: Ordered collections with push/pop operations
- **Hashes**: Field-value mappings within keys

The server uses a multi-threaded architecture to handle concurrent client connections and includes data persistence capabilities.

---


### Key Design Principles

1. **Singleton Pattern**: RedisDatabase uses singleton pattern to ensure a single instance across the entire application
2. **Thread Safety**: All operations protected by mutex locks
3. **Multi-threaded**: Each client connection handled in a separate thread
4. **Graceful Shutdown**: Signal handlers (SIGINT) for clean server shutdown
5. **Persistence**: Background thread periodically dumps database to disk
6. **RESP Protocol**: Redis Serialization Protocol for client-server communication

---

## Features

### Supported Commands

#### Common Commands
- **PING**: Server health check
- **ECHO**: Echo back the provided message
- **FLUSHALL**: Clear entire database

#### Key-Value Operations
- **SET key value**: Store a string value
- **GET key**: Retrieve a string value
- **KEYS**: List all keys in database
- **TYPE key**: Get data type of a key
- **DEL key**: Delete a key
- **EXPIRE key seconds**: Set expiration time
- **RENAME oldkey newkey**: Rename a key

#### List Operations
- **LPUSH key value**: Insert at head of list
- **RPUSH key value**: Insert at tail of list
- **LPOP key**: Remove and return head element
- **RPOP key**: Remove and return tail element
- **LLEN key**: Get list length
- **LINDEX key index**: Get element at index
- **LGET key**: Get entire list
- **LSET key index value**: Set element at index
- **LREM key count value**: Remove elements matching value

#### Hash Operations
- **HSET key field value**: Set hash field
- **HGET key field**: Get hash field value
- **HGETALL key**: Get all fields and values
- **HEXISTS key field**: Check field existence
- **HDEL key field**: Delete hash field
- **HKEYS key**: Get all field names
- **HVALS key**: Get all field values
- **HLEN key**: Get number of fields
- **HMSET key field1 value1 field2 value2 ...**: Set multiple fields

### Data Types Supported
- **Strings**: UTF-8 encoded text values
- **Lists**: Ordered collections with indexed access
- **Hashes**: Key-value mappings (nested objects)

### Performance Features
- Multi-threaded client handling
- In-memory operations (O(1) for most operations)
- Efficient data structure implementations
- Background persistence (doesn't block requests)

---

## Project Structure

```
redis-cpp/
├── src/
│   ├── main.cpp                    # Server entry point, persistence thread
│   ├── RedisServer.cpp             # Socket management & client handling
│   ├── RedisDatabase.cpp           # Database implementation & persistence
│   ├── RedisCommandHandler.cpp     # Command routing & processing
│   └── CommandHandlers.cpp         # Individual command implementations
├── include/
│   ├── RedisServer.h               # Server interface
│   ├── RedisDatabase.h             # Database interface
│   └── RedisCommandHandler.h       # Command handler interface
├── Redis-Client/                   # Client application
│   └── Client/
│       ├── main.cpp                # CLI entry point
│       ├── CLI.cpp/h               # Command-line interface
│       ├── RedisClient.cpp/h       # Network client
│       ├── CommandHandler.cpp/h    # Client-side command handling
│       └── ResponseParser.cpp/h    # Parse server responses
├── build/                          # Compiled object files
├── Makefile                        # Build configuration
├── dump.my_rdb                     # Persistent data storage
└── README.md                       # This file
```

---

## Components

### 1. RedisServer
**File**: `src/RedisServer.cpp`, `include/RedisServer.h`

Responsibilities:
- Create and manage TCP socket on specified port (default: 6379)
- Accept incoming client connections
- Spawn a new thread for each connected client
- Receive data from clients and route to command handler
- Send responses back to clients
- Graceful shutdown with signal handling (SIGINT/SIGTERM)

Key Methods:
- `RedisServer(int port)`: Constructor
- `void run()`: Main server loop
- `void shutdown()`: Graceful shutdown
- `void setupSignalHandlers()`: Register signal handlers

### 2. RedisDatabase
**File**: `src/RedisDatabase.cpp`, `include/RedisDatabase.h`

Responsibilities:
- Maintain in-memory data structures
- Implement all data operations (KV, List, Hash)
- Handle key expiration
- Thread-safe operations with mutex locking
- Persistence (dump/load)

Key Methods:
- `set(key, value)`: Store string value
- `get(key, value)`: Retrieve string value
- `lpush/rpush(key, value)`: List operations
- `hset/hget(key, field, value)`: Hash operations
- `dump(filename)`: Save database to file
- `load(filename)`: Load database from file

Key Data Structures:
```cpp
std::unordered_map<std::string, std::string> kv_store;
std::unordered_map<std::string, std::vector<std::string>> list_store;
std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store;
std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiry_map;
std::mutex db_mutex;  // Thread safety
```

### 3. RedisCommandHandler
**File**: `src/RedisCommandHandler.cpp`, `include/RedisCommandHandler.h`

Responsibilities:
- Parse incoming commands from clients
- Validate command syntax
- Route commands to appropriate handlers
- Format responses in RESP protocol
- Error handling and reporting

Key Methods:
- `processCommand(command)`: Main entry point
- `handleSet/Get/Del()`: KV operations
- `handleLpush/Rpush/Lpop()`: List operations
- `handleHset/Hget/Hgetall()`: Hash operations
- `handlePing/Echo/FlushAll()`: Server commands

RESP Protocol Format:
```
Simple String: +OK\r\n
Error: -ERR message\r\n
Integer: :1000\r\n
Bulk String: $6\r\nfoobar\r\n
Array: *2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n
Null: $-1\r\n
```

### 4. Client (CLI)
**Location**: `Redis-Client/Client/`

Provides interactive command-line interface to:
- Connect to Redis server
- Send commands
- Display formatted responses
- Support for all server commands

---

## Control Flow

### Server Startup Flow

```
main()
  │
  ├─→ Load database from dump.my_rdb
  │    (if exists, else start empty)
  │
  ├─→ Create RedisServer instance (port 6379)
  │
  ├─→ Spawn background persistence thread
  │    (dumps every 300 seconds)
  │
  └─→ server.run()
       │
       ├─→ Create TCP socket (AF_INET, SOCK_STREAM)
       │
       ├─→ Bind to port
       │
       ├─→ Listen for incoming connections
       │
       └─→ Main server loop:
            while(running) {
              accept() → client_connection
              spawn_thread() → handle_client(client_socket)
            }
```

### Request-Response Flow (Per Client)

```
Client connects via TCP
  │
  ├─→ Server spawns new thread
  │
  └─→ Thread enters while loop:
       │
       ├─→ recv(client_socket) → receive command
       │    (e.g., "SET mykey myvalue\r\n")
       │
       ├─→ RedisCommandHandler::processCommand()
       │    │
       │    ├─→ Parse command tokens
       │    │
       │    ├─→ Validate syntax
       │    │
       │    ├─→ Route to handler (handleSet, handleGet, etc.)
       │    │
       │    ├─→ Call RedisDatabase methods (with mutex lock)
       │    │    (operates on kv_store, list_store, hash_store)
       │    │
       │    └─→ Format response in RESP protocol
       │
       ├─→ send() → response back to client
       │    (e.g., "+OK\r\n")
       │
       └─→ Loop until client disconnects
            (recv returns <= 0)
```

### Data Persistence Flow

```
Background Thread (spawned in main)
  │
  └─→ Every 300 seconds:
       │
       ├─→ Sleep for 300 seconds
       │
       ├─→ RedisDatabase::dump("dump.my_rdb")
       │    │
       │    ├─→ Acquire mutex lock
       │    │
       │    ├─→ Iterate through kv_store
       │    │    └─→ Write: "KV key value\n"
       │    │
       │    ├─→ Iterate through list_store
       │    │    └─→ Write: "LIST key\n  ITEM value\n"
       │    │
       │    ├─→ Iterate through hash_store
       │    │    └─→ Write: "HASH key\n  FIELD field value\n"
       │    │
       │    └─→ Release mutex lock
       │
       └─→ Repeat (infinite loop)
```


### Build the Server

```bash
# Navigate to project directory
cd redis-cpp

# Build the server
make

# Or with rebuild (clean + build)
make rebuild
```

### Run the Server

```bash
# Default port 6379
./my_redis_server

# Custom port
./my_redis_server 6380
```

### Graceful Shutdown

```bash
# Press Ctrl+C to trigger SIGINT
# Server will:
#   1. Stop accepting new connections
#   2. Close client connections gracefully
#   3. Perform final database dump
#   4. Exit cleanly
```

### Testing Checklist

- [x] Key-Value operations (SET, GET, DEL, RENAME, EXPIRE, TYPE, KEYS)
- [x] List operations (LPUSH, RPUSH, LPOP, RPOP, LLEN, LINDEX, LGET, LSET, LREM)
- [x] Hash operations (HSET, HGET, HGETALL, HEXISTS, HDEL, HKEYS, HVALS, HLEN, HMSET)
- [x] Server commands (PING, ECHO, FLUSHALL)
- [x] Multi-client concurrent access
- [x] Data persistence (dump/load)
- [x] Graceful shutdown
- [x] Error handling

---

## Data Persistence

### Persistence Strategy

The server automatically persists data to `dump.my_rdb`:

**Frequency**: Every 5 minutes (300 seconds)
- Background thread handles persistence
- Non-blocking (doesn't interrupt client requests)

**Format**: Text-based line format
```
KV key value
LIST listname
  ITEM item1
  ITEM item2
HASH hashname
  FIELD field1 value1
  FIELD field2 value2
```

**Load on Startup**:
- Server automatically loads `dump.my_rdb` when starting
- If file doesn't exist, starts with empty database
- Data immediately available to clients


### Recovery
If server crashes:
1. Restart the server
2. Server reads dump.my_rdb
3. Reconstructs all KV pairs, lists, and hashes
4. Resumes serving clients

Data loss limited to modifications since last dump (max 5 minutes)


### Current Limitations
- Single-threaded persistence (no concurrent access during dump)
- Text-based persistence (not binary, larger file size)
- No support for transactions
- No pub/sub functionality
- Limited to single server (no clustering)
- Expiry check only on access



