
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_WIN32NAMEDPIPEENDPOINT_HPP
#define INCLUDE_RCF_WIN32NAMEDPIPEENDPOINT_HPP

#include <RCF/Endpoint.hpp>
#include <RCF/Export.hpp>
#include <RCF/ClientTransport.hpp>
#include <RCF/ServerTransport.hpp>

#include <SF/SerializeParent.hpp>

#include <RCF/util/Tchar.hpp>
#include <tchar.h>

namespace RCF {

    class RCF_EXPORT Win32NamedPipeEndpoint : public I_Endpoint
    {
    public:

        Win32NamedPipeEndpoint();

        Win32NamedPipeEndpoint(const tstring & pipeName);

        ServerTransportAutoPtr createServerTransport() const;
        ClientTransportAutoPtr createClientTransport() const;
        EndpointPtr clone() const;

        std::string asString();

#ifdef RCF_USE_SF_SERIALIZATION

        void serialize(SF::Archive &ar);

#endif

    private:
        tstring mPipeName;
    };

    RCF_EXPORT std::pair<tstring, HANDLE> generateNewPipeName();

} // namespace RCF

RCF_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION(RCF::Win32NamedPipeEndpoint)

#endif // ! INCLUDE_RCF_WIN32NAMEDPIPEENDPOINT_HPP
