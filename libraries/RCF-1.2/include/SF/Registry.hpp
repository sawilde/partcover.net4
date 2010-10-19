
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_REGISTRY_HPP
#define INCLUDE_SF_REGISTRY_HPP

#include <map>
#include <string>
#include <typeinfo>

#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>

#include <RCF/Export.hpp>
#include <RCF/ThreadLibrary.hpp>
#include <RCF/util/InitDeinit.hpp>

#include <SF/Tools.hpp>

#if defined(_MSC_VER) && _MSC_VER == 1200
#include <SF/SerializePolymorphic.hpp>
#include <SF/SerializeAny.hpp>
#include <SF/Serializer.hpp>
#endif

namespace SF {

    typedef util::ReadWriteMutex    ReadWriteMutex;
    typedef util::ReadLock          ReadLock;
    typedef util::WriteLock         WriteLock;

    class I_SerializerPolymorphic;
    class I_SerializerAny;

    class RCF_EXPORT Registry : boost::noncopyable
    {
    private:
        Registry();
        
        typedef std::string                         Rtti;
        typedef std::map<std::string, Rtti>         TypenameToRtti;
        typedef std::map<Rtti, std::string>         RttiToTypename;

        typedef std::map<
            std::pair<Rtti, Rtti>, 
            boost::shared_ptr<I_SerializerPolymorphic> > 
                                                    RttiToSerializerPolymorphic;

        typedef std::map<
            Rtti, 
            boost::shared_ptr<I_SerializerAny> > 
                                                    RttiToSerializerAny;
        
        TypenameToRtti                              mTypenameToRtti;
        RttiToTypename                              mRttiToTypename;
        RttiToSerializerPolymorphic                 mRttiToSerializerPolymorphic;
        RttiToSerializerAny                         mRttiToSerializerAny;
        ReadWriteMutex                              mReadWriteMutex;

    public:

        friend void initRegistrySingleton();

        static Registry &getSingleton();

        static Registry *getSingletonPtr();

#if !defined(_MSC_VER) || _MSC_VER > 1200

        template<typename Type>
        void registerAny(Type *);

        template<typename Type>
        void registerType(Type *, const std::string &typeName);

        template<typename Base, typename Derived>
        void registerBaseAndDerived(Base *, Derived *);

        template<typename Base>
        I_SerializerPolymorphic &getSerializerPolymorphic(
            Base *, 
            const std::string &derivedTypeName);

        template<typename T>
        std::string getTypeName()
        {
            return getTypeName( (T *) 0);
        }

        template<typename Type>
        void registerAny()
        {
            registerAny( (Type *) 0);
        }

        template<typename Type>
        void registerType(const std::string &typeName)
        {
            registerType( (Type *) 0);
        }

        template<typename Base, typename Derived>
        void registerBaseAndDerived()
        {
            registerBaseAndDerived( (Base *) 0, (Derived *) 0);
        }

        template<typename Base>
        I_SerializerPolymorphic &getSerializerPolymorphic(
            const std::string &derivedTypeName)
        {
            return getSerializerPolymorphic( (Base *) 0, derivedTypeName);
        }

#else

        template<typename Type>
        void registerAny(Type *)
        {
            WriteLock lock(mReadWriteMutex); 
            RCF_UNUSED_VARIABLE(lock);
            Rtti typeRtti = typeid(Type).name();
            RCF_ASSERT(mRttiToTypename.find(typeRtti) != mRttiToTypename.end());
            mRttiToSerializerAny[typeRtti].reset(new SerializerAny<Type>());
        }

        template<typename Type>
        void registerType(Type *, const std::string &typeName)
        {
            WriteLock lock(mReadWriteMutex); 
            RCF_UNUSED_VARIABLE(lock);
            Rtti typeRtti = typeid(Type).name();
            mRttiToTypename[typeRtti] = typeName;
            mTypenameToRtti[typeName] = typeRtti;

            // Instantiate Type's serialize function so we can register the 
            // base/derived info.
            // NB: release build optimizers had better not eliminate this.
            //if (0)
            //{
            //    serialize( *((Archive *) NULL), *((Type *) NULL), 0);
            //}
        }

        template<typename Base, typename Derived>
        void registerBaseAndDerived(Base *, Derived *)
        {
            WriteLock lock(mReadWriteMutex); 
            RCF_UNUSED_VARIABLE(lock);
            Rtti baseRtti = typeid(Base).name();
            Rtti derivedRtti = typeid(Derived).name();
            std::pair<Rtti, Rtti> baseDerivedRtti(baseRtti, derivedRtti);

            mRttiToSerializerPolymorphic[baseDerivedRtti].reset(
                new SerializerPolymorphic<Base,Derived>());
        }

        template<typename Base>
        I_SerializerPolymorphic &getSerializerPolymorphic(
            Base *, 
            const std::string &derivedTypeName)
        {
            ReadLock lock(mReadWriteMutex); 
            RCF_UNUSED_VARIABLE(lock);
            Rtti baseRtti = typeid(Base).name();
            Rtti derivedRtti = mTypenameToRtti[derivedTypeName];
            std::pair<Rtti, Rtti> baseDerivedRtti(baseRtti, derivedRtti);
            if (
                mRttiToSerializerPolymorphic.find(baseDerivedRtti) 
                == mRttiToSerializerPolymorphic.end())
            {
                RCF_THROW(
                    RCF::Exception(RCF::_SfError_BaseDerivedRegistration(baseRtti, derivedRtti)))
                    (derivedTypeName)(baseRtti)(derivedRtti);
            }
            return *mRttiToSerializerPolymorphic[ baseDerivedRtti ];
        }

#endif

        I_SerializerAny &getAnySerializer(const std::string &which);

        bool isTypeRegistered(const std::string &typeName);

        bool isTypeRegistered(const std::type_info &ti);

        template<typename T>
        std::string getTypeName(T *)
        {
            return getTypeName(typeid(T));
        }

        std::string getTypeName(const std::type_info &ti);

        void clear();

    };

    template<typename Type>
    inline void registerAny(Type *)
    {
        Registry::getSingleton().registerAny( (Type *) 0);
    }

    template<typename Type>
    inline void registerType(Type *, const std::string &typeName)
    {
        Registry::getSingleton().registerType( (Type *) 0, typeName);
    }

    template<typename Base, typename Derived>
    inline void registerBaseAndDerived( Base *, Derived *)
    {
        Registry::getSingleton().registerBaseAndDerived( 
            (Base *) 0, 
            (Derived *) 0);
    }

} // namespace SF

#if !defined(_MSC_VER) || _MSC_VER > 1200

#include <SF/SerializePolymorphic.hpp>
#include <SF/SerializeAny.hpp>
#include <SF/Serializer.hpp>

namespace SF {

    template<typename Type>
    inline void registerType(const std::string &typeName)
    {
        Registry::getSingleton().registerType( (Type *) 0, typeName);
    }

    template<typename Base, typename Derived>
    inline void registerBaseAndDerived()
    {
        Registry::getSingleton().registerBaseAndDerived( 
            (Base *) 0, 
            (Derived *) 0);
    }

    template<typename Type>
    void Registry::registerAny(Type *)
    {
        WriteLock lock(mReadWriteMutex); 
        RCF_UNUSED_VARIABLE(lock);
        Rtti typeRtti = typeid(Type).name();
        RCF_ASSERT(mRttiToTypename.find(typeRtti) != mRttiToTypename.end());
        mRttiToSerializerAny[typeRtti].reset(new SerializerAny<Type>());
    }

    template<typename Type>
    void Registry::registerType(Type *, const std::string &typeName)
    {
        WriteLock lock(mReadWriteMutex); 
        RCF_UNUSED_VARIABLE(lock);
        Rtti typeRtti = typeid(Type).name();
        mRttiToTypename[typeRtti] = typeName;
        mTypenameToRtti[typeName] = typeRtti;

        // Instantiate Type's serialize function so we can register the 
        // base/derived info.
        // NB: release build optimizers had better not eliminate this.
        //if (0)
        //{
        //    serialize( *((Archive *) NULL), *((Type *) NULL), 0);
        //}
    }

    template<typename Base, typename Derived>
    void Registry::registerBaseAndDerived(Base *, Derived *)
    {
        WriteLock lock(mReadWriteMutex); 
        RCF_UNUSED_VARIABLE(lock);
        Rtti baseRtti = typeid(Base).name();
        Rtti derivedRtti = typeid(Derived).name();
        std::pair<Rtti, Rtti> baseDerivedRtti(baseRtti, derivedRtti);

        mRttiToSerializerPolymorphic[baseDerivedRtti].reset(
            new SerializerPolymorphic<Base,Derived>);
    }

    template<typename Base>
    I_SerializerPolymorphic &Registry::getSerializerPolymorphic(
        Base *, 
        const std::string &derivedTypeName)
    {
        ReadLock lock(mReadWriteMutex); 
        RCF_UNUSED_VARIABLE(lock);
        Rtti baseRtti = typeid(Base).name();
        Rtti derivedRtti = mTypenameToRtti[derivedTypeName];
        std::pair<Rtti, Rtti> baseDerivedRtti(baseRtti, derivedRtti);
        if (
            mRttiToSerializerPolymorphic.find(baseDerivedRtti) 
            == mRttiToSerializerPolymorphic.end())
        {
            RCF_THROW(
                RCF::Exception(RCF::_SfError_BaseDerivedRegistration(baseRtti, derivedRtti)))
                (derivedTypeName)(baseRtti)(derivedRtti);
        }
        return *mRttiToSerializerPolymorphic[ baseDerivedRtti ];
    }

} // namespace SF

#endif

#endif // ! INCLUDE_SF_REGISTRY_HPP
