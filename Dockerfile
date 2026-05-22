FROM ubuntu:22.04

# 非対面でのインストール設定
ENV DEBIAN_FRONTEND=noninteractive

# 必要なパッケージのインストール
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

# 作業ディレクトリの設定
WORKDIR /app

# デフォルトのコマンド
CMD ["bash"]
