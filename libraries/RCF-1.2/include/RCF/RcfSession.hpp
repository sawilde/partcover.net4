
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_RCFSESSION_HPP
#define INCLUDE_RCF_RCFSESSION_HPP

#include <vector>

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/MethodInvocation.hpp>
#include <RCF/SerializationProtocol.hpp>
#include <RCF/ServerTransport.hpp>
#include <RCF/StubEntry.hpp>

namespace RCF {

    class Filter;

    typedef boost::shared_ptr<Filter> FilterPtr;

    class RcfSession;

    typedef boost::shared_ptr<RcfSession> RcfSessionPtr;
    typedef boost::weak_ptr<RcfSession> RcfSessionWeakPtr;

    class I_Future;

    class I_Parameters;

    class UdpServerTransport;
    class UdpSessionState;

    class FileTransferService;
    class FileUploadInfo;
    class FileDownloadInfo;

    typedef boost::shared_ptr<FileUploadInfo>   FileUploadInfoPtr;
    typedef boost::shared_ptr<FileDownloadInfo> FileDownloadInfoPtr;

    typedef std::pair<boost::uint32_t, RcfSessionWeakPtr>   PingBackTimerEntry;

    template<
        typename R, 
        typename A1,
        typename A2,
        typename A3,
        typename A4,
        typename A5,
        typename A6,
        typename A7,
        typename A8>
    class AllocateServerParameters;

    template<
        typename R, 
        typename A1,
        typename A2,
        typename A3,
        typename A4,
        typename A5,
        typename A6,
        typename A7,
        typename A8>
    class ServerParameters;

    class RCF_EXPORT RcfSession : 
        public I_Session,
        public boost::enable_shared_from_this<RcfSession>
    {
    public:
        RcfSession(RcfServer &server);
        ~RcfSession();

        typedef boost::function1<void, RcfSession&> OnWriteCompletedCallback;
        typedef boost::function1<void, RcfSession&> OnWriteInitiatedCallback;
        typedef boost::function1<void, RcfSession&> OnDestroyCallback;

        //*******************************
        // callback tables - synchronized

        // may well be called on a different thread than the one that executed the remote call
        void addOnWriteCompletedCallback(
            const OnWriteCompletedCallback &        onWriteCompletedCallback);

        void extractOnWriteCompletedCallbacks(
            std::vector<OnWriteCompletedCallback> & onWriteCompletedCallbacks);

        void setOnDestroyCallback(
            OnDestroyCallback                       onDestroyCallback);

        //*******************************

        const RCF::I_RemoteAddress &
                        getRemoteAddress();

        RcfServer &     getRcfServer();

        void            disconnect();

        bool            hasDefaultServerStub();
        StubEntryPtr    getDefaultStubEntryPtr();
        void            setDefaultStubEntryPtr(const StubEntryPtr &stubEntryPtr);
        void            setCachedStubEntryPtr(const StubEntryPtr &stubEntryPtr);

        void            enableSfSerializationPointerTracking(bool enable);

        int             getRcfRuntimeVersion();
        void            setRcfRuntimeVersion(int version);

        int             getArchiveVersion();
        void            setArchiveVersion(int version);

        void            setUserData(boost::any userData);
        boost::any      getUserData();

        void            getMessageFilters(std::vector<FilterPtr> &filters);
        void            getTransportFilters(std::vector<FilterPtr> &filters);

        void            lockTransportFilters();
        void            unlockTransportFilters();
        bool            transportFiltersLocked();

        SerializationProtocolIn &   getSpIn();
        SerializationProtocolOut &  getSpOut();

        bool                        getFiltered();
        void                        setFiltered(bool filtered);

        std::vector<FilterPtr> &    getFilters();

        void            setCloseSessionAfterWrite(bool close);

        boost::uint32_t getPingBackIntervalMs();

        boost::uint32_t getPingTimestamp();

        boost::uint32_t getTouchTimestamp();

        void            touch();

        void            sendPingBack(PingBackTimerEntry & nextEntry);
        bool            getAutoSend();

        void            setWeakThisPtr();

        void            dropReceiveBuffer();

        Mutex                                   mStopCallInProgressMutex;
        bool                                    mStopCallInProgress;
        
#if defined(_MSC_VER) && _MSC_VER < 1310

        // vc6: Can't seem to get declare ServerParameters<> and 
        // AllocateServerParameters<> as friends.
    public:    

#else

    private:

        template<
            typename R, 
            typename A1,
            typename A2,
            typename A3,
            typename A4,
            typename A5,
            typename A6,
            typename A7,
            typename A8>
        friend class AllocateServerParameters;

        template<
            typename R, 
            typename A1,
            typename A2,
            typename A3,
            typename A4,
            typename A5,
            typename A6,
            typename A7,
            typename A8>
        friend class ServerParameters;

#endif

        RcfServer &                             mRcfServer;

        Mutex                                   mMutex;
        std::vector<OnWriteCompletedCallback>   mOnWriteCompletedCallbacks;
        std::vector<OnWriteInitiatedCallback>   mOnWriteInitiatedCallbacks;
        OnDestroyCallback                       mOnDestroyCallback;

        int                                     mRcfRuntimeVersion;
        int                                     mArchiveVersion;
        
        bool                                    mTransportFiltersLocked;

        SerializationProtocolIn                 mIn;
        SerializationProtocolOut                mOut;

        // message filters
        std::vector<FilterPtr>                  mFilters;
        bool                                    mFiltered;

        MethodInvocationRequest                 mRequest;

        bool                                    mCloseSessionAfterWrite;
        boost::uint32_t                         mPingTimestamp;
        boost::uint32_t                         mTouchTimestamp;
        ByteBuffer                              mPingBackByteBuffer;
        PingBackTimerEntry                      mPingBackTimerEntry;

        enum IoState
        {
            Idle,
            Reading,
            Writing,
            WritingPingBack
        };

        Mutex                                   mIoStateMutex;
        bool                                    mWritingPingBack;
        std::vector<ByteBuffer>                 mQueuedSendBuffers;

        void clearParameters();

        void onReadCompleted();
        void onWriteCompleted();

        void processRequest();

        void sendResponse();
        void sendResponseException(const std::exception &e);
        void sendResponseUncaughtException();

        void encodeRemoteException(
            SerializationProtocolOut & out, 
            const RemoteException & e);

        void sendSessionResponse(); 

        void registerForPingBacks();
        void unregisterForPingBacks();

        friend class RcfServer;
        friend class AmdImpl;

        I_Parameters *                          mpParameters;
        std::vector<char>                       mParametersVec;

        bool                                    mAutoSend;

        RcfSessionWeakPtr                       mWeakThisPtr;

    private:

        // UdpServerTransport needs to explicitly set mIoState to Reading,
        // since it doesn't use async I/O with callbacks to RcfServer.
        friend class UdpServerTransport;
        friend class UdpSessionState;

    private:

        friend class FileTransferService;
        friend class PingBackService;

        FileDownloadInfoPtr                     mDownloadInfoPtr;
        FileUploadInfoPtr                       mUploadInfoPtr;

    private:

        boost::any                              mUserData;
        StubEntryPtr                            mDefaultStubEntryPtr;
        StubEntryPtr                            mCachedStubEntryPtr;

    };       

} // namespace RCF

#endif // ! INCLUDE_RCF_RCFSESSION_HPP
