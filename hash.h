#ifndef HASH_H
#define HASH_H
//INCLUDED(HASH)
//Included("hash");

namespace std {
	//friend unsigned long dec(const BigDec&);
	/*
	template<>
	struct hash<bmp::cpp_int> {
        size_t operator ()(const bmp::cpp_int& n) const noexcept {
            return std::hash(n.str());
        }
    };
	*/

	template <>
	struct hash<OPTR> {
		size_t operator ()(const OPTR& optr) const noexcept {
			return (size_t)optr.object_ptr; // TEMPORARY
		}
	};
}

#endif