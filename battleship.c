#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BACKLOG 1

typedef struct { int row; int col; } Coordinates; // Struct for passing coordinates between functions
typedef enum { NO_SHIP, DESTROYED, CARRIER, BATTLESHIP, CRUISER, SUBMARINE, DESTROYER } ShipType;
typedef enum { HIT, MISS, UNTRIED } ShotStatus;

ShipType** playerGrid; // Pointer for user's 2D array grid of placed ships
ShotStatus** shotGrid; // Pointer for user's 2D array grid of shots 
ShipType** playerGridCPU; // Pointer for Single Player CPU's 2D array grid of placed ships
ShotStatus** shotGridCPU; // Pointer for Single Player CPU's 2D array grid of shots 

const int GRID_SIZE = 10;
int sockfd, new_fd;
bool isServer = false;

// Forward declarations
void twoPlayerResponse(Coordinates shot, const char* result);
Coordinates getTwoPlayerShot();
char* makeTwoPlayerShot(char letter, int col);
void setupTwoPlayer();
void teardownTwoPlayer();
bool isValidInput(char* input, int shipLength);
void placeShip(char* input, ShipType type);
void shipPlacement();
void displayPlayerGrid();    
void displayShotGrid();      
void initialization();
void teardown();
bool acceptInput(char *letter, int *number);
bool checkWin(ShipType** grid);
bool checkTwoPlayerWin();
char* updateWorldState(char letter, int number, char** cpuShotResult);
void displayWorldState(char* playerShotResult);
// Single player helpers
void placeSinglePlayerShips(void);

// ----- Begin Single Player CPU Implementation -----
void singlePlayerResponse(Coordinates shot, const char* result) {
    // strcmp returns 0 when strings are equal.
    if (strcmp(result, "Hit!") == 0) {
        shotGridCPU[shot.row][shot.col] = HIT;
    } else {
        shotGridCPU[shot.row][shot.col] = MISS;
    }
}
Coordinates getSinglePlayerShot() {
    bool isValid = false;
    int row, col;
    Coordinates shot;
    while (!isValid) {
        row = rand() % 10;
        col = rand() % 10;
        if (shotGridCPU[row][col] == UNTRIED) {
            isValid = true;
            shot.row = row;
            shot.col = col;
        }
    }
    return shot;
}
char* makeSinglePlayerShot(char letter, int col) {
    int row = letter - 'A';
    bool isHit = false;
    // Player fires at CPU: mark player's shot grid; mark CPU ship cell as DESTROYED when hit.
    if (playerGridCPU[row][col] != NO_SHIP) {
        isHit = true;
        shotGrid[row][col] = HIT;
        playerGridCPU[row][col] = DESTROYED;
    } else {
        shotGrid[row][col] = MISS;
    }
    return isHit ? "Hit!" : "Miss!"; 
}
void setupSinglePlayer() {
    playerGridCPU = malloc(GRID_SIZE * sizeof(ShipType*));
    // Check / handle malloc failure
    if (!playerGridCPU) {
        printf("ERROR: Memory allocation failed for playerGridCPU.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < GRID_SIZE; i++) {
        playerGridCPU[i] = malloc(GRID_SIZE * sizeof(ShipType));
        // Check / handle malloc failure
        if (!playerGridCPU[i]) {
            printf("ERROR: Memory allocation failed for playerGridCPU.\n");
            // free already allocated rows
            for (int k = 0; k < i; k++) {
                free(playerGridCPU[k]);
            }
            free(playerGridCPU);
            exit(EXIT_FAILURE);
        }
        // Initialize grid to all no ships
        for (int j = 0; j < GRID_SIZE; j++) {
            playerGridCPU[i][j] = NO_SHIP;
        }
    }
    shotGridCPU = malloc(GRID_SIZE * sizeof(ShotStatus*));
    // Check / handle malloc failure
    if (!shotGridCPU) {
        printf("ERROR: Memory allocation failed for shotGridCPU.\n");
        for (int i = 0; i < GRID_SIZE; i++) {
            free(playerGridCPU[i]);
        }
        free(playerGridCPU);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < GRID_SIZE; i++) {
        shotGridCPU[i] = malloc(GRID_SIZE * sizeof(ShotStatus));
        if (!shotGridCPU[i]) {
            printf("ERROR: Memory allocation failed for shotGridCPU row.\n");
            for (int k = 0; k < i; k++) {
                free(shotGridCPU[k]);
            }
            free(shotGridCPU);
            for (int k = 0; k < GRID_SIZE; k++) {
                free(playerGridCPU[k]);
            }
            free(playerGridCPU);
            exit(EXIT_FAILURE);
        }
        // Initialize grid to all untried spots
        for (int j = 0; j < GRID_SIZE; j++) {
            shotGridCPU[i][j] = UNTRIED;
        }
    }
    placeSinglePlayerShips();
}
void placeSinglePlayerShips() {
    // Randomly place all CPU ships.
    ShipType ships[] = { CARRIER, BATTLESHIP, CRUISER, SUBMARINE, DESTROYER };
    int shipLengths[] = {5, 4, 3, 2, 1};
    int numShips = 5;

    for (int i = 0; i < numShips; i++) {
        ShipType type = ships[i];
        int shipLength = shipLengths[i];
        bool validPlacement = false;

        while (!validPlacement) {
            int isAcross = rand() % 2; // 0 => vertical placement, 1 => horizontal placement
            int startRow = rand() % GRID_SIZE;
            int startCol = rand() % GRID_SIZE;

            if (isAcross) {
                // Check if ship fits horizontally
                if (startCol + shipLength > GRID_SIZE) continue; 
                // Check for overlaps
                bool overlaps = false;
                for (int c = startCol; c < startCol + shipLength; c++) {
                    if (playerGridCPU[startRow][c] != NO_SHIP) {
                        overlaps = true;
                        break;
                    }
                }
                if (overlaps) continue; // Try new random spot
                // Place ship
                for (int c = startCol; c < startCol + shipLength; c++) {
                    playerGridCPU[startRow][c] = type;
                }
                validPlacement = true;
            } else {
                // Check if ship fits vertically
                if (startRow + shipLength > GRID_SIZE) continue;

                // Check for overlaps
                bool overlaps = false;
                for (int r = startRow; r < startRow + shipLength; r++) {
                    if (playerGridCPU[r][startCol] != NO_SHIP) {
                        overlaps = true;
                        break;
                    }
                }
                if (overlaps) continue; // Try new random spot
                // Place ship
                for (int r = startRow; r < startRow + shipLength; r++) {
                    playerGridCPU[r][startCol] = type;
                }
                validPlacement = true;
            }
        }
    }
}
void teardownSinglePlayer() {
    for (int i = 0; i < GRID_SIZE; i++) {
        free(playerGridCPU[i]);
        free(shotGridCPU[i]);
    }
    free(playerGridCPU);
    free(shotGridCPU);
}
// ----- Begin Two Player Implementation -----
void setupTwoPlayer() {
    printf("Waiting for opponent connection...\n");
    if (isServer) {
        struct sockaddr_storage their_addr;
        socklen_t sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        printf("Opponent connected!\n");
    } else {
        new_fd = sockfd; // client already connected
    }
}

void teardownTwoPlayer() {
    close(new_fd);
    if (isServer) close(sockfd);
}

Coordinates getTwoPlayerShot() {
    char buf[10];
    int n = recv(new_fd, buf, sizeof(buf) - 1, 0);
    buf[n] = '\0';
    Coordinates shot;
    shot.row = buf[0] - 'A';
    shot.col = atoi(buf + 1);
    return shot;
}

char* makeTwoPlayerShot(char letter, int col) {
    char msg[10];
    snprintf(msg, sizeof(msg), "%c%d", letter, col);
    send(new_fd, msg, strlen(msg), 0);
    char response[10];
    int n = recv(new_fd, response, sizeof(response) - 1, 0);
    if (n > 0) {
        response[n] = '\0';
        // Update our shot grid based on response
        int row = letter - 'A';
        if (strcmp(response, "Hit!") == 0) {
            shotGrid[row][col] = HIT;
        } else {
            shotGrid[row][col] = MISS;
        }
        // strdup is POSIX; to avoid implicit declaration warnings across environments, do a manual copy
        char* copy = (char*)malloc((size_t)n + 1);
        if (!copy) {
            return "Error";
        }
        memcpy(copy, response, (size_t)n + 1);
        return copy;
    }
    return "Error";
}

void twoPlayerResponse(Coordinates shot, const char* result) {
    send(new_fd, result, strlen(result), 0);
    // Update our grid when we're hit
    if (strcmp(result, "Hit!") == 0 && playerGrid[shot.row][shot.col] != NO_SHIP) {
        playerGrid[shot.row][shot.col] = DESTROYED;
    }
}
// ----- Start Main User Implementations -----
bool isValidInput(char* input, int shipLength) {
    // Accepts formats: "C37" (across C3-C7) or "CG4" (down C4-G4). Validates bounds, length, and overlap.
    char startRowChar = 0, endRowChar = 0;
    int startCol = -1, endCol = -1, startRowInt = -1, endRowInt = -1;
    bool isAcross = false;

    // Check if across or down format
    if (sscanf(input, " %c%1d%1d", &startRowChar, &startCol, &endCol) == 3) {
        isAcross = true;
        endRowChar = startRowChar;
    } else if (sscanf(input, " %c%c%d", &startRowChar, &endRowChar, &startCol) == 3) {
        isAcross = false;
    } else {
        printf("ERROR: Invalid Input. Please use format like C37 (C3-C7) or CG4 (C4-G4)\n");
        return false;
    }

    // Normalize to uppercase so inputs like "c37" or "cG4" are accepted
    if (startRowChar >= 'a' && startRowChar <= 'z') startRowChar -= ('a' - 'A');
    if (endRowChar   >= 'a' && endRowChar   <= 'z') endRowChar   -= ('a' - 'A');

    // Convert rows from char to int (A->0 .. J->9)
    startRowInt = startRowChar - 'A';
    endRowInt = endRowChar - 'A';

    // Check if input is within bounds
    if (startRowInt < 0 || startRowInt >= GRID_SIZE) {
        printf("ERROR: Start row out of bounds.\n");
        return false;
    }
    if (isAcross) {
        if (startCol < 0 || endCol < 0 || startCol >= GRID_SIZE || endCol >= GRID_SIZE) {
            printf("ERROR: Column out of bounds.\n");
            return false;
        }
        if (endCol < startCol) {
            printf("ERROR: End column must be >= start column.\n");
            return false;
        }
        if ((endCol - startCol + 1) != shipLength) {
            printf("ERROR: Invalid length. Expected ship length %d.\n", shipLength);
            return false;
        }
    } else { // verticle
        if (endRowInt < 0 || endRowInt >= GRID_SIZE) {
            printf("ERROR: End row out of bounds.\n");
            return false;
        }
        if (startCol < 0 || startCol >= GRID_SIZE) {
            printf("ERROR: Column out of bounds.\n");
            return false;
        }
        if (endRowInt < startRowInt) {
            printf("ERROR: End row must be >= start row.\n");
            return false;
        }
        if ((endRowInt - startRowInt + 1) != shipLength) {
            printf("ERROR: Invalid length. Expected ship length %d.\n", shipLength);
            return false;
        }
    }
    // Check for overlap
    if (isAcross) {
        for (int c = startCol; c <= endCol; c++) {
            if (playerGrid[startRowInt][c] != NO_SHIP) {
                printf("ERROR: Overlaps an existing ship.\n");
                return false;
            }
        }
    } else {
        for (int r = startRowInt; r <= endRowInt; r++) {
            if (playerGrid[r][startCol] != NO_SHIP) {
                printf("ERROR: Overlaps an existing ship.\n");
                return false;
            }
        }
    }
    return true;
}
// Assumes input was already validated
void placeShip(char* input, ShipType type) {
    // Parses orientation and fills playerGrid accordingly. Duplicate sscanf branch retained for robustness.
    char startRowChar = 0, endRowChar = 0;
    int startCol = -1, endCol = -1;
    bool isAcross = false;

    if (sscanf(input, " %c%1d%1d", &startRowChar, &startCol, &endCol) == 3) {
        isAcross = true; 
        endRowChar = startRowChar;
    } else if (sscanf(input, " %c%1d%1d", &startRowChar, &startCol, &endCol) == 3) {
        if (startCol >=0 && startCol <=9 && endCol >=0 && endCol <=9) { 
            isAcross = true; 
            endRowChar = startRowChar; 
        }
    } else if (sscanf(input, " %c%c%d", &startRowChar, &endRowChar, &startCol) == 3) {
        isAcross = false;
    }

    if (startRowChar >= 'a' && startRowChar <= 'z') startRowChar -= ('a' - 'A');
    if (endRowChar   >= 'a' && endRowChar   <= 'z') endRowChar   -= ('a' - 'A');

    int startRowInt = startRowChar - 'A';
    int endRowInt   = endRowChar   - 'A';

    if (isAcross) {
        for (int c = startCol; c <= endCol; ++c) {
            playerGrid[startRowInt][c] = type;
        }
    } else {
        for (int r = startRowInt; r <= endRowInt; ++r) {
            playerGrid[r][startCol] = type;
        }
    }
}
void shipPlacement() {
    char buffer[10000];
    char* currentShip;
    ShipType ships[] = { CARRIER, BATTLESHIP, CRUISER, SUBMARINE, DESTROYER };
    int shipLengths[] = { 5, 4, 3, 2, 1 };
    int numShips = 5;

    printf("Place your ships:\n");
    displayPlayerGrid(); 
    for (int i = 0; i < numShips; i++) {
        switch (ships[i]) {
            case DESTROYER: currentShip = "Destroyer"; break;
            case SUBMARINE: currentShip = "Submarine"; break;
            case CRUISER: currentShip = "Cruiser"; break;
            case BATTLESHIP: currentShip = "Battleship"; break;
            case CARRIER: currentShip = "Carrier"; break;
            default: currentShip = "Unknown"; break; // should never occur.
        }
        while (true) {
            // Example placeholders: (7 - i) shows last column for horizontal example; ('G' - i) shows last row for vertical example
            printf("Please enter a location for your %s (size %d), ex: C3%d or C%c3: ", currentShip, shipLengths[i], (7 - i), ('G' - i));
            if (!fgets(buffer, sizeof(buffer), stdin)) return;
            if (isValidInput(buffer, shipLengths[i])) { placeShip(buffer, ships[i]); break; }
        }
        displayPlayerGrid(); 
    }
    printf("All ships have been placed:\n");
}
void displayPlayerGrid() {
    printf("  0 1 2 3 4 5 6 7 8 9\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%c ", 'A' + i);
        for (int j = 0; j < GRID_SIZE; j++) {
            switch (playerGrid[i][j]) {
                case NO_SHIP:    printf(". "); break;
                case CARRIER:    printf("C "); break;
                case BATTLESHIP: printf("B "); break;
                case CRUISER:    printf("R "); break;
                case SUBMARINE:  printf("S "); break;
                case DESTROYER:  printf("D "); break;
                case DESTROYED:  printf("X "); break;
            }
        }
        printf("\n");
    }
}
void displayShotGrid() {
    printf("  0 1 2 3 4 5 6 7 8 9\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%c ", 'A' + i);
        for (int j = 0; j < GRID_SIZE; j++) {
            switch (shotGrid[i][j]) {
                case HIT:     printf("H "); break;
                case MISS:    printf("M "); break;
                case UNTRIED: printf(". "); break;
            }
        }
        printf("\n");
    }
}
void initialization() {
    srand(time(0)); // seed rng once during initialization.
    setupSinglePlayer();
    playerGrid = malloc(GRID_SIZE * sizeof(ShipType*));
    // Check / handle malloc failure
    if (!playerGrid) {
        printf("ERROR: Memory allocation failed for playerGrid.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < GRID_SIZE; i++) {
        playerGrid[i] = malloc(GRID_SIZE * sizeof(ShipType));
        // Check / handle malloc failure
        if (!playerGrid[i]) {
            printf("ERROR: Memory allocation failed for playerGrid.\n");
            // free already allocated rows
            for (int k = 0; k < i; k++) {
                free(playerGrid[k]);
            }
            free(playerGrid);
            exit(EXIT_FAILURE);
        }
        // Initialize grid to all no ships
        for (int j = 0; j < GRID_SIZE; j++) {
            playerGrid[i][j] = NO_SHIP;
        }
    }
    shotGrid = malloc(GRID_SIZE * sizeof(ShotStatus*));
    // Check / handle malloc failure
    if (!shotGrid) {
        printf("ERROR: Memory allocation failed for shotGrid.\n");
        for (int i = 0; i < GRID_SIZE; i++) {
            free(playerGrid[i]);
        }
        free(playerGrid);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < GRID_SIZE; i++) {
        shotGrid[i] = malloc(GRID_SIZE * sizeof(ShotStatus));
        if (!shotGrid[i]) {
            printf("ERROR: Memory allocation failed for shotGrid row.\n");
            for (int k = 0; k < i; k++) {
                free(shotGrid[k]);
            }
            free(shotGrid);
            for (int k = 0; k < GRID_SIZE; k++) {
                free(playerGrid[k]);
            }
            free(playerGrid);
            exit(EXIT_FAILURE);
        }
        // Initialize grid to all untried spots
        for (int j = 0; j < GRID_SIZE; j++) {
            shotGrid[i][j] = UNTRIED;
        }
    }
    shipPlacement();
    displayPlayerGrid();
}
void teardown() {
    // Free all allocated rows for both player grids, then the row arrays themselves
    for (int i = 0; i < GRID_SIZE; i++) {
        free(playerGrid[i]);
        free(shotGrid[i]);
    }
    free(playerGrid);
    free(shotGrid);
    teardownSinglePlayer();
}
bool acceptInput(char *letter, int *number) {
    // Prompt until the user provides a untried coordinate.
    // Returns false if user chooses to quit with 'Q'.
    char buffer[100];
    while (1) {
        // 1) Get a valid letter A-J (accept lowercase by normalizing)
        bool isValid = false;
        do {
            printf("Please enter a letter {A-J}, or quit game with {Q}:");
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                continue; // if fgets returns NULL, skip loop and reprompt
            }
            char c = buffer[0];
            if (c >= 'a' && c <= 'z') {
                c = c - 'a' + 'A'; // normalize to uppercase
            }
            if (c == 'Q') {
                return false; // user quit
            }
            if (c >= 'A' && c <= 'J') {
                *letter = c;
                isValid = true;
            } else {
                printf("Error: Letter must be between A and J.\n");
                isValid = false;
            }
        } while (!isValid);

        // 2) Get a valid number 0-9
        isValid = false;
        do {
            printf("Please enter a number {0-9}:");
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                continue;
            }
            int i;
            if ((sscanf(buffer, "%d", &i) == 1) && (i >= 0 && i <= 9)) {
                *number = i;
                isValid = true;
            } else {
                printf("Please enter a number between 0 and 9\n");
                isValid = false;
            }
        } while (!isValid);

        // 3) Reject coordinates previously targeted; re-prompt both fields
        int row = *letter - 'A';
        int col = *number;
        if (shotGrid[row][col] != UNTRIED) {
            printf("You already fired at %c%d. Pick a different target.\n", *letter, *number);
            continue; // restart prompts
        }
        return true;
    }
}
bool checkWin(ShipType** grid) {
    // Game over when no cell contains a ship type other than DESTROYED/NO_SHIP
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            // If any spot is a ship (and not destroyed), the game is not over
            if (grid[i][j] != NO_SHIP && grid[i][j] != DESTROYED) {
                return false; 
            }
        }
    }
    return true; // All ship parts are DESTROYED
}
bool checkTwoPlayerWin() {
    // In two-player mode, count total hits to see if we've sunk all 5 ships (5+4+3+2+1 = 15 cells)
    int hitCount = 0;
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (shotGrid[i][j] == HIT) {
                hitCount++;
            }
        }
    }
    return hitCount >= 15; // All enemy ships destroyed (carrier=5 + battleship=4 + cruiser=3 + submarine=2 + destroyer=1)
}
char* updateWorldState(char letter, int number, char** cpuShotResult) { 
    // Resolve player's shot on CPU, then CPU takes a random shot at player; update both shot grids
    char* playerShotResult = makeSinglePlayerShot(letter, number);
    Coordinates cpuShot = getSinglePlayerShot();
    char cpuShotRowChar = cpuShot.row + 'A'; // Convert 0-based row to display letter
    if (playerGrid[cpuShot.row][cpuShot.col] != NO_SHIP && playerGrid[cpuShot.row][cpuShot.col] != DESTROYED) {
        playerGrid[cpuShot.row][cpuShot.col] = DESTROYED;
        *cpuShotResult = "Hit!";
        printf("\nThe computer fired at %c%d and it was a Hit!", cpuShotRowChar, cpuShot.col);
    } else {
        *cpuShotResult = "Miss!";
        printf("\nThe computer fired at %c%d and it was a Miss!", cpuShotRowChar, cpuShot.col);
    }
    singlePlayerResponse(cpuShot, *cpuShotResult);
    return playerShotResult;
}
void displayWorldState(char* playerShotResult) { 
    printf("\nYour shot was a %s\n", playerShotResult);
    
    printf("\nYOUR SHOTS (H=Hit, M=Miss)\n");
    displayShotGrid();
    
    printf("\nYOUR SHIPS (X=Hit)\n");
    displayPlayerGrid();
    printf("\n");
}
int main(int argc, char *argv[]) {
	char letter;
	int number;
    bool isRunning = true;
	char* playerShotResult;
    char* cpuShotResult;
    if (argc == 2) { // server mode: one parameter = port
        int yes=1, rv;
        const char* port = argv[1];
        struct addrinfo hints, *servinfo, *p;

        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        rv = getaddrinfo(NULL, port, &hints, &servinfo); // get local address

        for (p = servinfo; p != NULL; p = p->ai_next) {
            // create socket
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                continue;
            }
            // bind to address
            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { close(sockfd); continue; }
            break;
        }
        freeaddrinfo(servinfo);
        // listen for connections
        if (listen(sockfd, BACKLOG) == -1) {
            perror("listen");
            exit(1);
        }
        printf("Server listening on port %s...\n", port);

        isServer = true;
        setupTwoPlayer();
        initialization();
        bool myTurn = true;
        char* result;
        while (1) {
            if (myTurn) {
                printf("\n=== YOUR TURN ===\n");
                displayShotGrid();
                if (!acceptInput(&letter, &number)) break;
                result = makeTwoPlayerShot(letter, number);
                printf("You fired at %c%d and it was a %s\n", letter, number, result);
                free(result); // Free the allocated memory
                if (checkTwoPlayerWin()) {
                    printf("YOU WIN! You sank all enemy ships!\n");
                    break;
                }
            } else {
                printf("\n=== OPPONENT'S TURN ===\n");
                Coordinates shot = getTwoPlayerShot();
                printf("Opponent fired at %c%d\n", 'A' + shot.row, shot.col);
                // Check if it's a hit on our grid
                bool isHit = (playerGrid[shot.row][shot.col] != NO_SHIP && 
                             playerGrid[shot.row][shot.col] != DESTROYED);
                result = isHit ? "Hit!" : "Miss!";
                twoPlayerResponse(shot, result);
                printf("Result: %s\n", result);
                displayPlayerGrid();
                if (checkWin(playerGrid)) {
                    printf("YOU LOSE! The opponent sank all your ships!\n");
                    break;
                }
            }
            myTurn = !myTurn;
        }
        teardownTwoPlayer();
    } else if (argc == 3) { // client mode: first param = IP, second = port
        int rv;
        struct addrinfo hints, *servinfo, *p;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        // get destination info
        rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo);

        // try to connect
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) continue;
            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) { close(sockfd); continue; }
            break;
        }
        
        printf("Connected to server!\n");
        freeaddrinfo(servinfo);

        setupTwoPlayer();
        initialization();
        bool myTurn = false;
        char* result;
        while (1) {
            if (myTurn) {
                printf("\n=== YOUR TURN ===\n");
                displayShotGrid();
                if (!acceptInput(&letter, &number)) break;
                result = makeTwoPlayerShot(letter, number);
                printf("You fired at %c%d and it was a %s\n", letter, number, result);
                free(result); // Free the allocated memory
                if (checkTwoPlayerWin()) {
                    printf("YOU WIN! You sank all enemy ships!\n");
                    break;
                }
            } else {
                printf("\n=== OPPONENT'S TURN ===\n");
                Coordinates shot = getTwoPlayerShot();
                printf("Opponent fired at %c%d\n", 'A' + shot.row, shot.col);
                // Check if it's a hit on our grid
                bool isHit = (playerGrid[shot.row][shot.col] != NO_SHIP && 
                             playerGrid[shot.row][shot.col] != DESTROYED);
                result = isHit ? "Hit!" : "Miss!";
                twoPlayerResponse(shot, result);
                printf("Result: %s\n", result);
                displayPlayerGrid();
                if (checkWin(playerGrid)) {
                    printf("YOU LOSE! The opponent sank all your ships!\n");
                    break;
                }
            }
            myTurn = !myTurn;
        }
        teardownTwoPlayer();
    } else { // singleplayer mode: no parameters
        initialization();
        while(isRunning) {
            if (!acceptInput(&letter, &number)) { isRunning = false; continue; }
            playerShotResult = updateWorldState(letter, number, &cpuShotResult);
            displayWorldState(playerShotResult);
            if (checkWin(playerGridCPU)) {
                printf("YOU WIN! You sank all enemy ships!\n");
                isRunning = false;
            } else if (checkWin(playerGrid)) {
                printf("YOU LOSE! The computer sank all your ships!\n");
                isRunning = false;
            }
        }
        teardown();
    }
}