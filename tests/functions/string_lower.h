#ifndef STRING_LOWER_H
#define STRING_LOWER_H

TEST_REPR("Create and call monadic function");

UL::CppFunction lower({}, false, UL_LMBD {
    std::string str_obj;
    if (!argument_data->assign_args<1>(arguments, str_obj)) {
        //UL::ArgExc("Function lower takes 0 arguments beyond self");
        return nullptr;
    }
    std::for_each(str_obj.begin(), str_obj.end(), [](char& c) { c = std::tolower(c); });
    return str_obj;
}, {UL::Types::string});

std::vector<UL::ExternalObject> args(1);
args[0] = 10;//"Hello, World"s;
print("---------");
print(lower(args));

args[0] = "Hello, World!"s;
print("---------");
print(lower(args));

#endif