#ifndef DECL_DISPLAY_H
#define DECL_DISPLAY_H

//template <const char* Inclusion>
struct Included {
	Included(const char *header) {
		cout << "Included " << header << "\n";
	}	
};

#define INCLUDED(name) Included _(#name);

/*
#define INCLUDE(name)							\
	#ifndef DECL_DISPLAY_H						\
	#define DECL_DISPLAY_H						\
	cout << "Included " << name << "\n";		
*/
#endif