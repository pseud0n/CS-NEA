#define CHAR_SIZE sizeof(char)
#define DIR_NAME "bytecode-files"
#define DIR_PRELUDE "."
#define DIR_PRELUDE_LEN (sizeof(DIR_PRELUDE) / sizeof(char) - 1)
#define DIR_FULL DIR_PRELUDE "/" DIR_NAME
// e.g. "./bytecode-files"
#define DIR_LEN (sizeof(DIR_FULL) / sizeof(char) - 1)
#define DIR_LEN_FULL DIR_LEN + 1 + 3
// 1 for forward slash, 10 for name
// e.g.length of  "./bytecode-files/0123456789"
#define MAKE_FILE_PTR(name) FILE *name##_file_ptr = NULL, *name##_read_file_ptr = NULL;
