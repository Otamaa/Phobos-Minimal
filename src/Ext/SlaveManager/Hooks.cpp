#include "Body.h"

ASMJIT_PATCH(0x6AF588, SlaveManagerClass_Enslave_MissingOriginalOwner, 0xD)
{
	GET(SlaveManagerClass*, pManage, ESI);

	if (pManage->Owner)
		pManage->Owner->SlaveManager = nullptr;

	return 0x6AF595;
}