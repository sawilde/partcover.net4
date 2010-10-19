
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

// Copyright (C) 2001-2003
// William E. Kempf
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  William E. Kempf makes no representations
// about the suitability of this software for any purpose.
// It is provided "as is" without express or implied warranty.

#ifndef RCF_BOOST_THREAD_EXCEPTIONS_PDM070801_H
#define RCF_BOOST_THREAD_EXCEPTIONS_PDM070801_H

namespace boost {

class RCF_BOOST_THREAD_DECL thread_exception : public std::exception
{
protected:
    thread_exception();
    thread_exception(int sys_err_code);

public:
    ~thread_exception() throw();

    int native_error() const;

    const char* message() const;

private:
    int m_sys_err;
};

class RCF_BOOST_THREAD_DECL lock_error : public thread_exception
{
public:
    lock_error();
    lock_error(int sys_err_code);
    ~lock_error() throw();

    virtual const char* what() const throw();
};

class RCF_BOOST_THREAD_DECL thread_resource_error : public thread_exception
{
public:
    thread_resource_error();
    thread_resource_error(int sys_err_code);
    ~thread_resource_error() throw();

    virtual const char* what() const throw();
};

class RCF_BOOST_THREAD_DECL unsupported_thread_option : public thread_exception
{
public:
    unsupported_thread_option();
    unsupported_thread_option(int sys_err_code);
    ~unsupported_thread_option() throw();

    virtual const char* what() const throw();
};

class RCF_BOOST_THREAD_DECL invalid_thread_argument : public thread_exception
{
public:
    invalid_thread_argument();
    invalid_thread_argument(int sys_err_code);
    ~invalid_thread_argument() throw();

    virtual const char* what() const throw();
};

class RCF_BOOST_THREAD_DECL thread_permission_error : public thread_exception
{
public:
    thread_permission_error();
    thread_permission_error(int sys_err_code);
    ~thread_permission_error() throw();

    virtual const char* what() const throw();
};

} // namespace boost

#endif // RCF_BOOST_THREAD_CONFIG_PDM070801_H
