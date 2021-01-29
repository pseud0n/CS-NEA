#include "main_setup.h"


#define MKRRY UL::ExternalObject::make_array
#define MKDCT UL::ExternalObject::make_dict
int main() {
	cout << std::boolalpha; // print true & false not 1 & 0
	cout << SEP << SEP;
	{
		using namespace UL;
		#include "cache/do.h"
		#include "lookup_setup.cpp"
	}
	//#include "attributor.cpp"
	cout << SEP;
	cout << "\n"
			"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n"
			"┃ Standard: " << __cplusplus << ", compilation started at: " << __TIME__ << " UTC ┃\n"
			"┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n\n";
	
	{
		cout << SEP;

		//#include "tests/objects.h"
		print("Hai, Sensei!");

		UL::ExternalObject str("sArCaSm!"s);

		/*
		UL::ExternalObject attr = str.get_attr("length");
		print("-----------");
		print(attr);
		*/

		using namespace UL;
		
		ExternalObject func = make_monadic_method<Aliases::StringT, Aliases::StringT>(
			[](Aliases::StringT& str) -> Aliases::StringT {
				std::for_each(
					str.begin(), str.end(),
					[](char& c) { c = std::toupper(c); }
				);
				return str;
			}
		);

		print("noice");
				
		cout << SEP "EXITED LOCAL SCOPE\n";
	}

	cout << SEP;

	#include "cache/clean.h"

	cout << SEP "EXITED PROGRAM\n";
	return 0;
}
#undef MKRRY
#undef MKDCT