#ifndef CODE_BLOCK_CPP
#define CODE_BLOCK_CPP

class CodeBlock {
public:
	std::forward_list<std::pair<ExternalObject, CodeBlock>> catches;
	/*
	Can be called with keyword but not positional arguments
	e.g. {}(:kwargs)
	*/
	/*
	ExternalObject operator ()() {

	}
	*/

};

#endif