
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_TRACE_HPP
#define INCLUDE_UTIL_TRACE_HPP

#include <exception>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/current_function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "InitDeinit.hpp"
#include "ThreadLibrary.hpp"
#include "UnusedVariable.hpp"
#include "VariableArgMacro.hpp"

#include <RCF/Export.hpp>

//*************************************************
// Trace utility

namespace util {

    // TraceTarget

    class TraceTarget : boost::noncopyable
    {
    public:
        virtual ~TraceTarget() {}
        virtual std::string getName() = 0;
        virtual bool isNull() = 0;
        virtual void trace(const std::string &msg) = 0;
    };

    typedef boost::shared_ptr<TraceTarget> TraceTargetPtr;
   
    class RCF_EXPORT TraceTargetNull : public TraceTarget
    {
    public:
        std::string getName();
        bool isNull();
        void trace(const std::string &msg);
    };

    class RCF_EXPORT TraceTargetOds : public TraceTarget
    {
    public:
        std::string getName();
        bool isNull();       
        void trace(const std::string &msg);
    private:
        Mutex m;
    };

    class RCF_EXPORT TraceTargetStdout : public TraceTarget
    {
    public:
        std::string getName();
        bool isNull();
        void trace(const std::string &msg);
    private:
        Mutex m;
    };

    class RCF_EXPORT TraceTargetStderr : public TraceTarget
    {
    public:
        std::string getName();
        bool isNull();
        void trace(const std::string &msg);
    private:
        Mutex m;
    };

    class RCF_EXPORT TraceTargetFile : public TraceTarget
    {
    public:
        TraceTargetFile(const std::string &filename);
        bool isNull();
        std::string getName();
        void trace(const std::string &msg);
    private:
        Mutex m;
        std::auto_ptr<std::ofstream> fout;
        std::string filename;
    };

    // TraceChannel

    class RCF_EXPORT TraceChannel
    {
    public:
        TraceChannel(const std::string &traceChannelName);

        void            trace(const std::string &msg);
        void            setTraceTarget(const std::string &traceTargetName);
        TraceTarget &   getTraceTarget();
        std::string     getName() const;

    private:
        const std::string               traceChannelName;
        boost::shared_ptr<TraceTarget>  traceTarget;
    };


    // TraceManager

    class RCF_EXPORT TraceManager : boost::noncopyable
    {
    private:
        TraceManager();

        static TraceManager *&      singletonPtr();

    public:

        static TraceManager &       getSingleton();

        static TraceManager *       getSingletonPtr();

        static void                 deleteSingletonPtr();

    public:

        bool                        existsTraceChannel(
                                        const std::string &name);

        TraceChannel &              getTraceChannel(
                                        const std::string &name);

        std::vector<std::string>    getTraceChannelNames();

        TraceTargetPtr              getTraceTarget(
                                        const std::string &traceTargetName);

        void                        makeTraceTarget(
                                        const std::string &traceTargetName);

        void                        makeTraceTarget(
                                        const std::string &traceTargetName, 
                                        TraceTargetPtr traceTarget);

        TraceChannel &              getUtilTraceChannel();

        friend class TraceChannel; // so the TraceChannel objects can register themselves

        void                        registerTraceChannel(
                                        TraceChannel &traceChannel);


        typedef std::map<std::string, TraceChannel *>   TraceChannelMap;
        TraceChannelMap                                 traceChannelMap;

        typedef std::map<std::string, TraceTargetPtr>   TraceTargetMap;
        TraceTargetMap                                  traceTargetMap;

        std::auto_ptr<TraceChannel>                     utilTraceChannel;

    };
   
    // TraceFunctor, for variable arg macro semantics

    class TraceFunctor : public VariableArgMacroFunctor
    {
    public:
        TraceFunctor &init_trace(TraceChannel *traceChannel)
        {
            this->traceChannel = traceChannel;
            return *this;
        }

        void deinit()
        {
            std::string msg = header.str() + args.str() + "\n";
            traceChannel->trace(msg);
        }

    private:
        TraceChannel *traceChannel;
    };

    #ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4355 )  // warning C4355: 'this' : used in base member initializer list
    #endif

    #if defined(__GNUC__) && (__GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4))
    #define UTIL_TRACE_GCC_33_HACK (const util::VariableArgMacro<util::TraceFunctor> &)
    #else
    #define UTIL_TRACE_GCC_33_HACK
    #endif

    DECLARE_VARIABLE_ARG_MACRO( UTIL_TRACE, TraceFunctor );
    #define UTIL_TRACE(msg, channel)                                                \
        if (!channel.getTraceTarget().isNull())                                     \
            UTIL_TRACE_GCC_33_HACK                                                  \
            util::VariableArgMacro<util::TraceFunctor>()                            \
                .init_trace(&channel)                                               \
                .init(                                                              \
                    "TRACE: ",                                                      \
                    msg,                                                            \
                    __FILE__,                                                       \
                    __LINE__,                                                       \
                    BOOST_CURRENT_FUNCTION )                                        \
                .cast( (util::VariableArgMacro<util::TraceFunctor> *) NULL )        \
                .UTIL_TRACE_A

    #define UTIL_TRACE_A(x)                         UTIL_TRACE_OP(x, B)
    #define UTIL_TRACE_B(x)                         UTIL_TRACE_OP(x, A)
    #define UTIL_TRACE_OP(x, next)                  UTIL_TRACE_A.notify_((x), #x).UTIL_TRACE_ ## next

    #ifdef _MSC_VER
    #pragma warning( pop )
    #endif

    #define UTIL_TRACE_DEFAULT(msg)                                                 \
        UTIL_TRACE(msg, ::TraceManager::getSingleton().getUtilTraceChannel())

    // auto initialization
    UTIL_ON_INIT_NAMED( TraceManager::getSingleton().getUtilTraceChannel(), TraceInitialize1 )
    UTIL_ON_DEINIT_NAMED( TraceManager::deleteSingletonPtr(), TraceDeinitialize1 )

}

#endif //! INCLUDE_UTIL_TRACE_HPP
