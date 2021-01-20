TEST_REPR("Testing access and function of objects");

UL::ExternalObject str("Alex Scorza!"s);
print(str.get_attr("lower"));
print("COOL");
//print(str.get_attr("lower").get<UL::Aliases::CppFunctionViewT>()({}));