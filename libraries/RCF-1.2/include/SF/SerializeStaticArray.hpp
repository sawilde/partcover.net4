
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_SERIALIZESTATICARRAY_HPP
#define INCLUDE_SF_SERIALIZESTATICARRAY_HPP

#include <SF/Archive.hpp>

namespace SF {

    // serialize C-style static arrays

    template<typename T, unsigned int N>
    inline void serializeFundamentalStaticArray(
        Archive &           ar, 
        T                   (*pt)[N])
    {
        serializeFundamental(ar, (*pt)[0], N);
    }

    template<typename T, unsigned int N>
    inline void serializeNonfundamentalStaticArray(
        Archive &           ar, 
        T                   (*pt)[N])
    {
        for (unsigned int i=0; i<N; i++)
            ar & (*pt)[i];
    }


    template<bool IsFundamental>
    class SerializeStaticArray;

    template<>
    class SerializeStaticArray<true>
    {
    public:
        template<typename T, unsigned int N>
        void operator()(Archive &ar, T (*pt)[N])
        {
            serializeFundamentalStaticArray(ar, pt);
        }
    };

    template<>
    class SerializeStaticArray<false>
    {
    public:
        template<typename T, unsigned int N>
        void operator()(Archive &ar, T (*pt)[N])
        {
            serializeNonfundamentalStaticArray(ar, pt);
        }
    };


    template<typename T, unsigned int N>
    inline void preserialize(SF::Archive &ar, T (*pt)[N], const unsigned int)
    {
        static const bool IsFundamental = RCF::IsFundamental<T>::value;
        SerializeStaticArray<IsFundamental>()(ar, pt);
    }

} // namespace SF

#endif // ! INCLUDE_SF_SERIALIZESTATICARRAY_HPP
