
#include <stdexcept>
#include <string>

#include <RCF/test/TestMinimal.hpp>

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/test/TransportFactories.hpp>
#include <RCF/util/CommandLine.hpp>

#include <SF/Archive.hpp>
#include <SF/Registry.hpp>
#include <SF/string.hpp>

namespace Test_ErrorReporting {

    std::string makeString(std::size_t len)
    {
        std::string s;
        for (std::size_t i=0; i<len; ++i)
        {
            s += '0' + i%10;
        }
        return s;
    }

    // a class to trigger serialization errors in read or write direction
    class A
    {
    public:
        A() : a1(RCF_DEFAULT_INIT), a2(RCF_DEFAULT_INIT), a3(), mWhich(RCF_DEFAULT_INIT)
        {}

        A(int which) : a1(17), a2(18.19), a3("twenty"), mWhich(which)
        {}

        class MyException : public std::runtime_error
        {
        public:
            MyException() : std::runtime_error("MyException: no go")
            {}

            ~MyException() throw()
            {}
        };

        template<typename Archive>
        void serialize(Archive &ar, const unsigned int)
        {
            RCF_ASSERT(0); // won't be doing boost serialization in this test. TODO: maybe should?
        }

        void serialize(SF::Archive &ar)
        {
            RCF_ASSERT(ar.isRead() || ar.isWrite());
            switch (mWhich)
            {
            case 0:
                ar & a1 & a2;
                break;
            case 1:
                ar & a1 & a2;
                throw MyException();
                ar & a3;
                break;
            case 2:
                ar & a3;
                break;
            default:
                RCF_ASSERT(0);
            }
        }

        int mWhich;

        int a1;
        double a2;
        std::string a3;
    };

    class MyRemoteException : public RCF::RemoteException
    {
    public:

        MyRemoteException() :
            mError(RCF_DEFAULT_INIT),
            mWhat()
        {}

        MyRemoteException(int error, const std::string &msg) :
            mError(error),
            mWhat(msg)
        {}

        ~MyRemoteException() throw()
        {}

        const char *what() const throw()
        {
            return mWhat.c_str();
        }

        int getErrorId() const
        {
            return mError;
        }

        void serialize(SF::Archive &ar)
        {
            ar & mError & mWhat;
        }

        template<typename Archive>
        void serialize(Archive &ar, const unsigned int)
        {
            ar & mError & mWhat;
        }

    private:

        void throwSelf() const
        {
            MyRemoteException myRemoteException(*this);
            throw myRemoteException;
        }

        int mError;
        std::string mWhat;

    };

    AUTO_RUN( SF::registerBaseAndDerived( (RCF::RemoteException*) 0, (MyRemoteException*) 0) );
    AUTO_RUN( SF::registerType( (MyRemoteException*) 0, "MyRemoteException") );

    class Echo
    {
    public:
        std::string echo(const std::string &s)
        {
            sLog = s;
            return s;
        }
        static std::string sLog;

        std::size_t putString(const std::string &s)
        {
            return s.size();
        }

        std::string getString(std::size_t len)
        {
            return makeString(len);
        }

        void putA(const A &a)
        {
        }

        A getA(int which)
        {
            return A(which);
        }

        void throwRemoteException(int errorCode, const std::string &errorString)
        {
            throw RCF::RemoteException( RCF::Error(errorCode), errorString);
        }

        void throwMyRemoteException(int errorCode, const std::string &errorString)
        {
            throw MyRemoteException(errorCode, errorString);
        }
    };

    std::string Echo::sLog;

} // namespace Test_ErrorReporting

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_ErrorReporting::A)
RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(Test_ErrorReporting::MyRemoteException)

namespace Test_ErrorReporting {

    RCF_BEGIN(I_Echo, "I_Echo")
        RCF_METHOD_R1(std::string, echo, const std::string &)
        RCF_METHOD_R1(std::size_t, putString, const std::string &)
        RCF_METHOD_R1(std::string, getString, std::size_t)
        RCF_METHOD_V1(void, putA, const A &)
        RCF_METHOD_R1(A, getA, int)
        RCF_METHOD_V2(void, throwRemoteException, int, const std::string &)
        RCF_METHOD_V2(void, throwMyRemoteException, int, const std::string &)
    RCF_END(I_Echo)

} // namespace Test_ErrorReporting

#ifdef BOOST_WINDOWS
#include <RCF/Win32NamedPipeServerTransport.hpp>
#else
// Dummy declaration so we can use typeid.
namespace RCF { class Win32NamedPipeServerTransport {}; };
#endif

int test_main(int argc, char **argv)
{
    printTestHeader(__FILE__);

    RCF::RcfInitDeinit rcfInitDeinit;

    using namespace Test_ErrorReporting;

    util::CommandLine::getSingleton().parse(argc, argv);

    // Check error translation.
    int rcfError = RCF::RcfError_ClientCancel;
    RCF::Error err(rcfError);
    RCF::Exception e(err);
    std::string eWhat = e.what();
    std::string errorMsg = RCF::getErrorString(rcfError);
    // Assuming here that RcfError_ClientCancel has no replacements...
    RCF_CHECK( eWhat.find(errorMsg) != std::string::npos );

    {
        RCF::Error err(RCF::_RcfError_ClientConnectTimeout(5000, ""));
        std::string s = err.getErrorString();
        RCF_CHECK( s.find("5000") != std::string::npos );
    }

    for (unsigned int i=0; i<RCF::getTransportFactories().size(); ++i)
    {

        RCF::TransportFactoryPtr transportFactoryPtr = RCF::getTransportFactories()[i];
        RCF::TransportPair transports = transportFactoryPtr->createTransports();
        RCF::ServerTransportPtr serverTransportPtr( transports.first );
        RCF::ClientTransportAutoPtr clientTransportAutoPtr( *transports.second );

        RCF::writeTransportTypes(std::cout, *serverTransportPtr, *clientTransportAutoPtr);

        serverTransportPtr->setMaxMessageLength(1000*10);
        clientTransportAutoPtr->setMaxMessageLength(1000*10);

        std::string s0 = "something special";

        Echo echo;
        RCF::RcfServer server( serverTransportPtr );
        server.start();
        server.bind( (I_Echo *) 0, echo);

        //***********************************************
        // message size errors

        // no error
        {
            serverTransportPtr->setMaxMessageLength(1000*10);
            clientTransportAutoPtr->setMaxMessageLength(1000*10);
            RCF_CHECK( RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).putString(makeString(50)) == 50);
            RCF_CHECK( std::string(RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).getString(50)).size() == 50);
        }

        // try to trigger client transport send error
        {
            serverTransportPtr->setMaxMessageLength(1000*10);
            clientTransportAutoPtr->setMaxMessageLength(100);
            RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).putString(makeString(500));
            RCF_CHECK(1==1);
        }

        // trigger client transport receive error
        {
            serverTransportPtr->setMaxMessageLength(1000*10);
            clientTransportAutoPtr->setMaxMessageLength(500);
            try
            {
                RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).getString(500);
                RCF_CHECK(1==0);
            }
            catch(RCF::Exception &e)
            {
                RCF_CHECK(1==1);
                RCF_CHECK( e.getErrorId() == RCF::RcfError_ClientMessageLength );
            }
        }

        // trigger server transport receive error
        {
            serverTransportPtr->setMaxMessageLength(500);
            clientTransportAutoPtr->setMaxMessageLength(1000*10);
            try
            {
                RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).putString(makeString(500));
                RCF_CHECK(1==0);
            }
            catch(RCF::RemoteException &e)
            {
                RCF_CHECK(1==1);
                RCF_CHECK( e.getErrorId() == RCF::RcfError_ServerMessageLength );
            }
            catch(RCF::Exception &e)
            {
                if (typeid(*serverTransportPtr) == typeid(RCF::Win32NamedPipeServerTransport))
                {
                    // TODO: named pipe server transport currently doesn't report 
                    // message length errors.
                }
                else
                {
                    // server did a hard close, we didn't get any error info
                    RCF_CHECK(1==0);
                }
            }
        }

        // try to trigger server transport send error
        {
            serverTransportPtr->setMaxMessageLength(500);
            clientTransportAutoPtr->setMaxMessageLength(1000*10);
            RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).getString(500);
            RCF_CHECK(1==1);
        }

        //************************************************
        // serialization errors

        serverTransportPtr->setMaxMessageLength(1000*10);
        clientTransportAutoPtr->setMaxMessageLength(1000*10);

        // no errors
        A a;
        RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).putA(a);
        a = RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).getA(0);

        // outbound client serialization error
        try
        {
            RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).putA( A(1) );
            RCF_CHECK(1==0);
        }
        catch(const RCF::SerializationException &e)
        {
            std::string msg = e.what();
            RCF_CHECK(1==1);
            RCF_CHECK(e.getErrorId() == RCF::RcfError_Serialization);
        }

        // server deserialization error
        try
        {
            RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).putA( A(2) );
            RCF_CHECK(1==0);
        }
        catch(RCF::RemoteException &e)
        {
            std::string msg = e.what();
            RCF_CHECK(1==1);
            RCF_CHECK(e.getErrorId() == RCF::SfError_DataFormat);
        }

        // server serialization error
        try
        {
            RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).getA(1);
            RCF_CHECK(1==0);
        }
        catch(RCF::RemoteException &e)
        {
            std::string msg = e.what();
            RCF_CHECK(1==1);
            RCF_CHECK(e.getErrorId() == RCF::RcfError_Serialization);
        }

        // inbound client serialization error
        try
        {
            RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).getA(2);
            RCF_CHECK(1==0);
        }
        catch(const RCF::Exception &e)
        {
            std::string msg = e.what();
            RCF_CHECK(1==1);
            RCF_CHECK(e.getErrorId() == RCF::SfError_DataFormat);
        }


        //************************************************
        // returning non RCF error codes and strings, through RemoteException

        int myErrorCode = 12345;
        std::string myErrorString = "qwerty";
        try
        {
            RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).throwRemoteException(myErrorCode, myErrorString);
            RCF_CHECK(1==0);
        }
        catch(const RCF::RemoteException &e)
        {
            std::string msg = e.what();
            RCF_CHECK(1==1);
            RCF_CHECK(e.getErrorId() == myErrorCode);
            //RCF_CHECK(e.what() == myErrorString);
            RCF_CHECK( std::string(e.what()).find(myErrorString) != std::string::npos);
        }

        //************************************************
        // throwing and catching RemoteException-derived exceptions across the wire

        try
        {
            RcfClient<I_Echo>( clientTransportAutoPtr->clone() ).throwMyRemoteException(myErrorCode, myErrorString);
            RCF_CHECK(1==0);
        }
        catch(const MyRemoteException &e)
        {
            std::string msg = e.what();
            RCF_CHECK(1==1);
            RCF_CHECK(e.getErrorId() == myErrorCode);
            RCF_CHECK(e.what() == myErrorString);
        }
        catch(const RCF::RemoteException &e)
        {
            std::string msg = e.what();
            RCF_CHECK(1==0);
            RCF_CHECK(e.getErrorId() == myErrorCode);
            RCF_CHECK(e.what() == myErrorString);
        }


        //************************************************
        // message header errors (eg corruption)

        // TODO


        //************************************************
        // transport filter errors

        // TODO


        //************************************************
        // payload filter errors

        // TODO


    }


    return boost::exit_success;
}
