
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_VECTOR_HPP
#define INCLUDE_SF_VECTOR_HPP

#include <vector>

#include <boost/cstdint.hpp>
#include <boost/mpl/assert.hpp>

#include <SF/Serializer.hpp>
#include <SF/SerializeStl.hpp>
#include <SF/Stream.hpp>
#include <SF/Tools.hpp>

namespace SF {

    // std::vector

    template<typename T, typename A>
    inline void serializeVector(
        SF::Archive &ar,
        std::vector<T,A> &t,
        boost::mpl::false_ *)
    {
        serializeStlContainer<PushBackSemantics>(ar, t);
    }

    template<typename T, typename A>
    inline void serializeVector(
        SF::Archive &ar,
        std::vector<T,A> &t,
        boost::mpl::true_ *)
    {
        serializeVectorFast(ar, t);
    }

    template<typename T, typename A>
    inline void serialize_vc6(
        SF::Archive &ar,
        std::vector<T,A> &t, 
        const unsigned int)
    {
        typedef typename RCF::IsFundamental<T>::type type;
        serializeVector(ar, t, (type *) 0);
    }

    template<typename T, typename A>
    inline void serializeVectorFast(
        SF::Archive &ar,
        std::vector<T,A> &t)
    {
        if (ar.isRead())
        {
            boost::uint32_t count = 0;
            ar & count;

            if (count)
            {
                SF::IStream &is = *ar.getIstream();

                t.resize(0);

                std::size_t minSerializedLength = sizeof(T);
                if (ar.verifyAgainstArchiveSize(count*minSerializedLength))
                {
                    t.reserve(count);
                }

                boost::uint32_t elementsRemaining = count;
                const boost::uint32_t BufferSize = 512;
                while (elementsRemaining)
                {
                    boost::uint32_t elementsRead = count - elementsRemaining;
                    boost::uint32_t elementsToRead = RCF_MIN(BufferSize, elementsRemaining);
                    boost::uint32_t bytesToRead = elementsToRead*sizeof(T);
                    t.resize( t.size() + elementsToRead);

                    RCF_VERIFY(
                        is.read( (char*) &t[elementsRead] , bytesToRead) == bytesToRead,
                        RCF::Exception(RCF::_SfError_ReadFailure()))
                        (elementsToRead)(BufferSize)(count);

                    elementsRemaining -= elementsToRead;
                }
            }
        }
        else if (ar.isWrite())
        {
            boost::uint32_t count = static_cast<boost::uint32_t>(t.size());
            ar & count;
            if (count)
            {
                boost::uint32_t bytesToWrite = count * sizeof(T);
                ar.getOstream()->writeRaw(
                    (SF::Byte8 *) &t[0],
                    bytesToWrite);
            }
        }
    }

} // namespace SF

#endif // ! INCLUDE_SF_VECTOR_HPP
