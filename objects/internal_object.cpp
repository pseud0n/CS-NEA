#ifndef INTERNAL_OBJECT_CPP
#define INTERNAL_OBJECT_CPP

template <typename StoredT>
template <typename... CtorTs>
void InternalObject<StoredT>::emplace(CtorTs&&... args) {
	/*
	auto io_ptr = new InternalObject<StoredT>;
	io_ptr.type = AssociatedData<StoredT>::enum_type
	reference_count = 1;
	io_ptr.is_immovable = AssociatedData<StoredT>::is_immovable
	*/

	print("In InternalObject Ctor");
	stored_value.emplace(std::forward<CtorTs>(args)...);
}

template <typename StoredT>
InternalObject<StoredT>::InternalObject()
	: type(AssociatedData<StoredT>::enum_type), reference_count(1),
		is_immovable(AssociatedData<StoredT>::is_immovable),
		stored_value(std::nullopt) {}

template <typename StoredT>
InternalObject<StoredT>::InternalObject(StoredT construct_from) // e.g. std::string
/*
	: type(Corresponding<StoredT>::enum_type), is_immovable(Corresponding<StoredT>::is_immovable), reference_count(1), attrs((AttrsT*)0), stored_value(std::move(construct_from)) */{
	type = AssociatedData<StoredT>::enum_type;
	reference_count = 1;
	is_immovable = AssociatedData<StoredT>::is_immovable;
	stored_value = std::forward<StoredT>(construct_from);
	/*
	If a large object like a long string is passed in, it should be
	moved, not copied.

	If we construct from a new dictionary, that can be moved when
	passed in to the function, or copied otherwise
	*/
}



template <typename StoredT>
InternalObject<StoredT>::~InternalObject() {
	//if (attrs) delete attrs; // If dictionary exists, delete it
};

#endif