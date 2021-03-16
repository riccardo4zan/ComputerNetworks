/**
 * Program that converts an unsigned short int from little endian to big endian
 */

#include <stdio.h>

/**
 * This function exhanges the first two bytes of the 'in' param.
 * This function does not modify the given pointers, but it modifies
 * the memory area refered by the given pointers.
 * param in: a pointer to a memory area containing the two bytes to exchange
 * param out: a pointer to a memory area containing the first tho bytes of 'in' exchanged
 */ 
void exchange(char* in, char* out){
	//Copy given pointers
	char* cin = in;
	char* cout = out;

	//Second byte of 'in' copy in first byte of 'out'
	cin++;
	*cout = *cin;
	//First byte of 'in' copy in second byte of 'out'
	cin--;
	cout++;
	*cout = *cin;
}

/**
 * This function prints out two bytes (in exadecimal value) of the memory area refered by the param 'pointer'.
 * This function does not modify the given pointer.
 * param pointer: a pointer to a memory area
 */
void print_two_bytes(char* pointer){

	char* cpointer = pointer;

	//Printing out result
	printf("%X\n", *cpointer);
	cpointer++;
	printf("%X\n", *cpointer);
}

int main(){
	//One byte of memory stores two exadecimal values
	unsigned short int myvalue = 0x0A0B;

	char* pointer = (char*) &myvalue;

	printf("Input:\n");
	print_two_bytes(pointer);
	
	unsigned short int myresult;

	char* res_pointer = (char*) &myresult;

	exchange(pointer, res_pointer);

	printf("Output:\n");
	print_two_bytes(res_pointer);
	
}
