
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_USINGOPENSSL_HPP
#define INCLUDE_RCF_USINGOPENSSL_HPP

#include <string>

#include <RCF/Export.hpp>

namespace RCF {

    // Calling ERR_print_errors_fp() crashes the whole app for some reason,
    // so call my_ERR_print_errors_fp() instead,
    // it does the exact same thing.

    RCF_EXPORT int              my_print_fp(
                                    const char *str, 
                                    size_t len, 
                                    void *fp);

    RCF_EXPORT void             my_ERR_print_errors_fp(FILE *fp);
    RCF_EXPORT std::string      getOpenSslErrors();
    RCF_EXPORT void             initOpenSsl();

} // namespace RCF

#endif // ! INCLUDE_RCF_USINGOPENSSL_HPP
