TEST_REPR("Making a dictionary with various keys and values");

print("scope 1");

{
	UL::ExternalObject o = MKRRY(nullptr, 1, "abc"s, MKRRY("dsd"s, true, 0, nullptr), -100, false, 10);
		print("Let's go");

	print(o);
}

print("scope 2");

{
	UL::ExternalObject o = MKDCT(MKRRY(1,2,3),2, 3,true);
	print("Constructed");
	o.get<UL::Aliases::DictT>().emplace(9, 10);
	print(o);
}