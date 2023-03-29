#include "Body.h"

#include <FPSCounter.h>
#include <BitFont.h>

std::unique_ptr<TacticalExt::ExtData> TacticalExt::Data = nullptr;

void TacticalExt::ExtData::InitializeConstants() { }

void TacticalExt::Allocate(TacticalClass* pThis)
{
	Data = std::make_unique<TacticalExt::ExtData>(pThis);
}

void TacticalExt::Remove(TacticalClass* pThis)
{
	Data = nullptr;
}

// =============================
// load / save

template <typename T>
void TacticalExt::ExtData::Serialize(T& Stm)
{
	Stm
		;
}

void TacticalExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TacticalClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TacticalExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TacticalClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TacticalExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool TacticalExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}


// =============================
// container hooks
 
DEFINE_HOOK(0x6D1E24, TacticalClass_CTOR, 0x5)
{
	GET(TacticalClass*, pItem, ESI);

	TacticalExt::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6D1E9B, TacticalClass_DTOR, 0xA)
{
	GET(TacticalClass*, pItem, EBX);

	TacticalExt::Remove(pItem);
	return 0;
}

IStream* TacticalExt::g_pStm = nullptr;

DEFINE_HOOK_AGAIN(0x6DBD20, TacticalClass_SaveLoad_Prefix, 0x7)
DEFINE_HOOK(0x6DBE00, TacticalClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(IStream*, pStm, 0x8);

	TacticalExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x6DBDED, TacticalClass_Load_Suffix, 0x6)
{
	auto buffer = TacticalExt::Global();
	if (!buffer)
		Debug::FatalErrorAndExit("TacticalClassExt_Load Apparently TacticalExt Global Pointer is missing !/n ");

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(TacticalExt::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(TacticalExt::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x6DBE18, TacticalClass_Save_Suffix, 0x5)
{
	auto buffer = TacticalExt::Global();

	if (!buffer)
		Debug::FatalErrorAndExit("TacticalClassExt_Save Apparently TacticalExt Global Pointer is missing !/n ");

	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(TacticalExt::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(TacticalExt::g_pStm);

	return 0;
}