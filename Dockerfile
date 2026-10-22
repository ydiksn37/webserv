FROM debian:12-slim

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    g++ \
    make \
    python3 \
    php-cgi \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
