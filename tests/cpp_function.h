#ifndef TESTS_CPP_FUNCTION_H_
#define TESTS_CPP_FUNCTION_H_


TEST_REPR("Create and call variadic function");

OPTR func = OPTR(new UL::CppFunction({10, 11, 33}, true, DY_LMBD {
    // arguments: std::vector<UL::Object*>
    int a, b, c, d, e, f;
    std::vector<std::string> extra_args;
    if(!argument_data->assign_args<3>(arguments, &extra_args, a, b, c, d, e, f)) {
        UL::ArgExc("Function takes 6+ integrals")();
        return OPTR(nullptr);
    }
    cout << "a-f: " << a << " " << b << " " << c << " " << d << " " << e << " " << f << "\n";
    cout << extra_args << "\n";
    return OPTR(a + b + c + d + e + f);
}));

std::vector<OPTR> arguments = {8, 3, 2, 1, 11, 100, "hello", "there", 5};

OPTR out = func(arguments);
cout << out << "\n";

//cout << UL::Location::show_instances() << "\n";

#endif
