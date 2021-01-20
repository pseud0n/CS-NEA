#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

/*
Exceptions have no specific instruction or keyword to make them run like you may
find in other languages.

When a bytecode instruction results in an error, the top scope is safely deleted
and the error is set to whatever was raised. Scopes are continually deleted
until a scope is found which has a catch for that specific error. Code blocks
have a list of exceptions that it will catch and corresponding code blocks to
run. An exception can also be called from a built-in function, which will
do the same procedure. If the last scope is deleted, the program exits with a
failure.
*/

#define EXC_CLASS(name, base) struct name : base { \
    name() { message = "<No message>"; } \
    name(const char* msg) { message = msg; } \
    void operator()() const { cerr << #name << " : " << message << "\n"; }};

//namespace UL {

    struct VirtualExc {
        const char *message;
        //virtual ~VirtualExc() = 0;
        virtual void operator ()() const = 0;
    };

    EXC_CLASS(BaseExc, VirtualExc)
        EXC_CLASS(RunTimeExc, BaseExc)
            EXC_CLASS(ArgExc, RunTimeExc)
        EXC_CLASS(CompileTimeExc, BaseExc)
            EXC_CLASS(APIExc, CompileTimeExc)

    
    
    /*
    EXC_CLASS(BaseExc, VirtualExc)
    EXC_CLASS(RunTimeExc, BaseExc)
    EXC_CLASS(ArgExc, BaseExc)
    */

    //Expands to

    /*
    struct BaseExc : public VirtualExc {
        BaseExc() { message = "<No message>"; }
        BaseExc(const char* msg) { message =  msg; }
        void operator ()() { cerr << "BaseExc" << ": " << message << "\n"; }
    };

    struct RuntimeExc : public BaseExc {
        RuntimeExc() { message = "<No message>"; }
        RuntimeExc(const char* msg) { message =  msg; }
        void operator ()() { cerr << "RunTimeExc" << ": " << message << "\n"; }
    };

    struct ArgExc : public RuntimeExc {
        ArgExc() { message = "<No message>"; }
        ArgExc(const char* msg) { message =  msg; }
        void operator ()() { cerr << "ArgExc" << ": " << message << "\n"; }
    };
    */
//}

#undef EXC_CLASS


#endif
