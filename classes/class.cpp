#ifndef CLASS_CPP
#define CLASS_CPP

/*
A builtin class is constructed using a switch in the Class class. When a new
class is created, it calls the constructor for Class' class, which is Class.

When an object without a non-default metaclass is constructed, it calls Class'
Call method, which activates a switch to find the right object, then constructs
an ExternalObject from the arguments.

The call for, e.g. List(*args) becomes List.Class.Call(List, *args), and List's
Class is Class, so it calls Class.Call(List, *args). A switch is performed on
the first argument to find the appropriate constructor.

The call for constructing a class would be Class(*args, :kw). It looks in Class'
type (which is Class again) for a Call method, to get
Class.Call(Class, *args, :kw). Included in the switch is an option for class.

When a function is created, there is the same process. It looks in Class for a
Call method and calls it to make Fn(*args) into Fn.Class.Call(Fn, *args) which
becomes Class.Call(Fn, *args). When turning Fn->Function, it calls the Amul
method: header{ print("Hey!"); } becomes Fn.Amul(header, { print("Hey!"); }).
Fn has a lookup which needs no switch because it can know the type it will be
next to already. It then returns the appropriate object using the header and
body.
*/
namespace Classes {
}

#endif