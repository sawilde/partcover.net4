
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

// Copyright (C) 2002-2003
// David Moore, William E. Kempf
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  William E. Kempf makes no representations
// about the suitability of this software for any purpose.
// It is provided "as is" without express or implied warranty.

namespace boost {

barrier::barrier(unsigned int count)
    : m_threshold(count), m_count(count), m_generation(0)
{
    if (count == 0)
        throw std::invalid_argument("count cannot be zero.");
}

barrier::~barrier()
{
}

bool barrier::wait()
{
    mutex::scoped_lock lock(m_mutex);
    unsigned int gen = m_generation;

    if (--m_count == 0)
    {
        m_generation++;
        m_count = m_threshold;
        m_cond.notify_all();
        return true;
    }

    while (gen == m_generation)
        m_cond.wait(lock);
    return false;
}

} // namespace boost
