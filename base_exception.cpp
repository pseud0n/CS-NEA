#ifndef BASE_EXCEPTION_CPP
#define BASE_EXCEPTION_CPP

#include "base_exception.h"

std::unordered_map<std::string, ExternalObject> all_builtins;

BaseException::BaseException()
	: message(default_value), type(all_builtins["ExcRuntime"]) {
}

BaseException::BaseException(ExternalObject&& obj, ExternalObject& type)
	: message(std::forward<ExternalObject>(obj)), type(type) {
}

template <typename T>
BaseException::BaseException(T&& obj, ExternalObject& type)
	: message(std::forward<T>(obj)), type(type){
}

BaseException::BaseException(const BaseException& other)
	: message(other.message), type(other.type){
}

BaseException& BaseException::operator =(const BaseException& other) {
	message = other.message;
	type = other.type;
}

BaseException::BaseException(BaseException&& other) noexcept
	: message(std::exchange(message, default_value)), type(std::exchange(type, all_builtins["ExcRuntime"])) {
}

BaseException& BaseException::operator =(BaseException& other) {
	message = std::exchange(other.message, default_value);
	type = std::exchange(other.type, all_builtins["ExcRuntime"]);
}

#endif

