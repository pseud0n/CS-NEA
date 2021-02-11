#include "main_setup.cpp"

using namespace UL;

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
	
	try {
		cout << SEP;

		auto strs = make_eo_vec("Hai, Sensei!"s);

		print(strs[0]);

		print(Classes::object.attrs_of(), strs[0].attrs_of(), Classes::string);
		print("Okay", Classes::string(), "Cool");
		
		cout << SEP "EXITED LOCAL SCOPE\n";
	} catch (std::runtime_error& e) {
		cout << "Program ended: " << e.what() << "\n";
		return -1;
	}

	cout << SEP;

	#include "cache/clean.h"

	cout << SEP "EXITED PROGRAM\n";
	return 0;
}
#undef MKRRY
#undef MKDCT