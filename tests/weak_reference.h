TEST_REPR("Create 5 objects to check reference counting and weak references");
UL::Object *object1 = new UL::Object(99);
UL::Object *object2 = new UL::Object("foo");
UL::Object *object3 = new UL::Object("bar");
UL::Object *object4 = new UL::Object(object1);
UL::Object *object5 = new UL::Object(object1, true); //weak
cout << UL::Location::show_instances() << "\n";
//cout << "Initialised\n";
cout << *object1 << ", " << *object2 << ", " << *object3 << ", " << *object4 << ", " << *object5 << "\n";
delete object1;
cout << UL::Location::show_instances() << "\n";
delete object4;
cout << UL::Location::show_instances() << "\n";