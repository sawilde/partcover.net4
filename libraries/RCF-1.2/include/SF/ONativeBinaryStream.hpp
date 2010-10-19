
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_ONATIVEBINARYSTREAM_HPP
#define INCLUDE_SF_ONATIVEBINARYSTREAM_HPP

#include <SF/Stream.hpp>

namespace SF {

    class ONativeBinaryStream : public OStream
    {
    public:
        ONativeBinaryStream(std::ostream &os) : OStream(os)
        {}

        I_Encoding &getEncoding()
        {
            return mEncoding;
        }

    private:
        EncodingBinaryNative mEncoding;
    };


}

#endif // ! INCLUDE_SF_ONATIVEBINARYSTREAM_HPP
