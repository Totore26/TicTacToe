FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    gcc \
    make \
    netbase \
    netcat-openbsd \
    iproute2 \
    net-tools \
    iputils-ping \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app/Server

# Copy only necessary files
COPY Server/ ./
COPY Comunicazione.h ../
COPY Server/Makefile .

RUN make clean && make

EXPOSE 8080

# Add debugging script with improved startup handling
COPY <<EOF /entrypoint.sh
#!/bin/bash
set -e

echo "Network configuration:"
ip addr

# Wait for network to be fully up
echo "Waiting for network..."
for i in {1..30}; do
    if ip addr show eth0 | grep -q "inet "; then
        break
    fi
    echo "Waiting for network interface to be ready... ($i/30)"
    sleep 1
done

# Show network status and bind address
ip addr show eth0
echo "Binding server to all interfaces (0.0.0.0)..."
netstat -ltn

echo "Starting server..."
exec ./server
EOF

RUN chmod +x /entrypoint.sh

# More specific healthcheck that checks actual binding
HEALTHCHECK --interval=2s --timeout=5s --start-period=10s --retries=5 \
    CMD netstat -ln | grep -q ':8080.*LISTEN.*' || exit 1

CMD ["/entrypoint.sh"]