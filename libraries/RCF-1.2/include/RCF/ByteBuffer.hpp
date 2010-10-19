
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_BYTEBUFFER_HPP
#define INCLUDE_RCF_BYTEBUFFER_HPP

#include <strstream>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/Tools.hpp>
#include <RCF/TypeTraits.hpp>

#include <RCF/util/Platform/OS/BsdSockets.hpp> // WSABUF

namespace RCF {

    // ByteBuffer class for facilitating zero-copy transmission and reception

    class RCF_EXPORT ByteBuffer
    {
    public:

        ByteBuffer();

        explicit
        ByteBuffer(std::size_t pvlen);

        explicit
        ByteBuffer(
            const std::vector<char> & vc);

        explicit
        ByteBuffer(
            const std::string & s);

        explicit
        ByteBuffer(
            const boost::shared_ptr<std::vector<char> > &spvc,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            const boost::shared_ptr<std::ostrstream> &spos,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            const boost::shared_ptr<std::ostrstream> &spos,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            const boost::shared_ptr<std::vector<char> > &spvc,
            bool readOnly = false);

        ByteBuffer(
            char *pv,
            std::size_t pvlen,
            std::size_t leftMargin,
            const boost::shared_ptr<std::vector<char> > &spvc,
            bool readOnly = false);

        ByteBuffer(
            const ByteBuffer &byteBuffer,
            std::size_t offset = 0,
            std::size_t len = -1);

        char *              getPtr()            const;
        std::size_t         getLength()         const;
        std::size_t         getLeftMargin()     const;
        bool                getReadOnly()       const;
        bool                isEmpty()           const;
        std::string         string()            const;

        void                setLeftMargin(std::size_t len);
        void                expandIntoLeftMargin(std::size_t len);
        ByteBuffer          release();
        void                clear();

                            operator bool();
        bool                operator !();

        static const std::size_t npos;

    private:
        // sentries
        boost::shared_ptr< std::vector<char> >      mSpvc;
        boost::shared_ptr< std::ostrstream >        mSpos;

        char *                                      mPv;
        std::size_t                                 mPvlen;
        std::size_t                                 mLeftMargin;
        bool                                        mReadOnly;
    };

    RCF_EXPORT bool operator==(const ByteBuffer &lhs, const ByteBuffer &rhs);

    RCF_EXPORT std::size_t lengthByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers);

    template<typename Functor>
    inline void forEachByteBuffer(
        const Functor &functor,
        const std::vector<ByteBuffer> &byteBuffers,
        std::size_t offset,
        std::size_t length = -1)
    {
        std::size_t pos0        = 0;
        std::size_t pos1        = 0;
        std::size_t remaining   = length;

        for (std::size_t i=0; i<byteBuffers.size(); ++i)
        {
            pos1 = pos0 + byteBuffers[i].getLength() ;

            if (pos1 <= offset)
            {
                pos0 = pos1;
            }
            else if (pos0 <= offset && offset < pos1)
            {
                std::size_t len = RCF_MIN(pos1-offset, remaining);

                ByteBuffer byteBuffer(
                    byteBuffers[i],
                    offset-pos0,
                    len);

                functor(byteBuffer);
                pos0 = pos1;
                remaining -= len;
            }
            else if (remaining > 0)
            {
                std::size_t len = RCF_MIN(pos1-pos0, remaining);

                ByteBuffer byteBuffer(
                    byteBuffers[i],
                    0,
                    len);

                functor(byteBuffer);
                pos1 = pos0;
                remaining -= len;
            }
        }
    }
    
    RCF_EXPORT ByteBuffer sliceByteBuffer(
        const std::vector<ByteBuffer> &slicedBuffers,
        std::size_t offset,
        std::size_t length = -1);

    RCF_EXPORT void sliceByteBuffers(
        std::vector<ByteBuffer> &slicedBuffers,
        const std::vector<ByteBuffer> &byteBuffers,
        std::size_t offset,
        std::size_t length = -1);

    RCF_EXPORT void copyByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers,
        char *pch);

    RCF_EXPORT void copyByteBuffers(
        const std::vector<ByteBuffer> &byteBuffers,
        ByteBuffer &byteBuffer);

} // namespace RCF

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(RCF::ByteBuffer)

namespace SF {

    class Archive;

    RCF_EXPORT void serialize(SF::Archive &ar, RCF::ByteBuffer &byteBuffer);

}

#endif // ! INCLUDE_RCF_BYTEBUFFER_HPP
