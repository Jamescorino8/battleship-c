#include <stdio.h>
#include <stdbool.h>
void initialization(){}
void teardown(){}
void acceptInput(char *letter, int *number) {
	char buffer[100];
	bool valid = false;
	do {
		printf("Please enter a letter {A-J}");
		if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
			continue;
		}
		char c = buffer[0];
		if (c >= 'a' && c <= 'z') {
			c = c - 'a' + 'A';
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
		printf("Please enter a number {0-9}");
		if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
			continue;
		}
		int i;
		if ((sscanf(buffer, "%d", &i) == 1) && (i >= 0 && i <= 9)) {
			*number = i;
			valid = true;
		} else {
			printf("Please enter a number between 0 and 9\n");
			valid = false;
		}
	} while (!valid);
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
		acceptInput(&letter, &number);
		result = updateWorldState(letter, number);
		displayWorldState(result);		
	}
	teardown();
	return 0;
}