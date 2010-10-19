
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_THREADLOCALCACHE_HPP
#define INCLUDE_RCF_THREADLOCALCACHE_HPP

#include <utility>
#include <vector>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/ThreadLocalData.hpp>
#include <RCF/util/Platform/OS/BsdSockets.hpp>

namespace RCF {

    // Thread local caching

    template<typename T>
    struct CacheType
    {
        typedef
            std::vector<
                std::pair<
                    boost::shared_ptr< bool>,
                    boost::shared_ptr< T > > > Val;
    };

#ifndef BOOST_WINDOWS
    typedef iovec WSABUF;
#endif

    class Filter;
    typedef boost::shared_ptr<Filter> FilterPtr;

    class ByteBuffer;
    class RcfSession;

    typedef boost::function1<void, RcfSession&> RcfSessionCallback;

    class RCF_EXPORT ObjectCache
    {
    public:

        typedef CacheType< std::vector<RCF::ByteBuffer> >::Val          VectorByteBufferCache;
        typedef CacheType< std::vector<int> >::Val                      VectorIntCache;
        typedef CacheType< std::vector<WSABUF> >::Val                   VectorWsabufCache;
        typedef CacheType< std::vector<FilterPtr> >::Val                VectorFilterCache;
        typedef CacheType< std::vector<RcfSessionCallback> >::Val       VectorRcfSessionCallbackCache;

        VectorByteBufferCache &                 getCache(VectorByteBufferCache *);
        VectorIntCache &                        getCache(VectorIntCache *);
        VectorWsabufCache &                     getCache(VectorWsabufCache *);
        VectorFilterCache &                     getCache(VectorFilterCache *);
        VectorRcfSessionCallbackCache &         getCache(VectorRcfSessionCallbackCache *);
        void clear();

    private:
        VectorByteBufferCache                   mVectorByteBufferCache;
        VectorIntCache                          mVectorIntCache;
        VectorWsabufCache                       mVectorWsabufCache;
        VectorFilterCache                       mVectorFilterCache;
        VectorRcfSessionCallbackCache           mVectorRcfSessionCallbackCache;
    };

    template<typename T>
    class ThreadLocalCached
    {
    public:

        typedef typename CacheType<T>::Val TCache;

        ThreadLocalCached()
        {
            ObjectCache &objectCache = getThreadLocalObjectCache();

            TCache &tCache = objectCache.getCache( (TCache *) NULL);
            for (std::size_t i=0; i<tCache.size(); ++i)
            {
                if (*tCache[i].first == false)
                {
                    mMarkPtr = tCache[i].first;
                    mtPtr = tCache[i].second;
                    break;
                }
            }
            if (!mtPtr)
            {
                typedef typename TCache::value_type ValueType;
                tCache.push_back( ValueType());
                tCache.back().first.reset( new bool());
                tCache.back().second.reset( new T());
                mMarkPtr = tCache.back().first;
                mtPtr = tCache.back().second;
            }
            *mMarkPtr = true;
        }

        ~ThreadLocalCached()
        {
            *mMarkPtr = false;
            mtPtr->resize(0);
        }

        T &get()
        {
            return *mtPtr;
        }

    private:

        boost::shared_ptr< bool> mMarkPtr;
        boost::shared_ptr< T > mtPtr;

    };

} // namespace RCF

#endif // ! INCLUDE_RCF_THREADLOCALCACHE_HPP
