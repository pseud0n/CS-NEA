/* A Bison parser, made by GNU Bison 3.7.3.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.7.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 2 "calc.y"


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

#line 291 "calc.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "calc.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_T_NUM = 3,                      /* T_NUM  */
  YYSYMBOL_T_ID = 4,                       /* T_ID  */
  YYSYMBOL_T_STR = 5,                      /* T_STR  */
  YYSYMBOL_T_LET = 6,                      /* ":="  */
  YYSYMBOL_T_STAR_STAR = 7,                /* "**"  */
  YYSYMBOL_T_DOT_DOT_DOT = 8,              /* "..."  */
  YYSYMBOL_T_DOT_DOT = 9,                  /* ".."  */
  YYSYMBOL_T_ADJ = 10,                     /* "->"  */
  YYSYMBOL_T_PLUS_PLUS = 11,               /* "++"  */
  YYSYMBOL_T_MINUS_MINUS = 12,             /* "--"  */
  YYSYMBOL_T_AND = 13,                     /* "AND"  */
  YYSYMBOL_T_OR = 14,                      /* "OR"  */
  YYSYMBOL_T_NOT = 15,                     /* "NOT"  */
  YYSYMBOL_T_IN = 16,                      /* "IN"  */
  YYSYMBOL_T_EEQ = 17,                     /* "==="  */
  YYSYMBOL_T_NEEQ = 18,                    /* "!=="  */
  YYSYMBOL_T_EQ = 19,                      /* "=="  */
  YYSYMBOL_T_NEQ = 20,                     /* "!="  */
  YYSYMBOL_T_GEQ = 21,                     /* ">="  */
  YYSYMBOL_T_LEQ = 22,                     /* "<="  */
  YYSYMBOL_T_GT = 23,                      /* ">"  */
  YYSYMBOL_T_LT = 24,                      /* "<"  */
  YYSYMBOL_25_ = 25,                       /* ';'  */
  YYSYMBOL_26_ = 26,                       /* ','  */
  YYSYMBOL_27_ = 27,                       /* '{'  */
  YYSYMBOL_28_ = 28,                       /* '}'  */
  YYSYMBOL_29_ = 29,                       /* '['  */
  YYSYMBOL_30_ = 30,                       /* ']'  */
  YYSYMBOL_31_ = 31,                       /* '('  */
  YYSYMBOL_32_ = 32,                       /* ')'  */
  YYSYMBOL_33_ = 33,                       /* '.'  */
  YYSYMBOL_34_ = 34,                       /* '='  */
  YYSYMBOL_35_ = 35,                       /* '*'  */
  YYSYMBOL_36_XNOR_ = 36,                  /* "XNOR"  */
  YYSYMBOL_37_XOR_ = 37,                   /* "XOR"  */
  YYSYMBOL_38_NOR_ = 38,                   /* "NOR"  */
  YYSYMBOL_39_NAND_ = 39,                  /* "NAND"  */
  YYSYMBOL_40_ = 40,                       /* '+'  */
  YYSYMBOL_41_ = 41,                       /* '-'  */
  YYSYMBOL_42_ = 42,                       /* '/'  */
  YYSYMBOL_43_ = 43,                       /* '%'  */
  YYSYMBOL_44_ = 44,                       /* "!"  */
  YYSYMBOL_45_ = 45,                       /* '@'  */
  YYSYMBOL_YYACCEPT = 46,                  /* $accept  */
  YYSYMBOL_start = 47,                     /* start  */
  YYSYMBOL_lines = 48,                     /* lines  */
  YYSYMBOL_line = 49,                      /* line  */
  YYSYMBOL_csv = 50,                       /* csv  */
  YYSYMBOL_basic_operand = 51,             /* basic_operand  */
  YYSYMBOL_52_1 = 52,                      /* $@1  */
  YYSYMBOL_53_2 = 53,                      /* $@2  */
  YYSYMBOL_expr = 54,                      /* expr  */
  YYSYMBOL_assignment = 55,                /* assignment  */
  YYSYMBOL_argument_expansion = 56,        /* argument_expansion  */
  YYSYMBOL_comparisons = 57,               /* comparisons  */
  YYSYMBOL_containment = 58,               /* containment  */
  YYSYMBOL_logical_xnor = 59,              /* logical_xnor  */
  YYSYMBOL_logical_xor = 60,               /* logical_xor  */
  YYSYMBOL_logical_nor = 61,               /* logical_nor  */
  YYSYMBOL_logical_or = 62,                /* logical_or  */
  YYSYMBOL_logical_nand = 63,              /* logical_nand  */
  YYSYMBOL_logical_and = 64,               /* logical_and  */
  YYSYMBOL_logical_not = 65,               /* logical_not  */
  YYSYMBOL_dots = 66,                      /* dots  */
  YYSYMBOL_add_sub = 67,                   /* add_sub  */
  YYSYMBOL_mul_div_mod = 68,               /* mul_div_mod  */
  YYSYMBOL_unary_negate = 69,              /* unary_negate  */
  YYSYMBOL_exponentiation = 70,            /* exponentiation  */
  YYSYMBOL_adjacent = 71,                  /* adjacent  */
  YYSYMBOL_72_3 = 72,                      /* $@3  */
  YYSYMBOL_pre_unary = 73,                 /* pre_unary  */
  YYSYMBOL_post_unary = 74,                /* post_unary  */
  YYSYMBOL_copy = 75,                      /* copy  */
  YYSYMBOL_call = 76,                      /* call  */
  YYSYMBOL_77_4 = 77                       /* $@4  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  50
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   147

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  46
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  32
/* YYNRULES -- Number of rules.  */
#define YYNRULES  79
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  123

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   284


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    43,     2,     2,
      31,    32,    35,    40,    26,    41,    33,    42,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    25,
       2,    34,     2,     2,    45,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    29,     2,    30,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    27,     2,    28,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      36,    37,    38,    39,    44
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   252,   252,   256,   257,   260,   261,   265,   266,   267,
     271,   272,   273,   274,   275,   275,   279,   279,   281,   282,
     286,   290,   291,   292,   295,   296,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   312,   313,   317,   318,   321,
     322,   326,   327,   331,   332,   336,   337,   342,   343,   347,
     348,   352,   353,   354,   358,   359,   360,   364,   365,   366,
     367,   370,   371,   375,   376,   380,   381,   381,   386,   387,
     388,   392,   393,   394,   395,   399,   400,   404,   405,   405
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "T_NUM", "T_ID",
  "T_STR", "\":=\"", "\"**\"", "\"...\"", "\"..\"", "\"->\"", "\"++\"",
  "\"--\"", "\"AND\"", "\"OR\"", "\"NOT\"", "\"IN\"", "\"===\"", "\"!==\"",
  "\"==\"", "\"!=\"", "\">=\"", "\"<=\"", "\">\"", "\"<\"", "';'", "','",
  "'{'", "'}'", "'['", "']'", "'('", "')'", "'.'", "'='", "'*'",
  "\"XNOR\"", "\"XOR\"", "\"NOR\"", "\"NAND\"", "'+'", "'-'", "'/'", "'%'",
  "\"!\"", "'@'", "$accept", "start", "lines", "line", "csv",
  "basic_operand", "$@1", "$@2", "expr", "assignment",
  "argument_expansion", "comparisons", "containment", "logical_xnor",
  "logical_xor", "logical_nor", "logical_or", "logical_nand",
  "logical_and", "logical_not", "dots", "add_sub", "mul_div_mod",
  "unary_negate", "exponentiation", "adjacent", "$@3", "pre_unary",
  "post_unary", "copy", "call", "$@4", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    59,    44,   123,   125,    91,
      93,    40,    41,    46,    61,    42,   280,   281,   282,   283,
      43,    45,    47,    37,   284,    64
};
#endif

#define YYPACT_NINF (-15)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -1,   -15,   -15,   -15,     2,     2,    86,   -15,   -12,   -15,
      34,    55,    99,    89,    21,   -15,    -1,    -8,     7,   -15,
       9,    33,    11,     0,     5,    24,    69,    46,    82,   -15,
      65,    36,    29,   -15,   -15,    12,   -15,   -15,    -3,   -15,
     -15,   -15,    65,   -15,    -1,    34,    67,    33,   -15,   -15,
     -15,   -15,   -15,    97,   -15,    34,    34,    55,    55,    55,
      55,    55,    55,    55,    55,    55,    55,    55,    55,    55,
      55,    55,    86,    86,    86,    86,    86,    86,    86,    99,
     -15,   -15,   -15,   -15,    84,   -13,   -15,   -15,    34,   -15,
     -15,   -15,    11,    11,    11,    11,    11,    11,    11,    11,
       0,     5,    24,    69,    46,    82,   -15,    36,    36,    29,
      29,   -15,   -15,   -15,   -15,    99,   -15,    34,   -15,   -14,
     -15,   -15,   -15
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    10,    12,    11,     0,     0,     0,     5,    14,    16,
       0,     0,     0,     0,     0,     2,     3,    77,     0,    20,
      21,    24,    26,    35,    37,    39,    41,    43,    45,    47,
      49,    51,    54,    57,    61,    63,    65,    68,    71,    75,
      69,    70,    50,    13,     0,     7,     0,    25,    62,    76,
       1,     4,    78,     0,     6,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      66,    72,    73,    74,     0,     0,     8,    18,     7,    19,
      23,    22,    27,    28,    29,    30,    31,    32,    33,    34,
      36,    38,    40,    42,    44,    46,    48,    53,    52,    55,
      56,    58,    59,    60,    64,     0,    15,     0,    17,     0,
      67,     9,    79
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -15,   -15,     4,   -15,    26,   -15,   -15,   -15,   -10,    25,
     -15,   108,    75,    59,    63,    58,    72,    73,    71,    74,
     137,    15,    31,    45,   -11,   -15,   -15,    32,   104,   -15,
     133,   -15
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,    14,    15,    16,    85,    17,    44,    45,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,   115,    36,    37,    38,
      39,    88
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      46,    48,     1,     2,     3,     1,     2,     3,    81,    82,
       4,     5,   117,   117,     6,    55,    43,   118,   122,    79,
      51,    50,    80,    52,     7,    53,     8,    65,     9,     8,
      10,     9,    54,    10,    11,    86,    66,     1,     2,     3,
      12,    83,    67,    56,    13,     4,     5,    13,    84,     6,
      57,    58,    59,    60,    61,    62,    63,    64,     1,     2,
       3,     8,    68,     9,    76,    10,     4,     5,   114,    11,
       6,    77,    78,    72,    73,    12,    74,    75,    86,    13,
      90,    91,     8,    69,     9,    70,    10,   107,   108,     1,
       2,     3,     1,     2,     3,    71,    12,     4,     5,    87,
      13,    89,     1,     2,     3,   109,   110,   121,    40,    41,
       4,     5,   116,     8,   119,     9,     8,    10,     9,    47,
      10,   111,   112,   113,   100,   102,     8,    12,     9,   101,
      10,    13,    92,    93,    94,    95,    96,    97,    98,    99,
     103,   105,   104,    42,    13,   106,    49,   120
};

static const yytype_int8 yycheck[] =
{
      10,    12,     3,     4,     5,     3,     4,     5,    11,    12,
      11,    12,    26,    26,    15,     6,    28,    30,    32,     7,
      16,     0,    10,    31,    25,    33,    27,    16,    29,    27,
      31,    29,    25,    31,    35,    45,    36,     3,     4,     5,
      41,    44,    37,    34,    45,    11,    12,    45,    44,    15,
      17,    18,    19,    20,    21,    22,    23,    24,     3,     4,
       5,    27,    38,    29,    35,    31,    11,    12,    79,    35,
      15,    42,    43,     8,     9,    41,    40,    41,    88,    45,
      55,    56,    27,    14,    29,    39,    31,    72,    73,     3,
       4,     5,     3,     4,     5,    13,    41,    11,    12,    32,
      45,     4,     3,     4,     5,    74,    75,   117,     4,     5,
      11,    12,    28,    27,    88,    29,    27,    31,    29,    11,
      31,    76,    77,    78,    65,    67,    27,    41,    29,    66,
      31,    45,    57,    58,    59,    60,    61,    62,    63,    64,
      68,    70,    69,     6,    45,    71,    13,   115
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,    11,    12,    15,    25,    27,    29,
      31,    35,    41,    45,    47,    48,    49,    51,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    73,    74,    75,    76,
      74,    74,    66,    28,    52,    53,    54,    57,    70,    76,
       0,    48,    31,    33,    25,     6,    34,    17,    18,    19,
      20,    21,    22,    23,    24,    16,    36,    37,    38,    14,
      39,    13,     8,     9,    40,    41,    35,    42,    43,     7,
      10,    11,    12,    44,    48,    50,    54,    32,    77,     4,
      55,    55,    58,    58,    58,    58,    58,    58,    58,    58,
      59,    60,    61,    62,    63,    64,    65,    67,    67,    68,
      68,    69,    69,    69,    70,    72,    28,    26,    30,    50,
      73,    54,    32
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    46,    47,    48,    48,    49,    49,    50,    50,    50,
      51,    51,    51,    51,    52,    51,    53,    51,    51,    51,
      54,    55,    55,    55,    56,    56,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    58,    58,    59,    59,    60,
      60,    61,    61,    62,    62,    63,    63,    64,    64,    65,
      65,    66,    66,    66,    67,    67,    67,    68,    68,    68,
      68,    69,    69,    70,    70,    71,    72,    71,    73,    73,
      73,    74,    74,    74,    74,    75,    75,    76,    77,    76
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     1,     2,     0,     1,     3,
       1,     1,     1,     2,     0,     4,     0,     4,     3,     3,
       1,     1,     3,     3,     1,     2,     1,     3,     3,     3,
       3,     3,     3,     3,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       2,     1,     3,     3,     1,     3,     3,     1,     3,     3,
       3,     1,     2,     1,     3,     1,     0,     4,     1,     2,
       2,     1,     2,     2,     2,     1,     2,     1,     0,     5
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YY_LOCATION_PRINT
#  if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#   define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

#  else
#   define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#  endif
# endif /* !defined YY_LOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  YYUSE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YY_LOCATION_PRINT (yyo, *yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Location data for the lookahead symbol.  */
YYLTYPE yylloc
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 5: /* line: ';'  */
#line 260 "calc.y"
                                                                                { add_instruction(I_NOP), add_instruction(I_END_EXP); }
#line 1814 "calc.tab.c"
    break;

  case 6: /* line: expr ';'  */
#line 261 "calc.y"
                                                                        { add_instruction(I_END_EXP); }
#line 1820 "calc.tab.c"
    break;

  case 10: /* basic_operand: T_NUM  */
#line 271 "calc.y"
                                                                        { add_number(yylval);  }
#line 1826 "calc.tab.c"
    break;

  case 11: /* basic_operand: T_STR  */
#line 272 "calc.y"
                                                                        { add_string(yylval); }
#line 1832 "calc.tab.c"
    break;

  case 12: /* basic_operand: T_ID  */
#line 273 "calc.y"
                                                                        { add_id(yylval);  }
#line 1838 "calc.tab.c"
    break;

  case 13: /* basic_operand: '{' '}'  */
#line 274 "calc.y"
                                                                        { add_instruction(I_MARK_CODE); add_instruction(I_NOP); add_instruction(I_MAKE_CODE); }
#line 1844 "calc.tab.c"
    break;

  case 14: /* $@1: %empty  */
#line 275 "calc.y"
                                                                                { add_instruction(I_MARK_CODE);  }
#line 1850 "calc.tab.c"
    break;

  case 15: /* basic_operand: '{' $@1 lines '}'  */
#line 276 "calc.y"
                                                                        { add_instruction(I_MAKE_CODE);  }
#line 1856 "calc.tab.c"
    break;

  case 16: /* $@2: %empty  */
#line 279 "calc.y"
                                                                                { add_instruction(I_MARK_LIST);  }
#line 1862 "calc.tab.c"
    break;

  case 17: /* basic_operand: '[' $@2 csv ']'  */
#line 280 "calc.y"
                                                                        { add_instruction(I_MAKE_LIST);  }
#line 1868 "calc.tab.c"
    break;

  case 19: /* basic_operand: basic_operand '.' T_ID  */
#line 282 "calc.y"
                                                                        { add_attr(yylval); }
#line 1874 "calc.tab.c"
    break;

  case 22: /* assignment: argument_expansion '=' assignment  */
#line 291 "calc.y"
                                                { add_instruction(I_ASSIGN);  }
#line 1880 "calc.tab.c"
    break;

  case 23: /* assignment: argument_expansion ":=" assignment  */
#line 292 "calc.y"
                                                { add_instruction(I_LET);  }
#line 1886 "calc.tab.c"
    break;

  case 25: /* argument_expansion: '*' comparisons  */
#line 296 "calc.y"
                                                                        { add_operator("*", '0');  }
#line 1892 "calc.tab.c"
    break;

  case 27: /* comparisons: comparisons "===" containment  */
#line 301 "calc.y"
                                                { add_operator("===", '2');  }
#line 1898 "calc.tab.c"
    break;

  case 28: /* comparisons: comparisons "!==" containment  */
#line 302 "calc.y"
                                                { add_operator("!==", '2');  }
#line 1904 "calc.tab.c"
    break;

  case 29: /* comparisons: comparisons "==" containment  */
#line 303 "calc.y"
                                                { add_operator("==", '2');  }
#line 1910 "calc.tab.c"
    break;

  case 30: /* comparisons: comparisons "!=" containment  */
#line 304 "calc.y"
                                                { add_operator("!=", '2');  }
#line 1916 "calc.tab.c"
    break;

  case 31: /* comparisons: comparisons ">=" containment  */
#line 305 "calc.y"
                                                { add_operator(">=", '2');  }
#line 1922 "calc.tab.c"
    break;

  case 32: /* comparisons: comparisons "<=" containment  */
#line 306 "calc.y"
                                                { add_operator("<=", '2');  }
#line 1928 "calc.tab.c"
    break;

  case 33: /* comparisons: comparisons ">" containment  */
#line 307 "calc.y"
                                                        { add_operator(">", '2');  }
#line 1934 "calc.tab.c"
    break;

  case 34: /* comparisons: comparisons "<" containment  */
#line 308 "calc.y"
                                                        { add_operator("<", '2');  }
#line 1940 "calc.tab.c"
    break;

  case 36: /* containment: containment "IN" logical_xnor  */
#line 313 "calc.y"
                                                { add_operator("IN", '2'); }
#line 1946 "calc.tab.c"
    break;

  case 38: /* logical_xnor: logical_xnor "XNOR" logical_xor  */
#line 318 "calc.y"
                                                { add_operator("XNOR", '2'); }
#line 1952 "calc.tab.c"
    break;

  case 40: /* logical_xor: logical_xor "XOR" logical_nor  */
#line 322 "calc.y"
                                                { add_operator("XOR", '2'); }
#line 1958 "calc.tab.c"
    break;

  case 42: /* logical_nor: logical_nor "NOR" logical_or  */
#line 327 "calc.y"
                                                { add_operator("NOR", '2'); }
#line 1964 "calc.tab.c"
    break;

  case 44: /* logical_or: logical_or "OR" logical_nand  */
#line 332 "calc.y"
                                                { add_operator("OR", '2'); }
#line 1970 "calc.tab.c"
    break;

  case 46: /* logical_nand: logical_nand "NAND" logical_and  */
#line 337 "calc.y"
                                                { add_operator("NAND", '2'); }
#line 1976 "calc.tab.c"
    break;

  case 48: /* logical_and: logical_and "AND" logical_not  */
#line 343 "calc.y"
                                                { add_operator("AND", '2');  }
#line 1982 "calc.tab.c"
    break;

  case 50: /* logical_not: "NOT" dots  */
#line 348 "calc.y"
                                                                        { add_operator("NOT", '0');  }
#line 1988 "calc.tab.c"
    break;

  case 52: /* dots: dots ".." add_sub  */
#line 353 "calc.y"
                                                                { add_operator("..", '2'); }
#line 1994 "calc.tab.c"
    break;

  case 53: /* dots: dots "..." add_sub  */
#line 354 "calc.y"
                                                                { add_operator("...", '2'); }
#line 2000 "calc.tab.c"
    break;

  case 55: /* add_sub: add_sub '+' mul_div_mod  */
#line 359 "calc.y"
                                                        { add_operator("+", '2'); }
#line 2006 "calc.tab.c"
    break;

  case 56: /* add_sub: add_sub '-' mul_div_mod  */
#line 360 "calc.y"
                                                        { add_operator("-", '2'); }
#line 2012 "calc.tab.c"
    break;

  case 58: /* mul_div_mod: mul_div_mod '*' unary_negate  */
#line 365 "calc.y"
                                                { add_operator("*", '2'); }
#line 2018 "calc.tab.c"
    break;

  case 59: /* mul_div_mod: mul_div_mod '/' unary_negate  */
#line 366 "calc.y"
                                                { add_operator("/", '2'); }
#line 2024 "calc.tab.c"
    break;

  case 60: /* mul_div_mod: mul_div_mod '%' unary_negate  */
#line 367 "calc.y"
                                                { add_operator("%", '2'); }
#line 2030 "calc.tab.c"
    break;

  case 62: /* unary_negate: '-' exponentiation  */
#line 371 "calc.y"
                                                                { add_operator("-", '0'); }
#line 2036 "calc.tab.c"
    break;

  case 64: /* exponentiation: adjacent "**" exponentiation  */
#line 376 "calc.y"
                                                { add_operator("**", '2'); }
#line 2042 "calc.tab.c"
    break;

  case 66: /* $@3: %empty  */
#line 381 "calc.y"
                                                                { add_instruction(I_MARK_CALL); }
#line 2048 "calc.tab.c"
    break;

  case 67: /* adjacent: adjacent "->" $@3 pre_unary  */
#line 382 "calc.y"
                                                                                { add_instruction(I_MAKE_CALL); }
#line 2054 "calc.tab.c"
    break;

  case 69: /* pre_unary: "++" post_unary  */
#line 387 "calc.y"
                                                                { add_operator("++", '0');  }
#line 2060 "calc.tab.c"
    break;

  case 70: /* pre_unary: "--" post_unary  */
#line 388 "calc.y"
                                                                { add_operator("--", '0');  }
#line 2066 "calc.tab.c"
    break;

  case 72: /* post_unary: copy "++"  */
#line 393 "calc.y"
                                                                        { add_operator("++", '1'); }
#line 2072 "calc.tab.c"
    break;

  case 73: /* post_unary: copy "--"  */
#line 394 "calc.y"
                                                                        { add_operator("--", '1'); }
#line 2078 "calc.tab.c"
    break;

  case 74: /* post_unary: copy "!"  */
#line 395 "calc.y"
                                                                        { add_operator("!", '1'); }
#line 2084 "calc.tab.c"
    break;

  case 76: /* copy: '@' call  */
#line 400 "calc.y"
                                                                        { add_operator("@", '0'); }
#line 2090 "calc.tab.c"
    break;

  case 78: /* $@4: %empty  */
#line 405 "calc.y"
                                                                                { add_instruction(I_MARK_CALL); }
#line 2096 "calc.tab.c"
    break;

  case 79: /* call: basic_operand '(' $@4 csv ')'  */
#line 406 "calc.y"
                                                                        { add_instruction(I_MAKE_CALL); }
#line 2102 "calc.tab.c"
    break;


#line 2106 "calc.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          goto yyexhaustedlab;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if 1
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 415 "calc.y"

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


