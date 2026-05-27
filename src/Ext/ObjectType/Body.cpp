#include "Body.h"

#include <Ext/House/Body.h>
#include <Ext/Building/Body.h>

#include <Utilities/Cast.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

BuildingClass* __fastcall FakeObjectTypeClass::WhoCanBuildMe(ObjectTypeClass* pThis, discard_t, bool intheory, bool bool2, bool legal, HouseClass* house)
{
	if(auto pTechno = type_cast<TechnoTypeClass*>(pThis)) {
		const auto nBuffer = HouseExtData::HasFactory(
			house,
			pTechno,
			intheory,
			bool2,
			legal,
		false);

		return nBuffer.first >= NewFactoryState::Available_Alternative ?
			nBuffer.second : nullptr;
	}

	return nullptr;
}


//#include <Ext/AnimType/Body.h>
//#include <Ext/AircraftType/Body.h>
//#include <Ext/BuildingType/Body.h>
//#include <Ext/BulletType/Body.h>
//#include <Ext/OverlayType/Body.h>
//#include <Ext/ParticleType/Body.h>
//#include <Ext/ParticleSystemType/Body.h>
//#include <Ext/SmudgeType/Body.h>
//#include <Ext/TerrainType/Body.h>
//#include <Ext/UnitType/Body.h>
//#include <Ext/InfantryType/Body.h>
//#include <Ext/VoxelAnimType/Body.h>
//
//HRESULT __stdcall FakeObjectTypeClass::__Load(ObjectTypeClass* pThis, IStream* pStm)
//{
//	AbstractType absType = AbstractType::None;
//	HRESULT hr = pStm->Read(&absType, sizeof(AbstractType), nullptr);
//
//	if (!SUCCEEDED(hr)) return hr;
//
//	using LoadStaticFn = void(__cdecl*)(void* key);
//	
//	static LoadStaticFn _Ext_Load = nullptr;
//
//	//some of the extension require to be avaible as son the ObjectTypeClass_Load avaible
//	switch (absType)
//	{
//	case AbstractType::AnimType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new AnimTypeExtData((AnimTypeClass*)pThis, noinit_t()));
//		_Ext_Load = (LoadStaticFn)AnimTypeExtContainer::LoadStatic;
//		break;
//	case AbstractType::AircraftType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new AircraftTypeExtData((AircraftTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::BuildingType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new BuildingTypeExtData((BuildingTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::BulletType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new BulletTypeExtData((BulletTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::OverlayType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new OverlayTypeExtData((OverlayTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::ParticleType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new ParticleTypeExtData((ParticleTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::ParticleSystemType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new ParticleSystemTypeExtData((ParticleSystemTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::SmudgeType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new SmudgeTypeExtData((SmudgeTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::TerrainType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new TerrainTypeExtData((TerrainTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::UnitType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new UnitTypeExtData((UnitTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::InfantryType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new InfantryTypeExtData((InfantryTypeClass*)pThis, noinit_t()));
//		break;
//	case AbstractType::VoxelAnimType:
//		(*(uintptr_t*)((char*)pThis + AbstractExtOffset)) = uintptr_t(new VoxelAnimTypeExtData((VoxelAnimTypeClass*)pThis, noinit_t()));
//		break;
//	default:
//		break;
//	}
//
//	if (_Ext_Load) {
//		_Ext_Load(pThis);
//	}
//
//	return pThis->ObjectTypeClass::Load(pStm)
//}
//
//DEFINE_FUNCTION_JUMP(CALL, 0x428814, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(CALL, 0x46C6AB, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(CALL, 0x549CBF, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(CALL, 0x5FEB04, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(CALL, 0x6447EB, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(CALL, 0x64566F, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(CALL, 0x6B585B, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(CALL, 0x71641C, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(CALL, 0x71E1DB, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(CALL, 0x74B81B, FakeObjectTypeClass::__Load)
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF2EC, FakeObjectTypeClass::__Load)
//
//HRESULT __stdcall FakeObjectTypeClass::__Save(ObjectTypeClass* pThis, IStream* pStm, BOOL fClearDirty)
//{
//	//additionally save the AbstractType for loading the extension
//	AbstractType absType = AbstractType::None;
//
//	if (auto ptr = (AbstractExtended*)(*(uintptr_t*)((char*)pThis + AbstractExtOffset))) {
//		absType = ptr->AbsType;
//	}
//
//	//non extended class will always return something so it wont break the game save
//	HRESULT hr = pStm->Write(&absType, sizeof(AbstractType), nullptr);
//	if (!SUCCEEDED(hr)) return hr;
//
//	return pThis->ObjectTypeClass::Save(pStm, fClearDirty);
//}
//DEFINE_FUNCTION_JUMP(CALL, 0x428814, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x42897F, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x46C73F, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x549D7F, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x5FEC1F, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x64483F, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x6457B3, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x6B58BF, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x716DD1, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x71E24F, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(CALL, 0x74B8DF, FakeObjectTypeClass::__Save)
//DEFINE_FUNCTION_JUMP(VTABLE, 0x7EF2F0, FakeObjectTypeClass::__Save)