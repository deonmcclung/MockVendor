# MockVendor
A unit testing utility for vending google mock files to the tests.

The objective of this library is to provide mock support to otherwise "concrete" classes. This
library, along with its usage paradigm, enables the user to mock nearly any single inheritance
class. The user can also instanciate and access that mock regardless of how the class instance
is instatiated.

By default, MockVendor will vend "nice" mocks that respond in default ways to return values.
The user can alter exact nature of this default when creating the mock source file. If a test
needs something other than default behavior, the test can push altered mocks onto the queue
for the class. Mocks are vended from the queue as the software under test attempts to construct
instances. A test cannot be completely agnostic to class usage. If multiple instances are created
over the lifetime of the test, then those instances are vended (popped off the queue) in the order
they are created, and the test must understand that order.

Usage:

    // MyClass.h (Real header file)
    #pragma once

    class MyClass
    {
    public:
        // Only mock the public functions

        // Note: Public functions must not be inlined (including the constructors)
        // since they connot be replaced by a mock.

        MyClass();
        virtual ~MyClass();

        int myFunc1(int arg);

        void myFunc2();

        bool myFunc3() const;

    private:
        // ... (not relevent to the mock)
    };

--------------------------------------------------------------------------------------------

    // MyClassMock.h

    #pragma once

    #include "MockVendor.h"

    // Include the real header file...
    #include "MyClass.h"

    class MyClassMock
    {
    protected:
        MyClassMock() = default;
        virtual ~MyClassMock() = default;

        MOCK_METHOD(int, myFunc1, (int arg)));
        MOCK_METHOD(void, myFunc2, ());
        MOCK_METHOD(bool, myFunc3, (), (const));
    };

    // The MockVendor takes for template arguments:
    // 1. The Mock Class (required)
    // 2. The Real Class (required)
    // 3. Mocked Base Class (optional) - if the class has a mocked base class
    // 4. Real Base Class (req if has #3) - if the class has a mocked base class
    using MyClassMockVendor = MockVendor<MyClassMock, MyClass>;

    // Replace the implementation functions with the following pattern:

    MyClass::MyClass()
    {
        MyClassMockVendor::vend(this);
    }

    MyClass::~MyClass()
    {
        MyClassMockVendor::destroy(this);
    }

    int MyClass::myFunc1(int arg)
    {
        return MyClassMockVendor::mock(this)->myFunc1(arg);
    }

    void MyClass:myFunc2()
    {
        // Return not required in void, but allowed and makes pattern consistent.
        return MyClassMockVendor::mock(this)->myFunc2();
    }

    bool MyClass::myFunc3() const
    {
        return MyClassMockVendor::mock(this)->myFunc3();
    }

--------------------------------------------------------------------------------------------


    // MyOtherClassTests.cpp

    // Include mock file(s)
    #include "MyClassMock.h"

    #include <gtest/gtest.h>

    TEST(MyOtherClassTest1)
    {
        // To use all default behavior from functions, no action is required
        // around MyClassMock. Default behavior mocks will be automatically created

        MyOtherClass testObj;     // Create a class that uses MyClass.
    }

    TEST(MyOtherClassTest2)
    {
        // To use non-default expectations, queue mocks (of desired niceness) onto the
        // MockVendor in creation order.

        MyClassVendor myClassVendor;
        auto myClassMock = std::make_shared<testing::NiceMock<MyClassMock>>;
        ON_CALL(*myClassMock, func3()).WillByDefault(testing::Return(true));
        myClassVendor.queueMock(myClassMock);

        // Create a class that uses MyClass.
        // When MyClass is constructed, it will use myClassMock above that has altered
        // behavior/expectations.
        MyOtherClass testObj;
    }

--------------------------------------------------------------------------------------------

# Release Notes:

## v1.0.0
 - Initial release
 - Add MPL 2.0 License
