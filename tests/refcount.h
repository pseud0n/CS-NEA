TEST_REPR("Testing reference counting, copying, etc.");

UL::ExternalObject o1 = "hello"s;
print(o1.refcount());
UL::ExternalObject o2 = o1;
print(o1.io_ptr, o2.io_ptr);
print(o1, o2);