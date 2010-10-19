
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <SF/Serializer.hpp>

#include <SF/Node.hpp>

namespace SF {

    void SerializerBase::invokeRead(Archive &ar)
    {
        Node node( "", "", 0, false );
        Node * pNode = NULL;

        if (ar.isFlagSet(Archive::NODE_ALREADY_READ))
        {
            LocalStorage & localStorage = ar.getIstream()->getLocalStorage();
            pNode = reinterpret_cast<Node *>(localStorage.getNode());
        }
        else
        {
            if ( ! ar.getIstream()->begin(node) )
                return;
            pNode = &node;
        }

        // Detect polymorphism, either through pointers or through references
        if (!ar.isFlagSet(Archive::POLYMORPHIC))
        {
            if (    ar.isFlagSet(Archive::POINTER) 
                ||  (!ar.isFlagSet(Archive::PARENT) && isDerived()))
            {
                if (pNode->type.length() > 0)
                {
                    ar.setFlag(Archive::POLYMORPHIC, true );
                    std::string derivedTypeName = pNode->type.cpp_str();
                    getSerializerPolymorphic(derivedTypeName);
                    ar.getIstream()->getLocalStorage().setNode(pNode);
                    ar.setFlag(Archive::NODE_ALREADY_READ);
                    invokeSerializerPolymorphic(ar);
                    return;
                }
            }
        }

        // May now assume that the object is not polymorphic
        UInt32 nid = pNode->id;
        bool bId = pNode->id ? true : false;
        bool bNode = pNode->ref ? false : true;
        bool bPointer = ar.isFlagSet(Archive::POINTER);
        ar.clearState();

        if (bId && bNode && bPointer)
        {
            newObject(ar);
            addToInputContext(ar.getIstream(), nid);
            serializeContents(ar);
        }
        else if ( !bId && bNode && bPointer )
        {
            newObject(ar);
            serializeContents(ar);
        }
        else if (bId && !bNode && bPointer)
        {
            queryInputContext(ar.getIstream(), nid);
            setFromId();
        }
        else if (bId && bNode && !bPointer)
        {
            addToInputContext(ar.getIstream(), nid);
            serializeContents(ar);
        }
        else if (!bId && bNode && !bPointer )
        {
            serializeContents(ar);
        }
        else if (!bId && !bNode && bPointer)
        {
            setToNull();
        }
        else if (!bId && !bNode && !bPointer)
        {
            RCF_THROW(RCF::Exception(RCF::_RcfError_DeserializationNullPointer()));
        }
        else if (bId && !bNode && !bPointer)
        {
            RCF_THROW(RCF::Exception(RCF::_SfError_RefMismatch()));
        }

        ar.getIstream()->end();
    }

    void SerializerBase::invokeWrite(Archive &ar)
    {
        Node in("", "", 0, false);

        // Detect polymorphism
        if (!ar.isFlagSet(Archive::PARENT) && isDerived())
        {
            ar.setFlag(Archive::POLYMORPHIC);
            getSerializerPolymorphic(getDerivedTypeName());
            invokeSerializerPolymorphic(ar);
            return;
        }

        // May now assume non-polymorphic object

        if (ar.isFlagSet(Archive::POLYMORPHIC))
        {
            in.type.assign(getTypeName());
        }

        bool bPointer = ar.isFlagSet(Archive::POINTER);
        bool bNonAtomic = isNonAtomic();

        if (isNull())
        {
            in.id = 0;
            in.ref = 1;
            ar.getOstream()->begin(in);
            ar.clearState();
        }
        else if (bPointer || bNonAtomic)
        {
            if (queryOutputContext(ar.getOstream(), in.id ))
            {
                if (bPointer)
                    in.ref = 1;
                ar.getOstream()->begin(in);
                ar.clearState();
                if (!bPointer)
                    serializeContents(ar);
            }
            else
            {
                addToOutputContext(ar.getOstream(), in.id);
                ar.getOstream()->begin(in);
                ar.clearState();
                serializeContents(ar);
            }
        }
        else
        {
            ar.getOstream()->begin(in);
            ar.clearState();
            serializeContents(ar);
        }

        ar.getOstream()->end();
    }

    SerializerBase::SerializerBase()
    {}

    SerializerBase::~SerializerBase()
    {}

    void SerializerBase::invoke(Archive &ar)
    {
        if (ar.isFlagSet(Archive::NO_BEGIN_END))
        {
            ar.clearFlag(Archive::NO_BEGIN_END);
            serializeContents(ar);
        }
        else
        {
            RCF_ASSERT( ar.isRead() ||ar.isWrite() );
            if (ar.isRead())
            {
                invokeRead(ar);
            }
            else //if (ar.isWrite())
            {
                invokeWrite(ar);
            }
        }
    }

} // namespace SF
