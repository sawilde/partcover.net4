
//******************************************************************************
// RCF - Remote Call Framework
// Copyright (c) 2005 - 2010, Jarl Lindrud. All rights reserved.
// Consult your license for conditions of use.
// Version: 1.2
// Contact: jarl.lindrud <at> gmail.com 
//******************************************************************************

#ifndef INCLUDE_RCF_HEAP_HPP
#define INCLUDE_RCF_HEAP_HPP

#include <algorithm>
#include <vector>

#include <boost/cstdint.hpp>

#include <RCF/ThreadLibrary.hpp>
#include <RCF/Tools.hpp>

#include <RCF/util/Platform/OS/GetCurrentTime.hpp>

namespace RCF {

    template<typename Element, typename Compare = std::less<Element> >
    class Heap
    {
    public:

        Heap() : mCompare()
        {
        }

        Heap(const Compare & compare) : mCompare(compare)
        {
        }

        void add(const Element & element)
        {
            mStore.push_back(element);
            std::push_heap(mStore.begin(), mStore.end(), mCompare);
        }

        void remove(const Element & element)
        {
            RCF::eraseRemove(mStore, element);
            std::make_heap(mStore.begin(), mStore.end(), mCompare);
        }

        Element & top()
        {
            return *mStore.begin();
        }

        const Element & top() const
        {
            return *mStore.begin();
        }

        void pop()
        {
            std::pop_heap(mStore.begin(), mStore.end(), mCompare);
            mStore.pop_back();
        }

        std::size_t size() const
        {
            return mStore.size();
        }

        bool empty() const
        {
            return mStore.empty();
        }

        void clear()
        {
            mStore.clear();
        }

        void setCompare(const Compare & compare, bool reorder = true)
        {
            mCompare = compare;

            if (reorder)
            {
                std::make_heap(mStore.begin(), mStore.end(), mCompare);
            }
        }

    private:

        Compare                 mCompare;
        std::vector<Element>    mStore;
    };

    class TimerCompare
    {
    public:

        TimerCompare() : 
            mBaseTimeMs(0)
        {}

        TimerCompare(boost::uint32_t baseTimeMs) : 
            mBaseTimeMs(baseTimeMs)
        {}

        // We want a greater-than comparison, so that the lowest
        // elements are at the top of the heap.
        template<typename T>
        bool operator()(
            const std::pair<boost::uint32_t, T> & lhs, 
            const std::pair<boost::uint32_t, T> & rhs)
        {
            boost::uint32_t lhsTimeMs = lhs.first - mBaseTimeMs;
            boost::uint32_t rhsTimeMs = rhs.first - mBaseTimeMs;

            return 
                    std::make_pair(lhsTimeMs, lhs.second)
                >   std::make_pair(rhsTimeMs, rhs.second);
        }

    private:

        boost::uint32_t mBaseTimeMs;
    };

    template<typename T>
    class TimerHeap
    {
    public:

        TimerHeap() : 
            mBaseTimeMs(0)
        {
            rebase();
        }

        void rebase()
        {
            Lock lock(mTimerHeapMutex);

            boost::uint32_t timeNowMs = Platform::OS::getCurrentTimeMs();
            if (timeNowMs - mBaseTimeMs > 1000*60*60)
            {
                mBaseTimeMs = mTimerHeap.empty() ?
                    timeNowMs - 1000*10 :
                    mTimerHeap.top().first - 1000*10;

                mTimerHeap.setCompare( TimerCompare(mBaseTimeMs), false );
            }
        }


        typedef std::pair<boost::uint32_t, T> TimerEntry;

        void add(const TimerEntry & timerEntry)
        {
            Lock lock(mTimerHeapMutex);
            mTimerHeap.add(timerEntry);

        }

        void remove(const TimerEntry & timerEntry)
        {
            Lock lock(mTimerHeapMutex);
            mTimerHeap.remove(timerEntry);
        }

        bool compareTop(const TimerEntry & timerEntry)
        {
            Lock lock(mTimerHeapMutex);
            if (mTimerHeap.empty())
            {
                return false;
            }
            return mTimerHeap.top() == timerEntry;
        }

        std::size_t size() const
        {
            Lock lock(mTimerHeapMutex);
            return mTimerHeap.size();
        }

        bool empty() const
        {
            Lock lock(mTimerHeapMutex);
            return mTimerHeap.empty();
        }

        void clear()
        {
            Lock lock(mTimerHeapMutex);
            mTimerHeap.clear();
        }

        bool popExpiredEntry(TimerEntry & timerEntry)
        {
            Lock lock(mTimerHeapMutex);

            if (    !mTimerHeap.empty() 
                &&  getTimeoutMs( mTimerHeap.top() ) == 0)
            {
                timerEntry= mTimerHeap.top();
                mTimerHeap.pop();
                return true;
            }

            return false;
        }

        bool getExpiredEntry(TimerEntry & timerEntry)
        {
            Lock lock(mTimerHeapMutex);

            if (    !mTimerHeap.empty() 
                &&  getTimeoutMs( mTimerHeap.top() ) == 0)
            {
                timerEntry= mTimerHeap.top();
                return true;
            }

            return false;
        }

        boost::uint32_t getNextEntryTimeoutMs()
        {
            Lock lock(mTimerHeapMutex);

            return mTimerHeap.empty() ?
                boost::uint32_t(-1) :
                getTimeoutMs(mTimerHeap.top());
        }

    private:

        boost::uint32_t getTimeoutMs(const TimerEntry & timerEntry)
        {
            return generateTimeoutMs(timerEntry.first);
        }

        boost::uint32_t                 mBaseTimeMs;

        mutable Mutex                   mTimerHeapMutex;
        Heap<TimerEntry, TimerCompare>  mTimerHeap;
    };


} // namespace RCF

#endif // ! INCLUDE_RCF_HEAP_HPP
