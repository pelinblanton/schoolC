/*Tried to use a built in library for semaphores- created my own P and V semaphores
* program reads in the "card" text from a file called “semaphore.txt”
* each card is 20 characters long
* to simulate card input producer reads from a file where each line has 20 characters.
* the program will replace double pound symbols, ##, with an asterisk, * .
* printer process: prints 25-character lines to the screen 
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
//#include <ezipcv2.h> 
#define CARD_SIZE 20

char *buffer; //single shared character buffer
char *printer_buffer;
//int SEM_PROD, SEM_SQUASH, SEM_PRINT;
char *SEM_PROD, *SEM_PRINT, *SEM_SQUASH;

void setup_shared_resources() {
    buffer = SHARED_MEMORY(1); //single character buffer
    printer_buffer = SHARED_MEMORY(1); //single character printer buffer

    //SEM_PROD = SEMAPHORE(SEM_BIN, 1); //producer semaphore, buffer empty initially
    SEM_PROD = SHARED_MEMORY(1);
    SEM_PRINT = SHARED_MEMORY(1);
    SEM_SQUASH = SHARED_MEMORY(1);
    SEM_PROD[0] = '1';
    
    SEM_PRINT[0] = '0';
    SEM_SQUASH[0] = '0';
    //SEM_SQUASH = SEMAPHORE(SEM_BIN, 0); //squash semaphore, buffer full initially
    //SEM_PRINT = SEMAPHORE(SEM_BIN, 0); //printer semaphore
}

void p(char* sem){
    while(sem[0] != '1'){
        //sleep(0.1);
    }
}

void v(char* sem){
    if(sem[0] == '1'){
        sem[0] = '0';
    } else {
        sem[0] = '1';
    }
    //sleep(0.1);
}

void producer() {
    FILE *file = fopen("semaphore.txt", "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    char card[CARD_SIZE + 1];
    while (fgets(card, sizeof(card), file)) {
        for (int i = 0; i < strlen(card); ++i) {
            //printf("prod 1: (%d,%d)\n", SEM_PROD, SEM_PRINT);
            p(SEM_PROD); //ensure buffer is empty
            //printf("prod 2: (%d,%d)\n", SEM_PROD, SEM_PRINT);

            buffer[0] = card[i]; //write character
            //printf("prod buffer: %c\n", buffer[0]);
            v(SEM_PROD);
            //sleep(0.1);
            v(SEM_SQUASH);
        }
            //sleep(0.1);
            //printf("prod 3: (%d,%d)\n", SEM_PROD, SEM_PRINT);

    }
    buffer[0] = '\0';

    v(SEM_PROD);
    v(SEM_SQUASH);
    fclose(file);

    exit(0);
}


void squash() {
    char last_char = '\0';  // to store the last character for checking ##
    while (1) {
        p(SEM_SQUASH);  // wait for data from producer
        if (buffer[0] == '\0') {  // check if producer is signaling end
           printer_buffer[0] = '\0';  // Signal end to printer
            v(SEM_PRINT);
            break;
        }
        else if(buffer[0] == '\n'){
             printer_buffer[0] = '\n';  
             continue;
        }
        else if(buffer[0] == '#'){
            if(last_char == '#'){
                printer_buffer[0] = '*';
                last_char = '\0';
                v(SEM_SQUASH);
                v(SEM_PRINT);  
                continue;
            } else {
                last_char = '#';
                v(SEM_SQUASH);
                v(SEM_PROD);
                continue;
            }
        }
        printer_buffer[0] = buffer[0];
        v(SEM_SQUASH);
        v(SEM_PRINT);  
    }
    exit(0);
}


void printer() {
    while (1) {
        p(SEM_PRINT); // Wait for data from squash
        if (printer_buffer[0] == '\0') {
            break;  // End of data stream
        }
        if (printer_buffer[0] == '\n') {
            printf("<EOL>");  // Print <EOL> instead of newline
        } else {
            printf("%c", printer_buffer[0]);  // Print character normally
        }
        v(SEM_PROD);  // Signal PROD to continue
    }
    exit(0);
}

int main() {
    SETUP();
    setup_shared_resources();

    int status1;
    int status2;
    int status3;

    int pid1;
    int pid2;
    int pid3;

    pid1 = fork();
    if (pid1 == -1) {
        perror("Fork failed");
        exit(1);
    }
    else if (pid1 == 0) {
        producer();
    }
    pid2 = fork();
    if (pid2 == -1) {
        perror("Fork failed");
        exit(1);
    }
    else if (pid2 == 0) {
        squash();
    }
    pid3 = fork();
    if (pid3 == -1) {
        perror("Fork failed");
        exit(1);
    }
    else if (pid3 == 0) {
        printer();
    }
 
    wait(&status1);
    wait(&status2);
    wait(&status3);

    return 0;
}