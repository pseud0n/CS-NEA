#ifndef IF_CPP
#define IF_CPP

#include "code_block.h"
#include "if.h"

/*
States:
	States::accept_if_body
	If(condition)
	Accepts
		Code block States::accept_any
	
	States::accept_any
	If(condition)->{}
	Accepts
		Code block: States::accept_call
		If: States::accept_if_body
		Call: complete!
	
	States::accept_call
	If(condition)->{}, If(condition)->{}->{}
	Accepts
		Call: complete!
*/

If::If(bool initial)
	: condition_met(initial) {
}

If::~If() {
}

bool If::is_modifiable() const {
	return code_block_ref.is_null();
}

void If::send_code(bool condition, const ExternalObject& code) {
	if (condition && is_modifiable()) {
		code_block_ref = code;
	}
}

void If::send_if(const If& object) {
	if (state != States::accept_any)
		THROW_ERROR(Exceptions::if_no_further_if());
	condition_met = object.condition_met;
	state = States::accept_if_body;
	//send_code(object.condition_met, *object.execute);
	// Only adds code if object's condition is true and self hasn't already found valid condition
}

void If::send_code_block(const ExternalObject& code_wrapper) {
	// Send code if condition of last If was true and no trues already met.
	// E.g. If(false){} sends nothing;
	// If(true){}If(true){} sends nothing the second time
	print("Sending code", code_wrapper, condition_met, code_wrapper.get<Aliases::CodeBlockT>());
	if (state == States::accept_any) {
		state = States::accept_call;
		send_code(true, code_wrapper.get<Aliases::CodeBlockT>()); // Else; send code regardless
	} else if (state == States::accept_if_body) {
		state = States::accept_any;
		print("Accepting body");
		send_code(condition_met, code_wrapper.get<Aliases::CodeBlockT>());
	} else {
		// Recieved a code block after else already given
		THROW_ERROR(Exceptions::if_no_further_code())
	}
	print("Sent code");
}

void If::evaluate() const {
	if (state == States::accept_if_body) {
		THROW_ERROR(Exceptions::if_no_call())
	}
	if (!code_block_ref.is_null())
		code_block_ref.get<Aliases::CodeBlockT>();
	else
		Bytecode::scopes.push_op(nullptr);
}


#endif
