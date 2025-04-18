# TicTacToe

Implementation of Tic-Tac-Toe (Tris) in C language with client-server architecture.

## Project Structure

```
.
tris-lso/
├── Client/
│   ├── Comunicazione.h
│   ├── Dockerfile
│   ├── client.c
│   ├── funzioni.c
│   ├── funzioni.h
│   └── strutture.h
├── Server/
│   ├── Comunicazione.h
│   ├── Dockerfile
│   ├── server.c
│   ├── funzioni.c
│   ├── funzioni.h
│   └── strutture.h
├── README.md
├── docker-compose.yml
├── eseguiWindows.bat
├── gen.sh
├── run.sh
├── runWin.ps1
└── runmac.sh
```
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

## Project Execution

### Prerequisites
- Docker installed and running
- Repository cloned or extracted to Desktop

### Execution Instructions

1. Open a terminal and navigate to the project folder:
   ```
   cd ~/Desktop/TicTacToe/tris-lso
   ```

2. Choose the appropriate script based on your operating system:

   - **macOS**:
     ```
     chmod +x runmac.sh
     ./runmac.sh
     ```

   - **Linux**:
     ```
     chmod +x run.sh
     ./run.sh
     ```

   - **Windows**:
     Double-click on `eseguiWindows.bat` or run PowerShell as administrator:
     ```
     .\runWin.ps1
     ```

3. Follow the on-screen instructions to start the desired number of clients.

4. Play Tic-Tac-Toe! The server will manage connections and matches between clients.

### Note
- Clients are launched in separate terminal windows.
- The first client can create a game, while others can join.
- Each game is a 1v1 Tic-Tac-Toe session.

