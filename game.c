/*
 * main.c
 *
 * The game's implementation. The handle_client function is the entry point of
 * a new thread that's created whenever a new client connects.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <pthread.h> //for threading, link with lpthread
#include <time.h>
#include "game.h"
#define dprint(expr) printf(#expr " = %d\n", expr)

//Client input buffer
#define BUFFER_SIZE 2000

// Size of the code the player has to guess
#define CODE_SIZE 4

// Upper and lower bounds for each of the numbers the user has to guess
#define CODE_LO 1
#define CODE_HI 10

/*
 * Some messages to be sent to the client
 */

char *welcome_msg =
"###############################################################################\n"
"############                   Hello stranger.                     ############\n"
"###############################################################################\n"
"\n"
"In this game you have to guess a secret code of 4 unique numbers, each of them\n"
"between 1 and 10.\n"
"After you make a guess, I will tell you how many of the numbers in your list\n"
"were entirely correct and how many were in the secret list but in a different\n"
"position.\n"
"\n"
"For example let's say the secret code is [1 8 2 4] and you guess [1 2 3 4].\n"
"2 numbers are correct and in the correct place, and 1 number is misplaced.\n"
"\n"
"You can find this program's source code at\n"
"https://github.com/tech-no-crat/cows_and_bulls\n"
"\n"
"Good luck!\n";

char *input_prompt_msg = 
"\n"
"Enter your guess as 4 space-seperated numbers:\n";

char *success_msg =
"\n"
"###############################################################################\n"
"############                      You won :)                       ############\n"
"###############################################################################\n"
"\n"
"\n"
"Hanging up now, bye.\n";

char *score_msg = "Guess #%2d: Misplaced %d, Correct %d\n";

/*
 * Helper methods
 */

// Returns 1 if elem exists in the first n elements of arr,
// 0 otherwise.
char exists(int elem, int *arr, int n) {
    int i;
    for(i = 0; i < n; i++)
        if(arr[i] == elem) return 1;
    return 0;
}

// Generates and returns an array of size CODE_SIZE with random unique numbers
// Each of the numbers will be in the [CODE_LO..CODE_HI] range.
int *generate_code() {
    int i, *arr;
    arr = malloc(CODE_SIZE * sizeof(int));
    srand(time(NULL));

    for(i = 0; i < CODE_SIZE; i++) {
        // Produce a random number until it doesn't exist
        // in the array up to this point.
        do {
            arr[i] = CODE_LO + rand() % CODE_HI;
        } while(exists(arr[i], arr, i));
    }

    return arr;
}

// Sends a message to a socket
// Avoids the annoying repetition of the char pointer in the
// second and third arguments of write().
void say(int socket, char *msg) {
    write(socket, msg, strlen(msg));
}

// Returns the number of correct elements, as defined in the game.
int count_correct(int *code, int *guess) {
    int i, count = 0;
    for(i = 0; i < CODE_SIZE; i++)
        if(code[i] == guess[i]) ++count;
    return count;
}

// Returns the number of misplaced elements, as defined in the game.
int count_misplaced(int *code, int *guess) {
    int i, j, count = 0;
    for(i = 0; i < CODE_SIZE; i++)
        for(j = 0; j < CODE_SIZE; j++)
            if(i!=j && code[i] == guess[j]) count++; 
    return count;
}

/*
 * Main game loop
 */

// Plays a game with the client connected to the given socket descriptior.
int play_game(int socket) {
    char buffer[BUFFER_SIZE];
    printf("[%d] Starting game with client.\n", socket); 

    // Generate the secret, random code. We know its size will be CODE_SIZE.
    int *code = generate_code();
    printf("[%d] Secret code set to [%d %d %d %d]\n", socket, code[0], code[1], code[2], code[3]); 

    int guess[CODE_SIZE];
    int tries = 0, correct = 0;
    char client_connected = 1;

    // Write the welcome message
    say(socket, welcome_msg);
    do {
        say(socket, input_prompt_msg); // Ask the client for the numbers

        printf("[%d] Waiting for client's guess.\n", socket); 
        // Read the client's response
        // TODO [IMPORTANT]: This assumes that each line will conveniently
        // arrive in a seperate call to recv (which seems to be the case
        // when dealing with human players using unix netcat to play).
        int bytes_read = recv(socket, buffer, BUFFER_SIZE, 0);
        if(bytes_read == 0) {
            client_connected = 0; // Whoops :(
            printf("[%d] Client disconnected.\n", socket); 
            continue;
        }

        // Parse the numbers
        // TODO: Do this in a way that works for CODE_SIZE elements in general
        sscanf(buffer, "%d %d %d %d", &guess[0], &guess[1], &guess[2], &guess[3]);
        ++tries;
        printf("[%d] Client sent guess number %d, [%d %d %d %d]\n", socket, tries, guess[0], guess[1], guess[2], guess[3]); 
        
        // Calculate the score
        correct = count_correct(code, guess);
        int misplaced = count_misplaced(code, guess);
        printf("[%d] Correct = %d, misplaced = %d\n", socket, correct, misplaced); 

        // Tell the player his score
        // TODO: Find a more appropriate size
        char msg[sizeof(score_msg) * 2 + 10];
        sprintf(msg, score_msg, tries, misplaced, correct);
        say(socket, msg);
    } while(client_connected && correct != CODE_SIZE);

    if(client_connected) {
        // The user found the code!
        // Now say goodbye and hang up.
        printf("[%d] Player won, hanging up.\n", socket); 
        say(socket, success_msg);
        close(socket);
    }

    free(code);
    return 0;
}

// Entry point for the thread
// Starts a new game.
void *handle_client(void *socket_d) {
    int socket = *(int *)socket_d;
    play_game(socket);
    return 0;
}
