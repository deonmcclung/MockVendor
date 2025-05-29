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

using ObjBMockVendor = MockVendor<ObjBMock, ObjB, ObjAMockVendor>;


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
