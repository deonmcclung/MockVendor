#pragma once

#include "../classes/obj_d.h"

#include <MockVendor/MockVendor.h>

#include <gmock/gmock.h>


class ObjDMock
{
public: // Methods
    MOCK_METHOD(void, obj_d_func, ());
};

using ObjDMockVendor = MockVendor<ObjDMock, ObjD>;

// This is a mock implementation of ObjD.

ObjD::ObjD()
{
    // Ignore any constructor parameters...
    ObjDMockVendor::vend(this);
}

ObjD::~ObjD()
{
    ObjDMockVendor::destroy(this);
}

void ObjD::obj_d_func()
{
    return ObjDMockVendor::mock(this)->obj_d_func();
}
