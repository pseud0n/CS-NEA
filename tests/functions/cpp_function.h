#ifndef TESTS_CPP_FUNCTION_H_
#define TESTS_CPP_FUNCTION_H_


TEST_REPR("Create and call variadic function");

UL::Tracker::repr();

auto a = new UL::Object();

a->attrs.try_emplace("f", new UL::Object(new UL::CppFunction({new UL::Object(5), new UL::Object(6)}, false, DY_LMBD {
    int v, w, x, y, z;
    if (!argument_data->assign_args<3>(arguments, v, w, x, y, z)) {
        //cout << repr_arg_error(3, 2, false, arguments.size()) << "\n";
        return UL::null_optr;
    }
    print(v, w, x, y, z);
    return OPTR(999);
})));

//print("?", i_func ? "callable" : "not callable");
//print("?", bool(a->attrs["f"]->union_val.function_val->function) ? "callable" : "not callable");
//print(a->attrs["f"]->union_val.function_val->function);


OPTR a1(13), a2(14), a3(15), a4(16);
std::vector<UL::Object*> args {a1, a2, a3, a4}; // Implcitly converted
print(args);

print("1");
print(*a->attrs["f"]);

cout << (*a->attrs["f"])(args) << "\n";
#endif
