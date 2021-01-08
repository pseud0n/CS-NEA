////////////////////////////////////////////////////////////////////////////////
#ifndef EXTERNAL_OBJECT_H
#define EXTERNAL_OBJECT_H

struct ExternalObject {
	bool is_weak;
	void *io_ptr;

	template <typename... Ts>
	static void* make_array(Ts...);
	
	template <typename K, typename V>
	static void* make_pair(K, V);
	
	template <typename K, typename V, typename... KVs>
	static void make_dict_helper(Aliases::DictT&, const K&, const V&, const KVs&...);
	
	static void make_dict_helper(Aliases::DictT&);
	
	template <typename... KVs>
	static void* make_dict(const KVs&...);

	
	static ExternalObject blank_object() {
		ExternalObject object;
		object.io_ptr = new InternalObject<Aliases::CustomT>;
		return object;
	}
	

	ExternalObject(); // Default constructor

	ExternalObject(void*, bool=false); // Construct from internal object
	ExternalObject& operator =(void*);

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
	ExternalObject& operator =(const ExternalObject&); // Move assignment

	template <typename T> ExternalObject(T, bool=false);
	template <typename T> ExternalObject& operator =(T);

	~ExternalObject();
	
	template <typename T> T& get() const;
	Types type() const;
	InternalObject<>::RefCountT& refcount() const;
	//void ExternalObject::create_from_blank(const T& construct_from);
	template <typename T> void create_from_blank(T, bool=false);
	template <typename CastT> typename std::remove_reference_t<CastT> cast() const;
	
};

/*
Suppose you wanted to construct from int:
	GetAssociatedData<GetCorrespondingType<int>>::enum_t = Types::number
	GetAssociatedData<GetCorrespondingType<int>>::is_immovable = true
*/

#endif