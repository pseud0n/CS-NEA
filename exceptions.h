#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#define EXC_CLASS(name, base) struct name : base { \
    name() { message = "<No message>"; } \
    name(const char* msg) { message = msg; } \
    void operator()() const { cerr << #name << " : " << message << "\n"; }};

namespace UL {

    struct VirtualExc {
        const char *message;
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
}

#undef EXC_CLASS

#endif