#ifndef PRINTER_H
#define PRINTER_H

#define HEADER(type, name) std::ostream& operator <<(std::ostream& stream, type name)


#define STD_OVERLOAD(type)                                                                                              \
    template <typename ElementT>                                                                                        \
    HEADER(const std::type<ElementT>&, object) {                                                                      \
        stream << "{";                                                                                                  \
        for (auto it = object.begin(); it != object.end(); ++it) { /* auto: std::type<ElementT>::iterator */          \
            if (it != object.begin()) stream << ", ";                                                                   \
            stream << *it;                                                                                              \
        }                                                                                                               \
        return stream << "}";                                                                                           \
    }

STD_OVERLOAD(vector)
STD_OVERLOAD(unordered_set)

namespace UL {
    HEADER(const Object&, object) {
        if (object.union_val.numerical_val == 0) //Since all pointers are stored in the same place, it doesn't matter which pointer is chosen
            return stream << "<void object>";
        switch (object.type) {
            case Builtins::null:
                return stream << "<None>";
            case Builtins::number:
                return stream << *object.union_val.numerical_val;
            case Builtins::string:
                return stream << "\"" << *object.union_val.string_val << "\"";
            case Builtins::cpp_function:
                return stream << "<Cpp Function " << &object << ">";
                break;
            case Builtins::bytecode_function:
                return stream << "<BC Function " << &object << ">";
                break;
            case Builtins::list:
                std::stringstream sstream;
                sstream << "{";
                for (auto it = object.union_val.list_val->begin(); it != object.union_val.list_val->end(); ++it) {
                    if (it != object.union_val.list_val->begin()) sstream << ", ";
                    sstream << *it; //Recursion!
                }
                sstream << "}";
                return stream << sstream.str();
                break;
        }
    }

    #define BI_CASE(type) case Builtins::type: return stream << #type;

    HEADER(Builtins, type) {
        switch(type) {
            BI_CASE(null)
            BI_CASE(number)
            BI_CASE(string)
            BI_CASE(cpp_function)
            BI_CASE(bytecode_function)
            BI_CASE(list)
        }
    }

}

#undef BI_CASE
#undef STD_OVERLOAD
#undef HEADER

#endif