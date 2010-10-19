
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TEST_DISABLBEGPFDIALOG_HPP
#define INCLUDE_RCF_TEST_DISABLBEGPFDIALOG_HPP

#include <boost/config.hpp>

#include <RCF/util/CommandLine.hpp>

#if defined(_MSC_VER) && _MSC_VER >= 1310
#include <RCF/test/MiniDump.hpp>
#endif

class GpfDialogCommandLineOption : public util::CommandLineOption<bool>
{
public:

    GpfDialogCommandLineOption() :
        util::CommandLineOption<bool>("gpfdialog", false, "Enable or disable display of GPF dialog.")
    {}

#ifdef BOOST_WINDOWS

    void on_notify_end()
    {

        // Install our custom unhandled exception filter, which will write out a
        // minidump if something nasty happens.

        bool enableGpfDialog = get();
        if (!enableGpfDialog)
        {
            DWORD dwFlags = SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS;
            DWORD dwOldFlags = SetErrorMode(dwFlags);
            SetErrorMode(dwOldFlags | dwFlags);
        }

#if defined(_MSC_VER) && _MSC_VER >= 1310
        setMiniDumpExceptionFilter(enableGpfDialog);
#endif

    }

#else

    void on_notify_end()
    {}

#endif

};

GpfDialogCommandLineOption *gdClo = NULL;

UTIL_ON_INIT_NAMED( gdClo = new GpfDialogCommandLineOption(); , PtoCloInitialize )

#endif // ! INCLUDE_RCF_TEST_DISABLBEGPFDIALOG_HPP
