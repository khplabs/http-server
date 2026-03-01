FROM gcc:13 AS builder

WORKDIR /app

RUN apt-get update && apt-get install -y cmake

COPY . .

RUN mkdir build && cd build && cmake .. && make

FROM debian:bookworm-slim

WORKDIR /app

COPY --from=builder /app/build/http_server .

COPY --from=builder /app/static ./static

EXPOSE 8080

CMD ["./http_server"]