#include "stdafx.h"

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