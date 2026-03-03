# HTTP/1.1 Server in C++

HTTP/1.1 server written in C++. No frameworks, no HTTP libraries. Just POSIX sockets.

The goal is to iterate on the architecture, benchmark each change, and document what the numbers actually show. Starting point is thread-per-connection. From here I want to explore thread pools, non-blocking I/O, and eventually an epoll-based event loop to see how each change affects throughput and latency under load.

## Architecture

```
Service (ClusterIP, port 80)
  |
  +---> Pod 1: http_server (port 8080)
  +---> Pod 2: http_server (port 8080)
```

The main thread blocks on `accept()` and spawns a `std::thread` for each incoming connection. Each thread handles the full request/response lifecycle for that connection, including the keep-alive loop, until the client disconnects or sends `Connection: close`.

```
accept() loop (main thread)
  |
  +---> std::thread -> parse request -> serve file -> keep-alive loop -> close
  +---> std::thread -> parse request -> serve file -> keep-alive loop -> close
  +---> std::thread -> parse request -> serve file -> keep-alive loop -> close
```

## Project structure

```
http-server/
+-- include/
|   +-- server.h          # TCP server, accept loop, thread spawning
|   +-- http_request.h    # HttpRequest struct with headers map
|   +-- http_parser.h     # Parses raw bytes into HttpRequest
|   +-- http_response.h   # Builds HTTP/1.1 response strings
|   +-- file_handler.h    # Reads static files, resolves MIME types
+-- src/
|   +-- main.cpp
|   +-- server.cpp
|   +-- http_parser.cpp
|   +-- http_response.cpp
|   +-- file_handler.cpp
+-- static/               # Files served by the server
+-- k8s/
|   +-- deployment.yaml   # 2 replicas with liveness/readiness probes
|   +-- service.yaml      # ClusterIP service on port 80
|   +-- ingress.yaml      # nginx ingress routing
+-- Dockerfile            # Multi-stage build, statically linked libstdc++
+-- CMakeLists.txt
```

## Build and run locally

**Requirements:** CMake 3.20+, GCC or Clang with C++17 support

```bash
git clone https://github.com/khplagemann/http-server.git
cd http-server
mkdir build && cd build
cmake ..
make
cd ..
./build/http_server
```

The server listens on port 8080. Open `http://localhost:8080` in a browser or test with curl:

```bash
curl http://localhost:8080/
curl http://localhost:8080/about.html
curl http://localhost:8080/missing.html  # returns 404
```

## Run with Docker

```bash
docker pull khplagemann/http-server:latest
docker run -p 8080:8080 khplagemann/http-server:latest
```

## Deploy to Kubernetes

```bash
kubectl apply -f k8s/
kubectl get pods
kubectl get ingress
```

## Benchmarks

Tested with [wrk](https://github.com/wg/wrk) on a MacBook Air M3 24GB RAM 512GB SSD against a local Docker container. Each result is the average of three 60-second runs at 8 threads.

```
wrk -t8 -c{connections} -d60s http://localhost:8080/
```

| Connections | Avg req/s | Avg latency | Errors/min | Notes |
|-------------|-----------|-------------|------------|-------|
| 100 | 45,975 | 3.1ms | 0 | Clean |
| 250 | 45,642 | 6.6ms | 109 | Stable |
| 500 | 44,481 | 12.8ms | 624 | Stable |
| 1,000 | 44,404 | 24.4ms | 3,306 | Stable, errors climbing |
| 2,500 | ~27,000 | 118ms | ~147,000 | Unstable, one of three runs collapsed |
| 5,000 | ~211 | 595ms | ~247,000 | Collapsed |
| 10,000 | ~8 | 755ms | ~210,000 | Failure |

Throughput stays flat from 100 to 1,000 connections at around 44-46k req/s. The error rate goes up but the server stays stable.

At 2,500 connections the behaviour becomes unpredictable. Two of three runs completed normally, one collapsed. This is where thread exhaustion starts to kick in.

By 5,000 the server has stopped working. The OS cannot spawn threads fast enough so most connections are dropped. At 10,000 only 8 requests per second got through.

This is the C10K problem. Thread-per-connection hits a hard ceiling because each thread takes memory and the OS has a thread limit. The fix is non-blocking I/O with an event loop, where one thread monitors thousands of file descriptors using `epoll` (Linux) or `kqueue` (macOS) and only handles a connection when it has data ready. That is what nginx does.

## Design decisions

**Thread-per-connection vs event loop**
Thread-per-connection means each connection has its own stack and can block on `read()` without affecting others. The downside is it does not scale beyond a few thousand connections.

## What I would change at production scale

- Replace thread-per-connection with an `epoll`-based event loop
- Add a thread pool to cap the number of live threads
- Implement chunked transfer encoding for large file streaming
- Add request timeouts so slow clients do not hold threads open
- TLS via OpenSSL
