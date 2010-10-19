
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ThreadLocalCache.hpp>

namespace RCF {

    ObjectCache::VectorByteBufferCache & ObjectCache::getCache(VectorByteBufferCache *)
    {
        return mVectorByteBufferCache;
    }

    ObjectCache::VectorIntCache & ObjectCache::getCache(VectorIntCache *)
    {
        return mVectorIntCache;
    }

    ObjectCache::VectorWsabufCache & ObjectCache::getCache(VectorWsabufCache *)
    {
        return mVectorWsabufCache;
    }

    ObjectCache::VectorFilterCache & ObjectCache::getCache(VectorFilterCache *)
    {
        return mVectorFilterCache;
    }

    ObjectCache::VectorRcfSessionCallbackCache & ObjectCache::getCache(VectorRcfSessionCallbackCache *)
    {
        return mVectorRcfSessionCallbackCache;
    }

    void ObjectCache::clear()
    {
        mVectorByteBufferCache.clear();
        mVectorIntCache.clear();
        mVectorWsabufCache.clear();
        mVectorFilterCache.clear();
        mVectorRcfSessionCallbackCache.clear();
    }

} // namespace RCF
