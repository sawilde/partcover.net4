
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

// (C) Copyright Michael Glassford 2004.
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if defined(BOOST_HAS_WINTHREADS)

    //#define WIN32_LEAN_AND_MEAN

    namespace
    {
        typedef ::std::list<thread_exit_handler> thread_exit_handlers;

        boost::once_flag once_init_threadmon_mutex = BOOST_ONCE_INIT;
        boost::mutex* threadmon_mutex;
        void init_threadmon_mutex(void)
        {
            threadmon_mutex = new boost::mutex;
            if (!threadmon_mutex)
                throw boost::thread_resource_error();
        }

        const DWORD invalid_tls_key = TLS_OUT_OF_INDEXES;
        DWORD tls_key = invalid_tls_key;

        unsigned long attached_thread_count = 0;
    }

    /*
    Calls to DllMain() and tls_callback() are serialized by the OS;
    however, calls to at_thread_exit are not, so it must be protected
    by a mutex. Since we already need a mutex for at_thread_exit(),
    and since there is no guarantee that on_process_enter(),
    on_process_exit(), on_thread_enter(), and on_thread_exit()
    will be called only from DllMain() or tls_callback(), it makes
    sense to protect those, too.
    */

    extern "C" RCF_BOOST_THREAD_DECL int RBT_at_thread_exit(
        thread_exit_handler exit_handler
        )
    {
        boost::call_once(init_threadmon_mutex, once_init_threadmon_mutex);
        boost::mutex::scoped_lock lock(*threadmon_mutex);

        //Allocate a tls slot if necessary.

        if (tls_key == invalid_tls_key)
            tls_key = TlsAlloc();

        if (tls_key == invalid_tls_key)
            return -1;

        //Get the exit handlers list for the current thread from tls.

        thread_exit_handlers* exit_handlers =
            static_cast<thread_exit_handlers*>(TlsGetValue(tls_key));

        if (!exit_handlers)
        {
            //No exit handlers list was created yet.

            try
            {
                //Attempt to create a new exit handlers list.

                exit_handlers = new thread_exit_handlers;
                if (!exit_handlers)
                    return -1;

                //Attempt to store the list pointer in tls.

                if (TlsSetValue(tls_key, exit_handlers))
                    ++attached_thread_count;
                else
                {
                    delete exit_handlers;
                    return -1;
                }
            }
            catch (...)
            {
                return -1;
            }
        }

        //Like the C runtime library atexit() function,
        //functions should be called in the reverse of
        //the order they are added, so push them on the
        //front of the list.

        try
        {
            exit_handlers->push_front(exit_handler);
        }
        catch (...)
        {
            return -1;
        }

        //Like the atexit() function, a result of zero
        //indicates success.

        return 0;
    }

    extern "C" RCF_BOOST_THREAD_DECL void RBT_on_process_enter(void)
    {
        boost::call_once(init_threadmon_mutex, once_init_threadmon_mutex);
        boost::mutex::scoped_lock lock(*threadmon_mutex);

        BOOST_ASSERT(attached_thread_count == 0);
    }

    extern "C" RCF_BOOST_THREAD_DECL void RBT_on_process_exit(void)
    {
        boost::call_once(init_threadmon_mutex, once_init_threadmon_mutex);
        boost::mutex::scoped_lock lock(*threadmon_mutex);

#ifndef __MINGW32__
        BOOST_ASSERT(attached_thread_count == 0);
#endif

        //Free the tls slot if one was allocated.

        if (tls_key != invalid_tls_key)
        {
            TlsFree(tls_key);
            tls_key = invalid_tls_key;
        }
    }

    extern "C" RCF_BOOST_THREAD_DECL void RBT_on_thread_enter(void)
    {
        //boost::call_once(init_threadmon_mutex, once_init_threadmon_mutex);
        //boost::mutex::scoped_lock lock(*threadmon_mutex);
    }

    extern "C" RCF_BOOST_THREAD_DECL void RBT_on_thread_exit(void)
    {
        boost::call_once(init_threadmon_mutex, once_init_threadmon_mutex);
        boost::mutex::scoped_lock lock(*threadmon_mutex);

        //Get the exit handlers list for the current thread from tls.

        if (tls_key == invalid_tls_key)
            return;

        thread_exit_handlers* exit_handlers =
            static_cast<thread_exit_handlers*>(TlsGetValue(tls_key));

        //If a handlers list was found, use it.

        if (exit_handlers && TlsSetValue(tls_key, 0))
        {
            BOOST_ASSERT(attached_thread_count > 0);
            --attached_thread_count;

            lock.unlock();

            //Call each handler and remove it from the list

            while (!exit_handlers->empty())
            {
                if (thread_exit_handler exit_handler = *exit_handlers->begin())
                    (*exit_handler)();
                exit_handlers->pop_front();
            }

            delete exit_handlers;
            exit_handlers = 0;
        }
    }

#endif //defined(BOOST_HAS_WINTHREADS)
