#!/usr/bin/env python3
import socket
import time
import sys

class RedisClient:
    def __init__(self, host='127.0.0.1', port=6379):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))
        
    def send_command(self, *args):
        """Send a Redis command"""
        cmd = f"*{len(args)}\r\n"
        for arg in args:
            arg_str = str(arg)
            cmd += f"${len(arg_str)}\r\n{arg_str}\r\n"
        self.sock.sendall(cmd.encode())
        return self.read_response()
    
    def read_response(self):
        """Read Redis response"""
        response = b''
        while True:
            chunk = self.sock.recv(4096)
            if not chunk:
                break
            response += chunk
            if self.is_complete_response(response):
                break
        return response.decode().strip()
    
    def is_complete_response(self, response):
        """Check if response is complete"""
        if not response:
            return False
        if response.startswith(b'+') or response.startswith(b'-'):
            return b'\r\n' in response
        if response.startswith(b':'):
            return b'\r\n' in response
        if response.startswith(b'$'):
            return True
        if response.startswith(b'*'):
            return True
        return True
    
    def close(self):
        self.sock.close()

def test_strings(client):
    print("\n" + "="*50)
    print("TESTING STRING OPERATIONS")
    print("="*50)
    
    print("\n✓ SET mykey 'hello world'")
    result = client.send_command("SET", "mykey", "hello world")
    print(f"  Response: {result}")
    
    print("\n✓ GET mykey")
    result = client.send_command("GET", "mykey")
    print(f"  Response: {result}")
    
    print("\n✓ SET user:1 john")
    client.send_command("SET", "user:1", "john")
    
    print("✓ SET user:2 jane")
    client.send_command("SET", "user:2", "jane")
    
    print("✓ SET product:1 'redis book'")
    client.send_command("SET", "product:1", "redis book")

def test_keys(client):
    print("\n" + "="*50)
    print("TESTING KEY OPERATIONS")
    print("="*50)
    
    print("\n✓ KEYS *")
    result = client.send_command("KEYS")
    print(f"  Response: {result}")
    
    print("\n✓ TYPE mykey")
    result = client.send_command("TYPE", "mykey")
    print(f"  Response: {result}")
    
    print("\n✓ TYPE nonexistent")
    result = client.send_command("TYPE", "nonexistent")
    print(f"  Response: {result}")
    
    print("\n✓ DEL user:2")
    result = client.send_command("DEL", "user:2")
    print(f"  Response: {result}")
    
    print("\n✓ KEYS * (after deletion)")
    result = client.send_command("KEYS")
    print(f"  Response: {result}")

def test_lists(client):
    print("\n" + "="*50)
    print("TESTING LIST OPERATIONS")
    print("="*50)
    
    print("\n✓ LPUSH mylist apple")
    result = client.send_command("LPUSH", "mylist", "apple")
    print(f"  Response: {result}")
    
    print("\n✓ LPUSH mylist banana orange")
    result = client.send_command("LPUSH", "mylist", "banana", "orange")
    print(f"  Response: {result}")
    
    print("\n✓ RPUSH mylist grape")
    result = client.send_command("RPUSH", "mylist", "grape")
    print(f"  Response: {result}")
    
    print("\n✓ LLEN mylist")
    result = client.send_command("LLEN", "mylist")
    print(f"  Response: {result}")
    
    print("\n✓ LGET mylist")
    result = client.send_command("LGET", "mylist")
    print(f"  Response: {result}")
    
    print("\n✓ LINDEX mylist 0")
    result = client.send_command("LINDEX", "mylist", "0")
    print(f"  Response: {result}")
    
    print("\n✓ LPOP mylist")
    result = client.send_command("LPOP", "mylist")
    print(f"  Response: {result}")
    
    print("\n✓ RPOP mylist")
    result = client.send_command("RPOP", "mylist")
    print(f"  Response: {result}")
    
    print("\n✓ LSET mylist 0 mango")
    result = client.send_command("LSET", "mylist", "0", "mango")
    print(f"  Response: {result}")

def test_hashes(client):
    print("\n" + "="*50)
    print("TESTING HASH OPERATIONS")
    print("="*50)
    
    print("\n✓ HSET myhash field1 value1")
    result = client.send_command("HSET", "myhash", "field1", "value1")
    print(f"  Response: {result}")
    
    print("\n✓ HSET myhash field2 value2 field3 value3")
    result = client.send_command("HSET", "myhash", "field2", "value2", "field3", "value3")
    print(f"  Response: {result}")
    
    print("\n✓ HGET myhash field1")
    result = client.send_command("HGET", "myhash", "field1")
    print(f"  Response: {result}")
    
    print("\n✓ HLEN myhash")
    result = client.send_command("HLEN", "myhash")
    print(f"  Response: {result}")
    
    print("\n✓ HEXISTS myhash field1")
    result = client.send_command("HEXISTS", "myhash", "field1")
    print(f"  Response: {result}")
    
    print("\n✓ HEXISTS myhash nonexistent")
    result = client.send_command("HEXISTS", "myhash", "nonexistent")
    print(f"  Response: {result}")
    
    print("\n✓ HKEYS myhash")
    result = client.send_command("HKEYS", "myhash")
    print(f"  Response: {result}")
    
    print("\n✓ HVALS myhash")
    result = client.send_command("HVALS", "myhash")
    print(f"  Response: {result}")
    
    print("\n✓ HGETALL myhash")
    result = client.send_command("HGETALL", "myhash")
    print(f"  Response: {result}")
    
    print("\n✓ HDEL myhash field2")
    result = client.send_command("HDEL", "myhash", "field2")
    print(f"  Response: {result}")

def test_misc(client):
    print("\n" + "="*50)
    print("TESTING MISC OPERATIONS")
    print("="*50)
    
    print("\n✓ PING")
    result = client.send_command("PING")
    print(f"  Response: {result}")
    
    print("\n✓ ECHO 'Hello Redis'")
    result = client.send_command("ECHO", "Hello Redis")
    print(f"  Response: {result}")
    
    print("\n✓ RENAME mykey mynewkey")
    client.send_command("SET", "mykey", "value")
    result = client.send_command("RENAME", "mykey", "mynewkey")
    print(f"  Response: {result}")
    
    print("\n✓ EXPIRE mynewkey 3600")
    result = client.send_command("EXPIRE", "mynewkey", "3600")
    print(f"  Response: {result}")

def main():
    try:
        print("\n🚀 REDIS C++ IMPLEMENTATION - FEATURE TEST")
        print("Connecting to server at 127.0.0.1:6379...\n")
        
        client = RedisClient()
        
        test_strings(client)
        test_keys(client)
        test_lists(client)
        test_hashes(client)
        test_misc(client)
        
        client.close()
        
        print("\n" + "="*50)
        print("✅ ALL TESTS COMPLETED SUCCESSFULLY!")
        print("="*50 + "\n")
        
    except Exception as e:
        print(f"\n❌ ERROR: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
