# C++ Chat Project

Lightweight chat server and client implemented in C++ using Muduo-like patterns.

## Summary
- Small educational chat system including server, client, and simple DB models.

## Repo layout (important paths)
- [CMakeLists.txt](CMakeLists.txt)
- [src/server](src/server) — server sources
- [src/client](src/client) — client sources
- [include](include) — public headers and service interfaces

## Prerequisites
- Linux
- CMake (>=3.10 recommended)
- A C++ compiler (GCC/Clang) with C++11/14 support
- MySQL (for loading the sample DB) if you plan to test DB-backed features

## Build (quick)
Run from the project root:

```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

Binaries will be placed in `bin/` (or the build's equivalent output).

## Run
- Start the server:

```bash
./bin/ChatServer
```

- Start a client (connects to localhost:8888 by default example):

```bash
./bin/ChatClient 127.0.0.1 8888
```

## Database (optional)
To load the provided sample data into a MySQL `chat` database:

```bash
mysql -u <user> -p chat < test/test_muduo/chat.sql
```

## Notes
- This is an codebase for my c++ high performance distribute server learning. Inspect `src/server` and `src/client` for core logic.
- Use `include/` for public interfaces and models.


