////////////////////////////////////////////////////////////////////////////////
#ifndef PRINTER_H
#define PRINTER_H

#include <iostream>
#include <ostream>
#include <type_traits>
#include "forward_decl.h"
#include <vector>
#include <variant>
#include <memory>

using std::clog;

#define SEP "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"

#define OSTREAM_HEADER(type, name) std::ostream& operator <<(std::ostream& stream, type name)

namespace UL {
#define TP_CASE(type) case Types::type: return stream << #type;
	OSTREAM_HEADER(Types, type) {
		switch(type) {
			TP_CASE(null_type)
			TP_CASE(number)
			TP_CASE(string)
			TP_CASE(boolean)
			TP_CASE(cpp_function)
			TP_CASE(cpp_function_view)
			TP_CASE(list)
			TP_CASE(array)
			TP_CASE(pair)
			TP_CASE(dictionary)
			TP_CASE(base_exception);
			TP_CASE(custom)
			TP_CASE(code_block)
			TP_CASE(any)
			default:
				return stream << "?";
		}
	}
#undef TP_CASE

}

template <typename... Ts>
OSTREAM_HEADER(std::variant<Ts...>, variant) {
	if (variant.index() == 0)
		return stream << "Variant:0";
	return stream << "Variant:1:" << std::get<1>(variant);
	//return stream << "Variant:" << variant.index();
}

std::string char_hex(unsigned char c) {
	std::ostringstream stream;
	std::ios_base::fmtflags initial_flags(stream.flags());
	stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c);
	stream.flags(initial_flags);
	return stream.str();
}

OSTREAM_HEADER(const std::vector<unsigned char>&, u_chars) {
		clog << "HELLO\n";
		stream << "{";
		for (auto it = u_chars.begin(); it != u_chars.end(); ++it) {
			if (it != u_chars.begin()) stream << " ";
			stream << std::hex << std::setfill('0') << std::setw(2) << (0xff & static_cast<int>(*it));	
		}
		stream.flags(basic_flags);
		return stream << "}";					
}

#define ITER_OVERLOAD(type)																							\
	template <typename...  Ts>																						\
	OSTREAM_HEADER(const type<Ts...>&, object) {																	\
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
/*
std::ostream& operator <<(std::ostream& stream, std::nullptr_t) {
	return stream << "nullptr\n";
}
*/

ITER_OVERLOAD(std::vector)
ITER_OVERLOAD(std::unordered_set)
ITER_OVERLOAD(std::unordered_map)
ITER_OVERLOAD(std::initializer_list)
ITER_OVERLOAD(std::set)

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
	clog << arg;
	((clog << ' ' << args), ...);
	clog << '\n';
}

void print() {
	clog << '\n';
}


namespace UL {
	std::ostream& operator<<(std::ostream&, const ExternalObject&);

	std::string no_matching_ctor(Types type_to_construct, std::vector<Types>& entered_types) {
		std::stringstream stream;
		stream << "Entered types (";
		for (size_t i = 0; i < entered_types.size(); ++i) {
			if (i != 0)	stream << ", ";
			stream << entered_types[i];
		}
		stream  << "), no valid constructor found to construct " << type_to_construct;
		return stream.str();
	}
	std::string repr_arg_count_error(size_t min, size_t optional, bool is_variadic, size_t entered) {
		std::stringstream error;
		if (entered < min || (!is_variadic && entered > min + optional)) {
			error << "Entered " << entered << " argument" << (entered == 1 ? "" : "s");
			error << " but function requires ";
			if (is_variadic) error << " at least " << min << " arguments";
			else if (optional == 0) error << "exactly " << min << " argument" << (min == 1 ? "":"s");
			else error << min << " to " << optional + min;
			error << "!!!!";
		}
		return error.str();
	}

	std::ostream& show_arg_error(std::ostream& error, size_t index, Types required_type, const ExternalObject& object) {
		error
			<< "For argument " << index << " (" << object <<  ") "
			<< ": acceptable type: " << *enum_to_class[required_type]
			<< ", got type: " << *enum_to_class[object.type()];
		return error;
	}

	std::ostream& show_simple_type_error(std::ostream& error, const std::vector<ExternalObject>& arguments) {
		error << "Could not call function from arguments [";
		for (size_t i = 0; i < arguments.size(); ++i) {
			if (i != 0)	error << ", ";
			error << arguments[i];
		}
		error << "]";
		return error;
	}

	std::string str_simple_type_error(const std::vector<ExternalObject>& arguments) {
		std::ostringstream oss;
		show_simple_type_error(oss, arguments);
		return oss.str();
	}

	std::string repr_arg_type_error(const std::vector<Types>& required_types, const std::vector<ExternalObject>& arguments) {
		std::ostringstream error;
		for (size_t i = 0; i < required_types.size(); ++i) {
			if (required_types[i] != Types::any && required_types[i] != arguments[i].type()) {
				show_arg_error(error, i, required_types[i], arguments[i]);
				break;
			}
		}
		return error.str();
	}
	
	std::ostream& repr_for_print(std::ostream& stream, const ExternalObject& eobject) {
		if (eobject.type() == Types::string) {
			return stream << eobject.get<Aliases::StringT>();
		}
		return stream << eobject;
	}

	OSTREAM_HEADER(const ExternalObject&, eobject) {
		//stream << (eobject.is_weak ? "[w]":"[s]");
		//stream << "[" << eobject.type() << "]";
		if (!eobject.io_ptr)
			return stream  << "Null";
		Types eo_type = setup_everything ? eobject.get_enum_type() : eobject.type();
		// A custom type my inherit from a builtin type, e.g. most exceptions
		switch (eo_type) {
			case Types::null_type:
				break;
			case Types::number:
				return stream << eobject.get<Aliases::NumT>();
			case Types::boolean:
				return stream << (eobject.get<Aliases::BoolT>() ? "True" : "False");
			case Types::string:
				return stream << '"' << eobject.get<Aliases::StringT>() << '"';
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
				return stream << eobject.get<Aliases::ArrayT>();
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
			case Types::base_exception:
				return repr_for_print(stream, eobject.get<Aliases::BaseExceptionT>().type) << "(" << eobject.get<Aliases::BaseExceptionT>().message << ")";
				//return stream << "BE";
			case Types::code_block:
				//return stream << "{" << eobject.io_ptr << ":"<< eobject.get<Aliases::CodeBlockT>().start_location << "}";
				return stream << eobject.get<Aliases::CodeBlockT>();
			case Types::if_chain:
			{
				const ExternalObject& code_block = eobject.get<Aliases::IfT>().code_block_ref;
				return stream << "If:";
				if (code_block.is_null())
					return stream << "Null";
				return stream << code_block;
			}
			case Types::custom:
			{
				const Aliases::CustomT& custom = eobject.get<Aliases::CustomT>();
				Aliases::CustomT::const_iterator name_it = custom.find("Name");
				if (name_it == custom.cend()) {
					// Does not have a name. Now look for ToString method
					//void *to_string_method = eobject.get_attr_no_throw("ToString");
					//if (to_string_method) {
						//stream << ExternalObject(to_string_method)();
					//} else {
						stream << "Object@" << eobject.io_ptr;
					//}
				}
				else {
					stream << "Class " << name_it->second.get<Aliases::StringT>(); 
				}
				return stream;
			}
			default:
				break;
		}
		return stream << "<UNDEFINED " << ">";
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

/*
OSTREAM_HEADER(const bmp::cpp_int, num) {
	return stream << "<" << num.str() << ">";
}
*/

#undef ITER_OVERLOAD
#undef OSTREAM_HEADER

#endif // For PRINTER_H
