//TODO: Implement error 8 / 10 / 17 / 19

#include "scanner.h"
#include <string.h>

// debug flag, this should be false when submitted
#define symList "symlist.txt"
#define MAX_SYMBOL_TABLE_SIZE 100
#define MAX_INSTRUCTION_SIZE 10000
#define ERROR_OFFSET -1

char *error[] = {
	"",
	/*01*/"Use = instead of :=.", //done
	/*02*/"= must be followed by a number.", //done
	/*03*/"Identifier must be followed by =.", //done
	/*04*/"const, var, procedure must be followed by identifier.", //done
	/*05*/"Semicolon or comma missing.", //done
	/*06*/"Incorrect symbol after procedure declaration.",
	/*07*/"Statement expected.",
	/*08*/"Incorrect symbol after statement part in block.",
	/*09*/"Period expected.", //implemented
	/*10*/"Semicolon between statements missing.",
	/*11*/"Undeclared identifier.", //implemented
	/*12*/"Assignment to constant or procedure is not allowed.",
	/*13*/"Assignment operator expected.",
	/*14*/"call must be followed by an identifier.", //done
	/*15*/"Call of a constant or variable is meaningless.", //done
	/*16*/"then expected.",
	/*17*/"Semicolon or } expected.",
	/*18*/"do expected.",
	/*19*/"Incorrect symbol following statement.",
	/*20*/"Relational operator expected.",
	/*21*/"Expression must not contain a procedure identifier.",
	/*22*/"Right parenthesis missing.",
	/*23*/"The preceding factor cannot begin with this symbol.",
	/*24*/"An expression cannot begin with this symbol.",
	/*25*/"This number is too large"

};

struct symbol
{
	int kind; // const = 1, var= 2, proc = 3
	char name[12]; // name up to 11 chars
	int val; // number (ASCII value)
	int level; // L level
	int addr; // M address
};

struct instruction
{
    int opCode;
    int L;
    int M;
};

struct symbol symbol_table[MAX_SYMBOL_TABLE_SIZE] = {0};
struct instruction instruction_list[MAX_INSTRUCTION_SIZE] = {0};

enum tokenEnum
{
    nullsym = 1, identSym, numberSym, plusSym, minusSym,    //5
    multSym,  slashSym, oddSym, eqSym, neqSym, lesSym, leqSym,  //12
    gtrSym, geqSym, lparenSym, rparenSym, commaSym, semicolonSym,   //18
    periodSym, becomesSym, beginSym, endSym, ifSym, thenSym,    //24
    whileSym, doSym, callSym, constSym, varSym, procSym, writeSym,  //31
    readSym , elseSym //33
};

int token = 0;
int number = 0;
char* ident;

int codePointer = 0;

FILE *lexemeListFile;


bool scannerOutput = false;

void initializeErrors();

void getToken();
void getIdent();
void getNumber();
void throwError(int i);
void emit(int op, int L, int M);

void parseConst();
void parseVar();
void parseProcedure();
void parseFactor();
void parseTerm();
void parseExpression();
void parseCall();
void parseBecome();
void parseBegin();
void parseIf();
void parseWhile();
void parseRead();
void parseWrite();
void parseStatement();
void parseBlock();
void parseProgram();
void parseCondition();

int isRelational(enum tokenEnum t);
void throwError(int i);

void emit(int op, int L, int M){

    instruction_list[codePointer].opCode = op;
    instruction_list[codePointer].L = L;
    instruction_list[codePointer].M = M;
}

struct symbol getSymbol(char* namein){
    //printf("getting symbol %s\n", namein);
    int i;
	for (i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++){
        //printf("looking at symbol %s\n", symbol_table[i].name);
		if (strcmp(symbol_table[i].name, namein) == 0){
		    //printf("returning symbol\n");
			return symbol_table[i];
		}
	}
	throwError(11);
	exit(-1); //shouldn't happen
}

void putSymbol(struct symbol symbolin){
    //printf("putting symbol\n");
    int i;
	for (i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++){
		if (symbol_table[i].kind == 0){
			symbol_table[i] = symbolin;
			return;
		}
	}

	printf("UNEXPECTED ERROR: Symboltable is full\n");
	return;
}

struct symbol constructSymbol(int kin, char *nam, int va, int L, int M){
    //printf("constructing sybmol\n");
	struct symbol toReturn;
	toReturn.kind = kin;
	int i;
	for (i = 0; i < 12; i++)
    {
        toReturn.name[i] = nam[i];
    }
	toReturn.val = va;
	toReturn.level = L;
	toReturn.addr = M;
	/*printf("putting symbol kind: %d\tname:", kin);
	for(i = 0; i < 12; i++)
    {
        printf("%c", nam[i]);
    }
    printf("\tval: %d\n", va);*/
	return toReturn;
}

void throwError(int i){
    printf("Throwing error...\n");
	printf("Error %d: %s\n",i, error[i]);
	exit(i);
}

void getNumber(){
    //printf("getting number\n");
	fscanf(lexemeListFile, "%d", &number);
	if (number > 99999){
		throwError(25);
	}
	if(scannerOutput){
		printf("%d ",number);
	}
	//printf("number is %d\n", number);
}

void getIdent(){
    //printf("getting ident\n");
	clearScanner(ident);
	fscanf(lexemeListFile, "%s", ident);
	if(scannerOutput){
		printf("%s ",ident);
	}
}

void getToken(){
    //printf("getting token\n");
	fscanf(lexemeListFile, "%d", &token);
	//printf("next token: %d\n", token);
	if(scannerOutput){
		printf("%d ",token);
	}
}


/**
 * ["const" ident "=" number {"," ident "=" number } ";" ]
 */
void parseConst(){
	if (token == constSym){
        //printf("parsing const\n");
		getToken();
		if (token != identSym){
			//ERROR: expcted ident
			throwError(4);
		}
		getIdent();
		getToken();
		if(token != eqSym)
        {
            if(token == becomesSym)
            {
                throwError(1);
            }
            //ERROR: expected eqsym
            throwError(3);
        }
        getToken();
        if(token != numberSym)
        {
            //ERROR: expected number
            throwError(2);
        }
        getNumber();
        //printf("line 228\n");
        putSymbol(constructSymbol(1,ident,number,0,0));
        getToken();
        //printf("past line 228\n");
		while (token == commaSym){
			getToken();
			if (token != identSym){
				//ERROR: expcted ident
				throwError(4);
			}
			getIdent();
			getToken();
			if(token != eqSym)
            {
                if(token == becomesSym)
                {
                    throwError(1);
                }
                //ERROR: expected eqsym
                throwError(3);
            }
            getToken();
            if(token != numberSym)
            {
                //ERROR: expected number
                throwError(2);
            }
            getNumber();
            putSymbol(constructSymbol(1,ident,number,0,0));
            getToken();
        }
        //printf("out of while loop\n");
		if (token != semicolonSym){
			//ERROR: expected semicolonSym
			//printf("token is not semicolon\n");
			throwError(5);
		}
		//printf("line 264\n");
		getToken();
    	//putSymbol(constructSymbol(1,ident,number,0,0));
	}
}


/**
 * [ "var" ident {"," ident} ";"]
 */
void parseVar(){
    //printf("parsing variable\n");
	if (token == varSym){
		getToken();
		if (token != identSym){
			//ERROR: expected ident
			throwError(4);
		}
		getIdent();
		//printf("line 253\n");
		putSymbol(constructSymbol(2,ident,0,0,0));
		getToken();
		while (token == commaSym){
			getToken();
			if (token != identSym){
				//ERROR: expected ident
				throwError(4);
			}
			getIdent();
			//printf("line 295\n");
			putSymbol(constructSymbol(2,ident,0,0,0));
			getToken();
		}
		if (token != semicolonSym){
			//ERROR: expected semicolon
			throwError(5);
		}
		getToken();
	}
}

/**
 *{ "procedure" ident ";" block ";" }
 */
void parseProcedure(){
    //printf("parsing procedure\n");
	while (token == procSym){
		getToken();
		if (token != identSym){
			//ERROR: expected ident
			throwError(4);
		}
		getIdent();
		putSymbol(constructSymbol(3,ident,0,0,0));
		getToken();
		if (token != semicolonSym){
			//ERROR: expected semicolon
			throwError(6);
		}
		getToken();
		parseBlock();
		if (token != semicolonSym){
			//ERROR: expected semicolon
			throwError(5);
		}
		getToken();
	}
}

/**
 * ident|number|"(" expression ")"
 */
void parseFactor(){
    //printf("parsing factor\n");
	if (token == identSym){
		getIdent();
		if (getSymbol(ident).kind == 3){
			//ERROR: ident is procedure
			throwError(21);
		}
		getToken();
	} else if (token == numberSym){
		getNumber();
		getToken();
	} else if (token == lparenSym){
		getToken();
		parseExpression();
		if (token != rparenSym){
			//ERROR: expected rparen
			throwError(22);
		}
		getToken();
	} else {
		//ERROR: expected ident/number/lparen
		throwError(24); //I think this should be error 23
	}
	//doFactor();
}

/**
 * factor {("*"|"/") factor}
 */
void parseTerm(){
    //printf("parsing term\n");
    parseFactor();
	/*if (token != multSym && token != slashSym){
		//ERROR: expected mult/slash or term
		throwError(23);
	}*/
	while (token == multSym || token == slashSym){
		getToken();
		parseFactor();
		//doTerm();
	}

}

/**
 * [ "+"|"-"] term { ("+"|"-") term}.
 */
 //I think we need to fix this
void parseExpression(){
    //printf("parsing expression\n");
	if (token == plusSym || token == minusSym){
		getToken();

		//ERROR: expected plus/minus or expression
		//throwError(23);
	}
	parseTerm();
	while(token == plusSym || token == minusSym){
		getToken();
		parseTerm();
	}
}

/**
 * ident ":=" expression
 */
void parseBecome(){
    //printf("parsing become\n");
	if (token != identSym){
		// ERROR: expected ident
		throwError(4);
	}
	getIdent();
	//printf("about to look for constants and proc\n");
	if (getSymbol(ident).kind == 1 || getSymbol(ident).kind == 3){
        //printf("line 415");
		throwError(12);
	}
    getToken();
	if (token != becomesSym){
        //ERROR: expected becomes
        throwError(13);
	}

	//overwrite var stuff here

	getToken();
	parseExpression();
}

void parseCall(){
    //printf("parsing call\n");
	getToken();
	if (token != identSym){
		//ERROR: expected ident
		throwError(14);
	}
	getIdent();
	if(getSymbol(ident).kind == 1 || getSymbol(ident).kind == 2){
		throwError(15);
	}
	getToken();
	// call stuff here
}

/**
 * "begin" statement { ";" statement} "end"
 */
void parseBegin(){
    //printf("parsing begin\n");
	getToken();
	parseStatement();
	while(token == semicolonSym){
		getToken();
		parseStatement();
	}

	if(token != endSym){
		//ERROR: expected end or semicolon
		throwError(10);
	}
	getToken();
}


/**
 * "if" condititon "then" statement ["else" statement]
 */
void parseIf(){ //generates code
    //printf("parsing if\n");
	getToken();
	parseCondition();
	if (token != thenSym){
		//ERROR: expected then
		throwError(16);
	}
	getToken();
	int jmpAddress = codePointer;
	//JPC
	emit(0, 0, 0);
	parseStatement();
	//correct target address for jump to skip
	instruction_list[jmpAddress].M = codePointer;
	if(token == elseSym){
		getToken();
        int jmpAddress = codePointer;
        //JPC
        emit(0, 0, 0);
        parseStatement();
        //correct target address for jump to skip
        instruction_list[jmpAddress].M = codePointer;
	}

}

/**
 * "while" condition "do" statement
 */
void parseWhile(){ //generates code
    //printf("parsing while\n");
	getToken();
	parseCondition();
	if(token != doSym){
		//ERROR: expected do
		throwError(18);
	}
	getToken();
	int jmpAddress = codePointer;
	//JPC
	emit(0, 0, 0);
	parseStatement();
	//correct target address for jump to skip
	instruction_list[jmpAddress].M = codePointer;

	//return to top of while
	//JMP
	emit(0, 0, jmpAddress);
}

/**
 * "read" ident
 */
void parseRead(){
	getToken();
	if (token != identSym){
		//ERROR: expected ident
        throwError(4);
	}
	getIdent();
	if (getSymbol(ident).kind == 1 || getSymbol(ident).kind == 3){
		throwError(12);
	}

	//read data stuff here
	getToken();
}


/**
 * "write" ident
 */
void parseWrite(){
	getToken();
	if (token != identSym){
		//ERROR: expected ident
		throwError(4);
	}
	getIdent();
	if (getSymbol(ident).kind == 2){
		//ERROR: cannot print procedure
	}

	//write stuff here
}
/**
 *[ ident ":=" expression
 * | "call" ident
 * | "begin" statement { ";" statement } "end"
 * | "if" condition "then" statement ["else" statement]
 * | "while" condition "do" statement
 * | "read" ident
 * | "write" ident
 * | e ] .
 */

void parseStatement(){
    //printf("parsing statement\n");
	if (token == identSym){
		parseBecome();
	} else if (token == callSym){
		parseCall();
	} else if (token == beginSym){
		parseBegin();
	} else if (token == ifSym){
		parseIf();
	} else if (token == whileSym){
		parseWhile();
	} else if (token == readSym){
		parseRead();
	} else if (token == writeSym){
		parseWrite();
	} /*else {
		throwError(7);
	}*/ //statement is nullable
	//getToken();
}

/**
 * block ::= const-declaration var-declaration procedure-declaration statement
 */
void parseBlock(){
    //printf("parsing block\n");
    if(token == constSym)
    {
        parseConst();
    }
    if(token == varSym)
    {
        parseVar();
    }
    if(token == procSym)
    {
        parseProcedure();
    }
	parseStatement();
}

/**
 * program ::= block "."
 */
void parseProgram(){
    //printf("parsing program\n");
    getToken();
	parseBlock();
	if (token != periodSym){
		throwError(9);
	}
	printf("\nThis program is error free!\n");
}

/**
 * "odd" expression | expression rel-op expression
 */
void parseCondition(){
    //getToken();
    if(token == oddSym)
    {
        getToken();
        parseExpression();
    }
    else
    {
        parseExpression();
        if(!isRelational(token))
        {
            //ERROR: expected relational operator
            if(token == becomesSym)
            {
                //use =, not :=
                throwError(1);
            }
            throwError(20);
        }
        getToken();
        parseExpression();
    }
}

int isRelational(enum tokenEnum t)
{
    if(t == eqSym || t == neqSym || t == leqSym || t == lesSym || t == geqSym || t == gtrSym )
    {
        return 1;
    }

    return 0;
}

void makeSymTable()
{
    FILE* out = fopen(symList, "w");
    fprintf(out, "Name   Type   Level   Value\n");
    int i;
    for(i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++)
    {
        int j;
         for (j = 0; j < 7; j++)
        {
            if(symbol_table[i].name[j] == '\0')
            {
                symbol_table[i].name[j+1] = '\0';
                symbol_table[i].name[j] = ' ';
            }
        }


        fprintf(out, "%s", symbol_table[i].name);
        // const = 1, var= 2, proc = 3
        if(symbol_table[i].kind == 1)
        {
            fprintf(out, "const  ");
            fprintf(out, "%d       ", symbol_table[i].level);
            fprintf(out, "%d", symbol_table[i].val);
        }
        if(symbol_table[i].kind == 2)
        {
            fprintf(out, "var    ");
            fprintf(out, "%d       ", symbol_table[i].level);
            fprintf(out, "%d", symbol_table[i].addr);
        }
        if(symbol_table[i].kind == 3)
        {
            fprintf(out, "proc   ");
            fprintf(out, "%d       ", symbol_table[i].level);
            fprintf(out, "%d", symbol_table[i].addr);
        }
        fprintf(out, "\n");
    }
}

int main(int argc, char **argv){

    //Scanner part
    //removeComments();
	//tokenizeInput();

    //parser part
    ident = malloc(12 * sizeof(char));
	if (argc > 1){
		if (strcmp("-l", argv[1]) == 0){
					scannerOutput = true;
			if(DEBUG){
				printf("Scanner output enabled\n");
			}
		} else {
			printf("Invalid input: %s\n", argv[1]);
		}
	}

	lexemeListFile = getFilePointer(lexemeListOutput, "r", DEBUG);

    //initializeErrors();
	//getToken();
	parseProgram();
	makeSymTable();
	return 0;
}
