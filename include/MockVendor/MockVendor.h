/**
 * @file MockVendor.h
 * @brief A framework to dispense mocks to a test environment
 *
 * @author Deon McClung
 *
 * @copyright 2023-2025 Deon McClung
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#ifndef __MOCK_VENDOR_H__
#define __MOCK_VENDOR_H__

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <map>
#include <exception>
#include <string>
#include <mutex>
#include <list>
#include <iomanip>

namespace
{
    // This mutex is locked for the entirety of every function. This library is focused
    // on correctness over multi-threading performance (which should not be common in a
    // testing environment anyway). The technical reason for the coarseness of the lock is
    // because there is interaction between objects of different types and it is difficult
    // to make a finer lock. This makes no pretence of concurrency; it effectively
    // eliminates concurrency in this library.
    std::recursive_mutex gMockVendorMutex;
}

class MockVendorException : public std::exception
{
public: // Methods
    MockVendorException(const std::string message)
        : mMessage(message)
    {
    }

    virtual ~MockVendorException() noexcept = default;

    virtual const char* what() const noexcept
    {
        return mMessage.c_str();
    }

private: // Members
    std::string mMessage;
};

template <typename Mock, typename Real>
struct MockVendorData
{
    using MockType = Mock;
    using RealType = Real;

    using MockList = std::list<std::shared_ptr<MockType>>;
    using MockMap = std::map<const RealType*, std::shared_ptr<MockType>>;

    inline static uint32_t sInstanceCount{0};
    inline static MockMap sMockMap;
    inline static MockList sMockList;
    inline static std::shared_ptr<MockType> sStaticMock;

}; // namespace mockvendor


/**
 * @brief A class that manages the distribution of mocks during a test for a given mock type.
 * @tparam Mock         The type of the mock for the class
 * @tparam Real         The actual code type of the class
 * @details The mock uses this class to vend mock instances for use during test operation. If
 * this class is not explicitely declared by the test, it exists in static space due to the presence
 * of the mock that uses it. If the test does not require custom behavor of a mock class, the test
 * does not need to declare an instance of this class. However, if the test requires non-default behavior
 * of a participating mock, then the test declares an instance of this class inside the scope of the test
 * and pushes on mock instances in the order of expected real-class instantiation inside the code under
 * test.
 */
template <typename Mock, typename Real, typename... BaseMockVendors>
class MockVendor
{
public: // Definitions
    using MockType = Mock;
    using RealType = Real;

public: // Methods
    MockVendor()
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        _construct<MockType, RealType, BaseMockVendors...>();
    }

    virtual ~MockVendor()
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        _destruct<MockType, RealType, BaseMockVendors...>();
    }

    /**
     * @brief Enqueue a mock for vending in FIFO order.
     * @param[in] mock      - A shared pointer to the mock which to enqueue
     */
    void queueMock(const std::shared_ptr<MockType>& mock)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        _queueMock<MockType, RealType, BaseMockVendors...>(mock);
    }

    /**
     * @brief Vend a mock associated with the passed real object.
     * @param[in] ths       - A pointer to the real object (the 'this')
     * @details This should be called from the real object's constructor with the 'this' pointer.
     *          If a mock is queued for vending, then it will be delivered. Otherwise, this method
     *          will vend a new mock with no expectations and default return values.
     */
    static void vend(const RealType* ths)
    {
        using MVD = MockVendorData<MockType, RealType>;

        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        if (!MVD::sMockList.empty())
        {
            // If we have a mock to vend...
            MVD::sMockMap[ths] = MVD::sMockList.front();
            MVD::sMockList.pop_front();
        }
        else
        {
            // Otherwise, create a mock to vend.
            MVD::sMockMap[ths] = std::make_shared<testing::NiceMock<MockType>>();
        }
    }

    /**
     * @brief Destroy the mock associated with the given real object.
     * @param[in] ths       - A pointer to the real object (the 'this')
     * @details This should be called from the real object's constructor with the 'this' pointer.
     */
    static void destroy(const RealType* ths)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        using MVD = MockVendorData<MockType, RealType>;

        if (MVD::sMockMap.find(ths) != MVD::sMockMap.end())
        {
            MVD::sMockMap.erase(ths);
        }
    }

    /**
     * @brief A method to access the mock from the real layer methods.
     * @param[in] ths       - A pointer to the 'this' object from the real layer methods
     * @return A pointer to the mock for the given 'this'.
     */
    static std::shared_ptr<MockType> mock(const RealType* ths)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        using MVD = MockVendorData<MockType, RealType>;
        return MVD::sMockMap[ths];
    }

    /**
     * @brief Set the static mock (to be used in static methods)
     */
    void setStaticMock(const std::shared_ptr<MockType>& staticMock)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        _setStaticMock<MockType, RealType>(staticMock);
    }

    /**
     * @brief Return a mock that is intended to be used by static real methods
     * @return A mock for use in static methods as a temporary (not held)
     */
    static std::shared_ptr<MockType> staticMock()
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        using MVD = MockVendorData<MockType, RealType>;
        if (MVD::sStaticMock != nullptr)
        {
            return MVD::sStaticMock;
        }
        else
        {
            // Return a temporary that will give default values from the mock.
            return std::make_shared<testing::NiceMock<MockType>>();
        }
    }

private: // Definitions
    static constexpr size_t MAX_LEAKED_REFS = 15;


private: // Methods
    template <typename MVD>
    static void _construct()
    {
        ++MVD::sInstanceCount;
    }

    template <typename M, typename R>
    static void _construct()
    {
        // Terminate the recursion
        _construct<MockVendorData<M, R>>();
    }

    template <typename M, typename R, typename Base, typename... OtherBases>
    static void _construct()
    {
        _construct<MockVendorData<M, R>>();

        // Recurse for remaining bases...
        _construct<typename Base::MockType, typename Base::RealType, OtherBases...>();
    }

    template <typename MVD>
    static void _destruct()
    {
        if (--MVD::sInstanceCount == 0)
        {
            // Checks
            if (!MVD::sMockList.empty())
            {
                ADD_FAILURE() << "Failure to consume all queued mocks for " << typeid(typename MVD::MockType).name();
            }

            if (!MVD::sMockMap.empty())
            {
                std::ostringstream str;
                str << "Not all mock instances were destroyed - " << MVD::sMockMap.size() << " remaining";
                if (MAX_LEAKED_REFS > 0)
                {
                    size_t cnt = 0;
                    for (auto& ref : MVD::sMockMap)
                    {
                        str << std::endl
                            << "   Real: " << std::left << std::setw(sizeof(void*)*2 + 2) << std::hex << ref.first
                            << "   Mock: " << std::left << std::setw(sizeof(void*)*2 + 2) << std::hex << ref.second.get();
                        ++cnt;
                        if (cnt >= MAX_LEAKED_REFS)
                        {
                            if (MVD::sMockMap.size() > MAX_LEAKED_REFS)
                            {
                                str << std::endl << "    More...";
                            }
                            break;
                        }
                    }
                }

                ADD_FAILURE() << str.str();

                // Now that the above information is printed. We need to wipe out these variables so
                // they don't interfere with other tests.
                MVD::sMockMap.clear();
                MVD::sMockList.clear();
                MVD::sStaticMock.reset();
            }

        } // if (--sInstanceCount == 0)
    }

    template <typename M, typename R>
    static void _destruct()
    {
        // Terminate the recursion
        _destruct<MockVendorData<M, R>>();
    }

    template <typename M, typename R, typename Base, typename... OtherBases>
    static void _destruct()
    {
        _destruct<MockVendorData<M, R>>();

        // Recurse for remaining bases...
        _destruct<typename Base::MockType, typename Base::RealType, OtherBases...>();
    }

    template <typename MVD>
    static void _queueMock(const std::shared_ptr<Mock>& mock)
    {
        MVD::sMockList.push_back(mock);
    }

    template <typename M, typename R>
    static void _queueMock(const std::shared_ptr<Mock>& mock)
    {
        // Terminate the recursion
        _queueMock<MockVendorData<M, R>>(mock);
    }

    template <typename M, typename R, typename Base, typename... OtherBases>
    static void _queueMock(const std::shared_ptr<Mock>& mock)
    {
        _queueMock<MockVendorData<M, R>>(mock);

        // Recurse the remaining bases...
        _queueMock<typename Base::MockType, typename Base::RealType, OtherBases...>(mock);
    }

    template <typename MV>
    static void _setStaticMock(const std::shared_ptr<Mock>& staticMock)
    {
        MV::sStaticMock = staticMock;
    }

    template <typename M, typename R>
    static void _setStaticMock(const std::shared_ptr<Mock>& staticMock)
    {
        // Terminate the recursion
        _setStaticMock<MockVendor<M, R>>(staticMock);
    }

    template <typename M, typename R, typename Base, typename... OtherBases>
    static void _setStaticMock(const std::shared_ptr<Mock>& staticMock)
    {
        _setStaticMock<MockVendor<M, R>>(staticMock);
        
        // Recurse the remaining bases...
        _setStaticMock<typename Base::MockType, typename Base::RealType, OtherBases...>(staticMock);
    }

}; // class MockVendor


#endif // __MOCK_VENDOR_H__
