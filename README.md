# TicTacToe
Project of the implementation of the Tris in Lingua C, architecture client server.
The project structure is as follows:

\`\`\`
.
├── Client
│   ├── client
│   ├── client.c
│   ├── Dockerfile
│   ├── funzioni.c
│   ├── funzioni.h
│   └── strutture.h
├── Comunicazione.h
├── docker-compose.yml
├── gen.sh
├── run.sh
├── runT.sh
└── Server
    ├── Dockerfile
    ├── funzioni
    │   ├── funzioni.c
    │   └── funzioni.h
    ├── server
    ├── server.c
    └── strutture.h
\`\`\`
### File Descriptions

#### Client/
- **`client`**: Compiled executable for the client application.
- **`client.c`**: Main source file for the client, handling communication with the server and game logic.
- **`Dockerfile`**: Docker configuration for building the client container.
- **`funzioni.c`**: Implementation of client-specific functions.
- **`funzioni.h`**: Header file declaring client-specific functions.
- **`strutture.h`**: Defines shared data structures used by the client.

#### Server/
- **`server`**: Compiled executable for the server application.
- **`server.c`**: Main source file for the server, managing connections, game sessions, and communication with clients.
- **`Dockerfile`**: Docker configuration for building the server container.
- **`funzioni/`**: Directory containing server-specific function files.
  - **`funzioni.c`**: Implementation of server-specific functions.
  - **`funzioni.h`**: Header file declaring server-specific functions.
- **`strutture.h`**: Defines shared data structures used by the server.

#### Root Files
- **`Comunicazione.h`**: Shared header file defining communication protocols and message formats between client and server.
- **`docker-compose.yml`**: Configuration file for orchestrating the client and server containers using Docker Compose.
- **`gen.sh`**: Script for compiling and running the project in a specific environment.
- **`run.sh`**: Script for compiling and launching the server and clients in separate terminal windows.
- **`runT.sh`**: Alternative script for running the server and clients, tailored for macOS environments.