#ifndef EXTERNAL_OBJECT_CPP
#define EXTERNAL_OBJECT_CPP

template <typename... Ts>
void* ExternalObject::make_array(Ts... construct_from) {
	auto temp = new InternalObject<Aliases::ArrayT>(Aliases::ArrayT(sizeof...(Ts)));
	size_t i = 0;
	((
		[&](){
			temp->stored_value[i++] = construct_from;
			//print(construct_from, i, temp->stored_value);
		}()
	), ...);
	return reinterpret_cast<void*>(temp);
	// Controlled by ExternalObject so will get deleted correctly
}

template <typename K, typename V>
void* ExternalObject::make_pair(K key, V value) {
	auto temp = new InternalObject<Aliases::PairT>();
	temp->stored_value.first 	= key;
	temp->stored_value.second 	= value;
	return (void*)temp;
}

template <typename K, typename V, typename... KVs>
void ExternalObject::make_dict_helper(Aliases::DictT& u_map_ref, const K& key, const V& value, const KVs&... Ks) {
	//cout << "Emplacing " << key << ":" << value << "\n";
	u_map_ref.emplace(key, value);
	make_dict_helper(u_map_ref, Ks...);
}

void ExternalObject::make_dict_helper(Aliases::DictT&) { // Does absolutely nothing! (sort of)
}

template <typename... KVs>
void* ExternalObject::make_dict(const KVs&... Ks) {
	auto temp = new InternalObject<Aliases::DictT>;
	make_dict_helper(temp->stored_value, Ks...);
	return reinterpret_cast<void*>(temp);
}

ExternalObject ExternalObject::blank_object() {
	ExternalObject object;
	auto new_io_ptr = new InternalObject<Aliases::CustomT>;
	//new_io_ptr.stored_value.emplace()
	cout << "BLANK OBJECT!\n";
	exit(0);
	
	return object;
}

ExternalObject::ExternalObject() : is_weak(true), io_ptr((void*)0) {
	cout << "Constructing new Null ExternalObject\n";
}

ExternalObject::ExternalObject(void* internal_object_ptr, bool)
	: is_weak(false), io_ptr(internal_object_ptr) { // Cannot be weak if from original
	print("Constructing from InternalObject pointer", io_ptr);
}

ExternalObject& ExternalObject::operator =(void* internal_object_ptr) {
	print("Assigning from InternalObject pointer", internal_object_ptr);
	is_weak = false;
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

ExternalObject::ExternalObject(const ExternalObject& copy_from, bool make_weak) {
	print("Copying from", copy_from);
	/*
	is_weak		make_weak		result
	false		false			false
	false		true			true
	true		false			true
	true		true			true		
	*/
	is_weak = copy_from.is_weak || make_weak;
	io_ptr = copy_from.io_ptr;
	if (io_ptr && !is_weak) {// Also increments references for cached objects
		++copy_from.refcount(); // Since method returns lvalue
		print("Incref'ing", *this, "to", refcount());
	}
}

ExternalObject& ExternalObject::operator =(const ExternalObject& copy_from) {
	print("Copy-assigning from", copy_from);
	is_weak = copy_from.is_weak;
	io_ptr = copy_from.io_ptr;
	if (io_ptr) {
		++refcount();
		print("Incref'ing", *this, "to", refcount());
	}
	return *this;
}

ExternalObject::ExternalObject(ExternalObject&& move_from) noexcept {
	print("Moving from", move_from);
	io_ptr = std::exchange(move_from.io_ptr, (void*)0);
	is_weak = move_from.is_weak;
	// The old object won't be deleted since it has a 0 pointer
}

ExternalObject& ExternalObject::operator =(ExternalObject&& move_from) noexcept {
	print("Move-assigning from", move_from);
	io_ptr = std::exchange(move_from.io_ptr, (void*)0);
	is_weak = move_from.is_weak;
	// The old object won't be deleted since it has a 0 pointer
	return *this;
}

#define DEL_IO_PTR(T) delete reinterpret_cast<UL::InternalObject<Aliases::T>*>(io_ptr); break;
ExternalObject::~ExternalObject() {
	/*
	#define SWITCH_MACRO(T) delete (UL::InternalObject<T>*)io_ptr
	GENERATE_SWITCH(type())
	#undef SWITCH_MACRO
	*/

	/*
	When deleting, the reference count of an object should be reduced.
	Depending on the value of is_weak, it may or may not decrement.
	Weak:
		NullT (always)
		BoolT (if caching turned on)
		NumT (if caching turned on an in range)
	*/
	if(io_ptr) {
		if (!is_weak) {
			// Guarantee of valid pointer; null: is_weak = true
			// Pointer is only invalid when object is null
			print("\tDecref'ing", *this, "to", refcount() - 1);
			if(--refcount()) return;
			// References still remain to underlying object
		} else {
			print("\tWeak; ignoring");
			return;
		}
		//print("Reduced reference count to", refcount());
		switch (type()) {
			print("Deleting ExternalObject:", *this);
			case Types::number:
				DEL_IO_PTR(NumT);
				{
					/*
					#ifdef DO_CACHE_DECL
					auto num_reference = get<Aliases::NumT>();
					if (num_reference > Cache::min && num_reference <= Cache::max)
						break;
					#endif // If no caching, always deletes; otherwise breaks if cached
					*/
					// If weak/cached, already caught and returned
				}
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
			case Types::custom:
				DEL_IO_PTR(CustomT)
			default:
				print("DID NOT DELETE", type(), io_ptr);
		}
	} else {
		print("Ignoring null io_ptr");
	}

}
#undef DEL_IO_PTR

template <typename T>
ExternalObject::ExternalObject(T construct_from, bool is_weak) {
	/*
	The 'type' of an InternalObject is the type it is constructed from.
	Object may be copied or moved into construct_from, but is never
	copied after that
	*/
	create_from_blank<T>(std::move(construct_from), is_weak);
}

template <typename T>
ExternalObject& ExternalObject::operator =(T construct_from) {
	print("Assigning from", construct_from);
	create_from_blank<T>(std::move(construct_from));
	return *this;
}


template <typename T>
T& ExternalObject::get() const {
	return ((InternalObject<T>*)io_ptr)->stored_value;
}

Types ExternalObject::type() const {
	//if (!io_ptr) return Types::null;
	return io_ptr ? *reinterpret_cast<Types*>(io_ptr) : Types::null;
	// Since this is the first 4 bytes of the object
}


InternalObject<>::RefCountT& ExternalObject::refcount() const {
	return *(InternalObject<>::RefCountT*)((Types*)io_ptr + 1);
	// +1: adds sizeof(Types) bytes to get to bytes for refcount 
} // We can use this as an lvalue and modify in-place

template <typename T>
void ExternalObject::create_from_blank(T construct_from, bool make_weak) {
	/*
	Turns an ExternalObject with io_ptr 0 into a normal object
	*/
	/*
	io_ptr = (void*)(
		new InternalObject<GetCorrespondingType<T>>(
			(
				GetCorrespondingType<T>(std::move(construct_from))
			)
		)
	);
	*/

	/*
	Below:
	
	If DO_CACHE_DECL, then check if it's a cacheable type.
	If it's not, 'else' into the standard procedure
	
	If undefined, then go straight to standard procedure
	*/

	cout << "Constructing new " << AssociatedData<GetCorrespondingType<T>>::enum_type << " ";
	if constexpr(std::is_same_v<GetCorrespondingType<T>, std::nullptr_t>) {
		io_ptr = 0;
		cout << "0\n";
		return;
	}
	#ifdef DO_CACHE_DECL
	if constexpr(std::is_same_v<GetCorrespondingType<T>, Aliases::BoolT>) {
		io_ptr = construct_from ? Cache::bool_true : Cache::bool_false;
		is_weak = true; // No reference counting involved
	} else if constexpr (std::is_same_v<GetCorrespondingType<T>, Aliases::NumT>) {
		int as_int = static_cast<int>(construct_from);
		// C++-style casts!
		// Values may be negative so signed is used
		if (as_int >= MIN_CACHED_INTEGER_VALUE && as_int <= MAX_CACHED_INTEGER_VALUE) {
			io_ptr = Cache::numbers[as_int - MIN_CACHED_INTEGER_VALUE];
			is_weak = true;
			// Should not delete reference when deleted since cached
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
		is_weak = make_weak;
		io_ptr = reinterpret_cast<void*>(
			new InternalObject<GetCorrespondingType<T>>(
					GetCorrespondingType<T>(std::move(construct_from))
			)
		);
	#ifdef DO_CACHE_DECL
	}
	#endif
	cout << io_ptr << "\n";
}

template <typename CastT>
typename std::remove_reference_t<CastT> ExternalObject::cast() const {
	/*
	You cast to a type and you get that type from the io_ptr
	*/
	return ( (InternalObject<std::remove_reference_t<CastT>>*)io_ptr )->stored_value;
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
*/



ExternalObject ExternalObject::get_attr(const char* name) {
	Aliases::CustomT&		   attrs   = attrs_of();
	Aliases::CustomT::iterator attr_it = attrs.find(name);
	print("-Looking for attr", name);
	print("-Lookin in", attrs);
	if (attr_it == attrs.end()) {
		// Now look in type
		attrs = attrs.at("Type").get<Aliases::CustomT>();
		Aliases::CustomT::iterator type_attr_it = attrs.find(name);
		print("-Look in class", attrs);
		if (type_attr_it == attrs.end()) {
			print("-Not found!");
			return nullptr;
		} else {
			print("-Found!");
			if (type_attr_it->second.type() == Types::cpp_function) {
				print("-It was a function!");
				auto fv = FunctionView(&type_attr_it->second.get<Aliases::CppFunctionT>(), *this);
				return fv;
			}
			return type_attr_it->second;
		}
	} else {
		// Found attribute
		print("-Found in object");
		return attr_it->second;
	}
	return nullptr;
}


#endif