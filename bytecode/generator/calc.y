
%{

#ifdef DISABLE_PRINTF
#define printf(str, ...) (0)
#endif

#define YYDEBUG 1
#define YYSTYPE char*

#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include "../instructions_enum.h"

#include "dir_macros.h"


	extern int yylex();
	extern int yyparse();
	extern FILE* yyin;
	void yyerror(const char*);

	MAKE_FILE_PTR(bytecode)
	MAKE_FILE_PTR(numbers)
	MAKE_FILE_PTR(strings)
	MAKE_FILE_PTR(attrs)
	MAKE_FILE_PTR(identifiers)

	void close_all();
	void clean_up();

	FILE *input_file_ptr = NULL;

	DIR *dir_ptr = NULL;
	struct dirent *entry = NULL;

	char file_name[3];
	char full_directory[DIR_LEN_FULL + 1 + 5 + 1] = DIR_FULL "/";

#define CHANGE_NAME(character) full_directory[DIR_LEN_FULL + 1] = character;
#define FOPEN(identifier) \
	/* printf("Opening " #identifier " %s\n", full_directory); */ \
	identifier##_file_ptr = fopen(full_directory, "ab+");
#define FOPEN_EXIT(identifier) \
	FOPEN(identifier) \
	if (identifier##_file_ptr == NULL) { \
		fprintf(stderr, "Could not create file %s", #identifier "_file_ptr\n"); \
		close_all(); \
		clean_up(); \
		return -1; \
	}
	//printf("Opened file " #identifier "_file_ptr successfully!\n");


#define FCLOSE(identifier) fclose(identifier##_file_ptr);

	void add_operator(const char* symbol, char type) {
		// Max e.g. "./operators.out 1 '...'" (prelude + 22 characters + null terminator)
		/*
		If previous command = "./operators.out 1 '**'\0"
		Set number to current type 0,1,2
		"./operators.out 0 '**'\0"
		*/
		static char command[DIR_PRELUDE_LEN + 22 + 1] = DIR_PRELUDE "/operators.out ! '";
		// Placeholders - 1st ! = 0,1,2 & 2nd + 2 spaces = 1-3 character operator
		command[DIR_PRELUDE_LEN + 15] = type; // '0' '1' '2'
		// 16 is always a space
		//sprintf(&command[DIR_PRELUDE_LEN + 18], "%s", null_chars);
		for (size_t i = 0; i < 4; ++i)
			command[DIR_PRELUDE_LEN + 18 + i] = '\0';
		// Replace 4 characters with null
		// Removes leftover characters if current operator is shorter than previous
		// Replaces 3 possible operator characters and 1 apostrophe
		strcat(command, symbol);
		command[DIR_PRELUDE_LEN + 18 + strlen(symbol)] = '\'';
		//printf("Last 4 characters: '%c' '%c' '%c' '%c'\n", command[DIR_PRELUDE_LEN + 18],command[DIR_PRELUDE_LEN + 19],command[DIR_PRELUDE_LEN + 20],command[DIR_PRELUDE_LEN + 21]);
		//printf("\ncommand: \"%s\" %lu, %lu\n", command, strlen(command) + 1, DIR_PRELUDE_LEN + 22 + 1);

		unsigned char result[2] = { I_OP, WEXITSTATUS(system(command)) };
		//printf("%x %x", result[0], result[1]);

		fwrite(result, CHAR_SIZE, 2 * CHAR_SIZE, bytecode_file_ptr);
		//fwrite("Hello!", sizeof(char), 7, bytecode_file_ptr);
	}

/*
	void add_operator(const char* symbol) {
		return add_operator(symbol, '2');
	}
*/

	void strcat_end(char* destination, const char* append) {
		// "abcdefg", "123"
		destination[strlen(destination) - strlen(append)] = '\0';
		// "abcd\0fg", "123" (\0 = null terminator)
		strcat(destination, append);
		// "abcd123"
	}
	
	void int_to_zfill(char* destination, int value) {
		// 3 chars max - Unix supports 0-255 only
		// Therefore max 3 characters
		// This program returns positive only but can also return -1 for error
		sprintf(destination, "%03d", value);
	}
	
	bool dir_contains_int(char* name_as_string) {
		while ((entry = readdir(dir_ptr)) != NULL) { // also includes . and ..
			if (!strcmp(name_as_string, entry->d_name)) {
				return true;
			}
		}
		return false;
	}
	
	unsigned char rand_unsigned() {
		return (rand() % 122) + 3; // 3 to 125
	}
	int contains_pattern(const char* chars, FILE* file_ptr) {
		size_t current_letter_index = 0;
		size_t current_match_length = 0;
		size_t current_word_index = 0;
		const size_t string_len = strlen(chars);
		for (int char_int = 0; (char_int = getc(file_ptr)) != EOF;) {
			//printf("Loop start %c vs %c %zu\n", char_int, chars[current_letter_index], current_letter_index);
			if (char_int == '\0' ) {
				/*
				Comparing, e.g. 32 (already met) to 3210 (checking if met), if the
				2 is met, the current character is null because it is beyond the
				size of the current vaue being checked.
				*/
				if (chars[current_letter_index] == '\0') { // If the word checked also ends here
					// Reached end at appropriate time
					//printf("Met null\n");
					return current_word_index;
				} else {
					// Too early! The matched string reached null before end of the word
					current_letter_index = current_match_length = 0;
					++current_word_index;
					continue;
				}
			} else {
				if (current_letter_index >= string_len || chars[current_letter_index] != (char)char_int) {
					//printf("Advancing to next null\n");
					while ((char_int = getc(file_ptr)) != 0);
					current_letter_index = current_match_length = 0;
					++current_word_index;
					continue;
					// Advance until next null (for loop advances 1 more)
				} else {
					++current_match_length;
				}
			}
			++current_letter_index;
		}
		return -1;
	}

	#define GENERIC_ADD_BODY(instruction, file_prefix, character) \
		static size_t count = 0; \
		unsigned char bytecode_instructions[2] = { I_PUSH_##instruction, 0 }; \
		fclose(file_prefix##_file_ptr); \
		CHANGE_NAME(character) \
		FOPEN(file_prefix) \
		int same_as = contains_pattern(yylval_string, file_prefix##_file_ptr); \
		/* printf("Contains string \"%s\" %d\n", yylval_string, contains_pattern(yylval_string, file_prefix##_file_ptr)); */ \
		if (same_as == -1) { \
			bytecode_instructions[1] = count++; \
			fwrite(yylval_string, CHAR_SIZE, strlen(yylval_string) / CHAR_SIZE, file_prefix##_file_ptr); \
			fwrite("\0", CHAR_SIZE, CHAR_SIZE, file_prefix##_file_ptr); \
		} else { \
			bytecode_instructions[1] = same_as; \
		} \
		fwrite(bytecode_instructions, CHAR_SIZE, 2 * CHAR_SIZE, bytecode_file_ptr);

	
	void add_number(const char* yylval_string) { GENERIC_ADD_BODY(INT, numbers, 'n') }
	void add_string(const char* yylval_string) { GENERIC_ADD_BODY(STR, strings, 's') }
	void add_attr(const char* yylval_string) { GENERIC_ADD_BODY(ATTR, attrs, 'a') }
	void add_id(const char* yylval_string) { GENERIC_ADD_BODY(ID, identifiers, 'i') }

	void add_instruction(unsigned char byte) {
		unsigned char byte_copy = byte;
		fwrite(&byte_copy, CHAR_SIZE, CHAR_SIZE, bytecode_file_ptr);
	}

	void syntax_error() {

	}

#define TRY_CLOSE(file_prefix) if (file_prefix##_file_ptr) { FCLOSE(file_prefix) }

	void close_all() {
		closedir(dir_ptr);

		TRY_CLOSE(bytecode)
		TRY_CLOSE(numbers)
		TRY_CLOSE(strings)
		TRY_CLOSE(attrs)
		TRY_CLOSE(identifiers)
	}
	
void clean_up() {
	printf("Cleaning up files; error in bytecode generator\n");
	close_all();
	system("rm *.bin"); // Deletes all 5 files; *.bin is just in case!
	system("rmdir .."); // Delete now-empty folder
}
%}

%locations

%define parse.error verbose

%token T_NUM T_ID T_STR
%token T_LET ":="
%token T_STAR_STAR "**" T_DOT_DOT_DOT "..." T_DOT_DOT ".." T_ADJ "->"
%token T_PLUS_PLUS "++" T_MINUS_MINUS "--"
%token T_AND "AND" T_OR "OR" T_NOT "NOT" T_IN "IN"
%token T_EEQ "===" T_NEEQ "!==" T_EQ "==" T_NEQ "!=" T_GEQ ">=" T_LEQ "<=" T_GT ">" T_LT "<"

/*
%left O_ADJ_MUL
%right O_ARG_EXPAND
%left "===" "!==" "==" "!=" ">=" "<=" ">" "<"
%right '='
%left ':'
%left "IN"
%left "..." ".."
%left '+' '-'
%left '*' '/'
%right O_NEG
%right "**"
%right "++" "--" // if before operand
%right '('
*/

%%

start:
	lines
;

lines: // 1 or more lines
	line
|	line lines

line:
	';'									{ add_instruction(I_NOP), add_instruction(I_END_EXP); }
|	expr ';'							{ add_instruction(I_END_EXP); }
;


csv:
|	expr
|	csv ',' expr
;

basic_operand:
	T_NUM								{ add_number(yylval);  }
|	T_STR								{ add_string(yylval); }
|	T_ID								{ add_id(yylval);  }
|	'{' '}'								{ add_instruction(I_MARK_CODE); add_instruction(I_NOP); add_instruction(I_MAKE_CODE); }
|	'{'									{ add_instruction(I_MARK_CODE);  }
	lines '}'							{ add_instruction(I_MAKE_CODE);  }
//|	'{'									{ add_instruction(I_MARK_CODE);  }
	//lines expr '}'						{ add_instruction(I_MAKE_CODE);  }
|	'['									{ add_instruction(I_MARK_LIST);  }
	csv ']'								{ add_instruction(I_MAKE_LIST);  }
|	'(' expr ')'				
|	basic_operand '.' T_ID						{ add_attr(yylval); }
;

expr:
	assignment
;

assignment:
	argument_expansion
|	argument_expansion '=' assignment	{ add_instruction(I_ASSIGN);  }
|	argument_expansion ":=" assignment	{ add_instruction(I_LET);  }

argument_expansion:
	comparisons
|	'*'	comparisons						{ add_operator("*", '0');  }
;

comparisons:
	containment
|	comparisons "===" containment		{ add_operator("===", '2');  }
|	comparisons "!==" containment		{ add_operator("!==", '2');  }
|	comparisons "==" containment		{ add_operator("==", '2');  }
|	comparisons "!=" containment		{ add_operator("!=", '2');  }
|	comparisons ">=" containment		{ add_operator(">=", '2');  }
|	comparisons "<=" containment		{ add_operator("<=", '2');  }
|	comparisons ">" containment			{ add_operator(">", '2');  }
|	comparisons "<" containment			{ add_operator("<", '2');  }
;

containment:
	logical_xnor
|	containment "IN" logical_xnor		{ add_operator("IN", '2'); }
;

logical_xnor:
	logical_xor
|	logical_xnor "XNOR" logical_xor		{ add_operator("XNOR", '2'); }

logical_xor:
	logical_nor
|	logical_xor "XOR" logical_nor		{ add_operator("XOR", '2'); }
;

logical_nor:
	logical_or
|	logical_nor "NOR" logical_or		{ add_operator("NOR", '2'); }
;

logical_or:
	logical_nand
|	logical_or "OR" logical_nand		{ add_operator("OR", '2'); }
;

logical_nand:
	logical_and
|	logical_nand "NAND" logical_and		{ add_operator("NAND", '2'); }
;


logical_and:
	logical_not
|	logical_and "AND" logical_not		{ add_operator("AND", '2');  }
;

logical_not:
	dots
|	"NOT" dots							{ add_operator("NOT", '0');  }
;

dots:
	add_sub
|	dots ".." add_sub					{ add_operator("..", '2'); }
|	dots "..." add_sub					{ add_operator("...", '2'); }
;

add_sub:
	mul_div_mod
|	add_sub '+' mul_div_mod				{ add_operator("+", '2'); }
|	add_sub '-' mul_div_mod				{ add_operator("-", '2'); }
;

mul_div_mod:
	unary_negate
|	mul_div_mod '*' unary_negate		{ add_operator("*", '2'); }
|	mul_div_mod '/' unary_negate		{ add_operator("/", '2'); }
|	mul_div_mod '%' unary_negate		{ add_operator("%", '2'); }
;
unary_negate:
	exponentiation
|	'-' exponentiation					{ add_operator("-", '0'); }
;

exponentiation:
	adjacent
|	adjacent "**" exponentiation		{ add_operator("**", '2'); }
;

adjacent:
	pre_unary
|	adjacent "->"						{ add_instruction(I_MARK_CALL); }
	pre_unary								{ add_instruction(I_MAKE_CALL); }
;

pre_unary:
	post_unary
|	"++" post_unary						{ add_operator("++", '0');  }
|	"--" post_unary						{ add_operator("--", '0');  }
;

post_unary:
	copy
|	copy "++"							{ add_operator("++", '1'); }
|	copy "--"							{ add_operator("--", '1'); }
|	copy "!"							{ add_operator("!", '1'); }
;

copy:
	call
|	'@' call							{ add_operator("@", '0'); }
;

call:
	basic_operand
|	basic_operand '('							{ add_instruction(I_MARK_CALL); }
	csv ')'								{ add_instruction(I_MAKE_CALL); }

/*
getattr:
	basic_operand
|	getattr '.' basic_operand					{ add_attr(yylval); }
;
*/

%%
int main(int argc, char** argv) {
	srand((unsigned int)clock());

	dir_ptr = opendir(DIR_FULL);

	//printf(DIR_FULL "\n");

	int file_name_as_uchar;
	// 1 for '/' and 5 for name of file (*.bin), 1 for '\0'
	// 5 more than initially needed

	char cwd[PATH_MAX];

	chdir(DIR_NAME);

	//printf("cwd:%s\n", getcwd(cwd, sizeof(cwd)));

	struct stat st = { 0 };


	for (;;) {
		//file_name_as_uchar = rand_unsigned();
		//if (file_name_as_uchar <= 1) continue; // 0 and 1 are reserved
		int_to_zfill(file_name, file_name_as_uchar = rand_unsigned());
		//char relative_dir[13] = "./"; 
		//strcat(relative_dir, file_name);
		if (stat(file_name, &st) == -1) {
			if (mkdir(file_name, 0777)) {
				// octal: rwx-xr-x (Unix)
				printf("Error creating '%s'", file_name);
				perror("mkdir");
				return -1; // Nothing created at this point
			} 
			break;
		}

	}

	chdir("..");

	strcat(full_directory, file_name);
	// e.g. now "./bytecode-files/0123456789"

	//printf("Full directory: %s\n", full_directory);

	//printf("cwd:%s\n", getcwd(cwd, sizeof(cwd)));

	/*
	b.bin: code

	n.bin: numbers/integers (stored as strings)
	s.bin: strings
	a.bin: attributes
	i.bin: identifiers

	Latter stored as e.g. print\0var1\0var2\0
	Numbers stored as strings so they can use null terminators.
	Likely to change in the future and store literal bytes
	boost::multiprecision::cpp_int can be constructed from string
	
	*/
	
	strcat(full_directory, "/b.bin");

	//printf("%s\n", full_directory);

	FOPEN(bytecode)
	//bytecode_file_ptr = fopen(full_directory, "ab+");

	CHANGE_NAME('n') FOPEN(numbers)

	CHANGE_NAME('s') FOPEN(strings)

	CHANGE_NAME('a') FOPEN(attrs)

	CHANGE_NAME('i') FOPEN(identifiers)

	input_file_ptr = fopen(argc == 1 ? "input.ul" : argv[1], "r");
	if (!input_file_ptr) {
		fprintf(stderr, "Could not open input file.\n");
		clean_up();
		return -1;
	}

	yyin = input_file_ptr;
	yydebug = 1;
	while(yyparse());

	close_all();

	//printf("Returning %d\n", file_name_as_uchar);

	return file_name_as_uchar;
}


