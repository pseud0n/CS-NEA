#ifndef EXTERNAL_OBJECT_CPP
#define EXTERNAL_OBJECT_CPP

#include "external_object.h"
#include "../forward_decl.h"
#include <array>

template <typename... Ts>
void* ExternalObject::make_array(Ts&&... construct_from) {
	print("Making array");
	auto temp = new InternalObject<Aliases::ArrayT>(sizeof...(Ts));
	size_t i = 0;
	((
	  [&](){
		print("Emplacing using", construct_from);
		temp->stored_value->replace(i++, std::forward<Ts>(construct_from));
	  }()
	), ...);
	print("Made array");
	return reinterpret_cast<void*>(temp);
	/* Controlled by ExternalObject so will get deleted correctly */
}

template <typename... Ts>
void* ExternalObject::make_array_w(Ts&&... construct_from) {
	print("Making weak array");
	auto temp = new InternalObject<Aliases::ArrayT>(sizeof...(Ts));
	print("New hashable array:", *temp->stored_value);
	size_t i = 0;
	((
	  [&](){
		print("Emplacing using", construct_from, "replacing", (*temp->stored_value)[i]);
		temp->stored_value->replace(i++, std::forward<Ts>(construct_from), true);
	  }()
	), ...);
	print("Made weak array");
	return reinterpret_cast<void*>(temp);
	/* Controlled by ExternalObject so will get deleted correctly */
}

#undef MAKE_MAKE_ARRAY
template <typename K, typename V>
void* ExternalObject::make_pair(K key, V value) {
	auto temp = new InternalObject<Aliases::PairT>();
	temp->stored_value->first 	= key;
	temp->stored_value->second 	= value;
	return reinterpret_cast<void*>(temp);
}

/*
template <typename K, typename V, typename... KVs>
void ExternalObject::make_dict_helper(Aliases::DictT& u_map_ref, K&& key, V&& value, KVs&&... Ks) {
	clog << "Emplacing " << key << ":" << value << "\n";
	u_map_ref.emplace(key, value);
	print("Emplaced!");
	make_dict_helper(u_map_ref, std::forward<KVs>(Ks)...); // Tail recursion
}



template <typename... KVs>
void* ExternalObject::make_dict(KVs&&... Ks) {
	auto temp = new InternalObject<Aliases::DictT>;
	make_dict_helper(*temp->stored_value, std::forward<KVs>(Ks)...);
	return reinterpret_cast<void*>(temp);
}
*/

template <typename T>
void ExternalObject::make_dict_helper(T&) {
	// Does absolutely nothing! (sort of)
}

template <typename T, typename... KVs>
void* ExternalObject::make_dict(const KVs&... kvs) {
	T dict;
	make_dict_helper<T>(dict, kvs...);
	return reinterpret_cast<void*>(new InternalObject<T>(std::move(dict)));
}

template <typename T, typename K, typename V, typename... KVs>
void ExternalObject::make_dict_helper(T& u_map_ref, const K& key, const V& value, const KVs&... kvs) {
	//clog << "Emplacing " << key << ":" << value << "\n";
	print(u_map_ref.size());
	u_map_ref.try_emplace(key, value);
	make_dict_helper<T>(u_map_ref, kvs...); // Tail recursion
}
/*
template <typename... KVs>
void* ExternalObject::make_std_dict(const KVs&... kvs) {
	return make_dict
}
*/
ExternalObject ExternalObject::blank_object() {
	ExternalObject object;
	//auto new_io_ptr = new InternalObject<Aliases::CustomT>;
	//new_io_ptr.stored_value.emplace()
	clog << "BLANK OBJECT!\n";
	exit(0);
	
	return object;
}


template <typename T, typename... Ts>
void* ExternalObject::specific_object(Ts&&... args) {
	return reinterpret_cast<void*>(new InternalObject<T>(std::forward<Ts>(args)...));
}

template <typename T, typename... Ts>
ExternalObject ExternalObject::emplace(Ts&&... args) {
	auto io_ptr = new InternalObject<GetCorrespondingType<T>>;
	// defaults stored_value to std::nullopt
	io_ptr->stored_value.emplace(std::forward<Ts>(args)...);
	return reinterpret_cast<void*>(io_ptr);
}

ExternalObject& ExternalObject::copy_from(const ExternalObject& other) {
	is_weak = false; // First reference to a new object
	is_immovable = other.is_immovable;
	print("Shallow copy!");
//#define SWITCH_MACRO(T) io_ptr = reinterpret_cast<void*>(new InternalObject<T>(other.get<T>()));
#define SWITCH_MACRO(T) io_ptr = reinterpret_cast<void*>(InternalObject<T>::copy(other.get<T>()));
	GENERATE_SWITCH(other.type());
#undef SWITCH_MACRO
	print(io_ptr);
	return *this;
}

void* ExternalObject::shallow_copy_of(const ExternalObject& object) {
#define SWITCH_MACRO(T) return reinterpret_cast<void*>(InternalObject<T>::copy(object.get<T>()));
	GENERATE_SWITCH(object.type());
#undef SWITCH_MACRO
}

ExternalObject ExternalObject::deep_copy_of(const ExternalObject& object) { // pass by value; copy
	/*
	Default copy of ExternalObject doesn't actually copy - it just makes a new reference
	Look at the objects which can contain another ExternalObject:
		DictT, CustomT
		ArrayT
		PairT
		FunctionT
		FunctionViewT
	*/
	//T object_copy(object); // Copy assignment operator; new, independent object made
	print("Made copy of object", &object, object);
	switch (object.type()) {
		case Types::null_type:
			return nullptr;
		case Types::array:
		{
			Aliases::ArrayT& original_array = object.get<Aliases::ArrayT>();
			Aliases::ArrayT new_object(original_array.size());
			print("Time to copy");
			for (size_t i = 0; i < original_array.size(); ++i) {
				new_object[i] = deep_copy_of(original_array[i]);
			}
			print("DEEP COPY ARRAY", new_object);
			return emplace<Aliases::ArrayT>(std::move(new_object)); // 
		}
		case Types::dictionary:
		{
			Aliases::DictT& original_dict = object.get<Aliases::DictT>();
			Aliases::DictT new_object;
			print("Making an attempt");
			for (const auto& pair: original_dict) {
				print(pair.first, ":", pair.second);
				new_object.try_emplace(pair.first, deep_copy_of(pair.second));
				// creates a new blank object then assigns it to a deep copy of the value
			}
			//return reinterpret_cast<void*>(new InternalObject<Aliases::DictT>(std::move(new_object)));
			
			print("DEEP COPY DICT", new_object, original_dict);
			return emplace<Aliases::DictT>(std::move(new_object)); // 
		}
		case Types::pair:
		{
			Aliases::PairT& original_pair = object.get<Aliases::PairT>();
			Aliases::PairT new_object;
//			new_object.first.deep_copy_of(original_pair.first);
//			new_object.second.deep_copy_of(original_pair.second);
			//return reinterpret_cast<void*>(new InternalObject<Aliases::PairT>(deep_copy_of(original_pair.first), deep_copy_of(original_pair.second)));
			return emplace<Aliases::PairT>(deep_copy_of(original_pair.first), deep_copy_of(original_pair.second)); // 
		}
		default:
#define SWITCH_MACRO(T) { T temp = object.get<T>(); return temp; }
			GENERATE_SWITCH(object.type());
#undef SWITCH_MACRO


	}
}

ExternalObject::ExternalObject() : is_weak(true), is_immovable(false), io_ptr(nullptr) {
	clog << "Constructing new Null ExternalObject\n";
}

ExternalObject::ExternalObject(void* internal_object_ptr, bool)
	: is_weak(false), is_immovable(false), io_ptr(internal_object_ptr) { // Cannot be weak if from original
	print("Constructing from InternalObject pointer", io_ptr);
}

ExternalObject& ExternalObject::operator =(void* internal_object_ptr) {
	this->~ExternalObject();
	print("Assigning from InternalObject pointer", internal_object_ptr);
	is_weak = false;
    is_immovable = false;
	io_ptr = internal_object_ptr;
	return *this;
}

/*
#ifdef DO_CACHE_DECL
ExternalObject::ExternalObject(bool construct_from)
	: io_ptr(construct_from ? Cache::bool_true : Cache::bool_false) {}

ExternalObject& ExternalObject::operator =(bool construct_from) {
	io_ptr = construct_from ? Cache::bool_true : Cache::bool_false;
	return *this;
}
#endif
*/

ExternalObject::ExternalObject(const ExternalObject& copy_from, bool make_weak) 
    : is_weak(copy_from.is_weak || make_weak), is_immovable(copy_from.is_immovable), io_ptr(copy_from.io_ptr) {
	print("Copying from", copy_from);
	/*
	is_weak		make_weak		result
	false		false			false
	false		true			true
	true		false			true
	true		true			true		
	*/
	if (io_ptr && !is_weak) {// Also increments references for cached objects
		++copy_from.refcount(); // Since method returns lvalue
		print("Incref'ing", *this, "to", refcount());
	}
}

ExternalObject& ExternalObject::operator =(const ExternalObject& copy_from) {
	this->~ExternalObject();
	print("Copy-assigning from", copy_from);
	is_weak = copy_from.is_weak;
    is_immovable = copy_from.is_immovable;
	io_ptr = copy_from.io_ptr;
	if (io_ptr && !is_weak) {
		++refcount();
		print("Incref'ing", *this, "to", refcount());
	}
	return *this;
}

ExternalObject::ExternalObject(ExternalObject& copy_from, bool make_weak)
    : is_weak(copy_from.is_weak || make_weak), is_immovable(copy_from.is_immovable), io_ptr(copy_from.io_ptr) {
	print("Semi-copying from", copy_from, "is_weak:", copy_from.is_weak);
	if (io_ptr && !is_weak) {// Also increments references for cached objects
		++copy_from.refcount(); // Since method returns lvalue
		print("Incref'ing", *this, "to", refcount());
	}
}
ExternalObject& ExternalObject::operator =(ExternalObject& copy_from) {
	this->~ExternalObject();
	print("Semi-copy-assigning from", copy_from);
	is_weak = copy_from.is_weak;
    is_immovable = copy_from.is_immovable;
	io_ptr = copy_from.io_ptr;
	if (io_ptr && !is_weak) {
		++refcount();
		print("Incref'ing", *this, "to", refcount());
	}
	return *this;
}

ExternalObject::ExternalObject(ExternalObject&& move_from) noexcept {
	print("Moving from", move_from);
	is_weak = move_from.is_weak;
	is_immovable = std::exchange(move_from.is_immovable, false); // The leftover null object should be assignable
	io_ptr = std::exchange(move_from.io_ptr, nullptr);
	// The old object won't be deleted since it has a 0 pointer
}

ExternalObject& ExternalObject::operator =(ExternalObject&& move_from) noexcept {
	this->~ExternalObject();
	print("Move-assigning from", move_from);
	is_weak = move_from.is_weak;
	is_immovable = std::exchange(move_from.is_immovable, false);
	io_ptr = std::exchange(move_from.io_ptr, nullptr);
	// The old object won't be deleted since it has a null pointer
	return *this;
}

#define DEL_IO_PTR(T) delete reinterpret_cast<UL::InternalObject<Aliases::T>*>(io_ptr); io_ptr = nullptr; break;
ExternalObject::~ExternalObject() {
	print(" -", *this);
	/*
	#define SWITCH_MACR0
	GENERATE_SWITCH(typ0
	#undef SWITCH_MACRO0
	*/

	/*
	When deleting, the reference count of an object should be reduced.
	Depending on the value of is_weak, it may or may not decrement.
	Weak:
		NullT (always)
		BoolT (if caching turned on)
		NumT (if caching turned on an in range)
		Any other type explicitly declared to be weak
	
	An object may contain a reference to itself
	When an object is deleted, the objects it contains must first be deleted
	*/
	if(io_ptr) {
		if (!is_weak) {
			// Guarantee of valid pointer; null: is_weak = true
			// Pointer is only invalid when object is null
			print("\tDecref'ing", *this, "to", refcount() - 1);
			size_t self_ref = ref_count_to<false>(io_ptr, nullptr);
			if (self_ref)
				print("self-ref:", self_ref);
			if(--refcount() != self_ref) return; // Then there are still remaining references
			// No external references still remain to underlying object
		} else {
			print("\tWeak; ignoring");
			return;
		}
		//print("Reduced reference count to", refcount());
//#ifdef DO_CACHE_DECL
//		if (type() == Types::boolean) return;
//#endif
		print("Deleting ExternalObject:", *this);
		switch (type()) {
			case Types::custom:
			/*
			{
				print("Clearing custom", get<Aliases::CustomT>());
				//get<Aliases::CustomT>().clear();
				Aliases::CustomT& dict = get<Aliases::CustomT>();
				for (Aliases::CustomT::iterator it = dict.begin(); it != dict.end();) {
					print("--", *it, dict);
					if (it->second.io_ptr != io_ptr) {
						print("delete");
						it = dict.erase(it); // Don't increment iterator if item deleted
						print("success");
					} else {
						++it;
					}
				}
				print("Cleared custom", dict);
			}
			*/
				print("Clearing custom", get<Aliases::CustomT>());
				/*
				std::erase_if(get<Aliases::CustomT>(), [this](const auto& pair){
					//print("> ", pair);
					return pair.second.io_ptr != io_ptr;
				});
				*/
				for (auto& pair : get<Aliases::CustomT>()) {
					if (pair.second.io_ptr == io_ptr) pair.second.weaken();
				}
				get<Aliases::CustomT>().clear();
				print("Cleared custom", get<Aliases::CustomT>());
				break;
			case Types::dictionary:
				get<Aliases::DictT>().clear();
				break;
			case Types::array:
				print("Clearing array");
				for (auto& item : get<Aliases::ArrayT>()) {
					if (item.io_ptr == io_ptr) item.weaken();
				}
				get<Aliases::ArrayT>().clear();
				print("Cleared array");
				break;
			case Types::pair:
				get<Aliases::PairT>() = {}; // Destructs elements inside
				break;
		}
		//print("Preparing to delete", *this, this, refcount());
#define SWITCH_MACRO(T) delete reinterpret_cast<InternalObject<T>*>(io_ptr); break;
		GENERATE_SWITCH(type())
#undef SWITCH_MACRO
		/*
		switch (type()) {
			case Types::number:
				DEL_IO_PTR(NumT);
			case Types::boolean:
				#ifdef DO_CACHE_DECL
				break; // true & false are cached
				#else
				DEL_IO_PTR(BoolT)
				#endif
			case Types::string:
				DEL_IO_PTR(StringT)
			case Types::bytecode_function:
				DEL_IO_PTR(ByteCodeFunctionT)
			case Types::cpp_function:
				DEL_IO_PTR(CppFunctionT)
			case Types::cpp_function_view:
				DEL_IO_PTR(CppFunctionViewT)
			case Types::pair:
				DEL_IO_PTR(PairT)
			case Types::list:
				DEL_IO_PTR(ListT)
			case Types::array:
				DEL_IO_PTR(ArrayT)
			case Types::dictionary:
				DEL_IO_PTR(DictT)
			case Types::base_exception:
				DEL_IO_PTR(BaseExceptionT)
			case Types::custom:
				DEL_IO_PTR(CustomT)
			default:
				print("DID NOT DELETE", type(), io_ptr);
		}
		*/
		//print("Deleted");
	} else {
		print("Ignoring null io_ptr");
	}

}
#undef DEL_IO_PTR

template <typename T>
ExternalObject::ExternalObject(T&& construct_from, bool is_weak, bool is_immovable) {
	/*
	The 'type' of an InternalObject is the type it is constructed from.
	Object may be copied or moved into construct_from, but is never
	copied after that
	*/
	create_from_blank<T>(std::forward<T>(construct_from), is_weak, is_immovable);
	/*
	Perfectly forwards lvaue or rvalue reference.
	Object may be constructed from either.
	*/
}

template <typename T>
ExternalObject& ExternalObject::operator =(T&& construct_from) {
	this->~ExternalObject();
	print("Assigning from", construct_from);
	create_from_blank<T>(std::forward<T>(construct_from));
	return *this;
}

bool ExternalObject::operator ==(const ExternalObject& o2) const {
	if (type() != o2.type())
		return false;
#define SWITCH_MACRO(T) return get<T>() == o2.get<T>();
	GENERATE_SWITCH(type())
#undef SWITCH_MACRO
}

ExternalObject ExternalObject::operator ()(std::vector<ExternalObject>& args) {
	switch (type()) {
		case Types::cpp_function:
			print("CppFunction!");
			return get<Aliases::CppFunctionT>()(args);
		case Types::cpp_function_view:
			print("CppFunctionView!");
			return get<Aliases::CppFunctionViewT>()(args);
		case Types::code_block:
			print("CodeBlock!");
			get<Aliases::CodeBlockT>()();
			return nullptr;
		default:
			print("Me:", *this, "my type attrs:", get_type().attrs_of());
			ExternalObject& my_type = get_type();
			ExternalObject call_me = my_type.get_attr("Call");
			std::vector<ExternalObject> args_plus_class {*this};
			print("Got to here...", *this);
			args_plus_class.insert(args_plus_class.end(), args.begin(), args.end());
			print("CALLING", *this ,"WITH", args_plus_class);
			return call_me(args_plus_class);
	}
}

ExternalObject ExternalObject::operator ()() {
	return operator ()(CppFunction::empty_eobject_vec);
}


template <typename T>
T& ExternalObject::get() const {
	return *reinterpret_cast<InternalObject<T>*>(io_ptr)->stored_value;
}

template <typename T>
T& ExternalObject::get_mut() {
	if constexpr (std::is_same_v<T, ExternalObject>)
		return *this;
	else
		return *reinterpret_cast<InternalObject<T>*>(io_ptr)->stored_value;
}

template <typename T>
InternalObject<T>* ExternalObject::get_ptr() {
	return reinterpret_cast<InternalObject<T>*>(io_ptr);
}

Types ExternalObject::type() const {
	return io_ptr ? *reinterpret_cast<Types*>(io_ptr) : Types::null_type;
	// Since this is the first 4 bytes of the object
}


InternalObject<InternalObjectNoValue>::RefCountT& ExternalObject::refcount() const {
	return *(InternalObject<InternalObjectNoValue>::RefCountT*)((Types*)io_ptr + 1);
	// +1: adds sizeof(Types) bytes to get to bytes for refcount 
} // We can use this as an lvalue and modify in-place

template <typename T>
void ExternalObject::create_from_blank(T construct_from, bool make_weak, bool make_immovable) {
	print("Creating from blank");
	/*
	Turns an ExternalObject with io_ptr 0 into a normal object
	
	If DO_CACHE_DECL, then check if it's a cacheable type.
	If it's not, 'else' into the standard procedure
	
	If undefined, then go straight to standard procedure.

	T may be an lvalue or rvalue reference.
    If T is an rvalue reference, then it has been passed in as an rvalue.
    E.g. if a CppFunction rvalue is passed into the constructor, then it needs
    to forward it, preserving this by moving the value. When the actual object
    needs to be transferred into an InternalObject, its value is moved into the
    object.

    If the value passed in was an lvalue reference, then it simply continues
    referring to it as a reference and does not copy or move it. When it is put
    into the InternalObject, it is copied so that if the original object is
    deleted, it will not remove the wrapped object and vice versa; they must act
    indepentently from one another.
	*/

	//using blah  = CorrespondingType<T>::Tp;
	using CorrespondingT = GetCorrespondingType<std::remove_reference_t<T>>;
	using AssociatedT = AssociatedData<CorrespondingT>;
	

	clog << "Constructing new " << AssociatedT::enum_type << " ";
	if constexpr(std::is_same_v<CorrespondingT, std::nullptr_t>) {
		io_ptr = nullptr;
        is_immovable = false;
		clog << "0\n";
		return;
	}
	#ifdef DO_CACHE_DECL
	if constexpr(std::is_same_v<CorrespondingT, Aliases::BoolT>) {
		is_weak = true; // No reference counting involved
        is_immovable = false;
		io_ptr = construct_from ? Cache::bool_true : Cache::bool_false;
	} else if constexpr (std::is_same_v<CorrespondingT, Aliases::NumT>) {
		int as_int = static_cast<int>(construct_from);
		is_immovable = false;
		// C++-style casts!
		// Values may be negative so signed is used
		if (as_int >= MIN_CACHED_INTEGER_VALUE && as_int <= MAX_CACHED_INTEGER_VALUE) {
			io_ptr = Cache::numbers[as_int - MIN_CACHED_INTEGER_VALUE];
			// Should not delete reference when deleted since cached
			is_weak = true;
		}
		else {
			io_ptr = reinterpret_cast<void*>(
				new InternalObject<Aliases::NumT>(construct_from)
			);
			is_weak = false;
			// Not weak; object treated regularly
		}
	} else {
	#endif
		// Always used, regardless of DO_CACHE_DECL
        is_immovable = make_immovable;
		is_weak = make_weak;
		io_ptr = reinterpret_cast<void*>(
			new InternalObject<CorrespondingT>(
					CorrespondingT(std::move(construct_from))
			)
		);
	#ifdef DO_CACHE_DECL
	}
	#endif
	clog << io_ptr << "\n";
}

template <typename CastT>
typename std::remove_reference_t<CastT> ExternalObject::cast() const {
	/*
	You cast to a type and you get that type from the io_ptr
	*/
    // return *reinterpret_cast<std::remove_reference<CastT>>(&get<CastT>());
	return *reinterpret_cast<InternalObject<std::remove_reference_t<CastT>>*>(io_ptr)->stored_value;
}

Aliases::CustomT& ExternalObject::attrs_of() const {
	if (type() == Types::custom)
		return get<Aliases::CustomT>();
	return builtin_dicts[type()];//.get<Aliases::CustomT>();
}


/*
std::optional<ExternalObject> ExternalObject::simple_get_attr(const char* name) const {
	Aliases::CustomT		   attrs	= attrs_of();
	Aliases::CustomT::iterator attrs_it = attrs.find(name);
	if (attrs_it == attrs.end())
		return std::nullopt;
	return attrs_it->second;
}

std::optional<ExternalObject>
ExternalObject::object_get_attr(const char* name) {
	std::optional<ExternalObject> result = simple_get_attr(name);
	if (result) {
		return result;
	}
	if ()
}
*/

ExternalObject& ExternalObject::get_type() const {
	print(attrs_of());
	return attrs_of().at("Type");
}

ExternalObject& ExternalObject::self_get_attr(const char* name) const {
	Aliases::CustomT& attrs = attrs_of();
	Aliases::CustomT::iterator attrs_it  = attrs.find("MRO"),
							   attrs_end = attrs.end();
	/*
    Procedure:
    Check for MRO
        Look in each object in MRO for attr
    Otherwise
        Look in object
    Look in type's MRO (guarantee that type has MRO)
    */

	if (attrs_it == attrs_end) {
		// No method resolution order; look for name
		attrs_it = attrs.find(name);
		if (attrs_it == attrs_end) {
			throw std::runtime_error("oh no");
		} else {
			return attrs_it->second;
		}
	} else {
		// Found MRO, look through it
		// For each item, look for name
		Aliases::CustomT::iterator mro_attrs_it; // attrs_it required in for loop
		Aliases::ArrayT& mro_array = attrs_it->second.get<Aliases::ArrayT>();
		for (const ExternalObject& cls : mro_array) {
			Aliases::CustomT& dummy = cls.attrs_of();
			mro_attrs_it = dummy.find(name);
			if (mro_attrs_it != dummy.cend()) {
				return mro_attrs_it->second;
			}
		}
		// Has MRO but none of the objects contain the attribute being looked for
		throw std::runtime_error("oh no "s + name);
	}
}

void* ExternalObject::get_attr_no_throw(const char* name) {
	print("--Getting attr", name, "from", *this, "with", attrs_of());
	try {
		ExternalObject& result = self_get_attr(name);
        print("--Found directly in object", result.type(), result);
		if (!result.is_weak) ++result.refcount(); // Since returning pointer, does not increase refcount
		// ALWAYS use InternalObject<T>* with ExternalObject to avoid memory leaks
		return result.io_ptr;
	} catch (const std::runtime_error& e) {
		print("--Not in object");
		Aliases::CustomT& attrs = attrs_of();
		try {
			print("Type:", attrs.at("Type").attrs_of());
			ExternalObject& result = attrs.at("Type").self_get_attr(name);
			// Will always have a type
			print("--Looking in type");
			if (result.type() == Types::cpp_function) {
				print("--It's a function!");
				return new InternalObject<FunctionView>(result, *this); // Emplace into InternalObject heap ptr
			}
			return result.io_ptr;
		} catch (const std::exception& e) {
			return nullptr;
		}
	}
}

void* ExternalObject::get_attr(const char* name) {
	void *result = get_attr_no_throw(name);
	if (!result) {
		std::ostringstream oss;
		oss << "Could not find attr " << name << " in " << *this;
		THROW_ERROR(oss.str());
	}
	return result;
}

	/*
	To avoid infinite recursion, return 1 when the original object is encountered
	However, this does not include the original object.
	When the function is called, it does the standard steps but first enters into
	a similar function which also checks if the object is the original one
	*/

size_t ref_count_no_recurse(const ExternalObject& object_to_check, void const* ptr_to_check, void const* current_io_ptr) {
	/*
	ptr_to_check: the original pointer (ptr)
	object_to_check: the object currently being checked, e.g. part of pair or element of array
	current_io_ptr:
	*/
	if (object_to_check.io_ptr != current_io_ptr)
		return object_to_check.ref_count_to<true>(ptr_to_check, nullptr);
	return ptr_to_check == current_io_ptr; // If same, don't recurse, just add that as reference
}

#define MAKE_MET_CONTAINERS \
	bool i_made_lookup = false;\
	if (met_containers == nullptr) {\
		i_made_lookup = true;\
		met_containers = new std::remove_pointer_t<decltype(met_containers)>;\
	}

#define DEL_MET_CONTAINERS if (i_made_lookup) delete met_containers;

template <bool CheckSelf>
size_t ExternalObject::ref_count_to(void const* ptr, std::vector<void const*>* met_containers) const {
	//print(*this, io_ptr, "Type:", type(), ptr == io_ptr, CheckSelf, met_containers ? *met_containers : std::vector<void const*>{});
	if (is_weak) {
		return 0;
	}
	if constexpr(CheckSelf) // Code only exists if true, otherwise deleted
	{
		//print("Checking self", io_ptr, ptr);

		if (io_ptr == ptr) {
			//print("Self; returning 1");
			return 1;
		}
	} else {
		size_t sum = 0;
	}
	/*
	if (met_containers == nullptr) {
		using SetT = std::remove_pointer_t<decltype(met_containers)>;
		std::vector<void const*> _met_containers;
		met_containers = &_met_containers;
		print("Made set");
	}
	*/
	switch (type()) {
		/*
		Needs to keep track of all previously met containers.
		If a container has already been met, there is no point in increasing the
		number of references to it and if it contains itself or the original
		object, it could cause infinite recursion.
		*/


		case Types::dictionary:
		{
			MAKE_MET_CONTAINERS
			met_containers->push_back(io_ptr);
			/*
			for (const auto& pair: get<Aliases::DictT>()) {
				sum += pair.first.ref_count_to<true>(ptr) + pair.second.ref_count_to<true>(ptr);
			}
			*/
			size_t sum = std::accumulate(
				get<Aliases::DictT>().begin(),
				get<Aliases::DictT>().end(),
				size_t(),
				[&](size_t current, const Aliases::PairT& pair) {
					return current + pair.first.ref_count_to<true>(ptr, met_containers) + pair.second.ref_count_to<true>(ptr, met_containers);
				}
			);
			DEL_MET_CONTAINERS
			return sum;
		}
		case Types::custom:
		{
			MAKE_MET_CONTAINERS
			met_containers->push_back(io_ptr); // Insert self as container
			size_t sum = 0;
			//print("Custom:", get<Aliases::CustomT>());
			//print("Cool");
			for (const auto& pair: get<Aliases::CustomT>()) {
				//print("", pair.first, pair.second);
				/*
				if (pair.second.io_ptr == io_ptr)
					++sum;
				else
					sum += pair.second.ref_count_to<true>(ptr);
				*/
				//sum += ref_count_no_recurse(pair.second, ptr, io_ptr);
				if (std::find(met_containers->begin(), met_containers->end(), pair.second.io_ptr) != met_containers->end()) { // If already met
					if (pair.second.io_ptr == ptr && !pair.second.is_weak)
						sum += 1; // Don't go through it again, just know it's there
					else
						sum += 0; // Already met, but not the droids we're looking for
				} else {
					//print(pair.second, pair.second.io_ptr , "not met");
					sum += pair.second.ref_count_to<true>(ptr, met_containers); // Look through the object
				}
			}
			DEL_MET_CONTAINERS
			//print(sum, "in", get<Aliases::CustomT>());
			return sum;
		}
			/*
			return std::accumulate(
				get<Aliases::CustomT>().begin(),
				get<Aliases::CustomT>().end(),
				size_t(),
				[&](size_t current, const Aliases::PairT& pair) {
					print("", pair.first);
					return current + pair.second.ref_count_to<true>(ptr);
				}
			);
			*/
		case Types::array:
		{
			MAKE_MET_CONTAINERS
			//print("Array", get<Aliases::ArrayT>(), ptr);
			met_containers->push_back(io_ptr);
			size_t sum = 0;
			for (const auto& item: get<Aliases::ArrayT>()) {
				/*
				Suppose an array contains an object that references itself

				*/
				/*
				if (item.io_ptr != io_ptr)
					sum += item.ref_count_to<true>(ptr);
				else
					++sum;
				*/
				//sum += ref_count_no_recurse(item, ptr, io_ptr);
				if (std::find(met_containers->begin(), met_containers->end(), item.io_ptr) != met_containers->end()) { // If already met
					if (item.io_ptr == ptr && !item.is_weak)
						sum += 1; // Don't go through it again, just know it's there
					else
						sum += 0;
				} else {
					//print(item, item.io_ptr , "not met");
					sum += item.ref_count_to<true>(ptr, met_containers); // Look through the object
				}
			}
			DEL_MET_CONTAINERS
			//print(sum, "in", get<Aliases::ArrayT>());
			return sum;
		}
			/*
			return std::accumulate(
				get<Aliases::ArrayT>().begin(),
				get<Aliases::ArrayT>().end(),
				size_t(),
				[&](size_t current, const ExternalObject& item ) { return current + item.ref_count_to<true>(ptr); }
			);
			*/
		case Types::pair:
		{
			MAKE_MET_CONTAINERS
			const Aliases::PairT& pair = get<Aliases::PairT>();
			size_t sum = pair.first.ref_count_to<true>(ptr, met_containers) + pair.second.ref_count_to<true>(ptr, met_containers);
			DEL_MET_CONTAINERS
			return sum;
		}
		default:
			return CheckSelf && io_ptr == ptr && !is_weak; // 0: false, 1: true
	}
}

/*
bool ExternalObject::subclass_of(const ExternalObject& cls) const {
	Aliases::ArrayT& mro = ExternalObject(get_type().get_attr("MRO")).get<Aliases::ArrayT>();
	for (auto it = mro.begin(); it != mro.end(); ++it)
		if (it->io_ptr == cls.io_ptr) return true;
	return false;
}

*/
std::optional<Types> ExternalObject::class_get_enum_type() const {
	// First item in MRO that corresponds to a specific enum
	const ExternalObject& my_mro = self_get_attr("MRO");
	Aliases::ArrayT& mro = my_mro.get<Aliases::ArrayT>();
	for (auto it = mro.begin(); it != mro.end(); ++it)
		if (it->type() != Types::custom) return it->type();
	return std::nullopt; // None found
}

Types ExternalObject::get_enum_type() const {
	// Only run this if everything has been set up
	if (type() == Types::custom) {
		const Aliases::CustomT& my_attrs = get<Aliases::CustomT>();
		Aliases::CustomT::const_iterator it;
		if ((it = my_attrs.find("Type")) == my_attrs.cend()) {
			return type();
		}
		/*
		const ExternalObject& my_mro = it->second.self_get_attr("MRO");
		Aliases::ArrayT& mro = my_mro.get<Aliases::ArrayT>();
		for (auto it = mro.begin(); it != mro.end(); ++it)
			if (it->type() != Types::custom) return it->type();
		*/
		std::optional<Types> result = it->second.class_get_enum_type();
		if (result)
			return *result;
	}
	return type();
}

void ExternalObject::weaken() {
	is_weak = true;
}

void ExternalObject::reassign(const ExternalObject& from) {
#define SWITCH_MACRO(Type) \
	{ \
		auto& io_stored_value = get_ptr<Type>()->stored_value; \
		using StoredValueT = std::remove_reference_t<decltype(io_stored_value)>; \
		io_stored_value.~StoredValueT(); \
	}

	print("REASSIGNING", *this);
	GENERATE_SWITCH(type()) // Call destructor of own io_ptr
	io_ptr = from.io_ptr; // This object now references from's
	refcount() += from.refcount(); // References counts are added

#undef SWITCH_MACRO
}

Types ExternalObject::find_custom_enum() const { // Should be called for a class
	decltype(enum_to_class)::const_iterator enum_eo_pair_it;
	const Aliases::ArrayT& mro = self_get_attr("MRO").get<Aliases::ArrayT>();
	print("MRO:", mro, "looking for", io_ptr);
	for (size_t index = 0; index < mro.size(); ++index) {
		// Iterate through MRO
		print(mro[index]);
		for (const decltype(enum_to_class)::value_type& pair : enum_to_class) {
			// Check if current super class is built-in
			// pair.second is a ExternalObject*
			print("   ", pair.second->io_ptr);
			if (pair.second->io_ptr == mro[index].io_ptr) {
				print("Found!", pair.first);
				return pair.first;
			}
		}
	}
	print("Not found...");
	// Last object is Object which is found in enum_to_class, so will always return
}

#endif
