#ifndef PRINTER_H
#define PRINTER_H

#define OSTREAM_HEADER(type, name) std::ostream& operator <<(std::ostream& stream, type name)


#define STD_OVERLOAD(type)                                                                                              \
    template <typename ElementT>                                                                                        \
    OSTREAM_HEADER(const std::type<ElementT>&, object) {                                                                \
        stream << "{";                                                                                                  \
        for (auto it = object.begin(); it != object.end(); ++it) { /* auto: std::type<ElementT>::iterator */            \
            if (it != object.begin()) stream << ", ";                                                                   \
            stream << *it;                                                                                              \
        }                                                                                                               \
        return stream << "}";                                                                                           \
    }

STD_OVERLOAD(vector)
STD_OVERLOAD(unordered_set)

namespace UL {
    OSTREAM_HEADER(const UserDefinedObject&, user_defined_object) {
        //return stream << user_defined_object.get_attribute<std::string>("string", "<No available repr>");
        //return stream << user_defined_object.get_attribute("string", "<No available repr>");
        return stream << "<A string>";
        /*
        user_defined_object.get_attribute("string", (std::string)[&user_defined_object](){
            std::stringstream stream;
            stream << "<UDO " << &user_defined_object << ">";
            return stream.str();
        }());
        */
    }

    OSTREAM_HEADER(const Object&, object) {
        if (object.union_val.numerical_val == 0) //Since all pointers are stored in the same place, it doesn't matter which pointer is chosen
            return stream << "<void object>";
        switch (object.type) {
            case Types::null:
                return stream << "<None>";
            case Types::number:
                return stream << *object.union_val.numerical_val;
            case Types::string:
                return stream << "\"" << *object.union_val.string_val << "\"";
            case Types::cpp_function:
                return stream << "<Cpp Function " << &object << ">";
                break;
            case Types::bytecode_function:
                return stream << "<BC Function " << &object << ">";
                break;
            case Types::user_defined_object:
                return stream << *object.union_val.udo_val;
                break;
            case Types::list:
                std::stringstream sstream;
                sstream << "{";
                for (auto it = object.union_val.list_val->begin(); it != object.union_val.list_val->end(); ++it) {
                    if (it != object.union_val.list_val->begin()) sstream << ", ";
                    sstream << *it; // Recursion! (stringstream is subclass of ostream)
                }
                sstream << "}";
                return stream << sstream.str();
                break;
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
        }
    }

}

#undef TP_CASE
#undef STD_OVERLOAD
#undef OSTREAM_HEADER

#endif