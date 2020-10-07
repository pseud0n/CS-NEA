TEST_REPR("Create and call variadic function");

auto func = new UL::Object(new UL::CppFunction({OBJ_RET(10), OBJ_RET(11), OBJ_RET(33)}, true, DY_LMBD {
    //arguments: std::vector<UL::Object*>
    int a, b, c, d, e, f;
    std::vector<std::string> extra_args;
    //cout << typeid(arguments).name() << "\n";
    if(!argument_data->assign_args<3>(arguments, &extra_args, &a, &b, &c, &d, &e, &f)) {
        UL::ArgExc("Function takes 6+ integrals")();
        return new UL::Object(nullptr);
    }
    cout << "a-f: " << a << " " << b << " " << c << " " << d << " " << e << " " << f << "\n";
    cout << extra_args << "\n";
    return new UL::Object(a + b + c + d + e + f);
}));

auto arg1 = new UL::Object(8);
auto arg2 = new UL::Object(3);
auto arg3 = new UL::Object(2);
auto arg4 = new UL::Object(1);
auto arg5 = new UL::Object(11);
auto arg6 = new UL::Object(100);
auto arg7 = new UL::Object("hello");
auto arg8 = new UL::Object("there");
auto arg9 = new UL::Object(5);
cout << "Construction complete\n";

auto out = (*func)({arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9});
cout << *out->union_val.numerical_val << "\n";

//cout << UL::Location::show_instances() << "\n";

delete arg1;
delete arg2;
delete arg3;
delete arg4;
delete arg5;
delete arg6;
delete arg7;
delete arg8;
delete arg9;

delete func;
delete out;

cout << UL::Location::show_instances() << "\n";