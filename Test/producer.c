#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "wrappers.h"

int rnd(int min, int max) {
	return rand() % (max - min + 1) + min; //slight bias towards first k
}

int main(int argc, char **argv) {
	int pet_type;
	int num_pets;
	int start;
	int dest;
	
	srand(time(0));

	if (argc != 1) {
		printf("wrong number of args\n");
		return -1;
	}
		
	pet_type = rnd(1, 2);
	num_pets = rnd(0,3);
	start = rnd(1, 10);
	do {
		dest = rnd(1, 10);
	} while(dest == start);
	
	long ret = issue_request(num_pets, pet_type, start, dest);
	printf("Issue (%d, %d, %d, %d) returned %ld\n", num_pets, pet_type, start, dest, ret);

	return 0;
}
