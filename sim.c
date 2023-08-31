// Libraries and DEFINEs //

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE 
#define MAX_LINE_SIZE				500			// max size of each line as defined in the project
#define MAX_LINE_NUM				512			// max lines number as defined in the project
#define WORD_SIZE					32			// each word size as defined in the project
#define REGISTER_NUM				16			// number of registers as defined in the project
#define MAX_POSITIVE_IMM_VALUE		2047		// maximun positive value when using 3 hex digits
#define MAX_POSITIVE_VALUE_32_BITS	2147483647  // maximun positive value when using 8 hex digits


#include <stdlib.h>												
#include <stdio.h>												
#include <string.h>	
#include <math.h>
#include <stdbool.h>
#include <ctype.h>


// STRUCTS //

typedef struct instruction {
	int opcode;
	int rd;
	int rs;
	int rt;
	int imm;
} Inst;

typedef struct files {
	FILE* memin;
	FILE* memout;
	FILE* regout;
	FILE* trace;
	FILE* cycles;
} Files;



/////  Declaration of functions  /////


//  Help functions  //

// Converting line to an instruction
Inst LineToInst(char line[]);

// checking if the 32-bit number is negative //
int check_if_neg_32_bits(int num);

// Power
int power(int base, int exp);

// Converting hexadecimal to decimal
int hex2dec(char hex[]);

// Checking files openings
int check_files(Files files);

// Sign extension in case the imm is a negative number
int check_if_neg(Inst* inst);



//  Load functions  //

// Fill initial memory
void load_memin(FILE* memin, Inst* instructions_array, int* mem);



//  Printing functions  //

// Memory content at the end of the run
void print_memout(FILE* memout, int* mem);

// Registers contents at the end of the run
void print_regout(FILE* regout, int* R);

// trace
void print_trace(FILE* trace, int* R, int pc, Inst inst);

// Total cycles num at end of the simulatoion
void print_cycles(FILE* cycles, int cycles_num);



//  Execute functions  //

void execute_instructions(Files files, Inst* instructions_array, int* R, int* mem, unsigned int* clk);

int single_instruction_execute(Files files, Inst* inst, int* R, int next_pc,
	unsigned int* clk, int* mem);



///// MAIN /////

int main(int argc, char* argv[]) {

	// Declaring variables //

	int R[REGISTER_NUM] = { 0 };
	unsigned int CLK = 0;
	unsigned int* clk = &CLK;

	int* mem = (int*)calloc(MAX_LINE_NUM, sizeof(mem));
	if (mem == NULL) {
		printf("Memory allocation failure. \n");
		exit(1);
	}

	Inst instructions_array[MAX_LINE_NUM];              // array to store the instrucions

	Files files = { NULL, NULL, NULL, NULL, NULL };    // initializing all files to NULL


	// opening files for read and for write //

	files.memin = fopen(argv[1], "r");
	files.memout = fopen(argv[2], "w");
	files.regout = fopen(argv[3], "w");
	files.trace = fopen(argv[4], "w");
	files.cycles = fopen(argv[5], "w");


	// checking that files had been opened //

	if (check_files(files)) {
		printf("One file or more could not be opened \n");
		return 0;
	}

	// Loading input file //

	load_memin(files.memin, instructions_array, mem);

	//execute instructions //

	execute_instructions(files, instructions_array, R, mem, clk);


	// Printing out files //

	print_memout(files.memout, mem);  // Memory in the end of the simulation

	print_regout(files.regout, R);	  // Registers' content in the end of the simulation

	print_cycles(files.cycles, *clk);  // How many cycles it took to run the simulation


	//  Closing files and freeing allocations //

	free(mem);
	fclose(files.memin);
	fclose(files.memout);
	fclose(files.regout);
	fclose(files.trace);
	fclose(files.cycles);
	printf("SIMULATOR FINISHED\n");
	return 0;
}


///// Help functions /////

int hex2dec(char hex_line[]) {
	int dec = strtoll(hex_line, NULL, 16);

	if (hex_line[0] >= '8' && hex_line[0] <= 'f') {		// Negetive number check
		dec -= power(2, WORD_SIZE);						// Using 2's comp formula for negative numbers: <A>_2's = <A> - 2^n where n is the number of bits 
	}

	return dec;
}


Inst LineToInst(char line[]) {
	Inst instruction;

	// initializing each member of the instruction as a string //

	char op[3] = { line[0] , line[1] , '\0' };
	char imm[4] = { line[5] , line[6] , line[7] , '\0' };
	char rd[2] = { line[2],'\0' };
	char rs[2] = { line[3],'\0' };
	char rt[2] = { line[4],'\0' };

	// converting each member to an integer and updating the instruction //

	instruction.opcode = (int)strtoul(op, NULL, 16);
	instruction.rd = (int)strtoul(rd, NULL, 16);
	instruction.rs = (int)strtoul(rs, NULL, 16);
	instruction.rt = (int)strtoul(rt, NULL, 16);;
	instruction.imm = (int)strtoul(imm, NULL, 16);

	return instruction;
}

int check_if_neg_32_bits(int num) {
	int dec = num;
	if (num > MAX_POSITIVE_VALUE_32_BITS) {
		dec -= power(2, WORD_SIZE);
	}
	return dec;
}


int check_if_neg(Inst* inst) {
	int extended = 0;
	if (inst->imm > MAX_POSITIVE_IMM_VALUE) {
		extended = inst->imm & 0x00000FFF;
		extended = inst->imm | 0xFFFFF000;
	}

	else {
		extended = inst->imm;
	}

	return extended;
}

int power(int base, int exp) {
	if (exp == 0)
		return 1;
	return base * power(base, exp - 1);
}

int check_files(Files files) {
	return(
		files.memin == NULL ||
		files.memout == NULL ||
		files.regout == NULL ||
		files.trace == NULL ||
		files.cycles == NULL);
}


//  Functions for loading data  //


void load_memin(FILE* memin, Inst* instructions_array, int* mem) {
	int inst_num = 0;
	int line_num = 0;
	char line[WORD_SIZE];
	while (fgets(line, WORD_SIZE, memin) != NULL && line_num < MAX_LINE_NUM) {
		Inst new_inst = LineToInst(line);
		int line_len = strlen(line);
		line[line_len - 1] = '\0';
		int line_dec = hex2dec(line);
		mem[line_num] = line_dec;      // storing the instruction in the memory- to be printed in memout
		line_num++;

		*(instructions_array + inst_num) = new_inst;
		inst_num++;
	}
}

// printing functions //

void print_memout(FILE* memout, int* mem) {
	int i = 0;
	int last_full_row = 0;
	while (i < MAX_LINE_NUM) {
		if (mem[i] != 0) {
			last_full_row = i;
		}
		i++;
	}
	i = 0;
	while (i <= last_full_row) {
		fprintf(memout, "%08X\n", (unsigned int)mem[i] & 0xffffffff);
		i++;
	}
}

void print_regout(FILE* regout, int* R) {
	int i = 2;
	while (i < 16) {
		fprintf(regout, "%08X\n", R[i]);
		i++;
	}
}


void print_trace(FILE* trace, int* R, int pc, Inst inst) {
	fprintf(trace, "%08X %02X%X%X%X%03X ", pc, inst.opcode, inst.rd, inst.rs, inst.rt, inst.imm);
	fprintf(trace, "%08X %08X %08X %08X ", R[0], R[1], R[2], R[3]);
	fprintf(trace, "%08X %08X %08X %08X ", R[4], R[5], R[6], R[7]);
	fprintf(trace, "%08X %08X %08X %08X ", R[8], R[9], R[10], R[11]);
	fprintf(trace, "%08X %08X %08X %08X\n", R[12], R[13], R[14], R[15]);
}


void print_cycles(FILE* cycles, int cycles_num) {
	fprintf(cycles, "%d", cycles_num);
}



/////  Execute functions  /////


int single_instruction_execute(Files files, Inst* inst, int* R, int next_pc,
	unsigned int* clk, int* mem) {

	R[1] = check_if_neg(inst);    // sign extension in case the imm is a negative number 

	print_trace(files.trace, R, next_pc, *inst);


	switch (inst->opcode) {
	case 0: { //add
		if (inst->rd > 1)
			R[inst->rd] = R[inst->rs] + R[inst->rt];
		else {
			next_pc = -1;
			printf("Cannot write to $zero / $imm register.\n");
		}
		break;
	}
	case 1: { //sub
		if (inst->rd > 1)
			R[inst->rd] = R[inst->rs] - R[inst->rt];
		else {
			next_pc = -1;
			printf("Cannot write to $zero / $imm register.\n");
		}
		break;
	}
	case 2: { //and
		if (inst->rd > 1)
			R[inst->rd] = R[inst->rs] & R[inst->rt];
		else {
			next_pc = -1;
			printf("Cannot write to $zero / $imm register.\n");
		}
		break;
	}
	case 3: { //or
		if (inst->rd > 1)
			R[inst->rd] = R[inst->rs] | R[inst->rt];
		else {
			next_pc = -1;
			printf("Cannot write to $zero / $imm register.\n");
		}
		break;
	}
	case 4: { //sll
		if (inst->rd > 1)
			R[inst->rd] = R[inst->rs] << R[inst->rt];
		else {
			next_pc = -1;
			printf("Cannot write to $zero / $imm register.\n");
		}
		break;
	}
	case 5: { //sra
		if (inst->rd > 1)
			R[inst->rd] = R[inst->rs] >> R[inst->rt];
		else {
			next_pc = -1;
			printf("Cannot write to $zero / $imm register.\n");
		}
		break;
	}
	case 6: { //srl
		if (inst->rd > 1) {
			if (R[inst->rs] == 0)
				R[inst->rd] = R[inst->rs];
			else
				R[inst->rd] = ((int)R[inst->rs] >> R[inst->rt]);
		}
		else {
			next_pc = -1;
			printf("Cannot write to $zero / $imm register.\n");
		}
		break;
	}
	case 7: { //beq
		if (R[inst->rs] == R[inst->rt])
			next_pc = R[inst->rd] - 1;
		break;
	}
	case 8: { //bne
		if (R[inst->rs] != R[inst->rt])
			next_pc = R[inst->rd] - 1;
		break;
	}
	case 9: { //blt
		if (R[inst->rs] < R[inst->rt])
			next_pc = R[inst->rd] - 1;
		break;
	}
	case 10: { //bgt
		if (R[inst->rs] > R[inst->rt])
			next_pc = R[inst->rd] - 1;
		break;
	}
	case 11: { //ble
		if (R[inst->rs] <= R[inst->rt])
			next_pc = R[inst->rd] - 1;
		break;
	}
	case 12: { //bge
		if (R[inst->rs] >= R[inst->rt])
			next_pc = R[inst->rd] - 1;
		break;
	}
	case 13: { //jal
		R[15] = next_pc + 1;
		next_pc = R[inst->rd] - 1;
		break;
	}
	case 14: { //lw
		if (inst->rd > 1) {
			R[inst->rd] = check_if_neg_32_bits(mem[R[inst->rs] + R[inst->rt]]);
		}
		else {
			next_pc = -1;
			printf("Cannot write to $zero / $imm register.\n");
		}
		break;
	}
	case 15: { //sw
		mem[R[inst->rs] + R[inst->rt]] = check_if_neg_32_bits(R[inst->rd]);
		break;
	}
	case 19: { //halt
		next_pc = -1;
	}
	}

	return next_pc;

}

void execute_instructions(Files files, Inst* instructions_array, int* R, int* mem, unsigned int* clk) {
	int next_pc = 0;
	int exit = 0;

	while (exit == 0) {
		next_pc = single_instruction_execute(files, instructions_array + next_pc, R, next_pc,
			clk, mem);

		if (next_pc == -1) {        // halt instruction
			exit = 1;
		}

		next_pc += 1;				// increase pc by 1
		*clk += 1;					// increase clk by 1
	}

}