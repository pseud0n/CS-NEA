#ifndef VARIADIC_FUNCTION_CPP
#define VARIADIC_FUNCTION_CPP

TEST_REPR("Create and call variadic function");

UL::CppFunction prod({}, true, UL_LMBD {
    Aliases::NumT u0;
	std::vector<Aliases::NumT> uns;
	print("In func");
    if (!argument_data->assign_variadic_args<1>(arguments, &uns, u0)) {
        //UL::ArgExc("Function lower takes 0 arguments beyond self");
        return nullptr;
    }
	print("args:", u0, uns);
	u0 = std::accumulate(uns.begin(), uns.end(), u0, std::multiplies<Aliases::NumT>());
    return u0;
}, {Types::number}, Types::number);

auto args = make_eo_vec(11, 12, 13, 14);

print("Made args");

ExternalObject result = prod(args);
print(result);

#endif