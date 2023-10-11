#pragma once

#include <list>

namespace cs_std
{
	/// <summary>
	/// Preallocates a region of memory upfront (or takes a pointer to a region of memory) and manages allocation and deallocation of memory within that region.
	/// </summary>
	class memory_allocator
	{
	private:
		struct region
		{
			void* memory;
			size_t size;
		};
		std::list<region> freeList;
		void* memory;
		// Size of the memory chunk in bytes
		size_t size;
	public:
		memory_allocator(size_t bytes) : memory(static_cast<void*>(new char[bytes])), size(bytes), freeList({ { this->memory, this->size } }) {}
		inline void* allocate(size_t bytes)
		{
			// Find the first region that has enough space
			// O(n), TODO improve later
			for (auto& region : this->freeList)
			{
				if (region.size >= bytes)
				{
					void* ptr = region.memory;
					region.memory = static_cast<char*>(region.memory) + bytes;
					region.size -= bytes;
					return ptr;
				}
			}
			// Should a well-conforming program need to check for nullptr?
			return nullptr;
		}
		inline void deallocate(void* region)
		{

		}
	};
}