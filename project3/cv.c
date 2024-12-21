// Name: Thinh Nguyen
// NetID: ngt218
// Description: a program that establishes a producer-consumer pair of threads 
// 		that chasing each other in a circle buffer of 15 positions. 
// 		The producer reads each character from user input and write to the buffer 
// 		while the consumer read each position of the buffer. 
// 		Assured correctness of reading and writing order using mutex and condition variables

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_SIZE 15
#define MAX_INPUT_LENGTH 50

char input[MAX_INPUT_LENGTH+1];
char buffer[BUFFER_SIZE];
int finished = false; // flag for producer to tell consumer it finished
pthread_mutex_t mutex[BUFFER_SIZE]; // mutex for every position in the buffer
pthread_cond_t allowWrite[BUFFER_SIZE]; // flag to prevent producer overwriting un-consumed data
pthread_cond_t allowRead[BUFFER_SIZE]; // flag to prevent consumer consuming un-renewed data
pthread_mutex_t finishedMutex; // mutex for finished flag

void* produce(void *args) {
	for (int i = 0; (i<MAX_INPUT_LENGTH) && (input[i]!='\0'); i++) {
		int buffer_index = i % BUFFER_SIZE; // get index in a circle buffer
		// hold a position in the buffer for writing
		pthread_mutex_lock(&mutex[buffer_index]);
		while (buffer[buffer_index] != '\0') { // the position has not been consumed yet 
			// because consumer will set the position's value to null after consuming (an agreement)
			// --> wait for the consumer
			pthread_cond_wait(&allowWrite[buffer_index], &mutex[buffer_index]);
		}
		buffer[buffer_index] = input[i];
		printf("Produced: %c\n", input[i]); // Successfully produced a value
		pthread_mutex_unlock(&mutex[buffer_index]);
		// Signal the consumer just in case it's waiting for producer at this position
		pthread_cond_signal(&allowRead[buffer_index]); 
	}
	// When finish producing, set finished to true
	pthread_mutex_lock(&finishedMutex); // lock just in case the consumer is also checking this
	finished = true;
	printf("Producer: done\n");
	pthread_mutex_unlock(&finishedMutex);
	return NULL;
}

void* consume(void *args) {
	for (int i = 0; ;i++) {
		int buffer_index = i % BUFFER_SIZE; // get index in a circle buffer
		pthread_mutex_lock(&mutex[buffer_index]); // hold a position in the buffer for reading
		while (buffer[buffer_index] == '\0') { // the position hasn't been produced yet
			// There are 2 scenarios: (1) consumer is faster than producer, (2) producer has already finished
			// Check if the producer had finished yet
			pthread_mutex_lock(&finishedMutex);
			if (finished) { // producer has finished
				printf("Consumer: done\n");
				finished = false; // set finished to false for next input
				// unlock all locking mutex before leaving
				pthread_mutex_unlock(&finishedMutex);
				pthread_mutex_unlock(&mutex[buffer_index]);
				return NULL;
				}
				pthread_mutex_unlock(&finishedMutex);
			// Confirmed that Producer is still working --> keep waiting
			pthread_cond_wait(&allowRead[buffer_index], &mutex[buffer_index]);
		}
		printf("Consumed: %c\n", buffer[buffer_index]); // successfully consumed a value
		buffer[buffer_index] = '\0'; // set postition to null after consuming (an agreement)
		pthread_mutex_unlock(&mutex[buffer_index]);
		// Signal the producer just in case it's waiting for consumer at this position
		pthread_cond_signal(&allowWrite[buffer_index]);
	}
}

int main() {
	char input_local[MAX_INPUT_LENGTH+1];
	// Initialize all mutexes and condition variables
	for (int i = 0; i < BUFFER_SIZE; i++) {
		pthread_mutex_init(&mutex[i], NULL);
		pthread_cond_init(&allowWrite[i], NULL);
		pthread_cond_init(&allowRead[i], NULL);
	} 
	pthread_mutex_init(&finishedMutex, NULL);
	
	while (1) {
		printf("Enter input (type 'exit' to quit): ");
		for (int i = 0; i <= MAX_INPUT_LENGTH; i++) {
			char c = getchar();
			if (i < MAX_INPUT_LENGTH && c != '\n') { // c == \n indicates the end of the current input
				input_local[i] = c;
			} else { 
				input_local[i] = '\0';
				if (c != '\n') { 
					// c != n when i reached MAX_INPUT_LENGTH indicates an input longer than MAX_INPUT_LENGTH
					// --> discard the exceeded part by traversing to the nearest newline character
					while(getchar() != '\n'); 
				}
				break;
			}
		}

		if (strcmp(input_local, "exit") == 0) {
			break; // terminate when user input "exit"
		}
	
		printf("Input: %s\n", input_local);
		printf("Count: %d characters\n", (int)strlen(input_local));
		strcpy(input, input_local); // copy the recorded user's input to the global variable

		for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = '\0'; // reset buffer

		// create and join producer and consumer threads
		pthread_t consumer, producer;
		if (pthread_create(&producer, NULL, &produce, NULL) != 0) printf("Error creating producer");
		if (pthread_create(&consumer, NULL, &consume, NULL) != 0) printf("Error creating consumer");
		if (pthread_join(producer, NULL) != 0) printf("Error joining producer");
		if (pthread_join(consumer, NULL) != 0) printf("Error joining consumer");
	}
	
	// destroy pthread objects before exiting
	for (int i = 0; i < BUFFER_SIZE; i++) {
		pthread_mutex_destroy(&mutex[i]);
		pthread_cond_destroy(&allowWrite[i]);
		pthread_cond_destroy(&allowRead[i]);
	}
	pthread_mutex_destroy(&finishedMutex);
	
	printf("Parent: done\n");
	return 0;
}