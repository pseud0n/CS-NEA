////////////////////////////////////////////////////////////////////////////////
#ifndef OBJECT_H
#define OBJECT_H

//Included("object");

class Object {
    /*
    The Object is the standard class which allows several different types to
	be contained in on type. An Object can be used in 2 ways: on the heap or
	on the stack. A heap Object should only be created as part of an
	ObjectPointer/OPTR to get reference counted. A stack Object should not be
	wrapped by an OPTR since it will get deallocated anyway. Note that a
	wrapped Object will be explicitly deleted (which UB if allocated on the
	stack)

    The strength of a stack Object is ignored since it won't be
	reference-counted The strength of a heap Object is important and
	is tracked by an OPTR This is why a heap Object must be wrapped by and OPTR,
	which automatically deletes the object. A stack Object still has a 
	reference count, but it is unused.

    Each object has an associated map. This determines where it will get its
	members from. The objects in this list represent the superclasses of the
	object. The first one of a class is always itself.
    */
	

    friend class ObjectPointer;
    friend const char* r_optr_constness(const OPTR&);
public:
	typedef std::unordered_map<std::string, OPTR> AttrsT;
	/*
	Better than decltype(Object::attrs) or similar
	Use typedefs instead of retyping as part of DRY principle
	*/

    Types type;
	/*
	Represents union type of underlying value if built-in, not the object's
	apparent type. A C++ union does not keep track of the type last stored.
	The value is unchanging once assigned, except for when an OPTR wraps
	an
	*/
    ObjectUnion union_val;
    bool is_const;
    /*
    References to this object will all be weak and ownership may not be
	transferred. A const object is used when the object is part of something
	built-in. For example, member functions of built-in types would not be
	reference counted.
	It would be pointless to keep track of references because we know that at
	least one reference will always remain. The original OPTR is a strong
	reference but all subsequent references are weak references 
    */
    unsigned short reference_count;
	// Number of existing references to this object
    AttrsT *attrs;
	/*
	This stores the names and values of several attributes.
	They are accessed using the name.
	A value could be directly inserted into one of these and reference nowehere
	else. As a result, the values need to be OPTRs in order to keep the contained
	object alive. The keys are stored as strings but will be converted to a UL
	string if they are accessed. Operations like printing this map will not
	require conversion but storing as a dictionary or as another type will
	require a string to become a UL object.
	AttrsT	56 bytes
	AttrsT*	8 bytes
	Not all objects have a dict - integers don't.
	When called (since the attrs dictionary is accessed through a function),
	a dictionary is returned with the appropriate attributes - but this does
	not actually get stored in the object.
	*/
	

    Object(bool=true);
    Object(std::nullptr_t, bool=true);
    //Object(double, bool=true);
    Object(int, bool=true);
    Object(const char*, bool=true);
    Object(CppFunction*, bool=true);
    Object(ByteCodeFunction*, bool=true);
    Object(UserDefinedObject*, bool=true);
    template <typename T> Object(const std::vector<T>*, bool=true);
	// Represents an array which cannot be modified and has constant lookup time
    template <typename ... T> Object(std::tuple<T...>, bool=true);
    Object(const OPTR&, bool=true);
    Object(const Object*, bool=true);
    //Object* make_reference();
    ~Object();
    
    void operator ++();
    void operator --();
    OPTR operator ()(const std::vector<Object*>&);
    //operator double();
    Object operator *(const Object&);

    operator int() const;
    operator std::string () const;
    //operator const Object* () const;
	operator Object* () const;

    template <typename CastT> typename std::remove_reference_t<CastT> cast() const;
	OPTR& get_attr(const std::string&);
	AttrsT::iterator get_iterator(const std::string&); // Non-const return value
	OPTR* get_mro_optrptr_if_present(const std::string&);
    OPTR* get_class_optrptr_if_present(const std::string&); // Descriptive.
	decltype(auto) get_current_value() const;

	size_t get_size() const;
};

#endif