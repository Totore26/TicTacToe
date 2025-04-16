FROM alpine:latest

WORKDIR /app

RUN apk update && \
    apk add --no-cache gcc make libc-dev linux-headers netcat-openbsd

COPY . /app/

RUN make clean && make run_client

EXPOSE 8080

CMD ["./client", "server", "8080", "1"]
