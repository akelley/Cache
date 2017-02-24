#include<stdio.h>

int main(){
	int x, y;
	FILE *fp;

	fp = fopen("trace.txt", "r");

	while(!feof(fp)){
		fscanf(fp, "%d %x", &x, &y);
		printf("Hexadecimal address; 0x%x\n", y);
	}

	fclose(fp);
}