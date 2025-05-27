/**
 * @file mockvendor_tests.cpp
 * @brief MockVendor tests
 *
 * @author Deon McClung
 *
 * @copyright 2025 Deon McClung
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

// Headers for collaborative objects in use
#include "classes/obj_a.h"
#include "classes/obj_b.h"
#include "classes/obj_c.h"

// Mocks for collaborating objects
#include "mocks/obj_a_mock.h"
#include "mocks/obj_b_mock.h"
#include "mocks/obj_c_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>


using testing::_;


class MockVendorTests : public testing::Test
{
};

TEST_F(MockVendorTests, SimpleClass)
{
    // Test setup
    auto obj_a_mock = std::make_shared<ObjAMock>();

    // Add expectations on the mock
    EXPECT_CALL(*obj_a_mock, open(_));
    EXPECT_CALL(*obj_a_mock, close());

    // Create a mock vendor for objects of ObjA.
    ObjAMockVendor obj_a_mv;

    // Queue the mock to be created when the first ObjA is created.
    obj_a_mv.queueMock(obj_a_mock);

    // Perform the test
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // The follow code could exist at any depth in the code under test.

    // Create an object of ObjA.
    auto obj_a = std::make_shared<ObjA>();
    obj_a->open("some_filename");
    obj_a->close();

    // Deleting the object completes its lifecycle. Objects under test
    // must be cleaned up for proper accounting.
    obj_a.reset();

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // End code under test
    
    // The test could perform additional validation, or not.
    
    // As the MockVendor(s) go out of scope, they check that all expected
    // mocked objects were created and destroyed.
}


TEST_F(MockVendorTests, SingleInheritance)
{
    // Test setup
    
    // Objects and mocks of ObjB contain all methods of ObjA.
    auto obj_b_mock = std::make_shared<ObjBMock>();

    // Add expectations on the mock
    EXPECT_CALL(*obj_b_mock, open(_));
    EXPECT_CALL(*obj_b_mock, close());
    EXPECT_CALL(*obj_b_mock, obj_b_func()).WillRepeatedly(testing::Return(5));
    EXPECT_CALL(*obj_b_mock, obj_b_v_func()).WillRepeatedly(testing::Return(6));

    // Create a mock vendor for objects of ObjB.
    ObjBMockVendor obj_b_mv;

    // Queue the mock to be created when the first ObjB is created.
    obj_b_mv.queueMock(obj_b_mock);

    // Perform the test
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // The follow code could exist at any depth in the code under test.

    // Create an object of ObjA.
    auto obj_b = std::make_shared<ObjB>();
    obj_b->open("some_filename");
    obj_b->close();
    auto value_to_return1 = obj_b->obj_b_func();
    auto value_to_return2 = obj_b->obj_b_v_func();

    // Deleting the object completes its lifecycle. Objects under test
    // must be cleaned up for proper accounting.
    obj_b.reset();

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // End code under test
    
    // The test could perform additional validation, or not.
    EXPECT_EQ(5, value_to_return1);
    EXPECT_EQ(6, value_to_return2);

    // As the MockVendor(s) go out of scope, they check that all expected
    // mocked objects were created and destroyed.

}

TEST_F(MockVendorTests, MultigenerationalObject)
{
    // Test setup
    
    // Objects and mocks of ObjC contain all methods of ObjB and ObjA.
    auto obj_c_mock = std::make_shared<ObjCMock>();

    // Add expectations on the mock
    EXPECT_CALL(*obj_c_mock, open(_));
    EXPECT_CALL(*obj_c_mock, close());
    EXPECT_CALL(*obj_c_mock, obj_b_func()).WillRepeatedly(testing::Return(5));
    EXPECT_CALL(*obj_c_mock, obj_c_func());
    EXPECT_CALL(*obj_c_mock, obj_b_v_func()).WillRepeatedly(testing::Return(7));

    // Create a mock vendor for objects of ObjC.
    ObjCMockVendor obj_c_mv;

    // Queue the mock to be created when the first ObjC is created.
    obj_c_mv.queueMock(obj_c_mock);

    // Perform the test
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // The follow code could exist at any depth in the code under test.

    // Create an object of ObjA.
    auto obj_c = std::make_shared<ObjC>();
    obj_c->open("some_filename");
    obj_c->close();
    auto value_to_return1 = obj_c->obj_b_func();
    auto value_to_return2 = obj_c->obj_b_v_func();
    obj_c->obj_c_func();

    // Deleting the object completes its lifecycle. Objects under test
    // must be cleaned up for proper accounting.
    obj_c.reset();

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // End code under test
    
    // The test could perform additional validation, or not.
    EXPECT_EQ(5, value_to_return1);
    EXPECT_EQ(7, value_to_return2);

    // As the MockVendor(s) go out of scope, they check that all expected
    // mocked objects were created and destroyed.
}


TEST_F(MockVendorTests, MixedObjects)
{
    // Test setup
    
    // Mixed Objects
    auto obj_c_mock = std::make_shared<ObjCMock>();
    auto obj_b_mock1 = std::make_shared<ObjBMock>();
    auto obj_b_mock2 = std::make_shared<ObjBMock>();

    // Add expectations on the mocks
    EXPECT_CALL(*obj_c_mock, open("C"));

    EXPECT_CALL(*obj_b_mock1, open("B1"));
    EXPECT_CALL(*obj_b_mock2, open("B2"));

    // Create mock vendors for the objects.
    ObjCMockVendor obj_c_mv;
    ObjBMockVendor obj_b_mv;

    // Queue the mocks to be created when their respective objects are created.
    // Objects of the same type must be queued according to their expected creation order.
    obj_c_mv.queueMock(obj_c_mock);
    obj_b_mv.queueMock(obj_b_mock1);
    obj_b_mv.queueMock(obj_b_mock2);


    // Perform the test
    // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    // The follow code could exist at any depth in the code under test.

    // Object creation order only matters for objects of the same type.
    auto obj_b1 = std::make_shared<ObjB>();
    auto obj_c = std::make_shared<ObjC>();

    {
        // Objects can be created on the stack. The creation order still counts.
        ObjB obj_b2;
        obj_b2.open("B2");
    }

    obj_b1->open("B1");

    obj_c->open("C");

    // Deleting the object completes its lifecycle. Objects under test
    // must be cleaned up for proper accounting.
    obj_c.reset();
    obj_b1.reset();

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // End code under test
    
    // As the MockVendor(s) go out of scope, they check that all expected
    // mocked objects were created and destroyed.
}
