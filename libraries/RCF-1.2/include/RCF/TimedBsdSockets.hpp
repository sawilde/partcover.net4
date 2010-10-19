
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TIMEDBSDSOCKETS_HPP
#define INCLUDE_RCF_TIMEDBSDSOCKETS_HPP

#include <RCF/ByteBuffer.hpp>
#include <RCF/ClientProgress.hpp>
#include <RCF/Export.hpp>

namespace RCF {

    class I_PollingFunctor
    {
    public:
        virtual ~I_PollingFunctor() {}
        virtual int operator()(int, int &, bool) = 0;
    };

    class PollingFunctor : public I_PollingFunctor
    {
    public:
        PollingFunctor(
            const ClientProgressPtr &clientProgressPtr,
            ClientProgress::Activity activity,
            unsigned int endTimeMs);

        int operator()(int fd, int &err, bool bRead);

    private:
        ClientProgressPtr mClientProgressPtr;
        ClientProgress::Activity mActivity;
        unsigned int mEndTimeMs;
    };

    // return -2 for timeout, -1 for error, 0 for ready
    RCF_EXPORT int pollSocket(
        unsigned int endTimeMs,
        int fd,
        int &err,
        bool bRead);

    // return -2 for timeout, -1 for error, 0 for ready
    RCF_EXPORT int pollSocketWithProgress(
        const ClientProgressPtr &ClientProgressPtr,
        ClientProgress::Activity activity,
        unsigned int endTimeMs,
        int fd,
        int &err,
        bool bRead);

    //******************************************************
    // nonblocking socket routines

    // returns -2 for timeout, -1 for error, otherwise 0
    RCF_EXPORT int timedConnect(
        I_PollingFunctor &pollingFunctor,
        int &err,
        int fd,
        const sockaddr *addr,
        int addrLen);

    // returns -2 for timeout, -1 for error, otherwise number of bytes sent (> 0)
    RCF_EXPORT int timedSend(
        I_PollingFunctor &pollingFunctor,
        int &err,
        int fd,
        const std::vector<ByteBuffer> &byteBuffers,
        std::size_t maxSendSize,
        int flags);

    class BsdClientTransport;

    // returns -2 for timeout, -1 for error, 0 for peer closure, otherwise size of packet read
    RCF_EXPORT int timedRecv(
        BsdClientTransport &clientTransport,
        I_PollingFunctor &pollingFunctor,
        int &err,
        int fd,
        const ByteBuffer &byteBuffer,
        std::size_t bytesRequested,
        int flags);

    RCF_EXPORT bool isFdConnected(int fd);

    std::pair<std::string, std::vector<std::string> > getLocalIps();

} // namespace RCF


#endif // ! INCLUDE_RCF_TIMEDBSDSOCKETS_HPP
