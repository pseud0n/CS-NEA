#ifndef TESTS_CPP_FUNCTION_H_
#define TESTS_CPP_FUNCTION_H_


TEST_REPR("Create and call variadic function");

UL::Tracker::repr();

OPTR func = OPTR(new UL::CppFunction({10, 11, 33}, true, DY_LMBD {
    // arguments: std::vector<UL::Object*>
    int a, b, c, d, e, f; // These 6 have to be (convertible to) integers
    std::vector<std::string> extra_args;
    // arguments: std::vector<OPTR>&, extra_args: std::vector<std::string>
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
UL::Tracker::repr();

#endif
