
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#include <SF/Registry.hpp>

#include <RCF/InitDeinit.hpp>

#include <SF/SerializeAny.hpp>
#include <SF/string.hpp>

namespace SF {

    // serialization for boost::any
    void serialize(SF::Archive &ar, boost::any &a)
    {
        if (ar.isWrite())
        {
            std::string which = 
                SF::Registry::getSingleton().getTypeName(a.type());

            ar & which;
            SF::Registry::getSingleton().getAnySerializer(which)
                .serialize(ar, a);
        }
        else
        {
            std::string which;
            ar & which;
            SF::Registry::getSingleton().getAnySerializer(which)
                .serialize(ar, a);
        }
    }

    void initRegistrySingleton();

    static Registry *pRegistry;

    Registry::Registry() :
        mReadWriteMutex(Platform::Threads::writer_priority)
    {}

    Registry &Registry::getSingleton()
    {
        if (!pRegistry)
        {
            initRegistrySingleton();
        }
        return *pRegistry;
    }

    Registry *Registry::getSingletonPtr()
    {
        return &getSingleton();
    }

    bool Registry::isTypeRegistered(const std::string &typeName)
    {
        ReadLock lock(mReadWriteMutex); 
        RCF_UNUSED_VARIABLE(lock);
        return mTypenameToRtti.find(typeName) != mTypenameToRtti.end();
    }

    bool Registry::isTypeRegistered(const std::type_info &ti)
    {
        ReadLock lock(mReadWriteMutex); 
        RCF_UNUSED_VARIABLE(lock);
        Rtti typeRtti = ti.name();
        return mRttiToTypename.find(typeRtti) != mRttiToTypename.end();
    }

    I_SerializerAny &Registry::getAnySerializer(const std::string &which)
    {
        ReadLock lock(mReadWriteMutex); 
        RCF_UNUSED_VARIABLE(lock);
        if (mTypenameToRtti.find(which) != mTypenameToRtti.end())
        {
            Rtti rtti = mTypenameToRtti[which];

            RCF_ASSERT(
                mRttiToSerializerAny.find(rtti) 
                != mRttiToSerializerAny.end());

            return *mRttiToSerializerAny[rtti];
        }
        RCF_THROW( RCF::Exception(RCF::_RcfError_AnySerializerNotFound(which), which));
    }

    std::string Registry::getTypeName(const std::type_info &ti)
    {
        ReadLock lock(mReadWriteMutex); 
        RCF_UNUSED_VARIABLE(lock);
        Rtti typeRtti = ti.name();
        if (mRttiToTypename.find(typeRtti) == mRttiToTypename.end())
        {
            return "";
        }
        else
        {
            return mRttiToTypename[typeRtti];
        }
    }

    void Registry::clear()
    {
        mTypenameToRtti.clear();
        mRttiToTypename.clear();
        mRttiToSerializerPolymorphic.clear();
        mRttiToSerializerAny.clear();
    }

    void initRegistrySingleton()
    {
        if (!pRegistry)
        {
            pRegistry = new Registry();
        }
    }

    void deinitRegistrySingleton()
    {
        delete pRegistry;
        pRegistry = NULL;
    }

    RCF_ON_INIT_DEINIT(
        initRegistrySingleton(),
        deinitRegistrySingleton())

}
