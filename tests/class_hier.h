////////////////////////////////////////////////////////////////////////////////
#ifndef CLASS_HIER_H
#define CLASS_HIER_H

TEST_REPR("Create a simple class hierarchy");

/*
Define the default 'Class' type
    init(*bases->Class, :attrs->String:Any) -> Class
        Returns a new class which can be instantiated
        > c3 = Class(c1, c2, 'foo': 'bar')
    mro(self) -> Array(Class)
        Returns [Class]
        > myClass = Class()
        > Assert(myClass.mro() == Class.mro(myClass))
        > Assert(Class.mro() == Class.mro(Class))
		All subclasses of Class have an MRO because it's defined in Class
		(and all of its subclasses)

    getattr(self, name->String) -> Any
        Returns a member of the class
        > c1 = Class('foo': 10)
        > c2 = Class(c1, 'bar': 5)
        > Assert(c2.foo == c2.getattr('foo'))
        This method access transcends the normal lookup routine
        Check for 'getattr' in the object & its superclasses (and pass as object.member(*args))
        Check for 'getatrr' in the object's type and their superclasses (and pass as object.type().member(object, *args))

        Object.getattr states that an name should be searched for in this same way.

        
        
c3 = Class(c1, c2, 'foo': 'bar')
c3.mro()
=> Class.mro(c3)

*/

//std::vector<Object*> optional_arguments, bool is_variadic, std::function<OPTR(CppFunction*, const std::vector<Object*>&)> func
auto ch_object = new UL::Object();
/*
*/
ch_object->attrs.emplace("ayy", "lmao"s); // (OPTR)
ch_object->attrs.emplace("Getattr", OPTR(new UL::Object(new UL::CppFunction(
    {}, false, DY_LMBD {
        UL::Object *lookup_object; // What to look up the object in
        std::string lookup_name;
		
        if (!argument_data->assign_args<2>(arguments, lookup_object, lookup_name))
            // Function takes 2 arguments: the object to look in initially and the name to find.
            return UL::null_optr;
        // Guaranteed that every class has an 'mro' and every object has a 'type' attribute
        // No guarantee that object's 'supers' will be a valid pointer
        //if (lookup_name == "Getattr")

        UL::Object::AttrsT::iterator
            lo_end 				= lookup_object->attrs.end(),
            object_dict_iter 	= lookup_object->attrs.find(lookup_name);

        if (object_dict_iter == lo_end) {
            print("FOUND DIRECTLY IN OBJECT");
            return object_dict_iter->second; // Returns OPTR
        }

		/*
		Next, it looks in the object's superclasses.
		However, this requires that the object is class.
		Whether an object is a type can be determined by if its type constains
		the MRO attribute. A class may also be an instance of a metaclass (so
		its type is an instance of Class).
		The search for the attribute is only as deep as checking if the
		name exists directly in the object.
		*/

        const OPTR *optr_ptr = lookup_object->get_class_optrptr_if_present(lookup_name);
		// NULL if MRO not found or if 
		if (optr_ptr) {
			print("FOUND IN CLASS (OR MRO)");
			// If: checks if `optr_ptr == (OPTR*)0` (NULL is False)
			return *optr_ptr;
			// Return the OPTR that was found in the class' MRO
        }

		/*
		It is not directly contained in the class' attrs map and it's also
		either not a class or the class' MRO does not have any objects which
		directly contain the name.
		Now, it looks in the type of the object. For a class this type is
		the 'Class' class.
		There is a guarantee that 'MRO' will be in any object's type.
		*/

		optr_ptr = lookup_object->get_attr("type").object_ptr->get_class_optrptr_if_present(lookup_name);
		if (optr_ptr) {
			print("FOUND IN CLASS' MRO");
			// If: checks if `optr_ptr == (OPTR*)0` (NULL is False)
			return *optr_ptr;
			// Return the OPTR that was found in the class' MRO
        }

		print("COULD NOT FIND ATTR", lookup_name);

        return UL::null_optr;
		}
	)))
);


auto ch_class = new UL::Object(); // Class

ch_object->attrs.emplace("Class", ch_class); // Object (the class) is an instance of Class


auto ch_foo = new UL::Object(); // an example class, foo
ch_object->attrs.emplace("Class", ch_class); // It's a class so it's class is instance
ch_foo->attrs.emplace("foo", 10);

//print(ch_foo->getattr("foo"));

auto ch_foo_inst = new UL::Object();

//print(ch_foo_inst->getattr("foo"));


/*

*/

#endif