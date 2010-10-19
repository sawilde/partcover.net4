
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_OBJECTPOOL_HPP
#define INCLUDE_RCF_OBJECTPOOL_HPP

#include <vector>
#include <strstream>

#include <boost/shared_ptr.hpp>

#include <RCF/ThreadLibrary.hpp>

namespace RCF {

    class ObjectPool
    {
    public:

        typedef boost::shared_ptr<std::vector<char> > VecPtr;
        typedef boost::shared_ptr<std::ostrstream> OstrStreamPtr;

        void get(VecPtr & vecPtr);
        void put(VecPtr & vecPtr);

        void get(OstrStreamPtr & ostrStreamPtr);
        void put(OstrStreamPtr & ostrStreamPtr);

        void enumerateBuffers(std::vector<std::size_t> & bufferSizes);
        void enumerateOstrstreams(std::vector<std::size_t> & bufferSizes);

    private:

        Mutex                       mVecPtrPoolMutex;
        std::vector<VecPtr>         mVecPtrPool;

        Mutex                       mOstrStreamPtrPoolMutex;
        std::vector<OstrStreamPtr>  mOstrStreamPtrPool;
    };

    ObjectPool & getObjectPool();

} // namespace RCF

#endif // ! INCLUDE_RCF_OBJECTPOOL_HPP
