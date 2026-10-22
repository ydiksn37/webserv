FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    make \
    python3 \
    php-cgi \
    curl \
    netcat-openbsd \
    siege \
    git \
    valgrind \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /app

CMD ["bash"]
