
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_SF_SERIALIZEPOLYMORPHIC_HPP
#define INCLUDE_SF_SERIALIZEPOLYMORPHIC_HPP

namespace SF {

    class Archive;

    //template<typename Base, typename Derived>
    //void registerBaseAndDerived(Base * = 0, Derived * = 0);

    class I_SerializerPolymorphic
    {
    public:
        virtual ~I_SerializerPolymorphic() {}
        virtual bool invoke(void **ppvb, Archive &ar) = 0;
    };

#if defined(_MSC_VER) && _MSC_VER <= 1200

    template<typename Base, typename Derived>
    class SerializerPolymorphic : public I_SerializerPolymorphic
    {
    public:
        static void instantiate() {}
        bool invoke(void **ppvb, Archive &ar);
    };

#else

    template<typename Base, typename Derived>
    void registerBaseAndDerived();

    template<typename Base, typename Derived>
    class SerializerPolymorphic : public I_SerializerPolymorphic
    {
    public:
        static SerializerPolymorphic &instantiate() { return instance; }
        SerializerPolymorphic() {}
        bool invoke(void **ppvb, Archive &ar);

        static SerializerPolymorphic instance;
        SerializerPolymorphic(int)
        {
            registerBaseAndDerived<Base, Derived>();
        }
    };

    // on gcc 3.2, this requires the SerializerPolymorphic ctor to be public
    template<class Base, class Derived>
    SerializerPolymorphic<Base, Derived> SerializerPolymorphic<Base, Derived>::instance(0);

#endif

}

#include <SF/Archive.hpp>
#include <SF/Serializer.hpp>

namespace SF {

    template<typename Base, typename Derived>
    bool SerializerPolymorphic<Base,Derived>::invoke(void **ppvb, Archive &ar)
    {
        if (ar.isWrite())
        {
            Base *pb = reinterpret_cast<Base *>(*ppvb);
            Derived *pd = static_cast<Derived *>(pb);
            ar & pd;
        }
        else if (ar.isRead())
        {
            if (ar.isFlagSet(Archive::POINTER))
            {
                Derived *pd = NULL;
                ar & pd;
                Base *pb = static_cast<Base *>(pd);
                *ppvb = pb;
            }
            else
            {
                Base *pb = reinterpret_cast<Base *>(*ppvb);
                Derived *pd = static_cast<Derived *>(pb);
                ar & *pd;
            }
        }
        return true;
    }

}

#endif // ! INCLUDE_SF_SERIALIZEPOLYMORPHIC_HPP
