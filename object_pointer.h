#ifndef OBJECT_POINTER_H
#define OBJECT_POINTER_H

//Included("object_pointer");

#define OPTR UL::ObjectPointer

namespace UL {
    class Object;

    class ObjectPointer { // Should be used instead of object pointers
    friend std::ostream& operator <<(std::ostream&, ObjectPointer);
    private:
        // This type should be predominantly used on the stack

    public:
		//static bool operator ==(const ObjectPointer& o1, const ObjectPointer& o2);

        Object* object_ptr; // 8 bytes
        bool is_weak;		// 1 byte
		
		static size_t hash (const ObjectPointer& o1, const ObjectPointer& o2);

        ObjectPointer();
        ObjectPointer(double, bool=true); // Where boolean corresponds to constness
        ObjectPointer(int, bool=true);
        ObjectPointer(std::nullptr_t, bool=true);
        // Weakness value is ignored for all of the above since the pointers are always weak
        // The default value is irrelevant but needs to work if optionally passed
        ObjectPointer(Object*, bool=true);
        template <typename ConstructorT> ObjectPointer(ConstructorT, bool=true);
        ObjectPointer(const ObjectPointer&, bool=true); // Copy constructor
        ObjectPointer(ObjectPointer&&) noexcept; // Move constructor
        ~ObjectPointer();

        ObjectPointer operator ()(const std::vector<Object*>&);
        ObjectPointer operator ()();
        Object& operator *() const;
		Object* operator ->() const;
		bool operator ==(const ObjectPointer&);

        template <typename ConstructFromT> void create_from_blank(const ConstructFromT&, bool=true);
        template <typename CastT> typename std::remove_reference<CastT>::type cast() const;
		ObjectPointer& operator=(ObjectPointer other);
		// https://en.cppreference.com/w/cpp/language/copy_assignment

    };
}

#endif
