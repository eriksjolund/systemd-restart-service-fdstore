FROM docker.io/library/ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y build-essential libsystemd-dev less findutils git libasio-dev autoconf automake

COPY server_fdstore.c /server_fdstore.c

RUN gcc server_fdstore.c -lsystemd -o /server

FROM docker.io/library/ubuntu:22.04
RUN apt-get update && apt-get install -y curl
COPY --from=builder /server /server
CMD ["/server"]
