#include <Phobos.h>

#include <Utilities/Debug.h>
#include <type_traits>
#include <utility>
#include <shared_mutex>
#include <unordered_set>
#include <Utilities/LambdaPatcher.h>

namespace Containers
{
	std::mutex s_mutex;
	std::unordered_set<uint64_t> s_resources;
}

void Test(REGISTERS* R)
{
	const std::lock_guard<std::mutex> lock(Containers::s_mutex);
}

DEFINE_HOOK(0x530277, MixFile_BoostTrap_FixLog, 0x6)
{
	LEA_STACK(char*, pFilename, STACK_OFFS(0x78, 0x40));

	Test(R);
	Debug::Log("%s Loaded! \n", pFilename);

	return 0x530289;
}


// Bruh ,..
DEFINE_VTABLE_PATCH(0x7E3D4C, fnptr<HouseClass* (BombClass*, void*)>([](BombClass* pThis, void* _) {
	Debug::Log(__FUNCTION__" Called ! \n");
	return pThis->Owner ? pThis->Owner->Owner : pThis->OwnerHouse;
	}));
