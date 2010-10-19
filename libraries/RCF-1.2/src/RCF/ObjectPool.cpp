
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/ObjectPool.hpp>

#include <RCF/Exception.hpp>
#include <RCF/InitDeinit.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    void ObjectPool::get(VecPtr & vecPtr)
    {
        Lock lock(mVecPtrPoolMutex);

        if (mVecPtrPool.empty())
        {
            mVecPtrPool.push_back( VecPtr( new std::vector<char>()));
        }

        vecPtr = mVecPtrPool.back();
        mVecPtrPool.pop_back();
    }

    void ObjectPool::put(VecPtr & vecPtr)
    {
        RCF_ASSERT(vecPtr);
        RCF_ASSERT(vecPtr.unique());

        vecPtr->resize(0);

        Lock lock(mVecPtrPoolMutex);
        mVecPtrPool.push_back(vecPtr);
        vecPtr.reset();
    }

    void ObjectPool::get(OstrStreamPtr & ostrStreamPtr)
    {
        Lock lock(mOstrStreamPtrPoolMutex);

        if (mOstrStreamPtrPool.empty())
        {
            mOstrStreamPtrPool.push_back( OstrStreamPtr( new std::ostrstream()));
        }

        ostrStreamPtr = mOstrStreamPtrPool.back();
        mOstrStreamPtrPool.pop_back();
    }

    void ObjectPool::put(OstrStreamPtr & ostrStreamPtr)
    {
        RCF_ASSERT(ostrStreamPtr);
        RCF_ASSERT(ostrStreamPtr.unique());

        ostrStreamPtr->clear(); // freezing may have set error state
        ostrStreamPtr->rdbuf()->freeze(false);
        ostrStreamPtr->rdbuf()->pubseekoff(0, std::ios::beg, std::ios::out);

        Lock lock(mOstrStreamPtrPoolMutex);
        mOstrStreamPtrPool.push_back(ostrStreamPtr);
        ostrStreamPtr.reset();
    }

    typedef ObjectPool::VecPtr VecPtr;

    std::size_t getVectorCapacity(VecPtr vecPtr)
    {
        return vecPtr->capacity();
    }

    void ObjectPool::enumerateBuffers(std::vector<std::size_t> & bufferSizes)
    {
        bufferSizes.resize(0);

        Lock lock(mVecPtrPoolMutex);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4267)
#endif

        for (std::size_t i=0; i<mVecPtrPool.size(); ++i)
        {
            bufferSizes.push_back( mVecPtrPool[i]->capacity() );
        }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    }

    typedef ObjectPool::OstrStreamPtr OstrStreamPtr;
    
    std::size_t getOstrstreamSize(OstrStreamPtr ostrStreamPtr)
    {
        // What's the current position.
        std::size_t currentPos = ostrStreamPtr->pcount();

        // What's the end position.
        ostrStreamPtr->rdbuf()->pubseekoff(0, std::ios::end, std::ios::out);
        std::size_t endPos = ostrStreamPtr->pcount();

        // Set it back to the current position.
        ostrStreamPtr->rdbuf()->pubseekoff(
            static_cast<std::ostrstream::off_type>(currentPos), 
            std::ios::beg, 
            std::ios::out);

        // Return the end position.
        return endPos;
    }

    void ObjectPool::enumerateOstrstreams(std::vector<std::size_t> & bufferSizes)
    {
        bufferSizes.resize(0);

        Lock lock(mOstrStreamPtrPoolMutex);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4267)
#endif

        for (std::size_t i=0; i<mOstrStreamPtrPool.size(); ++i)
        {
            bufferSizes.push_back( getOstrstreamSize(mOstrStreamPtrPool[i]) );
        }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

    }

    ObjectPool * gpObjectPool;

    ObjectPool & getObjectPool()
    {
        return *gpObjectPool;
    }

    RCF_ON_INIT_DEINIT( 
        gpObjectPool = new ObjectPool(); , 
        delete gpObjectPool; gpObjectPool = NULL; )

} // namespace RCF
