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
├── Windows.bat
├── Unix.sh
├── runWin.ps1
└── Mac.sh
```
### File Descriptions

#### Client/
- **`Comunicazione.h`**: Shared header file defining communication protocols and message formats between client and server.
- **`client.c`**: Main source file for the client, handling communication with the server and game logic.
- **`Dockerfile`**: Docker configuration for building the client container.
- **`funzioni.c`**: Implementation of client-specific functions.
- **`funzioni.h`**: Header file declaring client-specific functions.
- **`strutture.h`**: Defines shared data structures used by the client.

#### Server/
- **`Comunicazione.h`**: Shared header file defining communication protocols and message formats between client and server.
- **`server.c`**: Main source file for the server, managing connections, game sessions, and communication with clients.
- **`Dockerfile`**: Docker configuration for building the server container.
- **`funzioni/`**: Directory containing server-specific function files.
  - **`funzioni.c`**: Implementation of server-specific functions.
  - **`funzioni.h`**: Header file declaring server-specific functions.
- **`strutture.h`**: Defines shared data structures used by the server.

#### Root Files
- **`docker-compose.yml`**: Configuration file for orchestrating the client and server containers using Docker Compose.
- **`Windows.bat`**: Script for compiling and launching the server and clients in windows.
- **`Unix.sh`**: Script for compiling and running the project in a specific environment.
- **`runWin.ps1`**: Script for compiling and launching the server and clients by Double Click on Windows.bat.
- **`Mac.sh`**: Alternative script for running the server and clients, tailored for macOS environments.

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
     chmod +x Mac.sh
     ./Mac.sh
     ```

   - **Linux**:
     ```
     chmod +x Unix.sh
     ./Unix.sh
     ```

   - **Windows**:
     Double-click on `Windows.bat` or run PowerShell as administrator:
     ```
     .\runWin.ps1
     ```

3. Follow the on-screen instructions to start the desired number of clients.

4. Play Tic-Tac-Toe! The server will manage connections and matches between clients.

### Note
- Clients are launched in separate terminal windows.
- The first client can create a game, while others can join.
- Each game is a 1v1 Tic-Tac-Toe session.

