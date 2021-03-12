#ifndef DEFAULT_EXCEPTIONS_CPP
#define DEFAULT_EXCEPTIONS_CPP

#include "objects/external_object.h"
#include <sstream>

std::ostream& operator <<(std::ostream&, const ExternalObject&);

namespace Exceptions {
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
}

#endif
