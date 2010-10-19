
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/FileTransferService.hpp>

#include <boost/function.hpp>

#include <cstdio>
#include <iomanip>
#include <boost/limits.hpp>

#include <sys/stat.h>

#include <RCF/ObjectFactoryService.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/util/Platform/OS/Sleep.hpp>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>

namespace boost {
    namespace filesystem {

        void serialize(SF::Archive &ar, path &p)
        {
            if (ar.isWrite())
            {
                std::string s = p.string();
                ar & s;
            }
            else
            {
                std::string s;
                ar & s;
                p = path(s);
            }
        }

    }
}

namespace RCF {

    namespace fs = boost::filesystem;

    FileUploadInfo::~FileUploadInfo()
    {
        mFileStream.close();

        // Best effort only.
        try
        {
            fs::remove_all(mUploadPath);
        }
        catch(const std::exception & e)
        {
            std::string error = e.what();
        }
        catch(...)
        {

        }
    }

    FileDownloadInfo::~FileDownloadInfo()
    {
    }

    FileTransferService::FileTransferService(
        const fs::path & tempFileDirectory) :
            mTempFileDirectory(tempFileDirectory)
    {
    }

    boost::int32_t FileTransferService::findUploadedFile(
        boost::filesystem::path &    manifestBase,
        FileManifest &                manifest)
    {
        FileUploadInfoPtr uploadInfoPtr = getCurrentRcfSession().mUploadInfoPtr;

        if (!uploadInfoPtr)
        {
            return RcfError_NoUpload;
        }
        //else if (uploadInfoPtr->mFailed)
        //{
        //   return RcfError_UploadFailed;
        //}
        else if (uploadInfoPtr->mCompleted)
        {
            //path = uploadInfoPtr->mUploadPath;
            manifestBase = uploadInfoPtr->mUploadPath;
            manifest = uploadInfoPtr->mManifest;
            return RcfError_Ok;
        }
        else
        {
            return RcfError_UploadInProgress;
        }
    }

    boost::int32_t FileTransferService::prepareDownload(
        const fs::path & whichFile)
    {
        fs::path manifestBase;
        if (fs::is_directory( whichFile ))
        {
            manifestBase = (whichFile / "..").normalize();
        }
        else
        {
            manifestBase = whichFile.branch_path();
        }

        FileManifest manifest(whichFile);

        return prepareDownload(manifestBase.string(), manifest);
    }

    boost::int32_t FileTransferService::prepareDownload(
        const fs::path & manifestBase,
        const FileManifest & manifest)
    {
        FileDownloadInfoPtr downloadInfoPtr( new FileDownloadInfo(mDownloadThrottle) );
        downloadInfoPtr->mDownloadPath = manifestBase;
        downloadInfoPtr->mManifest = manifest;
        getCurrentRcfSession().mDownloadInfoPtr = downloadInfoPtr;
        return RcfError_Ok;
    }

    void FileTransferService::removeUpload()
    {
        getCurrentRcfSession().mUploadInfoPtr.reset();
    }

    void FileTransferService::removeDownload()
    {
        getCurrentRcfSession().mDownloadInfoPtr.reset();
    }

    boost::uint32_t FileTransferService::beginUpload(
        const FileManifest & manifest,
        const std::vector<FileChunk> & chunks,
        FileChunk & chunk)
    {
        namespace fs = boost::filesystem;

        FileUploadInfoPtr uploadInfoPtr( new FileUploadInfo() );

        uploadInfoPtr->mManifest = manifest;

        uploadInfoPtr->mTimeStampMs = Platform::OS::getCurrentTimeMs();

        // Check the access callback (if any).
        if (    !mUploadCallback.empty()
            &&  !mUploadCallback(*uploadInfoPtr))
        {
            return RcfError_AccessDenied;
        }

        // Create a temp folder to upload to.
        std::size_t tries = 0;
        while (tries++ < 10)
        {
            std::ostringstream os;
            os 
                << "RCF-Upload-" 
                << std::setw(10)
                << std::setfill('0')
                << rand();

            fs::path tempFolder = fs::path(mTempFileDirectory) / os.str();

            if (!fs::exists(tempFolder))
            {
                bool ok = fs::create_directory(tempFolder);
                if (ok)
                {
                    uploadInfoPtr->mUploadPath = tempFolder.string();
                    break;
                }
            }
        }

        getCurrentRcfSession().mUploadInfoPtr = uploadInfoPtr;

        chunk.mFileIndex = 0;
        chunk.mOffset = 0;
        chunk.mData = ByteBuffer();

        return RcfError_Ok;
    }

    boost::uint32_t FileTransferService::uploadChunks(
        const std::vector<FileChunk> & chunks)
    {
        namespace fs = boost::filesystem;

        // Find the upload.
        FileUploadInfoPtr uploadInfoPtr = getCurrentRcfSession().mUploadInfoPtr;
        
        if (!uploadInfoPtr)
        {
            return RcfError_NoUpload;
        }

        FileUploadInfo & uploadInfo = * uploadInfoPtr;

        if (uploadInfo.mCompleted)
        {
            // TODO: better error message.
            return RcfError_NoUpload;
        }

        for (std::size_t i=0; i<chunks.size(); ++i)
        {
            const FileChunk & chunk = chunks[i];

            if (chunk.mFileIndex != uploadInfo.mCurrentFile)
            {
                // TODO: better error message.
                return RcfError_FileOffset;
            }

            std::pair< boost::filesystem::path, FileInfo> file =
                uploadInfo.mManifest.mFiles[uploadInfo.mCurrentFile];

            if (uploadInfo.mCurrentPos == 0)
            {
                fs::path filePath =  file.first;
                filePath = uploadInfo.mUploadPath / filePath;

                if ( !fs::exists( filePath.branch_path() ) )
                {
                    fs::create_directories( filePath.branch_path() );
                }

                uploadInfo.mFileStream.open( 
                    filePath.string().c_str(), 
                    std::ios::binary | std::ios::trunc );

                if (!uploadInfo.mFileStream)
                {
                    return RcfError_FileWrite;
                }
            }

            std::ofstream & fout = uploadInfoPtr->mFileStream;

            // Check stream state.
            if (!fout)
            {
                return RcfError_FileWrite;
            }

            // Check the offset position.
            uploadInfo.mCurrentPos = fout.tellp();
            if (chunk.mOffset != uploadInfo.mCurrentPos)
            {
                return RcfError_FileOffset;
            }

            // Check the chunk size.
            boost::uint64_t fileSize = file.second.mFileSize;
            boost::uint64_t remainingFileSize = fileSize - uploadInfo.mCurrentPos;
            if (chunk.mData.getLength() > remainingFileSize)
            {
                return RcfError_UploadFileSize;
            }

            // Write the chunk to the file.
            fout.write( chunk.mData.getPtr(), chunk.mData.getLength() );

            // Check write operation.
            if (fout.fail())
            {
                fout.close();
                //uploadInfoPtr->mFailed = true;
                return RcfError_FileWrite;
            }

            uploadInfoPtr->mTimeStampMs = Platform::OS::getCurrentTimeMs();

            // Check if last chunk.
            uploadInfo.mCurrentPos = fout.tellp();
            if (uploadInfo.mCurrentPos == fileSize)
            {
                fout.close();
                //uploadInfoPtr->mCompleted = true;
                ++uploadInfo.mCurrentFile;
                uploadInfo.mCurrentPos = 0;

                if (uploadInfo.mCurrentFile == uploadInfo.mManifest.mFiles.size())
                {
                    uploadInfo.mCompleted = true;
                }
            }
        }

        return RcfError_Ok;
    }


    boost::uint32_t FileTransferService::beginDownload(
        FileManifest & manifest,
        const FileTransferRequest & request,
        std::vector<FileChunk> & chunks)
    {
        FileDownloadInfoPtr downloadInfoPtr = getCurrentRcfSession().mDownloadInfoPtr;

        if (!downloadInfoPtr)
        {
            return RcfError_NoDownload;
        }

        manifest = downloadInfoPtr->mManifest;

        // TODO: optional first chunks.
        RCF_UNUSED_VARIABLE(request);
        chunks.clear();

        return RcfError_Ok;
    }

    boost::uint32_t FileTransferService::downloadChunks(
        const FileTransferRequest & request,
        std::vector<FileChunk> & chunks,
        boost::uint32_t & adviseWaitMs)
    {
        // Find the download.
        FileDownloadInfoPtr downloadInfoPtr = getCurrentRcfSession().mDownloadInfoPtr;

        if (!downloadInfoPtr)
        {
            return RcfError_NoDownload;
        }

        adviseWaitMs = 0;


        // Check offset.
        if (    request.mFile != downloadInfoPtr->mCurrentFile 
            ||  request.mPos != downloadInfoPtr->mCurrentPos)
        {
            return RcfError_FileOffset;
        }

        chunks.clear();

        boost::uint32_t chunkSize = request.mChunkSize;

        // Trim the chunk size, according to throttle settings.
        Throttle::Reservation & reservation = downloadInfoPtr->mReservation;
        reservation.allocate();
        boost::uint32_t bps = reservation.getBps();
        if (bps != boost::uint32_t(-1))
        {
            const boost::uint32_t mTransferWindowS = 5;
            const boost::uint32_t mThrottleRateBytesPerS = bps;

            if (mThrottleRateBytesPerS)
            {
                if (downloadInfoPtr->mTransferWindowTimer.elapsed(mTransferWindowS*1000))
                {
                    downloadInfoPtr->mTransferWindowTimer.restart();
                    downloadInfoPtr->mTransferWindowBytes = 0;
                }

                boost::uint32_t bytesTotal = mThrottleRateBytesPerS * mTransferWindowS;
                boost::uint32_t bytesRemaining =  bytesTotal - downloadInfoPtr->mTransferWindowBytes;

                if (bytesRemaining < chunkSize)
                {
                    boost::uint32_t startTimeMs = downloadInfoPtr->mTransferWindowTimer.getStartTimeMs();
                    boost::uint32_t nowMs = getCurrentTimeMs();
                    if (nowMs < startTimeMs + 1000*mTransferWindowS)
                    {
                        adviseWaitMs = startTimeMs + 1000*mTransferWindowS - nowMs;
                    }
                }

                chunkSize = RCF_MIN(chunkSize, bytesRemaining);
            }
        }

        boost::uint32_t totalBytesRead = 0;

        while (
                totalBytesRead < chunkSize 
            &&  downloadInfoPtr->mCurrentFile != downloadInfoPtr->mManifest.mFiles.size())
        {

            if (downloadInfoPtr->mCurrentPos == 0)
            {
                fs::path manifestBase = downloadInfoPtr->mDownloadPath;
                fs::path filePath = downloadInfoPtr->mManifest.mFiles[downloadInfoPtr->mCurrentFile].first;
                fs::path totalPath = manifestBase / filePath;

                downloadInfoPtr->mFileStream.clear();
                downloadInfoPtr->mFileStream.open(
                    totalPath.string().c_str(),
                    std::ios::in | std::ios::binary);

                // TODO: error handling.
                RCF_VERIFY(downloadInfoPtr->mFileStream, Exception(_RcfError_FileOpen()))(totalPath.string());
            }

            boost::uint64_t fileSize = downloadInfoPtr->mManifest.mFiles[downloadInfoPtr->mCurrentFile].second.mFileSize;
            boost::uint64_t bytesRemainingInFile =  fileSize - downloadInfoPtr->mCurrentPos;
            boost::uint64_t bytesRemainingInChunk = chunkSize - totalBytesRead;

            ByteBuffer byteBuffer( static_cast<std::size_t>( 
                RCF_MIN(bytesRemainingInChunk, bytesRemainingInFile) ));

            std::ifstream & fin = downloadInfoPtr->mFileStream;

            boost::uint32_t bytesRead = fin.read( 
                byteBuffer.getPtr(), 
                byteBuffer.getLength() ).gcount();

            if (fin.fail() && !fin.eof())
            {
                // TODO: error handling
                return RcfError_FileRead;
            }

            FileChunk fileChunk;
            fileChunk.mFileIndex = downloadInfoPtr->mCurrentFile;
            fileChunk.mOffset = downloadInfoPtr->mCurrentPos;
            fileChunk.mData = ByteBuffer(byteBuffer, 0, bytesRead);
            chunks.push_back(fileChunk);

            totalBytesRead += bytesRead;

            downloadInfoPtr->mCurrentPos += bytesRead;
            if (downloadInfoPtr->mCurrentPos == downloadInfoPtr->mManifest.mFiles[downloadInfoPtr->mCurrentFile].second.mFileSize)
            {
                fin.close();
                downloadInfoPtr->mCurrentPos = 0;
                ++downloadInfoPtr->mCurrentFile;
            }
        }

        downloadInfoPtr->mTransferWindowBytes += totalBytesRead;

        if (downloadInfoPtr->mCurrentFile == downloadInfoPtr->mManifest.mFiles.size())
        {
            downloadInfoPtr->mReservation.release();
        }

        return RcfError_Ok;
    }

    void FileTransferService::setDownloadThrottle(boost::uint32_t users, boost::uint32_t kbps)
    {
        mDownloadThrottle.set(users, kbps);
    }

    void FileTransferService::setUploadCallback(UploadAccessCallback uploadCallback)
    {
        mUploadCallback = uploadCallback;
    }

    void FileTransferService::onServiceAdded(RcfServer &server)
    {
        server.bind( (I_FileTransferService *) NULL, *this);
    }

    void FileTransferService::onServiceRemoved(RcfServer &server)
    {
        server.unbind( (I_FileTransferService *) NULL);
    }

    void FileManifest::serialize(SF::Archive & ar, unsigned int) 
    {
        ar & mFiles;
    }

    void FileInfo::serialize(SF::Archive & ar, unsigned int) 
    {
        ar & mFileSize & mFileCrc;
    }

    void FileChunk::serialize(SF::Archive & ar, unsigned int)
    {
        ar & mFileIndex & mOffset & mData;
    }

    void FileTransferRequest::serialize(SF::Archive & ar, unsigned int)
    {
        ar & mFile & mPos & mChunkSize;
    }

    FileManifest::FileManifest(boost::filesystem::path pathToFiles) 
    {
        RCF_ASSERT( fs::exists(pathToFiles) );

        if (fs::is_directory(pathToFiles))
        {
            for ( 
                fs::recursive_directory_iterator iter(pathToFiles); 
                iter != fs::recursive_directory_iterator(); 
                ++iter )
            { 

                fs::path fullPath = *iter;
                if ( !fs::is_directory(fullPath) )
                {
                    FileInfo fileInfo;
                    fileInfo.mFileSize = fs::file_size(fullPath);
                    fileInfo.mFileCrc = 0;

                    // TODO: this is a bit of a kludge.
                    std::string base = (pathToFiles / "..").normalize().string();
                    std::string full = fullPath.string();
                    RCF_ASSERT(full.substr(0, base.length()) == base);
                    std::string relative = full.substr(base.length()+1);
                    fs::path relativePath(relative);

                    mFiles.push_back( std::make_pair(relativePath, fileInfo) );
                }
            }
        }
        else
        {
            FileInfo fileInfo;
            fileInfo.mFileSize = fs::file_size(pathToFiles);
            mFiles.push_back( std::make_pair(pathToFiles.leaf(), fileInfo) );
        }
    }

    FileUploadInfo::FileUploadInfo() : 
        mCompleted(false),
        mTimeStampMs(0),
        mCurrentFile(0),
        mCurrentPos(0)
    {}

    FileDownloadInfo::FileDownloadInfo(Throttle & downloadThrottle) :
        mCurrentFile(0),
        mCurrentPos(0),
        mTransferWindowBytes(0),
        mReservation(downloadThrottle)
    {}

    Throttle::Reservation::Reservation(Throttle & throttle) : 
        mThrottle(throttle), 
        mBps(0),
        mReserved(false)
    {
    }

    Throttle::Reservation::~Reservation()
    {
        release();
    }

    void Throttle::Reservation::allocate()
    {
        // See if we can upgrade our existing reservation, or create one if necessary.
        if (mBps != boost::uint32_t(-1))
        {
            Lock lock(mThrottle.mMutex);
            if (mThrottle.mSettings.empty())
            {
                release(false);
                
                mBps = boost::uint32_t(-1);
                mReserved = false;
            }
            else
            {
                for (
                    Settings::reverse_iterator iter = mThrottle.mSettings.rbegin(); 
                    iter != mThrottle.mSettings.rend(); 
                    ++iter)
                {
                    boost::uint32_t bps = iter->first;
                    boost::uint32_t maxAllowed = iter->second.first;
                    boost::uint32_t & booked = iter->second.second;

                    if (bps <= mBps)
                    {
                        break;
                    }
                    
                    if (booked < maxAllowed)
                    {
                        release(false);

                        ++booked;
                        mBps = bps;
                        mReserved = true;

                        break;
                    }
                }
            }
        }
    }

    void Throttle::Reservation::release(bool sync)
    {
        if (mReserved)
        {
            if (sync)
            {
                Lock lock(mThrottle.mMutex);
                releaseImpl();
            }
            else
            {
                releaseImpl();
            }
        }
    }

    void Throttle::Reservation::releaseImpl()
    {
        RCF_ASSERT(mReserved);
        Settings::iterator iter = mThrottle.mSettings.find(mBps);
        RCF_ASSERT(iter != mThrottle.mSettings.end());
        boost::uint32_t & booked = iter->second.second;
        --booked;
        RCF_ASSERT(booked != boost::uint32_t(-1));
        mBps = 0;
        mReserved = false;
    }

    boost::uint32_t Throttle::Reservation::getBps()
    {
        return mBps;
    }

    void Throttle::set(boost::uint32_t users, boost::uint32_t bps)
    {
        Lock lock(mMutex);
        mSettings[bps].first = users;
    }

} // namespace RCF
