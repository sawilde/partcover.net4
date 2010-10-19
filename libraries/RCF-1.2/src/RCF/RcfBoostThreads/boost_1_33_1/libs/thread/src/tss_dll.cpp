
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

#if defined(BOOST_HAS_WINTHREADS) && defined(BOOST_THREAD_BUILD_DLL)

    #if defined(__BORLANDC__)
        extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID /*lpReserved*/)
    #else
        extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID /*lpReserved*/)
    #endif
    {
        switch(dwReason)
        {
            case DLL_PROCESS_ATTACH:
            {
                RBT_on_process_enter();
                RBT_on_thread_enter();
                break;
            }

            case DLL_THREAD_ATTACH:
            {
                RBT_on_thread_enter();
                break;
            }

            case DLL_THREAD_DETACH:
            {
                RBT_on_thread_exit();
                break;
            }

            case DLL_PROCESS_DETACH:
            {
                RBT_on_thread_exit();
                RBT_on_process_exit();
                break;
            }
        }

        return TRUE;
    }

    extern "C" void tss_cleanup_implemented(void)
    {
        /*
        This function's sole purpose is to cause a link error in cases where
        automatic tss cleanup is not implemented by Boost.Threads as a
        reminder that user code is responsible for calling the necessary
        functions at the appropriate times (and for implementing an a
        tss_cleanup_implemented() function to eliminate the linker's
        missing symbol error).

        If Boost.Threads later implements automatic tss cleanup in cases
        where it currently doesn't (which is the plan), the duplicate
        symbol error will warn the user that their custom solution is no
        longer needed and can be removed.
        */
    }

#endif //defined(BOOST_HAS_WINTHREADS) && defined(BOOST_THREAD_BUILD_DLL)
