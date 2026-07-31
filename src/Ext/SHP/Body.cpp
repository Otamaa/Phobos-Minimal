#include "Body.h"

#include <Ext/Convert/Blitters/Blitter.h>

#include <Utilities/Debug.h>

#include <Memory.h>

void SHPExtData::EnsureResident(bool verbose)
{
	auto pOwner = this->AttachedToObject;
	auto pAlpha = this->AlphaSHP;

	if (!Blitters.empty() && !RLeBlitters.empty())
	{
		if (!pOwner)
			return;

		if (!pOwner->Loaded)
			pOwner->Load();

		if (!pAlpha->Loaded)
		{
			pAlpha->Load();

			if (!pAlpha->Loaded)
				return;
		}

		if (!pOwner->Loaded)
			return;
	}

	if (this->AlphaSHP && verbose)
		Debug::Log("File %s alpha has been loaded successfully.\n", pOwner->Filename);
}

bool SHPExtData::LoadAlphaImage()
{
	auto pOwner = this->AttachedToObject;
	std::string filename = pOwner->Filename;

	const size_t tail = filename.size() >= 4 ? filename.size() - 4 : 0;
	if (_stricmp(filename.c_str() + tail, ".APH") == 0)
		return false;

	filename += ".APH";


	auto pAlpha = GameCreate<SHPReference>(filename.c_str());

	if (!pAlpha)
		return false;

	pAlpha->Load();

	bool accepted = false;

	if (pAlpha->Loaded)
	{
		if (pAlpha->Width == pOwner->Width
			&& pAlpha->Height == pOwner->Height
			&& pAlpha->Frames == pOwner->Frames)
		{
			this->AlphaSHP = pAlpha;
			accepted = true;
		}
	}

	if (!accepted)
	{
		pAlpha->Unload();
		GameDelete<false, false>(pAlpha);
		return false;
	}

	return true;
}