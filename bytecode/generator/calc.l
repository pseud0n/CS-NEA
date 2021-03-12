
%option noyywrap

%{
	// gist.github.com/baccigalupi/1042801 for explanation of format
	#include "dir_macros.h"
	#define YYSTYPE char*
	#include "calc.tab.h"

	extern char *yylval;
	extern int yylineno;
	extern FILE *bytecode_file_ptr, numbers_file_ptr, strings_file_ptr, attrs_file_ptr, identifiers_file_ptr;
	extern char file_name[10];
	extern void close_all();

	unsigned int line_number = 1;
%}

%option yylineno

digit [0-9]
integer {digit}+
alpha [A-Za-z]
id {alpha}({alpha}|{integer}|')*
string \"([^\"\\]|\\.)*\"
whitespace [\t\r\n ]
line_feed [\n]
operator (\+\+|--|-|!|\+|\*|\/|\/\/|%|\*\*|\.\.|\.\.\.)
char_return (\+|-|!|\+|\*|\/|%|\(|\)|\,|\{|\}|\[|\]|=|:|@|\.|;)

%%

"**"			return T_STAR_STAR;
"..."			return T_DOT_DOT_DOT;
".."			return T_DOT_DOT;
"++"			return T_PLUS_PLUS;
"--"			return T_MINUS_MINUS;
"AND"			return T_AND;
"OR"			return T_OR;
"NOT"			return T_NOT;
"IN"			return T_IN;
"=="			return T_EQ;
"!="			return T_NEQ;
"==="			return T_EEQ;
"!=="			return T_NEEQ;
">="			return T_GEQ;
"<="			return T_LEQ;
">"				return T_GT;
"<"				return T_LT;
"->"			return T_ADJ;
":="			return T_LET;

{integer}		{ yylval = strdup(yytext); return T_NUM; }
{id}			{ yylval = strdup(yytext); return T_ID;  }
{string}		{ yylval = strndup(yytext, strlen(yytext) - 1) + 1; return T_STR; }

whitespace
\n				++line_number;
{char_return}	return yytext[0];

%%
/*
int yywrap() {
	return 1;
}
*/

void yyerror(const char* str) {
	/*
	static const size_t line_bufffer_length = 256;
	static char line_buffer[line_bufffer_length];
	fgets(line_buffer, line_bufffer_length, yyin);
	*/
	close_all();
	char remove_command[6 + DIR_LEN_FULL + 1] = "rm -r ";
	strcat(remove_command, DIR_FULL "/");
	strcat(remove_command, file_name);
	printf("Remove command: %s\n", remove_command);
	system(remove_command);
	fprintf(stderr, "Error: %s\nLine number: %u\nAt: '%s'\n",str, line_number, yytext);
	exit(1);
}


