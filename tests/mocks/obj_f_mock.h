#pragma once

#include "../classes/obj_f.h"

#include <MockVendor/MockVendor.h>

#include <gmock/gmock.h>


class ObjFMock
{
public: // Methods
    MOCK_METHOD(void, obj_f_func, ());
};

using ObjFMockVendor = MockVendor<ObjFMock, ObjF>;


ObjF::ObjF()
{
    // Ignore any constructor parameters...
    ObjFMockVendor::vend(this);
}

ObjF::~ObjF()
{
    ObjFMockVendor::destroy(this);
}

void ObjF::obj_f_func()
{
    return ObjFMockVendor::mock(this)->obj_f_func();
}
