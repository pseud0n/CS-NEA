#ifndef INSTRUCTIONS_ENUMS_H
#define INSTRUCTIONS_ENUMS_H

enum Instructions { // C does not support 'enum class'

	// 1 argument
	I_NOP=0x00,			// 0x00
	
	I_PUSH_ID,			// 0x01
	I_PUSH_ATTR,		// 0x02
	I_PUSH_INT,			// 0x03
	I_PUSH_STR,			// 0x04
                              
	I_OP,				// 0x05
                               
	// 0 arguments            
	I_ASSIGN,			// 0x06
	I_LET,				// 0x07
                               
	I_MARK_CALL,		// 0x08 
	I_MAKE_CALL,		// 0x09 
                              
	I_MARK_LIST,		// 0x0a 
	I_MAKE_LIST,		// 0x0b
	                          
	I_MARK_CODE,		// 0x0c 
	I_MAKE_CODE,		// 0x0d
                              
	I_ADD_BYTE,			// 0x0e 
                              
	I_END_EXP=0xff		// 0xff 
                              
};                            
                              
#endif
