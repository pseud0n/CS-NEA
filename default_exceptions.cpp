#ifndef DEFAULT_EXCEPTIONS_CPP
#define DEFAULT_EXCEPTIONS_CPP

#include "objects/external_object.h"
#include <sstream>

//std::ostream& operator <<(std::ostream&, const ExternalObject&);

namespace Exceptions {
	using namespace UL;

	auto arg_types_wrong = [](const std::vector<ExternalObject>& objects) {
		std::ostringstream oss;
		show_simple_type_error(oss, objects);
		return oss.str();
	};

	auto out_of_range = [](Aliases::NumT& index) {
		std::ostringstream oss;
		oss << "Index " << index << " is out of valid range";
		return oss.str();
	};

	auto attr_not_found = [](const char* name, const ExternalObject& object) {
		std::ostringstream oss;
		oss << "Could not find attr " << name << " in " << object;
		return oss.str();
	};

	auto expected_bool = [](const ExternalObject& object) {
		std::ostringstream oss;
		oss << "ToBool did not return a boolean, returned " << object;
		return oss.str();
	};

	auto comparison_different_types = [](const ExternalObject& object1, const ExternalObject& object2) {
		std::ostringstream oss;
		oss << "Expected the same type for comparison of builtin objects (" << object1 << "&" << object2 <<")";
		return oss.str();
	};

	auto if_no_further_if = []() {
		std::ostringstream oss;
		oss << "If FSM does not accept If";
		return oss.str();
	};

	auto if_no_further_code = []() {
		std::ostringstream oss;
		oss << "If FSM does not accept code";
		return oss.str();
	};
	
	auto if_no_call = []() {
		std::ostringstream oss;
		oss << "If FSM does not accept call";
		return oss.str();
	};
}

#endif
