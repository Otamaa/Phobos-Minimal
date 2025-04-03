#include "Body.h"

#include <FPSCounter.h>
#include <BitFont.h>

//IStream* TacticalExt::g_pStm = nullptr;
//std::unique_ptr<TacticalExt::ExtData> TacticalExt::Data = nullptr;
//
//void TacticalExt::Allocate(TacticalClass* pThis)
//{
//	Data = std::make_unique<TacticalExt::ExtData>(pThis);
//}
//
//void TacticalExt::Remove(TacticalClass* pThis)
//{
//	Data = nullptr;
//}

// =============================
// load / save

//template <typename T>
//void TacticalExt::ExtData::Serialize(T& Stm)
//{
//	Stm
//		.Process(this->Initialized)
//		;
//}

// =============================
// container hooks

//ASMJIT_PATCH(0x6D1E24, TacticalClass_CTOR, 0x5)
//{
//	GET(TacticalClass*, pItem, ESI);
//
//	TacticalExt::Allocate(pItem);
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x6DC48E, TacticalClass_DTOR_A, 0xA)
//{
//	GET(TacticalClass*, pItem, ESI);
//	TacticalExt::Remove(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH(0x6D1E9B, TacticalClass_DTOR_B, 0xA)
//{
//	GET(TacticalClass*, pItem, ECX);
//	TacticalExt::Remove(pItem);
//	return 0;
//}
//
//ASMJIT_PATCH_AGAIN(0x6DBD20, TacticalClass_SaveLoad_Prefix, 0x7)
//ASMJIT_PATCH(0x6DBE00, TacticalClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(IStream*, pStm, 0x8);
//
//	TacticalExt::g_pStm = pStm;
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x6DBDED, TacticalClass_Load_Suffix, 0x6)
//{
//	auto buffer = TacticalExt::Global();
//	if (!buffer)
//		Debug::FatalErrorAndExit("TacticalClassExt_Load Apparently TacticalExt Global Pointer is missing !/n ");
//
//	PhobosByteStream Stm(0);
//	if (Stm.ReadBlockFromStream(TacticalExt::g_pStm))
//	{
//		PhobosStreamReader Reader(Stm);
//
//		if (Reader.Expect(TacticalExt::ExtData::Canary) && Reader.RegisterChange(buffer))
//			buffer->LoadFromStream(Reader);
//	}
//
//	return 0;
//}
//
//ASMJIT_PATCH(0x6DBE18, TacticalClass_Save_Suffix, 0x5)
//{
//	auto buffer = TacticalExt::Global();
//
//	if (!buffer)
//		Debug::FatalErrorAndExit("TacticalClassExt_Save Apparently TacticalExt Global Pointer is missing !/n ");
//
//	PhobosByteStream saver(sizeof(TacticalExt));
//	PhobosStreamWriter writer(saver);
//
// writer.Save(TacticalExt::Canary);
// writer.Save(buffer);
//
//	buffer->SaveToStream(writer);
//	saver.WriteBlockToStream(TacticalExt::g_pStm);
//
//	return 0;
//}
