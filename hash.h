#ifndef HASH_H
#define HASH_H

namespace std {
	//friend unsigned long dec(const BigDec&);
	template<>
	struct hash<bmp::cpp_int> {
        size_t operator ()(const bmp::cpp_int& n) const noexcept {
            return std::hash<std::string>()(n.str());
        }
    };

	template <>
	struct hash<OPTR> {
		size_t operator ()(const OPTR& optr) const noexcept {
			return (size_t)optr.object_ptr; // TEMPORARY
		}
	};

	template <>
	struct hash<UL::ExternalObject> {
		size_t operator ()(const UL::ExternalObject& object) const noexcept {
			return (size_t)object.io_ptr; // TEMPORARY
		}
	};

	/*
	template<>
	struct hash<UL::Aliases::ArrayT> {
        size_t operator ()(const UL::Aliases::ArrayT& v) const noexcept {
        	return boost::hash_range(v.begin(), v.end());
        }
    };
	*/
}

#endif