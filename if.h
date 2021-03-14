#ifndef IF_H
#define IF_H

#include "code_block.h"

class If {
private:
	using States = enum class States {
		accept_any,
		accept_if_body,
		accept_call,
	};
	States state = States::accept_if_body;
	bool condition_met = false;

	void send_code(bool, const ExternalObject&);
	void evaluate() const;
	bool is_modifiable() const;

public:
	ExternalObject code_block_ref;

	If(bool);
	~If();

	void send_code_block(const ExternalObject&);
	void send_if(const If&);
};

bool operator ==(const If& o1, const If& o2) {
	return &o1 == &o2;
}

#endif
