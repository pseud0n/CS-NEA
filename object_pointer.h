#ifndef OBJECT_POINTER_H_
#define OBJECT_POINTER_H_

#define OPTR UL::ObjectPointer

namespace UL {
    struct Object;

    class ObjectPointer { // Should be used instead of object pointers
    friend std::ostream& operator <<(std::ostream&, ObjectPointer);
    private:
        // This type should be predominantly used on the stack
        Object* object_ptr; // The associated object

    public:
        bool is_weak;
        ObjectPointer();
        ObjectPointer(Object*, bool=false);
        ObjectPointer(double, bool=false);
        ObjectPointer(int, bool=false);
        template <typename ConstructorT> ObjectPointer(ConstructorT, bool=false);
        ~ObjectPointer();
        ObjectPointer(const ObjectPointer&, bool=false);
        ObjectPointer operator ()(const std::vector<OPTR>&);
        ObjectPointer operator ()();
        template <typename ConstructFromT> void create_from_blank(ConstructFromT, bool=false);
        template <typename CastT> typename std::remove_reference<CastT>::type cast() const;
    };
}

#endif
