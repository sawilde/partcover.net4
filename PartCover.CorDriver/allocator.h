#pragma once

typedef unsigned char byte_t;
typedef byte_t *byteptr_t;



struct PointedAllocatorChunk
{
	struct ChunkPoint
	{
		ChunkPoint() : next(0), offset(0), length(0) {}

		ChunkPoint* next;

		byteptr_t   offset;
		ptrdiff_t   length;
	};

	PointedAllocatorChunk(size_t bytes) 
		: m_next(0), m_data(static_cast<byteptr_t>(malloc(bytes))), m_len(bytes) {}

	~PointedAllocatorChunk() { free(m_data) ; }

	PointedAllocatorChunk* m_next;

	bool Contains(byteptr_t pointer) const { return pointer >= m_data && pointer < m_data + m_len; }

	byteptr_t Reserve(size_t count);
	void Deserve(byteptr_t pointer);

private:
	ChunkPoint* CreatePointAfter(ChunkPoint* previous, size_t len);

	ChunkPoint	m_point;
	byteptr_t	const m_data;
	size_t		const m_len; 
};

struct ForwardAllocatorChunk
{
	ForwardAllocatorChunk(size_t bytes) 
		: m_next(0), m_data(static_cast<byteptr_t>(malloc(bytes))), m_dataFilled(m_data), m_len(bytes) {}

	~ForwardAllocatorChunk() { free(m_data) ; }

	ForwardAllocatorChunk* m_next;

	bool Contains(byteptr_t pointer) const { return pointer >= m_data && pointer < m_data + m_len; }

	byteptr_t Reserve(size_t count) 
	{
		byteptr_t res = 0;
		if (m_dataFilled + count < m_data + m_len) 
		{
			res = m_dataFilled;
			m_dataFilled += count;
		}
		return res;
	}

	void Deserve(byteptr_t pointer);

private:
	byteptr_t	const m_data;
	size_t		const m_len; 
	byteptr_t	m_dataFilled;
};

template<class T, class chunk_t = PointedAllocatorChunk>
struct FieldAllocator
{
	enum { ValueSize = sizeof T };

	FieldAllocator() : m_chunk(0) {}
	~FieldAllocator() { Cleanup(); }

	T* Alloc(int count)
	{
		const int bytesNeeded = count * ValueSize;
		return static_cast<T*>(AllocChunkPoint(bytesNeeded));
	}

	void Release(T* pointer)
	{
		return FindChunk(pointer)->Deserve(pointer);
	}

	void Cleanup() 
	{
		while(m_chunk != 0) 
		{
			chunk_t* chunk = m_chunk;
			m_chunk = chunk->m_next;

			delete chunk;
		}
	}


private:
	FieldAllocator(const FieldAllocator& allocator) {}
	FieldAllocator& operator = (const FieldAllocator& allocator) const {}

	chunk_t* AppendChunk(size_t bytes)
	{
		chunk_t* chunk = new chunk_t(max(bytes, 1024 * 64));
		if (m_chunk != 0) {
			m_chunk->m_next = chunk;
		} else {
			m_chunk = chunk;
		}
		return chunk;
	}

	void* AllocChunkPoint(size_t bytes)
	{
		if (m_chunk == 0) {
			AppendChunk(bytes);
		}

		void* res = 0;
		chunk_t* chunk = m_chunk;
		do {
			res = chunk->Reserve(bytes);
			if (res != 0)
				return res;
			chunk = chunk->m_next;
		} while(chunk != 0);

		return AppendChunk(bytes)->Reserve(bytes);
	}

	chunk_t* FindChunk(byteptr_t pointer)
	{
		chunk_t* chunk = m_chunk;
		while(chunk != 0) 
		{
			if (chunk->IsAllocated(pointer))
				return chunk;
			chunk = chunk->m_next;
		}
		return 0;
	}

	chunk_t* m_chunk;
};

