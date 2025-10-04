#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void displayPlayerGrid();
void staticShipPlacement();
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

ShipType** playerGrid; // Pointer for grid of placed ships
ShotStatus** shotGrid; // Pointer for grid of shots
const int GRID_SIZE = 10;

void shipPlacement() {
    
    // Place Carrier
    for (int i = 0; i < 5; i++) {
        playerGrid[0][i] = CARRIER;
    }
    // Place Battleship
    for (int i = 2; i < 6; i++) {
        playerGrid[i][2] = BATTLESHIP;
    }
    // Place Cruiser
    for (int i = 5; i < 8; i++) {
        playerGrid[4][i] = CRUISER;
    }
    //Place Submarine
    playerGrid[7][7] = SUBMARINE;
    playerGrid[8][7] = SUBMARINE;
    //Place Destroyer
    playerGrid[9][9] = DESTROYER;
    
    printf("All ships have been placed\n");
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
                default:         printf("? "); break;
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
	bool valid = false;
	do {
		printf("Please enter a letter {A-J}, or quit game with 'Q':");
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
			valid = true;
		} else {
			printf("Error: Letter must between A and J.\n");
			valid = false;
		}
	} while (!valid);
	valid = false;
	do {
		printf("Please enter a number {0-9}:");
		if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
			continue;
		}
		int i;
		// Check if the input is a valid integer between 0 and 9 
		if ((sscanf(buffer, "%d", &i) == 1) && (i >= 0 && i <= 9)) {
			*number = i;
			valid = true;
		} else {
			printf("Please enter a number between 0 and 9\n");
			valid = false;
		}
	} while (!valid);
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
