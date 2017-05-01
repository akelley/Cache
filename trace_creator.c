#include <stdio.h>
#include <stdlib.h>

int generateHexNumber(){
	int number = rand() & 0xff;
	number |= (rand() & 0xff) << 8;
	number |= (rand() & 0xff) << 16;
	number |= (rand() & 0xff) << 24;
	return number;
}

int main(){
	int size = 10000;
	int traceType;
	int lines = 0;
	char var;

	FILE *traceFile;
	traceFile = fopen("trace.txt", "w");
	
	printf("Press 0 for a random trace; Press 1 for a sequential trace: ");
	scanf("%d", &traceType);

	if(traceType == 0){
		while(lines != size){
			int address = generateHexNumber();
			fprintf(traceFile, "0 %08x\n", address);
			lines++;
		}
	}

	else{
		int address = 0;

		while(lines != size){	
			fprintf(traceFile, "0 %08x\n", address);
			address += 4;
			lines++;
		}
	}

	printf("Trace file population complete.\n");
	printf("Program exiting...\n");
	fclose(traceFile);
	exit(0);
}