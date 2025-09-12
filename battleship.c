#include <stdio.h>
#include <stdbool.h>
void initialization()
{
        // empty for now
}
void teardown()
{
        // empty for now
}
void acceptInput()
{
        printf("Please enter a letter {A-J}");
        fgets(*letter, 1, stdin);
        printf("Please enter a number {0-9}");
        scanf("%d", &number);
}
void updateWorldState(char letter, int number)
{
        (number % 2 == 0) ? printf("Hit!") : printf("Miss!");
        // TODO add return (?)
}
void displayWorldState()
{
        // TODO Print result calculated in updateWorldState()
        return 0;
}
int main()
{
        // TODO "loop until flag is set"
        initialization();
        while() {
                acceptInput();
                updateWorldState();
                displayWorldState();
        }
        teardown();
        return 0;
}