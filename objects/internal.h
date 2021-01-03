#ifndef INTERNAL_OBJECT_H
#define INTERNAL_OBJECT_H


template <typename StoredT = void*> // e.g. int, then MyT = cpp_int
// void* is just a filler value (void is not available)
struct InternalObject {
	/*
	This object is constructed from a type which is then mapped to a type which it stores.
	*/
	using AttrsT = std::unordered_map<std::string, OPTR>;
	using RefCountT = unsigned short;

	Types type;
	RefCountT reference_count;
	bool is_immovable;
	//AttrsT *attrs;
	StoredT stored_value;
	// unidentified underlying object
	// Alternatively, a pointer to a dictionary
	
	InternalObject();
	InternalObject(StoredT);
	~InternalObject();
	//..
};

#endif