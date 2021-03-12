// 2.32 3:30 Mr Anderson Thursday & Friday 
#include "main_setup.cpp"

using namespace UL;

int main(int argc, char** argv) {
	clog << std::boolalpha; // print true & false not 1 & 0
	clog << SEP << SEP;
	{
		using namespace UL;
#include "cache/do.h"
#include "lookup_setup.cpp"
	}
	//#include "attributor.cpp"
	//std::cin.get();
	clog << SEP;
	clog << "\n"
			"┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n"
			"┃ Standard: " << __cplusplus << ", compilation started at: " << __TIME__ << " UTC ┃\n"
			"┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n\n";
	
	try {
		clog << SEP;
		/*
        auto args = make_eo_vec("hello"s, 10);
		print("Okay");
		ExternalObject str1(Classes::string(args));
		std::cin.get();
		args = make_eo_vec(5);
		ExternalObject str2(Classes::string(args));
		args = make_eo_vec(str1, str2);
		print(str1, str2, ExternalObject(Classes::string.get_attr("Add"))(args));
		print("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");
		*/

		fs::current_path(fs::weakly_canonical(fs::path(argv[0])).parent_path()); // Path where file is stored, not called from
		
		const char *file_name = argc == 1 ? "input.ul" : argv[1];

		int generator_exit_status = WEXITSTATUS(std::system(("./parser.out "s + file_name).c_str()));
		print("Status:", generator_exit_status);
		if (generator_exit_status < 3 || generator_exit_status > 125) {
			cout << "Interpreter exited due to bytecode generation failure (exit code" << generator_exit_status << ")\n";
			std::cin.get();
#include "cache/clean.cpp"
			return generator_exit_status;
		}
		std::string status_as_str = std::to_string(generator_exit_status);
		status_as_str = std::string(3 - status_as_str.size(), '0') + status_as_str;
		std::string new_directory = "./bytecode-files/"s + status_as_str;

		fs::current_path(new_directory); // Directory with bytecode files

		std::ifstream input_file("b.bin", std::ios_base::binary);

		Bytecode::scopes.bytecode = decltype(Bytecode::scopes.bytecode)(
			std::istreambuf_iterator<char>(input_file),
			std::istreambuf_iterator<char>()
		);

		input_file.close();

		//print(Bytecode::scopes.bytecode);

		Bytecode::bytes_file_to_word("i.bin", Bytecode::scopes.identifiers);
		Bytecode::bytes_file_to_word("n.bin", Bytecode::scopes.numbers);
		Bytecode::bytes_file_to_word("s.bin", Bytecode::scopes.strings);
		Bytecode::bytes_file_to_word("a.bin", Bytecode::scopes.attrs);

		//print(Bytecode::scopes.identifiers, Bytecode::scopes.numbers, Bytecode::scopes.strings, Bytecode::scopes.attrs);

#if defined(SHOW_BC) || defined(SHOW_CLOG)
#define COUT_CODE(name) cout << #name ": " << Bytecode::scopes.name << "\n";
		cout << "\n";
		COUT_CODE(bytecode)
		COUT_CODE(identifiers)
		COUT_CODE(numbers)
		COUT_CODE(strings)
		COUT_CODE(attrs)
#undef COUT_CODE

#endif

		fs::current_path("..");
		for (const auto& entry : fs::directory_iterator(".")) print(entry.path());
		fs::remove_all("./"s + status_as_str);
		fs::remove("./"s + status_as_str);

		//std::system(rm_cmd.c_str());

		fs::current_path(fs::current_path());
		print("CWD: ", fs::current_path());

		cout << '\n';

		setup_everything = true;
		Bytecode::scopes();
		setup_everything = false;

		//ExternalObject d = MKCSTM("a"s,"b"s,"c"s,"d"s);
		//d.get<Aliases::CustomT>().emplace("e", d);
		//print(d.get<Aliases::CustomT>());

		/*
		Plumber::get();
		ExternalObject d = MKCSTM("a"s,"b"s,"c"s,"d"s);
		//delete (InternalObject<Aliases::ArrayT>*)a.io_ptr;
		ExternalObject a = MKRRY(1,2,3);
		a.get<Aliases::ArrayT>().emplace_back(d);
		d.get<Aliases::CustomT>().emplace("e"s,d);
		d.get<Aliases::CustomT>().emplace("f"s,d);
		print(d.get<Aliases::CustomT>());
		print("Made", d.get<Aliases::CustomT>(), "&", a);
		print(d.ref_count_to<false>(d.io_ptr, 0),"!!");
		print(a.ref_count_to<false>(d.io_ptr, 0),"!!");
		print(a.ref_count_to<false>(a.io_ptr, 0),"!!");
		print(d.refcount(), "!!");
		// 2 1 0 4:
		//d.get<Aliases::CustomT>().clear();
		print(d);
		Plumber::get();
		*/

		/*
		ExternalObject d = MKCSTM();
		d.get<Aliases::CustomT>().emplace("Type", d);
		print(d, d.get<Aliases::CustomT>());
		clog << SEP "EXITED LOCAL SCOPE\n";
		*/

		/*
		ExternalObject o = MKCSTM();
		ExternalObject c = MKCSTM();

		print("S1");
		o.get<Aliases::CustomT>().emplace("Name"s,"o"s);
		c.get<Aliases::CustomT>().emplace("Name"s, "c"s);

		print("S2");
		auto& custom = o.get<Aliases::CustomT>();
		auto array = MKRRY_W(o);
		print("Okay");
		custom.emplace("MRO"s, array);
		print("Okay");
		print(o.get<Aliases::CustomT>(), o.refcount());
		o.get<Aliases::CustomT>().emplace("Type"s, c);

		print("S3");
		c.get<Aliases::CustomT>().emplace("MRO"s, MKRRY_W(c, o));
		c.get<Aliases::CustomT>().emplace("Type"s, c);

		print("S4");
		print(o.get<Aliases::CustomT>(), c.get<Aliases::CustomT>());
		print("EXITED LOCAL SCOPE");
		*/

//#include "tests/functions/variadic_function.cpp"



	} catch (std::runtime_error& e) {
		setup_everything = false;
#include "cache/clean.cpp"
	cout << FG_NEW << "\nProgram exited: " << e.what() << ": " << *Bytecode::scopes.exception << FBG_DEFAULT << "\n";
		std::cin.get();
		return 1;
	}
	Plumber::get();
	clog << SEP;

#include "cache/clean.cpp"
	clog << SEP;
	return 0;
}
#undef MKRRY
#undef MKDCT
