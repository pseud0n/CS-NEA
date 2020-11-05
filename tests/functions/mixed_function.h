#ifndef MIXED_FUNCTION_H
#define MIXED_FUNCTION_H

TEST_REPR("Create and call varied function function");

auto get_attr = new UL::CppFunction({}, false, DY_LMBD {
    const UL::Object *obj; // Don't delete or modify!
    std::string string;
    //First argument: Object, second argument: string
    if (!argument_data->assign_args<2>(arguments, obj, string)) {
        UL::ArgExc("Function takes object and string");
        return UL::null_optr;
    }
    cout << "OBJ " << *obj << " STR " << string << "\n";
    return OPTR(std::string("yo yo yo"));
});

auto a1 = new UL::Object(5.6);
auto a2 = new UL::Object(std::string("Hello There"));

cout << "---------\n";
cout << (*get_attr)({a1, a2}) << "\n";

delete a1;
delete a2;

#endif
