#include <sys/util.h>
#include <ti/screen.h>
#include <string.h>
#include <fileioc.h>
#include <sys/timers.h>
#include <keypadc.h>




/*

int* binToArray(char in) {
	int out[8];
	int count = 0;
	for (unsigned int mask = 0x80; mask != 0; mask >>= 1) {
		if(in & mask) {
			out[count] = 1;
		}else {
			out[count] = 0;
		}
		count++;
	}
}

*/




/*  - Variable Declarations -  */

uint8_t handle;
bool end = false;

// Memory
int16_t ram16k[16000];


// Instructions
int inst[16] = {1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,0}; // Current instruction, expressed as array of int
int16_t value = 1; // Current instruction, expressed as char


// Registers
int16_t aReg;
int16_t dReg;
int16_t PC;
int16_t aluOut;


// Flags
bool zeroF;
bool negF;






void jumpPC() {
	PC = aReg;
}


int16_t ramRead() {
	
	return 0;
}

void ramWrite() {
}


void alu() {
	int16_t x = dReg;
	int16_t y;
	int16_t out;
	// Select A register or Memory for Y
	if(inst[3] == 0) {
		y = aReg;
	} else {
		y = ramRead();
	}
	//printf("x: %i y: %i", x, y);
	//os_NewLine();
	// Zero out values	
	x = x*(inst[4] == 0);
	y = y*(inst[6] == 0);
	//printf("x: %i y: %i", x, y);
	//os_NewLine();
	//Invert values
	if (inst[5] == 1) {x = ~x;}
	if (inst[7] == 1) {y = ~y;}
	//printf("x: %i y: %i", x, y);
	//os_NewLine();
	// Add or bitwise AND
	if (inst[8] == 1) {
		out = x + y;
	} else {
		out = x&y;
	}
	// Invert output
	if (inst[9] == 1) {
		out = ~out;
	}
	// Set flags
	zeroF = (out == 0);
	negF = (out < 0);
	// Write to output
	aluOut = out;
	return;
}


void loadInst() {
	int count = ti_Read(&value, 2, 1, handle);
	if (count != 1) {
		value = 0;
		end = true;
	}
	for(int i = 0; i < 16; i++) {
		inst[15-i] = 1 & (value >> i);
	}	
}


void printInst() {
	for(int i = 0; i < 16; i++) {
		printf("%i", inst[i]);
	}
	os_NewLine();
}


void dest() {
	if(inst[10] == 1) {aReg = aluOut;}
	if(inst[11] == 1) {dReg = aluOut;}
	if(inst[12] == 1) {ramWrite();}
}


void processInst() {
	if (inst[0] == 0) {
		aReg = value;
		return;
	}
	
	alu();
	dest();
}


void run() {
	bool key, prevkey;
	//for(int i = 0; i < 10; i++) {
	while(true) {
		loadInst();
		if(end) {
			//break;
		}
		printInst();
		processInst();
		
		
		key = kb_Data[1] == kb_2nd;
		if (key && !prevkey) {
			end = true;
			break;
		}
		prevkey = key;
	}
}



int main(void)
{

	// Clear screen
	os_ClrHome();
	
	// Open file, creates handle object
	handle = ti_Open("TEST", "r");
	if(handle == 0) {	// 0 if error while reading file
		return 0;
	}
	
	run();
	
	printf("%i", dReg);
	os_NewLine();
	
	printf("%i", ti_GetSize(handle));
	os_NewLine();
	
	char* input = "";
	os_GetStringInput("exit", input, 0);

    return 0;
}
