#ifndef DECL_DISPLAY_H
#define DECL_DISPLAY_H

//template <const char* Inclusion>
struct _Say {
	_Say(const char *header) {
		clog << "**" << header << "**\n";
	}	
};

#define SAY(name) _Say _(#name);

#endif
