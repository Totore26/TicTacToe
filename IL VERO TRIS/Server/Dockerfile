FROM alpine:latest

WORKDIR /app

RUN apk update && \
    apk add --no-cache gcc make libc-dev linux-headers netcat-openbsd

COPY . /app/

RUN make clean && make run_server

EXPOSE 8080

CMD ["./server", "8080", "0.0.0.0", "5"]
