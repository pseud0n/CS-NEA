#ifndef PRINTER_H_
#define PRINTER_H_

//void print_box(std::string msg) cout << '┏' <<  << "┓\n┃"  "┃\n┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n\n";

#define SEP "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"

#define OSTREAM_HEADER(type, name) std::ostream& operator <<(std::ostream& stream, type name)

#define ITER_OVERLOAD(type)                                                                                              \
    template <typename  Ts>                                                                                        \
    OSTREAM_HEADER(const type<Ts>&, object) {                                                                \
        stream << "{";                                                                                                  \
        for (auto it = object.begin(); it != object.end(); ++it) { /* auto: std::type<ElementT>::iterator */            \
            if (it != object.begin()) stream << ", ";                                                                   \
            stream << *it;                                                                                              \
        }                                                                                                               \
        return stream << "}";                                                                                           \
    }

template <typename K, typename V>
std::ostream& operator <<(std::ostream& stream, const std::pair<K, V>& p) {
    return stream << p.first << ": " << p.second;
}

ITER_OVERLOAD(std::vector)
//ITER_OVERLOAD(const std::vector)
ITER_OVERLOAD(std::unordered_set)
//ITER_OVERLOAD(std::unordered_map)

template <typename T>
std::string repr_deref(std::vector<T> v) {
    std::stringstream sstream;
    sstream << "{";                                                                                                  \
        for (auto it = v.begin(); it != v.end(); ++it) { /* auto: std::type<ElementT>::iterator */            \
            if (it != v.begin()) sstream << ", ";                                                                   \
            sstream << **it;                                                                                              \
        }                                                                                                               \
    sstream << "}";    
    return sstream.str();
}

template <typename Arg, typename... Args>
void print(Arg&& arg, Args&&... args) {
    cout << std::forward<Arg>(arg);
    ((cout << ' ' << std::forward<Args>(args)), ...);
    cout << '\n';
}

void print() {
    cout << '\n';
}


std::string repr_arg_error(size_t min, size_t optional, bool is_variadic, size_t entered) {
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
        /*
        user_defined_object.get_attribute("string", (std::string)[&user_defined_object](){
            std::stringstream stream;
            stream << "<UDO " << &user_defined_object << ">";
            return stream.str();
        }());
        */
    }

    OSTREAM_HEADER(const Object&, object) {
        if (object.union_val.numerical_val == NULL) // Since all pointers are stored in the same place, it doesn't matter which pointer is chosen
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
            case Types::user_defined_object:
                return stream << *object.union_val.udo_val;
            case Types::list:
            {
                std::stringstream sstream;
                sstream << "{";
                for (auto it = object.union_val.list_val->begin(); it != object.union_val.list_val->end(); ++it) {
                    if (it != object.union_val.list_val->begin()) sstream << ", ";
                    sstream << *it; // Recursion! (stringstream is subclass of ostream)
                }
                sstream << "}";
                return stream << sstream.str();
            }
            default:
            	return stream; // Fallback; should not be visited under expected circumstances
        }
    }

    #define TP_CASE(type) case Types::type: return stream << #type;

    OSTREAM_HEADER(Types, type) {
        switch(type) {
            TP_CASE(null)
            TP_CASE(number)
            TP_CASE(string)
            TP_CASE(cpp_function)
            TP_CASE(bytecode_function)
            TP_CASE(list)
            TP_CASE(user_defined_object)
        	default:
        		return stream;
        }
    }

    OSTREAM_HEADER(OPTR, optr) {
    	return stream << "OPTR " << *optr.object_ptr;
    }

    const char* r_optr_strength(const OPTR& optr) {
        return optr.is_weak ? "weak" : "strong";
    }

    const char* r_optr_constness(Object* object) {
        return object->is_const ? "const" : "non-const";
    }

}


#undef TP_CASE
#undef ITER_OVERLOAD
#undef OSTREAM_HEADER

#endif
