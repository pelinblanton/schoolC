/**
 * @author Pelin Blanton 
 * @date  October 27 , 2023
 * This program reads from an input file and determines the word frequency for words in the 
 * text file and the program is executed with two file names as command line arguments.
 * The program builds a list of word frequencies based on the words in the input file. 
 * If the input file does not exist or can't be opened, an error message is printed and the program exits.
 **/  

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define MAX_STRING_SIZE 20

/* Struct for each unique word and the count (number of times the word appears) */
typedef struct{
    char *word;
    int count;
} WordFreq;

void check_pointer( char * ptr) {
    return;
    if ( ptr == NULL ) {
    printf("NULL POINTER!!\n");
    return;
    }
    printf("%p\n", ptr);
}

void print_all_words(WordFreq **wfpp, int numWords) {
    int i;
    if(numWords == 0) { return; }
    for( i = 0; i < numWords; i++ ) {
        printf("%d %s\n", wfpp[i]->count, wfpp[i]->word);
    }
}

/* Sorts the list of WordFreq structs in decending order */
void bubbleSort(WordFreq **wfpp, int numWords) {
    int swap;
    int i;
    do {
        swap = 0;
        for (i = 1; i < numWords; i++) {
            if (wfpp[i - 1]->count < wfpp[i]->count) {
                /* Swaps the elements if they are in the wrong order */
                WordFreq *temp = wfpp[i - 1];
                wfpp[i - 1] = wfpp[i];
                wfpp[i] = temp;
                swap = 1; /* Set swap to 1 for true to continue the loop */
            }
        }
    } while (swap);
}

/* Outputs word list to output file */
int outputWords(char outFileName[], WordFreq **wfpp, int numWords){ 
      int i;
      FILE *outFile;
    /* Sorts the wfpp array in descending order based on count using bubble sort function */
    bubbleSort(wfpp, numWords);
    outFile = fopen(outFileName, "w");

    if (outFile == NULL) {
        fprintf(stderr, "Error opening output file.\n");
        return 1;
    }
    /* Prints the word and then the count */
    for(i = 0; i < numWords; i++) {
        fprintf(outFile, "%s %d\n", wfpp[i]->word, wfpp[i]->count);
    }

    fclose(outFile);
    return 0;
}

/* Adds a word to the word frequency list or the count it updated if the word exists */
WordFreq ** addToList(WordFreq **wfpp, char * buf, int * numWords) { 
    check_pointer((char *)wfpp);
    check_pointer((char *)buf);
    print_all_words(wfpp, *numWords);

    if ( *numWords == 0 ) {
        wfpp[0] = (WordFreq *) malloc(sizeof(WordFreq)); /* Make word strut for the first word */
        check_pointer((char *)wfpp[0]);
        wfpp[0]->word = (char *)malloc(strlen(buf) + 1);
        strcpy(wfpp[0]->word, buf);
        wfpp[0]->count = 1;

        *numWords = 1;
        print_all_words(wfpp, *numWords);
        return wfpp;
    }

    /* Checks if the word already exists in the list */
    int i;
    for(i = 0; i < *numWords; i++) {
        if (strcmp (buf, wfpp[i]->word) == 0 ) {
            /* If word is found, then increment the count */
            wfpp[i]->count++;
            print_all_words(wfpp, *numWords);

            return wfpp;
        }
    }

    /* If the word is not found add it to the list */
    wfpp = (WordFreq **)realloc(wfpp, sizeof(WordFreq *) * (*numWords + 1)); 
    wfpp[*numWords] = (WordFreq *)malloc(sizeof(WordFreq));
    /* Allocate memory for the word string in the new WordFreq struct for the new word */
    wfpp[*numWords]->word = (char *)malloc(strlen(buf) + 1);
    strcpy(wfpp[*numWords]->word, buf);
    wfpp[*numWords]->count = 1;

    (*numWords)++; /* Increment the number of unique words */
    print_all_words(wfpp, *numWords);

    return wfpp;
}

/* Allocates and builds a string buffer from file input and gets one word at a time from the file */
 int getWord(char * buffer, FILE *filePtr) {
    check_pointer((char *)buffer);
    check_pointer((char *)filePtr);
    char ch;
    /* char *buffer;*/
    int n;
    /* Initializing variables */
    n = 0; 
    buffer[0] = '\0';
    ch = fgetc(filePtr);

    while(ch != EOF) {
        /* If the character is in the alphabet, convert word into lowercase and add it to the buffer.
          Then, push the null terminator back one element in the list. */
        if(isalpha(ch)) {
            ch = tolower(ch);
            buffer[n] = ch;
            n++;
            buffer[n] = '\0';
            if(n >= 19){ /* Checks to make sure the string length is no more than 20 include \0 */
                return 1;
            }
        }
        /* If word is not in alpha and current buffer has chars return buffer */  
        else if(n > 0){
            /*return buffer;*/
            return 1;
        }
        /* Read the next character and continue */
        ch = fgetc(filePtr);
    }
    return 0;
}

int main(int argc, char* argv[]) {
    FILE * filePtr;
    int  numWords; /* number of unique word*/
    char buffer[MAX_STRING_SIZE]; /* fixed size buffer for processing characters */
    char * word; 
    int  result;

    /* Makes sure that there are three command line arguments.*/

    if(argc != 3){
        printf("Usage: %s <input filename> <output filename> \n", argv[0]);
        return 1;
    }

    char * inFileName = argv[1]; /*outfile name is the 3rd command that is inputted */
    char * outFileName = argv[2]; /*outfile name is the 3rd command that is inputted */
    printf("Input file : %s\n", argv[1]);  
    printf("Output file: %s\n", outFileName);
    /* checks to see if input file can be opened */
    filePtr = fopen(argv[1], "rie");

    if(filePtr == 0) {
        printf("InFile - Could not open %s \n", argv[1]);
        return 0;
    }

    /* SECTION 2 - Handle memory mgmt, process input file */
    WordFreq ** wfpp = (WordFreq **) malloc(sizeof(WordFreq *)); /* Allocate a list of WordFreq pointers */
    check_pointer((char *)wfpp);
    wfpp[0] = NULL;

    while ((getWord(buffer, filePtr)) > 0) {
     /*   strcpy(buffer, word);  Word being read in from file, copy that into the buffer */
        wfpp = addToList(wfpp, buffer, &numWords);
    }

    printf("Input file %s has %d unique words\n", inFileName, numWords);
    result = outputWords(outFileName, wfpp, numWords);

    if (result != 0) {
        fprintf(stderr, "Error writing to the output file.\n");
        return 1;

    }
    return 0;
}