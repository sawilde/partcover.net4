
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_SERIALIZATIONPROTOCOL_HPP
#define INCLUDE_RCF_SERIALIZATIONPROTOCOL_HPP

#include <map>
#include <string>
#include <strstream>

#include <boost/mpl/bool_fwd.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/ByteOrdering.hpp>

#ifdef RCF_USE_PROTOBUF
#include <google/protobuf/message.h>
#endif

// Serialization protocols

#include <RCF/SerializationDefs.hpp>
#include <RCF/Protocol/Protocol.hpp>

#if defined(RCF_USE_SF_SERIALIZATION)
#include <RCF/Protocol/SF.hpp>
#endif

#if defined(RCF_USE_BOOST_SERIALIZATION) || defined(RCF_USE_BOOST_XML_SERIALIZATION)
#include <RCF/Protocol/BoostSerialization.hpp>
#endif

namespace RCF {

    RCF_EXPORT bool isSerializationProtocolSupported(int protocol);

    RCF_EXPORT std::string getSerializationProtocolName(int protocol);

    class PointerContext
    {
    public:
        template<typename SmartPtr, typename T>
        SmartPtr get(SmartPtr *, T t)
        {
            // TODO: need to use typeid().name()
            return static_cast<SmartPtr>(
                mPtrMap[&typeid(SmartPtr)][ static_cast<void *>(t) ] );
        }

        template<typename SmartPtr, typename T>
        void set(SmartPtr sp, T t)
        {
            mPtrMap[&typeid(SmartPtr)][ static_cast<void *>(t) ] =
                static_cast<void *>(sp);
        }

        void clear()
        {
            mPtrMap.clear();
        }

    private:
        std::map< const std::type_info *, std::map<void*, void*> > mPtrMap;
    };


    class Token;
    class MethodInvocationRequest;
    class MethodInvocationResponse;

    class RCF_EXPORT SerializationProtocolIn
    {
    public:
        SerializationProtocolIn();
        ~SerializationProtocolIn();

        std::istream& getIstream() { return *mIsPtr; }

        void            setSerializationProtocol(int protocol);
        int             getSerializationProtocol() const;

        void            reset(
                            const ByteBuffer &data, 
                            int protocol, 
                            int rcfRuntimeVersion, 
                            int archiveVersion);

        void            clearByteBuffer();
        void            clear();
        void            extractSlice(ByteBuffer &byteBuffer, std::size_t len);
        std::size_t     getArchiveLength();
        std::size_t     getRemainingArchiveLength();

        template<typename T>
        void read(const T *pt)
        {
            read(*pt);
        }

        template<typename T>
        void read(T &t)
        {
            try
            {
                switch (mProtocol)
                {
                case 1: mInProtocol1 >> t; break;
                case 2: mInProtocol2 >> t; break;
                case 3: mInProtocol3 >> t; break;
                case 4: mInProtocol4 >> t; break;

#ifdef RCF_USE_BOOST_XML_SERIALIZATION
                case 5: mInProtocol5 >> boost::serialization::make_nvp("Dummy", t); break;
#else
                case 5: mInProtocol5 >> t; break;
#endif

                default: RCF_ASSERT(0)(mProtocol);
                }
            }
            catch(const RCF::Exception &e)
            {
                RCF_UNUSED_VARIABLE(e);
                throw;
            }
            catch(const std::exception &e)
            {
                std::ostringstream os;
                os
                    << "Deserialization error, object type: "
                    << typeid(t).name()
                    << ", error type: "
                    << typeid(e).name()
                    << ", error msg: "
                    << e.what();

                RCF_THROW(
                    RCF::SerializationException(
                        _RcfError_Deserialization(typeid(t).name(), typeid(e).name(), e.what()),
                        os.str()));
            }
        }

        PointerContext mPointerContext;

        int             getRuntimeVersion();

    private:

        void            bindProtocol();
        void            unbindProtocol();

        friend class ClientStub; // TODO
        friend class RcfSession; // TODO

        int                                     mProtocol;
        ByteBuffer                              mByteBuffer;
        std::istrstream *                       mIsPtr;
        std::vector<char>                       mIstrVec;

        Protocol< boost::mpl::int_<1> >::In     mInProtocol1;
        Protocol< boost::mpl::int_<2> >::In     mInProtocol2;
        Protocol< boost::mpl::int_<3> >::In     mInProtocol3;
        Protocol< boost::mpl::int_<4> >::In     mInProtocol4;
        Protocol< boost::mpl::int_<5> >::In     mInProtocol5;

        int                                     mRcfRuntimeVersion;
        int                                     mArchiveVersion;
    };

    class RCF_EXPORT SerializationProtocolOut
    {
    public:
        SerializationProtocolOut();

        std::ostrstream& getOstrstream() { return *mOsPtr; }

        void    setSerializationProtocol(int protocol);
        int     getSerializationProtocol() const;
        void    clear();
        void    reset(
                    int protocol,
                    std::size_t margin,
                    ByteBuffer byteBuffer,
                    int rcfRuntimeVersion,
                    int archiveVersion);

        template<typename T>
        void    write(const T &t)
        {
            try
            {
                switch (mProtocol)
                {
                case 1: mOutProtocol1 << t; break;
                case 2: mOutProtocol2 << t; break;
                case 3: mOutProtocol3 << t; break;
                case 4: mOutProtocol4 << t; break;

#ifdef RCF_USE_BOOST_XML_SERIALIZATION
                case 5: mOutProtocol5 << boost::serialization::make_nvp("Dummy", t); break;
#else
                case 5: mOutProtocol5 << t; break;
#endif

                default: RCF_ASSERT(0)(mProtocol);
                }
            }
            catch(const std::exception &e)
            {
                std::ostringstream os;
                os
                    << "serialization error, object type: "
                    << typeid(t).name()
                    << ", error type: "
                    << typeid(e).name()
                    << ", error msg: "
                    << e.what();

                RCF_THROW(
                    RCF::SerializationException(
                        _RcfError_Serialization(typeid(t).name(), typeid(e).name(), e.what()),
                        os.str()));
            }
        }

        void    insert(const ByteBuffer &byteBuffer);
        void    extractByteBuffers();
        void    extractByteBuffers(std::vector<ByteBuffer> &byteBuffers);

        int     getRuntimeVersion();

    private:

        void    bindProtocol();
        void    unbindProtocol();


        friend class ClientStub; // TODO
        friend class RcfSession; // TODO
        
        int                                                 mProtocol;
        std::size_t                                         mMargin;
        boost::shared_ptr<std::ostrstream>                  mOsPtr;
        std::vector<std::pair<std::size_t, ByteBuffer> >    mByteBuffers;

        // these need to be below mOsPtr, for good order of destruction
        Protocol< boost::mpl::int_<1> >::Out                mOutProtocol1;
        Protocol< boost::mpl::int_<2> >::Out                mOutProtocol2;
        Protocol< boost::mpl::int_<3> >::Out                mOutProtocol3;
        Protocol< boost::mpl::int_<4> >::Out                mOutProtocol4;
        Protocol< boost::mpl::int_<5> >::Out                mOutProtocol5;

        int                                                 mRcfRuntimeVersion;
        int                                                 mArchiveVersion;
    };

    inline void serialize(
        SerializationProtocolOut &,
        const Void *)
    {
    }

    inline void serialize(
        SerializationProtocolOut &,
        const Void &)
    {
    }

    inline void deserialize(
        SerializationProtocolIn &,
        Void *)
    {
    }

    inline void deserialize(
        SerializationProtocolIn &,
        Void &)
    {
    }

    template<typename T>
    void serializeImpl(
        SerializationProtocolOut &out,
        const T &t,
        long int)
    {
        out.write(t);
    }

    template<typename T>
    void deserializeImpl(
        SerializationProtocolIn &in,
        T &t,
        long int)
    {
        in.read(t);
    }

#ifdef RCF_USE_PROTOBUF

    // Some compile-time gymnastics to detect Protobuf classes, so we don't 
    // pass them off to SF or Boost.Serialization.

    template<typename T>
    void serializeProtobufOrNot(
        SerializationProtocolOut &out,
        const T &t,
        boost::mpl::false_ *)
    {
        serializeImpl(out, t, 0);
    }

    template<typename T>
    void deserializeProtobufOrNot(
        SerializationProtocolIn &in,
        T &t,
        boost::mpl::false_ *)
    {
        deserializeImpl(in, t, 0);
    }

    template<typename T>
    void serializeProtobufOrNot(
        SerializationProtocolOut &out,
        const T & t,
        boost::mpl::true_ *)
    {
        std::ostrstream & os = out.getOstrstream();
        
        os.write("%%%%", 4);
        std::size_t beginPos = os.tellp();

        // Make room for the protobuf object.
        // TODO: Less obtuse way of reserving space.
        int byteSize = t.ByteSize();
        for (int i=0; i<byteSize; ++i)
        {
            os.write("%", 1);
        }
        std::size_t endPos = os.tellp();
        RCF_ASSERT(endPos > beginPos);

        // Write the protobuf object straight into the underlying buffer.
        char * pch = os.str();
        os.freeze(false);
        bool ok = t.SerializeToArray(pch + beginPos, endPos - beginPos);
        RCF_VERIFY(ok, Exception(_RcfError_ProtobufWrite(typeid(t).name())))(typeid(t));

        // Write the prepended length field.
        boost::uint32_t len = endPos - beginPos;
        char buffer[4] = {0};
        memcpy(buffer, &len, 4);
        RCF::machineToNetworkOrder(buffer, 4, 1);
        os.seekp(beginPos-4);
        os.write(buffer, 4);
        os.seekp(endPos);
    }

    template<typename T>
    void serializeProtobufOrNot(
        SerializationProtocolOut &out,
        const T * pt,
        boost::mpl::true_ *)
    {
        serializeProtobufOrNot(out, *pt, (boost::mpl::true_ *) NULL);
    }

    template<typename T>
    void serializeProtobufOrNot(
        SerializationProtocolOut &out,
        T * pt,
        boost::mpl::true_ *)
    {
        serializeProtobufOrNot(out, *pt, (boost::mpl::true_ *) NULL);
    }

    template<typename T>
    void deserializeProtobufOrNot(
        SerializationProtocolIn &in,
        T &t,
        boost::mpl::true_ *)
    {
        std::istream & is = in.getIstream();

        char buffer[4];
        is.read(buffer, 4);
        boost::uint32_t len = 0;
        memcpy( &len, buffer, 4);
        RCF::networkToMachineOrder(&len, 4, 1);

        ByteBuffer byteBuffer;
        in.extractSlice(byteBuffer, len);
        bool ok = t.ParseFromArray(byteBuffer.getPtr(), byteBuffer.getLength());
        RCF_VERIFY(ok, Exception(_RcfError_ProtobufRead(typeid(t).name())))(typeid(t));
    }

    template<typename T>
    void deserializeProtobufOrNot(
        SerializationProtocolIn &in,
        T * & pt,
        boost::mpl::true_ *)
    {
        if (pt == NULL)
        {
            pt = new T();
        }

        deserializeProtobufOrNot(in, *pt, (boost::mpl::true_ *) NULL);
    }

    template<typename T>
    void serialize(
        SerializationProtocolOut &out,
        const T * pt)
    {
        typedef boost::is_base_and_derived<google::protobuf::Message, T>::type type;
        serializeProtobufOrNot(out, pt, (type *) NULL);
    }

    template<typename T>
    void serialize(
        SerializationProtocolOut &out,
        T * pt)
    {
        typedef boost::is_base_and_derived<google::protobuf::Message, T>::type type;
        serializeProtobufOrNot(out, pt, (type *) NULL);
    }

    template<typename T>
    void serialize(
        SerializationProtocolOut &out,
        const T & t)
    {
        typedef boost::is_base_and_derived<google::protobuf::Message, T>::type type;
        serializeProtobufOrNot(out, t, (type *) NULL);
    }

    template<typename T>
    void deserialize(
        SerializationProtocolIn &in,
        T * & pt)
    {
        typedef boost::is_base_and_derived<google::protobuf::Message, T>::type type;
        deserializeProtobufOrNot(in, pt, (type *) NULL);
    }

    template<typename T>
    void deserialize(
        SerializationProtocolIn &in,
        T & t)
    {
        typedef boost::is_base_and_derived<google::protobuf::Message, T>::type type;
        deserializeProtobufOrNot(in, t, (type *) NULL);
    }

#else

    // Whatever is passed to serialize() and deserialize(), is passed directly
    // on to the selected serialization framework. We have to be careful with
    // this, because Boost.Serialization is very picky about whether one 
    // serializes a pointer or a value.

    template<typename T>
    void serialize(
        SerializationProtocolOut &out,
        const T & t)
    {
        serializeImpl(out, t, 0);
    }

    template<typename T>
    void deserialize(
        SerializationProtocolIn &in,
        T & t)
    {
        deserializeImpl(in, t, 0);
    }

#endif

} // namespace RCF

#endif // ! INCLUDE_RCF_SERIALIZATIONPROTOCOL_HPP
