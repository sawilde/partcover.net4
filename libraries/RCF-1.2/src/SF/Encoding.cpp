
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <SF/Encoding.hpp>

#include <RCF/Exception.hpp>

namespace SF {

    void encodeBool(bool value, std::vector<char> &vec, std::size_t &pos)
    {
        RCF_ASSERT(pos <= vec.size());
        if (pos + 1 > vec.size())
        {
            vec.resize(vec.size()+1);
        }

        value ?
            vec[pos] = 1 :
            vec[pos] = 0;
        pos += 1;
    }

    void encodeInt(int value, std::vector<char> &vec, std::size_t &pos)
    {
        RCF_ASSERT(pos <= vec.size());
        if (pos + 5 > vec.size())
        {
            vec.resize(vec.size()+5);
        }

        if (0 <= value && value < 255)
        {
            RCF_ASSERT(pos+1<=vec.size());
            vec[pos] = static_cast<char>(value);
            pos += 1;
        }
        else
        {
            RCF_ASSERT(pos+1<=vec.size());
            vec[pos] = (unsigned char)(255);
            pos += 1;

            RCF_ASSERT(pos+4<=vec.size());
            BOOST_STATIC_ASSERT(sizeof(int) == 4);
            RCF::machineToNetworkOrder(&value, 4, 1);
            memcpy(&vec[pos], &value, 4);
            pos += 4;
        }
    }

    void encodeString(
        const std::string &value,
        std::vector<char> &vec,
        std::size_t &pos)
    {
        int len = static_cast<int>(value.length());
        SF::encodeInt(len, vec, pos);

        RCF_ASSERT(pos <= vec.size());
        if (pos + len > vec.size())
        {
            vec.resize(vec.size()+len);
        }
        memcpy(&vec[pos], value.c_str(), len);
        pos += len;
    }

    void encodeBool(bool value, const RCF::ByteBuffer &byteBuffer, std::size_t &pos)
    {
        RCF_ASSERT(pos+1 <= byteBuffer.getLength());

        value ?
            byteBuffer.getPtr()[pos] = 1 :
            byteBuffer.getPtr()[pos] = 0;
        pos += 1;
    }

    void encodeInt(int value, const RCF::ByteBuffer &byteBuffer, std::size_t &pos)
    {
        if (0 <= value && value < 255)
        {
            RCF_ASSERT(pos+1<=byteBuffer.getLength());
            byteBuffer.getPtr()[pos] = static_cast<char>(value);
            pos += 1;
        }
        else
        {
            RCF_ASSERT(pos+1<=byteBuffer.getLength());
            byteBuffer.getPtr()[pos] = (unsigned char)(255);
            pos += 1;

            RCF_ASSERT(pos+4<=byteBuffer.getLength());
            BOOST_STATIC_ASSERT(sizeof(int) == 4);
            RCF::machineToNetworkOrder(&value, 4, 1);
            memcpy(&byteBuffer.getPtr()[pos], &value, 4);
            pos += 4;
        }
    }

    void decodeBool(bool &value, const RCF::ByteBuffer &byteBuffer, std::size_t &pos)
    {
        RCF_VERIFY(
            pos+1 <= byteBuffer.getLength(),
            RCF::Exception(RCF::_RcfError_Decoding()));

        char ch = byteBuffer.getPtr()[pos];
       
        RCF_VERIFY(
            ch == 0 || ch == 1,
            RCF::Exception(RCF::_RcfError_Decoding()));

        pos += 1;
        value = ch ? true : false;
    }

    void decodeInt(int &value, const RCF::ByteBuffer &byteBuffer, std::size_t &pos)
    {
        RCF_VERIFY(
            pos+1 <= byteBuffer.getLength(),
            RCF::Exception(RCF::_RcfError_Decoding()));

        unsigned char ch = byteBuffer.getPtr()[pos];
        pos += 1;

        if (ch < 255)
        {
            value = ch;
        }
        else
        {
            RCF_VERIFY(
                pos+4 <= byteBuffer.getLength(),
                RCF::Exception(RCF::_RcfError_Decoding()));

            BOOST_STATIC_ASSERT(sizeof(int) == 4);
            memcpy(&value, byteBuffer.getPtr()+pos, 4);
            RCF::networkToMachineOrder(&value, 4, 1);
            pos += 4;
        }
    }

    void decodeString(
        std::string &value,
        const RCF::ByteBuffer &byteBuffer,
        std::size_t &pos)
    {
        int len = 0;
        decodeInt(len, byteBuffer, pos);

        RCF_VERIFY(
            pos+len <= byteBuffer.getLength(),
            RCF::Exception(RCF::_RcfError_Decoding()));

        value.assign(byteBuffer.getPtr()+pos, len);
        pos += len;
    }

} // namespace SF
