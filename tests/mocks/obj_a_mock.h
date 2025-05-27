#pragma once

#include "../classes/obj_a.h"

#include <MockVendor/MockVendor.h>

#include <gmock/gmock.h>


class ObjAMock
{
public: // Methods
    MOCK_METHOD(void, open, (const std::string& filename));
    MOCK_METHOD(void, close, ());
};

using ObjAMockVendor = MockVendor<ObjAMock, ObjA>;

// This is a mock implementation of ObjA.

ObjA::ObjA()
{
    // Ignore any constructor parameters...
    ObjAMockVendor::vend(this);
}

ObjA::~ObjA()
{
    ObjAMockVendor::destroy(this);
}

void ObjA::open(const std::string& filename)
{
    // For mocked methods, pass parameters straight on to the mock.
    return ObjAMockVendor::mock(this)->open(filename);
}

void ObjA::close()
{
    // Not required, but for consistency in pattern, return even void methods.
    return ObjAMockVendor::mock(this)->close();
}

