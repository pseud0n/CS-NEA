////////////////////////////////////////////////////////////////////////////////
#ifndef PRINTER_H
#define PRINTER_H

#include <ostream>
#include <type_traits>

#define SEP "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"

#define OSTREAM_HEADER(type, name) std::ostream& operator <<(std::ostream& stream, type name)

#define ITER_OVERLOAD(type)																							\
	template <typename...  Ts>																							\
	OSTREAM_HEADER(const type<Ts...>&, object) {																		\
		stream << "{";																								\
		for (auto it = object.begin(); it != object.end(); ++it) { /* auto: std::type<ElementT>::iterator */		\
			if (it != object.begin()) stream << ", ";																\
			stream << *it;																							\
		}																											\
		return stream << "}";																						\
	}

template <typename K, typename V>
std::ostream& operator <<(std::ostream& stream, const std::pair<K, V>& p) {
	return stream << p.first << ": " << p.second;
}

std::ostream& operator <<(std::ostream& stream, std::nullptr_t) {
	return stream << "nullptr\n";
}

ITER_OVERLOAD(std::vector)
ITER_OVERLOAD(std::unordered_set)
ITER_OVERLOAD(std::unordered_map)
ITER_OVERLOAD(std::initializer_list)

template <typename T>
std::string repr_deref(std::vector<T> v) {
	std::stringstream sstream;
	sstream << '{';											\
		for (auto it = v.begin(); it != v.end(); ++it) { 	\
			if (it != v.begin()) sstream << ", ";			\
			sstream << **it;								\
		}													\
	sstream << '}';	
	return sstream.str();
}

template <typename Arg, typename... Args>
void print(const Arg& arg, const Args&... args) {
	cout << arg;
	((cout << ' ' << args), ...);
	cout << '\n';
}

void print() {
	cout << '\n';
}


namespace UL {
	#define TP_CASE(type) case Types::type: return stream << #type;

	OSTREAM_HEADER(Types, type) {
		switch(type) {
			TP_CASE(any)
			TP_CASE(blank)
			TP_CASE(null)
			TP_CASE(number)
			TP_CASE(string)
			TP_CASE(boolean)
			TP_CASE(cpp_function)
			TP_CASE(cpp_function_view)
			TP_CASE(bytecode_function)
			TP_CASE(list)
			TP_CASE(array)
			TP_CASE(pair)
			TP_CASE(dictionary)
			TP_CASE(custom)
			default:
				return stream << "?";
		}
	}

	#undef TP_CASE
	
	std::string no_matching_ctor(Types type_to_construct, std::vector<Types>& entered_types) {
		std::stringstream stream;
		stream << "Entered types " << "entered_types" << ", no valid constructor found to construct " << type_to_construct;
		return stream.str();
	}
	std::string repr_arg_count_error(size_t min, size_t optional, bool is_variadic, size_t entered) {
		std::stringstream error;
		if (entered < min || (!is_variadic && entered > min + optional)) {
			error << "Entered " << entered << " arguments but function requires ";
			if (is_variadic) error << " at least " << min << " arguments";
			else if (optional == 0) error << "exactly " << min << " argument" << (min == 1 ? "":"s");
			else error << min << " to " << optional + min;
		}
		return error.str();
	}

	std::string repr_arg_type_error(const CppFunction& cpp_function, std::vector<ExternalObject>& arguments) {
		std::stringstream error;
		//print("here");
		if (cpp_function.has_type_requirement) {
			/*// For extra debug:
			std::vector<Types> v(arguments.size());
			std::transform(
				arguments.begin(), arguments.end(), v.begin(),
				[](const ExternalObject& o) -> Types { return o.type(); }
			);
			print("has type requirement", arguments, v, cpp_function.required_types, cpp_function.required_types.size());
			*/
			for (size_t i = 0; i < cpp_function.required_types.size(); ++i) {
				if (cpp_function.required_types[i] != Types::any && cpp_function.required_types[i] != arguments[i].type()) {
					//print("Error!");
					error
						<< "For argument " << i
						<< ": acceptable type: " << cpp_function.required_types[i] 
						<< ", got type: " << arguments[i].type();
					break;
				}
			}
		}
		//print("Got to end");
		return error.str();
	}
	
	OSTREAM_HEADER(const ExternalObject&, eobject) {
		if (!eobject.io_ptr)
			return stream  << "Null";
		switch (eobject.type()) {
			case Types::null:
				break;
			case Types::number:
				return stream << eobject.get<Aliases::NumT>();
			case Types::boolean:
				return stream << eobject.get<Aliases::BoolT>();
			case Types::string:
				return stream << '\'' << eobject.get<Aliases::StringT>() << '\'';
			case Types::cpp_function:
				return stream << "<func@" << eobject.io_ptr << ">";
			case Types::cpp_function_view:
				return stream << "<func view@" << eobject.io_ptr << ">";
			case Types::bytecode_function:
				return stream << "<Bytecode Function>";
			case Types::pair:
			{
				auto& pair = eobject.get<Aliases::PairT>();
				return stream << pair.first << ":" << pair.second;
			}
				//eobject.get<Aliases::PairT>();
				// Since overload for std::pair already made
			case Types::array:
				/*
				{
				std::stringstream sstream;
				sstream << "[";
				for (auto it = eobject.get<TypeAliases::ListT>().begin(); it != eobject.get<TypeAliases::ListT>().end(); ++it) {
					if (it != eobject.get<TypeAliases::ListT>().begin()) sstream << ", ";
					sstream << *it;
				}
				sstream << "]";
				return stream << sstream.str();
				}
				*/
			
			/*
			{
				stream << "[";
				auto& array_ref = eobject.get<Aliases::ArrayT>();
				for (auto it = array_ref.begin(); it != array_ref.end(); ++it) {
					if (it != array_ref.begin()) stream << ", ";
					stream << *it;
				}
				stream << "]";
				return stream;
			}
			*/
				return stream << eobject.get<Aliases::ArrayT>();
			case Types::list:
			/*
				{
				std::stringstream sstream;
				sstream << "List[";
				for (auto it = eobject.get<Aliases::ListT>().begin(); it != eobject.get<TypeAliases::ListT>().end(); ++it) {
					if (it != eobject.get<TypeAliases::ListT>().begin()) sstream << ", ";
					sstream << *it;
				}
				sstream << "]";
				return stream << sstream.str();
				}
			*/
				return stream << "It's a list";
			case Types::dictionary:
			{
				stream << "Dict[";
				auto& dict_ref = eobject.get<Aliases::DictT>();
				for (auto it = dict_ref.begin(); it != dict_ref.end(); ++it) {
					if (it != dict_ref.begin()) stream << ", ";
					stream << it->first << ':' << it->second; // print each pair
				}
				stream << "]";
				return stream;
			}
			case Types::custom:
				stream << "Object@" << eobject.io_ptr;
				//2stream.operator<< <Aliases::CustomT>(eobject.get<Aliases::CustomT>());
				return stream;
			default:
				break;
		}
		return stream << "<HELP>";
	}
} // UL

#ifdef STANDALONE_BIG_DEC_H

OSTREAM_HEADER(const BigDec&, dec) {
	// Function is friend to BigDec
	/*
	"1"(3) 	-> 0.001 	(1->2) ✔
	"12" 	-> 0.012 	(2->1) ✔
	"123"	-> 0.123	(3->0) ✔
	"1234"	-> 1.234	(4->0)
	"12345"	-> 12.345	(5->0)
	*/
	std::string repr = dec.value.str();
	//return stream << repr;
	//for (size_t i = 0; i < repr.length() - BigDec::precision);
	//print("Formatting", repr, "with precision", BigDec::precision);
	bool is_negative;
	if (repr[0] == '-') {
		is_negative = true;
		repr.erase(repr.begin());
		stream << '-';
	} 
	size_t diff = repr.length() - BigDec::precision;
	if (repr == "0") return stream << "0.0";
	if (repr.length() <= BigDec::precision) {
		stream << "0." << std::string(BigDec::precision - repr.length(), '0') << repr;
	} else {
		print("Long", repr.length(), BigDec::precision);
		stream << repr.substr(0, diff) << '.' << repr.substr(diff);
	}
	return stream;
}

#endif // For STANDALONE_BIG_DEC_H

OSTREAM_HEADER(const bmp::cpp_int, num) {
	return stream << "<" << num.str() << ">\n";
}

#undef ITER_OVERLOAD
#undef OSTREAM_HEADER

#endif // For PRINTER_H
