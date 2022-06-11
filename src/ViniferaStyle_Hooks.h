#pragma once
#include "Phobos.h"

#ifdef aaa
static uintptr_t GetAddr(const char* pExportName, uintptr_t nOffs)
{
	void* addr = nullptr;
	if (!Phobos::AresModuleExportData.empty())
	{
		for (auto const& Export : Phobos::AresModuleExportData)
		{
			if (_strcmpi(Export.name, pExportName) == 0)
			{
				addr = Export.address;
				break;
			}
		}
	}

	if (addr)
	{
		return ((uintptr_t)addr) + nOffs;
	}

	return 0x0;
}

namespace Vinifera_Style
{
	void RegisterHooks();
}
#endif