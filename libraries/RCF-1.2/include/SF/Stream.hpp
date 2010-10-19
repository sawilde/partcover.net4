
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_STREAM_HPP
#define INCLUDE_SF_STREAM_HPP

#include <iosfwd>
#include <map>
#include <string>
#include <strstream>

#include <boost/noncopyable.hpp>

#include <RCF/Export.hpp>

#include <SF/DataPtr.hpp>
#include <SF/Encoding.hpp>
#include <SF/I_Stream.hpp>

namespace SF {

    //**************************************************
    // Encoding of object data

    template<typename E>
    class Encoding : public I_Encoding
    {
    public:
        virtual UInt32 getCount(
            DataPtr &               data, 
            const std::type_info &  type)
        {
            return countElements( (E *) 0, data, type);
        }

        virtual void toData(
            DataPtr &               data, 
            void *                  pvObject, 
            const std::type_info &  type, 
            int                     nCount)
        {
            encodeElements( (E *) 0, data, pvObject, type, nCount);
        }

        virtual void toObject(
            DataPtr &               data, 
            void *                  pvObject, 
            const std::type_info &  type, 
            int                     nCount)
        {
            decodeElements( (E *) 0, data, pvObject, type, nCount);
        }
    };

    //**************************************************
    // Context handling

    class RCF_EXPORT ContextRead
    {
    public:
        ContextRead();
        virtual ~ContextRead();
        virtual void add(SF::UInt32 nid, const ObjectId &id);
        virtual void add(void *ptr, const std::type_info &objType, void *pObj);
        virtual bool query(SF::UInt32 nid, ObjectId &id);
        virtual bool query(void *ptr, const std::type_info &objType, void *&pObj);
        virtual void clear();
    private:
        bool bEnabled_;
        std::auto_ptr<std::map<UInt32, ObjectId> > nid_id_ptr_;
        std::auto_ptr<std::map<std::string, std::map< void *, void * > > > type_ptr_obj_ptr_;
    };

    class RCF_EXPORT ContextWrite
    {
    public:
        ContextWrite();
        virtual ~ContextWrite();
        virtual void setEnabled(bool enabled);
        virtual bool getEnabled();
        virtual void add(const ObjectId &id, UInt32 &nid);
        virtual bool query(const ObjectId &id, UInt32 &nid);
        virtual void clear();
    private:
        bool bEnabled_;
        UInt32 currentId_;
        std::auto_ptr<std::map<ObjectId, UInt32> > id_nid_ptr_;
    };

    class RCF_EXPORT WithContextRead
    {
    public:
        virtual ~WithContextRead();
        virtual ContextRead &getContext();
    private:
        ContextRead context;
    };

    class RCF_EXPORT WithContextWrite
    {
    public:
        virtual ~WithContextWrite();
        virtual ContextWrite &getContext();
        void enableContext() { getContext().setEnabled(true); }
        void disableContext() { getContext().setEnabled(false); }
        void clearContext() { getContext().clear(); }
    private:
        ContextWrite context;
    };

    //**************************************************
    // Stream local storage

    class RCF_EXPORT LocalStorage : boost::noncopyable
    {
    public:
        LocalStorage();
        virtual ~LocalStorage();
        void setNode(Node *);
        Node *getNode();

    private:
        Node *pNode_;
    };

    class RCF_EXPORT WithLocalStorage
    {
    public:
        virtual ~WithLocalStorage();
        virtual LocalStorage &getLocalStorage();
    private:
        LocalStorage localStorage_;
    };

    //****************************************************

    // For dependency reasons these functions should be defined separately, but
    // vc6 can't deal with that, hence the ifdef hack.

#if defined(_MSC_VER) && _MSC_VER == 1200

    class WithSemanticsRead
    {
    public:
        virtual ~WithSemanticsRead() {}

        template<typename T>
        WithSemanticsRead &operator>>(T &t)
        {
            Archive ar(Archive::READ, static_cast<IStream *>(this) );
            ar & t;
            return *this;
        }

        template<typename T>
        WithSemanticsRead &operator>>(const T &t)
        {
            Archive ar(Archive::READ, static_cast<IStream *>(this) );
            ar & t;
            return *this;
        }

    };

    class WithSemanticsWrite
    {
    public:
        virtual ~WithSemanticsWrite() {}

        template<typename T>
        WithSemanticsWrite &operator<<(const T &t)
        {
            Archive ar(Archive::WRITE, static_cast<OStream *>(this) );
            ar & t;
            return *this;
        }
    };

#else

    class WithSemanticsRead
    {
    public:
        virtual ~WithSemanticsRead() {}

        template<typename T>
        WithSemanticsRead &operator>>(T &t);

        template<typename T>
        WithSemanticsRead &operator>>(const T &t);
    };

    class WithSemanticsWrite
    {
    public:
        virtual ~WithSemanticsWrite() {}

        template<typename T>
        WithSemanticsWrite &operator<<(const T &t);
    };

#endif

    //****************************************************
    // Base stream classes

    typedef Encoding<Text> EncodingText;
    typedef Encoding<BinaryNative> EncodingBinaryNative;
    typedef Encoding<BinaryPortable> EncodingBinaryPortable;

    class Node;
    class SerializerBase;

    class RCF_EXPORT IStream :
        public WithSemanticsRead,
        public WithContextRead,
        public WithLocalStorage,
        boost::noncopyable
    {
    public:
        IStream();

        IStream(
            std::istream &  is, 
            std::size_t     archiveSize = 0, 
            int             rcfRuntimeVersion = 0, 
            int             archiveVersion = 0);

        void        setIs(
                        std::istream &  is, 
                        std::size_t     archiveSize = 0, 
                        int             rcfRuntimeVersion = 0, 
                        int             archiveVersion = 0);

        void        clearState();

        bool        supportsReadRaw();
        UInt32      readRaw(Byte8 *&pBytes, UInt32 nLength);
        UInt32      read(Byte8 *pBytes, UInt32 nLength);

        bool        verifyAgainstArchiveSize(std::size_t bytesToRead);

        bool        begin(Node &node);
        bool        get(DataPtr &value);
        void        end();
        UInt32      read_int(UInt32 &n);
        UInt32      read_byte(Byte8 &byte);
        void        putback_byte(Byte8 byte);

        virtual I_Encoding &
                    getEncoding() = 0;

        int         getRuntimeVersion();
        int         getArchiveVersion();

        void        setArchiveVersion(int archiveVersion);
        void        setRuntimeVersion(int runtimeVersion);

        void        ignoreVersionStamp(bool ignore = true);

    private:

        std::istream *      mpIs;
        std::istrstream *   mpIstr;
        std::size_t         mArchiveSize;
        int                 mRcfRuntimeVersion;
        int                 mArchiveVersion;
        bool                mIgnoreVersionStamp;
    };

    class RCF_EXPORT OStream :
        public WithSemanticsWrite,
        public WithContextWrite,
        public WithLocalStorage,
        boost::noncopyable
    {
    public:
        OStream();

        OStream(
            std::ostream &  os, 
            int             rcfRuntimeVersion = 0, 
            int             archiveVersion = 0);

        void        setOs(
                        std::ostream &  os, 
                        int             rcfRuntimeVersion = 0, 
                        int             archiveVersion = 0);

        void        clearState();

        UInt32      writeRaw(const Byte8 *pBytes, UInt32 nLength);

        void        begin(const Node &node);
        void        put(const DataPtr &value);
        void        end();
        UInt32      write_int(UInt32 n);
        UInt32      write_byte(Byte8 byte);
        UInt32      write(const Byte8 *pBytes, UInt32 nLength);

        virtual I_Encoding &
                    getEncoding() = 0;

        int         getRuntimeVersion();
        int         getArchiveVersion();

        void        setArchiveVersion(int archiveVersion);
        void        setRuntimeVersion(int runtimeVersion);

        void        suppressVersionStamp(bool suppress = true);

    private:

        void        writeVersionStamp();

        std::ostream *  mpOs;
        int             mRcfRuntimeVersion;
        int             mArchiveVersion;
        bool            mSuppressVersionStamp;
        bool            mVersionStampWritten;
    };

} // namespace SF

#include <SF/Archive.hpp>
#include <SF/Serializer.hpp>

#if !defined(_MSC_VER) || _MSC_VER > 1200

namespace SF {

    template<typename T>
    WithSemanticsRead &WithSemanticsRead::operator>>(T &t)
    {
        Archive ar(Archive::READ, static_cast<IStream *>(this) );
        ar & t;
        return *this;
    }

    template<typename T>
    WithSemanticsRead &WithSemanticsRead::operator>>(const T &t)
    {
        Archive ar(Archive::READ, static_cast<IStream *>(this) );
        ar & t;
        return *this;
    }

    template<typename T>
    WithSemanticsWrite &WithSemanticsWrite::operator<<(const T &t)
    {
        Archive ar(Archive::WRITE, static_cast<OStream *>(this) );
        ar & t;
        return *this;
    }

} // namespace SF

#endif

#endif // !INCLUDE_SF_STREAM_HPP
