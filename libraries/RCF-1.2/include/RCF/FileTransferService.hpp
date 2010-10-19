
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_FILETRANSFERSERVICE_HPP
#define INCLUDE_RCF_FILETRANSFERSERVICE_HPP

#include <fstream>
#include <map>

#include <RCF/Idl.hpp>
#include <RCF/Service.hpp>
#include <RCF/StubEntry.hpp>
#include <RCF/Token.hpp>

#include <SF/vector.hpp>
#include <SF/map.hpp>

#include <boost/filesystem/path.hpp>

namespace RCF {

    //--------------------------------------------------------------------------
    // I_FileTransferService

    class RCF_EXPORT FileInfo
    {
    public:
        FileInfo() : mFileSize(0), mFileCrc(0) {}
        boost::uint64_t mFileSize;
        boost::uint32_t mFileCrc;

        void serialize(SF::Archive & ar, unsigned int);
    };

    class RCF_EXPORT FileManifest
    {
    public:
        typedef std::vector< std::pair< boost::filesystem::path, FileInfo> > Files;
        Files mFiles;

        FileManifest() {}

        FileManifest(boost::filesystem::path pathToFiles);

        void serialize(SF::Archive & ar, unsigned int);
    };

    class FileChunk
    {
    public:

        FileChunk() : mFileIndex(0), mOffset(0)
        {}

        boost::uint32_t mFileIndex;
        boost::uint64_t mOffset;
        ByteBuffer mData;

        void serialize(SF::Archive & ar, unsigned int);
    };

    class FileTransferRequest
    {
    public:
        FileTransferRequest() : mFile(0), mPos(0), mChunkSize(0)
        {}

        boost::uint32_t mFile;
        boost::uint64_t mPos;
        boost::uint32_t mChunkSize;

        void serialize(SF::Archive & ar, unsigned int);
    };


    RCF_BEGIN(I_FileTransferService, "I_FileTransferService")

        RCF_METHOD_R3(
            boost::uint32_t,                        // error code
                beginUpload,
                    const FileManifest &,           // upload manifest
                    const std::vector<FileChunk> &, // optional first chunks
                    FileChunk&)                     // where to start uploading
        
        RCF_METHOD_R1(
            boost::uint32_t,                        // error code
                uploadChunks, 
                    const std::vector<FileChunk> &) // file chunks to upload

        RCF_METHOD_R3(
            boost::uint32_t,                        // error code
                beginDownload,
                    FileManifest &,                 // download manifest
                    const FileTransferRequest &,    // transfer request
                    std::vector<FileChunk> &)       // optional first chunks

        RCF_METHOD_R3(
            boost::uint32_t,                        // error code
                downloadChunks,
                    const FileTransferRequest &,    // transfer request
                    std::vector<FileChunk> &,       // file chunks to download
                    boost::uint32_t &)              // advised wait for next call

    RCF_END(I_FileTransferService)

    class Throttle
    {
    public:
        void set(boost::uint32_t users, boost::uint32_t bps);

        class Reservation
        {
        public:
            Reservation(Throttle & throttle);
            ~Reservation();

            void allocate();
            void release(bool sync = true);
            boost::uint32_t getBps();

        private:

            void releaseImpl();

            Throttle & mThrottle;
            boost::uint32_t mBps;
            bool mReserved;
        };

    private:
        Mutex mMutex;

        // bps -> (max, booked)
        typedef 
        std::map<
            boost::uint32_t, 
            std::pair<boost::uint32_t, boost::uint32_t> > Settings;
        
        Settings mSettings;
    };

    class FileUploadInfo;
    class FileDownloadInfo;

    typedef boost::shared_ptr<FileUploadInfo>                   FileUploadInfoPtr;
    typedef boost::shared_ptr<FileDownloadInfo>                 FileDownloadInfoPtr;

    typedef boost::function1<bool, const FileUploadInfo &>      UploadAccessCallback;
    typedef boost::function1<bool, const FileDownloadInfo &>    DownloadAccessCallback;
    
    class FileUploadInfo : 
        public TokenMapped, 
        boost::noncopyable
    {
    public:
        FileUploadInfo();

        ~FileUploadInfo();

        FileManifest            mManifest;
        boost::filesystem::path mUploadPath;
        std::ofstream           mFileStream;
        bool                    mCompleted;
        boost::uint32_t         mTimeStampMs;
        boost::uint32_t         mCurrentFile;
        boost::uint64_t         mCurrentPos;
    };

    class Timer
    {
    public:
        Timer() 
        { 
            restart(); 
        }

        bool elapsed(boost::uint32_t timeMs) 
        { 
            boost::uint32_t nowMs = getCurrentTimeMs();
            return nowMs - mStartTimeMs > timeMs;
        }

        void restart()
        {
            mStartTimeMs = getCurrentTimeMs();
        }

        boost::uint32_t getStartTimeMs()
        {
            return mStartTimeMs;
        }

    private:
        boost::uint32_t mStartTimeMs;
    };

    class FileDownloadInfo : 
        public TokenMapped, 
        boost::noncopyable
    {
    public:

        FileDownloadInfo(Throttle & downloadThrottle);

        ~FileDownloadInfo();
        
        boost::filesystem::path mDownloadPath;
        FileManifest            mManifest;
        std::ifstream           mFileStream;
        boost::uint32_t         mCurrentFile;
        boost::uint64_t         mCurrentPos;

        Timer                   mTransferWindowTimer;
        boost::uint32_t         mTransferWindowBytes;

        Throttle::Reservation   mReservation;
    };

    typedef boost::shared_ptr<FileUploadInfo>   FileUploadInfoPtr;
    typedef boost::shared_ptr<FileDownloadInfo> FileDownloadInfoPtr;

    class RCF_EXPORT FileTransferService : public I_Service
    {
    public:
        FileTransferService(const boost::filesystem::path & tempFileDirectory = "");

        void setDownloadThrottle(boost::uint32_t users, boost::uint32_t bps);

        void setUploadCallback(UploadAccessCallback uploadCallback);

        //----------------------------------------------------------------------
        // Uploads

        boost::int32_t      findUploadedFile(
                                boost::filesystem::path &   manifestBase,
                                FileManifest &              manifest);

        void                removeUpload();


        //----------------------------------------------------------------------
        // Downloads

        boost::int32_t      prepareDownload(
                                const boost::filesystem::path & whichFile);

        boost::int32_t      prepareDownload(
                                const boost::filesystem::path & manifestBase,
                                const FileManifest & manifest);

        void                removeDownload();


        //----------------------------------------------------------------------
        // Remotely accessible.

        boost::uint32_t     beginUpload(
                                const FileManifest & manifest,
                                const std::vector<FileChunk> & chunks,
                                FileChunk & chunk);

        boost::uint32_t     uploadChunks(
                                const std::vector<FileChunk> & chunks);

        boost::uint32_t     beginDownload(
                                FileManifest & manifest,
                                const FileTransferRequest & request,
                                std::vector<FileChunk> & chunks);

        boost::uint32_t     downloadChunks(
                                const FileTransferRequest & request,
                                std::vector<FileChunk> & chunks,
                                boost::uint32_t & adviseWaitMs);

        //----------------------------------------------------------------------

    private:

        void                onServiceAdded(
                                RcfServer &         server);

        void                onServiceRemoved(
                                RcfServer &         server);

        boost::filesystem::path mTempFileDirectory;

        UploadAccessCallback    mUploadCallback;

        Throttle                mDownloadThrottle;
    };

    typedef boost::shared_ptr<FileTransferService> FileTransferServicePtr;

} // namespace RCF

#endif // ! INCLUDE_RCF_FILETRANSFERSERVICE_HPP
