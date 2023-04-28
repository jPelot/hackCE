#include <sys/util.h>
#include <ti/screen.h>
#include <string.h>
#include <fileioc.h>
#include <sys/timers.h>
#include <keypadc.h>



char jmp[8][4] = {"","JGT","JEQ","JGE","JLT","JNE","JLE","JMP"};


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
int16_t ram16k[16384];


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




void jump() {
	if(inst[13] == 1 && negF) {ti_Seek(aReg*2, SEEK_SET, handle);}
	if(inst[14] == 1 && zeroF) {ti_Seek(aReg*2, SEEK_SET, handle);}
	if(inst[13] == 1 && !negF && !zeroF) {ti_Seek(aReg*2, SEEK_SET, handle);}
}


int16_t ramRead() {
	if(aReg < 0) {
		return 0;
	}
	if(aReg < 16384) {
		return ram16k[aReg];
	}
	return 0;
}

void ramWrite() {
	if(aReg < 0) {
		return;
	}
	if(aReg < 16384) {
		ram16k[aReg] = aluOut;
	}
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
	printf(" ");
	if(inst[0] == 0) {
		printf("@%i", value);
		os_NewLine();
		return;
	}
	
	if(inst[10] == 1) { printf("A");}
	if(inst[11] == 1) { printf("D");}
	if(inst[12] == 1) { printf("M");}
	if(inst[10] + inst[11] + inst[12] > 0) {printf("=");}
	
	if(inst[13] + inst[14] + inst[15] > 0) {
		printf(";");
		printf("%s",jmp[value & 7]);
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
	jump();
}


void run() {
	bool key, prevkey;
	//for(int i = 0; i < 10; i++) {
	while(true) {
		loadInst();
		if(end) {
			break;
		}
		printInst();
		processInst();
		
		
		key = kb_Data[1] == kb_2nd;
		if (key && !prevkey) {
			break;
		}
		prevkey = key;
	}
}



int main(void)
{

	// Clear screen
	os_ClrHome();
	
	printf("loading TEST\n");
	// Open file, creates handle object
	handle = ti_Open("TEST", "r");
	if(handle == 0) {	// 0 if error while reading file
		return 0;
	}
	
	run();
	
	printf("A: %i D: %i", aReg, dReg);
	os_NewLine();
	
	char* input = "";
	os_GetStringInput("exit", input, 0);

    return 0;
}
