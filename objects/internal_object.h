#ifndef INTERNAL_OBJECT_H
#define INTERNAL_OBJECT_H

class InternalObjectNoValue {};
// 1 byte, since must have memory which can be referenced

template <typename StoredT/* = InternalObjectNoValue*/> // e.g. int, then MyT = cpp_int
class InternalObject {
public:
	/*
	This object is constructed from a type which is then mapped to a type which it stores.
	*/
	using RefCountT = unsigned short;

	Types type;
	RefCountT reference_count;
	bool is_immovable;
	//AttrsT *attrs;
	std::optional<StoredT> stored_value;
	// unidentified underlying object
	// Alternatively, a pointer to a dictionary
	
	template <typename T> static InternalObject<StoredT>* copy(const T&);
	template <typename T> static InternalObject<StoredT>* deep_copy(const T&);

	InternalObject();
	template<typename... Ts> InternalObject(Ts&&...);
	~InternalObject();

	template <typename... CtorTs>
	void emplace(CtorTs&&...);
};

#endif
