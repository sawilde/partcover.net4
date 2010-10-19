
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_UTIL_PLATFORM_THREADS_RCFBOOSTTHREADS_HPP
#define INCLUDE_UTIL_PLATFORM_THREADS_RCFBOOSTTHREADS_HPP

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4275 ) // warning C4275: non dll-interface class 'std::runtime_error' used as base for dll-interface class 'boost::thread_resource_error'
#pragma warning( disable : 4251 ) // warning C4251: 'boost::thread_group::m_threads' : class 'std::list<_Ty>' needs to have dll-interface to be used by clients of class 'boost::thread_group'
#endif

#include <boost/noncopyable.hpp>

#include <RCF/RcfBoostThreads/RcfBoostThreads.hpp>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <RCF/util/DefaultInit.hpp>
#include <RCF/util/UnusedVariable.hpp>

namespace Platform {

    namespace Threads {

        typedef RCF::RcfBoostThreads::boost::thread thread;
        typedef RCF::RcfBoostThreads::boost::thread_group thread_group;

        template<typename T>
        struct thread_specific_ptr
        {
            typedef RCF::RcfBoostThreads::boost::thread_specific_ptr<T> Val;
        };

        typedef RCF::RcfBoostThreads::boost::mutex                    mutex;
        typedef RCF::RcfBoostThreads::boost::try_mutex                try_mutex;

        class condition : boost::noncopyable
        {
        public:
            template<typename T> void  wait(const T &t) { mCondition.wait(t); }
            template<typename T> bool timed_wait(const T &t, int timeoutMs)
            {
                RCF::RcfBoostThreads::boost::xtime xt;
                RCF::RcfBoostThreads::boost::xtime_get(&xt, RCF::RcfBoostThreads::boost::TIME_UTC);
                xt.sec += timeoutMs / 1000;
                xt.nsec += 1000*(timeoutMs - 1000*(timeoutMs / 1000));
                return mCondition.timed_wait(t, xt);
            }
            void notify_one() { mCondition.notify_one(); }
            void notify_all() { mCondition.notify_all(); }
        private:
            RCF::RcfBoostThreads::boost::condition mCondition;
        };

        // simple drop in replacement for boost::read_write_mutex, until there's a final version in boost.threads

        enum read_write_scheduling_policy
        {
            writer_priority
            , reader_priority
            , alternating_many_reads
            , alternating_single_read
        };

        class read_write_mutex;

        namespace detail {

            class scoped_read_lock : boost::noncopyable
            {
            public:
                scoped_read_lock(read_write_mutex &rwm);
                ~scoped_read_lock();
                void lock();
                void unlock();

            private:
                typedef mutex::scoped_lock                scoped_lock;
                read_write_mutex &                      rwm;
                bool                                    locked;
            };

            class scoped_write_lock : boost::noncopyable
            {
            public:
                scoped_write_lock(read_write_mutex &rwm);
                ~scoped_write_lock();
                void lock();
                void unlock();

            private:
                typedef mutex::scoped_lock                scoped_lock;
                read_write_mutex &                      rwm;
                scoped_lock                             readLock;
                scoped_lock                             writeLock;
                bool                                    locked;
            };

        } // namespace detail

        class read_write_mutex : boost::noncopyable
        {
        public:
            read_write_mutex(read_write_scheduling_policy rwsp) :
              readerCount(RCF_DEFAULT_INIT)
              {
                  RCF_UNUSED_VARIABLE(rwsp);
              }

        private:

            typedef mutex::scoped_lock    scoped_lock;

            void waitOnReadUnlock(scoped_lock &lock)
            {
                readUnlockEvent.wait(lock);
            }

            void notifyReadUnlock()
            {
                readUnlockEvent.notify_all();
            }

            mutex                                    readMutex;
            mutex                                    writeMutex;
            condition                               readUnlockEvent;
            int                                     readerCount;

        public:

            typedef detail::scoped_read_lock        scoped_read_lock;
            typedef detail::scoped_write_lock       scoped_write_lock;

            friend class detail::scoped_read_lock;
            friend class detail::scoped_write_lock;

        };

        namespace detail {

            inline scoped_read_lock::scoped_read_lock(read_write_mutex &rwm) :
                rwm(rwm),
                locked(RCF_DEFAULT_INIT)
            {
                lock();
            }

            inline scoped_read_lock::~scoped_read_lock()
            {
                unlock();
            }

            inline void scoped_read_lock::lock()
            {
                if (!locked)
                {
                    {
                        scoped_lock lock( rwm.readMutex );
                        ++rwm.readerCount;
                    }
                    locked = true;
                }
            }

            inline void scoped_read_lock::unlock()
            {
                if (locked)
                {
                    {
                        scoped_lock lock( rwm.readMutex );
                        --rwm.readerCount;
                        rwm.notifyReadUnlock();
                    }
                    locked = false;
                }
            }

            inline scoped_write_lock::scoped_write_lock(read_write_mutex &rwm) :
                rwm(rwm),
                readLock(rwm.readMutex, false),
                writeLock(rwm.writeMutex, false),
                locked(RCF_DEFAULT_INIT)
            {
                lock();
            }

            inline scoped_write_lock::~scoped_write_lock()
            {
                unlock();
            }

            inline void scoped_write_lock::lock()
            {
                if (!locked)
                {
                    readLock.lock();
                    while (rwm.readerCount > 0)
                    {
                        rwm.waitOnReadUnlock(readLock);
                    }
                    writeLock.lock();
                    locked = true;
                }
            }

            inline void scoped_write_lock::unlock()
            {
                if (locked)
                {
                    writeLock.unlock();
                    readLock.unlock();
                    locked = false;
                }
            }

        } // namespace detail


    } // namespace Threads

} // namespace Platform

#endif // ! INCLUDE_UTIL_PLATFORM_THREADS_RCFBOOSTTHREADS_HPP
