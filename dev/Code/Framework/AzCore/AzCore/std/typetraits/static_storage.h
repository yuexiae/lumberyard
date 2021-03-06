/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#pragma once

#include <AzCore/std/typetraits/aligned_storage.h>
#include <AzCore/std/typetraits/alignment_of.h>
#include <AzCore/std/utils.h>
#include <AzCore/std/parallel/atomic.h>

namespace AZStd
{
    // Extension: static_storage: Used to initialize statics in a thread-safe manner
    // VS2013 and some other platforms do not handle non-pod function statics correctly in a thread-safe manner
    // VS2015+/clang init them as part of static init, or interlocked (as per standard)

// VS2013 doesn't believe that ptr->~T() references ptr. We wholeheartedly disagree.
#if defined(AZ_COMPILER_MSVC) && _MSC_VER <= 1800
#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#endif

    // These are similar to default_delete/no_delete, except they just invoke the destructor (or not)
    template <class T>
    struct default_destruct
    {
        template <class U, class = typename AZStd::enable_if<AZStd::is_constructible<U*, T*>::value, void>::type>
        void operator()(U* ptr)
        {
            ptr->~T();
        }
    };

    template <class T>
    struct no_destruct
    {
        template <class U, class = typename AZStd::enable_if<AZStd::is_constructible<U*, T*>::value, void>::type>
        void operator()(U*)
        {
        }
    };

#if defined(AZ_COMPILER_MSVC) && _MSC_VER <= 1800
#pragma warning(pop)
#endif

    template <typename T, class Destructor = default_destruct<T>>
    class static_storage
    {
    public:
        template <class ...Args>
        static_storage(Args&&... args)
        {
            // m_object is not initialized, it can only have 1 correct value, which
            // we can detect below in operator T&()
            m_object.store(new (&m_storage) T(AZStd::forward<Args>(args)...));
        }

        ~static_storage()
        {
            T* expected = reinterpret_cast<T*>(&m_storage);
            if (m_object.compare_exchange_strong(expected, nullptr))
            {
                Destructor()(reinterpret_cast<T*>(&m_storage));
            }
        }

        operator const T&() const
        {
            return operator T&();
        }

        operator T&()
        {
            // spin wait for m_object to have the correct value, someone must be constructing it
            T* obj = nullptr;
            do {
                obj = m_object.load();
            } while (obj != reinterpret_cast<T*>(&m_storage)); 
            return *obj;
        }

    private:
        typename aligned_storage<sizeof(T), alignment_of<T>::value>::type m_storage;
        AZStd::atomic<T*> m_object;
    };
}
