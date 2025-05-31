#pragma once

#include "../classes/obj_g.h"

#include "obj_a_mock.h"
#include "obj_d_mock.h"
#include "obj_f_mock.h"

#include <MockVendor/MockVendor.h>

#include <gmock/gmock.h>


class ObjGMock : public ObjAMock, public ObjDMock, public ObjFMock
{
    // Extended class mocks mirror the inheritance structure of the
    // object they are mocking

public: // Methods
    MOCK_METHOD(void, obj_g_func, ());
};

using ObjGMockVendor = MockVendor<ObjGMock, ObjG, ObjAMockVendor, ObjDMockVendor, ObjFMockVendor>;


ObjG::ObjG()
{
    // Ignore any constructor parameters...
    ObjGMockVendor::vend(this);
}

ObjG::~ObjG()
{
    ObjGMockVendor::destroy(this);
}

void ObjG::obj_g_func()
{
    return ObjGMockVendor::mock(this)->obj_g_func();
}
