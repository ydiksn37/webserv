#!/bin/bash

# Dockerイメージのビルドとコンテナの起動
docker-compose up -d --build

echo "---------------------------------------------------"
echo "Webserv Docker environment is ready!"
echo "To enter the container, run:"
echo "  docker exec -it $(docker-compose ps -q webserv) bash"
echo "---------------------------------------------------"
