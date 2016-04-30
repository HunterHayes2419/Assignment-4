//#include <regex.h>
// I really just threw this in because I never used imports before, and I wanted to try to make it work
#include "functions.h"

// Filenames for the input and output files
#define inputCode "input.txt"
#define cleanInputCode "cleaninput.txt" // This will be written then read
#define lexemeTableOutput "lexemetable.txt"
#define lexemeListOutput "lexemelist.txt"

#define scannerSize 11
#define maxNumberSize 5
// debug flag, this should be false when submitted
#define DEBUG false


/**
 * I spent a good amount of time writing code to check the syntax of every statement, making sure everything is both
 * sytaxtically AND grammatically correct. After reading the instructions again, to my fortune and dismay, I discovered
 * that only the grammar is incorrect. This has no effect on the parsing whatsoever.
 */
const char *tokens[] = {
	"", "NULL", "ident", "number", "+", "-", "*", "/", "odd", "=", "<>", "<", "<=", ">", ">=", "(", ")", ",", ";", ".", ":=",
	"begin", "end", "if", "then", "while", "do", "call", "const", "var", "procedure", "write", "read", "else"
};

const char *reservedWords[] = {
	"odd", "begin", "end", "if", "then", "while", "do", "call", "const", "var", "procedure", "write", "read", "else",
};
const char specialChar[] = {
	'+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>', ';', ':'
};
const char invisibleChar[] = {
	' ', '\t', '\n'
};


/**
 * Creates a new file referenced by cleanInputCode that has no comments
 */
void removeComments(){
	/**
	 * We have our two files, the input we read from and the cleanInput we write to
	 * bool commentFlag for comment mode, where we dont keep any of the chars
	 * char scanner, which is the char we add or discard
	 */
	FILE *inputFile = getFilePointer(inputCode, "r", DEBUG);
	FILE *cleanInputFile = getFilePointer(cleanInputCode, "w", DEBUG);

	bool commentFlag = false;
	char scanner;

	if(DEBUG){
		printf("About to start looping...\n");
	}
	/**
	 * https://stackoverflow.com/questions/12048576/how-to-find-eof-through-fscanf
	 */
	// while(!feof(inputFile)){
	//for (; fscanf(inputFile,"%c",&scanner) != EOF ; ){
	while(fscanf(inputFile,"%c",&scanner) != EOF){

		if (commentFlag){
			if(scanner == '*'){
				fscanf(inputFile,"%c",&scanner);
				if (scanner == '/'){ // If we reach the end of the comment flag, we stop comment mode and continue
					commentFlag = false;
				}
			}
			continue;
		} else {
			if (scanner == '/'){
				fscanf(inputFile,"%c", &scanner);
				if (scanner == '*'){ // when we reach a '/', we check if the next char is '*'
					commentFlag = true; // if it is, we activate comment flag
					continue;
				} else { // if it's not, we store the '/'
					fprintf(cleanInputFile, "%c",'/');
				}
			}
		}
		if (DEBUG){
			printf("%c", scanner);
		}
		// storing the character
		fprintf(cleanInputFile, "%c", scanner);
	}

	fclose(inputFile);
	fclose(cleanInputFile);
	if (DEBUG){
		printf("Finished removing comments!!\n\n");
	}
}


/**
 * Checks if char is a character A-Za-z
 * @param c		Character to check
 * @return 		true if it is a character A-Za-z
 */
bool isChar(char c){
	if ((int) c >= (int)'A' && (int) c <= (int)'Z'){
		return true;
	} else if ((int) c >= (int) 'a' && (int) c <= (int)'z'){
		return true;
	}
	return false;
}

/**
 * Checks if it's a number 0-9
 * @param c		Character to check
 * @return 		true if it is a character 0-9
 */
bool isNumber(char c){
	return (int) c >= (int) '0' && (int) c <= (int)'9';
}

/**
 * Checks if it's an invisible character defined in the invisibleChar array
 * @param c		Character to check
 * @return 		true if is a space, tab, or newline
 */
bool isInvisible(char c){
	int i;
	for (i = 0; i < 3; i++){
		if (c == invisibleChar[i]){
			return true;
		}
	}
	return false;
}

/**
 * Checks if it's a special ascii character that's defined
 * @param c		Character to check
 * @return 		true if it is exists in the array specialChar
 */
bool isSpecialChar(char c){
	int i;
	for (i = 0; i < 13; i++){
		if (c == specialChar[i]){
			return true;
		}
	}
	return false;
}

/**
 * Checks if it's an accepted character in this language
 * @param c		Character to check
 * @return 		true if the character recieved is expected
 */
bool isValidChar(char c){
	return isChar(c) || isNumber(c) || isInvisible(c) || isSpecialChar(c);
}

/**
 * Checks if the character signifies the end of a word
 * @param c		Character to check
 * @return 		true if it is a special character or an invisible character
 */
bool isTerminatingChar(char c){
	return isInvisible(c) || isSpecialChar(c);
}

/**
 * Checks if the word is predefined and reserved for a token
 * @param c		Word to check
 * @return 		true if it is predefined as a reserved word
 */
bool isReservedWord(char* c){
	int i;
	for (i = 0; i < 14; i++){
		if(strcmp(c, reservedWords[i]) == 0){
			return true;
		}
	}
	return false;
}

/**
 * Checks if it's a number 0-9
 * @param c		Word to check
 * @return 		integer of the token corresponding to the reserved word, or -1 if the word does not exist
 */
int getReservedToken(char* c){
	int i;
	for (i = 0; i < 14; i++){
		if(strcmp(c, reservedWords[i]) == 0){
			return i == 0? 8 : i + 20;
		}
	}
	return -1;
}

/**
 * Clears the scanner of all data, setting all values to '\0'
 */
void clearScanner(char* scanner){
	int i;
	for (i = 0; i < scannerSize; i++){
		scanner[i] = '\0';
	}
}

/**
 * Writes input data to the lexemeFiles
 * @param scanner 			The number or ident to write, if not NULL when not a number or ident
 * @param tokenID 			The token to write
 * @param lexemeTableFile 	Pointer to the table file to write to
 * @param lexemeListFile 	Pointer to the list file to write to
 */
void writeToLexemeFile(char* scanner, int tokenID, FILE *lexemeTableFile, FILE *lexemeListFile){
	if (tokenID == 2 || tokenID == 3){
		fprintf(lexemeTableFile, "%s\t%d\n",scanner, tokenID);
		fprintf(lexemeListFile, "%d %s ", tokenID, scanner);
	} else {
		fprintf(lexemeTableFile, "%s\t%d\n",tokens[tokenID], tokenID);
		fprintf(lexemeListFile, "%d ", tokenID);
	}
}
/**
 * Reads word in the scanner and writes to the lexeme file
 * @param isNum 			Boolean setting number token instead of ident token
 * @param scanner 			Scanner of word data to read
 * @param lexemeTableFile 	Pointer to the table file to write to
 * @param lexemeListFile 	Pointer ot the list file to write to
 */
void resolveScanner(bool isNum, char* scanner, FILE *lexemeTableFile, FILE *lexemeListFile){
	if (DEBUG){
		printf("Resolving scanner for: %s\n", scanner);
	}

	if(isReservedWord(scanner)){
		writeToLexemeFile('\0', getReservedToken(scanner), lexemeTableFile, lexemeListFile);
	} else if (isNum){
		writeToLexemeFile(scanner, 3, lexemeTableFile, lexemeListFile);
	} else {
		writeToLexemeFile(scanner, 2, lexemeTableFile, lexemeListFile);
	}
}

/**
 * Writes special character token to the lexeme file
 * @param holderChar 		The character in question to parse, may or may not require a look ahead
 * @param cleanInputFile	File to read the lookahead char from
 * @param lexemeTableFile 	Pointer to the table file to write to
 * @param lexemeListFile 	Pointer ot the list file to write to
 * @return 					The last unparsed character
 */
char handleSpecialChar(char holderChar, FILE *cleanInputFile, FILE *lexemeTableFile, FILE *lexemeListFile){
	switch (holderChar){ // single char cases
		case '+':
			writeToLexemeFile('\0', 4, lexemeTableFile, lexemeListFile);
			break;
		case '-':
			writeToLexemeFile('\0', 5, lexemeTableFile, lexemeListFile);
			break;
		case '*':
			writeToLexemeFile('\0', 6, lexemeTableFile, lexemeListFile);
			break;
		case '/':
			writeToLexemeFile('\0', 7, lexemeTableFile, lexemeListFile);
			break;
		case '(':
			writeToLexemeFile('\0', 15, lexemeTableFile, lexemeListFile);
			break;
		case ')':
			writeToLexemeFile('\0', 16, lexemeTableFile, lexemeListFile);
			break;
		case '=':
			writeToLexemeFile('\0', 9, lexemeTableFile, lexemeListFile);
			break;
		case ',':
			writeToLexemeFile('\0', 17, lexemeTableFile, lexemeListFile);
			break;
		case ';':
			writeToLexemeFile('\0', 18, lexemeTableFile, lexemeListFile);
			break;
		case '.':
			writeToLexemeFile('\0', 19, lexemeTableFile, lexemeListFile);
			break;
	}
	char nextChar = ' ';
	fscanf(cleanInputFile, "%c", &nextChar);

	if (DEBUG){
		printf("Handling special character: %c%c\n",holderChar, nextChar);
	}

	if(!isSpecialChar(nextChar)){ // single char cases for double chars
		switch(holderChar){
			case '<':
				writeToLexemeFile('\0', 11, lexemeTableFile, lexemeListFile);
				break;
			case '>':
				writeToLexemeFile('\0', 13, lexemeTableFile, lexemeListFile);
				break;
			case ':':
				writeToLexemeFile('\0', 1, lexemeTableFile, lexemeListFile);
				printf("Error: unexpected colon: %c%c\n", holderChar, nextChar);
				return '\0';
		}
	} else if (nextChar == '='){ // double char cases for '='
		switch (holderChar){
			case '<':
				writeToLexemeFile('\0', 12, lexemeTableFile, lexemeListFile);
				return ' ';
			case '>':
				writeToLexemeFile('\0', 14, lexemeTableFile, lexemeListFile);
				return ' ';
			case ':':
				writeToLexemeFile('\0', 20, lexemeTableFile, lexemeListFile);
				return ' ';
		}
	} else if (holderChar == '<' && nextChar == '>'){ // the notequal case
		writeToLexemeFile('\0', 10, lexemeTableFile, lexemeListFile);
		return ' ';
	}

	if (isSpecialChar(nextChar)){
        return handleSpecialChar(nextChar, cleanInputFile, lexemeTableFile, lexemeListFile);
	}
	return nextChar;
}

/**
 * Reads file from cleanInputFile and parses data to the lexeme files
 */
void tokenizeInput(){
	FILE *cleanInputFile = getFilePointer(cleanInputCode, "r", DEBUG);
	FILE *lexemeTableFile = getFilePointer(lexemeTableOutput, "w", DEBUG);
	FILE *lexemeListFile = getFilePointer(lexemeListOutput, "w", DEBUG);
	char* scanner = malloc(sizeof(char) * (scannerSize + 1)); // extra for the NUL char
	char holderChar = '^';
	int pos = 0;
	clearScanner(scanner); // initializing values to NUL
	bool isNum = false;

	while(fscanf(cleanInputFile,"%c",&holderChar) != EOF){ // scanning loop
		if (isValidChar(holderChar)){ // makes sure every read character is valid
			if (pos == 0){ // makes sure it doesn't read the scanner if the first character is invisible
				if(isNumber(holderChar)){
					isNum = true;
				}
				if (isInvisible(holderChar)){
					continue;
				}
			}
			if(isTerminatingChar(holderChar)){ // if it's the end of the word to be read
				if (pos >= (isNum ? 5 : 11)){ // checks if word is out of range
					printf("Error %d: %s is out of range\n", (isNum ? 2 : 3), (isNum ? "number" : "ident"));
					return;
                } else if (pos != 0){ // resolves word
                    resolveScanner(isNum, scanner, lexemeTableFile, lexemeListFile);
                    clearScanner(scanner);
                    isNum = false;
                    pos = 0;
                }
                if (isSpecialChar(holderChar)){ // if we have an extra special char
                    char nextChar = handleSpecialChar(holderChar, cleanInputFile, lexemeTableFile, lexemeListFile);
                    if (nextChar == '\0'){ // the condition where something went wrong when resolving the special character
                        return;
                    } else if (isNumber(nextChar)){ // adding the extra character
                        isNum = true;
                        scanner[pos] = nextChar;
                        pos++;
                    } else if (isChar(nextChar)){
                        scanner[pos] = nextChar;
                        pos++;
                    } else if (isInvisible(nextChar)){
                    	continue;
                    } else {
                        printf("Something terrible happened after recieving a special character\n");
                        return;
                    }
                    continue;
				}
				//resolveScanner(isNum, scanner, lexemeTableFile, lexemeListFile);
				//clearScanner(scanner);

				continue;
			} // if all the break checks fail at this point, then we finally add it to the scanner
			scanner[pos] = holderChar;
			pos++;
		} else {
			printf("Error 4: chararcter not valid: %c\n",holderChar);
			return;
		}
	}
	if (scanner[0] != '\0'){
        resolveScanner(isNum, scanner, lexemeTableFile, lexemeListFile);
	}
	fclose(cleanInputFile);
	fclose(lexemeTableFile);
	fclose(lexemeListFile);
	if (DEBUG){
		printf("Finished tokenizing input!!!\n\n");
	}
}

