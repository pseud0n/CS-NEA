const auto vec2 = new std::vector<OPTR>(2);
(*vec2)[0].create_from_blank(99);
(*vec2)[1].create_from_blank(101);

const auto vec = new std::vector<OPTR>(3);
(*vec)[0].create_from_blank(3);
(*vec)[1].create_from_blank(vec2);
(*vec)[2].create_from_blank(5);

clog << "Cool\n";
OPTR x(vec);
clog << x << "\n";
