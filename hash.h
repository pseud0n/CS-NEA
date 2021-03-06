#ifndef HASH_H
#define HASH_H

namespace std {
	using namespace UL;
	//friend unsigned long dec(const BigDec&);
	template<>
	struct hash<bmp::cpp_int> {
        size_t operator ()(const bmp::cpp_int& n) const noexcept {
            return std::hash<std::string>()(n.str());
        }
    };

	template<>
	struct hash<CppFunction> {
        size_t operator ()(const CppFunction& n) const noexcept {
            return std::hash<size_t>()(((size_t)&n) >> 3); // 3: 64 bit
        }
    };


	template<>
	struct hash<FunctionView> {
        size_t operator ()(const FunctionView& n) const noexcept {
            return std::hash<size_t>()(((size_t)&n) >> 3); // 3: 64 bit
        }
    };

	template<>
	struct hash<BaseException> {
        size_t operator ()(const BaseException& n) const noexcept {
            return std::hash<size_t>()((size_t)&n); // 3: 64 bit
        }
	};

	template<>
	struct hash<ExternalObject> {
		size_t operator ()(const ExternalObject&) const noexcept; 
	};

	/*
	template<>
	struct hash<PairT> {
        size_t operator ()(const Aliases::PairT& n) const noexcept {
            return std::hash<size_t>()()
        }
    };
	*/
/*
	template <>
	struct hash<UL::ExternalObject> {
		size_t operator ()(const UL::ExternalObject& object) const noexcept {
			#define SWITCH_MACRO(T) return std::hash<T>()(object.get<T>())
			GENERATE_SWITCH(object.type())
			#undef SWITCH_MACRO
		}
	};
	
	#define NO_HASH_DEFINED(T) \
		template <>\
		struct hash<T> {
			size_t operator ()(const T&) const noexcept {
				RAISE(Exc::());
			}
		};
	#undef NO_HASH_DEFINED
*/
	

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
