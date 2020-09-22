#ifndef OBJECT_H
#define OBJECT_H

namespace UL {

    struct Object;

    enum class Builtins;
    struct ByteCodeFunction;
    struct CppFunction;
    struct Location;

    union ObjectUnion {
        std::string *string_val; //std::string pointer
        double *numerical_val; //pointer to double
        CppFunction *function_val; //pointer to a C++ function
        ByteCodeFunction *bytecode_val; //pointer to a bytecode function, which stores a start val & number of lines
        std::vector<Object> *list_val; //pointer to a vector of objects
    };

    struct Object {
        ObjectUnion union_val;
        Builtins type;
        bool is_weak;
        Location *reference_location;
        Object(double x, bool is_weak=false);
        Object (const char* string, bool is_weak=false);
        Object(CppFunction* cpp_function, bool is_weak=false);
        Object(ByteCodeFunction* bc_function, bool is_weak=false);
        Object(std::vector<Object>* values, bool is_weak=false);
        Object(std::vector<Object> values, bool is_weak=false);
        Object(const Object* from, bool force_weak=false);
        ~Object();
        Object operator *(const Object& arg2);
        void strengthen();
    };
}

std::ostream& operator <<(std::ostream& stream, UL::Object& object);

#endif