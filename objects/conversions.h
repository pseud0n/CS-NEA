#ifndef MAKE_TYPE_CONVERSION

template <typename T> struct CorrespondingType;
template <typename T> struct AssociatedData;

#define MAKE_TYPE_CONVERSION(from, to, enum_t, make_immovable)		\
	template <> struct CorrespondingType<from> {					\
		using Tp = to;												\
	};																\
	template <> struct AssociatedData<to> {							\
		static inline Types enum_type = enum_t;						\
		static inline bool is_immovable = make_immovable;			\
	};

template <typename T>
struct CorrespondingReverse;

#define R_MAKE_TYPE_CONVERSION(from, to, enum_t, make_immovable) 	\
	MAKE_TYPE_CONVERSION(from, to, enum_t, make_immovable)			\
	template <> struct CorrespondingReverse<to>	{					\
		using Tp = from;											\
	};

template <typename T>
using GetCorrespondingType =
	typename CorrespondingType<std::remove_reference_t<T>>::Tp;

MAKE_TYPE_CONVERSION(std::string, std::string, Types::string, true)
MAKE_TYPE_CONVERSION(bool, bool, Types::boolean, true)
MAKE_TYPE_CONVERSION(std::nullptr_t, std::nullptr_t, Types::null, true)
MAKE_TYPE_CONVERSION(int, Aliases::NumT, Types::number, true)
MAKE_TYPE_CONVERSION(Aliases::ArrayT, Aliases::ArrayT, Types::array, false)
MAKE_TYPE_CONVERSION(Aliases::PairT, Aliases::PairT, Types::pair, false)
MAKE_TYPE_CONVERSION(Aliases::DictT, Aliases::DictT, Types::dictionary, false)
MAKE_TYPE_CONVERSION(Aliases::CustomT, Aliases::CustomT, Types::custom, false)
MAKE_TYPE_CONVERSION(Aliases::CppFunctionT, Aliases::CppFunctionT, Types::cpp_function, false)
MAKE_TYPE_CONVERSION(Aliases::CppFunctionViewT, Aliases::CppFunctionViewT, Types::cpp_function_view, false)
#endif