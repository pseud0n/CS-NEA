#ifndef STRING_LOWER_H
#define STRING_LOWER_H

TEST_REPR("Create and call monadic function");

auto lower = new UL::CppFunction({}, false, DY_LMBD {
    std::string str_obj;
    if (!argument_data->assign_args<1>(arguments, str_obj)) {
        UL::ArgExc("Function lower takes 0 arguments beyond self");
        return UL::null_optr;
    }
    std::for_each(str_obj.begin(), str_obj.end(), [](char& c) { c = std::tolower(c); });
    cout << "STR " << str_obj << "\n";
    return OPTR(str_obj);
});

auto argument = new UL::Object(std::string("Hello There"));
cout << "---------\n";
cout << (*lower)({argument}) << "\n";

#endif