////////////////////////////////////////////////////////////////////////////////
#ifndef PRINTER_H
#define PRINTER_H

#include <ostream>
#include <type_traits>

#define SEP "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"

#define OSTREAM_HEADER(type, name) std::ostream& operator <<(std::ostream& stream, type name)

#define ITER_OVERLOAD(type)																							\
	template <typename  Ts>																							\
	OSTREAM_HEADER(const type<Ts>&, object) {																		\
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
//ITER_OVERLOAD(const std::vector)
ITER_OVERLOAD(std::unordered_set)
//ITER_OVERLOAD(std::unordered_map)
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


std::string repr_arg_count_error(size_t min, size_t optional, bool is_variadic, size_t entered) {
	std::stringstream error;
	if (entered < min || (!is_variadic && entered > min + optional)) {
		error << "Entered " << entered << " arguments but function requires ";
		if (is_variadic) error << " at least " << min << " arguments";
		else if (optional == min) error << "exactly " << min << " arguments";
		else error << min << " to " << optional + min;
	}
	return error.str();
}


namespace UL {

	OSTREAM_HEADER(const UserDefinedObject&, user_defined_object) {
		return stream << user_defined_object.get_attribute<std::string>("string", "<No available repr>");
		//return stream << user_defined_object.get_attribute("string", "<No available repr>");
		//return stream << "<A string>";
		;
		/*
		user_defined_object.get_attribute("string", (std::string)[&user_defined_object](){
			std::stringstream stream;
			stream << "<UDO " << &user_defined_object << ">";
			return stream.str();
		}());
		*/
	}

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

	OSTREAM_HEADER(const Object&, object) {


		if ((void*)object.union_val.numerical_val == 0) // Since all pointers are stored in the same place, it doesn't matter which pointer is chosen
			if (object.type != Types::null) return stream << "<Void>";
		switch (object.type) {
			case Types::null:
				return stream << "<Null>";
			case Types::number:
				return stream << *object.union_val.numerical_val;
			case Types::string:
				return stream << "\"" << *object.union_val.string_val << "\"";
			case Types::cpp_function:
				return stream << "<Cpp Function " << &object << ">";
			case Types::bytecode_function:
				return stream << "<BC Function " << &object << ">";
			case Types::custom:
				return stream << *object.union_val.udo_val;
			case Types::list:
			{ // Since cases are labels, local variable sstream must exist in own scope
				std::stringstream sstream;
				sstream << "[";
				for (auto it = object.union_val.vector_val->begin(); it != object.union_val.vector_val->end(); ++it) {
					if (it != object.union_val.vector_val->begin()) sstream << ", ";
					sstream << *it->object_ptr;
				}
				sstream << "]";
				return stream << sstream.str();
			}
			case Types::array:
				return stream << "ARRAY";
			default:
				return stream; // Fallback; should not be visited under expected circumstances
		}
	}


	OSTREAM_HEADER(OPTR, optr) {
		return stream << "OPTR(" << *optr.object_ptr << ")";
	}

	const char* r_optr_strength(const OPTR& optr) {
		return optr.is_weak ? "weak" : "strong";
	}

	const char* r_optr_constness(Object* object) {
		return object->is_const ? "const" : "non-const";
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
				return stream << "<C++ Function>";
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
				stream << "Object";
				stream.operator<< <Aliases::CustomT>(eobject.get<Aliases::CustomT>());
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
