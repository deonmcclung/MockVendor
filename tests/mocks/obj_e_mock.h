#pragma once

#include "../classes/obj_e.h"

#include "obj_a_mock.h"
#include "obj_d_mock.h"

#include <MockVendor/MockVendor.h>

#include <gmock/gmock.h>


class ObjEMock : public ObjAMock, public ObjDMock
{
    // Extended class mocks mirror the inheritance structure of the
    // object they are mocking

public: // Methods
    MOCK_METHOD(void, obj_e_func, ());
};

using ObjEMockVendor = MockVendor<ObjEMock, ObjE, ObjAMockVendor, ObjDMockVendor>;


ObjE::ObjE()
{
    // Ignore any constructor parameters...
    ObjEMockVendor::vend(this);
}

ObjE::~ObjE()
{
    ObjEMockVendor::destroy(this);
}

void ObjE::obj_e_func()
{
    return ObjEMockVendor::mock(this)->obj_e_func();
}
