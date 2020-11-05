#ifndef OBJECT_POINTER_H_
#define OBJECT_POINTER_H_

#define OPTR UL::ObjectPointer

namespace UL {
    class Object;

    class ObjectPointer { // Should be used instead of object pointers
    friend std::ostream& operator <<(std::ostream&, ObjectPointer);
    private:
        // This type should be predominantly used on the stack

    public:
        Object* object_ptr; // The associated object
        bool is_weak;
        ObjectPointer();
        ObjectPointer(double, bool=true); // Where boolean corresponds to constness
        ObjectPointer(int, bool=true);
        ObjectPointer(std::nullptr_t, bool=true);
        // Weakness value is ignored for all of the above since the pointers are always weak
        // The default value is irrelevant but needs to work if optionally passed
        ObjectPointer(Object*, bool=true);
        template <typename ConstructorT> ObjectPointer(ConstructorT, bool=true);
        ObjectPointer(const ObjectPointer&, bool=true);
        ~ObjectPointer();
        ObjectPointer operator ()(const std::vector<Object*>&);
        ObjectPointer operator ()();
        template <typename ConstructFromT> void create_from_blank(ConstructFromT, bool=true);
        template <typename CastT> typename std::remove_reference<CastT>::type cast() const;
        operator Object* () const; // For ease of use
    };
}

#endif
