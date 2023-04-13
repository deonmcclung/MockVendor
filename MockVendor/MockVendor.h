/**
 * @file MockVendor.h
 * @brief A framework to dispense mocks to a test environment
 *
 * @author Deon McClung
 *
 * @copyright 2023 Deon McClung
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
template <typename Mock, typename Real>
class MockVendor
{
public: // Definitions
    using MockType = Mock;
    using RealType = Real;

    template <typename BaseMockType, typename BaseRealType>
    class BaseLink;

    template <typename LinkMockType, typename LinkRealType>
    template <typename BaseMockType, typename BaseRealType>
    friend class MockVendor<LinkMockType, LinkRealType>::BaseLink;

public: // Methods
    MockVendor()
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        sInstance = this;
    }

    virtual ~MockVendor()
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);

        // Checks
        if (!mMockList.empty())
        {
            ADD_FAILURE() << "Failure to consume all queued mocks for " << typeid(MockType).name();
        }

        if (!sMockMap.empty())
        {
            std::ostringstream str;
            str << "Not all mock instances were destroyed - " << sMockMap.size() << " remaining";
            if (MAX_LEAKED_REFS > 0)
            {
                size_t cnt = 0;
                for (auto& ref : sMockMap)
                {
                    str << std::endl
                        << "   Real: " << std::left << std::setw(sizeof(void*)*2 + 2) << std::hex << ref.first
                        << "   Mock: " << std::left << std::setw(sizeof(void*)*2 + 2) << std::hex << ref.second.get();
                    ++cnt;
                    if (cnt >= MAX_LEAKED_REFS)
                    {
                        if (sMockMap.size() > MAX_LEAKED_REFS)
                        {
                            str << std::endl << "    More...";
                        }
                        break;
                    }
                }
            }

            ADD_FAILURE() << str.str();

            // It is important to print the message above declaring any mock leaks because the below
            // clear will wipe out that information.
            // However, it is important to clear so that subsequent tests are not affected and may
            // themselves report any leaks.
            sMockMap.clear();
        }

        sInstance = nullptr;
    }

    /**
     * @brief Enqueue a mock for vending in FIFO order.
     * @param[in] mock      - A shared pointer to the mock which to enqueue
     */
    void queueMock(const std::shared_ptr<MockType>& mock)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        mMockList.push_back(mock);
    }

    /**
     * @brief Vend a mock associated with the passed real object.
     * @param[in] ths       - A pointer to the real object (the 'this')
     * @details This should be called from the real object's constructor with the 'this' pointer.
     *          If a mock is queued for vending, then it will be delivered. Otherwise, this method
     *          will vend a new mock with no expectations and default return values.
     */
    static std::shared_ptr<MockType> vend(const RealType* ths)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);

        if (sInstance != nullptr && !sInstance->mMockList.empty())
        {
            // If we have a mock to vend...
            sMockMap[ths] = sInstance->mMockList.front();
            sInstance->mMockList.pop_front();
            sLastMockWasPopped = true;
        }
        else
        {
            // Otherwise, create a mock to vend.
            sMockMap[ths] = std::make_shared<testing::NiceMock<MockType>>();
        }

        if (sBaseLinks != nullptr)
        {
            // If we have base classes...
            // We need to un-vend the base classes, but only if they were queued mocks (popped).
            for (auto linkPtr = sBaseLinks; linkPtr != nullptr; linkPtr = linkPtr->getNext())
            {
                linkPtr->linkRestoreMockIfPopped(ths, sMockMap[ths]);
            }
        }

        return sMockMap[ths];
    }

    /**
     * @brief Destroy the mock associated with the given real object.
     * @param[in] ths       - A pointer to the real object (the 'this')
     * @details This should be called from the real object's constructor with the 'this' pointer.
     */
    static void destroy(const RealType* ths)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);

        if (sMockMap.find(ths) != sMockMap.end())
        {
            // If it is found in the map, delete it.
            sMockMap.erase(ths);
        }
    }

    /**
     * @brief Move the mock reference from one class instance to another
     * @param[in] to        - A pointer to the instance receiving the mock
     * @param[in] from      - A pointer to the instance losing the mock
     */
    static void move(const RealType* to, const RealType* from)
    {
        if (from != to)
        {
            sMockMap[to] = sMockMap[from];
            sMockMap.erase(from);
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
        return sMockMap[ths];
    }

    /**
     * @brief Set the static mock (to be used in static methods)
     */
    void setStaticMock(const std::shared_ptr<MockType>& staticMock)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        mStaticMock = staticMock;
    }

    /**
     * @brief Return a mock that is intended to be used by static real methods
     * @return A mock for use in static methods as a temporary (not held)
     */
    static std::shared_ptr<MockType> staticMock()
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        if (sInstance != nullptr && sInstance->mStaticMock != nullptr)
        {
            return sInstance->mStaticMock;
        }
        else
        {
            // Return a temporary that will give default values from the mock.
            return std::make_shared<testing::NiceMock<MockType>>();
        }
    }

private: // Definitions
    class BaseLinkBase;

    using MockList = std::list<std::shared_ptr<MockType>>;
    using MockMap = std::map<const RealType*, std::shared_ptr<MockType>>;

    static constexpr size_t MAX_LEAKED_REFS = 15;

private: // Methods
    static void _addBaseLink(BaseLinkBase* newLink, BaseLinkBase*& next)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        next = sBaseLinks;
        sBaseLinks = newLink;
    }

    static bool _wasLastMockPopped()
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);
        return sLastMockWasPopped;
    }

    static void _restoreMock(const RealType* ths, std::shared_ptr<MockType>& poppedMock, const std::shared_ptr<MockType>& inheritedMock)
    {
        std::scoped_lock<std::recursive_mutex> lock(gMockVendorMutex);

        if (sInstance != nullptr)
        {
            sInstance->mMockList.push_front(poppedMock);
        }

        sMockMap[ths] = inheritedMock;

        sLastMockWasPopped = false;
    }

private: // Static Members
    inline static MockVendor*       sInstance{ nullptr };
    inline static MockMap           sMockMap;
    inline static bool              sLastMockWasPopped{ false };
    inline static BaseLinkBase*     sBaseLinks{ nullptr };

private: // Members
    MockList                        mMockList;
    std::shared_ptr<MockType>       mStaticMock;

}; // class MockVendor


/**
 * @brief This is a common base class for base links
 * @details Although this is a public class, the end user does not use these directly,
 * but instead declares BaseLink classes, which inherit from this class.
 */
template <typename MockType, typename RealType>
class MockVendor<MockType, RealType>::BaseLinkBase
{
public: // Methods
    virtual void linkRestoreMockIfPopped(const RealType* ths, const std::shared_ptr<MockType>& inheritedMock) = 0;

    BaseLinkBase* getNext() const { return mNext; }

protected: // Methods
    // Construction only by inheriting classes
    BaseLinkBase()
    {
        // Automatically add the link to the MockVendor
        MockVendor<MockType, RealType>::_addBaseLink(this, mNext);
    }
    virtual ~BaseLinkBase() = default;

private:
    BaseLinkBase* mNext{ nullptr };
};

/**
 * @brief Declare a base class connection between Real and Mock types.
 * @details The user declares a global instance of this class to identify or link
 * together inherited and derived-class relationships. This is only done when the
 * base class has an attending mock type.
 * @tparam BaseMockType     The type of the mock for the base class
 * @tparam BaseRealType     The actual code type of the base class
 */
template <typename MockType, typename RealType>
template <typename BaseMockType, typename BaseRealType>
class MockVendor<MockType, RealType>::BaseLink : public BaseLinkBase
{
public: // Methods
    BaseLink() = default;
    virtual ~BaseLink() = default;

    virtual void linkRestoreMockIfPopped(const RealType* ths, const std::shared_ptr<MockType>& inheritedMock) override
    {
        if (MockVendor<BaseMockType, BaseRealType>::_wasLastMockPopped())
        {
            auto poppedMock = MockVendor<BaseMockType, BaseRealType>::mock(ths);
            MockVendor<BaseMockType, BaseRealType>::_restoreMock(ths, poppedMock, inheritedMock);
        }
    }
};

#endif // __MOCK_VENDOR_H__
