#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH 500
#define MAX_LEXI_LEVELS 3


#define machinecode "mcode.txt"


/**
 * Guide to return values:
 * 0:		Standard return from int main(), should happen after it hits a halt
 * 1:		Input lexicographical number is larger than 2 bits, invalid input
 * 2:		Something went wrong in the function scanInCode
 * 3:		Something went wrong in the funciton writeStackTrace
 */


 /**
  * @author	Kurtis Doobay
  * Author's notes:
  * I feel like I could do this better, but I'm satistfied how it came out.
  * I wanted to use this assignment to try out doing different files, like
  * writing all the stack logic in stack.c/stack.h, but I ultimately didn't
  * have enough time after finishing, and writing programs more than once
  * is a bad habit of mine, and it enables my lack of planning.
  */

void executeCycle();

struct instruction{
	int OP;	// The operation code, used to fetch the operation function
	int L;	// The lexicographical level
	int M;	// A number, program address, data address, or operation for the OPR op code
};


char OPCodeToInstruction[12][4]  = {"---\0","LIT\0","OPR\0","LOD\0","STO\0","CAL\0","INC\0","JMP\0","JPC\0","SIO\0","SIO\0","SIO\0"};


int PC;
int BP;
int SP;
struct instruction IR;
int lastPC; //Absolutely useless piece of information, but I need to include for output purposes
struct instruction code[MAX_CODE_LENGTH];
int stack[MAX_STACK_HEIGHT];

bool shouldLoop = true; // This is the halt flag


/**
 * Initializes variables for use
 * Should only run once before the run loop
 */
void initVm(){

	PC = 0; // program counter, or instruction pointer, used to fetch data from code array
	BP = 1; // points to the base of the activation record in the stack, used to jump after return
	SP = 0; // points to the top of the stack, NOTE: no need to clear stack information after the SP

	lastPC = -1; // The instruction we're on right now

	IR.OP = 0; // The instruction register, literally the code about to be executed
	IR.L = 0;
	IR.M = 0;

	memset(stack, 0, MAX_STACK_HEIGHT * sizeof(int));
	memset(code, 0, MAX_CODE_LENGTH * sizeof(struct instruction));
}

/**
 * Basic constructor for instruction
 * @param	OP	The OPCode for the instruction
 * @param	L	The lexicographical level
 * @param	M	
 * @return		Formatted struct with all the values
 */
struct instruction newInstruction(int op, int l, int m){
	struct instruction i;
	i.OP = op;
	i.L = l;
	i.M = m;
	return i;
}

/**
 * Simulates the fetch cycle of the P-Machine
 * Gets the instruction from the code array and increments PC
 */
void fetchCycle(){
	IR = code[PC];
	lastPC = PC;
	PC++;
}

/**
 * Gets the data the next level down in the stack, miraculously makes everything work
 * @param	L	The number of levels down, a 2 bit number, refers to lexicographical number
 * @return		The base pointer L levels down
 */
int base(int L){

	int bp = BP;
	while (L > 0){
		bp = stack[bp+1]; // the position of the previous base pointer
		L--;
	}

	return bp;
}


/**
 * Gets stacktrace.txt file pointer to pass around in main
 * @return	The FILE pointer to stacktrace.txt
 */
FILE* stackTraceFile(){
	FILE *fp;
	char filename[15] = "stacktrace.txt\0";
	//printf("Attempting to open %s...\n", filename);	// DEBUG
	fp = fopen(filename, "w");

	if (fp == NULL){
	//	printf("Something went wrong! Aborting...\n");	// DEBUG
		exit(3);
	}
	//else { 									// DEBUG
	//	printf("Opened %s!\n\n", filename);	// DEBUG
	//}										// DEBUG

	return fp;
}

void writeInstruction(FILE *fp){

	fprintf(fp, "Line\tOP\tL\tM\n");

	int n = 0; // this is our counter for printing out the code
	while(code[n].OP != 0){ // while the code we're reading is valid
		//printf("Writing instruction %d\t%s\t%d\t%d\n", n, OPCodeToInstruction[code[n].OP], code[n].L, code[n].M); // DEBUG
		fprintf(fp, "%d\t\t%s\t%d\t%d\n", n, OPCodeToInstruction[code[n].OP], code[n].L, code[n].M);
		n++;
	}
	fprintf(fp, "\n");
}

/**
 * Formats the stack with the '|' character at points in the stack for calls
 * @return	The pointer to an array with the formatted stack to output
 */
char* getFormattedStack(int pointer, int basepointer){

	char *thisStack = malloc(sizeof(char) * MAX_STACK_HEIGHT);
	int i = 0; // Used for getting data from stack
	int n = 0; // Used for formatting stack

	if (basepointer < 1){ // in the case of broken programs
		thisStack[n] = '\0';
		return thisStack;
		shouldLoop = false;
	}
	
	if(basepointer != 1){ // in the case where the is a call, we need to format the break line in
		char *lastStack = malloc(sizeof(char) * MAX_STACK_HEIGHT);

		lastStack = getFormattedStack(basepointer - 1, stack[basepointer + 1]); // we recursively call everything previous
		while (lastStack[n] != '\0'){ // and add it to our current formatted stack
			thisStack[n] = lastStack[n];
			n++;
		}
		free(lastStack);
		thisStack[n] = '|'; // and after previous information is added, we add a break line to signify the call
		n++;
	}

	for (i = basepointer - 1; i < pointer; i++){ // The information after (basepointer - 1) to pointer is now added, the current call
			int stackNumber = stack[i];
			if (stackNumber > 10){ // a simple check to get proper numbers
				int digits = log10(stackNumber); // log of a number less than 1 is negative, and that ruins everything. Best to only do the loop to things that need it
				while (digits >= 0){ // This loop properly stores the digits in the char array, considering multiple digit numbers
					thisStack[n + digits] = stackNumber % 10 + '0';
					stackNumber /= 10;
					digits--;
				}

				n += log10(stack[i]) + 1; // making sure we increment the amount of spaces we moved
			} else { // in the case of a single digit
				thisStack[n] = stackNumber + '0';
				n++;
			}


		thisStack[n] = ' '; // adding a space for easy-on-the-eyes formatting
		n++;
	}
	thisStack[n] = '\0'; // and the signifier to stop reading

	return thisStack;
}

/**
 * Does some pre-formatting for the output file for the VM
 */
void PreVMState(FILE *fp){
	//printf("Attempting to write to stacktrace.txt...\n"); // DEBUG
	fprintf(fp, "\t\t\t\tPC\tBP\tSP\tStack\n");

	//char formattedStack[MAX_STACK_HEIGHT];
	//strcpy(formattedStack, getFormattedStack(SP,BP));

	fprintf(fp, "Initial Values\t%d\t%d\t%d\t%s\n", PC, BP, SP, getFormattedStack(SP,BP));
	//printf("Wrote to stacktrace.txt!\n"); // DEBUG
}

/**
 * Writes the current VM state to the file, called in the main loop
 */
void printVMState(FILE *fp){
	//char formattedStack[MAX_STACK_HEIGHT];
	//strcpy(formattedStack, getFormattedStack(SP,BP));

	// printf("Attempting to print to file...\n");
	// fprintf(fp, "%d\t", lastPC);
	// fprintf(fp, "%s\t", OPCodeToInstruction[IR.OP]);
	// fprintf(fp, "%d\t", IR.L);
	// fprintf(fp, "%d\t", IR.M);
	// fprintf(fp, "%d\t", PC);
	// fprintf(fp, "%d\t", BP);
	// fprintf(fp, "%d\t", SP);
	// printf("Printing stack...\n");
	// fprintf(fp, "%s\n", getFormattedStack(SP,BP));

	fprintf(fp, "%d\t%s\t%d\t%d\t%d\t%d\t%d\t%s\n", lastPC, OPCodeToInstruction[IR.OP], IR.L, IR.M, PC, BP, SP, getFormattedStack(SP,BP));
}

/**
 * Scans in mcode.txt and stores instructions in code[]
 * Note: the last instruction will repeat itself if there's a running line. Shouldn't matter if it's a halt.
 */
void scanInCode(){ // Code to get machine code from file
	FILE *fp;
	//printf("Attempting to open %s...\n", filename); // DEBUG
	fp = fopen(machinecode, "r");

	if (fp == NULL){
	//	printf("Something went wrong! Aborting...\n"); // DEBUG
		exit(2);
	}
	//else {									// DEBUG
	//	printf("Opened %s!\n\n", filename);	// DEBUG
	//}										// DEBUG

	//printf("Beginning to populate code from file...\n"); // DEBUG

	int n = 0;

	int dataOP = -1; // Initializing variables to scan in
	int dataL = -1;
	int dataM = -1;

	while (!feof(fp)){
		fscanf(fp, "%d", &dataOP);
		fscanf(fp, "%d", &dataL);
		fscanf(fp, "%d", &dataM);


		// EXPERIMENTAL CODE HERE
		if (dataOP == -1 || dataL == -1 || dataM == -1){
			break;
		}
		// EXPERIMENTAL CODE ENDS HERE

		code[n] = newInstruction(dataOP, dataL, dataM);
		n++;

		dataOP = -1; // Initializing variables to scan in
		dataL = -1;
		dataM = -1;
	}

	fclose(fp);

	//printf("Finished populating code from file!\n\n"); // DEBUG
}

/**
 * This is where the magic happens!
 * Does the run loop after initilizing some variables
 */

 /*
int main(){
	FILE *fp = stackTraceFile();
	initVm();
	scanInCode();
	writeInstruction(fp);

	PreVMState(fp);
	
	while(shouldLoop){
		fetchCycle();
		executeCycle();
		printVMState(fp);
	}
	fclose(fp);
	return 0;
}
*/

/**
 * Reads the instruction in the IR does the instruction in the VM
 * Details of the expectation of each instruction is below
 */
void executeCycle(){
	switch(IR.OP){
		/**
		 * Our 22 cases are as follows:
		 *
		 * 01: LIT, Push M onto the stack
		 * 02: OPR, Operation further defined...
		 * 03: LOD, Push value into top of the stack from M offset, L levels down (uses base pointer)
		 * 04: STO, Pop value from top of stack at at M offset, L levels down (uses base pointer)
		 * 05: CAL, Create new activation record, call procedure at index M (pc = M)
		 * 06: INC, Allocate M locals, first four are: Functional Value (Return value), Static Link (SL), Dynamic Link (DL), and Return Address (RA)
		 * 07: JMP, Jump to index M
		 * 08: JPC, Jump to index M if current stack object is 0
		 * 09: SIO, (SIO,0,1) Pop top stack element to screen
		 * 10: SIO, (SIO,0,2) Input object then push it to the stack
		 * 11: SIO, (SIO,0,3) Halt machine
		 * 
		 */
		case 1: // LIT
			stack[SP] = IR.M;
			SP++;
			break;
		case 2: // OPR
			switch(IR.M){
				/*
				* Our 14 cases are as follows:
				* 
				* 00: RET, return and go to the last base
				* 01: NEG, reverse the value of the current stack object
				* 02: ADD, add current stack object to the last, decrements stack
				* 03: SUB, subtract current stack object from the last, decrements stack
				* 04: MUL, multiply current stack object to the last, decrements stack
				* 05: DIV, divide the last stack object from the current, decrements stack
				* 06: ODD, mods the current stack object by two
				* 07: MOD, mods last stack object by current, decrements stack
				* 08: EQL, checks equality between current stack object and the last, decrements stack
				* 09: NEQ, checks inequality between current stack object and the last, decrements stack
				* 10: LSS, checks if last stack object was less than the current, decrements stack
				* 11: LEQ, checks if last stack object was less than or equal to current, decrements stack
				* 12: GTR, checks if last stack object was greater than current, decrements stack
				* 13: GEQ, checks if last stack object was greater than or equal to current, decrements stack
				*/
				case 0: // RET
					SP = BP - 1;
					PC = stack[SP + 3];
					BP = stack[SP + 2];
					break;
				case 1: // NEG
					stack[SP - 1] = -stack[SP - 1];
					break;
				case 2: // ADD
					SP--;
					stack[SP - 1] = stack[SP - 1] + stack[SP];
					break;
				case 3: // SUB
					SP--;
					stack[SP - 1] = stack[SP - 1] - stack[SP];
					break;
				case 4: // MUL
					SP--;
					stack[SP - 1] = stack[SP - 1] * stack[SP];
					break;
				case 5: // DIV
					SP--;
					stack[SP - 1] = stack[SP - 1] / stack[SP];
					break;
				case 6: // ODD
					stack[SP - 1] = stack[SP - 1] % 2;
					break;
				case 7: // MOD
					SP--;
					stack[SP - 1] = stack[SP - 1] % stack[SP];
					break;
				case 8: // EQL
					SP--;
					stack[SP - 1] = stack[SP - 1] == stack[SP];
					break;
				case 9: // NEQ
					SP--;
					stack[SP - 1] = stack[SP - 1] != stack[SP];
					break;
				case 10: // LSS
					SP--;
					stack[SP - 1] = stack[SP - 1] < stack[SP];
					break;
				case 11: // LEQ
					SP--;
					stack[SP - 1] = stack[SP - 1] <= stack[SP];
					break;
				case 12: // GTR
					SP--;
					stack[SP - 1] = stack[SP - 1] > stack[SP];
					break;
				case 13: // GEQ
					SP--;
					stack[SP - 1] = stack[SP - 1] >= stack[SP];
					break;
				default:
					printf("Something terrible went wrong, default case for M code under OPR reached, M: %d, aborting...\n", IR.M);
					shouldLoop = false;

			}
			break;
		case 3: // LOD
			stack[SP] = stack[base(IR.L) - 1 + IR.M];
			SP++;
			break;
		case 4: // STO
			SP--;
			stack[base(IR.L) - 1 + IR.M] = stack[SP];
			break;
		case 5: // CAL
			// When calling, we need to push our activation record to the stack
			stack[SP] = 0; // The first element is our return value (FV)
			stack[SP+1] = base(IR.L); // The second element is the static link (SL), the base pointer L levels down
			stack[SP+2] = BP; // Our third element is our dynamic link (DL), our current base pointer
			stack[SP+3] = PC; // And our last element is our return address (RA), the program counter

			BP = SP + 1; // Our current base pointer becomes the bottom of the activation record
			PC = IR.M; // And finally we jump
			break;
		case 6: // INC
			SP = SP + IR.M;
			break;
		case 7: // JMP
			PC = IR.M;
			break;
		case 8: // JPC
			SP--;
			if (stack[SP] == 0){ // if equal
				PC = IR.M;
			}
			break;
		case 9: // (SIO,0,1), We're assuming 1 is the input M code
			SP--;
			printf("%d\n", stack[SP]);
			break;
		case 10: // (SIO,0,2), We're assuming 2 is the input M code
			scanf("%d", &(stack[SP]));
			SP++;
			break;
		case 11: // (SIO,0,3), We're assuming 3 is the input M code
			shouldLoop = false;
			break;
		default:
			printf("Something terrible went wrong, default case for OP code reached, OPCODE: %d, aborting...\n", IR.OP);
			shouldLoop = false;
	}
}

