
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_ARCHIVE_HPP
#define INCLUDE_SF_ARCHIVE_HPP

#include <boost/noncopyable.hpp>

#include <RCF/Export.hpp>
#include <RCF/util/DefaultInit.hpp>

#include <SF/DataPtr.hpp>

namespace SF {

    class IStream;
    class OStream;

    class RCF_EXPORT Archive : boost::noncopyable
    {
    public:

        enum Direction
        {
            READ,
            WRITE
        };

        enum Flag
        {
            PARENT              = 1 << 0,
            POINTER             = 1 << 1,
            NODE_ALREADY_READ   = 1 << 2,
            NO_BEGIN_END        = 1 << 3,
            POLYMORPHIC         = 1 << 4
        };

        Archive(Direction dir, IStream *stream);
        Archive(Direction dir, OStream *stream);

        Archive &   operator&(Flag flag);
        bool        isRead() const;
        bool        isWrite() const;
        IStream *   getIstream() const;
        OStream *   getOstream() const;
        bool        isFlagSet(Flag flag) const;
        void        setFlag(Flag flag, bool bEnable = true);
        void        clearFlag(Flag flag);
        void        clearState();
        DataPtr &   getLabel();
        bool        verifyAgainstArchiveSize(std::size_t bytesToRead);

        int         getRuntimeVersion();
        int         getArchiveVersion();

    private:

        Direction       mDir;
        IStream *       mIstream;
        OStream *       mOstream;
        DataPtr         mLabel;
        unsigned int    mFlags;
    };

}

#include <SF/Serializer.hpp>

#endif // ! INCLUDE_SF_ARCHIVE_HPP
