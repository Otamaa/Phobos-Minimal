#include "allocator.h"

void* operator new[](size_t size, const char*, int, unsigned, unsigned, const char*){
	return malloc(size);
}

void operator delete[](void* p, const char*, int, unsigned, unsigned, const char*) {
	free(p);
}

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line) {
	(void)pName; (void)flags; (void)debugFlags; (void)file; (void)line;  // Avoid unused warnings
	return malloc(size);
}

void operator delete[](void* ptr, const char*, int, unsigned, const char*, int) noexcept {
	free(ptr);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	(void)pName; (void)flags; (void)debugFlags; (void)file; (void)line; // Avoid unused warnings

#if defined(_MSC_VER)  // Windows
	return _aligned_malloc(size, alignment);
#elif defined(__APPLE__) || defined(__linux__)  // macOS/Linux
	void* ptr = nullptr;
	if (posix_memalign(&ptr, alignment, size) != 0)
	{
		return nullptr;
	}
	return ptr;
#else
	return aligned_alloc(alignment, size);  // C++17+
#endif
}

void operator delete[](void* ptr, size_t, size_t, const char*, int, unsigned, const char*, int) noexcept
{
#if defined(_MSC_VER)
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}