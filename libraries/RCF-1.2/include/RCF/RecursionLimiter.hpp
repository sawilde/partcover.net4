
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_RECURSIONLIMITER_HPP
#define INCLUDE_RCF_RECURSIONLIMITER_HPP

#include <boost/type_traits/is_same.hpp>
#include <boost/mpl/bool.hpp>

#include <RCF/ByteBuffer.hpp>
#include <RCF/Tools.hpp>

namespace RCF {

    template<typename T1,typename T2>
    class RecursionState
    {
    public:
        RecursionState() :
            mRecursing(RCF_DEFAULT_INIT),
            mT1(RCF_DEFAULT_INIT),
            mT2(RCF_DEFAULT_INIT)
        {}

        void assign(const T1 &t1)
        {
            mT1 = t1;

            clearArg(t1);
        }

        void assign(const T1 &t1, const T2 &t2)
        {
            mT1 = t1;
            mT2 = t2;

            clearArg(t1);
            clearArg(t2);
        }

        void clear()
        {
            mRecursing = false;
            mT1 = T1();
            mT2 = T2();
        }

        bool    mRecursing;
        T1      mT1;
        T2      mT2;

    private:
        void clearArg_(const ByteBuffer &byteBuffer, boost::mpl::true_*)
        {
            const_cast<ByteBuffer &>(byteBuffer).clear();
        }

        template<typename T>
        void clearArg_(const T &, boost::mpl::false_*)
        {}

        template<typename T>
        void clearArg(const T &t)
        {
            typedef typename boost::is_same<T, ByteBuffer>::type type;
            clearArg_(t, (type*) 0);
        }
    };

    // Utility for avoiding recursive function calls on the stack, by unwinding
    // the relevant part of the stack and then reissuing the requested function
    // call.

    template<typename StateT, typename X, typename Pfn>
    void applyRecursionLimiter(
        StateT &        state, 
        Pfn             pfn,
        X &             x)
    {
        //state.assign(t1);
        if (state.mRecursing)
        {
            state.mRecursing = false;
        }
        else
        {
            // gcc295 seems to need the namespace qualifier for make_obj_guard anyway
            using namespace boost::multi_index::detail;

            scope_guard guard = boost::multi_index::detail::make_obj_guard(
                state,
                &StateT::clear);

            RCF_UNUSED_VARIABLE(guard);

            while (!state.mRecursing)
            {
                state.mRecursing = true;
                ((&x)->*pfn)();
            }
        }
    }


    template<typename StateT, typename X, typename Pfn, typename T1>
    void applyRecursionLimiter(
        StateT &        state, 
        Pfn             pfn,
        X &             x, 
        const T1 &      t1)
    {
        state.assign(t1);
        if (state.mRecursing)
        {
            state.mRecursing = false;
        }
        else
        {
            // gcc295 seems to need the namespace qualifier for make_obj_guard anyway
            using namespace boost::multi_index::detail;

            scope_guard guard = boost::multi_index::detail::make_obj_guard(
                state,
                &StateT::clear);

            RCF_UNUSED_VARIABLE(guard);

            while (!state.mRecursing)
            {
                state.mRecursing = true;
                ((&x)->*pfn)(state.mT1);
            }
        }
    }

    template<typename StateT, typename X, typename Pfn, typename T1, typename T2>
    void applyRecursionLimiter(
        StateT &        state, 
        Pfn             pfn,
        X &             x, 
        const T1 &      t1, 
        const T2 &      t2)
    {
        state.assign(t1, t2);
        if (state.mRecursing)
        {
            state.mRecursing = false;
        }
        else
        {
            // gcc295 seems to need the namespace qualifier for make_obj_guard anyway
            using namespace boost::multi_index::detail;

            scope_guard guard = boost::multi_index::detail::make_obj_guard(
                state,
                &StateT::clear);

            RCF_UNUSED_VARIABLE(guard);

            while (!state.mRecursing)
            {
                state.mRecursing = true;
                ((&x)->*pfn)(state.mT1, state.mT2);
            }
        }
    }

} // namespace RCF

#endif // ! INCLUDE_RCF_RECURSIONLIMITER_HPP
