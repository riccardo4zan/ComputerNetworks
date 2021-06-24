#include <stdio.h>

int main(){

	int number = 0x0A0B0C0D;

	unsigned char* pointer = (unsigned char*) &number;

	if(*pointer == 0x0A){
		printf("Big endian\n");
	} else {
		printf("Little endian\n");
	}

}
