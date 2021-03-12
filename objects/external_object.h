////////////////////////////////////////////////////////////////////////////////
#ifndef EXTERNAL_OBJECT_H
#define EXTERNAL_OBJECT_H

#include "internal_object.h"
#include "../forward_decl.h"
#include <vector>
#include <optional>

struct ExternalObject {
	bool is_weak,		// Does not strongly reference object
		 is_immovable;	// Object cannot be reassigned
	void *io_ptr;


	template <typename... Ts>
	static void* make_array(Ts&&...);

	template <typename... Ts>
	static void* make_array_w(Ts&&...);
	
	template <typename K, typename V>
	static void* make_pair(K, V);
	
	template <typename T, typename K, typename V, typename... KVs>
	static void make_dict_helper(T&, const K&, const V&, const KVs&...);
	
	template <typename T>
	static void make_dict_helper(T&);
	
	template <typename T, typename... KVs>
	static void* make_dict(const KVs&...);

	template <typename... KVs>
	static void* make_custom(const KVs&...);

	static ExternalObject blank_object();

	template <typename T, typename... Ts>
	static void* specific_object(Ts&&...);
	
	template <typename T, typename... Ts>
	static ExternalObject emplace(Ts&&...);

	ExternalObject& copy_from(const ExternalObject& other);

	static void* shallow_copy_of(const ExternalObject&);

	static ExternalObject deep_copy_of(const ExternalObject& other);

	ExternalObject(); // Default constructor

	ExternalObject(void*, bool=false); // Construct from InternalObject
	ExternalObject& operator =(void*); // Assign from InternalObject

	/*
	#ifdef DO_CACHE_DECL
	ExternalObject(bool);
	ExternalObject& operator =(bool);

	ExternalObject()
	#endif
	*/

	ExternalObject(ExternalObject&&) noexcept; // Move constructor
	ExternalObject& operator =(ExternalObject&&) noexcept; // Move assignment

	ExternalObject(const ExternalObject&, bool=false); // Copy constructor
	ExternalObject& operator =(const ExternalObject&); // Copy assignment

	ExternalObject(ExternalObject&, bool=false);
	ExternalObject& operator =(ExternalObject&);

	template <typename T> ExternalObject(T&&, bool=false, bool=false); // Rvalue constructor
	template <typename T> ExternalObject& operator =(T&&); // Rvalue assignment

	~ExternalObject();

	bool operator ==(const ExternalObject& other) const;

	ExternalObject operator ()(std::vector<ExternalObject>&);
	// Cannot guarantee const-ness!
	ExternalObject operator ()();
	// Call to method (1 arg) or function (0 args)

	ExternalObject as_immovable();
	
	template <typename T> T& get() const;
	template <typename T> T& get_mut();
	template <typename T> InternalObject<T>* get_ptr();
	Types type() const;
	InternalObject<InternalObjectNoValue>::RefCountT& refcount() const;
	//void ExternalObject::create_from_blank(const T& construct_from);
	template <typename T> void create_from_blank(T, bool=false, bool=false);
	template <typename CastT> typename std::remove_reference_t<CastT> cast() const;

	//ExternalObject simple_get_attr(const char* name) const;
	Aliases::CustomT& attrs_of() const;
	ExternalObject& get_type() const;
	std::optional<ExternalObject> simple_get_attr(const char* name);
	//std::optional<ExternalObject> class_get_attr;
	ExternalObject& self_get_attr(const char* name) const; // Just self, not type
	void* get_attr_no_throw(const char* name); // self & type (procedure is similar for both)
	void* get_attr(const char* name); // self & type (procedure is similar for both)
	// get_attr isn't necessarily const!

	template <bool CheckSelf>
	size_t ref_count_to(void const*, std::vector<void const*>*) const;

	//bool subclass_of(const ExternalObject&) const;
	std::optional<Types> class_get_enum_type() const;
	Types get_enum_type() const;

	void weaken();

	void reassign(const ExternalObject&);
	Types find_custom_enum() const;
};

#endif
