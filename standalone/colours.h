#ifndef FBG_DEFAULT

/*
Name            FG  BG
-----------------------
Black           30  40
Red             31  41
Green           32  42
Yellow          33  43
Blue            34  44
Magenta         35  45
Cyan            36  46
White           37  47
Bright Black    90  100
Bright Red      91  101
Bright Green    92  102
Bright Yellow   93  103
Bright Blue     94  104
Bright Magenta  95  105
Bright Cyan     96  106
Bright White    97  107
*/

#define MAKE_COL(n) 	"\x1B[" #n "m"

#define FBG_DEFAULT 	MAKE_COL(0)
#define FG_NEW			MAKE_COL(91)
#define FG_DELETE 		MAKE_COL(92)
#define BG_WARNING		MAKE_COL(101)
#define FG_GENERIC		MAKE_COL(93)


#endif