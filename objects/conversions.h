#ifndef MAKE_TYPE_CONVERSION

template <typename T> struct CorrespondingType;
template <typename T> struct AssociatedData;

#define ADD_TYPE_CONVERSION(from, to)								\
	template <> struct CorrespondingType<from> {					\
		using Tp = to;												\
	};

#define MAKE_TYPE_CONVERSION(from, to, enum_t, make_immovable)		\
	ADD_TYPE_CONVERSION(from, to)									\
	template <> struct AssociatedData<to> {							\
		static inline Types enum_type = enum_t;						\
		static inline bool is_immovable = make_immovable;			\
	};

template <typename T>
struct CorrespondingReverse;

template <typename T>
using GetCorrespondingType =
	typename CorrespondingType<std::remove_reference_t<T>>::Tp;

template <typename T>
using GetAssociatedData =
	AssociatedData<std::remove_reference_t<T>>;

MAKE_TYPE_CONVERSION(std::string, std::string, Types::string, true)
MAKE_TYPE_CONVERSION(bool, bool, Types::boolean, true)

MAKE_TYPE_CONVERSION(std::nullptr_t, std::nullptr_t, Types::null, true)

MAKE_TYPE_CONVERSION(int, Aliases::NumT, Types::number, true)
ADD_TYPE_CONVERSION(size_t, Aliases::NumT)

MAKE_TYPE_CONVERSION(Aliases::ArrayT, Aliases::ArrayT, Types::array, false)

MAKE_TYPE_CONVERSION(Aliases::PairT, Aliases::PairT, Types::pair, false)

MAKE_TYPE_CONVERSION(Aliases::DictT, Aliases::DictT, Types::dictionary, false)

MAKE_TYPE_CONVERSION(Aliases::CustomT, Aliases::CustomT, Types::custom, false)

MAKE_TYPE_CONVERSION(Aliases::CppFunctionT, Aliases::CppFunctionT, Types::cpp_function, false)
ADD_TYPE_CONVERSION(Aliases::CppFunctionT&, Aliases::CppFunctionT)

MAKE_TYPE_CONVERSION(Aliases::CppFunctionViewT, Aliases::CppFunctionViewT, Types::cpp_function_view, false)

MAKE_TYPE_CONVERSION(Aliases::BaseExceptionT, Aliases::BaseExceptionT, Types::base_exception, false)
#endif