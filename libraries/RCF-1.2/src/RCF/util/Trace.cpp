
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <RCF/util/Trace.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

#include <RCF/util/Platform/OS/OutputDebugString.hpp>

namespace util {

    std::string TraceTargetNull::getName() 
    { 
        return "null"; 
    }
    
    bool TraceTargetNull::isNull() 
    { 
        return true; 
    }

    void TraceTargetNull::trace(const std::string &msg) 
    { 
        RCF_UNUSED_VARIABLE(msg); 
    }

    std::string TraceTargetOds::getName()
    {
        return "ODS";
    }

    bool TraceTargetOds::isNull()
    {
        return false;
    }
    
    void TraceTargetOds::trace(const std::string &msg)
    {
        Lock lock(m); RCF_UNUSED_VARIABLE(lock);
        Platform::OS::OutputDebugString( msg.c_str() );
    }

    std::string TraceTargetStdout::getName()
    {
        return "stdout";
    }

    bool TraceTargetStdout::isNull()
    {
        return false;
    }

    void TraceTargetStdout::trace(const std::string &msg)
    {
        Lock lock(m); RCF_UNUSED_VARIABLE(lock);
        std::cout << msg;
        std::cout.flush();
    }
    
    std::string TraceTargetStderr::getName()
    {
        return "stderr";
    }

    bool TraceTargetStderr::isNull()
    {
        return false;
    }

    void TraceTargetStderr::trace(const std::string &msg)
    {
        Lock lock(m); RCF_UNUSED_VARIABLE(lock);
        std::cerr << msg;
        std::cerr.flush();
    }

    TraceTargetFile::TraceTargetFile( const std::string &filename ) :
        fout( new std::ofstream(filename.c_str())),
        filename(filename)
    {}

    bool TraceTargetFile::isNull()
    {
        return false;
    }

    std::string TraceTargetFile::getName()
    {
        return filename;
    }

    void TraceTargetFile::trace(const std::string &msg)
    {
        Lock lock(m); RCF_UNUSED_VARIABLE(lock);
        *fout << msg;
        fout->flush();
    }

    TraceChannel::TraceChannel(const std::string &traceChannelName) :
        traceChannelName(traceChannelName)
    {
        setTraceTarget("null");
        TraceManager::getSingleton().registerTraceChannel(*this);
    }

    void TraceChannel::setTraceTarget(const std::string &traceTargetName)
    {
        traceTarget = TraceManager::getSingleton().getTraceTarget(traceTargetName);
    }

    void TraceChannel::trace(const std::string &msg)
    {
        traceTarget->trace(msg);
    }

    TraceTarget & TraceChannel::getTraceTarget() 
    { 
        return *traceTarget; 
    }

    std::string TraceChannel::getName() const 
    { 
        return traceChannelName; 
    }

    TraceManager *& TraceManager::singletonPtr()
    {
        static TraceManager *pTraceManager = NULL;
        return pTraceManager;
    }

    TraceManager & TraceManager::getSingleton()
    {
        return *getSingletonPtr();
    }

    TraceManager * TraceManager::getSingletonPtr()
    {
        if (singletonPtr() == NULL)
        {
            delete singletonPtr();
            singletonPtr() = new TraceManager;
        }
        return singletonPtr();
    }

    void TraceManager::deleteSingletonPtr()
    {
        delete singletonPtr();
        singletonPtr() = NULL;
    }

    TraceManager::TraceManager()
    {
        makeTraceTarget("", boost::shared_ptr<TraceTarget>( new TraceTargetOds ) );
        makeTraceTarget("ODS", boost::shared_ptr<TraceTarget>( new TraceTargetOds ) );
        makeTraceTarget("stdout", boost::shared_ptr<TraceTarget>( new TraceTargetStdout ) );
        makeTraceTarget("stderr", boost::shared_ptr<TraceTarget>( new TraceTargetStderr ) );
        makeTraceTarget("null", boost::shared_ptr<TraceTarget>( new TraceTargetNull ) );
    }

    bool TraceManager::existsTraceChannel(const std::string &name)
    {
        return traceChannelMap.find(name) != traceChannelMap.end();
    }

    TraceChannel & TraceManager::getTraceChannel(const std::string &name)
    {
        TraceChannelMap::const_iterator it = traceChannelMap.find(name);
        if (it == traceChannelMap.end())
        {
            // TODO: put in a .cpp file!
            //UTIL_THROW(std::runtime_error("trace channel does not exist"))(name);
            throw std::runtime_error("trace channel does not exist");
        }
        TraceChannel *pTraceChannel = (*it).second;
        return *pTraceChannel;
    }

    std::vector<std::string> TraceManager::getTraceChannelNames()
    {
        std::vector<std::string> names;
        for (
            TraceChannelMap::const_iterator it = traceChannelMap.begin();
            it != traceChannelMap.end();
            it++)
            {
                names.push_back( (*it).second->getName() );
            }
        return names;
    }

    boost::shared_ptr<TraceTarget> TraceManager::getTraceTarget(
        const std::string &traceTargetName)
    {
        if (traceTargetMap.find(traceTargetName) == traceTargetMap.end())
        {
            throw std::runtime_error("trace channel does not exist");
        }
        return traceTargetMap[traceTargetName];
    }

    void TraceManager::makeTraceTarget(const std::string &traceTargetName)
    {
        makeTraceTarget(
            traceTargetName,
            boost::shared_ptr<TraceTarget>( new TraceTargetFile(traceTargetName) ));
    }

    void TraceManager::makeTraceTarget(const std::string &traceTargetName, boost::shared_ptr<TraceTarget> traceTarget)
    {
        if (traceTargetMap.find(traceTargetName) == traceTargetMap.end())
        {
            traceTargetMap[traceTargetName] = traceTarget;
        }
    }

    TraceChannel & TraceManager::getUtilTraceChannel()
    {
        if (NULL == utilTraceChannel.get())
        {
            utilTraceChannel.reset( new TraceChannel("util") );
        }
        return *utilTraceChannel;
    }

    void TraceManager::registerTraceChannel(TraceChannel &traceChannel)
    {
        std::string name = traceChannel.getName();
        traceChannelMap[name] = &traceChannel;
    }
    
} // namespace util
