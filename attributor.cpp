#ifndef ATTRIBUTOR_CPP
#define ATTRIBUTOR_CPP
{
	using namespace UL;

	print("Hai, sensei!");
	
	auto&
	insert_ref = builtin_lookup.at(Types::string);

	CppFunction lower({}, false, UL_LMBD {
		std::string str_obj;
		if (!argument_data->assign_args<1>(arguments, str_obj)) {
			//UL::ArgExc("Function lower takes 0 arguments beyond self");
			return nullptr;
		}
		std::for_each(str_obj.begin(), str_obj.end(), [](char& c) { c = std::tolower(c); });
		return str_obj;
	}, {Types::string}); 


	insert_ref.try_emplace("lower", lower);
}

#endif