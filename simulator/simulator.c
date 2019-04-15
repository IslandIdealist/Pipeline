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
#define JALR 5
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000
#define BLANKINSTRUCTION 0x0000000

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
	int isStall;
} statetype;

void runInstrs( statetype* state );
void noopIFID(statetype *stateptr);
void noopIDEX(statetype *stateptr);
void noopEXMEM(statetype *stateptr);
void noopMEMWB(statetype *stateptr);
void noopWBEND(statetype *stateptr);
int signextend(int field);
int field0(int instruction);
int field1(int instruction);
int field2(int instruction);
int opcode(int instruction);
void execute( statetype* state, statetype* newstate );
void memory( statetype* state, statetype* newstate );
void writeback( statetype* state, statetype* newstate );
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
	runInstrs( state );

	printstate( state );
	free( state );
	exit( EXIT_SUCCESS );
}


void runInstrs( statetype* state ) {
	statetype* newstate = malloc( 1 * sizeof(statetype) ); // <frd>

	noopIFID( state );
	noopIDEX( state );
	noopEXMEM( state );
	noopMEMWB( state );
	noopWBEND( state );

	while(1){
		printstate( state );

		/* check for halt */
		if(HALT == opcode(state-> MEMWB.instr)) {
			printf("machine halted\n");
			printf("total of %d cycles executed\n", state-> cycles);
			printf("total of %d instructions fetched\n", state-> fetched);
			printf("total of %d instructions retired\n", state-> retired);
			printf("total of %d branches executed\n", state-> branches);
			printf("total of %d branch mispredictions\n", state-> mispreds);
			exit( EXIT_FAILURE );
		}

		*newstate = *state;
		++(newstate-> cycles);
		newstate-> isStall = 0;

		/*------------------ IF stage ----------------- */
		newstate-> IFID.instr = state-> instrmem[state-> pc];
		newstate-> IFID.pcplus1 = (state-> pc) + 1;
		newstate-> fetched += (opcode( state-> instrmem[state-> pc] ) != BLANKINSTRUCTION) ? 1 : 0;

		/*------------------ ID stage ----------------- */
		newstate-> IDEX.instr = state-> IFID.instr;
		newstate-> IDEX.pcplus1 = state-> IFID.pcplus1;
		newstate-> IDEX.readregA = state-> reg[field0( state-> IFID.instr )];
		newstate-> IDEX.readregB = state-> reg[field1( state-> IFID.instr )];
		newstate-> IDEX.offset = signextend( field2( state-> IFID.instr ) );

		/*------------------ EX stage ----------------- */
		execute( state, newstate );

		/*------------------ MEM stage ----------------- */
		memory( state, newstate );

		/*------------------ WB stage ----------------- */
		writeback( state, newstate );

		*state = *newstate;
		/* this is the last statement before the end of the loop.
		It marks the end of the cycle and updates the current
		state with the values calculated in this cycle
		– AKA “Clock Tick”. */
	}

	free( newstate );
}


void execute( statetype* state, statetype* newstate ) {
	int curOp = opcode( state-> IDEX.instr );
	int regA = field0( state-> IDEX.instr );
	int regB = field1( state-> IDEX.instr );
	int immediate = state-> IDEX.offset;
	int regDest = state-> IDEX.instr & 7;
	int pc = state-> IDEX.pcplus1;
	int valA = state-> reg[regA];
	int valB = state-> reg[regB];

	newstate-> EXMEM.instr = state-> IDEX.instr;
	newstate-> EXMEM.branchtarget = pc + immediate;
	newstate-> EXMEM.readreg = state-> IDEX.readregA;

	if (curOp == ADD) {
		if (regDest < 1 || regDest > 7 || regA < 0 || regA > 7 || regB < 0 || regB > 7) {
			printf( "ERROR: add was given an improper register\n" );
			exit( EXIT_FAILURE );
		}

		// logic for forwarding data
		valA = (field0 (state-> WBEND.instr) == regA) ? state-> WBEND.writedata : valA;
		valA = (field0 (state-> MEMWB.instr) == regA) ? state-> MEMWB.writedata : valA;
		valB = (field0 (state-> WBEND.instr) == regB) ? state-> WBEND.writedata : valB;
		valB = (field0 (state-> MEMWB.instr) == regB) ? state-> MEMWB.writedata : valB;

		newstate-> EXMEM.aluresult = valA + valB;
	}
	else if (curOp == NAND) {
		if (regDest < 1 || regDest > 7 || regA < 0 || regA > 7 || regB < 0 || regB > 7) {
			printf( "ERROR: nand was given an improper register\n" );
			exit( EXIT_FAILURE );
		}

		// logic for forwarding data
		valA = (field0 (state-> WBEND.instr) == regA) ? state-> WBEND.writedata : valA;
		valA = (field0 (state-> MEMWB.instr) == regA) ? state-> MEMWB.writedata : valA;
		valB = (field0 (state-> WBEND.instr) == regB) ? state-> WBEND.writedata : valB;
		valB = (field0 (state-> MEMWB.instr) == regB) ? state-> MEMWB.writedata : valB;

		newstate-> EXMEM.aluresult = ~ (valA & valB);
	}
	else if (curOp == LW) {
		int error = (regA < 1 || regA > 7)                       ? 1 : 0;
				error = (regB < 0 || regB > 7)                       ? 1 : error;
				error = (state-> reg[regB] + immediate < 0)          ? 1 : error;
				error = (state-> reg[regB] + immediate >= NUMMEMORY) ? 1 : error;

		if (error) {
			printf( "ERROR: lw was given an improper reg or offset\n" );
			exit( EXIT_FAILURE );
		}

		// logic for forwarding data
		valB = (field0 (state-> WBEND.instr) == regB) ? state-> WBEND.writedata : valB;
		valB = (field0 (state-> MEMWB.instr) == regB) ? state-> MEMWB.writedata : valB;

		newstate-> EXMEM.aluresult = valB + immediate;

		// LW stall logic
		int nextInstr = opcode( state-> IFID.instr );

		if (nextInstr == SW || nextInstr == BEQ || nextInstr == ADD || nextInstr == NAND) {
			if (regA == field0( state-> IFID.instr ) || regA == field1( state-> IFID.instr )) {    // ??? ok to grab value from 2 buffers previous ???
				newstate-> IFID = state-> IFID;
				noopIDEX( newstate );
				newstate-> isStall = 1;
			}
		}
		else if (nextInstr == LW) {
			if (regA == field1( state-> IFID.instr )) {
				newstate-> IFID = state-> IFID;
				noopIDEX( newstate );
				newstate-> isStall = 1;
			}
		}
	}
	else if (curOp == SW) {
		int error = (regA < 1 || regA > 7)                       ? 1 : 0;
				error = (regB < 0 || regB > 7)                       ? 1 : error;
				error = (state-> reg[regB] + immediate < 0)          ? 1 : error;
				error = (state-> reg[regB] + immediate >= NUMMEMORY) ? 1 : error;

		if (error) {
			printf( "ERROR: sw was given an improper reg or offset\n" );
			exit( EXIT_FAILURE );
		}

		// logic for forwarding data
		valA = (field0 (state-> WBEND.instr) == regA) ? state-> WBEND.writedata : valA;
		valA = (field0 (state-> MEMWB.instr) == regA) ? state-> MEMWB.writedata : valA;
		valB = (field0 (state-> WBEND.instr) == regB) ? state-> WBEND.writedata : valB;
		valB = (field0 (state-> MEMWB.instr) == regB) ? state-> MEMWB.writedata : valB;

		newstate-> EXMEM.aluresult = valB + immediate;
	}
	else if (curOp == BEQ) {
		int error = (regA < 0 || regA > 7) ? 1 : 0;
				error = (regB < 0 || regB > 7) ? 1 : error;

		if (error || (state-> reg[regA] == state-> reg[regB] && (state-> pc + immediate < 0 || state-> pc + immediate >= NUMMEMORY))) {
			printf( "ERROR: beq was given an improper reg or offset\n" );
			exit( EXIT_FAILURE );
		}

		// logic for forwarding data
		valA = (field0 (state-> WBEND.instr) == regA) ? state-> WBEND.writedata : valA;
		valA = (field0 (state-> MEMWB.instr) == regA) ? state-> MEMWB.writedata : valA;
		valB = (field0 (state-> WBEND.instr) == regB) ? state-> WBEND.writedata : valB;
		valB = (field0 (state-> MEMWB.instr) == regB) ? state-> MEMWB.writedata : valB;

		++(newstate-> branches);
		newstate-> EXMEM.aluresult = (valA - valB);
	}
	else if (curOp == JALR) {
		if (regA < 1 || regA > 7 || regB < 0 || regB > 7) {
			printf( "ERROR: jalr was given an improper register\n" );
			exit( EXIT_FAILURE );
		}

		//newstate-> reg[regA] = state-> pc;
		//newstate-> pc = state-> reg[regB];
	}
	else if (curOp == NOOP) {
		// nothing
	}
	else if (curOp > 7) {
		printf( "ERROR: improper opcode was given\n" );
		exit( EXIT_FAILURE );
	}
}

void memory( statetype* state, statetype* newstate ) {
	newstate-> MEMWB.instr = state-> EXMEM.instr;

	// set writedata in next buffer
	if (ADD == opcode( state-> EXMEM.instr ) || NAND == opcode( state-> EXMEM.instr )) {
		newstate-> MEMWB.writedata = state-> EXMEM.aluresult;
	} else if (LW == opcode( state-> EXMEM.instr )) {
		newstate-> MEMWB.writedata = state-> datamem[state-> EXMEM.aluresult];
	} else {
		newstate-> MEMWB.writedata = 0; // ??? should this be here ???
	}

	// write to datamem
	if (SW == opcode( state-> EXMEM.instr )) {
		newstate-> datamem[state-> EXMEM.aluresult] = state-> EXMEM.readreg;
	}

	// set new pc to correct value
	if (BEQ == opcode( state-> EXMEM.instr ) && state-> EXMEM.aluresult == 0) {
		newstate-> pc = state-> EXMEM.branchtarget;
		noopIFID( newstate );
		noopIDEX( newstate );
		noopEXMEM( newstate );
	  ++(newstate-> mispreds);
	}
	else if (newstate-> isStall) {
		// pc remains the same
	}
	else {
		newstate-> pc = (state-> pc) + 1;
	}
}

void writeback( statetype* state, statetype* newstate ) {
	newstate-> WBEND.instr = state-> MEMWB.instr;
	newstate-> WBEND.writedata = state-> MEMWB.writedata;

	if (ADD == opcode( state-> MEMWB.instr ) || NAND == opcode( state-> MEMWB.instr )) {
		newstate-> reg[field2( state-> MEMWB.instr )] = state-> MEMWB.writedata;
	}
	if (LW == opcode( state-> MEMWB.instr )) {
		newstate-> reg[field0( state-> MEMWB.instr )] = state-> MEMWB.writedata;
	}

	if (state-> cycles >= 3) { // ??? are instrs only retired once in WBEND ???
		++(newstate-> retired);
	}
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

int field0(int instruction){ // regA
	return( (instruction>>19) & 0x7);
}

int field1(int instruction){ // regB
	return( (instruction>>16) & 0x7);
}

int field2(int instruction){ // immediate/offset value
	return(instruction & 0xFFFF);
}

int opcode(int instruction){ // opcode
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
	} else if (opcode(instr) == JALR) {
		strcpy(opcodestring, "jalr");
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
