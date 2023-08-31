
#include <Phobos.h>

struct AllocatorData
{
	size_t m_blocksize;
	uintptr_t m_caller;
};

static std::unordered_map<void* , AllocatorData> Data {};


DEFINE_HOOK(0x7C8E17, Game_OperatorNew, 0x6)
{
	GET_STACK(size_t, blocksize, 0x4);
	GET_STACK(uintptr_t, caller, 0x0);

	void* pointer = CRT::malloc(blocksize , CRT::AllocatorMode());
	Data[pointer] = { blocksize , caller };
	R->EAX(pointer);
	return 0x7C9441;
}

DEFINE_HOOK(0x7C8B3D, Game_OperatorDelete, 0x9)
{
	GET_STACK(void*, ptr, 0x4);

	if(Data.contains(ptr))
		Data.erase(ptr);

	CRT::free(ptr);
	return 0x7C8B47;
}