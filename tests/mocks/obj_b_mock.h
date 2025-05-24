#pragma once

#include "../classes/obj_b.h"

#include <MockVendor/MockVendor.h>

#include <gmock/gmock.h>


class ObjBMock : public ObjAMock
{
    // Extended class mocks mirror the inheritance structure of the
    // object they are mocking

public: // Methods
    MOCK_METHOD(int, obj_b_func, ());
    MOCK_METHOD(int, obj_b_v_func, ());
};

using ObjBMockVendor = MockVendor<ObjBMock, ObjB>;

// Establish a link that says ObjB extends ObjA.
// We must actually declare a global object of this type.
// At present, only one base class is supported (future will support more)
ObjBMockVendor::BaseLink<ObjAMock, ObjA> g_obj_b_obj_a_link;


// This is a mock implementation of ObjA.

ObjB::ObjB()
{
    // Ignore any constructor parameters...
    ObjBMockVendor::vend(this);
}

ObjB::~ObjB()
{
    ObjBMockVendor::destroy(this);
}

int ObjB::obj_b_func()
{
    return ObjBMockVendor::mock(this)->obj_b_func();
}

int ObjB::obj_b_v_func()
{
    return ObjBMockVendor::mock(this)->obj_b_v_func();
}

