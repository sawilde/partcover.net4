
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_SERIALIZESMARTPTR_HPP
#define INCLUDE_SF_SERIALIZESMARTPTR_HPP

#include <SF/Archive.hpp>
#include <SF/Stream.hpp>

namespace SF {

    // These macros could be written as templates, but borland's compiler won't handle it.

    // 1. Non-ref counted smart pointer. SmartPtr<T> must support reset() and operator->().

#define SF_SERIALIZE_SIMPLE_SMARTPTR( SmartPtr )                                        \
    template<typename T>                                                                \
    inline bool serializeSimpleSmartPtr(SmartPtr<T> **pps, SF::Archive &ar)             \
    {                                                                                   \
        if (ar.isRead())                                                                \
        {                                                                               \
            if (ar.isFlagSet(Archive::POINTER))                                         \
            {                                                                           \
                *pps = new SmartPtr<T>();                                               \
            }                                                                           \
            T *pt = NULL;                                                               \
            ar & pt;                                                                    \
            (**pps).reset(pt);                                                          \
        }                                                                               \
        else if (ar.isWrite())                                                          \
        {                                                                               \
            T *pt = NULL;                                                               \
            if (*pps && (**pps).get())                                                  \
            {                                                                           \
                pt = (**pps).operator->();                                              \
            }                                                                           \
            ar & pt;                                                                    \
        }                                                                               \
        return true;                                                                    \
    }                                                                                   \
                                                                                        \
    template<typename T>                                                                \
    inline bool invokeCustomSerializer(SmartPtr<T> **ppt, Archive &ar, int)             \
    {                                                                                   \
        return serializeSimpleSmartPtr(ppt, ar);                                        \
    }

    // 2. Ref counted smart pointer. Must support operator=(), operator->(), and get().

#define SF_SERIALIZE_REFCOUNTED_SMARTPTR( SmartPtr )                                    \
    template<typename T >                                                               \
    inline bool serializeRefCountedSmartPtr(SmartPtr<T> **pps, SF::Archive &ar)         \
    {                                                                                   \
        if (ar.isRead())                                                                \
        {                                                                               \
            if (ar.isFlagSet(Archive::POINTER)) *pps = new SmartPtr<T>;                 \
            T *pt = NULL;                                                               \
            ar & pt;                                                                    \
            typedef ObjectId IdT;                                                       \
                                                                                        \
            ContextRead &ctx = ar.getIstream()->getContext();                           \
                                                                                        \
            void *pv = NULL;                                                            \
            if (pt && ctx.query( pt, typeid(SmartPtr<T>), pv ))                         \
            {                                                                           \
                SmartPtr<T> *ps_prev = reinterpret_cast< SmartPtr<T> * >(pv);           \
                **pps = *ps_prev;                                                       \
            }                                                                           \
            else if (pt)                                                                \
            {                                                                           \
                ctx.add( pt, typeid(SmartPtr<T>), *pps );                               \
                **pps = SmartPtr<T>(pt);                                                \
            }                                                                           \
            else                                                                        \
            {                                                                           \
                **pps = SmartPtr<T>(pt);                                                \
            }                                                                           \
            return true;                                                                \
        }                                                                               \
        else /*if (ar.isWrite())*/                                                      \
        {                                                                               \
            T *pt = NULL;                                                               \
            if (*pps)                                                                   \
            {                                                                           \
                pt = (**pps).get();                                                     \
            }                                                                           \
            ar & pt;                                                                    \
            return true;                                                                \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    template<typename T>                                                                \
    inline bool invokeCustomSerializer(SmartPtr<T> **ppt, Archive &ar, int)             \
    {                                                                                   \
        return serializeRefCountedSmartPtr(ppt, ar);                                    \
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZERSMARTPTR_HPP
