#include "Body.h"

#include <Utilities/Macro.h>

// =============================
// load / save

template <typename T>
void InfantryExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Initialized)
		.Process(this->IsUsingDeathSequence)
		.Process(this->CurrentDoType)
		.Process(this->ForceFullRearmDelay)
		.Process(this->SkipTargetChangeResetSequence)
		;
}

// =============================
// container

InfantryExtContainer InfantryExtContainer::Instance;

// =============================
// container hooks

ASMJIT_PATCH(0x517ACC, InfantryClass_CTOR, 0x6)
{
	GET(InfantryClass*, pItem, ESI);

	if(pItem->Type)
		InfantryExtContainer::Instance.Allocate(pItem);

	return 0;
}

ASMJIT_PATCH(0x517D90, InfantryClass_DTOR, 0x5)
{
	GET(InfantryClass* const, pItem, ECX);
	InfantryExtContainer::Instance.Remove(pItem);
	return 0;
}

ASMJIT_PATCH(0x521960, InfantryClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(InfantryClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);
	InfantryExtContainer::Instance.PrepareStream(pItem, pStm);

	return 0;
}ASMJIT_PATCH_AGAIN(0x521B00, InfantryClass_SaveLoad_Prefix, 0x8)

HRESULT __stdcall FakeInfantryClass::_Load(IStream* pStm)
{

	HRESULT res = this->InfantryClass::Load(pStm);

	if (SUCCEEDED(res))
	{
		InfantryExtContainer::Instance.ClearExtAttribute(this);
		auto buffer = InfantryExtContainer::Instance.AllocateUnchecked(this);
		InfantryExtContainer::Instance.SetExtAttribute(this, buffer);

		if (!buffer)
			return -1;

		PhobosByteStream loader { 0 };
		if (!loader.ReadBlockFromStream(pStm))
			return -1;

		PhobosStreamReader reader { loader };
		if (!reader.Expect(InfantryExtData::Canary))
			return -1;

		reader.RegisterChange(buffer);
		buffer->LoadFromStream(reader);

		if (reader.ExpectEndOfBlock())
		{
			return S_OK;
		}
	}

	return res;
}

HRESULT __stdcall FakeInfantryClass::_Save(IStream* pStm, bool clearDirty)
{

	HRESULT res = this->FootClass::Save(pStm, clearDirty);

	if (SUCCEEDED(res))
	{
		InfantryExtData* const buffer = InfantryExtContainer::Instance.GetExtAttribute(this);

		// write the current pointer, the size of the block, and the canary
		PhobosByteStream saver { InfantryExtData::size_Of() };
		PhobosStreamWriter writer { saver };

		writer.Save(InfantryExtData::Canary);
		writer.Save(buffer);

		// save the data
		buffer->SaveToStream(writer);

		// save the block
		if (!saver.WriteBlockToStream(pStm))
		{
			//Debug::LogInfo("[SaveGame] FakeAnimClass fail to write 0x%X block(s) to stream", saver.Size());
			return -1;
		}

		//Debug::LogInfo("[SaveGame] FakeAnimClass used up 0x%X bytes", saver.Size());
	}

	return res;
}

DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB06C, FakeInfantryClass::_Load)
DEFINE_FUNCTION_JUMP(VTABLE, 0x7EB070, FakeInfantryClass::_Save)

// ASMJIT_PATCH(0x51AA23, InfantryClass_Detach, 0x6)
// {
// 	GET(InfantryClass* const, pThis, ESI);
// 	GET(void*, target, EDI);
// 	GET_STACK(bool, all, STACK_OFFS(0x8, -0x8));
//
// 	InfantryExt::ExtMap.InvalidatePointerFor(pThis, target, all);
//
// 	return pThis->Type == target ? 0x51AA2B : 0x51AA35;
// }