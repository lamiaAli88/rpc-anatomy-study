#!/bin/bash

echo "=========================================="
echo "Starting Full RPC Benchmark Suite"
echo "=========================================="

# 1. إنشاء المجلدات إذا لم تكن موجودة
mkdir -p src scripts data plots include

# 2. تجميع جميع الأكواد بنظام التحسين -O3
echo "[+] Compiling C++ programs with -O3 optimization..."
g++ -O3 -Iinclude src/in_process.cpp src/schema.pb.cc -o src/in_process -lprotobuf
g++ -O3 src/unix_server.cpp -o src/unix_server
g++ -O3 -Iinclude src/unix_client.cpp src/schema.pb.cc -o src/unix_client -lprotobuf
g++ -O3 src/tcp_server.cpp -o src/tcp_server
g++ -O3 -Iinclude src/tcp_client.cpp src/schema.pb.cc -o src/tcp_client -lprotobuf

echo "------------------------------------------"

# 3. تشغيل تجارب In-Process
echo "[+] 1/3 Running In-Process Experiments..."
./src/in_process

echo "------------------------------------------"

# 4. تشغيل تجارب Unix Domain Socket
echo "[+] 2/3 Running Unix Domain Socket Experiments..."
./src/unix_server &
SERVER_PID=$!
sleep 1
./src/unix_client
kill $SERVER_PID 2>/dev/null

echo "------------------------------------------"

# 5. تشغيل تجارب TCP Loopback
echo "[+] 3/3 Running TCP Loopback Experiments..."
./src/tcp_server &
SERVER_PID=$!
sleep 1
./src/tcp_client
kill $SERVER_PID 2>/dev/null

echo "=========================================="
echo "All benchmarks completed successfully!"
echo "Raw data stored in the 'data/' directory."
echo "=========================================="
