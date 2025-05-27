#pragma once

#include "../classes/obj_c.h"

#include <MockVendor/MockVendor.h>

#include <gmock/gmock.h>


class ObjCMock : public ObjBMock
{
    // Extended class mocks mirror the inheritance structure of the
    // object they are mocking

public: // Methods
    MOCK_METHOD(void, obj_c_func, ());
    MOCK_METHOD(int, obj_b_v_func, ());
};

using ObjCMockVendor = MockVendor<ObjCMock, ObjC>;

// Establish a link that says ObjC extends ObjB (which extends ObjA).
// We must actually declare a global object of this type.
// At present, only one base class is supported (future will support more)
ObjCMockVendor::BaseLink<ObjBMock, ObjB> g_obj_c_obj_b_link;


// This is a mock implementation of ObjA.

ObjC::ObjC()
{
    // Ignore any constructor parameters...
    ObjCMockVendor::vend(this);
}

ObjC::~ObjC()
{
    ObjCMockVendor::destroy(this);
}

void ObjC::obj_c_func()
{
    return ObjCMockVendor::mock(this)->obj_c_func();
}

int ObjC::obj_b_v_func()
{
    // This overrides the function at the B-level, but not easy to notice
    // since they are the same signature, but this does call the function on
    // the ObjCMockVendor.
    return ObjCMockVendor::mock(this)->obj_b_v_func();
}

