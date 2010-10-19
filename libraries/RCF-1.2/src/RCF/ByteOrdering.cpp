
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <boost/type_traits/is_same.hpp>

#include <RCF/ByteOrdering.hpp>
#include <RCF/Exception.hpp>
#include <RCF/Tools.hpp>
#include <RCF/util/Platform/Machine/ByteOrder.hpp>

namespace RCF {

    typedef Platform::Machine::LittleEndian         LittleEndian;
    typedef Platform::Machine::BigEndian            BigEndian;

    typedef BigEndian                               NetworkByteOrder;
    typedef Platform::Machine::ByteOrder            MachineByteOrder;

    void swapBytes(char *b1, char *b2)
    {
        //*b2 ^= *b1;
        //*b1 ^= *b2;
        //*b2 ^= *b1;

        char temp = *b1;
        *b1 = *b2;
        *b2 = temp;
    }

    void reverseByteOrder(void *buffer, int width, int count)
    {
        RCF_ASSERT(width > 0)(width);
        RCF_ASSERT(count > 0)(count);
        if (width == 1) return;

        BOOST_STATIC_ASSERT( sizeof(char) == 1 );   
        char *chBuffer = static_cast<char *>(buffer);
        for (int i=0; i<count; i++)
        {
            for (int j=0;j<width/2;j++)
            {
                swapBytes(
                    chBuffer + i*width + j,
                    chBuffer + i*width + width - j - 1 );
            }
        }

    }

    void machineToNetworkOrder(void *buffer, int width, int count)
    {
        if ( boost::is_same<MachineByteOrder, NetworkByteOrder>::value )
        {
            reverseByteOrder(buffer, width, count);
        }
    }

    void networkToMachineOrder(void *buffer, int width, int count)
    {
        if ( boost::is_same<MachineByteOrder, NetworkByteOrder>::value )
        {
            reverseByteOrder(buffer, width, count);
        }
    }

} // namespace RCF
