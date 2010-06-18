#include "stdafx.h"
#include <iostream>

byteptr_t PointedAllocatorChunk::Reserve(size_t count)
{
	if (count > m_len) 
		return 0;

	byteptr_t pointPtr = m_point.offset;
	if (pointPtr == 0) {
		m_point.offset = m_data;
		m_point.length = count;
		return m_point.offset;
	}

	ChunkPoint* point = &m_point;
	while(point->next != 0) {
		byteptr_t pointEnd = point->offset + point->length;
		if (point->next->offset >= pointEnd + count) 
		{
			return CreatePointAfter(point, count)->offset;
		}

		point = point->next;
	};

	byteptr_t pointEnd = point->offset + point->length;
	if (m_data + m_len >= pointEnd + count) 
	{
		return CreatePointAfter(point, count)->offset;
	}

	return 0;
}

PointedAllocatorChunk::ChunkPoint* PointedAllocatorChunk::CreatePointAfter(ChunkPoint* previous, size_t len)
{
	ChunkPoint* point = new ChunkPoint;

	point->length = len;
	point->offset = previous->offset + previous->length;
	point->next = previous->next;
	previous->next = point;

	return point;
}


track_type * get_map() {
    // don't use normal new to avoid infinite recursion.
    static track_type * track = new (std::malloc(sizeof *track)) track_type;
    static track_printer printer(track);
    return track;
}

#ifdef TRACK_MEMORY_ALLOCATION

void * operator new(std::size_t size) throw(std::bad_alloc) {
    // we are required to return non-null
    void * mem = std::malloc(size == 0 ? 1 : size);
    if(mem == 0) {
        throw std::bad_alloc();
    }
    (*get_map())[mem] = size;
    return mem;
}

void operator delete(void * mem) throw() {
    if(get_map()->erase(mem) == 0) {
        // this indicates a serious bug
        //std::cerr << "bug: memory at " 
        //          << mem << " wasn't allocated by us\n";
    }
    std::free(mem);
}

#endif

void print_memory_usage() {
	long total = 0;
	track_type::const_iterator mem_it = get_map()->begin();
	while(mem_it != get_map()->end())
	{
		total += mem_it->second;
		std::cout << mem_it->second << " bytes" << std::endl;
		mem_it++;
	}
	std::cout << "Total " << total << " bytes";
}