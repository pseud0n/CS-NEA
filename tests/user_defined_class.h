TEST_REPR("Create a custom data type");

auto UL::UserDefinedObject(
    // A class is an instance of `Class`. A meta class inherits from `Class`
    {
        "name"  : "Person",
        "type"  : Classes::Class,
        "bases" : { },
    }
)