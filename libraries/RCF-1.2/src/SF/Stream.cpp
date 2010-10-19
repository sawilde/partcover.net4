
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <SF/Stream.hpp>

#include <RCF/ByteOrdering.hpp>
#include <RCF/Version.hpp>

#include <SF/DataPtr.hpp>
#include <SF/Encoding.hpp>
#include <SF/Node.hpp>
#include <SF/Tools.hpp>

#include <vector>

namespace SF {

    // ContextRead

    ContextRead::ContextRead() :
        bEnabled_(true)
    {}

    ContextRead::~ContextRead()
    {}

    void ContextRead::add(UInt32 nid, const ObjectId &id)
    {
        if (bEnabled_)
        {
            if (nid_id_ptr_.get() == NULL)
            {
                nid_id_ptr_.reset( new std::map<UInt32, ObjectId>());
            }
            (*nid_id_ptr_)[nid] = id;
        }
    }

    void ContextRead::add(void *ptr, const std::type_info &objType, void *pObj)
    {
        if (bEnabled_)
        {
            if (type_ptr_obj_ptr_.get() == NULL)
            {
                type_ptr_obj_ptr_.reset( new std::map<std::string, std::map< void *, void * > >() );
            }
            (*type_ptr_obj_ptr_)[ objType.name() ][ ptr ] = pObj;
        }
    }

    bool ContextRead::query(UInt32 nid, ObjectId &id)
    {
        if (nid_id_ptr_.get() && nid_id_ptr_->find( nid ) != nid_id_ptr_->end())
        {
            id = (*nid_id_ptr_)[ nid ];
            return true;
        }
        else
        {
            return false;
        }
    }

    bool ContextRead::query(void *ptr, const std::type_info &objType, void *&pObj)
    {
        if (type_ptr_obj_ptr_.get() &&
            (*type_ptr_obj_ptr_)[ objType.name() ].find(ptr) != (*type_ptr_obj_ptr_)[ objType.name() ].end() )
        {
            pObj = (*type_ptr_obj_ptr_)[ objType.name() ][ ptr ];
            return true;
        }
        else
        {
            return false;
        }
    }

    void ContextRead::clear()
    {
        if (nid_id_ptr_.get())
        {
            nid_id_ptr_->clear();
        }
        if (type_ptr_obj_ptr_.get())
        {
            type_ptr_obj_ptr_->clear();
        }
    }

    // ContextWrite

    ContextWrite::ContextWrite() :
        bEnabled_(false),
        currentId_(1)
    {}

    ContextWrite::~ContextWrite()
    {}

    void ContextWrite::setEnabled(bool enabled)
    {
        bEnabled_ = enabled;
        if (bEnabled_ && id_nid_ptr_.get() == NULL)
        {
            id_nid_ptr_.reset( new std::map<ObjectId, UInt32>());
        }
    }

    bool ContextWrite::getEnabled()
    {
        return bEnabled_;
    }

    void ContextWrite::add(const ObjectId &id, UInt32 &nid)
    {
        if (bEnabled_)
        {
            if (id_nid_ptr_->find( id ) != id_nid_ptr_->end())
            {
                nid = (*id_nid_ptr_)[ id ];
            }
            else
            {
                nid = (*id_nid_ptr_)[ id ] = currentId_++;
            }
        }
    }
    bool ContextWrite::query(const ObjectId &id, UInt32 &nid)
    {
        if ( bEnabled_ && id_nid_ptr_->find( id ) != id_nid_ptr_->end() )
        {
            nid = (*id_nid_ptr_)[ id ];
            return true;
        }
        else
        {
            return false;
        }
    }

    void ContextWrite::clear()
    {
        if (id_nid_ptr_.get())
        {
            id_nid_ptr_->clear();
        }
        currentId_ = 1;
    }

    // WithContextRead

    WithContextRead::~WithContextRead()
    {}

    ContextRead &WithContextRead::getContext()
    {
        return context;
    }

    // WithContextWrite

    WithContextWrite::~WithContextWrite()
    {}

    ContextWrite &WithContextWrite::getContext()
    {
        return context;
    }

    // LocalStorage

    LocalStorage::LocalStorage() :
        pNode_(RCF_DEFAULT_INIT)
    {}

    LocalStorage::~LocalStorage()
    {}

    void LocalStorage::setNode(Node *pNode)
    {
        pNode_ = pNode;
    }

    Node *LocalStorage::getNode()
    {
        return pNode_;
    }

    // WithLocalStorage

    WithLocalStorage::~WithLocalStorage()
    {}

    LocalStorage &WithLocalStorage::getLocalStorage()
    {
        return localStorage_;
    }


    enum {
        Begin,
        Data,
        End,
        Blank,
        BeginVersionStamp
    };


    // IStream

    IStream::IStream() :
        mpIs(RCF_DEFAULT_INIT),
        mpIstr(RCF_DEFAULT_INIT),
        mRcfRuntimeVersion( RCF::getRuntimeVersion() ),
        mArchiveVersion(0),
        mIgnoreVersionStamp(false)
    {
    }

    IStream::IStream(
        std::istream &  is, 
        std::size_t     archiveSize, 
        int             rcfRuntimeVersion, 
        int             archiveVersion) :

            mpIs(RCF_DEFAULT_INIT),
            mpIstr(RCF_DEFAULT_INIT),
            mArchiveSize(RCF_DEFAULT_INIT),
            mRcfRuntimeVersion( rcfRuntimeVersion ),
            mArchiveVersion( archiveVersion ),
            mIgnoreVersionStamp(false)
    {
        setIs(is, archiveSize, rcfRuntimeVersion, archiveVersion);
    }

    void IStream::setIs(
        std::istream &  is, 
        std::size_t     archiveSize, 
        int             rcfRuntimeVersion, 
        int             archiveVersion) 
    { 
        mpIs = &is; 
        mpIstr = dynamic_cast<std::istrstream *>(mpIs); 
        mArchiveSize = archiveSize;

        mRcfRuntimeVersion = rcfRuntimeVersion;
        if (mRcfRuntimeVersion == 0)
        {
            mRcfRuntimeVersion = RCF::getRuntimeVersion();
        }

        mArchiveVersion = archiveVersion;
    }

    bool IStream::verifyAgainstArchiveSize(std::size_t bytesToRead)
    {
        if (mArchiveSize)
        {
            std::size_t bytesRead = mpIs->tellg();
            std::size_t bytesRemaining = mArchiveSize - bytesRead;
            return bytesToRead <= bytesRemaining;
        }
        else
        {
            return false;
        }
    }

    void IStream::clearState() 
    { 
        WithContextRead::getContext().clear();
    }

    bool IStream::begin(Node &node)
    {
        while (true)
        {
            Byte8 byte = 0;
            read_byte(byte);

            switch (byte)
            {
            case Blank: 
                {
                    Byte8 count = 0;
                    read_byte(count);
                    std::vector<Byte8> buffer(count);
                    UInt32 bytesRead = read( &(buffer[0]), count);
                    if (bytesRead != static_cast<UInt32>(count))
                    {
                        RCF_THROW(RCF::Exception(RCF::_SfError_DataFormat()))(bytesRead)(count);
                    }
                    continue;
                }

            case BeginVersionStamp:
                {
                    int runtimeVersion = 0;
                    int archiveVersion = 0;

                    const size_t BufferLen = 10;
                    char buffer[BufferLen] = {0};
                    RCF::ByteBuffer byteBuffer( &buffer[0], BufferLen);
                    std::size_t pos0 = mpIs->tellg();

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4996 )  // warning C4996: 'std::basic_istream<_Elem,_Traits>::readsome': Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct. To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'
#endif
                    std::size_t bytesRead = mpIs->readsome(buffer, BufferLen);

#ifdef _MSC_VER
#pragma warning( pop )
#endif
                    byteBuffer = RCF::ByteBuffer(byteBuffer, 0, bytesRead);
                    std::size_t pos1 = 0;
                    decodeInt(runtimeVersion, byteBuffer, pos1);
                    decodeInt(archiveVersion, byteBuffer, pos1);

                    mpIs->seekg(
                        static_cast<std::istream::off_type>(pos0 + pos1), 
                        std::ios_base::beg);

                    if (!mIgnoreVersionStamp)
                    {
                        mRcfRuntimeVersion = runtimeVersion;
                        mArchiveVersion = archiveVersion;
                    }

                    continue;
                }

            case Begin:
                {
                    read_byte( byte );
                    Byte8 attrSpec = byte;

                    // id
                    if (attrSpec & 1)
                    {
                        read_int(node.id);
                    }

                    // ref
                    attrSpec = attrSpec >> 1;
                    if (attrSpec & 1)
                    {
                        node.ref = 1;
                    }

                    // type
                    attrSpec = attrSpec >> 1;
                    if (attrSpec & 1)
                    {
                        UInt32 length = 0;
                        read_int(length);
                        node.type.allocate(length);
                        read(node.type.get(), length );
                    }

                    // label
                    attrSpec = attrSpec >> 1;
                    if (attrSpec & 1)
                    {
                        UInt32 length = 0;
                        read_int(length);
                        node.label.allocate(length);
                        read(node.label.get(), length);
                    }

                    return true;
                }

            default:

                RCF_THROW(RCF::Exception(RCF::_SfError_DataFormat()))(byte);
            }
        }

    }

    bool IStream::get(DataPtr &value)
    {
        Byte8 byte;
        read_byte( byte );
        if (byte == Data)
        {
            UInt32 length = 0;
            read_int(length);
            value.allocate(length);
            read(value.get(), length);
            return true;
        }
        else
        {
            putback_byte(byte);
            return false;
        }
    }

    void IStream::end()
    {
        Byte8 byte;
        read_byte(byte);
        if (byte != End)
        {
            RCF_THROW(RCF::Exception(RCF::_SfError_DataFormat(), "no end symbol"))(byte);
        }
    }

    UInt32 IStream::read_int(UInt32 &n)
    {
        UInt32 bytesRead = read( reinterpret_cast<SF::Byte8 *>(&n), 4);
        RCF::networkToMachineOrder( &n, 4, 1);
        return bytesRead;
    }

    UInt32 IStream::read_byte(Byte8 &byte)
    {
        UInt32 bytesRead = read(&byte, 1);
        return bytesRead;
    }

    UInt32 IStream::read(Byte8 *pBytes, UInt32 nLength)
    {
        mpIs->read(pBytes, nLength);
        if (mpIs->fail())
        {
            RCF_THROW(RCF::Exception(RCF::_SfError_ReadFailure()))(nLength)(mpIs->gcount());
        }
        return static_cast<UInt32>(mpIs->gcount());
    }

    bool IStream::supportsReadRaw()
    {
        return mpIstr != NULL;
    }

    void IStream::putback_byte( Byte8 byte )
    {
        mpIs->putback(byte);
    }

    int IStream::getRuntimeVersion()
    {
        return mRcfRuntimeVersion;
    }

    int IStream::getArchiveVersion()
    {
        return mArchiveVersion;
    }

    void IStream::setArchiveVersion(int archiveVersion)
    {
        mArchiveVersion = archiveVersion;
    }

    void IStream::setRuntimeVersion(int runtimeVersion)
    {
        mRcfRuntimeVersion = runtimeVersion;
    }

    void IStream::ignoreVersionStamp(bool ignore)
    {
        mIgnoreVersionStamp = ignore;
    }

    // OStream

    OStream::OStream() : 
        mpOs(RCF_DEFAULT_INIT), 
        mRcfRuntimeVersion(0), 
        mArchiveVersion(0),
        mSuppressVersionStamp(false),
        mVersionStampWritten(false)
    {
    }

    OStream::OStream(
        std::ostream &  os, 
        int             rcfRuntimeVersion, 
        int             archiveVersion) : 

            mpOs(RCF_DEFAULT_INIT), 
            mRcfRuntimeVersion(rcfRuntimeVersion),
            mSuppressVersionStamp(false),
            mVersionStampWritten(false)
    {
        setOs(os, rcfRuntimeVersion, archiveVersion);
    }

    void OStream::setOs(
        std::ostream &  os, 
        int             rcfRuntimeVersion, 
        int             archiveVersion) 
    { 
        mpOs = &os; 

        mRcfRuntimeVersion = rcfRuntimeVersion;
        if (mRcfRuntimeVersion == 0)
        {
            mRcfRuntimeVersion = RCF::getRuntimeVersion();
        }

        mArchiveVersion = archiveVersion;
    }

    void OStream::clearState() 
    { 
        WithContextWrite::getContext().clear(); 

        mVersionStampWritten = false;
    }

    void OStream::writeVersionStamp()
    {

        if (mRcfRuntimeVersion >= 6)
        {
            const std::size_t BufferLen = 1+5+5;
            char buffer[BufferLen] = {0};
            buffer[0] = BeginVersionStamp;

            RCF::ByteBuffer byteBuffer(&buffer[0], BufferLen);
            std::size_t pos = 1;
            encodeInt(mRcfRuntimeVersion, byteBuffer, pos);
            encodeInt(mArchiveVersion, byteBuffer, pos);

            writeRaw(&buffer[0], static_cast<UInt32>(pos));
        }
    }

    void OStream::begin( const Node &node )
    {
        if (!mSuppressVersionStamp && !mVersionStampWritten)
        {
            writeVersionStamp();
            mVersionStampWritten = true;
        }

        write_byte( (Byte8) Begin );

        Byte8 attrSpec = 0;
        if (node.id != 0)
        {
            attrSpec |= 1<<0;
        }
        if (node.ref != 0)
        {
            attrSpec |= 1<<1;
        }
        if (node.type.get() != NULL)
        {
            attrSpec |= 1<<2;
        }
        if (node.label.get() != NULL)
        {
            attrSpec |= 1<<3;
        }

        write_byte( attrSpec );

        if (node.id != 0)
        {
            write_int(node.id);
        }
        if (node.type.get() != NULL)
        {
            write(node.type.get(), node.type.length());
        }
        if (node.label.get() != NULL)
        {
            write(node.label.get(), node.label.length() );
        }
    }

    void OStream::put(const DataPtr &value)
    {
        write_byte( (Byte8) Data );
        write(value.get(), value.length());
    }

    void OStream::end()
    {
        write_byte( (Byte8) End );
    }

    UInt32 OStream::write_int(UInt32 n)
    {
        BOOST_STATIC_ASSERT( sizeof(n) == 4 );
        RCF::machineToNetworkOrder(&n, 4, 1);
        mpOs->write( reinterpret_cast<char*>(&n), 4);
        if (mpOs->fail())
        {
            RCF_THROW(RCF::Exception(RCF::_SfError_WriteFailure()))(n);
        }
        return 4;
    }

    UInt32 OStream::write_byte(Byte8 byte)
    {
        mpOs->write(&byte, 1);
        if (mpOs->fail())
        {
            RCF_THROW(RCF::Exception(RCF::_SfError_WriteFailure()));
        }
        return 1;
    }

    UInt32 OStream::write(const Byte8 *pBytes, UInt32 nLength)
    {
        UInt32 bytesWritten = 0;
        bytesWritten += write_int(nLength);
        mpOs->write(pBytes, nLength);
        if (mpOs->fail())
        {
            RCF_THROW(RCF::Exception(RCF::_SfError_WriteFailure()))(nLength);
        }
        bytesWritten += nLength;
        return bytesWritten;
    }

    UInt32 OStream::writeRaw(const Byte8 *pBytes, UInt32 nLength)
    {
        mpOs->write(pBytes, nLength);
        if (mpOs->fail())
        {
            RCF_THROW(RCF::Exception(RCF::_SfError_WriteFailure()))(nLength);
        }
        return nLength;
    }

    int OStream::getRuntimeVersion()
    {
        return mRcfRuntimeVersion;
    }

    int OStream::getArchiveVersion()
    {
        return mArchiveVersion;
    }

    void OStream::setArchiveVersion(int archiveVersion)
    {
        mArchiveVersion = archiveVersion;
    }

    void OStream::setRuntimeVersion(int runtimeVersion)
    {
        mRcfRuntimeVersion = runtimeVersion;
    }

    void OStream::suppressVersionStamp(bool suppress)
    {
        mSuppressVersionStamp = suppress;
    }

} // namespace SF
