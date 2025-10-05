#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

bool isValidInput(char* input, int shipLength);
void displayPlayerGrid();
void shipPlacement();
void initialization();
void teardown();
bool acceptInput(char *letter, int *number);
char* updateWorldState(char letter, int number);
void displayWorldState(char* result);

typedef enum {
    NO_SHIP,
    CARRIER,
    BATTLESHIP,
    CRUISER,
    SUBMARINE,
    DESTROYER
} ShipType;
typedef enum {
    HIT,
    MISS,
    UNTRIED
} ShotStatus;

ShipType** playerGrid; // Pointer for 2D array grid of placed ships
ShotStatus** shotGrid; // Pointer for 2D array grid of shots
const int GRID_SIZE = 10;

// TODO add support for C3-C7 format
bool isValidInput(char* input, int shipLength) {
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

    // Convert rows from char to int
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
// assumes input was already validated
void placeShip(char* input, ShipType type) {
    char startRowChar = 0, endRowChar = 0;
    int startCol = -1, endCol = -1;
    bool isAcross = false;

    if (sscanf(input, " %c%1d%1d", &startRowChar, &startCol, &endCol) == 3) {
        isAcross = true; endRowChar = startRowChar;
    } else if (sscanf(input, " %c%1d%1d", &startRowChar, &startCol, &endCol) == 3) {
        if (startCol >=0 && startCol <=9 && endCol >=0 && endCol <=9) { isAcross = true; endRowChar = startRowChar; }
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
    printf("Place your ships:\n");   
    displayPlayerGrid(); 
    // Carrier
    while (1) {
        printf("Please enter a location for your Carrier (size 5), ex: C37 or CG4: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return; 
        if (isValidInput(buffer, 5)) { placeShip(buffer, CARRIER); break; }
    }
    displayPlayerGrid(); 
    // Battleship
    while (1) {
        printf("Please enter a location for your Battleship (size 4), ex: C36 or CF4: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        if (isValidInput(buffer, 4)) { placeShip(buffer, BATTLESHIP); break; }
    }
    displayPlayerGrid(); 
    // Cruiser
    while (1) {
        printf("Please enter a location for your Cruiser (size 3), ex: C35 or CE4: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        if (isValidInput(buffer, 3)) { placeShip(buffer, CRUISER); break; }
    }
    displayPlayerGrid(); 
    // Submarine
    while (1) {
        printf("Please enter a location for your Submarine (size 2), ex: C34 or CD4: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        if (isValidInput(buffer, 2)) { placeShip(buffer, SUBMARINE); break; }
    }
    displayPlayerGrid(); 
    // Destroyer
    while (1) {
        printf("Please enter a location for your Destroyer (size 1), ex: C33 or CC3: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) return;
        if (isValidInput(buffer, 1)) { placeShip(buffer, DESTROYER); break; }
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
            }
        }
        printf("\n");
    }
}
void initialization(){
    playerGrid = malloc(GRID_SIZE * sizeof(ShipType*));
    for (int i = 0; i < GRID_SIZE; i++) {
        playerGrid[i] = malloc(GRID_SIZE * sizeof(ShipType));
        // Initialize grid to all no ships
        for (int j = 0; j < GRID_SIZE; j++) {
            playerGrid[i][j] = NO_SHIP;
        }
    }
    shotGrid = malloc(GRID_SIZE * sizeof(ShotStatus*));
    for (int i = 0; i < GRID_SIZE; i++) {
        shotGrid[i] = malloc(GRID_SIZE * sizeof(ShotStatus));
        // Initialize grid to all untried spots
        for (int j = 0; j < GRID_SIZE; j++) {
            shotGrid[i][j] = UNTRIED;
        }
    }
    shipPlacement();
    displayPlayerGrid();
}
void teardown(){
    for (int i = 0; i < GRID_SIZE; i++) {
        free(playerGrid[i]);
        free(shotGrid[i]);
    }
    free(playerGrid);
    free(shotGrid);
    printf("Game Over");
}
bool acceptInput(char *letter, int *number) {
	char buffer[100];
	bool isValid = false;
	do {
		printf("Please enter a letter {A-J}, or quit game with {Q}:");
		if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
			continue; // if fgets returns NULL, skip loop and reprompt
		}
		char c = buffer[0];
        // force convert char input to uppercase
		if (c >= 'a' && c <= 'z') {
			c = c - 'a' + 'A';
		}
        if (c == 'Q') {
            return false; // return false if user quit
        }
		if (c >= 'A' && c <= 'J') {
			*letter = c;
			isValid = true;
		} else {
			printf("Error: Letter must be between A and J.\n");
			isValid = false;
		}
	} while (!isValid);
	isValid = false;
	do {
		printf("Please enter a number {0-9}:");
		if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
			continue;
		}
		int i;
		// Check if the input is a valid integer between 0 and 9 
		if ((sscanf(buffer, "%d", &i) == 1) && (i >= 0 && i <= 9)) {
			*number = i;
			isValid = true;
		} else {
			printf("Please enter a number between 0 and 9\n");
			isValid = false;
		}
	} while (!isValid);
    return true; // return false if game continues
}
char* updateWorldState(char letter, int number) { 
    return (number % 2 == 0) ? "Hit!" : "Miss!"; 
}
void displayWorldState(char* result) { 
    printf("%s\n", result);
    
}
int main() {
	char letter;
	int number;
    bool isRunning = true;
	char* result;
	initialization();
	while(isRunning) {
        if (!acceptInput(&letter, &number)) {
            isRunning = false;
            continue;
        }
		result = updateWorldState(letter, number);
		displayWorldState(result);
	}
	teardown();
	return 0;
}