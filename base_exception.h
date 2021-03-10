#ifndef BASE_EXCEPTION_H
#define BASE_EXCEPTION_H

class BaseException {
private:
	static inline auto default_value = nullptr;
	//static inline ExternalObject& default_exc = Classes::base_exception;
public:
	ExternalObject message;
	ExternalObject type;

	BaseException();

	BaseException(ExternalObject&&, ExternalObject&);

	template <typename T>
	BaseException(T&&, ExternalObject&);

	BaseException(const BaseException& other);

	BaseException& operator =(const BaseException& other);

	BaseException(BaseException&& other) noexcept;

	BaseException& operator =(BaseException& other);
};


bool operator ==(const BaseException& o1, const BaseException& o2) {
	return o1.message == o2.message; // Cannot know if functions are the same, so just check if the internal pointers are equal
}

#endif
