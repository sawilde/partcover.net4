
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_TEST_MINIDUMP_HPP
#define INCLUDE_RCF_TEST_MINIDUMP_HPP

#include <DbgHelp.h>

void createMiniDump(EXCEPTION_POINTERS * pep = NULL);

void setMiniDumpExceptionFilter(bool enableGpfDialog);

#endif
