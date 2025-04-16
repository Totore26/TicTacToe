FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    netbase \
    iproute2 \
    net-tools \
    iputils-ping \
    netcat-openbsd \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app/Client

# Copy only necessary files
COPY Client/ ./
COPY Comunicazione.h ../

RUN gcc -o client client.c funzioni.c

# Add debugging script with improved connection handling
COPY <<EOF /entrypoint.sh
#!/bin/bash
set -e

MAX_RETRIES=30
RETRY_INTERVAL=2

echo "Network configuration:"
ip addr

echo "Waiting for server..."
count=0
while ! nc -z -v -w5 server 8080; do
    ((count++))
    if [ $count -ge $MAX_RETRIES ]; then
        echo "ERROR: Server did not become ready within $(($MAX_RETRIES * $RETRY_INTERVAL)) seconds."
        exit 1
    fi
    echo "Attempt $count/$MAX_RETRIES: Server not ready, retrying in $RETRY_INTERVAL seconds..."
    sleep $RETRY_INTERVAL
done

echo "Server is ready!"
echo "Starting client with pseudo-terminal..."
exec script -qfec "./client" /dev/null
EOF

RUN chmod +x /entrypoint.sh

CMD ["/entrypoint.sh"]