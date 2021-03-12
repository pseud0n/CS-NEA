#ifndef STANDLALONE_BIG_DEC_H
#define STANDALONE_BIG_DEC_H

#include <string>

class BigDec {
	friend std::ostream& operator <<(std::ostream&, const BigDec&);
private:
	using PrecisionT = size_t; // For comparison with string length
	static inline PrecisionT precision = 5;
	static inline bmp::cpp_int ten_pow_precision = bmp::pow(bmp::cpp_int(10), precision);
	bmp::cpp_int value;

public:

	static PrecisionT get_precision() {
		return precision;
	}
	
	static void set_precision(PrecisionT to) {
		precision = to;
		ten_pow_precision = bmp::pow(bmp::cpp_int(10), precision);
	}

	BigDec()
		: value(0) {
	}

	BigDec(bmp::cpp_int&& from) {
		value = from * ten_pow_precision;
	}

	template <typename T>
	BigDec(const T& construct_from)
		: value(construct_from) {
	}

	BigDec(const std::string& integer_part, const std::string& fractional_part, bool is_positive=true)
		: value(std::string(is_positive ? "-" : "") + integer_part + fractional_part) {
		/*
		Assume precision = 3
		fractiona_part:
		"123":
			Simply appends "123"
		"12":
			Adds 1 '0' to geto length 3
			Appends "012"
		"1234":
			Removes the last digit
			Appends "123"
		"1235":
			Removes last digit, but since it's a 5, it rounds up
			It adds the 4 digits and then divides the resulting number by 10
			Appends "124"
		
		If the number of places exceeds 4 then it is automatically cut down.
		This is so that the extra memory is not allocated by the cpp_int.
		*/
		std::string construct_from = std::string(is_positive? "-" : "") + integer_part;
		
	}

	bool operator ==(const BigDec& other) {
		return value == other.value;
	}

	BigDec operator +(const BigDec& other) {
		return BigDec(value + other.value);
		// Number of places can't increase
	}

	BigDec operator -(const BigDec& other) {
		return BigDec(value - other.value);
	}
};

#endif