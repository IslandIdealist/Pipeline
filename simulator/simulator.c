#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define NUMMEMORY 65536
#define NUMREGS 8

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDstruct{
	int instr;
	int pcplus1;
} IFIDType;

typedef struct IDEXstruct{
	int instr;
	int pcplus1;
	int readregA;
	int readregB;
	int offset;
} IDEXType;

typedef struct EXMEMstruct{
	int instr;
	int branchtarget;
	int aluresult;
	int readreg;
} EXMEMType;

typedef struct MEMWBstruct{
	int instr;
	int writedata;
} MEMWBType;

typedef struct WBENDstruct{
	int instr;
	int writedata;
} WBENDType;

typedef struct statestruct{
	int pc;
	int instrmem[NUMMEMORY];
	int datamem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles;    /* Number of cycles run so far */
	int fetched;   /* Total number of instructions fetched */
	int retired;   /* Total number of completed instructions */
	int branches;  /* Total number of branches executed */
	int mispreds;  /* Number of branch mispredictions*/
} statetype;

int  runInstrs( statetype* state );
void noopIFID(statetype *stateptr);
void noopIDEX(statetype *stateptr);
void noopEXMEM(statetype *stateptr);
void noopMEMWB(statetype *stateptr);
void noopWBEND(statetype *stateptr);
int field0(int instruction);
int field1(int instruction);
int field2(int instruction);
int opcode(int instruction);
void printinstruction(int instr);
void printstate(statetype *stateptr);

int main( int argc, char** argv ) {
// GETOPT
	int   opt;
	char* userFile;

	while ((opt = getopt( argc, argv, "i:" )) != -1) {
		switch (opt) {
			case 'i':
				userFile = strdup( optarg ); // <frd>
				break;
			default:
				printf( "ERROR: Invalid command arguments\n" );
				exit( EXIT_FAILURE );
		}
	}


// OPEN FILE AND SAVE INTO MEMORY
	FILE*      openFile = fopen( userFile, "r" );
	char*      curStr = malloc( 12 * sizeof(char) ); // <frd>
	statetype* state  = calloc( 1, sizeof(statetype) ); // <frd>

	free( userFile );

	if (openFile == NULL) {
		printf( "ERROR: Problem opening input file\n" );
		exit( EXIT_FAILURE );
	}

	while (fgets( curStr, 11, openFile ) != NULL) {
		state -> instrmem[state-> numMemory] = atoi( curStr );
		state -> datamem[state-> numMemory] = atoi( curStr );
		++(state -> numMemory);
	}

	free( curStr );
	fclose( openFile );


// PERFORM INSTRUCTIONS AND PRINT STATE
	int numInstrs = runInstrs( state );

	printstate( state );
	free( state );
	printf( "\nINSTRUCTIONS: %d\n", numInstrs );
	exit( EXIT_SUCCESS );
}


int runInstrs( statetype* state ) {
	int curInstr;
	int immediate;
	int regDest;
	int regB;
	int regA;
	int opcode = (state-> instrmem[state-> pc] >> 22) & 7;
	int numInstrs = 1;
	statetype* newstate = malloc( 1 * sizeof(statetype) ); // <frd>

	while(1){
		noopIFID( state );
		noopIDEX( state );
		noopEXMEM( state );
		noopMEMWB( state );
		noopWBEND( state );

		//printstate(&state);
		printstate( state );

		/* check for halt */
		//if(HALT == opcode(state-> MEMWB.instr)) {
		if(HALT == opcode) {
			printf("machine halted\n");
			printf("total of %d cycles executed\n", state-> cycles);
			printf("total of %d instructions fetched\n", state-> fetched);
			printf("total of %d instructions retired\n", state-> retired);
			printf("total of %d branches executed\n", state-> branches);
			printf("total of %d branch mispredictions\n", state-> mispreds);
			exit(0);
		}

		//newstate = state;
		memcpy( newstate, state, sizeof(statetype) );
		//newstate.cycles++;
		newstate-> cycles++;
		/*------------------ IF stage ----------------- */
		/*------------------ ID stage ----------------- */
		/*------------------ EX stage ----------------- */
		/*------------------ MEM stage ----------------- */
		/*------------------ WB stage ----------------- */
		//state = newstate;
		memcpy( state, newstate, sizeof(statetype) );
		/* this is the last statement before the end of the loop.
		It marks the end of the cycle and updates the current
		state with the values calculated in this cycle
		– AKA “Clock Tick”. */
	}

/*
	while (opcode != 6) { // continue while instructions isn't halt
		curInstr   = state-> instrmem[state-> pc];
		immediate  = curInstr & 0xffff; // 65535
		immediate -= (immediate & (1 << 15)) ? (1 << 16) : 0;
		regDest    = curInstr & 7;
		regB       = (curInstr >> 16) & 7;
		regA       = (curInstr >> 19) & 7;
		opcode     = (curInstr >> 22) & 7;

		printstate( state );
		state-> pc++;
		numInstrs += (opcode == 6) ? 0 : 1;

		if (opcode == 0) { // add
			if (regDest < 1 || regDest > 7 || regA < 0 || regA > 7 || regB < 0 || regB > 7) {
				printf( "ERROR: add was given an improper register\n" );
				exit( EXIT_FAILURE );
			}

			state-> reg[regDest] = state-> reg[regA] + state-> reg[regB];
		}
		else if (opcode == 1) { // nand
			if (regDest < 1 || regDest > 7 || regA < 0 || regA > 7 || regB < 0 || regB > 7) {
				printf( "ERROR: nand was given an improper register\n" );
				exit( EXIT_FAILURE );
			}

			state-> reg[regDest] = ~ (state-> reg[regA] & state-> reg[regB]);
		}
		else if (opcode == 2) { // lw
			int error = (regA < 1 || regA > 7)                       ? 1 : 0;
		      error = (regB < 0 || regB > 7)                       ? 1 : error;
					error = (state-> reg[regB] + immediate < 0)          ? 1 : error;
					error = (state-> reg[regB] + immediate >= NUMMEMORY) ? 1 : error;

			if (error) {
				printf( "ERROR: lw was given an improper reg or offset\n" );
				exit( EXIT_FAILURE );
			}

			state-> reg[regA] = state-> datamem[state-> reg[regB] + immediate];
		}
		else if (opcode == 3) { // sw
			int error = (regA < 1 || regA > 7)                       ? 1 : 0;
		      error = (regB < 0 || regB > 7)                       ? 1 : error;
					error = (state-> reg[regB] + immediate < 0)          ? 1 : error;
					error = (state-> reg[regB] + immediate >= NUMMEMORY) ? 1 : error;

			if (error) {
				printf( "ERROR: sw was given an improper reg or offset\n" );
				exit( EXIT_FAILURE );
			}

			state-> datamem[state-> reg[regB] + immediate] = state-> reg[regA];
		}
		else if (opcode == 4) { // beq
			int error = (regA < 0 || regA > 7) ? 1 : 0;
		      error = (regB < 0 || regB > 7) ? 1 : error;

			if (error || (state-> reg[regA] == state-> reg[regB] && (state-> pc + immediate < 0 || state-> pc + immediate >= NUMMEMORY))) {
				printf( "ERROR: beq was given an improper reg or offset\n" );
				exit( EXIT_FAILURE );
			}

			state-> pc += (state-> reg[regA] == state-> reg[regB]) ? immediate : 0;
		}
		else if (opcode == 5) { // jalr
			if (regA < 1 || regA > 7 || regB < 0 || regB > 7) {
				printf( "ERROR: jalr was given an improper register\n" );
				exit( EXIT_FAILURE );
			}

			state-> reg[regA] = state-> pc;
			state-> pc = state-> reg[regB];
		}
		else if (opcode == 7) { // noop
			// nothing
		}
		else if (opcode > 7) {
			printf( "ERROR: improper opcode was given\n" );
			exit( EXIT_FAILURE );
		}
	}
*/

	free( newstate );

	return numInstrs;
}


void noopIFID(statetype *stateptr){
	stateptr -> IFID.instr = NOOPINSTRUCTION;
}

void noopIDEX(statetype *stateptr){
	stateptr -> IDEX.instr = NOOPINSTRUCTION;
	stateptr -> IDEX.readregA = 0;
	stateptr -> IDEX.readregB = 0;
	stateptr -> IDEX.offset = 0;
}

void noopEXMEM(statetype *stateptr){
	stateptr -> EXMEM.instr = NOOPINSTRUCTION;
	stateptr -> EXMEM.branchtarget = 0;
	stateptr -> EXMEM.aluresult = 0;
	stateptr -> EXMEM.readreg = 0;
}

void noopMEMWB(statetype *stateptr){
	stateptr -> MEMWB.instr = NOOPINSTRUCTION;
	stateptr -> MEMWB.writedata = 0;
}

void noopWBEND(statetype *stateptr){
	stateptr -> WBEND.instr = NOOPINSTRUCTION;
	stateptr -> WBEND.writedata = 0;
}

int signextend(int field) {
	return (field & (1 << 15)) ? field - (1 << 16) : field;
}

int field0(int instruction){
	return( (instruction>>19) & 0x7);
}

int field1(int instruction){
	return( (instruction>>16) & 0x7);
}

int field2(int instruction){
	return(instruction & 0xFFFF);
}

int opcode(int instruction){
	return(instruction>>22);
}

void printinstruction(int instr) {
	char opcodestring[10];
	if (opcode(instr) == ADD) {
		strcpy(opcodestring, "add");
	} else if (opcode(instr) == NAND) {
		strcpy(opcodestring, "nand");
	} else if (opcode(instr) == LW) {
		strcpy(opcodestring, "lw");
	} else if (opcode(instr) == SW) {
		strcpy(opcodestring, "sw");
	} else if (opcode(instr) == BEQ) {
		strcpy(opcodestring, "beq");
	} else if (opcode(instr) == HALT) {
		strcpy(opcodestring, "halt");
	} else if (opcode(instr) == NOOP) {
		strcpy(opcodestring, "noop");
	} else {
		strcpy(opcodestring, "data");
	}

	if(opcode(instr) == ADD || opcode(instr) == NAND){
		printf("%s %d %d %d\n", opcodestring, field2(instr), field0(instr), field1(instr));
	}else if(0 == strcmp(opcodestring, "data")){
		printf("%s %d\n", opcodestring, signextend(field2(instr)));
	}else{
		printf("%s %d %d %d\n", opcodestring, field0(instr), field1(instr),
		signextend(field2(instr)));
	}
}

void printstate(statetype *stateptr){
	int i;

	printf("\n@@@\nstate before cycle %d starts\n", stateptr->cycles);
	printf("\tpc %d\n", stateptr->pc);

	printf("\tdata memory:\n");
	for (i=0; i<stateptr->numMemory; i++) {
    printf("\t\tdatamem[ %d ] %d\n", i, stateptr->datamem[i]);
	}

	printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
    printf("\t\treg[ %d ] %d\n", i, stateptr->reg[i]);
	}

	printf("\tIFID:\n");
	printf("\t\tinstruction ");
	printinstruction(stateptr->IFID.instr);
	printf("\t\tpcplus1 %d\n", stateptr->IFID.pcplus1);

	printf("\tIDEX:\n");
	printf("\t\tinstruction ");
	printinstruction(stateptr->IDEX.instr);
	printf("\t\tpcplus1 %d\n", stateptr->IDEX.pcplus1);
	printf("\t\treadregA %d\n", stateptr->IDEX.readregA);
	printf("\t\treadregB %d\n", stateptr->IDEX.readregB);
	printf("\t\toffset %d\n", stateptr->IDEX.offset);

	printf("\tEXMEM:\n");
	printf("\t\tinstruction ");
	printinstruction(stateptr->EXMEM.instr);
	printf("\t\tbranchtarget %d\n", stateptr->EXMEM.branchtarget);
	printf("\t\taluresult %d\n", stateptr->EXMEM.aluresult);
	printf("\t\treadreg %d\n", stateptr->EXMEM.readreg);

	printf("\tMEMWB:\n");
	printf("\t\tinstruction ");
	printinstruction(stateptr->MEMWB.instr);
	printf("\t\twritedata %d\n", stateptr->MEMWB.writedata);

	printf("\tWBEND:\n");
	printf("\t\tinstruction ");
	printinstruction(stateptr->WBEND.instr);
	printf("\t\twritedata %d\n", stateptr->WBEND.writedata);
}
