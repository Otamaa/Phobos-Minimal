#include "Body.h"

#include <Ext/Convert/Blitters/Blitter.h>

#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Utilities/Macro.h>

#include <Memory.h>

HelperedVector <SHPExtData*> SHPExtData::Array;

void SHPExtData::FinalizeAlpha(SHPExtData* pExt, SHPCaches* pAlpha, SHPCaches* pShape)
{
	// The owner is re-read through [esi+4] here rather than reused; kept as-is.
	SHPCaches* const pOwner = pExt->AttachedToObject;

	// 1006FBE1..1006FBEF -- reads as "both caches already populated".
	// Always false at construction time (both vectors were just zeroed),
	// so the forced-load block below always runs. Preserved verbatim.
	bool const skipForcedLoad = !pExt->Cache10.empty() && !pExt->Cache1C.empty();

	if (!skipForcedLoad)
	{
		// Defensive null checks at 1006FBF1 / 1006FBF5. Both operands are
		// non-null on every reachable path, but the original tests them.
		if (!pOwner || !pAlpha)
			return;

		if (!pOwner->Loaded)
			pOwner->Load();

		if (!pAlpha->Loaded) {
			pAlpha->Load();

			if (!pAlpha->Loaded)
				return;
		}

		if (!pOwner->Loaded)
			return;
	}

	// 1006FC1F: note this re-tests the stored field, not the local.
	if (pExt->AlphaSHP)
		Debug::Log("File %s alpha has been loaded successfully.\n", pShape->Filename);
}

bool SHPExtData::LoadAlphaImage()
{
	auto pOwner = this->AttachedToObject;
	std::string filename = pOwner->Filename;
	constexpr const char* AlphaSuffix = ".APH";

	if (filename.size() >= 4
			&& _stricmp(filename.c_str() + filename.size() - 4, AlphaSuffix) == 0)
	{
		return false;
	}

	filename += AlphaSuffix;

	auto pAlpha = GameCreate<SHPCaches>(filename.c_str());

	if (!pAlpha)
		return false;

	pAlpha->Load();

	bool accepted = false;

	if (pAlpha->Loaded)
	{
		if (pAlpha->CurrentHeader.Width == pOwner->CurrentHeader.Width
			&& pAlpha->CurrentHeader.Height == pOwner->CurrentHeader.Height
			&& pAlpha->CurrentHeader.Frames == pOwner->CurrentHeader.Frames)
		{
			this->AlphaSHP = pAlpha;
			accepted = true;
		}
	}

	if (!accepted) {
		GameDelete<true, false>(pAlpha);
		return false;
	}

	return true;
}

ASMJIT_PATCH(0x69E4F0, SHPReference_CTOR, 5)
{
	GET(SHPCaches*, pThis, ESI);

	auto pExt = new SHPExtData();

	pExt->AttachedToObject = pThis;

	SHPExtData::Array.push_back(pExt);

	if (!pExt->AlphaSHP) {
		pExt->LoadAlphaImage();

		if (!pExt->AlphaSHP)
			return 0;   // every failure path falls straight to the epilogue
	}

	SHPExtData::FinalizeAlpha(pExt, pExt->AlphaSHP, pThis);
	return 0;
}

ASMJIT_PATCH(0x69E509, SHPReference_DTOR, 5)
{
	GET(SHPCaches*, pThis, ESI);

	auto it = SHPExtData::Array.find_if([pThis](SHPExtData* pSHP) {
		return pSHP->AttachedToObject == pThis;
	});

	if (it != SHPExtData::Array.end()) {
		(*it)->Cache1C.ResetSize();
		(*it)->Cache10.ResetSize();

		if (auto pAlph = (*it)->AlphaSHP) {
			GameDelete<true,false>(pAlph);
			(*it)->AlphaSHP = nullptr;
		}

		delete *it;
		SHPExtData::Array.erase(it);
	}

	return 0;
}