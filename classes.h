#ifndef CLASSES_H
#define CLASSES_H
//Included<"classes">();

/*
A builtin type is interfaced through an OPTR and can be treated as one.
When creating a user-defined type, a built-in class can be inherited from.
It will have to be treated as a UL object in that instance.
Built-in types inheriting from other types will inherit from other BuiltinClass objects
*/

//types: null, number, string, cpp_function, bytecode_function, list, user_defined_object

/*
When a normal function is called, it will pass in the exact arguments entered.
If we call, for example
    string.lower()
it is equivalent to
    String.lower(string)
As such, the String object implements a function which takes a single argument.
When the method is called of the instance, the value of self is passed in implicitly.
This is because the method is first found in the String type.
*/

#define BLANK_RETURN_FN(return_val) new UL::Object(new UL::CppFunction({ }, false, DY_LMBD { return OPTR(new UL::Object(return_val, true)); }), true)

#define EMPLACE_KV(name, key, value) UL::Classes::name.attributes.try_emplace(key, value);
//Adds a attribute (name (std::string) and value (UL::Object*)) to 
#define EMPLACE_KV_BF(name, key, value) EMPLACE_KV(name, key, BLANK_RETURN_FN(value))
#define EMPLACE_KV_BM(name, key, value) EMPLACE_KV(name, key, BLANK_RETURN_FN(value))

//EMPLACE_KV_BF(object, "type", "<Object>")

//EMPLACE_KV_BF(null, "type", "<Null>")

//namespace Classes {
    // The functions should be weak references since they have no reason to be tracked
    //std::unordered_map<std::string, OPTR> m;
    //m.try_emplace("type", BLANK_RETURN_FN("Object") );
    //BuiltinClass object({ }); // No super classes - this is at the top of the hierarchy   
    //UL::Classes::object.emplace("type", BLANK_RETURN_FN("<Object>"));
    //UL::Classes::object.attributes.try_emplace("type", BLANK_RETURN_FN("<Object>"));
    
    /*
    BuiltinClass null(
        { &object },
        {
            { "type", BLANK_RETURN_FN("Number") }
        }
    );

    BuiltinClass number(
        { &object },
        {
            { "type", BLANK_RETURN_FN("NullType") }
        }
    );

    BuiltinClass string(
        { &object },
        {
            { "type", BLANK_RETURN_FN("String") }
        }
    );
    */
//}

#undef BLANK_RETURN_FN
#undef EMPLACE_KV
#undef EMPLACE_KV_BF
#undef EMPLACE_KV_BM

#endif