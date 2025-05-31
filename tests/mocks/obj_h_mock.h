#pragma once

#include "../classes/obj_h.h"

#include "obj_b_mock.h"
#include "obj_c_mock.h"
#include "obj_d_mock.h"

#include <MockVendor/MockVendor.h>

#include <gmock/gmock.h>


class ObjHMock : public ObjDMock, public ObjCMock, public ObjFMock
{
    // Extended class mocks mirror the inheritance structure of the
    // object they are mocking

public: // Methods
    MOCK_METHOD(void, obj_h_func, ());
};

using ObjHMockVendor = MockVendor<ObjHMock, ObjH, ObjDMockVendor, ObjCMockVendor, ObjFMockVendor>;


ObjH::ObjH()
{
    // Ignore any constructor parameters...
    ObjHMockVendor::vend(this);
}

ObjH::~ObjH()
{
    ObjHMockVendor::destroy(this);
}

void ObjH::obj_h_func()
{
    return ObjHMockVendor::mock(this)->obj_h_func();
}
