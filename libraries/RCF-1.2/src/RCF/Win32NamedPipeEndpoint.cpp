
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/Win32NamedPipeEndpoint.hpp>

#include <RCF/InitDeinit.hpp>
#include <RCF/util/Tchar.hpp>
#include <RCF/Win32NamedPipeClientTransport.hpp>
#include <RCF/Win32NamedPipeServerTransport.hpp>

#ifdef RCF_USE_SF_SERIALIZATION
#include <SF/Registry.hpp>
#endif

// missing stuff in mingw and vc6 headers
#if defined(__MINGW32__) || (defined(_MSC_VER) && _MSC_VER == 1200)
#define FILE_FLAG_FIRST_PIPE_INSTANCE   0x00080000
#endif

namespace RCF {

    Win32NamedPipeEndpoint::Win32NamedPipeEndpoint()
    {}

    Win32NamedPipeEndpoint::Win32NamedPipeEndpoint(
        const tstring & pipeName) :
            mPipeName(pipeName)
    {}

    ServerTransportAutoPtr Win32NamedPipeEndpoint::createServerTransport() const
    {
        return ServerTransportAutoPtr(
            new Win32NamedPipeServerTransport(mPipeName));
    }

    ClientTransportAutoPtr Win32NamedPipeEndpoint::createClientTransport() const
    {            
        return ClientTransportAutoPtr(
            new Win32NamedPipeClientTransport(mPipeName));
    }

    EndpointPtr Win32NamedPipeEndpoint::clone() const
    {
        return EndpointPtr( new Win32NamedPipeEndpoint(*this) );
    }

    std::pair<tstring, HANDLE> generateNewPipeName()
    {
        tstring pipePrefix = RCF_T("\\\\.\\pipe\\RcfTestPipe_");
        static unsigned int i = 0;
        while (true)
        {

#ifdef UNICODE
            typedef std::wostringstream     tostringstream;
#else
            typedef std::ostringstream      tostringstream;
#endif
            tostringstream tos;
            tos 
                << pipePrefix
                << ++i;

            tstring candidateName = tos.str();

            DWORD dwOpenMode = 
                    PIPE_ACCESS_DUPLEX 
                |   FILE_FLAG_OVERLAPPED 
                |   FILE_FLAG_FIRST_PIPE_INSTANCE;

            DWORD dwPipeMode = 
                    PIPE_TYPE_BYTE 
                |   PIPE_READMODE_BYTE 
                |   PIPE_WAIT;

            HANDLE hPipe = CreateNamedPipe( 
                candidateName.c_str(),
                dwOpenMode,
                dwPipeMode,
                1,          // MaxPipeInstances
                4096,       // OutBufferSize
                4096,       // InBufferSize
                0,          // DefaultTimeoutMs
                NULL);      // pSecurityASttributes

            DWORD dwErr = GetLastError();
            RCF_UNUSED_VARIABLE(dwErr);

            if (hPipe != INVALID_HANDLE_VALUE)
            {
                return std::make_pair(candidateName, hPipe);
            }

            CloseHandle(hPipe);
        }
    }

    std::string Win32NamedPipeEndpoint::asString()
    {
        std::ostringstream os;
        os << "Named pipe endpoint \"" << util::toString(mPipeName) << "\"";
        return os.str();
    }


#ifdef RCF_USE_SF_SERIALIZATION

    void Win32NamedPipeEndpoint::serialize(SF::Archive &ar)
    {
        serializeParent( (I_Endpoint*) 0, ar, *this);
        ar & mPipeName;
    }

#endif

    inline void initWin32NamedPipeEndpointSerialization()
    {
#ifdef RCF_USE_SF_SERIALIZATION
        SF::registerType( (Win32NamedPipeEndpoint *) 0, "RCF::Win32NamedPipeEndpoint");
        SF::registerBaseAndDerived( (I_Endpoint *) 0, (Win32NamedPipeEndpoint *) 0);
#endif
    }

    RCF_ON_INIT_NAMED( initWin32NamedPipeEndpointSerialization(), InitWin32NamedPipeEndpointSerialization );

} // namespace RCF
