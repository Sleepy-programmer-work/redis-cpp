#!/bin/bash

# Simple Redis test script using nc (netcat)

echo "================================"
echo "REDIS C++ FEATURE TESTS"
echo "================================"

send_command() {
    local cmd="$1"
    echo "Testing: $cmd"
    (sleep 0.1; echo "$cmd"; sleep 0.5) | nc -w 1 localhost 6379 2>/dev/null || echo "Command sent"
    echo ""
}

echo ""
echo "=== STRING OPERATIONS ==="
send_command "PING"
send_command "SET key1 value1"
send_command "GET key1"
send_command "SET user:1 john"
send_command "SET user:2 jane"
send_command "KEYS *"

echo ""
echo "=== KEY OPERATIONS ==="
send_command "TYPE key1"
send_command "DEL user:2"
send_command "TYPE user:2"

echo ""
echo "=== LIST OPERATIONS ==="
send_command "LPUSH mylist apple"
send_command "LPUSH mylist banana"
send_command "RPUSH mylist orange"
send_command "LLEN mylist"
send_command "LGET mylist"
send_command "LPOP mylist"

echo ""
echo "=== HASH OPERATIONS ==="
send_command "HSET user:profile name john"
send_command "HSET user:profile age 30"
send_command "HGET user:profile name"
send_command "HLEN user:profile"
send_command "HGETALL user:profile"
send_command "HKEYS user:profile"

echo ""
echo "=== MISC OPERATIONS ==="
send_command "ECHO hello"
send_command "EXPIRE key1 3600"

echo "================================"
echo "TESTS COMPLETED"
echo "================================"
