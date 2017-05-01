#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int C, B, L, S, K; 					// Cache size, Block size, # of Lines, # of Sets, # of blocKs per set (1 if Direct-Mapped)
int tagBits, indexBits, offset;		
int cacheType; 						// cache type -- probably unnecessary, since we don't deal with ICaches here
FILE *traceFile;

typedef struct cacheLine{
	int vBit;						// valid bit, although honestly, why do we need this if we're already comparing tags?
	int LRUrank;					// LRU bit 
	char tag[33];					// tag in binary form
	int data;						// basically,  just the given address left in decimal form
}cacheLine;

void selectFile(){					//==============> Select trace file name
	int c;								
	printf("Select 0 for default trace file, or 1 to enter one in manually: ");
	scanf("%d", &c);

	if(c == 0){	
		traceFile = fopen("trace.txt", "r");
		printf("Opening default trace file (trace.txt)...\n\n");
	}

	else if(c == 1){
		printf("Please enter the filename: ");
		char name[50];
		scanf ("%s", name);	
		traceFile = fopen(name, "r");

		if(traceFile == NULL){
			perror("Error: ");
			exit(1);
		}

		else{
			printf("Filename %s opened...\n\n", &name);
		}
	}

	else{
		printf("Invalid input.\n\n");
		exit(0);
	}
}

void selectType(){					//================> Select cache type
	int c;
	printf("Please specify 0 for unified cache, 1 for separate caches: ");
	scanf("%d", &c);
	if(c == 0){	
		printf("Unified cache selected.\n\n");
	}

	else if(c == 1){	
		printf("Separate caches selected.\n\n");
	}

	else{
		printf("Invalid input.\n");
		exit(0);
	}
}


void getDimensions(){
	printf("Please specify total cache size (in bytes): ");
	if(scanf("%d", &C) == 1) {
	  	//printf("Total cache size of %d bytes selected.\n", C);
	}

	else {
  		printf("Invalid data entered.\n");
	}

	printf("Please specify block size (in bytes): ");
	if(scanf("%d", &B) == 1) {
	  	//printf("Block size of %d bytes selected.\n", B);
	}

	else {
  		printf("Invalid data entered.\n");
	}

	printf("Please specify # of blocks per set: ");
	if(scanf("%d", &K) == 1) {
	  	//printf("%d lines per set selected.\n", K);
	}

	else {
  		printf("Invalid data entered.\n");
	}

	printf("Total cache size of %d bytes selected.\n", C);
	printf("Block size of %d bytes selected.\n", B);
	printf("%d lines per set selected.\n", K);

	L = C/B;
	S = L/K;
	offset = log2(B); // taken from textbook; not specifically the block or byte offset
	indexBits = log2(S);
	tagBits = 32 - indexBits - offset;

	printf("\n# of blocks: %d\n", L);
	printf("# of sets: %d\n", S);
	printf("# of tag bits: %d\n", tagBits);
	printf("# of index bits: %d\n", indexBits);
	printf("# of bits for byte offset: %d\n\n", offset);
}

char *hexToBinary(char *ch){	
	char *hexDigitToBinary[16] = {"0000", "0001", "0010", "0011", 
							      "0100", "0101", "0110", "0111", 
							      "1000", "1001", "1010", "1011", 
							      "1100", "1101", "1110", "1111"}; 

    char hexDigits[16] = {'0', '1', '2', '3', '4', '5', '6', '7', 
    					  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
		
	char binaryNumber[33] = "";
	char *bN;

	bN = binaryNumber;
	int i, j;
	for(i = 0; i < 33; i++){  
		for(j = 0; j < 16; j++){
	        if(ch[i] == hexDigits[j]){
	            strcat(binaryNumber, hexDigitToBinary[j]);
	        }
	    }
	} 
	return (char *)bN;
}


void initCache(struct cacheLine cache[S][K]){
	int x, y;
  
	for(x = 0; x < S; x++){
    	for(y = 0; y < K; y++){
	       cache[x][y].LRUrank = 0;
	       cache[x][y].vBit = 0;
    	}
  	}
}

void readTrace(struct cacheLine cache[S][K]){
	int i, j, k, x, y;
	int counter = 0;
	int hit = 0;
	char hexStr[33];
	char totalB[33];
	char tagB[tagBits + 1];
	char indexB[indexBits + 1];
	char offsetB[offset + 1];
	char cacheArray[S][K * 2][33];
	int validArray[S][K * 2];
	int LRUarray[S][K * 2];

	for(i = 0; i < S; i++){
		for(j = 0; j < K*2; j+=2){
			validArray[i][j] = 0;
			LRUarray[i][j] = 0;
		}
	}

	memset(totalB, '\0', sizeof(totalB));
	memset(tagB, '\0', sizeof(tagB));
	memset(indexB, '\0', sizeof(indexB));
	memset(offsetB, '\0', sizeof(offsetB));
	memset(hexStr, '\0', sizeof(hexStr));
	memset(cacheArray, '\0', sizeof(cacheArray));
	
	while(1){	
		fscanf(traceFile, "%d %08x", &x, &y);
		if(feof(traceFile))
			break;
		counter++;
		sprintf(hexStr, "%08x", y);
		char *hS;
		hS = hexToBinary(hexStr);
		
		strncpy(tagB, hS, tagBits);
		strncpy(indexB, (hS + tagBits), indexBits);
		strncpy(offsetB, (hS + tagBits + indexBits), offset);
		strncpy(totalB, hS, 33);

		//printf("Address (Hexadecimal): 0x%08x; (Binary): %s", y, totalB);
		//printf(" Tag bits: %s; Index bits: %s; Offset: %s\n", tagB, indexB, offsetB);
		int index = strtol(indexB, 0, 2);
		for(i = 0; i < S; i++){
			if(i == index){
				/*
				int b = log2(K);									// OTHER ROUTINE: using log2(K) bits of tagBits to place tags in their fields
				char temp[b + 1];									// char array for log2(K) bits of tagBits
				strncpy(temp, (tagB + (tagBits - b)), b);			// put them in temp
				int c = strtol(temp, 0, 2);							// convert temp from binary to decimal
				//printf("tag: %s; tag #: %d; binary: %s; decimal: %d\n", tagB, a+1, temp, c);
				//printf("binary: %s; decimal: %d\n", temp, c);
				
				if (strcmp(validArray[i][c*2], "0") == 0){			// if valid bit is O
					strcpy(cacheArray[i][c*2], tagB);				// enter tag into corresponding block
					strcpy(cacheArray[i][(c*2) + 1], totalB);		// next one should be the "data"
					strcpy(validArray[i][c*2], "1");				// change valid bit to 1
				}*/

				int lowest = K;										// lowest = highest possible rank (HIGHEST = most recent)
				int highest = 0;
				int field = 0;										// field = lowest tag field
				int found = 0;									
				for(j = 0; j < K * 2; j+=2){						// LRU METHOD (I think): go through and find the lowest & highest ranking tag fields							
					if(strcmp(cacheArray[i][j], tagB) == 0 && validArray[i][j] == 1){
						hit++;
						found = 1;
						break;
					}
				}
				
				if(found == 0){
					for(j = 0; j < K * 2; j+=2){
						if(LRUarray[i][j] < lowest){
							lowest = LRUarray[i][j];
							field = j;
						}

						if(LRUarray[i][j] > highest){
							highest = LRUarray[i][j];
						}
					}

					if(highest == 0){
						LRUarray[i][field]++;
					}

					if(highest == K){
						LRUarray[i][field] = K;
						for(j = 0; j < K * 2; j+=2){
							if(j != field)
								LRUarray[i][j]--;
						}
					}

					else{
						LRUarray[i][field] = ++highest;
					}

					strcpy(cacheArray[i][field], tagB);					// enter tag into corresponding block
					strcpy(cacheArray[i][field + 1], totalB);			// next one should be the "data"
					validArray[i][field] = 1;							// change valid bit to 1
				}
			}
		}	
	}

	for(i = 0; i < S; i++){
		printf("Index: %03d \n", (i + 1));
		for(j = 0; j < K * 2; j+=2){
			printf("[Tag #%d - %28s; ", ((j/2)), cacheArray[i][j]);
			printf("Data: %33s; Valid: %1d; Rank: %1d]\n", cacheArray[i][j+1], validArray[i][j], LRUarray[i][j]);
		}
		printf("\n");
	}

	memset(totalB, '\0', sizeof(totalB));
	memset(tagB, '\0', sizeof(tagB));
	memset(indexB, '\0', sizeof(indexB));
	memset(offsetB, '\0', sizeof(offsetB));
	memset(hexStr, '\0', sizeof(hexStr));
	memset(cacheArray, '\0', sizeof(cacheArray));
	
	printf("# of hits: %d\n# of misses: %d\n", hit, counter);
	double hitRate = (double)hit * 100.00 / (double)counter;
	printf("Hit rate: %.2lf%%; Miss rate: %.2lf%%\n", hitRate, (100.00 - hitRate));
	fclose(traceFile);
}

int main(){
	selectFile();
	getDimensions();
	struct cacheLine cache[S][K];
	//initCache(cache);
	readTrace(cache);
	return(0);
}